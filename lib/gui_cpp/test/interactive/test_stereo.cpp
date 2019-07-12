#include <gui_cpp/gui_viewer.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>

void display()
{

    kjb::opengl::Teapot teapot;

    glPushMatrix();
    glScalef(0.1, 0.1, 0.1);
    teapot.render();

    glTranslatef(2, 0, -0.5);
    teapot.render();
    glTranslatef(-4, 0, 1.0);
    teapot.render();
    glPopMatrix();
}

int main()
{
    try
    {
        kjb::opengl::Glut_window wnd;
        kjb::gui::Viewer viewer(500, 500);
        //kjb::gui::Viewer viewer();
        viewer.attach(wnd);


        // 3650: desktop 23" monitor
//        viewer.enable_horizontal_stereo(2.0, 0.03, 1.87);
        viewer.enable_anaglyph_stereo(0.9, 0.06, 3650);
//        viewer.enable_anaglyph_stereo(2.0, 0.06, 0.325);

        viewer.add_render_callback(display);

        //viewer.display();

        glutMainLoop();
    }
    catch(kjb::Exception& ex)
    {
        std::cout << ex.get_msg() << std::endl;
        return 1;
    }
    return 0;
}

