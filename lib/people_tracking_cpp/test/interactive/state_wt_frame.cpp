/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Ernesto Brau, Jinyan Guan
|
* =========================================================================== */

/* $Id: state.cpp 20375 2016-02-15 20:33:07Z jguan1 $ */

#include <i_cpp/i_image.h>
#include <l_cpp/l_serialization.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <people_tracking_cpp/pt_face_2d.h>
#include <people_tracking_cpp/pt_scene_viewer.h>
#include <detector_cpp/d_bbox.h>
#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_sprite.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <gr_cpp/gr_primitive.h>
#include <g_cpp/g_quaternion.h>
#include <iostream>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;

const double win_width = 500;
const double win_height = 500;

// camera and image path
std::string camera_fp;
std::string image_fp;

// 3D state
Perspective_camera camera;
Complete_state cs;
Complete_state pcs;
double height;
double width;
double girth;
Image frame_img;

// program state
bool mode_3d = true;

/** @brief  Display scene. */
void display();

/** @brief  Setup GL for 2D rendering. */
void prepare_for_rendering_2d();

/** @brief  Reshape scene. */
void reshape(int, int);

/** @brief  Handle key input. */
void handle_key(unsigned char, int, int);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cout << "Usuage: " << argv[0] << " camera-fp image-fp\n";
        return EXIT_SUCCESS;
    }
    else
    {
        camera_fp = argv[1];
        image_fp = argv[2];
        frame_img = Image(image_fp);
    }
    try
    {
        //camera.set_camera_centre(Vector(0.0, 2.0, 0.0));
        //camera.set_focal_length(3*win_width / 2.0);
        //camera.set_pitch(M_PI/15.0);
        load(camera, camera_fp);

        // initialize state
        double cx = camera.get_camera_centre_x();
        double cy = camera.get_camera_centre_y();
        double cz = camera.get_camera_centre_z();
        double yaw = camera.get_yaw();
        double pitch = camera.get_pitch();
        std::cout << "camera location: " << cx << " " << cy << " " << cz << std::endl;
        std::cout << "camera orientation: " << yaw << " " << pitch << std::endl;
        cs.position[0] = -1.4;
        cs.position[1] = 0.0;
        cs.position[2] = -6.0;
        cs.body_dir = 0.0;
        cs.face_dir[0] = 0.0;
        cs.face_dir[1] = 0.0;

        pcs = cs;

        height = 1.75;
        width = 0.5;
        girth = 0.3;


        // GL stuff
#ifdef KJB_HAVE_GLUT

        size_t w = frame_img.get_num_cols();
        size_t h = frame_img.get_num_rows();
        Glut_window win(w, h);

        win.set_display_callback(display);
        win.set_reshape_callback(reshape);
        win.set_keyboard_callback(handle_key);

        glutMainLoop();
#endif
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display()
{
    // clear stuff
    glClearColor(0.7, 0.7, 0.7, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    //glEnable(GL_DEPTH_TEST);
    // render image
    Sprite sprite(frame_img, 0);
    double new_width = get_viewport_width();
    double new_height = get_viewport_height();
    sprite.set_position(new_width/2, new_height/2);
    sprite.center_origin();
    sprite.render();

    // colors
    Vector color_3d(0.0, 0.0, 1.0);
    Vector color_2d(1.0, 0.0, 0.0);
    Vector color_gd(1.0, 1.0, 1.0);
    Vector color_fc(0.0, 1.0, 0.0);
    Vector color_md(1.0, 0.0, 1.0);

    if(mode_3d)
    {
        //// DISPLAY 3D WORLD
        camera.prepare_for_rendering(false);
        glEnable(GL_DEPTH_TEST);

        // draw ground
        glColor(color_gd);
        render_xz_plane_grid(-20.0, 20.0, -100.0, 0.0, 2);

        Head head(cs, height, width, girth);
        glColor(color_3d);

        // draw body
        Cylinder cyl(width / 2.0, width / 2.0, height - head.height);
        glLineWidth(1.0);
        glPushMatrix();
            glTranslate(cs.position);
            glRotated(180*cs.body_dir/M_PI, 0.0, 1.0, 0.0);
            glScaled(1.0, 1.0, girth / width);
            glRotated(-90.0, 1.0, 0.0, 0.0);
            cyl.wire_render();
        glPopMatrix();

        // draw head
        Sphere sph(1/2.0);
        glLineWidth(1.0);
        glPushMatrix();
            glTranslate(head.center);
            glRotate(head.orientation);
            glScaled(head.width, head.height, head.girth);
            sph.wire_render();
        glPopMatrix();

        // draw directions
        Quaternion baq(0.0, cs.body_dir, 0.0, Quaternion::XYZS);
        Vector bart = baq.rotate(Vector(0.0, 0.0, 1.5*girth));
        Arrow3d barr(
                Vector(cs.position[0], height/2, cs.position[2]),
                bart,
                height/24);
        barr.render();

        //Vector fart = head.orientation.rotate(Vector(0.0, 0.0, head.girth/2));
        Vector fart = head.orientation * Vector(0.0, 0.0, head.girth/2);
        Arrow3d farr(
                head_to_world(Vector(0.0, 0.0, 1.0), head, true),
                fart,
                head.height/12);
        farr.render();

        // draw face
        Vector_vec ffts = face_features(cs, height, width, girth);
        Ellipse els(head.height/20, head.height/20);
        glColor(color_fc);
        glLineWidth(1.0);
        for(size_t i = 0; i < ffts.size(); i++)
        {
            glPushMatrix();
                glTranslate(ffts[i]);
                glRotate(head.orientation);
                els.solid_render();
            glPopMatrix();
        }
    }
    else
    {
        //// DISPLAY 2D STUFF
        prepare_for_rendering_2d();
        glDisable(GL_DEPTH_TEST);
        glColor(color_2d);

        // body and face
        Body_2d body = project_cstate(cs, camera, height, width, girth);
        Face_2d face = project_cstate_face(cs, camera, height, width, girth);

        // draw full box
        const Bbox& cbox = body.full_bbox;
        glLineWidth(3.0);
        cbox.wire_render();

        // draw body box
        const Bbox& bbox = body.body_bbox;
        glLineWidth(3.0);
        bbox.wire_render();

        // draw face box
        const Bbox& fbox = face.bbox;
        glLineWidth(3.0);
        fbox.wire_render();

        // draw face features
        Ellipse els(3.0, 3.0);

        // eyes
        if(!face.left_eye.empty())
        {
            glLineWidth(1.0);
            glPushMatrix();
                glTranslate(face.left_eye);
                els.solid_render();
            glPopMatrix();
        }

        if(!face.right_eye.empty())
        {
            glLineWidth(1.0);
            glPushMatrix();
                glTranslate(face.right_eye);
                els.solid_render();
            glPopMatrix();
        }

        // nose
        if(!face.nose.empty())
        {
            glLineWidth(1.0);
            glPushMatrix();
                glTranslate(face.nose);
                els.solid_render();
            glPopMatrix();
        }

        // mouf
        if(!face.left_mouth.empty())
        {
            glLineWidth(1.0);
            glPushMatrix();
                glTranslate(face.left_mouth);
                els.solid_render();
            glPopMatrix();
        }

        if(!face.right_mouth.empty())
        {
            glLineWidth(1.0);
            glPushMatrix();
                glTranslate(face.right_mouth);
                els.solid_render();
            glPopMatrix();
        }

        // draw face model dir
        glColor(color_md);
        glLineWidth(1.0);
        face.model_dir = face_model_direction(pcs, cs, camera,
                                              height, width, girth);
        render_arrow(fbox.get_center(),
                     fbox.get_center() + 20*face.model_dir);

        // draw body model dir
        glColor(color_md);
        glLineWidth(1.0);
        body.model_dir = model_direction(pcs, cs, camera,
                                         height, width, girth);
        render_arrow(bbox.get_center(),
                     bbox.get_center() + 20*body.model_dir);
    }
#ifdef KJB_HAVE_GLUT
    glutSwapBuffers();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_MODELVIEW);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void prepare_for_rendering_2d()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-win_width / 2.0, win_width / 2.0,
            -win_height / 2.0, win_height / 2.0,
            -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(unsigned char key, int, int)
{
    pcs = cs;
    switch(key)
    {
        case 'h':
        {
            height -= 0.02;
            if(height <= 0.0)
            {
                height = 0.05;
            }

            break;
        }

        case 'H':
        {
            height += 0.02;
            break;
        }

        case 'w':
        {
            width -= 0.05;
            if(width <= 0.0)
            {
                width = 0.05;
            }

            break;
        }

        case 'W':
        {
            width += 0.05;
            break;
        }

        case 'g':
        {
            girth -= 0.05;
            if(girth <= 0.0)
            {
                girth = 0.05;
            }

            break;
        }

        case 'G':
        {
            girth += 0.05;
            break;
        }

        case 'x':
        {
            cs.position[0] -= 0.05;
            break;
        }

        case 'X':
        {
            cs.position[0] += 0.05;
            break;
        }

        case 'z':
        {
            cs.position[2] -= 0.05;
            break;
        }

        case 'Z':
        {
            cs.position[2] += 0.05;
            break;
        }

        case 'r':
        {
            cs.body_dir -= M_PI / 48.0;
            if(cs.body_dir < -M_PI) cs.body_dir += 2*M_PI;
            break;
        }

        case 'R':
        {
            cs.body_dir += M_PI / 48.0;
            if(cs.body_dir > M_PI) cs.body_dir -= 2*M_PI;
            break;
        }

        case 't':
        {
            cs.face_dir[0] -= M_PI / 48.0;
            if(cs.face_dir[0] < -M_PI) cs.face_dir[0] += 2*M_PI;
            break;
        }

        case 'T':
        {
            cs.face_dir[0] += M_PI / 48.0;
            if(cs.face_dir[0] > M_PI) cs.face_dir[0] -= 2*M_PI;
            break;
        }

        case 'e':
        {
            cs.face_dir[1] -= M_PI / 48.0;
            if(cs.face_dir[1] < -M_PI/2) cs.face_dir[1] = -M_PI/2;
            std::cout << "face pitch: " << cs.face_dir[1] << std::endl;
            break;
        }

        case 'E':
        {
            cs.face_dir[1] += M_PI / 48.0;
            if(cs.face_dir[1] > M_PI/2) cs.face_dir[1] = M_PI/2;
            break;
        }

        case 'k':
        {
            cs.position[0] = 0.0;
            cs.position[1] = 0.0;
            cs.position[2] = -5.0;
            cs.body_dir = 0.0;
            cs.face_dir[0] = 0.0;
            cs.face_dir[1] = 0.0;

            height = 1.75;
            width = 0.5;
            girth = 0.3;

            break;
        }

        case 'p':
        {
            camera.set_pitch(camera.get_pitch() + M_PI/12);
            break;
        }

        case 'P':
        {
            camera.set_pitch(camera.get_pitch() - M_PI/12);
            break;
        }

        case 'm':
        {
            mode_3d = !mode_3d;
            break;
        }

        case 'q':
        case 'Q':
        {
            exit(EXIT_SUCCESS);
        }
    }
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}

