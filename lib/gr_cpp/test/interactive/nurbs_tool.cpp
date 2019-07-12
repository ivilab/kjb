#define DEBUG

#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_spline.h>
//#include <geom_model.h>
#include <iostream>
#include <ostream>
#include <algorithm>

using namespace kjb;
using namespace kjb::opengl;
using namespace std;

struct Point
{
    double x;
    double y;
};

enum Point_type {NONE_PT, CURVE_PT, HANDLE_PT_1, HANDLE_PT_2};
const float  DISTANCE_THRESHOLD = 10;
const float  DISTANCE_THRESHOLD_2 = 100;

int width, height;
int highlight_index = -1;
Point_type highlight_type = NONE_PT;

GLuint texture;

Gl_polybezier_curve curve;
Vector temp_pt;
Vector temp_pt_2;

GLenum mouse_modifiers = 0;

bool redraw = false;

double modelview[16];
double projection[16];
int viewport[4];

Vector get_world_coords(const Vector& pt);
Vector get_screen_coords(const Vector& pt);
void print_ctl_pts(ostream ost);
void mouse_btn(int button, int state, int x, int y);
void mouse_active_motion(int x, int y);
void mouse_passive_motion(int x, int y);
void mouse_all_motion(int x, int y);
void init();
void draw();
void reshape(int width, int height);

float get_nearest_point(const Vector& pt);

int main(int argc, char** argv)
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("Nurbs tool");
    glutDisplayFunc(draw);
//  glutIdleFunc(draw);
    glutReshapeFunc(reshape);


    glutMouseFunc(mouse_btn);
    glutMotionFunc(mouse_active_motion);
    glutPassiveMotionFunc(mouse_passive_motion);
//  glutEntryFunc(mouse_entry);

    glEnable( GL_TEXTURE_2D );
    
    init();
    glutMainLoop();
    return 0;
}

void print_ctl_pts(ostream ost)
{
    // output a c-style list of control positions, in order.
}

void mouse_btn(int button, int state, int x, int y)
{
    mouse_modifiers = glutGetModifiers();

    // is shift down?
    if(mouse_modifiers & GLUT_ACTIVE_SHIFT)
    {
        if(highlight_index < 0)
        {
            Vector cursor(3);
            y = height - y;
            cursor[0] = x; cursor[1] = y; cursor[2] = 0;
            Vector new_pt = get_world_coords(cursor);

            float u = get_nearest_point(new_pt);

            temp_pt = curve.evaluate(u);
            temp_pt_2 = new_pt;

            glutPostRedisplay();
        }
    }
    // are we near a point?
    // increase multiplicity of point
    // return

    // find nearest point on curve
    // near enough
    // yes: add point there
    // no: do nothing
}

void mouse_active_motion(int x, int y)
{
    Vector cursor(3);
    cursor[0] = x; 
    cursor[1] = height-y;
    cursor[2] = 0;

    Vector new_pt = get_world_coords(cursor);
    Vector cur_pt;
    Vector delta;

    switch(highlight_type)
    {
        case CURVE_PT:
            cur_pt = curve.get_curve_point(highlight_index);
            delta = new_pt - cur_pt;

            curve.set_curve_point(highlight_index, new_pt);

            // move handles accordingly
            cur_pt = curve.get_handle_point_1(highlight_index);
            curve.set_handle_point_1(highlight_index, cur_pt + delta);

            cur_pt = curve.get_handle_point_2(highlight_index);
            curve.set_handle_point_2(highlight_index, cur_pt + delta);

            redraw = 1;
            break;
        case HANDLE_PT_1:
            curve.set_handle_point_1(highlight_index, new_pt);
            if(!(mouse_modifiers & GLUT_ACTIVE_ALT))
            {
                curve.symmetrize_handle_2(highlight_index);
            }

            redraw = 1;
            break;
        case HANDLE_PT_2:
            curve.set_handle_point_2(highlight_index, new_pt);
            if(!(mouse_modifiers & GLUT_ACTIVE_ALT))
            {
                curve.symmetrize_handle_1(highlight_index);
            }
            redraw = 1;
            break;
    }

    return mouse_all_motion(x, y);
}

Vector get_world_coords(const Vector& pt)
{
    Vector result(3);
    gluUnProject(pt[0], pt[1], pt[2], modelview, projection, viewport, &result[0], &result[1], &result[2]);

    result[2] = 0;

    return result;
}

