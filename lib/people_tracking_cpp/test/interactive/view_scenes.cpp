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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#include <people_tracking_cpp/pt_scene_viewer.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_association.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <i_cpp/i_image.h>
#include <l_cpp/l_iterator.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_word_list.h>
#include <l/l_sys_mal.h>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

// globals
typedef Word_list::const_iterator Word_iterator;
const_circular_iterator<Word_iterator> scene_dp_p(
                                        Word_iterator(0, NULL),
                                        Word_iterator(0, NULL));
vector<string> info_text(3);
Scene* cur_scene_p;
bool render_ground;
bool render_cylinders;
bool render_heads;
bool render_faces;
bool render_bottoms;
bool render_trails;
bool render_model_boxes;
bool render_face_boxes;
bool render_face_features;
bool render_model_vectors;
bool render_data_boxes;
bool render_flow_vectors;

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

/** @brief  Read the scene in scene_dp. */
void load_current_scene();

/** @brief  Helper function. */
void update_viewer(Scene_viewer& viewer);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
#ifdef TEST
    kjb_c::kjb_init();
    kjb_c::kjb_l_set("heap-checking", "off");
    kjb_c::kjb_l_set("initialization-checking", "off");
#endif

    if(argc != 5)
    {
        cout << "Usage: "
             << argv[0]
             << " scenes-dir"
             << " data-dir"
             << " facemarks-dir"
             << " features-dir"
             << endl;

        return EXIT_FAILURE;
    }

    try
    {
        string scenes_dp = argv[1];
        string data_dp = argv[2];
        string fm_dp = argv[3];
        string feat_dp = argv[4];

        // read data
        vector<string> x_of_fps = 
            file_names_from_format(feat_dp + "/x_int_s4/%05d.txt");
        vector<string> y_of_fps = 
            file_names_from_format(feat_dp + "/y_int_s4/%05d.txt");

        vector<Integral_flow> flows_x(x_of_fps.begin(), x_of_fps.end());
        vector<Integral_flow> flows_y(y_of_fps.begin(), y_of_fps.end());

        double img_width = flows_x.front().img_width();
        double img_height = flows_x.front().img_height();

        Box_data data(img_width, img_height, 0.99);
        data.read(file_names_from_format(data_dp + "/%05d.txt"));

        Facemark_data fm_data = parse_deva_facemarks(
                        file_names_from_format(fm_dp + "/%05d.txt"),
                        img_width,
                        img_height);

        // find scenes on disk
        Word_list scene_dps(scenes_dp + "/*", kjb_c::is_directory);
        scene_dp_p = make_const_circular_iterator(scene_dps);

        // read first scene
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        cur_scene_p = &scene;
        load_current_scene();

        // create viewer
        Scene_viewer viewer(scene, img_width, img_height);
        viewer.set_flows(flows_x, flows_y);
        viewer.set_facemarks(fm_data);
        viewer.weigh_data_box_color(true);
        viewer.draw_text(true);
        viewer.draw_3d_mode();
        viewer.set_key_callback(bind(handle_key, boost::ref(viewer), _1, _2, _3));

        // set state
        update_viewer(viewer);

        // GL stuff
        glEnable(GL_DEPTH_TEST);
#ifdef KJB_HAVE_GLUT
        glutMainLoop();
#endif
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(Scene_viewer& viewer, unsigned char key, int, int)
{
    const size_t frame_jump = 10;

    switch(key)
    {
        case 'n':
        {
            viewer.advance_frame();
            break;
        }

        case 'p':
        {
            viewer.rewind_frame();
            break;
        }

        case 'N':
        {
            viewer.advance_frame(frame_jump);
            break;
        }

        case 'P':
        {
            viewer.rewind_frame(frame_jump);
            break;
        }

        case 'j':
        {
            scene_dp_p++;
            load_current_scene();
            break;
        }

        case 'k':
        {
            scene_dp_p--;
            load_current_scene();
            break;
        }

        case '2':
        {
            viewer.draw_2d_mode();
            break;
        }

        case '3':
        {
            viewer.draw_3d_mode();
            break;
        }

        case 'w':
        {
            Image img = opengl::get_framebuffer_as_image();
            img.write("output/view_scenes_cpp.jpg");
            break;
        }

        case 'q':
        {
            exit(0);
        }

        default:
        {
            break;
        }
    }

    update_viewer(viewer);
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void load_current_scene()
{
    string scene_dp = *scene_dp_p;
    read_scene(*cur_scene_p, scene_dp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_viewer(Scene_viewer& viewer)
{
    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();
    size_t num_tracks = viewer.scene().association.size();
    string sdir = *scene_dp_p;
    sdir.insert(0, "/");
    string sname = sdir.substr(sdir.rfind('/') + 1, string::npos);

    info_text[0] = "Scene: " + sname;
    info_text[1] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);
    info_text[2] = "# tracks: " + lexical_cast<string>(num_tracks);
    viewer.set_text_lines(info_text.begin(), info_text.end());
}

