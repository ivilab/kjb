/* $Id$ */
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


/**
 * This is a prototype for performing chamfer distance computation in 
 * Cuda.
 *
 * @author Kyle simek
 */

#include <iomanip>
#include <time.h>

#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_int_matrix.h>
#include <gr_cpp/gr_sprite.h> /* this comes before opengl stuff*/
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gpu_cpp/gpu_cuda.h>
/*#include <g_cpp/g_chamfer.h>*/
#include <edge_cpp/edge_chamfer.h>

#include <likelihood_cpp/edge_chamfer_likelihood.h>

#include <boost/scoped_ptr.hpp>

#ifdef KJB_HAVE_OPENGL
#include <GL/gl.h>
#endif

#ifdef KJB_HAVE_CUDA
#include <cuda.h>
#include <cudaGL.h>
#else
#error "cuda required"
#endif

using namespace kjb;
using namespace kjb::opengl;
using namespace kjb::gpu;

using namespace std;

#define XPETE(a) \
{ \
    try{ \
        a; \
    } \
    catch(Exception& e)     \
    {     \
        std::cerr << "Failed with error: " << e.get_msg() << "\nAborting..." << std::endl;     \
        abort();     \
    }     \
}

// GLOBALS
Image in_img;
Matrix in_img_gray;

Matrix edge_img_gray;
Matrix distance_map;

Sprite edge_sprite;
int edge_count = 0;

Framebuffer_object* fbo;
opengl::Buffer* pbo[2];
Cuda_reduce_module* reduce_mod;

Gpu_chamfer_likelihood* likelihood = NULL;

CUgraphicsResource cuda_pbo_handle;

int width, height;

void generate_edge_sprite()
{
    using kjb_c::kjb_rand;
    // generate edge image
    edge_img_gray = Matrix(height, width, 0.0);
//    Matrix edges(height, width, 255.0);

    const float FILL_RATE = 0.25;
    const int NUM_EDGE_PTS = width * height * FILL_RATE;

    for(int i = 0; i < NUM_EDGE_PTS; i++)
    {
        int x = kjb_rand() * width;
        int y = kjb_rand() * height;

        edge_img_gray(y, x) = 255.0;
    }

    edge_sprite = Sprite(Texture().set_mask_4ui(edge_img_gray));
    edge_sprite.enable_depth_test(false);
    edge_sprite.set_filters(GL_NEAREST, GL_NEAREST);

    edge_count = 0;
    for(int i = 0; i < edge_img_gray.get_length(); i++)
    {
        if(edge_img_gray[i] > 0.0)
            edge_count++;
    }

    assert(edge_count > 0);
}

double cpu_reduce()
{
    double total = 0;
    int count = 0;

    for(int i = 0; i < edge_img_gray.get_length(); i++)
    {
        if(edge_img_gray[i] > 0.0)
        {
            total += distance_map[i];
            count++;
        }
    }

    assert(count > 0.0);

    return total / count;
}

void cleanup()
{

}


int key(int k, int x, int y)
{
    switch(k)
    {
        case 'q':
            cleanup();
            exit(1);
            break;
        default:
            break;
    }
}

void gpu_run()
{
    int num_iterations = 1000;
    for(int i = 0; i < num_iterations; i++)
    {
        (*likelihood)(edge_sprite);
        likelihood->get_sum() / likelihood->get_num_model_points();
    }
}

void cpu_render_and_read(int p_i)
{
    fbo->bind();
    GL_ETX();
    edge_sprite.render();
    GL_ETX();
    pbo[p_i]->bind();
    GL_ETX();
    glReadPixels(0,0,width, height, GL_RED, GL_FLOAT, 0);
    GL_ETX();
}

