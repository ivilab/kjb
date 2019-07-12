#define DEBUG

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_spline.h>
#include <m_cpp/m_vector.h>

void draw();
void reshape(int width, int height);

using namespace kjb;
using namespace kjb::opengl;
using kjb_c::pause_on_next;

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("KJB Bezier Test");
    glutDisplayFunc(draw);
//  glutIdleFunc(draw);
    glutReshapeFunc(reshape);


    //adding here the mouse processing callbacks
//    glutMouseFunc(mouse_btn);
//    glutMotionFunc(mouse_active_motion);
//    glutPassiveMotionFunc(mouse_passive_motion);
//  glutEntryFunc(mouse_entry);
    
    glutMainLoop();

    return 0;
}


void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
    Gl_bezier_curve c1(3, 3, GL_MAP1_VERTEX_3);

    glColor3f(0.,0.,0.);

    Vector v(3);

    v[0] = 0; v[1] = 0; v[2] = 0;
    c1.set_control_point(0, v);
    v[0] = 1; v[1] = 0; v[2] = 0;
    c1.set_control_point(1, v);
    v[0] = 1; v[1] = 1; v[2] = 0;
    c1.set_control_point(2, v);
    v[0] = 0; v[1] = 1; v[2] = 0;
    c1.set_control_point(3, v);


    Gl_polybezier_curve c2(3, GL_MAP1_VERTEX_3);
    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    c2.insert_curve_point(0, v);
    v[0] = 1.0; v[1] = 0.5; v[2] = 0.0;
    c2.insert_curve_point(1, v);
    v[0] = 0.5; v[1] = 1.0; v[2] = 0.0;
    c2.insert_curve_point(2, v);
    v[0] = 0.0; v[1] = 0.5; v[2] = 0.0;
    c2.insert_curve_point(3, v);
    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    c2.insert_curve_point(4, v);

    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_1(0, v);
    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_1(1, v);
    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    c2.set_handle_point_1(2, v);
    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    c2.set_handle_point_1(3, v);
    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_1(4, v);

    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_2(0, v);
    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    c2.set_handle_point_2(1, v);
    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    c2.set_handle_point_2(2, v);
    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_2(3, v);
    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    c2.set_handle_point_2(4, v);

    gluNurbsProperty(c1.get_nurbs_object(), GLU_SAMPLING_METHOD, GLU_PATH_LENGTH);
    gluNurbsProperty(c1.get_nurbs_object(), GLU_SAMPLING_TOLERANCE, 1);

    gluBeginCurve(c1.get_nurbs_object());
    c1.gl_call();
    gluEndCurve(c1.get_nurbs_object());

    gluBeginCurve(c2.get_nurbs_object());
    c2.gl_call();
    gluEndCurve(c2.get_nurbs_object());

//    c2.print_svg();
    for(float i = 0; i < 1.; i += .05)
    {
        Vector v = c2.evaluate(i);
        Vector dv = c2.gradient(i);
//        std::cout << v.get_length() << std::endl;
//        std::cout << "c1(" << i << ") " << v[0] << ",\t" << v[1] << ",\t" << v[2] << std::endl;

        glPointSize(5.);
        glColor3f(i, 0, 0);
        glBegin(GL_POINTS);
        glVertex(v);
        glEnd();

        glBegin(GL_LINES);
        glVertex(v);
        glVertex(v + .05 * dv);
        glEnd();
    }

    glFlush();

    glutSwapBuffers(); 
}

void reshape(int width, int height)
{
    if(width == 0) width = 1;
    if(height == 0) height = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glViewport(0, 0, width, height);
    glOrtho(-.1,1.1, -.1, 1.1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);
}
