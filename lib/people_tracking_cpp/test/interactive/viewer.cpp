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
#include <people_tracking_cpp/pt_association.h>
#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <gr_cpp/gr_opengl_headers.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_iterator.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_mal.h>
#include <string>
#include <vector>
#include "utils.h"
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

// state
typedef Box_data::Box_set::const_iterator Data_citerator;
typedef Ascn::const_iterator Ass_citerator;
const_circular_iterator<Data_citerator> active_databox_p;
const_circular_iterator<Ass_citerator> active_target_p;
vector<string> info_text(3);
bool render_ground;
bool render_cylinders;
bool render_bodies;
bool render_heads;
bool render_faces;
bool render_bottoms;
bool render_trails;
bool render_full_boxes;
bool render_body_boxes;
bool render_face_boxes;
bool render_face_features;
bool render_model_vectors;
bool render_face_vectors;
bool render_data_boxes;
bool render_facemarks;
bool render_flow_vectors;
bool render_flow_face_vectors;
bool render_text;
bool render_solid;
bool overhead_mode;

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

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

    try
    {
        // create (or read) scene
        size_t num_frames = 500;
        double img_width = 1200;
        double img_height = 800;
        const size_t max_tracks = 0;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        vector<string> frames_fps = create_or_read_scene(
                                            argc, argv, num_frames, 
                                            img_width, img_height,
                                            data, fm_data, 
                                            flows_x, flows_y, scene, 
                                            max_tracks, true);

        // create viewer
        Scene_viewer viewer(scene, img_width, img_height);
        viewer.set_flows(flows_x, flows_y);
        viewer.set_facemarks(fm_data);
        viewer.weigh_data_box_color(true);
        viewer.set_key_callback(bind(handle_key, boost::ref(viewer), _1, _2, _3));

        // active stuff
        active_databox_p = make_const_circular_iterator(
            scene.association.get_data()[viewer.frame() - 1]);
        active_target_p = make_const_circular_iterator(scene.association);

        // set state
        render_ground = true;
        render_cylinders = false;
        render_bodies = true;
        render_heads = true;
        render_faces = true;
        render_bottoms = true;
        render_trails = true;
        render_full_boxes = false;
        render_body_boxes = false;
        render_face_boxes = false;
        render_face_features = false;
        render_model_vectors = false;
        render_face_vectors = false;
        render_data_boxes = false;
        render_facemarks = false;
        render_flow_vectors = false;
        render_flow_face_vectors = false;
        render_text = true;
        render_solid = false;
        overhead_mode = false;

        // set frames 
        if(!frames_fps.empty())
        {
            viewer.set_frame_images(frames_fps.begin(), frames_fps.end());
            viewer.draw_images(true);
        }
        update_viewer(viewer);

        // GL stuff
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
    catch(const std::exception& sex)
    {
        cerr << sex.what() << endl;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(Scene_viewer& viewer, unsigned char key, int, int)
{
    const size_t frame_jump = 100;
    const Scene& scene = viewer.scene();

    switch(key)
    {
        case 'n':
        {
            viewer.advance_frame();
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            break;
        }

        case 'p':
        {
            viewer.rewind_frame();
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            break;
        }

        case 'N':
        {
            viewer.advance_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            break;
        }

        case 'P':
        {
            viewer.rewind_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            break;
        }

        case 'j':
        {
            active_target_p++;
            break;
        }

        case 'k':
        {
            active_target_p--;
            break;
        }

        case 'l':
        {
            if(!scene.association.get_data()[viewer.frame() - 1].empty())
            {
                active_databox_p++;
            }
            break;
        }

        case 'h':
        {
            if(!scene.association.get_data()[viewer.frame() - 1].empty())
            {
                active_databox_p--;
            }
            break;
        }

        case 'g':
        {
            render_ground = !render_ground;
            break;
        }

        case 'c':
        {
            render_cylinders = !render_cylinders;
            break;
        }

        case 'b':
        {
            render_bodies = !render_bodies;
            break;
        }

        case 'e':
        {
            render_heads = !render_heads;
            break;
        }

        case 'f':
        {
            render_faces = !render_faces;
            break;
        }

        case 'o':
        {
            render_bottoms = !render_bottoms;
            break;
        }

        case 't':
        {
            render_trails = !render_trails;
            break;
        }

        case 'u':
        {
            render_full_boxes = !render_full_boxes;
            break;
        }

        case 'd':
        {
            render_body_boxes = !render_body_boxes;
            break;
        }

        case 'a':
        {
            render_face_boxes = !render_face_boxes;
            break;
        }

        case 'r':
        {
            render_face_features = !render_face_features;
            break;
        }

        case 'v':
        {
            render_model_vectors = !render_model_vectors;
            render_face_vectors = !render_face_vectors;
            break;
        }

        case 'y':
        {
            render_data_boxes = !render_data_boxes;
            break;
        }

        case 'm':
        {
            render_facemarks = !render_facemarks;
            break;
        }

        case 'w':
        {
            render_flow_vectors = !render_flow_vectors;
            render_flow_face_vectors = !render_flow_face_vectors;
            break;
        }

        case '2':
        {
            render_ground = false;
            render_cylinders = false;
            render_bodies = false;
            render_heads = false;
            render_faces = false;
            render_bottoms = false;
            render_trails = false;
            render_full_boxes = false;
            render_body_boxes = true;
            render_face_boxes = true;
            render_face_features = true;
            render_model_vectors = true;
            render_face_vectors = true;
            render_data_boxes = true;
            render_facemarks = true;
            render_flow_vectors = true;
            render_flow_face_vectors = true;
            break;
        }

        case '3':
        {
            render_ground = true;
            render_cylinders = false;
            render_bodies = true;
            render_heads = true;
            render_faces = true;
            render_bottoms = true;
            render_trails = true;
            render_full_boxes = false;
            render_body_boxes = false;
            render_face_boxes = false;
            render_face_features = false;
            render_model_vectors = false;
            render_face_vectors = false;
            render_data_boxes = false;
            render_facemarks = false;
            render_flow_vectors = false;
            render_flow_face_vectors = false;
            break;
        }

        case 'x':
        {
            render_text = !render_text;
            break;
        }

        case 'z':
        {
            render_solid = !render_solid;
            break;
        }

        case 'i':
        {
            overhead_mode = !overhead_mode;
            break;
        }

        case 's':
        {
            Image img = opengl::get_framebuffer_as_image();
            img.write("output/viewer_cpp.jpg");
            break;
        }

        case 'D':
        {
            viewer.draw_images(true);
            break;
        }
        
        case '-':
        {
            viewer.draw_images(false);
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

void update_viewer(Scene_viewer& viewer)
{
    // set what to display
    if(overhead_mode)
    {
        viewer.set_overhead_view();
    }
    else
    {
        viewer.clear_alternative_camera();

        //viewer.draw_images(true);
        viewer.draw_ground(render_ground);
        viewer.draw_cylinders(render_cylinders);
        viewer.draw_bodies(render_bodies);
        viewer.draw_heads(render_heads);
        viewer.draw_faces(render_faces);
        viewer.draw_bottoms(render_bottoms);
        viewer.draw_trails(render_trails);
        viewer.draw_full_boxes(render_full_boxes);
        viewer.draw_body_boxes(render_body_boxes);
        viewer.draw_face_boxes(render_face_boxes);
        viewer.draw_face_features(render_face_features);
        viewer.draw_model_vectors(render_model_vectors);
        viewer.draw_face_vectors(render_face_vectors);
        viewer.draw_data_boxes(render_data_boxes);
        viewer.draw_facemarks(render_facemarks);
        viewer.draw_flow_vectors(render_flow_vectors);
        viewer.draw_flow_face_vectors(render_flow_face_vectors);
    }

    viewer.draw_solid(render_solid);
    viewer.draw_text(render_text);

    // set colors
    if(!viewer.scene().association.get_data()[viewer.frame() - 1].empty())
    {
        viewer.set_data_box_style(
                            *active_databox_p,
                            Vector(0.0, 1.0, 0.0),
                            2.0, true);
    }
    viewer.set_target_style(*active_target_p, Vector(1.0, 0.0, 0.0), 2.0, true);

    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();
    size_t num_tracks = viewer.scene().association.size();
    size_t tr = distance(viewer.scene().association.begin(),
                         active_target_p.get_iterator());
    size_t num_boxes = viewer.scene().association.get_data()[fr - 1].size();
    size_t bx = distance(viewer.scene().association.get_data()[fr - 1].begin(),
                         active_databox_p.get_iterator());

    info_text[0] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);
    info_text[1] = "Track: " + lexical_cast<string>(tr + 1) + "/"
                             + lexical_cast<string>(num_tracks);
    info_text[2] = "Box: " + lexical_cast<string>(bx + 1) + "/"
                           + lexical_cast<string>(num_boxes);
    viewer.set_text_lines(info_text.begin(), info_text.end());
}

