/* $Id: edge_chamfer_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */


/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== */


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "likelihood_cpp/edge_chamfer_likelihood.h"
#include "m_cpp/m_arith.h"
#include "gr_cpp/gr_fbo_offscreen.h"

using namespace kjb;

#ifdef KJB_HAVE_GLEW
#ifdef KJB_HAVE_CUDA
#ifdef KJB_HAVE_OPENGL

CUarray Base_gpu_chamfer_likelihood::map_rbo()
{
    CUarray result_array = 0;

    // pre-mapping.  not sure why this isn't part of getMappedPointer call below.
    CU_ETX(cuGraphicsMapResources(1, &cuda_rbo_handle_, 0));

    CU_ETX(cuGraphicsSubResourceGetMappedArray(&result_array, cuda_rbo_handle_, 0, 0));

    ASSERT(result_array);

    return result_array;

}



/*  -------------------------------------------------------------------------- */
/*  -------------------------------------------------------------------------- */
/*  -------------------------------------------------------------------------- */

//#ifdef KJB_HAVE_CUDPP
#if 0

Gpu_chamfer_likelihood::Gpu_chamfer_likelihood(int width, int height) :
        Base_gpu_chamfer_likelihood(width, height),
        util_mod_(),
        cudpp_handle_(),
        distance_map_(0),
        position_map_(0),
        buffer_ptr_(0),
        workspace_ui_(0),
        buffer_pitch_bytes_(0),
        buffer_pitch_elements_(0),
        N_(0)
{
    size_t uint_size = sizeof(unsigned int);

    CUDPPResult result = cudppCreate(&cudpp_handle_);

    if(result != CUDPP_SUCCESS)
        KJB_THROW_2(kjb::Runtime_error, "Failed to initialize cudpp library.");

    CU_ETX(cuMemAllocPitch(
            &workspace_ui_,
            &buffer_pitch_bytes_,
            width * uint_size,
            height,
            uint_size));

    // convert from bytes to elements
    buffer_pitch_elements_ = buffer_pitch_bytes_ / uint_size;
    N_ = buffer_pitch_elements_ * height;

    // the buffer array must be big enough to store a matrix
    // of floats and a matrix of unsigned ints, so take the max of the two
    // (the need for a second buffer is avoided if we add a dedicated reduce function, which would clean this up a lot)
#if 1 
    size_t buffer_element_size = MAX(sizeof(float), uint_size);

    Cuda_size_t junk_pitch; // don't care, just need a placeholder variable

    CU_ETX(cuMemAllocPitch(
            &buffer_ptr_,
            &junk_pitch,
            width * buffer_element_size,
            height,
            buffer_element_size));

#else
    CU_ETX(cuMemAllocPitch(
            &buffer_ptr_,
            &buffer_pitch_bytes_,
            width * sizeof(float),
            height,
            sizeof(float)));
    N_ = buffer_pitch_bytes_ * height / sizeof(float);
    buffer_pitch_elements_ = buffer_pitch_bytes_ / sizeof(float);
#endif


    // this is important to set the pitch gutter to zero
    // so it isn't included in the reduction sum
    ASSERT(sizeof(float) * 8 == 32);
    cuMemsetD32(buffer_ptr_, 0, N_);

}


void Gpu_chamfer_likelihood::set_maps(const CUdeviceptr distance, const CUdeviceptr position, size_t num_edges, size_t N)
{
    if(N > 0 && N != N_)
    {
        KJB_THROW_2(Illegal_argument, "Distance and position arrays must have same size as the display buffer.");
    }

    distance_map_ = distance;
    position_map_ = position;
    data_points_ = num_edges;
}


CUdeviceptr Gpu_chamfer_likelihood::create_distance_map(const Matrix& distance_map)
{

    if(distance_map.get_num_cols() != offscreen_buffer_.get_width() ||
       distance_map.get_num_rows() != offscreen_buffer_.get_height() )
    {
        KJB_THROW_2(Illegal_argument, "Distance_map dimensions doesn't match the color buffer dimensions.");
    }

#if CUDA_VERSION < 3020
    unsigned int pitch;
#else
    size_t pitch;
#endif

    CUdeviceptr ptr = gpu::create_cuda_pitch<Matrix, float>(distance_map, pitch, true);

    size_t N = distance_map.get_num_rows() * pitch / sizeof(float);

    // this should always be the same, since both the buffer
    // and the distance map have the same width,height dimensions,
    // and were both allocated using cuMemAllocPitch, so the 
    // pitches are the same.  Nevertheless, sanity check it.
    ASSERT(N == N_);

    return ptr;
}



