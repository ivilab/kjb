/**
 * @file
 * @brief implementation of C++ helper class for FFT-based convolution in FFTW
 * @author Kyle Simek
 * @author Prasad Gabur
 * @author Kobus Barnard
 * @author Andrew Predoehl
 */

/* $Id: m_convolve.cpp 15035 2013-07-29 23:32:59Z predoehl $ */
/* {{{======================================================================= *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or
   |  fitness for any particular task. Nonetheless, I am interested in hearing
   |  about problems that you encounter.
   |
 * ====================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m/m_find.h"
#include "l_cpp/l_exception.h"
#include "m_cpp/m_convolve.h"
#include "m/m_convolve.h"
#include "m_cpp/m_int_vector.h"

#ifdef KJB_HAVE_FFTW

#include "m_cpp/m_flip.h"

#include <algorithm>
#include <functional>

/*
 * If you set the following macro flag to a positive value, you can use the
 * basic fftw_execute() function on your plan.  If you set this flag to zero,
 * this only calls the new-array-execute functions, but as a consequence we
 * can unify the reentrant and non-reentrant code neatly.
 */
#define WANT_BEST_PERFORMANCE_EVEN_IF_IT_MAKES_THE_CODE_UGLY 0

namespace
{

const char* MASK_NAG =
    "Mask not yet set.  You must call Fft_convolution_2d::set_mask()\n"
    "before first call to convolve() or reflect_and_convolve().";

typedef kjb::Fftw_convolution_2d::Sizes FftSizes;

/**
 * FFTW likes array dimensions that factors as (2^a 3^b 5^c 7^d 11^e 13^f)
 * where a through f are integers and (e+f <=1)
 *
 * This function receives an integer and pads it to a number that fits this
 * constraint.
 *
 * Note: this function is not optimal (e.g. passing 169 returns 182, where 180
 * would be better), but it should be a decent heuristic.
 *
 * Note: The problem is very similar to an unbounded knapsack problem, NP-hard.
 */
unsigned pad_to_friendly_size_(unsigned n)
{
    static const unsigned factors[] = {2, 3, 5, 7, 11, 13};

    unsigned result = 1;
    size_t i, num_factors = sizeof factors / sizeof(unsigned);
    while (n > 1)
    {
        for (i = 0; i < num_factors; ++i)
        {
            unsigned f = factors[i];
            if (n%f == 0)
            {
                // can only use one of 11 or 13, and can only use it once
                if (f == 11 || f == 13) num_factors -= 2;
                n /= f;
                result *= f;
                break;
            }
        }

        if (i >= num_factors) n += 1;
    }
    return result;
}

/**
 * @brief return the smallest power of 2 equal to or exceeding n
 *
 * This sometimes works better than Kyle's "friendly size" heuristic.
 */
unsigned pad_to_binary_size(unsigned n)
{
    unsigned result;
    for (result = 1; result < n; result <<= 1)
        ;
    return result;
}



inline double& Re(FFTW::fftw_complex* p) { return p[0][0]; }

inline double& Im(FFTW::fftw_complex* p) { return p[0][1]; }

inline const double& Re(const FFTW::fftw_complex* p) { return p[0][0]; }

inline const double& Im(const FFTW::fftw_complex* p) { return p[0][1]; }


template <class T>
void padded_to_data(double* padded_data, T* out, const FftSizes& s)
{
    // remove padding
    const int mask_row_offset = s.mask_rows / 2,
              mask_col_offset = s.mask_cols / 2,
              cols_remnant = s.pad_cols - s.data_cols - mask_col_offset;

    padded_data += mask_row_offset * s.pad_cols;    // start row offset
    for (int i=0; i < s.data_rows; ++i)
    {
        padded_data += mask_col_offset;    // start column offset
        for (int j=0; j < s.data_cols; ++j)
        {
            *out++ = *padded_data++;
        }
        padded_data += cols_remnant;
    }
}

template <class MATRIX>
void matrix_to_padded(const MATRIX& in, double* out, const FftSizes& s)
{
    const typename MATRIX::Value_type *data = in.get_c_matrix() -> elements[0];
    const int cols_remnant = s.pad_cols - in.get_num_cols();
    std::fill_n(out, s.Nreal(), 0.0);
    for (int r = 0; r < in.get_num_rows(); ++r, out += cols_remnant)
    {
        for (int c = 0; c < in.get_num_cols(); ++c)
        {
            *out++ = *data++;
        }
    }
}


template <typename T>
void postprocess(double* data, T* out, const FftSizes& s)
{
    // normalize inverse_fftw result
    const int N = s.Nreal();
    std::transform(data, data + N, data,
                   std::bind1st(std::multiplies<double>(), 1.0/double(N)));
    padded_to_data(data, out, s);
}


void prep(const kjb::Matrix& mat_in, kjb::Matrix& mat_out, const FftSizes& s)
{
    if (! s.is_matrix_size_same_as_data_size(mat_in))
    {
        KJB_THROW(kjb::Dimension_mismatch);
    }

    // a no-op if size is already correct:
    mat_out.resize(s.data_rows, s.data_cols);
}


void ow_multiply_fftw_(
    kjb::FFTW_complex_vector* v1,
    const kjb::FFTW_complex_vector& v2,
    const int Nc
)
{
    FFTW::fftw_complex* f1 = v1 -> begin();
    const FFTW::fftw_complex* f2 = v2.begin();
    // element-wise complex-multiplication
    for (int i=0; i < Nc; ++i, ++f1, ++f2)
    {
        const double out_re = Re(f1) * Re(f2) - Im(f1) * Im(f2),
                     out_im = Re(f1) * Im(f2) + Re(f2) * Im(f1);
        Re(f1) = out_re;
        Im(f1) = out_im;
    }
}


// special case:  output mr is almost a copy of m but has a duplicate last row
void copy_last_row(const kjb::Matrix& m, kjb::Matrix* mr)
{
    NTX(mr);
    *mr = m;
    mr -> resize(1 + m.get_num_rows(), m.get_num_cols());
    mr -> set_row(m.get_num_rows(), m.get_row(m.get_num_rows() - 1));
}


void add_top_and_bottom_reflection(
    const kjb::Matrix& m,
    kjb::Matrix* mr,
    int tb_size,
    int bb_size
)
{
    using kjb_c::copy_matrix_with_selection_2;

    NTX(mr);
    if (tb_size < 0 || bb_size < tb_size || tb_size + bb_size <= 0)
    {
        KJB_THROW(kjb::Illegal_argument);
    }

    // special case:  zero top reflection, one row of bottom reflection
    if (0 == tb_size)
    {
        if (bb_size != 1) KJB_THROW(kjb::Cant_happen);
        copy_last_row(m, mr);
        return;
    }

    // Get vertically flipped copy of input; extract top, bottom borders
    kjb::Matrix v(m);
    v.ow_vertical_flip();
    const kjb_c::Matrix *vp = v.get_c_matrix();

    // Define top, bottom border selection.
    kjb::Int_vector b_sel(m.get_num_rows(), 0), t_sel(m.get_num_rows(), 0);
    std::fill_n(&b_sel[0], bb_size, 1);
    KJB(ASSERT(tb_size <= m.get_num_rows()));
    std::fill_n(&t_sel[m.get_num_rows() - tb_size], tb_size, 1);

    // Extract the top and bottom border elements
    kjb_c::Matrix *bb_p = 0, *tb_p = 0, *mbt_p = 0;
    ETX(copy_matrix_with_selection_2(&bb_p, vp, b_sel.get_c_vector(), 0));
    kjb::Matrix b_border(bb_p);
    ETX(copy_matrix_with_selection_2(&tb_p, vp, t_sel.get_c_vector(), 0));
    kjb::Matrix t_border(tb_p);

    /*
     * Concatenate original matrix, bottom border, and top border, which
     * (surprisingly) works, because the DFT is naturally cyclic.
     */
    const kjb_c::Matrix *ar[3];
    ar[0] = m.get_c_matrix();
    ar[1] = bb_p;
    ar[2] = tb_p;
    ETX(kjb_c::concat_matrices_vertically(&mbt_p, 3, ar));
    kjb::Matrix mbt(mbt_p);
    mr -> swap(mbt);
}


void ow_copy_last_column(kjb::Matrix* mr)
{
    NTX(mr);
    const kjb::Vector lc = mr -> get_col(mr -> get_num_cols() - 1);
    mr -> resize(mr -> get_num_rows(), 1 + mr -> get_num_cols());
    mr -> set_col(mr -> get_num_cols() - 1, lc);
}



void ow_add_left_and_right_reflection(
    kjb::Matrix* mr,
    int lb_size,
    int rb_size
)
{
    using kjb_c::copy_matrix_with_selection_2;

    NTX(mr);
    if (lb_size < 0 || rb_size < lb_size || lb_size + rb_size <= 0)
    {
        KJB_THROW(kjb::Illegal_argument);
    }

    // special case:  zero left border, unity-width right border
    if (0 == lb_size)
    {
        if (rb_size != 1) KJB_THROW(kjb::Cant_happen);
        ow_copy_last_column(mr);
        return;
    }

    // Get horizontally flipped version of mr
    kjb::Matrix h(*mr);
    h.ow_horizontal_flip();
    const kjb_c::Matrix *hp = h.get_c_matrix();

    // Define left, right borders.
    kjb::Int_vector l_sel(h.get_num_cols(), 0), r_sel(h.get_num_cols(), 0);
    std::fill_n(&r_sel[0], rb_size, 1);
    KJB(ASSERT(lb_size <= h.get_num_cols()));
    std::fill_n(&l_sel[h.get_num_cols() - lb_size], lb_size, 1);

    // Extract the left, right border elements
    kjb_c::Matrix *l_border_p = 0, *r_border_p = 0, *m3_p = 0;
    ETX(copy_matrix_with_selection_2(&l_border_p, hp, 0,l_sel.get_c_vector()));
    kjb::Matrix l_border(l_border_p);
    ETX(copy_matrix_with_selection_2(&r_border_p, hp, 0,r_sel.get_c_vector()));
    kjb::Matrix r_border(r_border_p);

    // Concatenate m2 matrix, right border, and left border.  Cyclic magic.
    const kjb_c::Matrix *ar[3];
    ar[0] = mr -> get_c_matrix();
    ar[1] = r_border_p;
    ar[2] = l_border_p;
    ETX(kjb_c::concat_matrices_horizontally(&m3_p, 3, ar));
    kjb::Matrix m3(m3_p);
    mr -> swap(m3);
}


/**
 * @brief pad matrix with its own reflection, and copy into a buffer
 * @param[i] m input matrix to be frilled with some of its own reflection
 * @param[o] buf start of buffer where we will write the frilled matrix.
 * @param[i] s aggregate structure storing sizes of relevant FFT plan
 * @pre buf is assumed to be large enough to hold the output.
 * @pre number of (rows, columns) of m agrees with (s.data_rows, s.data_cols).
 * @throws kjb::KJB_error if anything goes wrong
 *
 * In many image processing applications, we handle the boundary cases (i.e.,
 * first few rows, last few rows, first few columns, etc.) by modeling the
 * input as reflected on its borders.  This makes no sense in, say, speech
 * processing, but it does make sense with many textures.  This function does
 * the thankless task of not only copying the input matrix into buffer 'buf'
 * but also surrounding the matrix with a layer of its own reflection.  The
 * thickness of this layering depends on the size of the mask, which in turn
 * affects the size of the output buffer.  Basically, we fill all surplus space
 * in the output buffer with reflection.  The details are tedious.
 */
void reflect_into_input_buf(
    const kjb::Matrix& m,
    double *buf,
    const FftSizes& s
)
{
    using kjb_c::copy_matrix_with_selection_2;

    const int   h_excess = s.pad_cols - s.data_cols,
                v_excess = s.pad_rows - s.data_rows,
                tb_size = v_excess / 2,         // top border size
                lb_size = h_excess / 2,         // left border size
                bb_size = v_excess - tb_size,   // bottom border size
                rb_size = h_excess - lb_size;   // right border size

    kjb::Matrix mr;

    KJB(ASSERT(m.get_num_rows() == s.data_rows));
    KJB(ASSERT(m.get_num_cols() == s.data_cols));

    if (v_excess > 0)
    {
        add_top_and_bottom_reflection(m, &mr, tb_size, bb_size);
    }
    else
    {
        mr = m;
    }

    if (h_excess > 0)
    {
        ow_add_left_and_right_reflection(&mr, lb_size, rb_size);
    }

    kjb::Matrix::Value_type* mrbegin = & mr.at(0);
    std::copy(mrbegin, mrbegin + s.Nreal(), buf);
}


}

