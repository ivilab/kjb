/* $Id: gui_text_overlay.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef GUI_CPP_GUI_TEXT_OVERLAY_H
#define GUI_CPP_GUI_TEXT_OVERLAY_H

#ifdef KJB_HAVE_GLUT

#include <string>
#include <gui_cpp/gui_overlay.h>
#include <boost/optional.hpp>
#include <m_cpp/m_vector_d.h>

namespace kjb
{
namespace gui
{

class Text_overlay : public Overlay
{
typedef Overlay Base;

enum Vertical_alignment {valign_top, valign_center, valign_bottom};
enum Horizontal_alignment {halign_left, halign_center, halign_right};

public:
    Text_overlay(float x, float y, float width);

    void size(float width)
    {
        Base::set_size(width, HEIGHT);
    }

    void background_color(float x, float y, float z, float a = 1.0)
    {
        bg_color_ = Vector4(x,y,z,a);
    }

    void color(float x, float y, float z, float a = 1.0)
    {
        color_ = Vector4(x,y,z,a);
    }

    void make_transparent()
    {
        bg_color_ = boost::none;
    }
    
    void set(const std::string& str)
    {
        text_ = str;
    }

    void align_left()
    {
        alignment_ = halign_left;
    }

    void align_right()
    {
        alignment_ = halign_right;
    }

    void align_center()
    {
        alignment_ = halign_center;
    }

//    void vertical_align_top()
//    {
//        v_alignment_ = valign_top;
//    }
//
//    void vertical_align_center()
//    {
//        v_alignment_ = valign_center;
//    }
//
//    void vertical_align_bottom()
//    {
//        v_alignment_ = valign_bottom;
//    }
//
    virtual void render() const;

private:
    std::string text_;
    Horizontal_alignment alignment_;
    Vertical_alignment v_alignment_;
    kjb::Vector4 color_;
    boost::optional<kjb::Vector4> bg_color_;

    static void* FONT;
    static const int CHAR_WIDTH = 8;
    static const int CHAR_HEIGHT = 13;
    static const int PADDING = 3;
    static const int HEIGHT = 2 * PADDING + CHAR_HEIGHT;

};


/**
 * Similar to Text_overlay, but with multi-line support.
 *
 * (Under development, vertical alignment and background mask
 *  not finished)
 */
class Textarea_overlay : public Overlay
{
typedef Overlay Base;

enum Vertical_alignment {valign_top, valign_center, valign_bottom};
enum Horizontal_alignment {halign_left, halign_center, halign_right};

public:
    Textarea_overlay(float x, float y, float width, float height) :
        Base(x, y, width, height),
        text_(""),
        alignment_(halign_left),
        v_alignment_(valign_bottom),
        color_(0.0,0.0,0.0,1.0),
        bg_color_()
    {}

    void size(float width, float height)
    {
        Base::set_size(width, height);
    }

    void background_color(float x, float y, float z, float a = 1.0)
    {
        bg_color_ = Vector4(x,y,z,a);
    }

    void color(float x, float y, float z, float a = 1.0)
    {
        color_ = Vector4(x,y,z,a);
    }

    void make_transparent()
    {
        bg_color_ = boost::none;
    }
    
    void set(const std::string& str)
    {
        text_ = str;
    }

    void align_left()
    {
        alignment_ = halign_left;
    }

    void align_right()
    {
        alignment_ = halign_right;
    }

    void align_center()
    {
        alignment_ = halign_center;
    }

//    void vertical_align_top()
//    {
//        v_alignment_ = valign_top;
//    }
//
//    void vertical_align_center()
//    {
//        v_alignment_ = valign_center;
//    }
//
//    void vertical_align_bottom()
//    {
//        v_alignment_ = valign_bottom;
//    }
//
    virtual void render() const;


private:
    std::string text_;
    Horizontal_alignment alignment_;
    Vertical_alignment v_alignment_;
    kjb::Vector4 color_;
    boost::optional<kjb::Vector4> bg_color_;

    static void* FONT;
    static const int CHAR_WIDTH = 8;
    static const int CHAR_HEIGHT = 13;
    static const int PADDING = 3;
    static const int LINE_HEIGHT = 2 * PADDING + CHAR_HEIGHT;

};
} // namespace gui
} // namespace kjb

#endif /* have_glut */
#endif
