/**
 * @file
 * @brief declaration of C++ helper class for FFT-based convolution using FFTW
 * @author Kyle Simek
 * @author Prasad Gabur
 * @author Kobus Barnard
 * @author Andrew Predoehl
 */

/* $Id: m_convolve.h 15035 2013-07-29 23:32:59Z predoehl $ */

/* {{{======================================================================= *
   |
   |  Copyright (c) 1994-2013 by members of the Interdisciplinary Visual
   |  Intelligence Laboratory.
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact kobus AT sista DOT arizona DOT edu.
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or
   |  fitness for any particular task. Nonetheless, I am interested in hearing
   |  about problems that you encounter.
   |
 * ====================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_M_CPP_M_CONVOLVE_H
#define KJB_CPP_M_CPP_M_CONVOLVE_H

#include "m_cpp/m_matrix.h"

#ifdef KJB_HAVE_FFTW /* Include the header, if we can. */

/*
 * Their header has a built-in extern-C gadget, so we do not need one here.
 * However, we would like to wrap their symbols in a namespace.
 */
namespace FFTW {
#include <fftw3.h>
}

#else /* KJB_HAVE_FFTW not defined */
#warning "This code requires FFTW version 3 to work properly."
const int FFTW_ESTIMATE = 0;
const int FFTW_MEASURE = 0;
const int FFTW_PATIENT = 0;
const int FFTW_EXHAUSTIVE = 0;
const int FFTW_DESTROY_INPUT = 0;
#endif /* KJB_HAVE_FFTW */


#include <utility>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


/**
 * @file Convolution-related classes and functions
 */

namespace kjb
{

#ifdef KJB_HAVE_FFTW /* This block defines a speciality vector for FFTW */


// We don't need the end pointer currently, but future changes might demand it.
#define KJB_FFTW_VECTOR_NEEDS_END 0


/**
 * @brief RAII class to allocate memory using FFTW-preferred alignment
 *
 * The ctor and dtor are not reentrant.  To quote the FFTW docs, "The upshot
 * is that the only thread-safe (re-entrant) routine in FFTW is fftw_execute
 * (and the new-array variants thereof).  All other routines (e.g., the
 * planner) should only be called from one thread at a time."
 *
 * My interpretation:  fftw_malloc and fftw_free are NOT reentrant.
 *
 * To keep the code simple, copying and assignment are disallowed.
 */
template <typename T>
class FFTW_vector
{
    T* m_begin;
#if KJB_FFTW_VECTOR_NEEDS_END
    T* m_end;
#endif
    FFTW_vector<T>(const FFTW_vector<T>&); // teaser
    FFTW_vector<T>& operator=(const FFTW_vector<T>&); // teaser

public:
    FFTW_vector(size_t n = 0)
    :   m_begin(0)
    {
        if (n && 0 == (m_begin = (T*) FFTW::fftw_malloc(sizeof(T) * n)))
        {
            KJB_THROW(Resource_exhaustion);
        }
#if KJB_FFTW_VECTOR_NEEDS_END
        m_end = m_begin + n;
#endif
    }

    ~FFTW_vector()
    {
        if (m_begin) FFTW::fftw_free(m_begin);
    }

    T* begin()
    {
        return m_begin;
    }

    const T* begin() const
    {
        return m_begin;
    }

#if KJB_FFTW_VECTOR_NEEDS_END
    T* end()
    {
        return m_end;
    }

    const T* end() const
    {
        return m_end;
    }
#endif

    void swap(FFTW_vector<T>& v)
    {
        using std::swap;
        swap(m_begin, v.m_begin);
#if KJB_FFTW_VECTOR_NEEDS_END
        swap(m_end, v.m_end);
#endif
    }
};


/// @brief typedef for the usual output of an FFT
typedef FFTW_vector<FFTW::fftw_complex> FFTW_complex_vector;

/// @brief typedef for the usual input of a real FFT
typedef FFTW_vector<double> FFTW_real_vector;



#else /* what to do if FFTW library is not available? */
typedef void* FFTW_complex_vector;
typedef void* FFTW_real_vector;
#endif






/**
 * @brief RAII class to manage an FFTW plan.
 *
 * @warning The ctor and dtor are not reentrant.
 *
 * @see Fftw_convolution_2d -- This is just a helper class for
 *      Fftw_convolution_2d.  I cannot think of a reason anyone would
 *      want to instantiate one of these things independently.
 */
template <class T, class U> class FFTW_Plan2d {};



#ifdef KJB_HAVE_FFTW /* This block defines FFTW plan wrappers. */

// "forward" FFT plan management
template<>
class FFTW_Plan2d<double, FFTW::fftw_complex>
{
    FFTW::fftw_plan plan;

