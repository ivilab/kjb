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

#define ZPR

#include <gr_cpp/gr_sprite.h>
/*
// Sometimes glu.h includes glut.h--sometimes not.
*/
#ifdef KJB_HAVE_OPENGL
#ifdef MAC_OSX
#    include <OpenGL/glu.h>
#    include <GLUT/glut.h>
#else 
	#ifdef WIN32
	#	 include <GL/glu.h>
	#	 include <glut.h>
	#else 
	#    include <GL/glu.h>       
	#    include <GL/glut.h>       
	#endif
#endif
#else
#error "not opengl"
#endif

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_iarchive.hpp>
#endif


#ifdef KJB_HAVE_BST_POPTIONS
#include <boost/program_options.hpp>
#else
#error "Need boost program options"
#endif

#include <iostream>
#include <vector>
#include <string>

#include <i_cpp/i_image.h>
#include <gr_cpp/gr_opengl.h>
#include <m_cpp/m_vector.h>
#include <g_cpp/g_quaternion.h>
#include <g_cpp/g_camera_calibration.h>

#include <boost/foreach.hpp>
#include "opengl_test.h"
#include <gr_cpp/gr_primitive.h>

#include <st_cpp/st_turntable_camera.h>

#include <fstream>


namespace po = boost::program_options;
using namespace std;
using namespace kjb;
using namespace kjb::opengl;
using namespace boost::foreach;

string PROGRAM_NAME = "test_turntable_camera";
string VERSION_STRING = "Turntable camera test tool, v0.01";


Turntable_camera t_camera;
Turntable_camera other_camera;
vector<Calibrated_camera> cameras;
vector<Gl_sprite> sprites;
vector<string> mat_fnames;
vector<string> img_fnames; // this doesn't need to be global
string camera_fname;

vector<Vector> debug_pts;
bool manual_operation = false;
bool estimated_camera = true;
int cur_view = 0;
Vector ppt_delta(2, 0.0);
Vector ppt(2, 0.0);

void process_options(int argc, char* argv[])
{
    try 
    {
        po::variables_map options;

        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "Print version string")
            ("help", "Print this help message")
            ("config-file,I", po::value<string>(), "A configuration file to read options from.")
            ;

        
        po::options_description config("Configuration");
        config.add_options()
            ("calib-files,m", po::value<vector<string> >(&mat_fnames)->multitoken(), "A list of roughly-calibrated camera matrices.")
            ("camera-file,c", po::value<string>(&camera_fname), "A saved turntable camera file")
            ("img-files,i", po::value<vector<string> >(&img_fnames)->multitoken(), "A list of calibration images.")
                ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config);

        po::options_description config_file_options;
        config_file_options.add(config);

        po::options_description visible;
        visible.add(generic).add(config);

        store(po::command_line_parser(argc, argv)
                .options(cmdline_options)
//                .positional(p)
                .run(),
            options);

        notify(options);

        if(options.count("help"))
        {
            cout << "Usage: " + PROGRAM_NAME + " -I input.conf" << "\n";
            cout << visible << "\n";
            exit(0);
        }

        if(options.count("version"))
        {
            cout << VERSION_STRING << "\n";
            exit(0);
        }

        // READ CONFIG FILE
        string config_fname;
        if(options.count("config-file"))
            config_fname = options["config-file"].as<string>();
        else
        {
            if(argc == 1)
                config_fname = "cameras.conf";
        }

        if(!config_fname.empty())
        {
            ifstream ifs(config_fname.c_str());
            if(ifs.fail())
            {
                cerr << "Error opening config file." << endl;
                exit(1);
            }

            store(parse_config_file(ifs, config_file_options), options);
            notify(options);
        }



        if(!options.count("calib-files") && !options.count("camera-file"))
        {
            cerr << "Must specify either calib-files or camera-file.\n";
            exit(1);
        }

        if(options.count("calib-files") && options.count("camera-file"))
        {
            cerr << "Must specify either calib-files or camera-file, but not both.\n";
            exit(1);
        }

        if(!options.count("img-files"))
        {
            cerr << "img-files not specified.\n";
            exit(1);
        }

        sprites = vector<Gl_sprite>(img_fnames.begin(), img_fnames.end());

        if(!options.count("camera-file") && mat_fnames.size()  != sprites.size())
        {
            cerr << "calib-files and img-files must have same count.\n";
            exit(1);

        }
    }
    catch(exception& e)
    {
        cout << e.what() << "\n";
        exit(1);
    }    

}

