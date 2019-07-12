#include <iostream>

#include <sample_cpp/sample_dynamics.h> 
#include <likelihood_cpp/edge_points_likelihood.h>
#include <st_cpp/st_parapiped.h>
#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_offscreen.h>
#include <glut_cpp/glut_perspective_camera.h>
#include <glut_cpp/glut_parapiped.h>
#include <edge_cpp/edge.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

/** This is a multithreaded program. It must be compiled in
 * PRODUCTION mode, otherwise it will not work.
 * Also, GLUT is required
 */
using namespace std;

#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_X11
#ifdef MAC_OSX
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif
#endif

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <sample_cpp/dynamics_moves.h>
#define NONE 0
#define CAMERA_SELECTED 1
#define PARAPIPED_SELECTED 2

#define WRITE_PARAPIPED 9
#define EXIT_ID 10
static kjb::Offscreen_buffer* offscreen2 = 0;
static kjb::Perspective_camera * camera = 0;
static kjb::Parametric_parapiped * pp = 0;
static kjb::Parametric_parapiped * best_pp = 0;
static kjb::Edge_set * edge_set = 0;
static kjb::Independent_edge_points_likelihood * likelihood = 0;
static float gwidth = 400;
static float gheight = 300;
static uint32_t _current_action = 0;
static unsigned int object_selected = NONE;
double best_ll = 0;


static int _argc;
static char ** _argv;

boost::mutex _mutex;

/* -------------------------------------------------------------------------- */

void init_onscreen_buffer(int argc, char **argv);

kjb::Int_matrix model_map;
std::vector<kjb::Int_vector> edge_indexes;

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
    kjb::prepare_model_map(model_map,polymesh);
    std::vector<Model_edge> model_edges;
    prepare_model_edges(model_edges, polymesh, camera.get_rendering_interface());

    int edge_counter = 0;
	for(unsigned int i = 0; i < model_map.get_num_rows(); i++)
	{
		for(unsigned int j = 0; j < model_map.get_num_cols(); j++)
		{
			int is_edge = model_map(i, j);
		    if(is_edge > 0)
		    {
			    edge_indexes[edge_counter](0) = i;
			    edge_indexes[edge_counter](1) = j;
			    edge_indexes[edge_counter](2) = is_edge;
		 	    edge_counter++;
		    }
		}
	}

    /** Finally, we compute the likelihood */
    return this->operator()(model_map, model_edges, edge_indexes, edge_counter);

}

}

using namespace kjb;



double compute_likelihood(Parametric_parapiped & ipp,Perspective_camera & ic)
{
	//std::cout << "Compute the goddamn ll" << std::endl;
	double ll = likelihood->compute_likelihood<Parametric_parapiped, Perspective_camera>(ipp, ic);
	return ll;
}

void logger(const Parametric_parapiped & ipp, const Perspective_camera & ic, double ll)
{
	std::cout << "LOGGER" << std::endl;
	_mutex.lock();
	(*pp) = ipp;
    (*camera) = ic;
    _mutex.unlock();
}

void getter(Parametric_parapiped & ipp, Perspective_camera & ic)
{
	_mutex.lock();
	ipp = (*pp);
	ic = (*camera);
	_mutex.unlock();
}


static void display_glut()
{
	unsigned t0=clock();
	glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.66,0.66,0.66);

    _mutex.lock();
    camera->prepare_for_rendering(true);
    pp->solid_render();
    std::cout << "GLUT:" << pp->get_centre_x() << std::endl;
    _mutex.unlock();

    kjb::Image image;
    glFlush();
    camera->get_rendering_interface().capture_gl_view(image);
    edge_set->draw(image, 255.0, 0.0, 0.0);
    camera->get_rendering_interface().set_gl_view(image);
    glFlush();

    glutSwapBuffers();

}