CUdeviceptr Gpu_chamfer_likelihood::create_position_map(const Int_matrix& row_position_map, const Int_matrix& col_position_map)
{
    ASSERT(row_position_map.get_num_rows() == col_position_map.get_num_rows());
    ASSERT(row_position_map.get_num_cols() == col_position_map.get_num_cols());

    const size_t num_rows = row_position_map.get_num_rows();
    const size_t num_cols = col_position_map.get_num_cols();

    if(num_cols != (size_t) offscreen_buffer_.get_width() ||
       num_rows != (size_t) offscreen_buffer_.get_height())
    {
        KJB_THROW_2(Illegal_argument, "Position_map dimensions doesn't match the color buffer dimensions.");
    }

    Int_matrix positions(num_rows, num_cols);

    for(size_t row = 0; row < num_rows; row++)
    for(size_t col = 0; col < num_cols; col++)
    {
        int d_row = row_position_map(row, col);
        int d_col = col_position_map(row, col);

        positions(row, col) = d_row * num_cols + d_col;
    }

#if CUDA_VERSION < 3020
    unsigned int pitch;
#else
    size_t pitch;
#endif

    CUdeviceptr ptr = gpu::create_cuda_pitch<Int_matrix, unsigned int>(positions, pitch, true);

    size_t N = num_rows * pitch / sizeof(unsigned int);

    // this should always be the same, since both the buffer
    // and the distance map have the same width,height dimensions,
    // and were both allocated using cuMemAllocPitch, so the 
    // pitches are the same.  Nevertheless, sanity check it.
    ASSERT(N == N_);

    return ptr;

}


void Gpu_chamfer_likelihood::operator()(const Renderable& r) 
{

    // TODO these should probably be error checks
    ASSERT(distance_map_);
    ASSERT(buffer_ptr_);

// 1. RENDER TO A LINEAR ARRAY
    offscreen_buffer_.render(r);

    // make color buffer Cuda-accessible
    CUarray render_buffer = map_rbo();


    // Copy 2D texture memory to a 1D vector
    CUdeviceptr linear_buffer = array_to_linear(render_buffer);
    // (no need to free; it actually points to a member that will be free'd on destruction)

    // release rbo from cuda
    unmap_rbo();


// 2. get sum of distances and count the model points
    // add up all 1's in buffer.  
    model_points_ = reduce_mod_.reduce<float>(linear_buffer, N_);

    // element-wise multiply render buffer and the distance_map (already in cuda memory)
    sum_ = reduce_mod_.chamfer_reduce(distance_map_, linear_buffer, N_, square_values_);

// 3. count correspondences
    // copy position matrix to a scratch buffer
    CU_ETX(cuMemcpyDtoD(workspace_ui_, position_map_, N_ * sizeof(unsigned int)));

    // use color buffer as mask so positions only remain where edges exist
    util_mod_.ow_ew_multiply<unsigned int, float>(workspace_ui_, linear_buffer, (unsigned int) N_);

    // sort to group positions together
    uint_cuda_sort_(workspace_ui_);

    // convert discontinuities to '1'; others '0'
    util_mod_.detect_changes<unsigned int>(workspace_ui_, (unsigned int) N_);

    // reduce to get sum
//        bool kjb_reduce = true;
//        if(kjb_reduce)
//            correspondences_ reduce_mod.reduce_uint(positions, N_);
//        else
    {
        // re-use the buffer as an unsigned int scratch space
        CUdeviceptr workspace_ui_2 = linear_buffer;
        correspondences_ = uint_reduce_(workspace_ui_, workspace_ui_2); // use cudpp
    }

}