#endif



unsigned pad_best(unsigned n)
{
#if KJB_HAVE_FFTW
    return std::min(pad_to_friendly_size_(n), pad_to_binary_size(n));
#else
    return 0;
#endif
}



/* / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ */ 

namespace kjb
{


Fftw_convolution_2d::Sizes::Sizes(int dr, int dc, int mr, int mc)
:   data_rows(dr),
    data_cols(dc),
    mask_rows(mr),
    mask_cols(mc),
    pad_rows(pad_best(mr + dr - 1)),
    pad_cols(pad_best(mc + dc - 1))
{}



/**
 * @brief Initialize the convolver by specifying data and mask dimensions.
 *
 * @param data_num_rows exact number of rows in each data matrix
 * @param data_num_cols exact number of col in each data matrix
 * @param mask_max_rows maximum number of rows in convolution mask (kernel)
 * @param mask_max_cols maximum number of cols in convolution mask (kernel)
 * @param tuning_algorithm The approach that FFTW should use to tune its fft
 *          algorithm.  Acceptable values in order of increaing performance
 *          (and initialization time) are FFTW_ESTIMATE, FFTW_MEASURE,
 *          FFTW_PATIENT, FFTW_EXHAUSTIVE.  Default is FFTW_MEASURE, which is a
 *          good balance of performance and startup-time.  FFT_PATIENT gives
 *          ~1.4x speedup, at the expense of several extra seconds of
 *          additional startup time.  In parallel mode, FFT_PATIENT should
 *          show even greater speedup (unconfirmed claim).
 *
 * Mask data is specified later by calling set_mask().
 * This ctor sets up the FFTW plans, and thus it will may take a few seconds.
 *
 * @todo see if this has a bug when the given mask is smaller than the
 *       dimensions given here:  it might cause the output to shift.
 */
Fftw_convolution_2d::Fftw_convolution_2d(
    int data_num_rows,
    int data_num_cols,
    int mask_max_rows,
    int mask_max_cols,
    int fft_alg_type
)
:   sizes_(data_num_rows, data_num_cols, mask_max_rows, mask_max_cols),
    fft_algorithm_type_(fft_alg_type | FFTW_DESTROY_INPUT),
    is_mask_set_(false)
#ifdef KJB_HAVE_FFTW
    ,    /* Sorry for the ugly syntax here.  It is to accomodate NO_LIBS. */
    mask_(alloc_work_buf_impl_()),
    data_(alloc_work_buf_impl_()),
    mask_plan_(new FFTW_plan_r2c(sizes_.pad_rows, sizes_.pad_cols,
        mask_.first -> begin(), mask_.second -> begin(), fft_algorithm_type_)),
    data_plan_(new FFTW_plan_r2c(sizes_.pad_rows, sizes_.pad_cols,
        data_.first -> begin(), data_.second -> begin(), fft_algorithm_type_)),
    data_inv_plan_(new FFTW_plan_c2r(sizes_.pad_rows, sizes_.pad_cols,
        data_.second -> begin(), data_.first -> begin(), fft_algorithm_type_))
#endif
{
#ifndef KJB_HAVE_FFTW
    KJB_THROW(Missing_dependency);
#endif
}




#ifdef KJB_HAVE_FFTW
void Fftw_convolution_2d::set_mask(const Matrix& m)
{
    // TODO:  see if the mask must be centered (it might be small)
    is_mask_set_ = true;
    if (! sizes_.is_matrix_size_within_mask_size(m))
    {
        KJB_THROW(Dimension_mismatch);
    }

    // fourier transform mask
    matrix_to_padded(m, mask_.first -> begin(), sizes_);
    mask_plan_ -> execute();
}


void Fftw_convolution_2d::set_gaussian_mask(double sigma)
{
    int mask_size = sigma * 6 + 0.5;
    kjb_c::Matrix* cmat = NULL;
    ETX(kjb_c::get_2D_gaussian_mask(&cmat, mask_size, sigma));

    Matrix mat(cmat);
    set_mask(mat);
}




/**
 * @brief a cover-your-tuchus function
 *
 * This function is called only if the caller does two things in sequence:
 * (1) calls allocate_work_buffer(), and (2) calls a two-parameter convolution.
 * That is expected to be rare, since (1) should occur only for a multithreaded
 * application, and (2) is not reentrant and should only occur when just one
 * thread is calling.  Why would anyone mix calls like that to one object?
 *
 * Event (1) causes this object to give away its internal buffer called data_.
 * Event (2) requires the data_ buffer.  So, we have to reallocate the buffer.
 *
 * Still, I want the class to keep providing correct output.
 *
 * This function is not reentrant!
 */ 
void Fftw_convolution_2d::reset_data_plan_() const
{
    KJB(TEST_PSE(("Expensive call to %s, due to mixing reentrant and"
                    " non-reentrant\nconvolutions with one Fftw_convolution_2d"
                    " object.  You will get better\nperformance if you "
                    "separate those calls by using different objects.\n",
                    __func__)));

    data_ = alloc_work_buf_impl_();

#if WANT_BEST_PERFORMANCE_EVEN_IF_IT_MAKES_THE_CODE_UGLY 
    KJB(UNTESTED_CODE());
    data_plan_.reset(new FFTW_plan_r2c( sizes_.pad_rows, sizes_.pad_cols,
        data_.first -> begin(), data_.second -> begin(), fft_algorithm_type_));
    data_inv_plan_.reset(new FFTW_plan_c2r( sizes_.pad_rows, sizes_.pad_cols,
        data_.second -> begin(), data_.first -> begin(), fft_algorithm_type_));
#endif
}



// because this alters the data_ field, this is not reentrant.
void Fftw_convolution_2d::convolution_by_dft_() const
{
#if WANT_BEST_PERFORMANCE_EVEN_IF_IT_MAKES_THE_CODE_UGLY 
    if (! is_mask_set_)
    {
        KJB_THROW_2(Runtime_error, MASK_NAG);
    }
    data_plan_ -> execute(); // possibly faster than new array execute ???
    const FFTW_complex_vector& mask_fft = * mask_.second.get();
    ow_multiply_fftw_(data_.second.get(), mask_fft, sizes_.Ncomplex());
    data_inv_plan_ -> execute();
#else
    convolution_by_dft_(data_);  // simpler code; still not reentrant!
#endif
}


/**
 * @brief perform convolution on input, and a predefined mask (kernel)
 * @param[i] mat input matrix, to be convolved with a predefined mask
 * @param[o] mat_out output matrix, where convolved result is written
 * @pre must have already defined the mask (i.e., the filter kernel)
 * @throws Dimension_mismatch if the input size (rows and columns) differ
 *         from the sizes given to the ctor.
 *
 * Parameters mat and mat_out may safely refer to the same Matrix.
 * This class requires you define the mask somehow, prior to calling this
 * function.
 * You can define the mask by calling set_mask() or set_gaussian_mask().
 * Boundary effects are handled by zero-padding behind the scenes.
 * If you do not want zero-padding, but reflection, @see reflect_and_convolve.
 *
 * Because it calls reset_data_plan_(), this function is not reentrant.
 */ 
void Fftw_convolution_2d::convolve(const Matrix& mat, Matrix& mat_out) const
{
    if (!data_.first) reset_data_plan_();
    prep(mat, mat_out, sizes_);
    matrix_to_padded(mat, data_.first -> begin(), sizes_);
    convolution_by_dft_();
    postprocess(data_.first -> begin(), &mat_out(0, 0), sizes_);
}


// Because it calls reset_data_plan_(), this function is not reentrant.
void Fftw_convolution_2d::reflect_and_convolve(
    const Matrix& mat_in,
    Matrix& mat_out
)   const
{
    if (!data_.first) reset_data_plan_();
    prep(mat_in, mat_out, sizes_);
    reflect_into_input_buf(mat_in, data_.first -> begin(), sizes_);
    convolution_by_dft_();
    postprocess(data_.first -> begin(), &mat_out(0, 0), sizes_);
}



/* / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ */ 





/**
 * @brief allocate memory for the DFT and inverse-DFT for one side of a convo
 * @warning not thread safe
 */
Fftw_convolution_2d::Work_buffer
Fftw_convolution_2d::alloc_work_buf_impl_() const
{
    Work_buffer b;
    b.first.reset(new FFTW_real_vector(sizes_.Nreal()));
    b.second.reset(new FFTW_complex_vector(sizes_.Ncomplex()));
    return b;
}


// reentrant version
void Fftw_convolution_2d::convolve(
    const Matrix& in,
    Matrix& out,
    Work_buffer b
)   const
{
    prep(in, out, sizes_);
    matrix_to_padded(in, b.first -> begin(), sizes_);
    convolution_by_dft_(b);
    postprocess(b.first -> begin(), &out(0, 0), sizes_);
}


// reentrant version
void Fftw_convolution_2d::convolution_by_dft_(Work_buffer b) const
{
    if (! is_mask_set_)
    {
        KJB_THROW_2(Runtime_error, MASK_NAG);
    }
    data_plan_ -> new_array_exec(b.first -> begin(), b.second -> begin());
    const FFTW_complex_vector& mask_fft = * mask_.second.get();
    ow_multiply_fftw_(b.second.get(), mask_fft, sizes_.Ncomplex());
    data_inv_plan_ -> new_array_exec(b.second -> begin(), b.first -> begin());
}


// reentrant version
void Fftw_convolution_2d::reflect_and_convolve(
    const Matrix& in,
    Matrix& out,
    Work_buffer b
)   const
{
    prep(in, out, sizes_);
    reflect_into_input_buf(in, b.first -> begin(), sizes_);
    convolution_by_dft_(b);
    postprocess(b.first -> begin(), &out(0, 0), sizes_);
}


#else
void Fftw_convolution_2d::set_mask(const Matrix&) {}
void Fftw_convolution_2d::set_gaussian_mask(double) {}
void Fftw_convolution_2d::convolve(const Matrix&, Matrix&) const {}
void Fftw_convolution_2d::reflect_and_convolve(const Matrix&, Matrix&) const {}

void Fftw_convolution_2d::convolve(const Matrix&, Matrix&, Work_buffer) const
{}

void Fftw_convolution_2d::reflect_and_convolve(
    const Matrix&,
    Matrix&,
    Work_buffer 
)   const
{}
#endif




/**
 * @brief get a handle to a work buffer needed for threadsafe convolution.
 * @return a Work_buffer (i.e., smart pointers) to memory for convolution
 * @warning not thread safe
 * @warning do not mix work buffers among different objects of this class,
 *           unless the results of get_sizes() are identical.
 * @post data_ contains two null smart pointers.
 *
 * If you call this function, you are
 * probably going to use the reentrant convolution methods, and therefore you
 * will probably not need the memory in the data_ field, if any.  Thus we
 * check whether data_ has memory, and if so we give it away in the expectation
 * (no guarantees) that it would probably never otherwise be used.
 *
 * To repeat:  the object this returns is a handle, an opaque pointer --
 * when originally written, a pair of smart pointers but we make no promises
 * about implementation -- and you should pass it around BY VALUE, via copying.
 * It is fine to store them in an array or vector, if you like.
 *
 * @warning It is also not thread-safe to let the last copy of a Work_buffer
 *          object go out of scope.  See @ref fftw_wb_dtion for discussion.
 */
Fftw_convolution_2d::Work_buffer
Fftw_convolution_2d::allocate_work_buffer() const
{
#ifdef KJB_HAVE_FFTW
    Work_buffer b = data_;
    if (data_.first)
    {
        data_ = Work_buffer();      // Store equiv. of (NULL,NULL) in data_.
    }
    else
    {
        b = alloc_work_buf_impl_();
    }
    KJB(ASSERT(!data_.first));
    KJB(ASSERT(b.first));
    return b;
#else
    return Work_buffer();
#endif
}



namespace debug
{

int test_reflect_into_input_buf(
    const Matrix& in,
    Matrix* out,
    const Fftw_convolution_2d::Sizes& s
)
{
#ifdef KJB_HAVE_FFTW
    NTX(out);
    out -> resize(s.pad_rows, s.pad_cols);
    reflect_into_input_buf(in, & out -> at(0), s);
    return kjb_c::NO_ERROR;
#else
    kjb_c::set_error("Code was built without libfftw");
    return kjb_c::ERROR;
#endif
}


} // namespace kjb::debug



int x_convolve_matrix(
    Matrix* output,
    const Matrix& input,
    const Vector& mask)
{
    KJB(NRE(output));
    kjb_c::Matrix *p = 00;
    KJB(ERE(kjb_c::x_convolve_matrix(&p,
                 input.get_c_matrix(), mask.get_c_vector())));
    Matrix q(p);
    output -> swap(q);
    return kjb_c::NO_ERROR;
}



} // namespace kjb



