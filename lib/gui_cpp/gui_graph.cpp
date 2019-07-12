/* $Id: gui_graph.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifdef KJB_HAVE_OPENGL
#include <iomanip>
#include <sstream>

#include <gui_cpp/gui_graph.h>
#include <m_cpp/m_vector_d.h>
#include <l_cpp/l_algorithm.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_opengl_color.h>

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/foreach.hpp>

using namespace kjb::opengl;

namespace kjb {
namespace gui {

const Vector3 Plot::DEFAULT_COLOR(-1.0, -1.0, -1.0);

// Stole these colors from matlab ;-)
const boost::array<Vector3, 7> Plot::COLOR_ORDER_ = boost::assign::list_of
    (Vector3(0.0, 0.0, 1.0))
    (Vector3(0.0, 0.5, 0.0))
    (Vector3(1.0, 0.0, 0.0))
    (Vector3(0.0, 0.75, 0.75))
    (Vector3(0.75, 0.0, 0.75))
    (Vector3(0.75, 0.75, 0.0))
    (Vector3(0.25, 0.25, 0.25));

// These affect the rendering geometry of the graph
const float Plot::CHARACTER_WIDTH = 8;
const float Plot::CHARACTER_HEIGHT = 13;
const float Plot::DATA_PANE_PADDING = 5;
const float Plot::TEXT_PADDING = 3;
const float Plot::TITLE_HEIGHT = CHARACTER_HEIGHT + TEXT_PADDING * 2;
const float Plot::X_AXIS_LABEL_HEIGHT = TITLE_HEIGHT;
const float Plot::Y_AXIS_LABEL_WIDTH = CHARACTER_WIDTH + TEXT_PADDING * 2;
const float Plot::HASH_LABEL_MAX_CHARS = 8; // -1.05e-01
const float Plot::HASH_LABEL_WIDTH = CHARACTER_WIDTH * HASH_LABEL_MAX_CHARS + TEXT_PADDING * 2;;
const float Plot::HASH_LABEL_HEIGHT = X_AXIS_LABEL_HEIGHT;

#ifdef KJB_HAVE_GLUT
//const void* Plot::CHARACTER_FONT = GLUT_BITMAP_8_BY_13;
const void* Plot::CHARACTER_FONT = GLUT_BITMAP_HELVETICA_10;
#endif

const float Plot::MIN_HASH_LABEL_WIDTH = HASH_LABEL_WIDTH;
const float Plot::MIN_HASH_LABEL_HEIGHT = HASH_LABEL_HEIGHT;
const float Plot::HASH_MARK_SIZE = 5;

const float Plot::Axis::SIZE_EPSILON = 200 * FLT_MIN;

void Plot::Data_range::reset()
{
    x_min = FLT_MAX;
    x_max = -FLT_MAX;
    y_min = FLT_MAX;
    y_max = -FLT_MAX;
    initialized = false;
}

void Plot::Data_range::expand(const Data_range& r)
{
    if(!r.initialized) return;
    if(r.x_min < x_min) x_min = r.x_min;
    if(x_max < r.x_max) x_max = r.x_max;
    if(r.y_min < y_min) y_min = r.y_min;
    if(y_max < r.y_max) y_max = r.y_max;

    assert(x_max - x_min < FLT_MAX);
    assert(y_max - y_min < FLT_MAX);
    assert(x_max - x_min > -FLT_MAX);
    assert(y_max - y_min > -FLT_MAX);

    initialized = true;
}

void Plot::Data_range::expand(float x, float y)
{
    if(x < x_min) x_min = x;
    if(x_max < x) x_max = x;
    if(y < y_min) y_min = y;
    if(y_max < y) y_max = y;

    assert(x_max - x_min < FLT_MAX);
    assert(y_max - y_min < FLT_MAX);
    assert(x_max - x_min > -FLT_MAX);
    assert(y_max - y_min > -FLT_MAX);

    initialized = true;
}


void Plot::Data_set::render() const
{
    GL_ETX();
    glColor(color);
    glLineWidth(thickness);

    typedef Data_map::const_iterator Iterator;
    typedef Data_map::value_type Value_pair;
    glBegin(GL_LINE_STRIP);

    // plot data
        for(Iterator it = data.begin(); it != data.end(); it++)
        {
            const Value_pair& pair = *it;
            glVertex2f(pair.first, pair.second);
        }

    glEnd();

    GL_ETX();

}

void Plot::Data_set::expand_data_range_(const Data_range& range) 
{
    data_range_.expand(range);
}


void Plot::Data_set::update_data_range() 
{
    data_range_.reset();
    typedef Data_map::const_iterator Iterator;
    for(Iterator it = data.begin(); it != data.end(); ++it)
    {
        data_range_.expand(it->first, it->second);
    }
}


Plot::Axis::Axis() :
    auto_(true),
    auto_include_x_(false),
    auto_include_y_(false),
    data_range_(),
    x_size(0),
    y_size(0),
    x_min(0),
    x_max(0),
    y_min(0),
    y_max(0)
{ }

void Plot::Axis::set(float x_min_, float x_max_, float y_min_, float y_max_)
{
    x_max = x_max_;
    y_max = y_max_;
    x_min = x_min_;
    y_min = y_min_;

    x_size = x_max - x_min;
    y_size = y_max - y_min;

    auto_ = false;
    assert(x_size >= 0);
    assert(y_size >= 0);
    assert(x_size < DBL_MAX);
    assert(y_size < DBL_MAX);
#ifdef TEST
    assert(y_size < 1e15); // test; delete me
    assert(x_size < 1e15); // test; delete me
#endif
}

void Plot::Axis::auto_set(bool include_x_axis, bool include_y_axis)
{
    if(!data_range_.initialized)
    {
        x_min = 0; x_max = 1;
        y_min = 0; y_max = 1;
        x_size = y_size = 1;
    }
    else
    {
        x_max = data_range_.x_max;
        y_max = data_range_.y_max;
        x_min = data_range_.x_min;
        y_min = data_range_.y_min;
    }

    if(include_x_axis)
    {
        x_max = std::max(x_max, 0.0f);
        x_min = std::min(x_min, 0.0f);
    }

    if(include_y_axis)
    {
        y_max = std::max(y_max, 0.0f);
        y_min = std::min(y_min, 0.0f);
    }

    x_size = x_max - x_min;
    y_size = y_max - y_min;

    assert(x_size >= 0);
    assert(y_size >= 0);
    assert(x_size < DBL_MAX);
    assert(y_size < DBL_MAX);

#ifdef TEST
    assert(y_size < 1e15); // test; delete me
    assert(x_size < 1e15); // test; delete me
#endif

    update_ideal_range_x();
    update_ideal_range_y();
}

/**
 * Find a quantization that splits this range into as many
 * bins as possible, with a maximum of 10.  Increment must be
 * a multiple of k*10^n, where n is an integer and k is one of 
 * {1, 2, 10}.  This gives quantizations that are "human-friendly",
 * hence the function's name.  Functionality was reverse-engineered
 * from matlab's default hash-mark spacing algorithm.
 *
 * @param xmin minimum of value range. Will be lowered to the nearest 
 *      value in the resulting quantization grid.
 * @param xmax maximum of value range.  Will be raised to the nearest
 *      value in the resulting quantization grid.
 * @param quantize_bounds Whether or not to modify xmin and xmax
 * @param min_incr Result must be at least as large as this
 *
 * @return The chosen quantization increment.
 */