void Gpu_chamfer_likelihood::gold_standard_process(const Renderable& r, Matrix distance, const Int_matrix& row_position, const Int_matrix& col_position, size_t num_points) 
{

    offscreen_buffer_.render(r);

    const size_t width = offscreen_buffer_.get_width();
    const size_t height = offscreen_buffer_.get_height();

    offscreen_buffer_.bind();
    // Copy 2D texture memory to a 1D vector
    size_t num_pixels = width * height;
    float* buffer = new float[width * height];
    glReadPixels(0,0,width, height, GL_RED, GL_FLOAT, buffer);
    offscreen_buffer_.unbind();

    Matrix masked_positions(height, width, 0.0);

    sum_ = 0;
    model_points_= 0;
    data_points_ = num_points;
    correspondences_ = 0;

    int i = 0;


    for(int row = height - 1; row >= 0; row--)
    for(size_t col = 0; col < width; col++, i++)
    {
        distance(row, col) *= buffer[i];

        if(buffer[i] > 0.0)
        {
            // binary scene
            ASSERT(buffer[i] == 1.0);

            model_points_++;


//                if(square_values_)
//                {
//                    double dist = distance(row, col);
//                    sum += dist * dist;
//                }
//                else
//                    sum += distance(row, col);

            int mask_row = (int) row_position(row, col);
            int mask_col = (int) col_position(row, col);

            masked_positions(mask_row, mask_col) = 1.0;
        }
    }

    if(square_values_)
    {
        for(size_t i = 0; i < num_pixels; i++)
        {
            distance[i] = distance[i] * distance[i];
        }
    }

    // sum up distances (use divide-and-conquor to avoid precision loss
    size_t length = num_pixels;
    while(length > 1)
    {
        size_t half_length = (length+1) / 2;

        for(size_t i = 0; i < half_length; i++)
        {
            if(i + half_length < length)
                distance[i] += distance[i + half_length];
        }

        length = half_length;
    }

    sum_ = distance[0];
    

    for(size_t i = 0; i < num_pixels; i++)
    {
        if(masked_positions[i] > 0.0)
        {
            ASSERT(masked_positions[i] == 1.0);
            correspondences_++;
        }
    }

//        sum_ = sum;

    delete[] buffer;

    GL_ETX();
}



#ifdef TEST
Matrix Gpu_chamfer_likelihood::get_buffer() const
{
    const size_t width = offscreen_buffer_.get_width();
    const size_t height = offscreen_buffer_.get_height();

    offscreen_buffer_.bind();
    // Copy 2D texture memory to a 1D vector
//        size_t num_pixels = width_ * height_;
    float* buffer = new float[width * height];
    glReadPixels(0,0,width, height, GL_RED, GL_FLOAT, buffer);
    offscreen_buffer_.unbind();

    Matrix result(height, width);

    int i = 0;

    for(int row = height-1; row >= 0; row--)
    for(size_t col = 0; col < width; col++, i++)
    {
        result(row, col) = buffer[i]*255;
    }

    delete[] buffer;

    return result;

}


void Gpu_chamfer_likelihood::display_buffer(const std::string& str) const
{

    ::kjb::opengl::gl_display(get_buffer(), str);
    
}
#endif /* TEST */



/// Receives a cuda pointer and returns a linear cuda array
CUdeviceptr Gpu_chamfer_likelihood::array_to_linear(CUarray array)
{
    const size_t width = offscreen_buffer_.get_width();
    const size_t height = offscreen_buffer_.get_height();

    // this is important to set the pitch gutter to zero
    // so it isn't included in the reduction sum
    ASSERT(sizeof(float) * 8 == 32);
    CU_ETX(cuMemsetD32(buffer_ptr_, 0, N_));

    // (we could avoid this memset if we didn't let the buffer double as a scratch space.  try commenting this out and see if performance changes.  if so, consider adding a second scratch space)
    // Also, we wouldn't even need a second workspace if we had a dedicated reduce kernel for unsigned ints

    CUDA_MEMCPY2D cpy_meta;
    cpy_meta.srcMemoryType = CU_MEMORYTYPE_ARRAY;
    cpy_meta.srcXInBytes = 0;
    cpy_meta.srcY = 0;
    cpy_meta.srcHost = 0;
    cpy_meta.srcDevice = 0;
    cpy_meta.srcArray = array;
    cpy_meta.srcPitch = 0;
    cpy_meta.dstXInBytes = 0;
    cpy_meta.dstY = 0;
    cpy_meta.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    cpy_meta.dstHost = 0;
    cpy_meta.dstDevice = buffer_ptr_;
    cpy_meta.dstArray = 0;
    cpy_meta.dstPitch = buffer_pitch_bytes_;
    cpy_meta.WidthInBytes = width * sizeof(float);
    cpy_meta.Height = height;
    
    CU_ETX(cuMemcpy2D(&cpy_meta));

    return buffer_ptr_;
}