    FFTW_Plan2d<double, FFTW::fftw_complex>(
            const FFTW_Plan2d<double, FFTW::fftw_complex>&); // teaser
    FFTW_Plan2d<double, FFTW::fftw_complex>& operator=(
            const FFTW_Plan2d<double, FFTW::fftw_complex>&); // teaser
public:
    FFTW_Plan2d<double, FFTW::fftw_complex>(
        int n0,
        int n1,
        double* in,
        FFTW::fftw_complex* out,
        unsigned flags
    )
    :   plan(FFTW::fftw_plan_dft_r2c_2d(n0, n1, in, out, flags))
    {
        if (0 == plan) KJB_THROW(Resource_exhaustion);
    }

    ~FFTW_Plan2d<double, FFTW::fftw_complex>()
    {
        FFTW::fftw_destroy_plan(plan);
    }

    void execute() const
    {
        FFTW::fftw_execute(plan);
    }

    void new_array_exec(double *in, FFTW::fftw_complex *out) const
    {
        FFTW::fftw_execute_dft_r2c(plan, in, out);
    }
};


// inverse-FFT plan management
template<>
class FFTW_Plan2d<FFTW::fftw_complex, double>
{
    FFTW::fftw_plan plan;

    FFTW_Plan2d<FFTW::fftw_complex, double>(
            const FFTW_Plan2d<FFTW::fftw_complex, double>&); // teaser
    FFTW_Plan2d<FFTW::fftw_complex, double>& operator=(
            const FFTW_Plan2d<FFTW::fftw_complex, double>&); // teaser
public:
    FFTW_Plan2d<FFTW::fftw_complex, double>(
        int n0,
        int n1,
        FFTW::fftw_complex* in,
        double* out,
        unsigned flags
    )
    :   plan(FFTW::fftw_plan_dft_c2r_2d(n0, n1, in, out, flags))
    {
        if (0 == plan) KJB_THROW(Resource_exhaustion);
    }

    ~FFTW_Plan2d<FFTW::fftw_complex, double>()
    {
        FFTW::fftw_destroy_plan(plan);
    }

    void execute() const
    {
        FFTW::fftw_execute(plan);
    }

