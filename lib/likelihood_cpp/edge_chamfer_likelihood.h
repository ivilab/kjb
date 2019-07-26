/* $Id: edge_chamfer_likelihood.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef KJB_EDGE_CHAMFER_LIKELIHOOD_H
#define KJB_EDGE_CHAMFER_LIKELIHOOD_H

#include <l/l_incl.h>
#include <l/l_def.h>
#include <edge_cpp/edge.h>
#include <edge/edge_base.h>
#include <g/g_chamfer.h>
#include <l/l_int_matrix.h>
#include <m/m_matrix.h>
#include <m_cpp/m_int_matrix.h>
#include <i_cpp/i_image.h>
#include <vector>
#include <list>

#include <l_cpp/l_debug.h>

#include "edge_cpp/edge_chamfer.h"

#ifdef KJB_HAVE_GLEW
#include "GL/glew.h"
#endif

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_object.h>
#include <gr_cpp/gr_sprite.h>
#include <gr_cpp/gr_renderable.h>
#include <gr_cpp/gr_fbo_offscreen.h>

#ifdef KJB_HAVE_CUDA
#include <cuda.h>
#include <cudaGL.h>
#endif

#ifdef KJB_HAVE_CUDPP
#include <cudpp.h>
#endif

#include <gpu_cpp/gpu_cudpp.h>


#include <gpu_cpp/gpu_cuda.h>
#include <gpu_cpp/gpu_cuda_util.h>

#ifdef TEST
#include <gr_cpp/gr_display.h>
#endif

namespace kjb {



#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_GLEW
class Base_chamfer_likelihood
{
public:
    Base_chamfer_likelihood(size_t width, size_t height) :
        sum_(0),
        model_points_(0),
        data_points_(0),
        correspondences_(0),
        square_values_(false),
        offscreen_buffer_(width, height)
    {
    }

    /// Call after operator() to retreive number of rendered model edge points
    size_t get_num_model_points() const
    {
        return model_points_;
    }

    /// Call after operator() to retreive number of model edge points with corresponding data points
    size_t get_num_correspondences() const
    {
        return correspondences_;
    }

    /// Call after operator() to retreive number of edge points in the data
    size_t get_num_data_points() const
    {
        return data_points_;
    }

    /** 
     * Get the sum of distance between all sets of corresponding model and edge points.
     *
     * If square_values() has been called, this will throw an exception; use get_sq_sum() instead.
     */
    double get_sum() const
    {
        if(square_values_)
        {
            KJB_THROW_2(Runtime_error, "Attempting to get sum when square_values is set.  Use get_sq_sum() instead.");
        }
        return sum_;
    }

    /** 
     * Get the sum of squared distances between all sets of corresponding model and edge points.
     *
     * If square_values() has not been called, this will throw an exception; use get_sum() instead.
     */
    double get_sq_sum() const
    {
        if(!square_values_)
        {
            KJB_THROW_2(Runtime_error, "Attempting to get squared sum when square_values is not set.  Use get_sum() instead, or call square_values(true) and re-evaluate.");
        }
        return sum_;
    }

    /**
     * Set to true to square values before summing them
     */
    void square_values(bool enabled)
    {
        square_values_ = enabled;
    }

protected:
    double sum_;
    size_t model_points_;
    size_t data_points_;
    size_t correspondences_;

    bool square_values_;

    ::kjb::opengl::Fbo_offscreen_buffer offscreen_buffer_;
};


#ifdef KJB_HAVE_CUDA

class Base_gpu_chamfer_likelihood : public Base_chamfer_likelihood
{
typedef Base_chamfer_likelihood Base;
public:
    /**
     * @param width_in Width of the display buffer in pixels
     * @param height_in Height of the display buffer in pixels
     */
    Base_gpu_chamfer_likelihood(int width_in, int height_in) :
        Base(width_in, height_in),
        cuda_rbo_handle_(),
        reduce_mod_()
    {
        CU_ETX(cuGraphicsGLRegisterImage(
            &cuda_rbo_handle_,
            offscreen_buffer_.get_color_buffer(),
            GL_RENDERBUFFER,
            CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE));
    }


    /// compute chamfer distance from white edge on black background renderable
    virtual void operator()(const Renderable& object) = 0;

    virtual ~Base_gpu_chamfer_likelihood()
    {
        cuGraphicsUnregisterResource(cuda_rbo_handle_);
    }

protected:

    CUarray map_rbo();

    void render_to_fbo(const Renderable& object);

    void unmap_rbo()
    {
        CU_ETX(cuGraphicsUnmapResources(1, &cuda_rbo_handle_, 0));
    }
protected:
    CUgraphicsResource cuda_rbo_handle_;

    gpu::Cuda_reduce_module reduce_mod_;
};


#ifdef KJB_HAVE_CUDPP
class Gpu_chamfer_likelihood : public Base_gpu_chamfer_likelihood
{
public:
    /**
     * Distance map sprites can be added after using push_back
     */
    Gpu_chamfer_likelihood(int width, int height);