double Plot::Axis::human_quantize_range_(
        float& xmin,
        float& xmax,
        bool quantize_bounds,
        size_t max_num_bins,
        float min_incr)
{
    float x_range = xmax - xmin;

    assert(x_range > 0);
    assert(x_range < DBL_MAX);

    // determine which decade the number falls in.
    // (This is equivalent to finding the value 'n' in the
    // description above)
    int decade = std::floor(log(x_range) / log(10));
    double scale = pow(10, decade);
    double incr;
    enum {BY_2, BY_5, BY_10} incr_type;

    assert(scale > 0);

    x_range /= scale;
    // x_range now in (1, 10].

    // determine which increment value will give a number
    // of bins as close as possible to 10 without exceeding it.
    // This becomes a piecewise constant function, implemented
    // here using 3 cases:
    if(x_range <= 2)
    {
        incr = 0.2 * scale;
        incr_type = BY_2;
    }
    else if(x_range <= 5)
    {
        incr = 0.5 * scale;
        incr_type = BY_5;
    }
    else // 5 < x_range <= 10
    {
        incr = 1.0 * scale;
        incr_type = BY_10;
    }

    // update range after snapping to grid
    if(quantize_bounds)
    {
        // lower x_min and increase x_max to fall on quantization
        // boundaries
        float new_x_min = xmin;
        float new_x_max = xmax;
        float old_range = new_x_max - new_x_min;
        quantize_range_(new_x_min, new_x_max, incr);

        x_range = new_x_max - new_x_min;
        assert(x_range >= old_range);
        // Quantizing can increase x_range, but only by
        // at most 2 * incr;  The new, larger range could
        // require more bins than allowed, but 
        // at worst, this bumps it up to the next incr level
    }

    assert(incr > 0);
    assert(x_range > 0);
    assert(x_range < DBL_MAX);

#ifdef TEST
    assert(x_range < 1e15);
    assert(incr < 1e25);
    assert(min_incr < 1e25);
    assert(max_num_bins > 0);
#endif

    size_t num_bins = x_range / incr;
    while(num_bins > max_num_bins || incr < min_incr)
    {
#ifdef TEST
        double old_incr = incr;
#endif

        // keep bumping up to next increment level 
        switch(incr_type)
        {
            case BY_2:
                incr = 0.5 * scale;
                incr_type = BY_5;
                break;
            case BY_5:
                incr = 1.0 * scale;
                incr_type = BY_10;
                break;
            case BY_10:
                scale *= 10;
                incr = 0.2 * scale;
                incr_type = BY_2;
                break;
            default:
                abort();
                break;
        }

#ifdef TEST
        assert(x_range < 1e15);
        assert(incr < 1e10);
#endif

        // re-quantize the range
        if(quantize_bounds)
        {
            float new_x_min = xmin;
            float new_x_max = xmax;

#ifdef TEST
            double old_x_range = new_x_max - new_x_min;
#endif
            quantize_range_(new_x_min, new_x_max, incr);

            x_range = new_x_max - new_x_min;
#ifdef TEST
            assert(x_range >= old_x_range);
#endif
        }

#ifdef TEST
        assert(incr > old_incr);
        assert(x_range < DBL_MAX);
        assert(x_range > DBL_MIN);
#endif

        num_bins = x_range / incr;
    }

    // quantize input range
    if(quantize_bounds)
    {
        quantize_range_(xmin, xmax, incr);
    }

    return incr;
}