    void new_array_exec(FFTW::fftw_complex *in, double* out) const
    {
        FFTW::fftw_execute_dft_c2r(plan, in, out);
    }
};


/// @brief convenience abbreviation for the type of a forward real FFT
typedef FFTW_Plan2d<double, FFTW::fftw_complex> FFTW_plan_r2c;

/// @brief convenience abbreviation for the type of a reverse real FFT
typedef FFTW_Plan2d<FFTW::fftw_complex, double> FFTW_plan_c2r;

#endif






/**
 * @brief A class for performing 2d convolution using the FFTW library.
 *
 * This should run faster than
 * <a href="http://vision.sista.arizona.edu/kobus/research/resources/doc/kjb/fourier_convolve_matrix.html">kjb_c::fourier_convolve_matrix()</a>
 * for applications that perform similar-sized convolutions many times, because
 * this class re-uses the plans constructed by FFTW, the construction of which
 * is usually the bottleneck for FFTW.  The results from the convolve() method
 * should be the same as those of fourier_convolve_matrix() except for
 * numerical noise.
 *
 * @section fft_boundary_convo Two ways to handle boundary conditions
 * If you use the convolve() method, boundary conditions are handled by padding
 * the data and mask with zeros before operating on them.  This padding is
 * removed before a result is returned.
 * Note that
 * <a href="http://vision.sista.arizona.edu/kobus/research/resources/doc/kjb/convolve_matrix.html">kjb_c::convolve_matrix()</a>
 * behaves differently (it reflects the input matrix at its boundary).
 * To emulate this behavior, use the reflect_and_convolve() method.
 *
 * Storage is allocated only once when the object is created.  Because of this,
 * all data used for convolutions must be of the same size.  Masks may differ
 * in size, but dimensions cannot exceed the maximums specified during
 * construction.  Setting maximum mask dimensions higher than needed won't
 * affect correctness, but will reduce performance.  Setting the maximum mask
 * dimensions too low will raise an exception when a larger mask is used.
 *
 * @section fft_unithread_convo Convolution using just one thread
 * If your application does not need to be parallelized, you may simply
 * disregard any methods that mention Work_buffer.  In other words,
 * use only the two-parameter convolve() and reflect_and_convolve() methods.
 *
 * @section fft_mt_convo Convolution using multiple threads
 * A few methods are reentrant (i.e., thread-safe).  If you wish to use
 * multiple threads to convolve a common mask with multiple matrices (each
 * convolution using ONE thread), this class can accomodate.  Here is the
 * general strategy:
 * - One thread instantiates class in object o and (usually) sets the mask
 * - Start some worker threads, and give each a pointer p to object o.
 * - Each worker thread needs its own Fftw_convolution_2d::Work_buffer b from
 *   allocate_work_buffer(), but this method is NOT reentrant!
 *   Use a mutex to force sequential calls, or
 *   have one starter thread perform all the allocation sequentially and
 *   distribute the buffers to the worker threads.
 * - Worker threads may use (only) the three-parameter convolve() method or
 *   reflect_and_convolve() method.  The third parameter is a Work_buffer, and
 *   obviously each worker thread should only use its own personal buffer for
 *   the appropriate class.
 * - Trivial methods get_sizes() and is_mask_set() are also reentrant.  No
 *   other methods are reentrant.  Also, the class FFTW_vector<T> is not
 *   reentrant.
 *
 * @section fftw_mt_advice Additional advice for multithreaded programs:
 * - A Work_buffer object is lightweight and can be copied by value, because
 *   it is just a couple of smart pointers,
 * - Annoyingly, it is NOT thread-safe for the last copy of a Work_buffer to be
 *   destroyed!  You have to serialize the destruction of the last copy (i.e.,
 *   serialize the deallocation of the memory).  See @ref fftw_wb_dtion below.
 * - If you instantiate multiple Fftw_convolution_2d objects (presumably with
 *   different masks) you should not share Work_buffer objects between them
 *   unless all four sizes given to the ctor are identical.
 * - Once you call allocate_work_buffer(), it is best not to call either of
 *   the thread-unsafe, two-parameter convolve methods afterwards, or you will
 *   incur a substantial time penalty.  The reason is that when calling
 *   allocate_work_buffer(), the object takes this as a hint that you will
 *   probably never need the thread-unsafe calls for the rest of the lifetime
 *   of the object. So, it frees some of its internal resources required for
 *   the thread-unsafe calls.
 *
 * @section fftw_wb_dtion Destroying a Work_buffer
 *   Because the Work_buffer is a shared pointer to a block of memory that
 *   must be deallocated sequentially (using fftw_free, which is not
 *   reentrant), the last copy of any Work_buffer cannot be destroyed
 *   simultaneously with any other FFTW calls
 *   (except for its thread-safe functions).
 *   Thus, if a thread-safe function is to call allocate_work_buffer or
 *   initiate the destruction of the work buffer, it must serialize those
 *   steps.
 *   There are a number of ways to do so.
 * - See @ref fftw_eg for one straightforward way to serialize those actions.
 * - Simpler alternative:  leave it to the main thread to allocate the buffers,
 *   to launch the threads, and to destroy all the buffers after the join.
 *   Advantage:  simplicity, and no locks.  Disadvantage:  none of the memory
 *   is released until all the convolution is over.
 *
 * @section fftw_eg Example multithreaded code
 * The following code snippet shows the general outline of a thread function
 * that performs convolution, returning NULL or non-NULL to indicate error or
 * success.  Also it accesses two global objects, c and mtx.
 *
 * @code
 * kjb_pthread_mutex mtx = KJB_PTHREAD_MUTEX_INITIALIZER;
 * Fftw_convolution_2d* c = NULL; // setup of c done elsewhere, e.g., in main()
 *
 * // Here is the function that the worker threads all call:
 * void* thread_work(void* input) {
 *     NRN(c); NRN(input);
 *     Fftw_convolution_2d::Work_buffer wb; // ok: default ctor is thread-safe
 *
 *     // Allocate work buffer, using 'mtx' to prevent races between threads.
 *     do { Mutex_lock l(&mtx); wb = c -> allocate_work_buffer(); } while(0);
 *
 *     // Now, do convolution on input using c and wb.
 *     // . . .
 *
 *     // Ready to exit.  Now, destroy any COPIES of wb asynchronously.
 *     // . . .
 *
 *     if (!work_buffer_is_unique(wb)) {
 *         set_error("wb not unique in thread %d", get_kjb_pthread_number());
 *         return NULL; // cannot destroy work buffer if another copy exists
 *     }
 *
 *     // Destroy the last copy synchronously.
 *     Mutex_lock l(&mtx);
 *     wb = Fftw_convolution_2d::Work_buffer(); // clobber its contents
 *     return input;
 * }
 * @endcode
 *
 * @ingroup kjbThreads
 *
 * @author Kyle Simek
 * @author Prasad Gabur
 * @author Kobus Barnard
 * @author Andrew Predoehl
 */
class Fftw_convolution_2d
{
public:
    typedef std::pair< boost::shared_ptr<FFTW_real_vector>,
                       boost::shared_ptr<FFTW_complex_vector> > Work_buffer;

