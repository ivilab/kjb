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
#include <people_tracking_cpp/pt_scene_diff.h>
#include <people_tracking_cpp/pt_integral_optimization.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <gr_cpp/gr_opengl_headers.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_iterator.h>
#include <l_cpp/l_exception.h>
#include <diff_cpp/diff_optim.h>
#include <diff_cpp/diff_util.h>
#include <l/l_sys_mal.h>
#include <string>
#include <vector>
#include "utils.h"
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/record.h>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

// other constants
const double tdx = 0.01;
const double tdz = 0.01;
const double tdb = M_PI/64;
const double tdy = M_PI/64;
const double tdp = M_PI/64;
const double height_delta = 0.02;

// state
typedef Box_data::Box_set::const_iterator Data_citerator;
typedef Ascn::const_iterator Ass_citerator;
typedef std::vector<Scene>::const_iterator Scene_citerator;
const_circular_iterator<Data_citerator> active_databox_p;
const_circular_iterator<Ass_citerator> active_target_p;
const_circular_iterator<Scene_citerator> active_proposal_p;
vector<string> info_text(13);
double blh;
double mlh;
double olh;
double flh;
double ppr;
double dpr;
double fpr;
double pst;
double grad_move_size;
double hmc_step_size;
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
vector<double> step_sizes;

const Scene_posterior* posterior_p;
const Scene_gradient* scene_grad_p;
const Scene_adapter* adapter_p;
const Scene* gt_scene_p;
Sample_scenes* sample_scenes_p;
Scene* scene_p;
Facemark_data* fm_data_p;
std::vector<Scene> samples;
std::vector<Scene> proposals; 

// output 
std::string outdir;
std::string datadir; 

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

/** @brief  Helper function. */
void update_viewer(Scene_viewer& viewer);

/** @brief  Update the posterior components. */
void update_posterior(const Scene& scene, size_t frame);

/** @brief  Set the changed index of a target. */
void set_changed_frame(const Target& target, size_t frame);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
#ifdef TEST
    kjb_c::kjb_init();
    //kjb_c::kjb_l_set("heap-checking", "off");
    //kjb_c::kjb_l_set("initialization-checking", "off");