void Plot::Axis::get_view_range(
        float& left ,
        float& bottom,
        float& width,
        float& height,
        float& hash_dx,
        float& hash_dy) const
{
    if(x_size < SIZE_EPSILON)
    {
//        set range to [-1:0.2:1] + x_min
        left = x_min - 1;
        width = 2;
        hash_dx = 0.2;
        float trash;
        quantize_range_(bottom, trash, hash_dy);
    }
    else
    {
        left = x_min;
        width = x_size;
        hash_dx = dx;
    }

    if(y_size < SIZE_EPSILON)
    {
//        set range to [-1:0.2:1] + y_min

        // TODO: really, ought to be storing these values in
        // y_min, y_max, y_size, etc. instead of hacking it here.
        bottom = y_min - 1;
        height = 2;
        hash_dy = 0.2;
        float trash;
        quantize_range_(bottom, trash, hash_dy);
    }
    else
    {
        bottom = y_min;
        height = y_size;
        hash_dy = dy;
    }
}

Plot::Plot(float x, float y, float width, float height, const Vector3& bg_color) :
    Base(x, y, width, height),
    bg_color_(bg_color),
    data_sets_(),
    mark_(),
    axis_(),
    auto_axis_(true),
    title_(),
    y_axis_label_(),
    x_axis_label_(),
    render_title_(false),
    render_y_axis_label_(false),
    render_x_axis_label_(false),
    data_pane_x_offset_(),
    data_pane_y_offset_(),
    data_pane_height_(),
    data_pane_width_(),
    show_box_(true),
    thin_mode_(false)
{
    update_data_pane_geometry();
}