Vector get_screen_coords(const Vector& pt)
{
    Vector result(3);
    gluProject(pt[0], pt[1], pt[2], modelview, projection, viewport, &result[0], &result[1], &result[2]);

    result[2] = 0;

    return result;
}

void mouse_passive_motion(int x, int y)
{
    y = height - y;
    Vector cursor(3);
    cursor[0] = x; 
    cursor[1] = y;
    cursor[2] = 0;

    float closest = FLT_MAX;

    if(highlight_index >= 0) redraw = true;

    highlight_index = -1;
    highlight_type = NONE_PT;

    // see if we're near a point.
    for(int i = 0; i < curve.size(); i++)
    {
        Vector pt;
        pt = get_screen_coords(curve.get_curve_point(i));

        float sq_distance = (cursor - pt).magnitude_squared();

        if( sq_distance < DISTANCE_THRESHOLD_2)
        {
            if(sq_distance < closest)
            {
                highlight_index = i;
                highlight_type = CURVE_PT;
                closest = sq_distance;
                redraw = true;
            }
        }

        // handle 1
        pt = get_screen_coords(curve.get_handle_point_1(i));
        sq_distance = (cursor - pt).magnitude_squared();

        if( sq_distance < DISTANCE_THRESHOLD_2)
        {
            if(sq_distance < closest)
            {
                highlight_index = i;
                highlight_type = HANDLE_PT_1;
                closest = sq_distance;
                redraw = true;
            }
        }

        // handle 2
        pt = get_screen_coords(curve.get_handle_point_2(i));
        sq_distance = (cursor - pt).magnitude_squared();

        if( sq_distance < DISTANCE_THRESHOLD_2)
        {
            if(sq_distance < closest)
            {
                highlight_index = i;
                highlight_type = HANDLE_PT_2;
                closest = sq_distance;
                redraw = true;
            }
        }
    }
    
    return mouse_all_motion(x,y);
}

void mouse_all_motion(int x, int y)
{
    // is shift key down?
    if(mouse_modifiers & GLUT_ACTIVE_SHIFT)
    {
        // are we near a point?
        if(false)
        {
//            change cursor
        } else {
            // change cursor back
        }

    }

    if(redraw) glutPostRedisplay();
}

void init()
{
    Vector v(3, 0.);

    v[0] = 0.5; v[1] =  0.0; v[2] = 0.0;
    curve.insert_curve_point(0, v);
    v[0] = 1.0; v[1] =  0.5; v[2] = 0.0;
    curve.insert_curve_point(1, v);
    v[0] = 0.5; v[1] =  1.0; v[2] = 0.0;
    curve.insert_curve_point(2, v);
    v[0] = 0.0; v[1] =  0.5; v[2] = 0.0;
    curve.insert_curve_point(3, v);
    v[0] = 0.5; v[1] =  0.0; v[2] = 0.0;
    curve.insert_curve_point(4, v);

    v[0] = 0.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_1(0, v);
    v[0] = 1.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_1(1, v);
    v[0] = 1.0; v[1] =  1.0; v[2] = 0.0;
    curve.set_handle_point_1(2, v);
    v[0] = 0.0; v[1] =  1.0; v[2] = 0.0;
    curve.set_handle_point_1(3, v);
    v[0] = 0.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_1(4, v);

    v[0] = 1.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_2(0, v);
    v[0] = 1.0; v[1] =  1.0; v[2] = 0.0;
    curve.set_handle_point_2(1, v);
    v[0] = 0.0; v[1] =  1.0; v[2] = 0.0;
    curve.set_handle_point_2(2, v);
    v[0] = 0.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_2(3, v);
    v[0] = 1.0; v[1] =  0.0; v[2] = 0.0;
    curve.set_handle_point_2(4, v);

    gluNurbsProperty(curve.get_nurbs_object(), GLU_SAMPLING_METHOD, GLU_PATH_LENGTH);
    gluNurbsProperty(curve.get_nurbs_object(), GLU_SAMPLING_TOLERANCE, 1);

    // read image
    Image img("test.jpg");

//    glShadeModel(GL_FLAT);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // allocate a texture name
    glGenTextures( 1, &texture );
    int err = glGetError();
    if(err)
    {
        cout << gluErrorString(err) << endl;
        exit(1);
    }

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // the texture wraps over at the edges (repeat)
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                   GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                   GL_NEAREST);


//    gluBuild2DMipmaps(img);
    glTexImage2D(img);
}