#endif

    try
    {
        const size_t rand_seed = 1234;
        // set random seed 
        kjb_c::kjb_seed_rand(rand_seed, rand_seed);
        kjb_c::kjb_seed_rand_2(rand_seed);
        ergo::global_rng<boost::mt19937>().seed(rand_seed);
        srand(rand_seed);

        // create (or read) scene
        //size_t num_frames = 200;
        //double img_width = 1200;
        //double img_height = 800;
        size_t num_frames = 100;
        double img_width = 600;
        double img_height = 500;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene gt_scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        const size_t num_tracks = 1;
        // initialize scene 
        std::vector<std::string> frame_fps = create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, gt_scene, num_tracks, true);
        fm_data_p = &fm_data;

        //update_facemarks(gt_scene.association, fm_data);
        //update_scene_state(gt_scene, fm_data, refine);
        //update_scene_state(gt_scene, fm_data, false);
        Scene scene = gt_scene;
        scene_p = &scene;

        gt_scene_p = &gt_scene;

        // create likelihood
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        const double face_sd = 1.0;
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

        const double gpsc = 100.0;
        const double gpsv = 10.0;
        Position_prior pos_prior(gpsc, gpsv);

        const double gpsc_dir = 200.0;
        const double gpsv_dir = M_PI*M_PI/64.0;
        Direction_prior dir_prior(gpsc_dir, gpsv_dir);

        const double gpsc_fdir = 200.0;
        const double gpsv_fdir = M_PI*M_PI/256.0;
        Face_direction_prior fdir_prior(gpsc_fdir, gpsv_fdir);

        // create posterior
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

        // create adapter
        //Scene_adapter adapter(&scene);
        //adapter_p = &adapter;

        // create gradient 
        /*vector<double> dx = trajectory_gradient_step_sizes(scene);
        Scene_gradient grad(posterior, dx);
        scene_grad_p = &grad;
        size_t D = dx.size();
        step_sizes.resize(D, grad_move_size);*/
        
        // create sampler 
        const size_t num_samples = 1;
        Sample_scenes sample_scenes(posterior);
        sample_scenes.set_num_iterations(num_samples);
        sample_scenes.set_num_leapfrog_steps(10);

        sample_scenes_p = &sample_scenes;

        // create viewer
        Scene_viewer viewer(scene, img_width, img_height);
        viewer.set_flows(flows_x, flows_y);
        viewer.set_facemarks(fm_data);
        viewer.weigh_data_box_color(true);
        viewer.set_key_callback(bind(handle_key, boost::ref(viewer), _1, _2, _3));
        viewer.set_frame_images(frame_fps.begin(), frame_fps.end());
        viewer.draw_images(true);
        //viewer.draw_images(false);

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
        grad_move_size = 1e-10;
        hmc_step_size = 1e-5;
        outdir = std::string("./output/gradient.cpp"); 
        // update viewer
        update_viewer(viewer);
        update_posterior(scene, 1);

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
    catch(const std::exception& sex)
    {
        cerr << sex.what() << endl;
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
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'p':
        {
            viewer.rewind_frame();
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'N':
        {
            viewer.advance_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'P':
        {
            viewer.rewind_frame(frame_jump);
            active_databox_p = make_const_circular_iterator(
                scene.association.get_data()[viewer.frame() - 1]);
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'j':
        {
            active_target_p++;
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'k':
        {
            active_target_p--;
            update_posterior(scene, viewer.frame());
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

        case '7':
        {
            active_target_p->trajectory().height += height_delta;
            //render_ground = !render_ground;
            break;
        }

        case '8':
        {
            active_target_p->trajectory().height -= height_delta;
            //render_ground = !render_ground;
            break;
        }

        case 'c':
        {
            render_cylinders = !render_cylinders;
            break;
        }

        /*case 'b':
        {
            render_bodies = !render_bodies;
            break;
        }*/

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

        /*case 'y':
        {
            render_data_boxes = !render_data_boxes;
            break;
        }*/

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
            Vector3 dv(-tdx, 0.0, 0.0);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'X':
        {
            Vector3 dv(tdx, 0.0, 0.0);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'z':
        {
            Vector3 dv(0.0, 0.0, -tdz);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'Z':
        {
            Vector3 dv(0.0, 0.0, tdz);
            move_trajectory_at_frame(
                                scene,
                                *active_target_p,
                                viewer.frame(),
                                dv,
                                false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'b':
        {
            move_trajectory_dir_at_frame(
                                    scene,
                                    *active_target_p,
                                    viewer.frame(),
                                    -tdb,
                                    false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'B':
        {
            move_trajectory_dir_at_frame(
                                    scene,
                                    *active_target_p,
                                    viewer.frame(),
                                    tdb,
                                    false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'y':
        {
            Vector2 dv(-tdy, 0.0);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'Y':
        {
            Vector2 dv(tdy, 0.0);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'i':
        {
            Vector2 dv(0.0, -tdp);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        case 'I':
        {
            Vector2 dv(0.0, tdp);
            move_trajectory_face_dir_at_frame(
                                            scene,
                                            *active_target_p,
                                            viewer.frame(),
                                            dv,
                                            false);
            set_changed_frame(*active_target_p, viewer.frame());
            update_posterior(scene, viewer.frame());
            break;
        }

        /*case 'z':
        {
            render_solid = !render_solid;
            break;
        }*/

        /*case 'i':
        {
            overhead_mode = !overhead_mode;
            break;
        }*/

        /*case 's':
        {
            Image img = opengl::get_framebuffer_as_image();
            img.write("output/gradient.jpg");
            break;
        }*/

        case 'A': 
        {
            grad_move_size *= 2.0;
            break;
        }

        case 'D': 
        {
            grad_move_size /= 2.0;
            break;
        }

        case 'T': 
        {
            *scene_p = *gt_scene_p;
            active_target_p = make_const_circular_iterator(scene_p->association);
            std::cout << "Render groud truth scene\n";
            break;
        }

        case 'U':
        {
            update_scene_state(*scene_p, *fm_data_p, true);
            std::cout << "Update scene state\n";
            break;
        }

        case 'C':
        {
            Scene_adapter adapter(scene_p);
            vector<double> dx = trajectory_gradient_step_sizes(scene);
            Scene_gradient grad(*posterior_p, dx);
            Vector g = grad(*scene_p);
            std::vector<double> exg(g.size());
            step_sizes.resize(exg.size(), grad_move_size);
            std::transform(
                    step_sizes.begin(),
                    step_sizes.end(),
                    g.begin(),
                    exg.begin(),
                    std::multiplies<double>());
            move_params(*scene_p, exg, adapter);
            update_posterior(*scene_p, viewer.frame());
            break;
        }

        case 'G':
        {
            // create gradient 
            Scene_adapter adapter(scene_p);
            vector<double> dx = trajectory_gradient_step_sizes(scene);
            std::cout << "dx: " << dx.size() << std::endl;
            Scene_gradient grad(*posterior_p, dx);
            size_t D = dx.size();
            step_sizes.resize(D, grad_move_size);
            gradient_ascent(*posterior_p, *scene_p, step_sizes, 
                            grad, adapter);
            update_posterior(*scene_p, viewer.frame());
            break;
        }

        case 'S':
        {
            std::cout << "HMC sampling \n";
            kjb_c::kjb_mkdir(outdir.c_str());
            ofstream log_fs((outdir + "/sample_logs.txt").c_str());
            ostream_iterator<ergo::step_detail> log_out(log_fs, "\n");
            sample_scenes_p->set_hmc_step_size(hmc_step_size);
            size_t smp_n = 1;
            size_t prp_n = 1;
            //Write_scene_iterator sample_out(outdir + "/samples", smp_n);
            //Write_scene_iterator proposal_out(outdir + "/proposals", prp_n);
            ofstream pd_fs((outdir + "/pp_detail.txt").c_str());
            std::ostream_iterator<std::string> pd_out(pd_fs, "\n==========\n\n");
           
            samples.clear();
            proposals.clear(); 
            samples.resize(1, *scene_p);
            proposals.resize(1, *scene_p);
            // sample
            sample_scenes_p->operator()(
                        *scene_p,
                        make_optional(log_out),
                        make_optional(samples.begin()),
                        make_optional(proposals.begin()),
                        make_optional(pd_out));

            active_proposal_p = make_const_circular_iterator(proposals);
            update_posterior(*scene_p, viewer.frame());
            break;
        }

        case 'M':
        {
            std::cout << " View proposals\n"; 
            *scene_p = *active_proposal_p;
            active_target_p = make_const_circular_iterator(scene_p->association);
            active_proposal_p++;
            break;
        }

        case 's':
        {
            write_scene(viewer.scene(), std::string("./output/gradient"));
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

    //update_posterior(scene);
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

        //viewer.draw_images(false);
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
    viewer.set_target_style(*active_target_p, Vector(1.0, 0.0, 0.0), 3.0, true);

    // set text
    size_t num_frames = viewer.scene().association.get_data().size();
    size_t fr = viewer.frame();
    size_t num_tracks = viewer.scene().association.size();
    size_t tr = distance(viewer.scene().association.begin(),
                         active_target_p.get_iterator());
    size_t num_boxes = viewer.scene().association.get_data()[fr - 1].size();
    size_t bx = distance(viewer.scene().association.get_data()[fr - 1].begin(),
                         active_databox_p.get_iterator());

    size_t i = 0;
    info_text[i++] = "Frame: " + lexical_cast<string>(fr) + "/"
                             + lexical_cast<string>(num_frames);
    info_text[i++] = "Track: " + lexical_cast<string>(tr + 1) + "/"
                             + lexical_cast<string>(num_tracks);
    info_text[i++] = "Box: " + lexical_cast<string>(bx + 1) + "/"
                           + lexical_cast<string>(num_boxes);
    info_text[i++] = "BLH: " + lexical_cast<string>(blh);
    info_text[i++] = "MLH: " + lexical_cast<string>(mlh);
    info_text[i++] = "OLH: " + lexical_cast<string>(olh);
    info_text[i++] = "FLH: " + lexical_cast<string>(flh);
    info_text[i++] = "PPR: " + lexical_cast<string>(ppr);
    info_text[i++] = "DPR: " + lexical_cast<string>(dpr);
    info_text[i++] = "FPR: " + lexical_cast<string>(fpr);
    info_text[i++] = "PST: " + lexical_cast<string>(pst);
    info_text[i++] = "GRAD MOVE SIZE: " + lexical_cast<string>(grad_move_size);
    info_text[i++] = "HMC STEP SIZE: " + lexical_cast<string>(hmc_step_size);
    KJB(ASSERT(i == 13));
    viewer.set_text_lines(info_text.begin(), info_text.end());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void set_changed_frame(const Target& target, size_t frame)
{
    // update the changed index
    if(!target.changed())
    {
        target.set_changed_start(frame);
        target.set_changed_end(frame);
    }
    else
    {
        // set the changed index
        if(frame < target.changed_start() && frame >= 1)
        {
            target.set_changed_start(frame);
        }
        else if (frame > target.changed_end())
        {
            target.set_changed_end(frame);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_posterior(const Scene& scene, size_t frame)
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
    //olh = posterior(scene);
    if(active_target_p->body_trajectory()[frame - 1])
        olh = posterior.of_likelihood().at_box(active_target_p->body_trajectory()[frame - 1]->value, frame);
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

#else
int main() { std::cout << "Cannot run. Need libergo.\n"; return EXIT_FAILURE; }
#endif