Plot::Data_set_iterator Plot::add_dataset(const Vector3& line_color)
{
    Vector3 color = line_color;
    if(color == DEFAULT_COLOR)
        color = COLOR_ORDER_[data_sets_.size() % COLOR_ORDER_.size()];

    data_sets_.push_front(Data_set());
    return data_sets_.begin();
}

Plot::Data_set_iterator Plot::add_dataset(const Data_map& data, const Vector3& line_color)
{
    Vector3 color = line_color;
    if(color == DEFAULT_COLOR)
        color = COLOR_ORDER_[data_sets_.size() % COLOR_ORDER_.size()];

    data_sets_.push_front(Data_set());
    Data_set_iterator data_set = data_sets_.begin();

    data_set->color = color;
    update_dataset(data_set, data);
    return data_set;
}

Plot::Data_set_iterator Plot::get_dataset(size_t index)
{
    if(index >= data_sets_.size())
        KJB_THROW(Index_out_of_bounds);

    Data_sets::iterator  it = data_sets_.begin();
    std::advance(it, data_sets_.size() - index - 1);
    return it;
}

void Plot::update_axis_data_range()
{
    axis_.update_data_range(
        boost::make_transform_iterator(data_sets_.begin(), 
            boost::bind(&Data_set::get_data_range, _1)),
        boost::make_transform_iterator(data_sets_.end(), 
            boost::bind(&Data_set::get_data_range, _1)));
}

void Plot::render() const
{
    GL_ETX();

    glPushAttrib(GL_ENABLE_BIT); // lighting  
    glPushAttrib(GL_CURRENT_BIT); // color

    glDisable(GL_LIGHTING);

    // render background
    if(bg_color_)
    {
        glColor(*bg_color_);
        glBegin(GL_QUADS);
        glVertex2f(x(), y());
        glVertex2f(x() + width(), y());
        glVertex2f(x() + width(), y() + height());
        glVertex2f(x(), y() + height());
        glEnd();
    }


    // Render text
    glColor3f(0.1, 0.1, 0.1);
    if(!thin_mode_ && render_title_)
    {
        centered_text_(
                title_,
                x() + width() / 2.0,
                y() + height() - TITLE_HEIGHT + TEXT_PADDING);
    }

    if(!thin_mode_ && render_y_axis_label_)
    {
        centered_vertical_text_(y_axis_label_, x() + TEXT_PADDING, y() + height()/2.0);
    }

    if(!thin_mode_ && render_x_axis_label_)
    {
        centered_text_(
                x_axis_label_,
                x() + width() / 2.0,
                y() + TEXT_PADDING);
    }


    // only render data if there's room
    if(data_pane_width() > 0 && data_pane_height() > 0)
    {
        glPushMatrix();
        glPushAttrib(GL_SCISSOR_BIT);
        glEnable(GL_SCISSOR_TEST);
        // rendering occurs in a 1x1 frame, so we need to blow it up to fit
        glTranslatef(x() + data_pane_x_offset(), y() + data_pane_y_offset(), 1.0);
        glScalef(data_pane_width(), data_pane_height(), 1.0);

        glScissor(x() + data_pane_x_offset(),
                y() + data_pane_y_offset(), 
                data_pane_width(), 
                data_pane_height());

        render_data_frame();

        glPopAttrib();
        glPopMatrix();

        // draw axes around plot
        if(!thin_mode_)
            draw_axes(axis_, show_box_);
    }



    glPopAttrib();
    glPopAttrib();

    GL_ETX();
}

void Plot::render_data_frame() const
{
    GL_ETX();

    // convert to "Axis space"
    // All displayable points can be rendered using their own coordinates
    float a_left, a_bottom, a_width, a_height, a_dx, a_dy;
    axis_.get_view_range(a_left, a_bottom, a_width, a_height, a_dx, a_dy);
    glScalef(1/a_width, 1/a_height, 1.0);
    glTranslatef(-a_left, -a_bottom, 0.0);

    // render data sets
    BOOST_FOREACH(const Data_set& data_set, data_sets_)
    {
        data_set.render();
    }

    // render data sets
    if(mark_)
    {
        BOOST_FOREACH(const Data_set& data_set, data_sets_)
        {
            draw_mark_(data_set, *mark_);
        }
    }

    GL_ETX();
}

