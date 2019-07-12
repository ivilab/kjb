/* $Id: test_chamfer.cpp 17425 2014-08-30 00:34:38Z predoehl $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker


#define DEPS_MET 1

#include <iostream>
#ifndef KJB_HAVE_CUDA
#undef DEPS_MET
#define DEPS_MET 0
#endif

#ifndef KJB_HAVE_CUDPP
#undef DEPS_MET
#define DEPS_MET 0
#endif 

#ifndef KJB_HAVE_GLEW
#undef DEPS_MET
#define DEPS_MET   0
#endif

#ifndef KJB_HAVE_OPENGL
#undef DEPS_MET
#define DEPS_MET   0
#endif

#if DEPS_MET == 1

#include <cuda.h>

#include <gpu_cpp/gpu_cuda.h>
#include <gpu_cpp/gpu_cudpp.h>
#include <gpu_cpp/gpu_cuda_debug.h>
#include <likelihood_cpp/edge_chamfer_likelihood.h>
#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_cpp_incl.h>

#include <iostream>
#include <time.h>

#include <gr_cpp/gr_sprite.h>
#include <gr_cpp/gr_glut.h>


using namespace std;
using namespace kjb;
using namespace kjb::gpu;
using namespace kjb::opengl;


// make a single renderable look like a multi-view renderable
class Faux_multiview : public Mv_renderable
{
public:
    Faux_multiview(const Renderable& r, size_t num_views) :
        renderable_(&r),
        num_(num_views)
    {}

    virtual size_t num_views() const { return num_; }

    virtual void set_active_view(size_t i) const
    { }

    virtual void render() const { renderable_->render(); }

private:
    const Renderable* renderable_;
    size_t num_;
};

void output_results(const Gpu_chamfer_likelihood& likelihood, bool squared = false)
{
    // get 4 numbers
    size_t sum;
    if(squared) sum = likelihood.get_sq_sum();
    else sum = likelihood.get_sum();

    size_t model_points = likelihood.get_num_model_points();
    size_t data_points = likelihood.get_num_data_points();
    size_t correspondences = likelihood.get_num_correspondences();

    cout << "sum: " << sum << endl;
    cout << "model_points: " << model_points << endl;
    cout << "data_points: " << data_points << endl;
    cout << "correspondences: " << correspondences << endl;
}
    
void display() {}

void test_multi(const Chamfer_transform& xfm, const Renderable& renderable)
{
    const size_t NUM_ANGLES = 18;

    Faux_multiview mv_renderable(renderable, NUM_ANGLES);

    Multi_gpu_chamfer_likelihood likelihood(
            xfm.get_num_cols(),
            xfm.get_num_rows());


    for(int i = 0; i < NUM_ANGLES; i++)
    {
        likelihood.push_back(xfm);
    }

    likelihood(mv_renderable);

    double sum = likelihood.get_sum();
    size_t model_points = likelihood.get_num_model_points();
    size_t data_points = likelihood.get_num_data_points();
    size_t correspondences = likelihood.get_num_correspondences();

    cout << "Multi-view results:" << endl;
    cout << "Avg sum: " << sum / NUM_ANGLES << endl;
    cout << "Avg model points: " << (double) model_points / NUM_ANGLES << endl;
    cout << "Avg data points: " << (double) data_points / NUM_ANGLES << endl;
    cout << "Avg correspondences: " << (double) correspondences/ NUM_ANGLES << endl;

    likelihood.square_values(true);
    likelihood(mv_renderable);

    sum = likelihood.get_sq_sum();
    model_points = likelihood.get_num_model_points();
    data_points = likelihood.get_num_data_points();
    correspondences = likelihood.get_num_correspondences();

    cout << "Multi-view results (squared):" << endl;
    cout << "Avg square sum: " << sum / NUM_ANGLES << endl;
    cout << "Avg model points: " << (double) model_points / NUM_ANGLES << endl;
    cout << "Avg data points: " << (double) data_points / NUM_ANGLES << endl;
    cout << "Avg correspondences: " << (double) correspondences/ NUM_ANGLES << endl;

}

int main(int argc, char* argv[])
{
    size_t NUM_ITERATIONS = 1000;

    try{
    // OPENGL INIT
    opengl::Glut_window wnd; // sets up opengl context
    wnd.set_display_callback(display);

    // CUDA INIT
    Cuda::ensure_initialized();
    CUcontext ctx;
    CU_ETX(cuGLCtxCreate(&ctx, 0, Cuda::get_device(0)));

    // GLEW INIT
    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
       /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
       exit(1);
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


    float FILL_RATE = 0.1;
//    float FILL_RATE = 1.0;

    // read image 
    Image img("input/edge_in.png");

    // get edges
    Edge_set_ptr edges = edge_image_to_edge_points(img, false);

    // transform
    Chamfer_transform xfm(edges);

    int width = img.get_num_cols();
    int height = img.get_num_rows();

    // construct likelihood
    Gpu_chamfer_likelihood likelihood(width, height);

    // create cuda structures
    std::vector<kjb::Int_matrix> positions = xfm.position_map();
    CUdeviceptr cu_distances = likelihood.create_distance_map(xfm.distance_map());
    CUdeviceptr cu_positions = likelihood.create_position_map(positions[0], positions[1]);



    // generate random edge image
    Matrix model_edges = create_random_matrix(height, width);
    model_edges = model_edges.threshold(1.0-FILL_RATE);
    model_edges *= 255;

    // make matrix renderable
    opengl::Sprite edge_sprite(model_edges);

    // COMPARE WITH GOLD STANDARD
    size_t trash = 0;

// GOLD STANDARD EVALUATE
    // pass to gold standard
    likelihood.gold_standard_process(edge_sprite, xfm.distance_map(), positions[0], positions[1], trash);

    cout << "\nGOLD STANDARD RESULTS:" << endl;
    output_results(likelihood);

    likelihood.square_values(true);
    likelihood.gold_standard_process(edge_sprite, xfm.distance_map(), positions[0], positions[1], trash);
    cout << "\nGOLD STANDARD RESULTS:" << endl;
    output_results(likelihood, true);


// GPU EVALUATE
    // pass maps to likelihood
    likelihood.square_values(false);
    likelihood.set_maps(cu_distances, cu_positions, edges->get_total_edge_points());

    // evaluate
    likelihood(edge_sprite);
    cout << "\nGPU RESULTS:" << endl;
    output_results(likelihood);

    likelihood.square_values(true);
    likelihood(edge_sprite);
    cout << "\nGPU RESULTS (squared):" << endl;
    output_results(likelihood, true);


    cout << "\nTesting multi-view:" << endl;
    test_multi(xfm, edge_sprite);


    cout << "\nTiming GPU squared algorithm " << NUM_ITERATIONS << " iterations: " << std::flush;
    clock_t begin = clock();
    for(int i = 0; i < NUM_ITERATIONS; i++)
    {

        likelihood.set_maps(cu_distances, cu_positions, edges->get_total_edge_points());

        likelihood(edge_sprite);
    }
    clock_t end = clock();
    cout << ((float) end - begin) / CLOCKS_PER_SEC << 's' << endl;





    cout << "\nTiming gold standard algorithm (this may take a while)... " << NUM_ITERATIONS << " iterations: " << std::flush;
    begin = clock();
    for(int i = 0; i < NUM_ITERATIONS; i++)
    {
        likelihood.gold_standard_process(edge_sprite, xfm.distance_map(), positions[0], positions[1], trash);
    }
    end = clock();
    cout << ((float) end - begin) / CLOCKS_PER_SEC << 's' << endl;


    // get 4 numbers
    // compare and return
    kjb_gpu_debug_to_vector_uint(cu_distances, width, height, width * sizeof(unsigned int));
    }
    catch(std::exception& er) {}
    catch(Cuda_error& er)
    {
        cout << "Cuda error: " << er.get_msg() << endl;
    }



    return 0;
}
#else

int main(int argc, char* argv[])
{
    std::cerr << "Missing dependencies.  Need cuda, cudpp, glew and opengl installed." << std::endl;

    return 0;
}

#endif /* deps_met */
