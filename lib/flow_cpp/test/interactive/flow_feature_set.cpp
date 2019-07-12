#include <l/l_sys_io.h>

#include <flow_cpp/flow_feature_set.h>
#include <flow_cpp/flow_visualizer.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>

#include <string>
#include <vector>

using namespace kjb;
using namespace kjb::opengl;
using namespace std; 

double percentile; 
Image* img_pre_p; 
Image* img_next_p;
const Image* cur_img_p;

Flow_feature_set flow_features; 
Axis_aligned_rectangle_2d box; 

const double win_width = 500;
const double win_height = 500;

/** @brief  Display scene. */
void display();

/** @brief  Reshape scene. */
void reshape(int, int);

/** @brief  Handle key input. */
void handle_key(unsigned char, int, int);

void unstandardize
(
    Axis_aligned_rectangle_2d& box, 
    int img_width,
    int img_height
);

void standardize
(
    Vector& v,
    int img_width,
    int img_height
);

int main(int argc, char** argv)
{
    try
    {
        if(argc != 4)
        {
            cout << "Usuage: " << argv[0] << " image-1 image-2 optical-flow-file \n";
            return EXIT_SUCCESS;
        }
        // initialize state
        percentile = 1.0;
        img_pre_p = new Image(argv[1]);
        img_next_p = new Image(argv[2]);
        cur_img_p = img_pre_p;

        string flow_fp(argv[3]);
        flow_features = read_flow_features(flow_fp);

        box.set_center(Vector(0.0, 0.0));
        box.set_width(100.0);
        box.set_height(100.0);

        // GL stuff
        Glut_window win(img_pre_p->get_num_cols(), img_pre_p->get_num_rows());
        win.set_display_callback(display);
        win.set_reshape_callback(reshape);
        win.set_keyboard_callback(handle_key);

        glutMainLoop();
    }
    catch(const kjb::Exception& ex)
    {
        delete img_pre_p;
        delete img_next_p;

        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display()
{
    // clear stuff
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // prepare for 2d rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int w = img_pre_p->get_num_cols();
    int h = img_pre_p->get_num_rows();
    glOrtho(-w/2.0, w/2.0, -h/2.0, h/2.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    set_framebuffer(*cur_img_p);

    // Draw the bounding box
    glBegin(GL_LINE_LOOP);
    glVertex2d(box.get_left(), box.get_top());
    glVertex2d(box.get_right(), box.get_top());
    glVertex2d(box.get_right(), box.get_bottom());
    glVertex2d(box.get_left(), box.get_bottom());
    glEnd();


    // Look up the features
    Axis_aligned_rectangle_2d roi(box);
    unstandardize(roi, w, h);
    vector<Feature_pair> fv = look_up_features(flow_features, roi);
    vector<Vector> vfv = valid_flow(fv, percentile); 
    Vector avf = average_flow(vfv);

    // Draw the features 
    if(fv.size() > 0)
    {
        Vector cur_feature_center(2, 0.0);
        for(size_t i = 0; i < fv.size(); i++)
        {
            Vector cur_feature = fv[i].first;
            Vector next_feature = fv[i].second; 

            standardize(cur_feature, w, h);
            standardize(next_feature, w, h);

            cur_feature_center += cur_feature;

            glColor3d(0.0, 0.0, 1.0);
            glBegin(GL_LINE_STRIP);
                glVertex2d(cur_feature(0), cur_feature(1));
                glVertex2d(next_feature(0), next_feature(1));
            glEnd();
            glPointSize(5.0f); 
            glBegin(GL_POINTS);
                glVertex2d(next_feature(0), next_feature(1));
            glEnd();
        }

        cur_feature_center /= fv.size();
        Vector end_point = cur_feature_center + avf;

        // Draw the average flow vector
        glColor3d(1.0, 0.0, 0.0);
        glBegin(GL_LINE_STRIP);
            glVertex2d(cur_feature_center(0), cur_feature_center(1));
            glVertex2d(end_point(0), end_point(1));
        glEnd();

        glPointSize(5.0f); 
        glBegin(GL_POINTS);
            glVertex2d(end_point(0), end_point(1));
        glEnd();
    }

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_MODELVIEW);
}

void handle_key(unsigned char key, int, int)
{
    switch(key)
    {
        case 'q':
        {
            delete img_pre_p; 
            delete img_next_p; 
            exit(EXIT_SUCCESS);
            break; 
        }
        case 'w':
        {
            box.set_width(box.get_width() + 10.0);
            break;
        }
        case 'W':
        {
            box.set_width(box.get_width() - 10.0);
            break;
        }
        case 'e':
        {
            box.set_height(box.get_height() + 10.0);
            break;
        }
        case 'E':
        {
            box.set_height(box.get_height() - 10.0);
            break;
        }
        case 'l': 
        {
            box.set_centre_x(box.get_centre_x() + 10.0);
            break;
        }
        case 'h': 
        {
            box.set_centre_x(box.get_centre_x() - 10.0);
            break;
        }
        case 'k': 
        {
            box.set_centre_y(box.get_centre_y() + 10.0);
            break;
        }
        case 'j': 
        {
            box.set_centre_y(box.get_centre_y() - 10.0);
            break;
        }
        case 'n': 
        {
            if(cur_img_p == img_pre_p)
                cur_img_p = img_next_p;
            else
                cur_img_p = img_pre_p;
            break;
        }
        case 'p':
        {
            percentile -= 0.1;
            if(percentile < 0.0) percentile = 0.0;
            break;
        }
        case 'P':
        {
            percentile += 0.1;
            if(percentile > 1.0) percentile = 1.0; 
            break;
        }
    }

    glutPostRedisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void unstandardize
(
    Axis_aligned_rectangle_2d& box, 
    int img_width,
    int img_height
)
{
    Vector t(img_width/2.0, img_height/2.0);
    Vector s(1.0, -1.0);
    scale(box, s); 
    translate(box, t);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void standardize
(
    Vector& v, 
    int img_width,
    int img_height
)
{
    Vector t(-img_width/2.0, -img_height/2.0);
    v += t; 
    v[1] = -v[1];
}