void Plot::draw_mark_(const Data_set& data_set, double x)
{
    static const Vector3 MARK_POINT_COLOR(0.8, 0.2, 0.2);
    static const Vector3 MARK_LINE_COLOR(MARK_POINT_COLOR);

    double y;
    try
    {
        y = kjb::lerp(data_set.data, x);
    }
    catch(Index_out_of_bounds&)
    {
        return;
    }

    glPushAttrib(GL_CURRENT_BIT);
    glPushAttrib(GL_POINT_BIT);
    glPointSize(5.0);
    glColor(MARK_POINT_COLOR);
    glBegin(GL_POINTS);
        glVertex2f(x,y);
    glEnd();

    glColor(MARK_LINE_COLOR);
    glBegin(GL_LINES);
        glVertex2f(x,0);
        glVertex2f(x,y);
    glEnd();

    glPopAttrib();
    glPopAttrib();
}

void Plot::draw_circle_(
        double x,
        double y,
        double radius,
        const Vector3& fill_color,
        const Vector3& line_color,
        size_t subdivisions)
{
    static const float t_incr = M_PI / subdivisions;

    glPushAttrib(GL_CURRENT_BIT);

    glTranslated(x, y, 0);
    glScaled(radius, radius, 1.0);

    glColor(fill_color);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0, 0.0);

    for(size_t i = 0; i < 2; i++)
    {

        for(float t = 0; t <= 2 * M_PI + t_incr; t += t_incr)
        {
            glVertex2f(cos(t), sin(t));
        }
        glEnd();

        // render again, but now do the outline
        glColor(line_color);
        glBegin(GL_LINES);
    }
    glEnd();

    glPopAttrib();
}

void Plot::update_hash_mark_spacing()
{
    // convert pixel widths into normalized pane coordinates
    axis_.set_hash_spacing(
        MIN_HASH_LABEL_WIDTH / data_pane_width(),
        MIN_HASH_LABEL_HEIGHT / data_pane_height());
}

void Plot::update_data_pane_geometry()
{
    data_pane_x_offset_ = 0;
    data_pane_y_offset_ = 0;

    data_pane_height_ = height();
    data_pane_width_ = width();

    // margin
    if(!thin_mode_)
    {
        data_pane_x_offset_ = DATA_PANE_PADDING;
        data_pane_y_offset_ = DATA_PANE_PADDING;

        data_pane_height_ -= 2.0 * DATA_PANE_PADDING;
        data_pane_width_ -= 2.0 * DATA_PANE_PADDING;

        // hash-mark labels
        data_pane_y_offset_ += HASH_LABEL_HEIGHT;
        data_pane_height_ -= HASH_LABEL_HEIGHT;

        data_pane_x_offset_ += HASH_LABEL_WIDTH;
        data_pane_width_ -= HASH_LABEL_WIDTH;

        if(render_title_)
        {
            data_pane_height_ -= TITLE_HEIGHT;
        }

        if(render_y_axis_label_)
        {
            data_pane_width_ -= Y_AXIS_LABEL_WIDTH;
            data_pane_x_offset_ += Y_AXIS_LABEL_WIDTH;
        }

        if(render_x_axis_label_)
        {
            data_pane_height_ -= X_AXIS_LABEL_HEIGHT;
            data_pane_y_offset_ += X_AXIS_LABEL_HEIGHT;
        }
    }

    // data pane may have shrunk, needed fewer hash marks
    update_hash_mark_spacing();
}

