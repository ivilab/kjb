/* $Id: test_edge_lines_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Luca Del Pero, Jinyan Guan
 * =========================================================================== */

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
#include <gr_cpp/gr_offscreen.h>

#include "likelihood_cpp/edge_lines_likelihood.h"
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
double Edge_lines_likelihood::compute_likelihood
(
    Parametric_parapiped & pp,
    Perspective_camera & camera
)
{
    /** We update the camera and the parapiped so that their
     * rendering interfaces reflect the values of their parameters.
     * At the same time the gl rendering context is prepared for rendering */
    camera.prepare_for_rendering(true);
    pp.update_if_needed();

    /** We compute the model map and prepare the model edges needed for
     * the likelihood computation */
    const Polymesh & polymesh = pp.get_polymesh();
    std::vector<Model_edge> model_edges;
    
    prepare_model_map(model_map,polymesh);
    prepare_rendered_model_edges(model_edges, model_map);

    /** Finally, we compute the likelihood */

    return this->operator()(model_edges);

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
    Parametric_parapiped pp(1.0,0.0,0.0, 50.0, 50.0, 60.0);
    
    pp.set_yaw(0.5);
    
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

    const Polymesh & polymesh = pp.get_polymesh();
    Int_matrix model_map(im.get_num_rows(), im.get_num_cols(), 0);

    kjb::Image img1 = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    kjb::Image img2 = img1;        
    kjb::Image img3 = img1;        
    kjb::Image img4 = img1;        
    kjb::Image img5 = img1;        
    kjb::Image img6 = img1;        
    kjb::Image img7 = img1; 
    
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
        
    std::vector<Model_edge> rendered_model_edges;
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    
    for (unsigned int i = 0; i < rendered_model_edges.size(); i++)
    {
        rendered_model_edges[i].draw(img6, 0, 0, 255, 1.0);
    }
    
    img6.write("rendered_model_edges.jpg");
   

//    kjb::Image testimg(img6);    
    
    kjb::Image testimg("001.jpg");    
    
    Canny_edge_detector canny(1.0, 0.01*255, 0.008*255, 10, true);
    kjb::Edge_set * data_edges = canny.detect_edges(testimg, true);

    data_edges->remove_short_edges(5);
    data_edges->break_edges_at_corners(0.8, 3);
    data_edges->remove_short_edges(25);

    data_edges->randomly_color(img5);

    Edge_segment_set edge_segments(data_edges, false);
    edge_segments.remove_overlapping_segments(*data_edges);
    edge_segments.randomly_color(img1, 1);

    std::cout<<"Number of data edges: "<<edge_segments.size()<<std::endl;
    
    std::vector<Model_edge> model_edges;
    prepare_model_edges(model_edges, polymesh, camera.get_rendering_interface());

    // Draw the model edges
    for (unsigned int i = 0; i < model_edges.size(); i++)
    {
        model_edges[i].draw(img3, 255, 0, 0, 1.0);       
        
        if (model_edges[i].is_visible())
        {
            model_edges[i].draw(img4, 0, 0, 255, 1.0);       
        }
    }
    
    Edge_lines_likelihood likelihood(
        edge_segments,  
        0.1, //stdev for difference in orientation
        100.0, //stdev for distance
        1e-10, //miss probability
        1e-2, //noise probability
        50.0, // max dist allowed
                M_PI_4,
                10
    );


    img1.write("data_edge_segments.jpg");
    img3.write("model_edge_segments.jpg");
    img4.write("visible_model_edge_segments.jpg");
    img5.write("data_edges_edges.jpg");
    model_map.write("model_map.txt");
    model_map = model_map*1000; 
    kjb::Image model_image(model_map);
    model_image.write("model_map.jpg");
    std::cout.precision(17);

    double ll; 
    kjb::Image img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_1.jpg");
    std::cout<< "1' Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;
    
    pp.set_centre_x(10.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_2.jpg");
    std::cout<< "2's Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;
    
    
    pp.set_centre_x(1.0);
    pp.set_centre_z(3.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_3.jpg");
    std::cout<< "3' Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;
    
    camera.set_focal_length(980.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_4.jpg");
    std::cout<< "4' Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;
    
    camera.set_focal_length(500.0);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_5.jpg");
    std::cout<< "5's Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;
    
//    camera.set_focal_length(1000); 
    camera.set_pitch(0.01);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    std::cout<< "6's Likelihood: "<<ll<<std::endl;
    img_corr.write("corr_6.jpg");
    std::cout<<std::endl;
    
    camera.set_focal_length(1000); 
    camera.set_pitch(0.1);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_7.jpg");
    std::cout<< "7's Likelihood: "<<ll<<std::endl;
    std::cout<<std::endl;

    camera.set_pitch(0.2);
    camera.set_focal_length(1000); 
    pp.set_yaw(0.5);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.set_focal_length(1000);
    camera.set_camera_centre_z(300);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_8.jpg");
    std::cout<< "8's Likelihood: "<<ll<<std::endl;
    
    pp.set_yaw(2.063);
    ll = likelihood.compute_likelihood<Parametric_parapiped, Perspective_camera>(pp, camera);
    camera.prepare_for_rendering(true);
    pp.update_if_needed();
    kjb::prepare_model_map(model_map,polymesh);
    kjb::prepare_rendered_model_edges(rendered_model_edges, model_map);
    img_corr = kjb::Image::create_initialized_image(im.get_num_rows(), im.get_num_cols(), 255,255,255);        
    likelihood.draw_correspondence(img_corr, img_corr, rendered_model_edges, 1.0);
    img_corr.write("corr_9.jpg");
    std::cout<< "9's Likelihood: "<<ll<<std::endl;

    delete data_edges;

    return 0;
}
#else

int main(int argc, char**argv)
{
    std::cout << "Missing dependency: OPENGL" << std::endl;
    return 1;
}
#endif