void Gpu_chamfer_likelihood::uint_cuda_sort_(CUdeviceptr uint_array)
{
    static CUDPPHandle plan;
    static bool plan_set = false;

    if(!plan_set)
    {
        CUDPPConfiguration config;
        config.algorithm = CUDPP_SORT_RADIX;
        config.op = CUDPP_ADD; // don't care
        config.datatype = CUDPP_UINT;
        config.options = CUDPP_OPTION_KEYS_ONLY;

        CUDPP_ETX(cudppPlan(cudpp_handle_, &plan, config, N_, 1, N_));
        plan_set = true;
    }

    CUDPP_ETX(cudppSort(plan, (void*) uint_array, NULL, N_));
}


double Gpu_chamfer_likelihood::uint_reduce_(CUdeviceptr uint_array, CUdeviceptr workspace)
{
    static CUDPPHandle plan;
    static bool plan_set = false;

    CUdeviceptr d_out = workspace;
    if(!plan_set)
    {
        CUDPPConfiguration config;
        config.algorithm = CUDPP_SCAN;
        config.op = CUDPP_ADD; // don't care
        config.datatype = CUDPP_UINT;
        config.options = CUDPP_OPTION_BACKWARD | CUDPP_OPTION_INCLUSIVE;

        cudppPlan(cudpp_handle_, &plan, config, N_, 1, N_);
        plan_set = true;
    }

    CUDPP_ETX(cudppScan(plan, (void*) d_out, (void*) uint_array, N_));

    unsigned int out;
    cuMemcpyDtoH(&out, workspace, sizeof(unsigned int));

    return out;
}


/*  -------------------------------------------------------------------------- */
/*  -------------------------------------------------------------------------- */
/*  -------------------------------------------------------------------------- */

void Multi_gpu_chamfer_likelihood::push_back(const Chamfer_transform& xfm)
{

    const Matrix& distance = xfm.distance_map();
    const std::vector<Int_matrix> positions =  xfm.position_map();
    size_t num_edges = xfm.get_num_points();

    push_back(distance, positions[0], positions[1], num_edges);
}

void Multi_gpu_chamfer_likelihood::push_back(
        const Matrix& distance_map,
        const Int_matrix& row_position_map,
        const Int_matrix& col_position_map,
        size_t num_points)
{
    CUdeviceptr distance_vector = likelihood_.create_distance_map(distance_map);
    CUdeviceptr position_vector = likelihood_.create_position_map(row_position_map, col_position_map);


    distance_maps_.push_back(distance_vector);
    position_maps_.push_back(position_vector);
    data_point_sizes_.push_back(num_points);
}


void Multi_gpu_chamfer_likelihood::operator()(const Mv_renderable& m) 
{
    // if modulo_ > 1, not all views are used.
    
//        size_t num_usable_views;
    std::vector<size_t> view_list = get_view_list(m.num_views());

    ASSERT(view_list.size() == distance_maps_.size());

    sum_ = 0;
    model_points_ = 0;
    correspondences_ = 0;
    data_points_ = 0;

    for(size_t i = 0; i < view_list.size(); i++)
    {
        size_t view_index = view_list[i];

        likelihood_.set_maps(
                distance_maps_[i],
                position_maps_[i],
                data_point_sizes_[i]
                );

        m.set_active_view(view_index);
        likelihood_(m);

        if(square_values_)
            sum_ += likelihood_.get_sq_sum();
        else
            sum_ += likelihood_.get_sum();

        model_points_ += likelihood_.get_num_model_points();
        correspondences_ += likelihood_.get_num_correspondences();
        data_points_ += likelihood_.get_num_data_points();
    }
}


std::vector<size_t> Multi_gpu_chamfer_likelihood::get_view_list(const size_t num_views)
{
    KJB(UNTESTED_CODE());

    if(view_list_.size() > 0)
    {
        return view_list_;
    }
    else
    {
        std::vector<size_t> view_list;
        ASSERT(modulo_ > 0);
        size_t num_usable_views = (num_views + modulo_ - 1) / modulo_;

        view_list.reserve(num_usable_views);
        for(size_t i = 0; i < num_views; i+=modulo_)
        {
            view_list.push_back(i);
        }

        return view_list;
    }

}