    /// @brief utility aggregate stores all sizes -- rarely used by caller
    struct Sizes
    {
        int data_rows, data_cols, mask_rows, mask_cols, pad_rows, pad_cols;
        Sizes(int, int, int, int);

        bool is_matrix_size_same_as_data_size(const Matrix& m) const
        {
            return m.get_num_rows()==data_rows && m.get_num_cols()==data_cols;
        }
        bool is_matrix_size_within_mask_size(const Matrix& m) const
        {
            return m.get_num_rows()<=mask_rows && m.get_num_cols()<=mask_cols;
        }
        int Nreal() const
        {
            return pad_rows * pad_cols;
        }
        int Ncomplex() const
        {
            return pad_rows * (pad_cols/2 + 1);
        }
    };


    Fftw_convolution_2d(
        int data_num_rows,
        int data_num_cols,
        int mask_max_rows,
        int mask_max_cols,
        int fft_alg_type = FFTW_MEASURE
    );

    /// @brief set mask, which must fit with mask size maxima given to ctor
    void set_mask(const Matrix&);

    /// @brief set mask to a circular gaussian kernel of given sigma (pixels)
    void set_gaussian_mask(double sigma);

    void convolve(const Matrix&, Matrix&) const;

    /// @brief convolve with mask, assuming input reflects at its boundaries
    void reflect_and_convolve(const Matrix&, Matrix&) const;

    /// @brief deprecated synonym for convolve method
    void execute(const Matrix& i, Matrix& o) const { convolve(i, o); }

    // not thread safe
    Work_buffer allocate_work_buffer() const;


/*  ABOVE THE LINE:  NOT THREAD SAFE
 * -----------------------------------------------------------------
 *  BELOW THE LINE:  THREAD SAFE
 */

    void convolve(const Matrix&, Matrix&, Work_buffer) const;

    /// @brief convolve with mask, assuming input reflects at its boundaries
    void reflect_and_convolve(const Matrix&, Matrix&, Work_buffer) const;

    /// @brief read access to the sizes specified at ctor time
    const Sizes& get_sizes() const
    {
        return sizes_;
    }

    /// @brief read access of the flag indicating whether the mask has been set
    bool is_mask_set() const
    {
        return is_mask_set_;
    }

private:

#ifdef KJB_HAVE_FFTW
    Work_buffer alloc_work_buf_impl_() const;

    void reset_data_plan_() const;

    void convolution_by_dft_() const;

    void convolution_by_dft_(Work_buffer) const;
#endif


    // because this field is const, all its members are automatically const.
    const Sizes sizes_;

    const int fft_algorithm_type_;

    bool is_mask_set_;

    Work_buffer mask_;
    mutable Work_buffer data_;


#ifdef KJB_HAVE_FFTW             /* Define plan members in the real thing. */
    boost::scoped_ptr<FFTW_plan_r2c> mask_plan_;
    mutable boost::scoped_ptr<FFTW_plan_r2c> data_plan_;
    mutable boost::scoped_ptr<FFTW_plan_c2r> data_inv_plan_;
#endif

};


/**
 * @brief test whether a Work_buffer object is the last handle to its memory
 *
 * This is a utility function useful for reasons described
 * in @ref fftw_wb_dtion
 *
 * @ingroup kjbThreads
 */
inline bool work_buffer_is_unique(const Fftw_convolution_2d::Work_buffer& b)
{
    return b.first.unique() && b.second.unique();
}


namespace debug
{

/// @brief test function provides transparency to hidden reflect_into_input_buf
int test_reflect_into_input_buf(
    const Matrix&,
    Matrix*,
    const Fftw_convolution_2d::Sizes&
);

} // namespace kjb::debug


/// @brief convolve by sliding across columns of input matrix, via mask.
/// I.e., inter-column differences are smoothed, row-to-row diffs untouched.
int x_convolve_matrix(Matrix*, const Matrix&, const Vector&);


} // namespace kjb

#endif

