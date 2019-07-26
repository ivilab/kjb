/* $Id: test_likelihood_computation.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Luca Del Pero
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "gr_cpp/gr_camera.h"
#include "gr_cpp/gr_opengl.h"
#include "st_cpp/st_perspective_camera.h"
#include "st_cpp/st_parapiped.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_distribution.h"

#include <sstream>
#include <boost/assign/std/vector.hpp>

#include "gr_cpp/gr_offscreen.h"
#include "likelihood_cpp/edge_points_likelihood.h"

#include "gr_cpp/gr_opengl.h"


using namespace kjb;
using namespace std;
using kjb_c::kjb_rand;

float width = 400;
float height = 300;
const float NEAR = 10;
const float FAR = 10000;

#ifdef KJB_HAVE_OPENGL
static kjb::Offscreen_buffer* offscreen = 0;

namespace kjb
{

/** Template specialization for computing the likelihood from
 *  a model and a camera. In this case the model is a Parametric_Parapiped
 *  and the camera is of type Perspective_camera
 *
 *  @param pp     The model to compute the likelihood for
 *  @param camera The camera under which the parapiped is to be rendered
 *  @param mode   The mode for the likelihood computation. Here only 0 is
 *                supported (from model map and edges), because this is how
 *                we compute the likelihood for parapipeds and cameras.
 */
template <>
double Independent_edge_points_likelihood::compute_likelihood
(
    Parametric_parapiped & pp,
    Perspective_camera & camera,
    unsigned int mode
)
{
    if(mode != FROM_MAP_AND_EDGES)
    {
        KJB_THROW_2(Illegal_argument, "Likelihood computation mode not supported for camera and parapiped");
    }

    /** We update the camera and the parapiped so that their
     * rendering interfaces reflect the values of their parameters.
     * At the same time the gl rendering context is prepared for rendering */
    camera.prepare_for_rendering(true);
    pp.update_if_needed();

    /** We compute the model map and prepare the model edges needed for
     * the likelihood computation */
    const Polymesh & polymesh = pp.get_polymesh();
#warning "[Code police] something wrong here"
#if 0
    prepare_model_map(model_map,polymesh);
    std::vector<Model_edge> model_edges;
    prepare_model_edges(model_edges, polymesh, camera.get_rendering_interface());

    /** Finally, we compute the likelihood */
    return this->operator()(model_map, model_edges);
#endif

}

}

void init_opengl(int argc, char* argv[])
{
    using namespace kjb;

    offscreen = kjb::create_and_initialize_offscreen_buffer((GLsizei)width,
            (GLsizei)height);
}

int main(int argc, char* argv[])
{

    //kjb_c::kjb_init();
    init_opengl(argc, argv);
    glColor3f(1.0,0.0,0.0);

    /* Parametric_parapiped pp(1.0,0.0,0.0, 30.0, 20.0, 10.0); yaw = 0
     * 1377.466 Best likelihood */
    Parametric_parapiped pp(1.0,0.0,0.0, 30.0, 20.0, 10.0);
    pp.set_yaw(0.062);

    //  f = 1000, c_z = 300
    Perspective_camera camera;
    camera.set_focal_length(1000);
    camera.set_camera_centre_z(300);

    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb_c::KJB_image * img = 0;
    pp.wire_occlude_render();
    pp.wire_render();
    Base_gl_interface::capture_gl_view(&img);
    kjb::Image im(img);
    im.write("data_edges.tiff");

    kjb::Image testimg("data_edges.tiff");
    Canny_edge_detector canny(1.0, 10, 5, 20, true);
    kjb::Edge_set * data_edges =  canny.detect_edges(testimg, true);
    data_edges->break_edges_at_corners(0.9, 7);
    data_edges->remove_short_edges(10);

#warning "[Code police] something wrong here"
#if 0 
    Independent_edge_points_likelihood likelihood(
        *data_edges,
        60, //Number of angle bins
        300, //Number of image rows
        400, //Number of image cols
        1.0, //stdev for difference in orientation
        1.0, //stdev for distance
        10.0, // max dist allowed
        1.0, //background probability
        1e-60, //noise probability
        1e-13, //silhouette miss probability
        1e-13 //inner edge miss probability
     );

    std::cout.precision(17);
    double ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 987.12314941307602) < DBL_EPSILON);

    pp.set_centre_z(1.2);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 3103.8998399271063) < DBL_EPSILON);

    pp.set_centre_x(3.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 8766.8430495962621) < DBL_EPSILON);

    pp.set_centre_x(1.0);
    pp.set_centre_z(0.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll +  987.12314941307602) < DBL_EPSILON);

    camera.set_focal_length(980.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 3366.2306146709052) < DBL_EPSILON);

    camera.set_focal_length(1000.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 987.12314941307602) < DBL_EPSILON);

    camera.set_pitch(0.01);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 16768.388521248802) < DBL_EPSILON);

    camera.set_pitch(0.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 987.12314941307602) < DBL_EPSILON);

    pp.set_yaw(1.2);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 32513.907612710493) < DBL_EPSILON);

    pp.set_yaw(0.063);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 989.12006862434043) < DBL_EPSILON);

    pp.set_yaw(0.062);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    ASSERT( fabs(ll + 987.12314941307602) < DBL_EPSILON);

    delete data_edges;
#endif

    return 0;
}
#else

int main(int argc, char**argv)
{
    std::cout << "Missing dependency: OPENGL" << std::endl;
    return 1;
}
#endif