    virtual ~Gpu_chamfer_likelihood()
    {
        cudppDestroy(cudpp_handle_);
        cuMemFree(buffer_ptr_);
        cuMemFree(workspace_ui_);
    }

    /**
     * @param distance Cuda array of floats representing distances to corresponding point.  The array must have the same dimensions as the render buffer, and must have been alocated using cuMemAllocPitch().  The create_distance_map method will create the array correctly for you.
     * @param position Cuda array of unsigned ints representing the address of the corresponding point (address = row * width + col).  The array must have the same dimensions as the render buffer, and must have been alocated using cuMemAllocPitch().  The create_position_map method will create the array correctly for you.
     * @param num_points The number of points in the space.
     * @param N The number of elements in distance and position arrays (including the "gutter" added by cuMemAllocPitch).  This is optional, and is used as a sanity check that the array sizes match the render buffer array size.  If ommitted or zero, it is assumed to be the same as buffer.  However, if sizes do differ, behavior is undefined and video memory could be corrupted, which is quite difficult to debug, so this parameter is recommended.
     *
     * @see create_distance_map
     * @see create_position_map
     */
    void set_maps(const CUdeviceptr distance, const CUdeviceptr position, size_t num_edges, size_t N = 0);

    /** 
     * Allocate and construct a distance map in Cuda from a kjb::Matrix.
     * Caller is responsible for freeing using cuMemFree().
     */
    CUdeviceptr create_distance_map(const Matrix& distance_map);

    /** 
     * Allocate and construct a position map in Cuda from two kjb::Int_matrices representing
     * the row and column indices of the nearest point, respectively.
     *
     * Caller is responsible for freeing using cuMemFree().
     */
    CUdeviceptr create_position_map(const std::vector<Int_matrix>& position_map)
    {
        return create_position_map(position_map[0], position_map[1]);
    }

    /** 
     * Allocate and construct a position map in Cuda from Int_matrices storing
     * the row and column indices of the nearest point.
     *
     * Caller is responsible for freeing using cuMemFree().
     */
    CUdeviceptr create_position_map(const Int_matrix& row_position_map, const Int_matrix& col_position_map);

    /**
     * Process the renderable r.
     *
     * @note The window that was active when this object was created must be active before calling this.  If this is called from the window's display callback, you should be fine.  However, if you're calling this by some other means (e.g. from the Glut class's "task pump" feature) you must not forget to active the window before calling this.  If using the Glut_window interface, use the set_current() method, or in native opengl, use the glSetWindow() function.  Otherwise, this will throw an Opengl_error exception.
     *
     * @TODO Consider storing a handle to the owning window inside this class and activate it at beginning of call.
     */
    void operator()(const Renderable& r);





    /// for debugging; does everything in cpu
    void gold_standard_process(const Renderable& r, const Chamfer_transform& xfm) 
    {
        std::vector<Int_matrix> positions = xfm.position_map();

        gold_standard_process(r, xfm.distance_map(), positions[0], positions[1], xfm.get_num_points());
    }

    /// for debugging; does everything in cpu
    void gold_standard_process(const Renderable& r, Matrix distance, const Int_matrix& row_position, const Int_matrix& col_position, size_t num_points) ;

#ifdef TEST
    /// return current contents of this object's frame-buffer object.
    Matrix get_buffer() const;

    // display the current contents of this object's frame-buffer object.
    void display_buffer(const std::string& str = "") const;
#endif /* TEST */

private:
    /// Receives a cuda pointer and returns a linear cuda array
    CUdeviceptr array_to_linear(CUarray array);


    /**
     * sort array in-place
     * @param uint_array An array of unsigned integers of length N_, where N_ is the number of elements in the display buffer
     *
     * @pre all values to sort fit into lowest 32 bits
     * @warning This is designed to only be called by operator().  Do not call directly.
     */
    void uint_cuda_sort_(CUdeviceptr uint_array);

    /**
     * Sum all elements in uint_array.
     *
     * @note Uses the scan algorithm and returns the last value.  Thus, a workspace is necessary
     * that is at least as big as uint_array.  This is likely to be much slower than a 
     * dedicated reduce procedure (I've heard it's around 3 times slower, but I cant
     * remember where, so take that with a grain of salt).
     *
     * @param uint_array the array to reduce
     * @param workspace An unsigned int array in cuda at least as big as uint_array
     */
    double uint_reduce_(CUdeviceptr uint_array, CUdeviceptr workspace);


    gpu::Cuda_utility_module util_mod_;

    CUDPPHandle cudpp_handle_;

    CUdeviceptr distance_map_;
    CUdeviceptr position_map_;
    CUdeviceptr buffer_ptr_;
    CUdeviceptr workspace_ui_;

#if CUDA_VERSION < 3020
    unsigned int buffer_pitch_bytes_;
#else
    size_t buffer_pitch_bytes_;
#endif

    size_t buffer_pitch_elements_;
    size_t N_;

};



/**
 * Chamfer likelihood for multi-view objects using gpu.
 */
