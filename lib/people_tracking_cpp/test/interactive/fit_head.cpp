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
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_sample_scenes.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <m_cpp/m_vector.h>
#include <i_cpp/i_image.h>
#include <camera_cpp/perspective_camera.h>
#include <camera_cpp/camera_backproject.h>
#include <l_cpp/l_iterator.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_serialization.h>
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
typedef Facemark_data::value_type::const_iterator Fm_citerator;
const Facemark_data* fm_data_p;
const_circular_iterator<Data_citerator> active_databox_p;
const_circular_iterator<Fm_citerator> active_fmark_p;
vector<string> info_text(3);
double img_width;
double img_height;

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

/** @brief  Helper function. */
void update_viewer(Scene_viewer& viewer);

/** @brief  Clear scene and add single-frame target. */
void make_scene(Scene& scene, size_t t);

/** @brief  Fit a head. */
void max_head(const Scene& scene);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cerr << "Usage: fit_head DATA_PATH" << endl;
        return EXIT_FAILURE;
    }

    try
    {
        // create (or read) scene
        string data_dp = argv[1];
        string frames_dp = data_dp + "/frames";
        string boxes_dp = data_dp + "/detection_boxes/person_inria";
        string fmarks_dp = data_dp + "/features/deva_face";
        string cam_fp = data_dp + "/camera/camera.txt";

        // read images
        vector<string> frames_fps = file_names_from_format(frames_dp + "/%05d.jpg");
        //size_t num_frames = frames_fps.size();
        img_width = Image(frames_fps[0]).get_num_cols();
        img_height = Image(frames_fps[0]).get_num_rows();

        // read data and camera
        Box_data data(img_width, img_height, 0.98);
        data.read(file_names_from_format(boxes_dp + "/%05d.txt"));

        Facemark_data fm_data = parse_deva_facemarks(
            file_names_from_format(fmarks_dp + "/%05d.txt"),
            img_width,
            img_height);
        fm_data_p = &fm_data;

        Perspective_camera camera;
        load(camera, cam_fp);

        // create scene
        Scene scene(Ascn(data), camera, 0.0, 0.0, 0.0);

        // create viewer
        Scene_viewer viewer(scene, 0.75*img_width, 0.75*img_height);
        viewer.set_facemarks(fm_data);
        viewer.set_frame_images(frames_fps.begin(), frames_fps.end());
        viewer.draw_ground(false);
        viewer.draw_cylinders(false);
        viewer.draw_bodies(true);
        viewer.draw_bottoms(false);
        viewer.draw_trails(false);
        viewer.draw_data_boxes(true);
        viewer.draw_facemarks(true);
        viewer.draw_images(true);
        viewer.draw_text(true);
        viewer.set_key_callback(bind(handle_key, ref(viewer), _1, _2, _3));

        // active stuff
        active_databox_p = make_const_circular_iterator(
            scene.association.get_data()[viewer.frame() - 1]);
        active_fmark_p = make_const_circular_iterator(
            fm_data[viewer.frame() - 1]);

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
    Scene& scene = const_cast<Scene&>(viewer.scene());

    switch(key)
    {
        case 'n':
        {
            viewer.advance_frame();
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            active_fmark_p = make_const_circular_iterator(
                (*fm_data_p)[viewer.frame() - 1]);
            break;
        }

        case 'p':
        {
            viewer.rewind_frame();
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            active_fmark_p = make_const_circular_iterator(
                (*fm_data_p)[viewer.frame() - 1]);
            break;
        }

        case 'N':
        {
            viewer.advance_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            active_fmark_p = make_const_circular_iterator(
                (*fm_data_p)[viewer.frame() - 1]);
            break;
        }

        case 'P':
        {
            viewer.rewind_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            active_fmark_p = make_const_circular_iterator(
                (*fm_data_p)[viewer.frame() - 1]);
            break;
        }

        case 'j':
        {
            if(!(*fm_data_p)[viewer.frame() - 1].empty())
            {
                active_fmark_p++;
            }
            break;
        }

        case 'k':
        {
            if(!(*fm_data_p)[viewer.frame() - 1].empty())
            {
                active_fmark_p--;
            }
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

        case 'r':
        {
            if(!scene.association.get_data()[viewer.frame() - 1].empty()
                    && !(*fm_data_p)[viewer.frame() - 1].empty())
            {
                make_scene(scene, viewer.frame());
                //max_head(scene);
                fit_head(
                    *scene.association.begin(),
                    viewer.frame(),
                    *active_fmark_p,
                    scene.camera,
                    img_width,
                    img_height,
                    5.0);
            }
            break;
        }

        case 'e':
        {
            if(!scene.association.get_data()[viewer.frame() - 1].empty()
                    && !(*fm_data_p)[viewer.frame() - 1].empty())
            {
                make_scene(scene, viewer.frame());
            }
            break;
        }

        case 'x':
        {
            scene.association.clear();
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
    // set colors
    if(!viewer.scene().association.get_data()[viewer.frame() - 1].empty())
    {
        viewer.set_data_box_style(
                            *active_databox_p,
                            Vector(0.0, 1.0, 0.0),
                            2.0, true);
        viewer.set_facemark_style(
                            *active_fmark_p,
                            Vector(0.0, 1.0, 0.0),
                            2.0, true);
    }

    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();

    size_t num_fmarks = (*fm_data_p)[fr - 1].size();
    size_t fm = distance(
                (*fm_data_p)[fr - 1].begin(),
                active_fmark_p.get_iterator());

    size_t num_boxes = viewer.scene().association.get_data()[fr - 1].size();
    size_t bx = distance(
                    viewer.scene().association.get_data()[fr - 1].begin(),
                    active_databox_p.get_iterator());

    info_text[0] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);
    info_text[1] = "Mark: " + lexical_cast<string>(fm + 1) + "/"
                             + lexical_cast<string>(num_fmarks);
    info_text[2] = "Box: " + lexical_cast<string>(bx + 1) + "/"
                           + lexical_cast<string>(num_boxes);
    viewer.set_text_lines(info_text.begin(), info_text.end());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void make_scene(Scene& scene, size_t t)
{
    // create target
    double h = get_entity_type_average_height(PERSON_ENTITY);
    double w = get_entity_type_average_width(PERSON_ENTITY);
    double g = get_entity_type_average_girth(PERSON_ENTITY);
    const Perspective_camera& cam = scene.camera;
    const Detection_box& dbox = *active_databox_p;

    Target tg(h, w, g, scene.association.get_data().size());
    tg.insert(make_pair(t, &dbox));
    tg.set_changed_all();

    // estimate 3d state
    tg.estimate_height(cam);
    tg.trajectory()[t - 1] = Trajectory_element();
    Ground_back_projector bproj(cam, 0.0);
    Vector gp = bproj(
                    dbox.bbox.get_bottom_center()[0],
                    dbox.bbox.get_bottom_center()[1]);

    tg.trajectory()[t - 1]->value.position = gp;
    update_direction(tg.trajectory(), t, true);
    tg.trajectory()[t - 1]->value.body_dir = active_fmark_p->yaw()*M_PI/180;
    tg.trajectory()[t - 1]->value.face_dir[0] = 0.0;
    tg.update_boxes(cam);
    tg.update_faces(cam);
    tg.face_trajectory()[t - 1]->value.facemark = &(*active_fmark_p);

    scene.association.clear();
    scene.association.insert(tg);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//void max_head(const Scene& scene)
//{
//    // crate important likelihood
//    const double face_sd = 5;
//    Facemark_likelihood fm_likelihood(*fm_data_p, face_sd, img_width, img_height);
//
//    // create garbage data and likelihoods
//    vector<Integral_flow> flows_x;
//    vector<Integral_flow> flows_y;
//
//    Box_likelihood box_likelihood(1.0, img_width, img_height);
//    Optical_flow_likelihood of_likelihood(
//        flows_x, flows_y, img_width, img_height,
//        1.0, 1.0, 1.0, 1.0);
//
//    Face_flow_likelihood ff_likelihood(
//        flows_x, flows_y, img_width, img_height,
//        1.0, 1.0, 1.0, 1.0);
//
//    Color_likelihood color_likelihood; 
//
//    // crate garbage priors
//    Position_prior pos_prior(1.0, 1.0);
//    Direction_prior dir_prior(1.0, 1.0);
//    Face_direction_prior fdir_prior(1.0, 1.0);
//
//    // create posterior
//    Scene_posterior posterior(
//                        box_likelihood,
//                        fm_likelihood,
//                        of_likelihood, 
//                        ff_likelihood, 
//                        color_likelihood,
//                        pos_prior,
//                        dir_prior,
//                        fdir_prior);
//    posterior.use_fm_lh() = true;
//    posterior.use_box_lh() = false;
//    posterior.use_of_lh() = false;
//    posterior.use_ff_lh() = false;
//    posterior.use_color_lh() = false;
//    posterior.use_pos_prior() = false;
//    posterior.use_dir_prior() = false;
//    posterior.use_fdir_prior() = false;
//    posterior.use_dim_prior() = true;
//
//    // create sampler
//    const size_t num_samples = 50;
//    Sample_scenes sample_scenes(posterior, true);
//    sample_scenes.set_num_iterations(num_samples);
//    sample_scenes.set_num_leapfrog_steps(2);
//    sample_scenes.set_hmc_step_size(0.001);
//
//    sample_scenes(scene);
//}

