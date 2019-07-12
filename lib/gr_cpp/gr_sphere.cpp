#include "gr_cpp/gr_sphere.h"
#include <iostream>
#include <fstream>
#include "m_cpp/m_matrix_stream_io.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_camera.h"
#include "gr_cpp/gr_offscreen.h"
#include "i_cpp/i_image.h"


using namespace kjb;

#ifdef KJB_HAVE_OPENGL
GLfloat sphere_light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat sphere_light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light */
#endif /* KJB_HAVE_OPENGL */

Sphere::Sphere(kjb::Vector & c, double radius){
    this->radius = radius;
    center = kjb::Vector(3);
    center = center.set(c.at(0),c.at(1),c.at(2));
}

const kjb::Vector & Sphere::GetCenter() const
{
    return center;
}

double Sphere::GetRadius() const
{
    return radius;
}

void Sphere::drawSphere(void)
{
        
#ifdef KJB_HAVE_OPENGL
        GLUquadricObj* sphere = NULL;
        sphere = gluNewQuadric();
        glColor3f(0,0,1); //color red
        gluQuadricDrawStyle(sphere, GLU_FILL);
        gluQuadricTexture(sphere, TRUE);
        gluQuadricNormals(sphere, GLU_SMOOTH);
        gluQuadricOrientation(sphere,GLU_OUTSIDE);
       
    const Vector & c = this->GetCenter();
 
        glTranslatef(c.at(0),c.at(1),c.at(2));
    gluSphere(sphere, radius, 20, 20);
    glTranslatef(-c.at(0),-c.at(1),-c.at(2));
#else
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif
}

void
Sphere::init(void)
{  
#ifdef KJB_HAVE_OPENGL
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sphere_light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, sphere_light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
#else
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif
}

/* disable main method for lib src
int m(int argc, char **argv){
    
    
    Offscreen_buffer * offscreen =  kjb::create_and_initialize_offscreen_buffer(500, 500);
    
    init();
    Perspective_camera camera;
    camera.set_camera_centre_z(5.0);
    camera.set_focal_length(800);
    camera.prepare_for_rendering(true);
    display();
    
    Image ki;
    Base_gl_interface::capture_gl_view(ki);
    ki.write("test.jpg");
    
    delete offscreen;
    return 0;            
    
    
}*/

