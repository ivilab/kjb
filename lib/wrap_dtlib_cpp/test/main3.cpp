#include <wrap_dtlib_cpp/texture.h>
#include <wrap_dtlib_cpp/textonhisto.h>
#include <glut_cpp/glut_parapiped.h>
#include <gr_cpp/gr_glut.h>
#include <iostream>

using namespace DTLib;
using namespace kjb;

kjb::Image * img2 = NULL;
kjb::Image * img = NULL;

int selected_x = 0;
int selected_y = 0;
bool changed_selection = false;
bool color_selected = false;
bool show_original = true;
int padding = 10;

CImg<FloatCHistogramPtr>* chi = NULL;
CImg<FloatCHistogramPtr>* thi = NULL;

static void display_glut()
{
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(changed_selection)
    {
        changed_selection = false;
        if(color_selected)
        {
            PrepareHeatMap(*img2, chi, padding, selected_x, selected_y);
        }
        else
        {
            PrepareHeatMap(*img2, thi, padding, selected_x, selected_y);
        }
    }

    if(show_original)
    {
        Base_gl_interface::set_gl_view(*img);
    }
    else
    {
        Base_gl_interface::set_gl_view(*img2);
    }
    glFlush();
    glutSwapBuffers();
}

static void keyboard_glut(unsigned char key, int a, int b)
{
    if(key == 'c')
    {
        color_selected = true;
        changed_selection = true;
        std::cout << "Switch to color" << std::endl;
    }
    else if(key == 't')
    {
        color_selected = false;
        changed_selection = true;
        std::cout << "Switch to texture" << std::endl;
    }
    else if(key == 'i')
    {
        show_original = true;
    }
    else if(key == 'h')
    {
        show_original = false;
    }
}

static void glutMouse(int button, int state, int x, int y)
{
    if( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
    {
        selected_x = x;
        selected_y = y;
        changed_selection = true;
        show_original = false;
        std::cout << "Selected pixel at (row, col)=(" << selected_y << "," << selected_x << ")" << std::endl;
        std::cout << "Pixel value: (" << (*img)(y,x,0) << "," << (*img)(y,x,1) << "," << (*img)(y,x,2)  << ")"<< std::endl;
        int x_position = selected_x - padding;
        int y_position = selected_y - padding;
        if((x_position < 0) || (y_position < 0))
        {
            std::cout << "Requested pixel is part of the padding" << std::endl;
            return;
        }

        if((x_position >= chi->Width()) || (y_position >= chi->Height()))
        {
            std::cout << "Requested pixel is part of the padding" << std::endl;
            return;
        }

        int position = (y_position*chi->Width()) + x_position;
        FloatCHistogramPtr ppHisto = chi->GetPix(position);
        if(!color_selected)
        {
            ppHisto = thi->GetPix(position);
        }
        ppHisto->Write(std::cout);
    }
}

static void timer_glut(int ignored)
{
    glutPostRedisplay();
    glutTimerFunc(30, timer_glut, 0);
}

int main(int argc, char ** argv)
{
    if(argc != 4)
    {
        std::cout << "Usage: ./main2 <base_dir> <out_dir> <image_name>" << std::endl;
        return 1;
    }
    std::string img_name(argv[1]);
    img_name.append("/");
    img_name.append(argv[3]);
    img_name.append(".jpg");
    std::string ochi(argv[1]);
    ochi.append("/histos/");
    ochi.append(argv[3]);
    std::string othi(ochi);
    ochi.append("_chi");
    othi.append("_thi");

    img = new kjb::Image(img_name.c_str());

    CImg<FloatCHistogramPtr> * m_pTextonHistoImg = NULL;
    CImg<FloatCHistogramPtr> * ColorHist = NULL;
    othi.append("_bin");
    ochi.append("_bin");

    thi = ReadTextonHistoImg(othi);
    chi = ReadColorHistoImg(ochi);

    img2 = new kjb::Image(img->get_num_rows(), img->get_num_cols(), 0, 0, 0);
    //PrepareHeatMap(img2, chi, 10, 67, 67);
    //img2.write("aaa.jpg");

    kjb::opengl::Glut::set_init_window_size(img->get_num_cols(), img->get_num_rows());
    kjb::opengl::Glut::init(argc, argv);
    glutCreateWindow("Textons");
    glutDisplayFunc(display_glut);
    glutKeyboardFunc(keyboard_glut);
    glutMouseFunc(glutMouse);
    glutTimerFunc(50, timer_glut, 0);

    glutMainLoop();

    zap(m_pTextonHistoImg);
    zap(ColorHist);

    return 0;
}