void Plot::centered_text_(const std::string& str, float x, float y) 
{
#ifdef KJB_HAVE_GLUT
    glRasterPos2f(x - str.size() * CHARACTER_WIDTH/2.0, y);
    for(size_t i = 0; i < str.length(); i++)
    {
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
    }
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

void Plot::right_text_(const std::string& str, float x, float y) 
{
#ifdef KJB_HAVE_GLUT
    glRasterPos2f(x - str.size() * CHARACTER_WIDTH, y);
    for(size_t i = 0; i < str.length(); i++)
    {
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
    }
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

void Plot::centered_vertical_text_(const std::string& str, float x, float y) 
{
#ifdef KJB_HAVE_GLUT
    float cur_y = y + str.size() * CHARACTER_HEIGHT / 2.0;
    for(size_t i = 0; i < str.length(); i++)
    {
        glRasterPos2f(x, cur_y);
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
        cur_y -= CHARACTER_HEIGHT + 1;
    }
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

void Plot::draw_axes( const Axis& axis, bool full_box) const
{
    GL_ETX();
    glColor(BLACK);

// DRAW AXIS LINES
    // adding 0.5 so lines render through center of pixels.
    // Failing to do this results in lines falling on the boundary
    // between two pixels and they "jitter" when you move the 
    // window around.
    float left = x() + data_pane_x_offset() + 0.5;
    float right = left + data_pane_width();
    float bottom = y() + data_pane_y_offset() + 0.5;
    float top = bottom + data_pane_height();

    if(full_box)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2f(left, top);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glEnd();
    }
    else
    {
        glBegin(GL_LINE_STRIP);
        glVertex2f(left, top);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glEnd();
    }

    GL_ETX();
    // convert axis coordinates to screen coordinates
    float axis_left, axis_bottom, axis_width, axis_height, axis_dx, axis_dy;
    axis.get_view_range(axis_left, axis_bottom, axis_width, axis_height, axis_dx, axis_dy);
    float axis_to_screen_x = data_pane_width() / axis_width;
    float axis_to_screen_y = data_pane_height() / axis_height;

    float dx = axis_dx * axis_to_screen_x;
    float dy = axis_dy * axis_to_screen_y;

    // if data pane is zero pixels in size, dx/dy will be zero, and
    // the loops below won't 
    if(dx < 1.0) dx = 1.0;
    if(dy < 1.0) dy = 1.0;

// DRAW HASH MARKS
    std::string label;
    glBegin(GL_LINES);
        for(float x = left; x < right; x += dx)
        {
            glVertex2f(x, bottom);
            glVertex2f(x, bottom + HASH_MARK_SIZE);
        }

        for(float y = bottom; y < top; y += dy)
        {
            glVertex2f(left, y);
            glVertex2f(left + HASH_MARK_SIZE, y);
        }

        // put hash marks on right and top of box, too
        if(full_box)
        {
            for(float x = left; x < right; x += dx)
            {
                glVertex2f(x, top);
                glVertex2f(x, top- HASH_MARK_SIZE);
            }

            for(float y = bottom; y < top; y += dy)
            {
                glVertex2f(right, y);
                glVertex2f(right - HASH_MARK_SIZE, y);
            }
        }
    glEnd();

    // RENDER LABELS
    // TODO: This will fail when axes don't start or 
    // end on a hash-line.
    size_t count = 0;
    if(right > left)
    {
        assert(dx > 0);
        count = (right - left)/dx + FLT_EPSILON * dx;
    }

    for(size_t i = 0; i <= count; ++i)
    {
        using std::ios;
        std::ostringstream ost;
        ost.precision(2);
        float label_value = axis_left + i * axis_dx;
        if(axis_width < 0.1 || axis_width > 100)
            ost.setf(std::ios::scientific, std::ios::floatfield);
        ost << label_value;
        label = ost.str();
        centered_text_(label, left + dx * i, bottom - CHARACTER_HEIGHT - TEXT_PADDING);
    }
    
    count = 0;
    if(top > bottom)
    {
        assert(dy > 0);
        count = (top-bottom)/dy + FLT_EPSILON * dy;
    }
    
    for(size_t i = 0; i <= count; ++i)
    {
        using std::ios;
        std::ostringstream ost;
        ost.precision(2);
        float label_value = axis_bottom + i * axis_dy;
        if(axis_height < 0.1 || axis_height > 100)
            ost.setf(std::ios::scientific, std::ios::floatfield);
        ost << label_value;
        label = ost.str();
        right_text_(label, left - TEXT_PADDING, bottom + i * dy);
    }

    GL_ETX();
}




} // namespace gui
} // namespace kjb
#endif /* have_opengl */