void dynamics_function()
{
	double alpha = 0.95;
	unsigned int kick = 0;

	offscreen2 = kjb::create_and_initialize_offscreen_buffer(gwidth, gheight);

	std::cout << "Created offscreen buffer" << std::endl;
	kjb::Vector deltas(10);
	deltas(PCD_PARAPIPED_X) = 0.005;
	deltas(PCD_PARAPIPED_Y) = 0.005;
	deltas(PCD_PARAPIPED_Z) = 0.005;
	deltas(PCD_PARAPIPED_HEIGHT) = 0.005;
	deltas(PCD_PARAPIPED_WIDTH) = 0.005;
	deltas(PCD_PARAPIPED_LENGTH) = 0.005;
	deltas(PCD_PARAPIPED_YAW) = 0.0005;
    deltas(PCD_CAMERA_FOCAL) = 0.05;
    deltas(PCD_CAMERA_PITCH) = 0.00003;
    deltas(PCD_CAMERA_ROLL) = 0.0;
	kjb::Vector etas(10);
	etas(PCD_PARAPIPED_X) = 0.5;
	etas(PCD_PARAPIPED_Y) = 0.5;
	etas(PCD_PARAPIPED_Z) = 0.5;
	etas(PCD_PARAPIPED_HEIGHT) = 1.5;
	etas(PCD_PARAPIPED_WIDTH) = 1.5;
	etas(PCD_PARAPIPED_LENGTH) = 1.5;
	etas(PCD_PARAPIPED_YAW) = 0.05;
	etas(PCD_CAMERA_FOCAL) = 10.0;
	etas(PCD_CAMERA_PITCH) = 0.03;
	etas(PCD_CAMERA_ROLL) = 0.0;


	Parapiped_camera_dynamics dynamics
	(
		deltas,
	    etas,
	    &compute_likelihood,
	    &getter,
	    &logger,
	    alpha,
	    kick,
	    true,  //true
	    true,  //true
	    true,  //true
	    true,  //true
	    true,  //true
	    true, //true
	    false, // true
	    false, //true
	    false, // true
	    false
	);
	std::cout << "Created move" << std::endl;

	std::cout << "Ready to run" << std::endl;
	dynamics.run(10000);


}

static void camera_callback(int i)
{
    object_selected = CAMERA_SELECTED;
}

static void pp_callback(int i)
{
    object_selected = PARAPIPED_SELECTED;
}

static void reshape_glut(int w, int h)
{
    float s = (float)h / (float)gheight;
    glPointSize(s+0.49999);
    glLineWidth(s);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gwidth = w;
    gheight = h;
    glViewport(0, 0, gwidth, gheight);

}
static void keyboard_glut(unsigned char key, int a, int b)
{

	if(object_selected == CAMERA_SELECTED)
	{
	    Glut_perspective_camera::keyboard_callback(key);
	}
	else if(object_selected == PARAPIPED_SELECTED)
	{
		Glut_parapiped::keyboard_callback(key);
	}

	if(key == 'w')
	{
		string ppname = "parapiped";
		_mutex.lock();
		pp->write(ppname.c_str());
		camera->write("perspective_camera");
		_mutex.unlock();
	}

	else if( (key == 'k' ) || (key == 'j'))
	{
		_mutex.lock();
		double ll = likelihood->compute_likelihood<Parametric_parapiped, Perspective_camera>(*pp, *camera);
		_mutex.unlock();
		std::cout << "The likelihood is" << ll << std::endl;
	}
}

static void timer_glut(int ignored)
{
    glutPostRedisplay();
    glutTimerFunc(10, timer_glut, 0);
}


static void main_menu_glut(int id)
{
    using namespace kjb;
	_current_action = id;

	if(id == EXIT_ID)
    {
        delete pp;
        delete camera;
        exit(EXIT_SUCCESS);
    }
}

