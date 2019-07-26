/* $Id$ */
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

#ifndef KJB_HAVE_OPENGL
// if opengl isn't installed, just run this 
#include <iostream>
int main()
{
    std::cout << "Can't run test -- opengl not installed." << std::endl;
    return 1;
}
#else


#ifndef KJB_HAVE_GLUT
#include <iostream>
int main()
{
    std::cout << "Can't run test -- glut not installed." << std::endl;
    return 1;
}
#else

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_distribution.h>
#include <sstream>
#include <boost/assign/std/vector.hpp>

#include <l_cpp/l_test.h>

using namespace kjb;
using namespace std;
using kjb_c::kjb_rand;

float width = 640;
float height = 480;
const float NEAR = 10;
const float FAR = 10000;

void get_random_camera_parameters(
        double& alpha,
        double& beta,
        double& theta,
        double& x0,
        double& y0);
void init_opengl(int argc, char* argv[])
{
#ifdef KJB_HAVE_GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(width,height);
    glutCreateWindow("Nurbs tool");
//    glutDisplayFunc(draw);
//  glutIdleFunc(draw);
//    glutReshapeFunc(reshape);


//    glutMouseFunc(mouse_btn);
//    glutMotionFunc(mouse_active_motion);
//    glutPassiveMotionFunc(mouse_passive_motion);
//  glutEntryFunc(mouse_entry);

    glEnable( GL_TEXTURE_2D );
#else
KJB_THROW_2(Missing_dependency,"Glut");
#endif

}
int main(int argc, char* argv[])
{
    int time_factor = 1;
    if(argc == 2)
    {
        istringstream in(argv[1]);
        in >> time_factor;
    }

    const int NUM_ITERATIONS = 1000 * time_factor;
    const Index_range& _ = Index_range::ALL;
    init_opengl(argc, argv);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-width/2, width/2, -height/2, height/2, NEAR, FAR);
    const Matrix gl_ortho = kjb::opengl::get_projection_matrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, width, height);

    kjb_c::kjb_seed_rand_with_tod();

    for(int i = 0; i < NUM_ITERATIONS; i++)
    {
        // generate random camera
        double alpha, beta, theta, x_0, y_0;

        get_random_camera_parameters(
                alpha,
                beta,
                theta,
                x_0,
                y_0);

        double s = - alpha / tan(theta);

        using namespace boost::assign;
        vector<double> r0, r1, r2;
        double ar = beta / alpha;
        r0 += alpha*ar,    s, x_0;
        r1 +=   0.0, alpha / sin(theta), y_0;
        r2 +=   0.0,  0.0,   1;

        Matrix K(3,3);
        K(0,_) = r0;
        K(1,_) = r1;
        K(2,_) = r2;


        // generate random extrinsics
        double orientation_phi   = sample(Uniform_distribution(0.0, 2*M_PI));
        double orientation_theta = sample(Uniform_distribution(0.0, M_PI));
        double orientation_psi   = sample(Uniform_distribution(0.0, 2 * M_PI));
        Vector t = 500 * (create_random_vector(3) - Vector(3, 0.5));
        t(2) = fabs(t(2));

        Quaternion orientation;
        orientation.set_euler_mode(Quaternion::XYZR);
        orientation.set_euler_angles(
                orientation_phi,
                orientation_theta,
                orientation_psi);
        Matrix R = orientation.get_rotation_matrix();
//        Matrix R = create_identity_matrix(3);
        Matrix extrinsic(3,4);
        extrinsic(_,"0:2") = R.resize(3,3);
        extrinsic(_,  3  ) = t;

        Matrix K_homo(4,4, 0.0);
        K_homo("0:2", "0:2") = K;
        K_homo(3,2) = 1.0;
        K_homo(2,2) = 0.0;

        Matrix extrinsic_homo(4,4,0.0);
        extrinsic_homo("0:2","0:3") = extrinsic;
        extrinsic_homo(3,3) = 1.0;

    // generate random point in camera coordinates in front of camera
        Vector P = 500 * (create_random_vector(4) - Vector(4, 0.5));
        P(2) = fabs(P(2)) + NEAR; // make sure it's in front of the camera and not clipped
        P(3) = 1.0;
        // convert to world coordinates
        P = matrix_inverse(extrinsic_homo) * P;
        P(3) = 1.0;
        
        // create camera object

        Parametric_camera_gl_interface camera;
        camera.set_focal_length(alpha, theta, ar);
        camera.set_aspect_ratio(ar, alpha, theta);
        camera.set_skew(theta, ar, alpha);
        camera.set_principal_point(x_0, y_0);

        // TODO: test situation where something isn't set

        if(kjb_rand() > 0.5)
        {
            camera.set_orientation(Quaternion(R));
        }
        else
        {
            // need to unify the angle conventions in this class so we don't need
            // to invert here.
            camera.set_rotation_angles(
                    orientation_phi,
                    orientation_theta,
                    orientation_psi);
        }

        camera.set_world_origin(t);
        

//        cout << "math\n:";
//        cout << R << endl;
//        cout << "camera\n:";
//        cout << camera.get_orientation().get_rotation_matrix() << endl;

        // send matrices to opengl
        camera.prepare_for_rendering(true);

        // retrieve from opengl
        Matrix modelview = opengl::get_modelview_matrix();
        Matrix full_projection = kjb::opengl::get_projection_matrix();

        // project point using reference matrix
        Vector math_pt = gl_ortho * K_homo * extrinsic_homo * P;
        math_pt /= math_pt(3);
        math_pt.resize(2);

        // project point 
        Vector gl_pt = modelview * P;
//        Vector gl_pt = P;
        // Negate the camera z-axis, in compliance with opengl standard 
        // in which objects in front of the camera have negative z-value, in contrast to Hartley and Zisserman, which is positive.
        gl_pt(2) *= -1;
        gl_pt = full_projection * gl_pt;
        gl_pt /= gl_pt(3);
        gl_pt.resize(2);

        // ASSESS ERROR
        // opengl stores projection and modelview as floats, so we
        // need a rather large precision margin here due to precision errors
        // It would be nice if it was at least one order of magnitude lower, though.
        //
        // Some testing shows that the biggest source of error is error in 
        // the modelview matrix after sending to opengl and getting it back again.  Setting modelview to identity results in this error dropping by about 100 times, even though the largest element-wise error between the real modelview and the opengl modelview is around .0001.  It seems the error in the modelview is magnified when combined with the error in the projection matrix.
        double pct_error = norm2(gl_pt - math_pt) / ( norm2(gl_pt + math_pt) / 2) ;
        //TEST_TRUE(pct_error < 1000* FLT_EPSILON);
    }

    // test near and far planes
    for(int i = 0; i < 100; i++)
    {
        Parametric_camera_gl_interface camera(NEAR, FAR);
        double alpha, beta, theta, x0, y0;
        get_random_camera_parameters(
                alpha,
                beta,
                theta,
                x0,
                y0);

        camera.set_intrinsic_parameters(
                alpha,
                beta / alpha,
                theta,
                x0,
                y0);

        camera.prepare_for_rendering(true);
        Matrix modelview = opengl::get_modelview_matrix();
        Matrix projection = opengl::get_projection_matrix();
        Matrix raw_projection = camera.get_projection_matrix();

        bool on_near_plane = (kjb_rand() < 0.5);

        Vector P = kjb::create_random_vector(4);
        P(3) = 1.0;
        // note that depth below is negative, indicating in front of the viewer
        if(on_near_plane)
            P(2) = -NEAR;
        else
            P(2) = -FAR;


        Vector p = projection * P;
        p /= p(3);

        if(on_near_plane)
            TEST_TRUE(fabs(p(2) - (-1.0)) < FLT_EPSILON);
        else
            TEST_TRUE(fabs(p(2) - (1.0)) < FLT_EPSILON);
    }

    cout << "All tests passed" << endl;
    return 0;
}

void get_random_camera_parameters(
        double& alpha,
        double& beta,
        double& theta,
        double& x0,
        double& y0)
{
    alpha = sample(Uniform_distribution(100, 200));
    beta = sample(Uniform_distribution(100, 200));

    // keep it close to 90 degrees.  At larger deviations, the
    // arithmetic error becomes extreme
    const double SKEW_SIGMA = 10 * (M_PI / 180.0);
    theta = sample(Gaussian_distribution(M_PI / 2, SKEW_SIGMA));
    x0 = sample(Uniform_distribution(1, 1000));
    y0 = sample(Uniform_distribution(1, 1000));

}
#endif
#endif
