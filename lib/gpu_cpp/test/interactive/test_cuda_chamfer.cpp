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
 * In general, this approach shouldn't be used, because it will by implemented
 * more cleanly in an object-oriented interface.  But I want to keep this test,
 * because it has some low-level testing code that will help debugging in
 * the future.  I anticipate problems when switching to an little-endian
 * architecture, so having this code at hand will make my life easier
 * when handling that.
 *
 * @author Kyle simek
 */

#include <iomanip>

#include <boost/scoped_array.hpp>
#include <l_cpp/l_cpp_incl.h>
#include <gr_cpp/gr_sprite.h> /* this comes before opengl stuff */
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gpu_cpp/gpu_cuda.h>
/*#include <g_cpp/g_chamfer.h> */
#include <edge_cpp/edge_chamfer.h>

#include <likelihood_cpp/edge_chamfer_likelihood.h>

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

// GLOBALS
Image in_img;
Matrix in_img_gray;

Matrix edge_img_gray;
int edge_count;

Matrix* cur_edge_mask;
Matrix distance_map;

Matrix* cur_mask = &edge_img_gray;

Gpu_chamfer_likelihood* likelihood;

CUarray distance_array;

int offset_x = 0;
int offset_y = 0;

bool random_mode = true;
bool perform_sum = false;
bool auto_mode = false;

Sprite distance_sprite;
Sprite edge_sprite;
Sprite in_sprite;
Sprite* cur_sprite = &edge_sprite;

Framebuffer_object* fbo;
Buffer* pbo;
Cuda_reduce_module* reduce_mod;

CUgraphicsResource cuda_pbo_handle;

int width, height;

float* to_float_array(const Matrix& m)
{
    float* out = new float[m.get_length()];

    float* cur = out;

    for(int row = m.get_num_rows() - 1; row >= 0; row--)
    for(int col = 0; col < m.get_num_cols(); col++)
    {
        *cur++ = (float) m(row,col);
    }

    return out;
}

