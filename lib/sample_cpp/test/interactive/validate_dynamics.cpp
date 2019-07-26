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
#include <sample/sample_misc.h>
#include <edge_cpp/features_manager.h>

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
#endif

#include <sample_cpp/dynamics_moves.h>
string output_img_name;
string room_name;
string camera_name;


using namespace kjb;

int main(int argc, char** argv)
{

	if(argc < 3)
	{
		std::cout << "Usage: <base_dir> <image_number>" << std::endl;
		return 1;
	}

	string base(argv[1]);
	base.append("/");

	string img_name(base);
	img_name.append("orientation_maps/or_");
	img_name.append(argv[2]);
	img_name.append(".tif");

	base.append("gt/");
	string camera_file(base);
	camera_file.append(argv[2]);
	camera_file.append("_camera.txt");

	string room_file(base);
	room_file.append(argv[2]);
	room_file.append("_room.txt");

	Image img(img_name);

	Parametric_parapiped pp(room_file.c_str());
	Perspective_camera camera(camera_file.c_str());

	static kjb::Offscreen_buffer* offscreen = kjb::create_and_initialize_offscreen_buffer(img.get_num_cols(), img.get_num_rows());

	camera.prepare_for_rendering(true);
	Base_gl_interface::set_gl_view(img);
	glColor3f(0.0, 0.0, 0.0);
    pp.wire_render();
    kjb_c::KJB_image * capture = NULL;
	Base_gl_interface::capture_gl_view(&capture);
	Image img2(capture);
	img2.write("validate.jpg");

	delete offscreen;



}

