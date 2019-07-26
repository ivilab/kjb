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

#ifndef KJB_HAVE_CUDA
#include <l/l_incl.h>
#include <iostream>
int main()
{
    std::cout << "Test requires cuda.\n";
    return EXIT_SUCCESS;
}
#else



#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_GLUT
#else
#error "glut not found"
#endif
#endif
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>


#include <gr_cpp/gr_opengl_object.h>
#include <gpu_cpp/gpu_cuda.h>
#include <cudaGL.h>
#include <gr_cpp/gr_primitive.h>
#include <gr_cpp/gr_glut.h>

#include <time.h>

using namespace std;
using namespace kjb;
using namespace kjb::opengl;
using namespace kjb::gpu;

Framebuffer_object* fbo;
Buffer* pbo;

CUgraphicsResource cuda_pbo_handle;
CUgraphicsResource cuda_rbo_handle;
Cuda_reduce_module* reduce_mod;

int WIDTH = 513;
int HEIGHT = 513;

float cpu_reduce(const float *data, int size)
{
    float sum = data[0];
    float c = (float)0.0;
    int i;
    for (i = 1; i < size; i++)
    {
        float y = data[i] - c;
        float t = sum + y;      
        c = (t - sum) - y;  
        sum = t;            
    }
    return sum;
}



void display()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(WIDTH)/HEIGHT, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // render teapot to fbo
    fbo->bind();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Teapot().render();

    // better to double-buffer this, to parallelize transfer and rendering
//    pbo->bind();
//    GLuint pbo_id = pbo->get();
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);
//    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, 0);

    // do sum in CPU
   
//    const GLfloat* result = (GLfloat*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//    float cpu_result = cpu_reduce(result, WIDTH * HEIGHT);
//    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//    pbo->unbind();
//    GL_ETX();
//
//
//    fbo->unbind();
//    glClearColor(1, 1, 1, 1);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    Teapot().render();
//
//    glutSwapBuffers();

//    CU_ETX(cuGraphicsMapResources(1, &cuda_pbo_handle, 0));
    CU_ETX(cuGraphicsMapResources(1, &cuda_rbo_handle, 0));

//    CUdeviceptr d_in;
    CUarray d_in;
//    size_t num_bytes;
//    CU_ETX(cuGraphicsResourceGetMappedPointer(&d_in, &num_bytes, cuda_pbo_handle));
    CU_ETX(cuGraphicsSubResourceGetMappedArray(&d_in, cuda_rbo_handle, 0, 0));

//    cout << num_bytes << endl;

    float gpu_result = reduce_mod->tex_reduce(d_in, WIDTH, HEIGHT);
//    CU_ETX(cuGraphicsUnmapResources(1, &cuda_pbo_handle, 0));
    CU_ETX(cuGraphicsUnmapResources(1, &cuda_rbo_handle, 0));

//    cout << "gpu_result: " << gpu_result << endl;


//    cout << "cpu_result: " << cpu_result << endl;

}

void key(unsigned int k, int x, int y)
{
    if(k == 'q')
    {
        exit(0);
    }

}

void cleanup()
{
}

int main()
{

    try{
    Glut_window wnd1;
    wnd1.set_display_callback(display);
    wnd1.set_keyboard_callback(key);
    wnd1.set_size(WIDTH, HEIGHT);

    GLenum err = glewInit();

    Cuda::ensure_initialized();


    cuGLInit();

    CUcontext cuda_ctx;
    cout << "Available memory" << Cuda::get_device(0).available_memory() << endl;
    CU_ETX(cuGLCtxCreate(&cuda_ctx, 0, Cuda::get_device(0)));

    Cuda_reduce_module reduce_mod_tmp;
    reduce_mod = &reduce_mod_tmp;


    if (GLEW_OK != err)
    {
       /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
       exit(1);
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


    cout << "max renderbuffer size: " << GL_MAX_RENDERBUFFER_SIZE << "kb" << endl;
    atexit(cleanup);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,0,6, 0,0,0, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)


    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration


    // create a texture object
    // TODO: disable mipmap
    Texture tex;
    tex.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
    tex.allocate(GL_RGBA8, WIDTH, HEIGHT);
    tex.unbind();

    
    // create fbo
    Framebuffer_object static_fbo;
    fbo = &static_fbo;
    
    // depth buffer
    Renderbuffer depth_buffer;
    depth_buffer.allocate(GL_DEPTH_COMPONENT, WIDTH, HEIGHT);

    // colorbuffer
    Renderbuffer color_buffer;
    color_buffer.allocate(GL_R32F, WIDTH, HEIGHT);

    // attach texture to color part of fbo
    fbo->attach_depth(depth_buffer);
    fbo->attach_color(color_buffer);

    fbo->check();

    //@ disable color buffer if you don't attach any color buffer image,
    //@ for example, rendering depth buffer only to a texture.
    //@ Otherwise, glCheckFramebufferStatus will not be complete.
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);

    // check FBO status
    if(GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        exit(1);
    }


    // create pbo
    opengl::Buffer static_pbo;
    pbo = &static_pbo;
    const int NUM_CHANNELS = 1;
    const int DATA_SIZE = WIDTH * HEIGHT * NUM_CHANNELS * sizeof(GLfloat);
    pbo->allocate(GL_PIXEL_PACK_BUFFER, DATA_SIZE, GL_STREAM_READ);


//     register with glut
    CU_ETX(cuGraphicsGLRegisterImage(&cuda_rbo_handle,
            color_buffer,
            GL_RENDERBUFFER,
            CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE));
//        CU_ETX(cuGraphicsGLRegisterBuffer(&cuda_pbo_handle,
//                *pbo,
//                CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE));

        clock_t begin = clock();
        for(int i = 0; i < 1000; i++)
        {
            display();
        }
        clock_t end = clock();
        cout << "elapsed: " << ((float) end - begin) / CLOCKS_PER_SEC << endl;
        exit(0);
//        glutMainLoop();
    }
    catch(Opengl_error& err)
//    catch(Cuda_error& err)
    {
        cout << err.get_msg() << endl;
        throw;
    }


    return 0;

}

#endif