class Multi_gpu_chamfer_likelihood
{
public:
    /**
     * Distance map sprites can be added after using push_back
     *
     * @param modulo Determines which of the views to use in evaluation.  Setting to 1 uses all views, 2 uses even-numbered views, 3 uses every third view, etc.  Note that increasing modulo will result in fewer calls to push_back().
     */
    Multi_gpu_chamfer_likelihood(int width, int height, int modulo = 1) :
        distance_maps_(),
        position_maps_(),
        data_point_sizes_(),
        likelihood_(width, height),
        sum_(0),
        model_points_(0),
        data_points_(0),
        correspondences_(0),
        square_values_(false),
        modulo_(modulo),
        view_list_()
    { }

    Multi_gpu_chamfer_likelihood(int width, int height, const std::vector<size_t>& view_list) :
        distance_maps_(),
        position_maps_(),
        data_point_sizes_(),
        likelihood_(width, height),
        sum_(0),
        model_points_(0),
        data_points_(0),
        correspondences_(0),
        square_values_(false),
        modulo_(1),
        view_list_(view_list)
    { }


    void square_values(bool enabled)
    {
        // storing the state here and in the inner
        // likelihood is not very elegant...

        square_values_ = enabled;
        likelihood_.square_values(enabled);
    }

    /**
     * Add data from a new viewpoint.
     */
    virtual void push_back(const Chamfer_transform& xfm);
    

    /**
     * Add a dataset for a new viewpoint.  
     *
     * @param distance_map  A map of distances between every position and the corresponding data set point.
     * @param row_position_map A map of row indices of corresponding data point
     * @param col_position_map A map of column indices of correspodning data point
     * @param num_points The number of data points.
     */
    virtual void push_back(
            const Matrix& distance_map,
            const Int_matrix& row_position_map,
            const Int_matrix& col_position_map,
            size_t num_points);


    virtual void operator()(const Mv_renderable& m);

    std::vector<size_t> get_view_list(const size_t num_views);

    /**
     * For debugging, a gold standard cpu-based version of the 
     * likelihood.
     *
     * If this object was created with modulo > 1, all vectors
     * passed to this method should have size = m.num_views() / modulo
     */
    void gold_standard_evaluate(
        const Mv_renderable& m,
        std::vector<Matrix> distances,
        std::vector<Int_matrix> row_positions,
        std::vector<Int_matrix> col_positions,
        std::vector<size_t> num_points
    );



    double get_sum() const
    {
        if(square_values_)
        {
            KJB_THROW_2(Runtime_error, "Attempting to get sum when square_values is set.  Use get_sq_sum() instead.");
        }
        return sum_;
    }

    double get_sq_sum() const
    {
        if(!square_values_)
        {
            KJB_THROW_2(Runtime_error, "Attempting to get squared sum when square_values is not set.  Use get_sum() instead, or call square_values(true) and re-evaluate.");
        }
        
        return sum_;
    }

    size_t get_num_correspondences() const
    {
        return correspondences_;
    }

    size_t get_num_data_points() const
    {
        return data_points_;
    }

    size_t get_num_model_points() const
    {
        return model_points_;
    }

#ifdef TEST
    void display_buffer(const std::string title = "") const
    {
        likelihood_.display_buffer(title);
    }

    Matrix get_buffer() const
    {
        return likelihood_.get_buffer();
    }
#endif /* TEST */

    ~Multi_gpu_chamfer_likelihood();


private:
    std::vector<CUdeviceptr> distance_maps_;
    std::vector<CUdeviceptr> position_maps_;
    std::vector<size_t> data_point_sizes_;
    Gpu_chamfer_likelihood likelihood_;

    double sum_;
    size_t model_points_;
    size_t data_points_;
    size_t correspondences_;

    bool square_values_;

    size_t modulo_;
    std::vector<size_t> view_list_;
};




#endif /* have cudpp */

#endif /* have cuda */

class Chamfer_likelihood : public Base_chamfer_likelihood
{
public:
    Chamfer_likelihood(size_t width, size_t height) :
        Base_chamfer_likelihood(width, height),
        distance_(NULL),
        row_position_(),
        col_position_(),
        num_edges_()
    {}

    void set_maps(
            const Matrix& distance,
            const Int_matrix& row_position,
            const Int_matrix& col_position,
            size_t num_edges);

    /// for debugging; does everything in cpu
    void set_maps(const Chamfer_transform& xfm) 
    {
        xfm.position_map(row_position_, col_position_);

        set_maps(xfm.distance_map(), row_position_, col_position_, xfm.get_num_points());
    }

    /** 
     * Compare against an opengl renderable.  Note that by
     * default, opengl renders things upside down, so this
     * function re-flips the image before evaluating. Some
     * projects do this flipping manually during rendering;
     * in this case, flipping in this function can 
     * can be disabled by setting invert_y to false.
     */
    void operator()(const Renderable& r, bool invert_y = true);
    void operator()(const kjb::Matrix& img);

    void evaluate_dispatch_(float* buffer, bool invert_y);
private:
    const Matrix* distance_;
    Int_matrix row_position_;
    Int_matrix col_position_;
    size_t num_edges_;
};
#endif /* have glew */
#endif /* have opengl  */

} // namespace kjb

#endif