void draw_direction_vector(const Vector& v)
{
    glVertex3f(0.0,0.0,0.0);
    glVertex(v);
}

void render_direction_vectors(const vector<Vector>& vectors)
{
    glBegin(GL_LINES);
    std::for_each(vectors.end(), vectors.end(), draw_direction_vector);
    glEnd();
}

void render_point(const Vector& v)
{
//    glBegin(GL_POINTS);
//    glVertex(v);
//    glEnd();

    glPushMatrix();
    glTranslate(v);
    kjb::opengl::Sphere(10.0).render();
    glPopMatrix();
}
void render_points(const vector<Vector>& pts)
{
    std::for_each(pts.begin(), pts.end(), render_point);
}

void draw_cube(float size)
{
    // display 3d cube over image
    glBegin(GL_QUADS);
        glColor4f(1.0, 0.0, 0.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(0.,size, 0.);
        glVertex3f(size,size, 0.);
        glVertex3f(size, 0., 0.);

        glColor4f(0.0, 1.0, 0.0, 0.8);
        glVertex3f(0., 0., size);
        glVertex3f(size, 0., size);
        glVertex3f(size,size, size);
        glVertex3f(0.,size, size);

        glColor4f(0.0, 0.0, 1.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(0., 0.,size);
        glVertex3f(0., size,size);
        glVertex3f(0., size, 0.);

        glColor4f(1.0, 1.0, 0.0, 0.8);
        glVertex3f(size, 0., 0.);
        glVertex3f(size, size, 0.);
        glVertex3f(size, size,size);
        glVertex3f(size, 0.,size);

        glColor4f(0.0, 1.0, 1.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(size, 0., 0.);
        glVertex3f(size, 0., size);
        glVertex3f(0., 0., size);

        glColor4f(1.0, 0.0, 1.0, 0.8);
        glVertex3f(0., size, 0.);
        glVertex3f(0., size, size);
        glVertex3f(size, size, size);
        glVertex3f(size, size, 0.);
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
        glVertex3f(0,0,0);
        glVertex3f(0,size,0);
        glVertex3f(size,size,0);
        glVertex3f(size,0,0);
        glVertex3f(0,0,0);
    glEnd();

    glBegin(GL_LINE_STRIP);
        glVertex3f(0,0,size);
        glVertex3f(0,size,size);
        glVertex3f(size,size,size);
        glVertex3f(size,0,size);
        glVertex3f(0,0,size);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(0,0,0);
        glVertex3f(0,0,size);
        glVertex3f(size,0,0);
        glVertex3f(size,0,size);
        glVertex3f(size,size,0);
        glVertex3f(size,size,size);
        glVertex3f(0,size,0);
        glVertex3f(0,size,size);
    glEnd();

}
void draw()
{
    // could kill the next two lines by initializing at start. but hey, its test code
    sprites[cur_view].set_position(0,0);
    sprites[cur_view].set_scale(1,1);
    sprites[cur_view].render();

    t_camera.set_principal_point(ppt + ppt_delta);
    t_camera.set_active_camera(cur_view);

    t_camera.pass_to_opengl();
//    cameras[cur_view].prepare_for_rendering();
    draw_cube(160);
//    render_direction_vectors(debug_pts);
}

void increment_camera()
{
    cur_view = (cur_view + 1) % 36;
    t_camera.set_active_camera(cur_view);
    other_camera.set_active_camera(cur_view);
}

void decrement_camera()
{
    cur_view = (cur_view + 36-1) % 36;
    t_camera.set_active_camera(cur_view);
    other_camera.set_active_camera(cur_view);
}

void key(unsigned int _key)
{
    switch(_key)
    {
        case 'n':
            increment_camera();
            manual_operation = true;
            glutPostRedisplay();
            break;
        case 'p':
            decrement_camera();
            manual_operation = true;
            glutPostRedisplay();
            break;
        case 'o':
            cout << opengl::get_projection_matrix();
            cout << opengl::get_modelview_matrix();
            manual_operation = true;
            break;
        case 'g':
            manual_operation = false;
            break;
        case 't':
            // toggle hand-picked vs estimated camera
            t_camera.swap(other_camera);
            estimated_camera = !estimated_camera;
            glutPostRedisplay();
            break;
        case '+':
            ppt_delta += Vector(1.0, 0.0);
            glutPostRedisplay();
            break;
        case '-':
            ppt_delta -= Vector(1.0, 0.0);
            glutPostRedisplay();
            break;
        case '[':
            ppt_delta -= Vector(0.0, 1.0);
            glutPostRedisplay();
            break;
        case ']':
            ppt_delta += Vector(0.0, 1.0);
            glutPostRedisplay();
            break;
        case 'q':
            exit(0);
            break;
    }
}



void idle()
{
    static int i = 0;

    if(manual_operation)
    {
        return;
    }

    if(i > 36000)
    {

        increment_camera();
        glutPostRedisplay();
        i = 0;
    }
    i++;
}

void cleanup()
{
    cout << "Cleanup called.\n";
//    axes.clear();
    sprites.clear();
    t_camera = Turntable_camera();
    other_camera = Turntable_camera();
    cameras.clear();
    debug_pts.clear();
}

int main(int argc, char* argv[])
{
    kjb_c::kjb_l_set("page", "off");
//    try 
    {
        const int WIDTH = 530;
        const int HEIGHT = 397;

        width = WIDTH;
        height = HEIGHT;
        init(argc, argv);

        process_options(argc, argv);


        Calibration_descriptor cal;
            cal.image_width = WIDTH;
            cal.image_height = HEIGHT;
            cal.convention = Calibration_descriptor::TOP_LEFT;
            cal.world_origin_in_front = true;

        Calibration_descriptor other_cal = cal;
        other_cal.convention = Calibration_descriptor::BOTTOM_LEFT;

        // could do this in process options
        if(mat_fnames.size())
        {
            cameras = vector<Calibrated_camera>(mat_fnames.size());
            for(int i = 0; i < mat_fnames.size(); i++)
            {
                // a few were calibrated differently
    //            if( i == 0 || i == 1 || i == 21)
    //                cameras[i] = Calibrated_camera(mat_fnames[i], other_cal);
    //            else
                    cameras[i] = Calibrated_camera(mat_fnames[i], cal);
            }

            t_camera = regularize_turntable_cameras(cameras);
            t_camera.set_active_camera(cur_view);
        }
        else
        {
            // turntable camera file was specified
#ifdef KJB_HAVE_BST_SERIAL
            try{
                ifstream ifs(camera_fname.c_str());
                boost::archive::text_iarchive ia(ifs);
                ia >> t_camera;
            }
            catch(Exception& e)
            {
                cerr << e.get_msg() << endl;
                exit(1);
            }
            catch(...)
            {
                cerr << "Unknown error occurred" << endl;
                exit(1);
            }
#else
            cerr << "Cannot read camera file, because program was compiled with boost::serialization installed.\n"
            exit(1);
#endif
        }
        ppt = t_camera.get_principal_point();
        Perspective_camera perfect_camera = t_camera.get_neutral_camera();

        // set up hand-tuned turntable camera, using obvious
        // default values like zero skew, square pixels, 
        // and known turntable coordinate system.
        // (Toggle using 't').
        perfect_camera.set_skew(-M_PI/2);
        perfect_camera.set_aspect_ratio(-1.0);
//        perfect_camera.set_principal_point(Vector(2, 0.0));

        other_camera = Turntable_camera(
                perfect_camera,
                -Vector(0.0,0.0,1.0),
                Vector(80., 80., 0.0),
                36, false);
                
//        debug_pts = debug_tt_camera(cameras);




        glDisable(GL_LIGHTING);

    }
        run();
//    catch(const kjb::Exception& ex)
//    {
//        cerr << ex.get_msg() << endl;
//        exit(1);
//    }

    return 0;
}