void Multi_gpu_chamfer_likelihood::gold_standard_evaluate(
    const Mv_renderable& m,
    std::vector<Matrix> distances,
    std::vector<Int_matrix> row_positions,
    std::vector<Int_matrix> col_positions,
    std::vector<size_t> num_points
)
{

    sum_ = 0;
    model_points_= 0;
    data_points_ = 0;
    correspondences_ = 0;

    std::vector<size_t> view_list = get_view_list(m.num_views());
    ASSERT(view_list.size() == distances.size());

    for(size_t i = 0; i < view_list.size(); i++)
    {
        size_t view_index = view_list[i];

        Matrix& distance = distances.at(i);
        Int_matrix& row_position = row_positions.at(i);
        Int_matrix& col_position = col_positions.at(i);
        size_t count = num_points.at(i);

        m.set_active_view(view_index);

        likelihood_.gold_standard_process(m, distance, row_position, col_position, count);

        if(square_values_)
            sum_ += likelihood_.get_sq_sum();
        else
            sum_ += likelihood_.get_sum();

        model_points_ += likelihood_.get_num_model_points();
        correspondences_ += likelihood_.get_num_correspondences();
        data_points_ += likelihood_.get_num_data_points();

    }

}


Multi_gpu_chamfer_likelihood::~Multi_gpu_chamfer_likelihood()
{
    for(size_t i = 0; i < distance_maps_.size(); i++)
    {
        // CU_ETX deliberately omitted.
        // Didn't your mother tell you not to throw in destructors?
        cuMemFree(distance_maps_[i]);
    }

    for(size_t i = 0; i < position_maps_.size(); i++)
    {
        // CU_ETX deliberately omitted.
        // Didn't your mother tell you not to throw in destructors?
        cuMemFree(position_maps_[i]);
    }
}

#endif /* have cudpp */

#endif /* have opengl  */
#endif /* have cuda */

void Chamfer_likelihood::set_maps(
        const Matrix& distance,
        const Int_matrix& row_position,
        const Int_matrix& col_position,
        size_t num_edges)
{
    distance_ = &distance;
    row_position_ = row_position;
    col_position_ = col_position;
    num_edges_ = num_edges;
}

void Chamfer_likelihood::operator()(const Renderable& r, bool invert_y) 
{
    ASSERT(distance_);

    offscreen_buffer_.render(r);

    const int width = distance_->get_num_cols();
    const int height = distance_->get_num_rows();


    // TODO: make this a member
    std::vector<float> buffer_v(width * height);
    float* buffer = &buffer_v[0];

    // TODO: use DMA to pipeline this
    // or http://stackoverflow.com/questions/3073796/how-to-use-glcopyimage2d
    offscreen_buffer_.bind();
    glReadPixels(0,0,width, height, GL_RED, GL_FLOAT, buffer);
    offscreen_buffer_.unbind();

    evaluate_dispatch_(buffer, invert_y);
}

void Chamfer_likelihood::operator()(const kjb::Matrix& r)
{
    const int width = distance_->get_num_cols();
    const int height = distance_->get_num_rows();

    if(r.get_num_cols() != width || r.get_num_rows() != height)
        KJB_THROW(kjb::Dimension_mismatch);

    int N = width * height;

    std::vector<float> buffer_v(N);

    std::copy(
        r.get_c_matrix()->elements[0],
        r.get_c_matrix()->elements[0]+N,
        buffer_v.begin());

    static const bool invert_y = false;
    evaluate_dispatch_(&buffer_v[0], invert_y);
}

void Chamfer_likelihood::evaluate_dispatch_(float* buffer, bool reverse_y)
{
    const int width = distance_->get_num_cols();
    const int height = distance_->get_num_rows();

    // TODO: make this a member to avoid constant allocation
//    Matrix masked_positions(height, width, 0.0);
    std::vector<char> masked_positions(width * height, 0);

    sum_ = 0;
    model_points_= 0;
    data_points_ = num_edges_;
    correspondences_ = 0;

    int i = 0;

    std::vector<double> distance;
    distance.reserve(num_edges_* 2);

    for(int row = 0; row < height; row++)
    for(int col = 0; col < width; col++, i++)
    {
        if(reverse_y)
            row = height-1 - row;

        if(buffer[i] > 0.0)
        {

            // TODO: make sure this a reasonable size
            distance.push_back((*distance_)(row, col));

            model_points_++;

            int mask_row = (int) row_position_(row, col);
            int mask_col = (int) col_position_(row, col);

            masked_positions[mask_row * width + mask_col] = 1;
        }
    }

    if(square_values_)
    {
        for(size_t i = 0; i < distance.size(); i++)
        {
            distance[i] = distance[i] * distance[i];
        }
    }

    sum_ = reduce_in_place(distance, distance.size());   

    // TODO: do this in the original loop
    for(size_t i = 0; i < masked_positions.size(); i++)
    {
        if(masked_positions[i])
        {
            correspondences_++;
        }
    }

    GL_ETX();
}

#endif /* have glew */
