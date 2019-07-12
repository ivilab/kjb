/* $Id: gui_text_overlay.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifdef KJB_HAVE_GLUT

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gui_cpp/gui_text_overlay.h>

namespace kjb
{
namespace gui
{

void* Text_overlay::FONT = GLUT_BITMAP_8_BY_13;
void* Textarea_overlay::FONT = GLUT_BITMAP_8_BY_13;

Text_overlay::Text_overlay(float x, float y, float width) :
    Base(x, y, width, HEIGHT),
    text_(""),
    alignment_(halign_left),
    v_alignment_(valign_bottom),
    color_(0.0,0.0,0.0,1.0),
    bg_color_()
{}

void Text_overlay::render() const
{
    GLboolean scissor_state;
    glGetBooleanv(GL_SCISSOR_TEST, &scissor_state);
    if(!scissor_state)
        glEnable(GL_SCISSOR_TEST);

    int begin_x;

    int string_width = text_.size() * CHAR_WIDTH;

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    switch(alignment_)
    {
        case halign_left:
            begin_x = x() + PADDING;
            break;
        case halign_right:
            begin_x = x() + PADDING + width() - string_width;
            break;
        case halign_center:
            begin_x = x() + PADDING + (width() - string_width)/2.0;
            break;
        default:
            KJB_THROW(Cant_happen_exception);
    }

//        switch(v_alignment_)
//        {
//            case valign_bottom:
//                begin_y = y() + PADDING;
//                break;
//            case valign_center:
//                begin_y = y() + PADDING - CHAR_HEIGHT/2;
//                break;
//            case valign_top:
//                begin_y = y() - PADDING - CHAR_HEIGHT;
//                break;
//            default:
//                KJB_THROW(Cant_happen_exception);
//        }
//
    glScissor(x(), y(), width(), height());

    if(bg_color_)
    {
        glBegin(GL_QUADS);
        kjb::opengl::glColor(*bg_color_);
        glVertex2f(begin_x - PADDING, y());
        glVertex2f(begin_x + PADDING + string_width, y());
        glVertex2f(begin_x + PADDING + string_width, y() + height());
        glVertex2f(begin_x - PADDING, y() + height());
        glEnd();
    }

    kjb::opengl::glColor(color_);
    glRasterPos2f(begin_x, y() + PADDING);

    for(size_t i = 0; i < text_.size(); i++)
    {
        glutBitmapCharacter(FONT, text_[i]);
    }

    if(!scissor_state)
        glDisable(GL_SCISSOR_TEST);
    glPopAttrib();
}

void Textarea_overlay::render() const
{
//        GLboolean scissor_state;
//        glGetBooleanv(GL_SCISSOR_TEST, &scissor_state);
//        if(!scissor_state)
//            glEnable(GL_SCISSOR_TEST);

    int begin_x;

    int string_width = text_.size() * CHAR_WIDTH;
//        int string_height = 1+std::count(text_.begin(), text_.end(), '\n');

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    switch(alignment_)
    {
        case halign_left:
            begin_x = x() + PADDING;
            break;
        case halign_right:
            begin_x = x() + PADDING + width() - string_width;
            break;
        case halign_center:
            begin_x = x() + PADDING + (width() - string_width)/2.0;
            break;
        default:
            KJB_THROW(Cant_happen_exception);
    }

//        begin_y = y() + PADDING;
//        switch(v_alignment_)
//        {
//            case valign_bottom:
//                begin_y = y() + PADDING;
//                break;
//            case valign_center:
//                begin_y = y() + PADDING - CHAR_HEIGHT/2;
//                break;
//            case valign_top:
//                begin_y = y() - PADDING - CHAR_HEIGHT;
//                break;
//            default:
//                KJB_THROW(Cant_happen_exception);
//        }

//        glScissor(x(), y(), width(), height());

    if(bg_color_)
    {
        // TODO - handle this
        glBegin(GL_QUADS);
        kjb::opengl::glColor(*bg_color_);
        glVertex2f(begin_x - PADDING, y());
        glVertex2f(begin_x + PADDING + string_width, y());
        glVertex2f(begin_x + PADDING + string_width, y() + height());
        glVertex2f(begin_x - PADDING, y() + height());
        glEnd();
    }

    kjb::opengl::glColor(color_);
    float cur_y = height() - CHAR_HEIGHT - PADDING;
    glRasterPos2f(begin_x, cur_y);

    for(size_t i = 0; i < text_.size(); i++)
    {
        if(text_[i] == '\n')
        {
            cur_y -= (CHAR_HEIGHT + PADDING * 0.5);
            glRasterPos2f(begin_x, cur_y + PADDING);
        }
        else
            glutBitmapCharacter(FONT, text_[i]);
    }

//        if(!scissor_state)
//            glDisable(GL_SCISSOR_TEST);
    glPopAttrib();
}


} // namespace gui
} // namespace kjb

#endif
