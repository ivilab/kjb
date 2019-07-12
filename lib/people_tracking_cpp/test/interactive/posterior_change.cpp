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
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_face_flow_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_util.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_iterator.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_mal.h>
#include <vector>
#include "utils.h"
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

// other constants
const double dx = 0.01;
const double dz = 0.01;
const double db = M_PI/64;
const double dy = M_PI/64;
const double dp = M_PI/64;
const Scene_posterior* posterior_p;

// state
typedef Box_data::Box_set::const_iterator Data_citerator;
typedef Ascn::const_iterator Ass_citerator;
const_circular_iterator<Ass_citerator> active_target_p;
double blh;
double mlh;
double olh;
double flh;
double ppr;
double dpr;
double fpr;
double pst;
vector<string> info_text(10);

/** @brief  Handle key input. */
void handle_key(Scene_viewer& viewer, unsigned char, int, int);

/** @brief  Helper function. */
void update_viewer(Scene_viewer& viewer);

/** @brief  Update the posterior components. */
void update_posterior(const Scene& scene);

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
        // create (o read) a scene and data
        size_t num_frames = 50;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);

        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);
        update_facemarks(scene.association, fm_data);

        // create posterior
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        const double face_sd = 2.0;
        Facemark_likelihood fm_likelihood(fm_data, face_sd, img_width, img_height);

        const double scale_x = 0.34990;
        const double scale_y = 0.17581;
        Optical_flow_likelihood of_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, 2*scale_x, 2*scale_y);

        Face_flow_likelihood ff_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, 2*scale_x, 2*scale_y);

        Color_likelihood color_likelihood; 

        double gpsc = 20.0;
        double gpsv = 10.0;
        Position_prior pos_prior(gpsc, gpsv);

        double gpsc_dir = 2.5;
        double gpsv_dir = 50.0;
        Direction_prior dir_prior(gpsc_dir, gpsv_dir);

        double gpsc_fdir = 1.8;
        double gpsv_fdir = M_PI*M_PI/36;
        Face_direction_prior fdir_prior(gpsc_fdir, gpsv_fdir);

        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        posterior.use_color_lh() = false;
        posterior.use_dim_prior() = false;

        posterior_p = &posterior;

        // create viewer
        Scene_viewer viewer(scene, img_width, img_height);
        viewer.set_facemarks(fm_data);
        viewer.set_flows(flows_x, flows_y);
        viewer.weigh_data_box_color(true);
        viewer.draw_text(true);
        viewer.draw_3d_mode();
        viewer.set_key_callback(bind(handle_key, boost::ref(viewer), _1, _2, _3));

        // active stuff
        active_target_p = make_const_circular_iterator(scene.association);

        // update viewer
        update_posterior(scene);
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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(Scene_viewer& viewer, unsigned char key, int, int)
{
    const size_t frame_jump = 10;
    const Scene& scene = viewer.scene();

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
            active_target_p++;
            break;
        }

        case 'k':
        {
            active_target_p--;
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

        case 'x':
        {
            Vector3 dv(-dx, 0.0, 0.0);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            update_posterior(scene);
            break;
        }

        case 'X':
        {
            Vector3 dv(dx, 0.0, 0.0);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            update_posterior(scene);
            break;
        }

        case 'z':
        {
            Vector3 dv(0.0, 0.0, -dz);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            update_posterior(scene);
            break;
        }

        case 'Z':
        {
            Vector3 dv(0.0, 0.0, dz);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            update_posterior(scene);
            break;
        }

        case 'b':
        {
            move_trajectory_dir_at_frame(
                                    scene,
                                    *active_target_p,
                                    viewer.frame(),
                                    -db,
                                    false);
            update_posterior(scene);
            break;
        }

        case 'B':
        {
            move_trajectory_dir_at_frame(
                                    scene,
                                    *active_target_p,
                                    viewer.frame(),
                                    db,
                                    false);
            update_posterior(scene);
            break;
        }

        case 'y':
        {
            Vector2 dv(-dy, 0.0);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            update_posterior(scene);
            break;
        }

        case 'Y':
        {
            Vector2 dv(dy, 0.0);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            update_posterior(scene);
            break;
        }

        case 'i':
        {
            Vector2 dv(0.0, -dp);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            update_posterior(scene);
            break;
        }

        case 'I':
        {
            Vector2 dv(0.0, dp);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            update_posterior(scene);
            break;
        }

        case 'w':
        {
            Image img = opengl::get_framebuffer_as_image();
            img.write("output/posterior_change_cpp.jpg");
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
    viewer.set_target_style(*active_target_p, Vector(0.0, 1.0, 0.0), 2.0, true);

    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();
    size_t num_tracks = viewer.scene().association.size();
    size_t tr = std::distance(viewer.scene().association.begin(),
                              active_target_p.get_iterator());

    info_text[0] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);
    info_text[1] = "Track: " + lexical_cast<string>(tr + 1) + "/"
                             + lexical_cast<string>(num_tracks);
    info_text[2] = "BLH: " + lexical_cast<string>(blh);
    info_text[3] = "MLH: " + lexical_cast<string>(mlh);
    info_text[4] = "OLH: " + lexical_cast<string>(olh);
    info_text[5] = "FLH: " + lexical_cast<string>(flh);
    info_text[6] = "PPR: " + lexical_cast<string>(ppr);
    info_text[7] = "DPR: " + lexical_cast<string>(dpr);
    info_text[8] = "FPR: " + lexical_cast<string>(fpr);
    info_text[9] = "PST: " + lexical_cast<string>(pst);

    viewer.set_text_lines(info_text.begin(), info_text.end());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_posterior(const Scene& scene)
{
    const Scene_posterior& posterior = *posterior_p;
    posterior.use_box_lh() = false;
    posterior.use_fm_lh() = false;
    posterior.use_of_lh() = false;
    posterior.use_ff_lh() = false;
    posterior.use_pos_prior() = false;
    posterior.use_dir_prior() = false;
    posterior.use_fdir_prior() = false;

    posterior.use_box_lh() = true;
    blh = posterior(scene);
    posterior.use_box_lh() = false;

    posterior.use_fm_lh() = true;
    mlh = posterior(scene);
    posterior.use_fm_lh() = false;

    posterior.use_of_lh() = true;
    olh = posterior(scene);
    posterior.use_of_lh() = false;

    posterior.use_ff_lh() = true;
    flh = posterior(scene);
    posterior.use_ff_lh() = false;

    posterior.use_pos_prior() = true;
    ppr = posterior(scene);
    posterior.use_pos_prior() = false;

    posterior.use_dir_prior() = true;
    dpr = posterior(scene);
    posterior.use_dir_prior() = false;

    posterior.use_fdir_prior() = true;
    fpr = posterior(scene);
    posterior.use_fdir_prior() = false;

    posterior.use_box_lh() = true;
    posterior.use_fm_lh() = true;
    posterior.use_of_lh() = true;
    posterior.use_ff_lh() = true;
    posterior.use_pos_prior() = true;
    posterior.use_dir_prior() = true;
    posterior.use_fdir_prior() = true;
    pst = posterior(scene);
}