void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

    // draw background image
    glEnable( GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture( GL_TEXTURE_2D, texture );
    glBegin(GL_QUADS);
    glTexCoord2d(0., 0.); glVertex3d(0., 0., .5);
    glTexCoord2d(1., 0.); glVertex3d(1., 0., .5);
    glTexCoord2d(1., 1.); glVertex3d(1., 1., .5);
    glTexCoord2d(0., 1.); glVertex3d(0., 1., .5);
    glEnd();
    glDisable( GL_TEXTURE_2D);

    // draw control point lines
    for(int i = 0; i < curve.size(); i++)
    {
        glColor3f(.7, .7, .7);
        glBegin(GL_LINES);
            glVertex(curve.get_curve_point(i));
            glVertex(curve.get_handle_point_1(i));
            glVertex(curve.get_curve_point(i));
            glVertex(curve.get_handle_point_2(i));
        glEnd();
    }

    glColor3f(.5,.5,.5);

    gluBeginCurve(curve.get_nurbs_object());
    curve.gl_call();
    gluEndCurve(curve.get_nurbs_object());


    glPointSize(5);
    // draw control points
    for(int i = 0; i < curve.size(); i++)
    {

        glColor3f(0, .7, .2);
        glBegin(GL_POINTS);
            glVertex(curve.get_curve_point(i));
            glVertex(curve.get_handle_point_1(i));
            glVertex(curve.get_handle_point_2(i));

            if(highlight_index == i)
            {
                glColor3f(.9, .9, 0.);

                switch(highlight_type)
                {
                    case CURVE_PT:
                        glVertex(curve.get_curve_point(i));
                        break;
                    case HANDLE_PT_1:
                        glVertex(curve.get_handle_point_1(i));
                        break;
                    case HANDLE_PT_2:
                        glVertex(curve.get_handle_point_2(i));
                        break;
                    default:
                        abort();
                }
            }
        glEnd();
    }

    // draw debugging point
    if(temp_pt.get_length() > 0)
    {
        glBegin(GL_POINTS);
            glColor3f(0., 0., 0.);
            glVertex(temp_pt);
            glVertex(temp_pt_2);
        glEnd();

        glBegin(GL_LINES);
            glVertex(temp_pt);
            glVertex(temp_pt_2);
        glEnd();
    }

    // show gradient around curve
    for(float i = 0; i < 1.; i += .01)
    {
        Vector v = curve.evaluate(i);
        Vector dv = curve.gradient(i);
//        std::cout << v.get_length() << std::endl;
//        std::cout << "c1(" << i << ") " << v[0] << ",\t" << v[1] << ",\t" << v[2] << std::endl;

//        glPointSize(5.);
//        glColor3f(i, 0, 0);
//        glBegin(GL_POINTS);
//        glVertex(v);
//        glEnd();

        glBegin(GL_LINES);
        glVertex(v);
        glVertex(v + .05 * dv);
        glEnd();
    }

    glFlush();

    glutSwapBuffers(); 

    redraw = false;


    int err = glGetError();
    if(err)
    {
        cout << gluErrorString(err) << endl;
        exit(1);
    }
}

void reshape(int w, int h)
{
    if(w == 0) w = 1;
    if(h == 0) h = 1;

    width = w;
    height = h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glViewport(0, 0, width, height);
    glOrtho(-0.1,1.1, -0.1, 1.1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
}



float get_nearest_point(const Vector& in_pt)
{
    // set endpoints to begin/end
    float NUM_SAMPLES = curve.size() * 10;

    float best_mag = FLT_MAX;
    float best_u = 0;

    float lower_bound = 0;
    float upper_bound = 0.999;
    float region_size = 0.999/NUM_SAMPLES;

    const int NUM_RECURSIONS = 5;
    for(int r = 0; r < NUM_RECURSIONS; r++)
    {
        for(int i = 0; i < NUM_SAMPLES; i++)
        {
            float u = lower_bound + i * region_size;

            Vector cur_pt = curve.evaluate(u);
            float cur_mag = (in_pt - cur_pt).magnitude_squared();

            if(cur_mag < best_mag)
            {
                best_u = u;
                best_mag = cur_mag;
            }
        }
        lower_bound = max(0.f, best_u - region_size);
        upper_bound = min(best_u + region_size, 0.999f);
        region_size = (upper_bound - lower_bound)/ NUM_SAMPLES;
    }

    return best_u;
}