void generate_edge_sprite()
{
    using kjb_c::kjb_rand;
    // generate edge image
    edge_img_gray = Matrix(height, width, 0.0);
//    Matrix edges(height, width, 255.0);

    const float FILL_RATE = 0.0005;
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


float cpu_chamfer(const Matrix& mask)
{
    float total = 0;

    const int num_rows = height;
    const int num_cols = width;

    int count = 0;

    for(int row = num_rows - 1; row >= 0; row--)
    for(int col = 0; col < num_cols; col++)
    {
        if(mask(row, col) > 0.0)
        {
            assert(mask(row, col) == 255);
            float value  = distance_map(row, col);
            total += value;
            count++;
        }
    }

    cout << "cpu total: " << total << endl;
    cout << "cpu count: " << count << endl;
    return total / count;
}

void render()
{
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glDisable(GL_BLEND);

    if(random_mode)
    {
        // render edge image
//        edge_sprite.set_position(0,9);
        edge_sprite.render();
        cur_mask = &edge_img_gray;
        cur_sprite = &edge_sprite;
    }
    else
    {
//        in_sprite.set_position(0,9);
        in_sprite.set_position(offset_x, offset_y);
        in_sprite.render();

        cur_mask = &in_img_gray;
        cur_sprite = &in_sprite;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    // render distance texture over it.
//    distance_sprite.set_position(0,9);
    distance_sprite.render();

    glDisable(GL_BLEND);
}

void display()
{
    cout << "display called..." << endl;
    using debug::to_bitstring;

    render();

    if(perform_sum || auto_mode)
    {
        (*likelihood)(*cur_sprite);

        cout << setprecision(7) << "likelihood: " << likelihood->get_sum()/ likelihood->get_num_model_points() << endl;

        fbo->bind();
        render();
        pbo->bind();
        glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
        pbo->unbind();
        fbo->unbind();

        // do total from pbo
        pbo->bind();
        const GLfloat* result = (GLfloat*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        GL_ETX();


        const GLfloat* cur_pbo = result;
        float pbo_total= 0;

        for(int row = height - 1; row >= 0; row--)
        for(int col = 0; col < width; col++)
        {
            float ref_value = ((*cur_mask)(row, col) > 0.0 ? distance_map(row, col) : 0.0);

            float out_value = *cur_pbo;
//            cout << to_bitstring(out_value) << '\t' << out_value << '\t';
//            cout << to_bitstring(ref_value) << '\t' << ref_value << '\n';
            pbo_total += out_value;

            *cur_pbo++;
        }

        cout << "pbo_total: " << pbo_total / edge_count << endl;

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        pbo->unbind();
        GL_ETX();

        // THE FOLLOWING ~8 LINES would be nice to have an OO solution for
        CU_ETX(cuGraphicsMapResources(1, &cuda_pbo_handle, 0));
        CUdeviceptr d_in;

#if CUDA_VERSION < 3020
        unsigned int num_bytes;
#else
        size_t num_bytes;
#endif

        CU_ETX(cuGraphicsResourceGetMappedPointer(&d_in, &num_bytes, cuda_pbo_handle));

        float gpu_total = reduce_mod->reduce<float>(d_in, width * height);
        CU_ETX(cuGraphicsUnmapResources(1, &cuda_pbo_handle, 0));

        // check number of alpha bits
        int alpha_bits;
        glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);

        std::cout << "alpha_bits: " << alpha_bits << std::endl;


        unsigned char* bdata = new unsigned char[width * height * 4];
//        float* data = new float[width * height];


        render();
        
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, bdata);

//        float* cur = data;
        float* cur = (float*) bdata;
        float total = 0;
        float cpu_total = 0;


        // write distance map to file
        for(int row = height - 1; row >= 0; row--)
        for(int col = 0; col < width; col++)
        {
            float out_value = *cur;
            float ref_value = ((*cur_mask)(row, col) > 0.0 ? distance_map(row, col) : 0.0);

            std::cout << to_bitstring(out_value) << "\t" << out_value << '\t' ;
            std::cout << to_bitstring(ref_value) << "\t" << ref_value << '\n';
            total += out_value;
            cpu_total += ref_value;

            cur++;
        }

        std::cout << "rendered total: " << total / edge_count << std::endl;
        std::cout << "gpu total: " << gpu_total / edge_count << std::endl;

        std::cout << "cpu_total: " << cpu_total / edge_count << std::endl;
        std::cout << "chamfer total: " << cpu_chamfer(*cur_mask) << std::endl;

//        delete[] data;
        delete[] bdata;

        perform_sum = false;

        if(auto_mode) exit(0);
    }


    glutSwapBuffers();
}

void cleanup()
{
   cuArrayDestroy(distance_array);

}


int key(int k, int x, int y)
{
    switch(k)
    {
        case 'q':
            cleanup();
            exit(1);
            break;
        case ' ':
            generate_edge_sprite();
            glutPostRedisplay();
            break;
        case '\t':
            random_mode = !random_mode;
            break;
        case 'a':
            offset_x -= 1;
            break;
        case 'd':
            offset_x += 1;
            break;
        case 's':
            offset_y -= 1;
            break;
        case 'w':
            offset_y += 1;
            break;

        case 'R':
        {
            perform_sum = true;
            glutPostRedisplay();
            break;
        }
        default:
            break;
    }
}

int main(int argc, char* argv[])
{
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
    in_img.display();

    Cuda::ensure_initialized();
    cuGLInit();

    CUcontext cuda_ctx;
    CU_ETX(cuGLCtxCreate(&cuda_ctx, 0, Cuda::get_device(0)));

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

// CREATE FBO
    Framebuffer_object static_fbo;
    fbo = &static_fbo;

    // depth buffer
    Renderbuffer depth_buffer;
    depth_buffer.allocate(GL_DEPTH_COMPONENT, width, height);

    // colorbuffer
    Renderbuffer color_buffer;
    color_buffer.allocate(GL_R32F, width, height);
    // this works too:
//    color_buffer.allocate(GL_RED, width, height);

    // attach texture to color part of fbo
    fbo->attach_depth(depth_buffer);
    fbo->attach_color(color_buffer);

    fbo->check();



// CREATE PBO
    opengl::Buffer static_pbo;
    pbo = &static_pbo;
    const int NUM_CHANNELS = 4;
    const int DATA_SIZE = width * height * NUM_CHANNELS * sizeof(GLuint);
    pbo->allocate(GL_PIXEL_PACK_BUFFER, DATA_SIZE, GL_STREAM_READ);

// REGISTER PBO WITH CUDA
    CU_ETX(cuGraphicsGLRegisterBuffer(&cuda_pbo_handle,
            *pbo,
            CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE));

// CREATE EDGE IMAGE SPRITE
//    in_sprite = Sprite(Texture().set_mask_4ui(in_img_gray));
    in_sprite = Sprite(Texture().set_mask_4ui(in_img_gray));
    in_sprite.enable_depth_test(false);
    in_sprite.set_filters(GL_NEAREST, GL_NEAREST);

// CREATE CHAMFER SPRITE
    Edge_set_ptr edges = Canny_edge_detector(1,1,1)(in_img, true);

    Chamfer_transform xfm(edges);

//
//    // check that it looks okay
//    distance_map.display("distances");


    distance_map = xfm.distance_map();
    Image distance_img(distance_map);
    distance_img.display();

    // convert distance map into a sprite
    // todo: handle full precision
    distance_sprite = Sprite(Texture().set_packed_float(distance_map));

    distance_sprite.enable_depth_test(false);
    distance_sprite.set_filters(GL_NEAREST, GL_NEAREST);

// SET UP LIKELIHOOD
    Gpu_chamfer_likelihood static_likelihood(width, height);
    CUdeviceptr cu_distances = static_likelihood.create_distance_map(xfm.distance_map());

    vector<Int_matrix> positions = xfm.position_map();
    CUdeviceptr cu_positions = static_likelihood.create_position_map(positions[0], positions[1]);

    static_likelihood.set_maps(cu_distances, cu_positions, edges->get_total_edge_points());
    likelihood = &static_likelihood;

// CREATE RANDOM EDGE SPRITE
    generate_edge_sprite();
    random_mode = true;

    // any flags means run in non-interactive mode
    if(argc >= 2)
    {
        cout << "auto mode enabled" << endl;
        auto_mode = true;
        display();
    }

    glutMainLoop();
    
    
    return 0;
}