void init_onscreen_buffer(int argc, char **argv)
{

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                                        | GLUT_ALPHA | GLUT_STENCIL);
    glutInitWindowSize(gwidth, gheight);
    glutCreateWindow("Dynamics");
    glutDisplayFunc(display_glut);
    glutReshapeFunc(reshape_glut);
    glutKeyboardFunc(keyboard_glut);
    glutTimerFunc(10, timer_glut, 0);

    kjb::opengl::default_init_opengl(gwidth, gheight);

    int camera_menu = Glut_perspective_camera::create_glut_perspective_camera_submenu(camera_callback, camera);
    Glut_perspective_camera::update_width_increment(29);
    Glut_perspective_camera::update_height_increment(29);
    Glut_perspective_camera::update_aspect_ratio_increment(0.05);
    Glut_perspective_camera::update_focal_length_increment(20.0);
    Glut_perspective_camera::update_pitch_increment(0.03);
    Glut_perspective_camera::update_yaw_increment(0.03);
    Glut_perspective_camera::update_roll_increment(0.1);
    Glut_perspective_camera::update_skew_increment(0.05);
    Glut_perspective_camera::update_translation_x_increment(4.0);
    Glut_perspective_camera::update_translation_y_increment(4.0);
    Glut_perspective_camera::update_translation_z_increment(4.0);
    Glut_perspective_camera::update_world_scale_increment(0.1);

    int pp_menu = Glut_parapiped::create_glut_parapiped_submenu(pp_callback, pp);
    Glut_parapiped::update_translation_x_increment(1.0);
    Glut_parapiped::update_translation_y_increment(5.0);
    Glut_parapiped::update_translation_z_increment(5.0);
    Glut_parapiped::update_pitch_increment(0.1);
    Glut_parapiped::update_yaw_increment(0.1);
    Glut_parapiped::update_roll_increment(0.1);

	glutCreateMenu(main_menu_glut);
	glutAddSubMenu("Camera", camera_menu);
	glutAddSubMenu("Parapiped", pp_menu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);

}


int main(int argc, char** argv)
{
	_argc = argc;
	_argv = argv;

	 kjb_c::kjb_l_set("heap-checking","off");

	 edge_indexes.resize(20000, Int_vector(2, 0));

	pp = new Parametric_parapiped(1.0,11.0,6.0, 10.0, 15.0, 10.0);
	pp->set_yaw(0.062);
	camera = new Perspective_camera();
	camera->set_focal_length(1000);
	camera->set_focal_length(800);
	camera->set_camera_centre_z(300);

	kjb::Image testimg("data_edges.tiff");
	Canny_edge_detector canny(1.0, 10, 5, 20, true);
	edge_set =  canny.detect_edges(testimg, true);
	edge_set->break_edges_at_corners(0.9, 7);
	edge_set->remove_short_edges(10);

	Independent_edge_points_likelihood * likelihood2 = new Independent_edge_points_likelihood(
		edge_set,
		60, //Number of angle bins
		300, //Number of image rows
		400, //Number of image cols
		1.0, //stdev for difference in orientation
		10.0, //stdev for distance
		20.0, // max dist allowed
		1.0, //background probability
		1e-60, //noise probability
		1e-13, //silhouette miss probability
		1e-13 //inner edge miss probability
	 );

	likelihood = new Independent_edge_points_likelihood(*likelihood2);

	init_onscreen_buffer(argc, argv);

	std::cout << "The initial ll is" << likelihood2->compute_likelihood<Parametric_parapiped, Perspective_camera>(*pp, *camera) << std::endl;
	std::cout << "The initial ll according to the copy is" << likelihood->compute_likelihood<Parametric_parapiped, Perspective_camera>(*pp, *camera) << std::endl;

	//dynamics_function();
	boost::thread t1(dynamics_function);
	//t1.join();

    std::cout << "Glut main loop" << std::endl;
    glutMainLoop();
    delete camera;
    delete pp;
    delete likelihood;
    delete likelihood2;
    delete edge_set;
    delete pp;
    return 0;

}

#else

//Temporary, fix it so that it works on Linux as well
int main(int argc, char **argv)
{
	std::cout << "Glut is not available, cannot run this interactive program" << std::endl;
    return 0;
}

#endif