// 
double cpu_process(int p_i)
{
    pbo[p_i]->bind();
    const GLfloat* result = (GLfloat*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

    double total = 0;
    int count_ = 0;

    int i = 0;
    for(int row = height - 1; row >= 0; row--)
    for(int col = 0; col < width; col++, i++)
    {
        if(result[i] > 0.0)
        {
            total += distance_map(row, col);
            count_++;
        }
    }

    if(!glUnmapBuffer(GL_PIXEL_PACK_BUFFER))     // release pointer to the mapped buffer
        KJB_THROW_2(Opengl_error, "glUnmapBuffer failed.");

    GL_ETX();

    return total / count_;
}

double cpu_display()
{
    cpu_render_and_read(0);
    return cpu_process(0);
}

void cpu_run()
{
    int num_iterations = 1000;

    cpu_render_and_read(0);
    unsigned int pbo = 1;
    for(int i = 0; i < num_iterations; i++, pbo ^= 1)
    {
        cpu_render_and_read(pbo);
        cpu_process(pbo ^ 1);
    }

    cpu_process(pbo ^ 1);
}

void display(){}

int main(int argc, char* argv[])
{
    try {
        Glut_window wnd;
        wnd.set_display_callback(display);
        wnd.set_keyboard_callback(key);

        in_img = Image("edge_in.png");
        width = in_img.get_num_cols();
        height = in_img.get_num_rows();
        wnd.set_size(width, height);

        // invert and convert to binary matrix 
        in_img_gray = in_img.get_inverted().to_grayscale_matrix();
        in_img_gray = in_img_gray.threshold(128) * 255;
        // write binary matrix back to image
        in_img = Image(in_img_gray);

        CUcontext cuda_ctx;

        Cuda::ensure_initialized();
    //    CU_EPETE(cuCtxCreate(&cuda_ctx, 0, Cuda::get_device(0)));

    //    size_t free, total;
    //    CU_EPETE(cuMemGetInfo(&free, &total));
    //
    //    std::cout << "free: " << free << "\ntotal: " << total << std::endl;

    //    CU_EPETE(cuGLInit());
        CU_EPETE(cuGLCtxCreate(&cuda_ctx, 0, Cuda::get_device(0)));


        Cuda_reduce_module reduce_mod_tmp;
        reduce_mod = &reduce_mod_tmp;

        GLenum err = glewInit();

        if (GLEW_OK != err)
        {
           /* Problem: glewInit failed, something is seriously wrong. */
           fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
           exit(1);
        }
        fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

        atexit(cleanup);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);

    //// CREATE FBO
        Framebuffer_object static_fbo;
        fbo = &static_fbo;

        // depth buffer
        Renderbuffer depth_buffer;
        depth_buffer.allocate(GL_DEPTH_COMPONENT, width, height);

        // colorbuffer
        Renderbuffer color_buffer;
        color_buffer.allocate(GL_RGBA, width, height);

        // attach texture to color part of fbo
        fbo->attach_depth(depth_buffer);
        fbo->attach_color(color_buffer);

        fbo->check();
    //
    //
    //
    //// CREATE PBO
    //    opengl::Buffer static_pbo;
    //    pbo = &static_pbo;
    //    const int NUM_CHANNELS = 4;
    //    const int DATA_SIZE = width * height * NUM_CHANNELS * sizeof(GLuint);
    //    pbo->allocate(GL_PIXEL_PACK_BUFFER, DATA_SIZE, GL_STREAM_READ);
    //
    //// REGISTER PBO WITH CUDA
    //    CU_ETX(cuGraphicsGLRegisterBuffer(&cuda_pbo_handle,
    //            *pbo,
    //            CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE));
    //

        const int DATA_SIZE = width * height * sizeof(GLfloat);

        opengl::Buffer static_pbos[2];
        static_pbos[0].allocate(GL_PIXEL_PACK_BUFFER, DATA_SIZE, GL_STREAM_READ);
        static_pbos[1].allocate(GL_PIXEL_PACK_BUFFER, DATA_SIZE, GL_STREAM_READ);
        pbo[0] = &static_pbos[0];
        pbo[1] = &static_pbos[1];

        Edge_set_ptr edges = Canny_edge_detector(1,1,1)(in_img, true);
        Chamfer_transform xfm(edges);

        bool use_direct_mode = false;
        if(argc == 2)
            use_direct_mode = true;

        CUarray distance_array;
        CUdeviceptr distance_vector;
        boost::scoped_ptr<Gpu_chamfer_likelihood> tmp_likelihood(new Gpu_chamfer_likelihood(width, height));
        likelihood = tmp_likelihood.get();

        distance_map = xfm.distance_map();
        CUdeviceptr cu_distances = tmp_likelihood->create_distance_map(xfm.distance_map());


        
        std::vector<kjb::Int_matrix> positions = xfm.position_map();

        CUdeviceptr cu_positions = tmp_likelihood->create_position_map(positions[0], positions[1]);
        tmp_likelihood->set_maps(cu_distances, cu_positions, edges->get_total_edge_points());

    // CREATE RANDOM EDGE SPRITE
        generate_edge_sprite();

        cout << "gold standard result: " << cpu_reduce() << endl;
        cout << "cpu result: " << cpu_display() << endl;
        (*tmp_likelihood)(edge_sprite);
        cout << "gpu result: " << tmp_likelihood->get_sum() / tmp_likelihood->get_num_model_points() << endl;

        clock_t begin = clock();
        if(argc == 4)
        { 
    //        try{ 
                cpu_run();
    //        } 
    //        catch(Exception& e)     
    //        {     
    //            std::cerr << "Failed with error in file " << e.get_file() << ":" << e.get_line() << "  " << e.get_msg() << "nAborting..." << std::endl;     
    //            abort();     
    //        }     
        }
        else
            gpu_run();

        clock_t end = clock();
        cout << "elapsed: " << ((float) end - begin) / CLOCKS_PER_SEC << endl;

        if (use_direct_mode)
        {
            cuArrayDestroy(distance_array);
        }
        else
        {
            cuMemFree(distance_vector);
        }

        exit(0);

        glutMainLoop();

        cuMemFree(cu_distances);
        cuMemFree(cu_positions);
    }
    catch(std::exception& ex) {}
    catch(Cuda_error& er) 
    {
        cout << er.get_msg() << endl;
    }
    
    
    return 0;
}

