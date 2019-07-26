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
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_entity.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
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

// modes
enum Traj_mode { GROUND_TRUTH, ESTIMATE_TRAJECTORY, REFINE_TRAJECTORY };

Traj_mode cur_mode;

// ground truth scene
const Scene* gt_scene_p;
const Scene* scene_p;
const Scene* ref_scene_p;

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

/** @brief  Helper function. */
//void clear_scene(const Scene& scene);

/** @brief  Helper function. */
void update_viewer(Scene_viewer& viewer);

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
bool render_solid;
bool render_image; 

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
//#ifdef TEST
//    kjb_c::kjb_init();
//    kjb_c::kjb_l_set("heap-checking", "off");
//    kjb_c::kjb_l_set("initialization-checking", "off");
//#endif

    try
    {
        // create (or read) scene
        size_t num_frames = 200;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data;
        Scene gt_scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        bool read_frame = false;
        const size_t num_tracks = 2;
        vector<string> frame_fps = create_or_read_scene(
                                        argc, argv, 
                                        num_frames, img_width, img_height,
                                        data, fm_data, flows_x, flows_y, gt_scene, 
                                        num_tracks, read_frame);

        // update scene without refning
        Scene scene = gt_scene;
        update_scene_state(scene, fm_data, false);

        // update scene with refning
        Scene ref_scene = gt_scene;
        update_scene_state(ref_scene, fm_data, true);

        // global pointers
        gt_scene_p = &gt_scene;
        scene_p = &scene;
        ref_scene_p = &ref_scene;

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
        render_solid = false;
        render_image = false;
        cur_mode = GROUND_TRUTH;

        // create viewer
        Scene_viewer viewer(gt_scene, img_width, img_height);
        viewer.set_flows(flows_x, flows_y);
        viewer.set_facemarks(fm_data);
        viewer.set_key_callback(bind(handle_key, boost::ref(viewer), _1, _2, _3));
        update_viewer(viewer);
        if(!frame_fps.empty())
        {
            viewer.set_frame_images(frame_fps.begin(), frame_fps.end());
            render_image = true;
        }

        // GL stuff
#ifdef KJB_HAVE_GLUT
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

        case 'G':
        {
            viewer.set_scene(*gt_scene_p);
            cur_mode = GROUND_TRUTH;
            break;
        }

        case 'E':
        {
            viewer.set_scene(*scene_p);
            cur_mode = ESTIMATE_TRAJECTORY;
            break;
        }

        case 'R':
        {
            viewer.set_scene(*ref_scene_p);
            cur_mode = REFINE_TRAJECTORY;
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

        case 'F':
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

        case 'z':
        {
            render_solid = !render_solid;
            break;
        }

        case 'w':
        {
            Image img = opengl::get_framebuffer_as_image();
            img.write("output/initialization_cpp.jpg");
            break;
        }

        case 'q':
        {
            exit(0);
        }

        default:
        {
            return;
        }
    }

    update_viewer(viewer);
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*void clear_scene(const Scene& scene)
{
    size_t num_frames = scene.association.get_data().size();
    const Target& target = *scene.association.begin();

    target.trajectory().clear();
    target.trajectory().resize(num_frames);

    target.body_trajectory().clear();
    target.body_trajectory().resize(num_frames);

    target.face_trajectory().clear();
    target.face_trajectory().resize(num_frames);

    Entity_type pet = get_entity_type("person");
    target.trajectory().height = get_entity_type_average_height(pet);
    target.trajectory().width = get_entity_type_average_width(pet);
    target.trajectory().girth = get_entity_type_average_girth(pet);
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_viewer(Scene_viewer& viewer)
{
    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();

    vector<string> info_text(2);
    info_text[0] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);

    if(cur_mode == GROUND_TRUTH)
    {
        info_text[1] = "Mode: GT";
    }
    else if(cur_mode == ESTIMATE_TRAJECTORY)
    {
        info_text[1] = "Mode: ET";
    }
    else if(cur_mode == REFINE_TRAJECTORY)
    {
        info_text[1] = "Mode: RT";
    }

    viewer.draw_images(render_image);
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

    viewer.draw_solid(render_solid);
    viewer.draw_text(true);

    viewer.set_text_lines(info_text.begin(), info_text.end());
}

