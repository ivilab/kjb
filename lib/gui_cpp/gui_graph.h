/* $Id: gui_graph.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_CPP_GUI_GRAPH_H
#define KJB_CPP_GUI_CPP_GUI_GRAPH_H

#ifdef KJB_HAVE_OPENGL

#include <gui_cpp/gui_overlay.h>
#include <m_cpp/m_vector_d.h>

#include <map>
#include <list>
#include <cmath>
#include <boost/array.hpp>
#include <boost/optional.hpp>

namespace kjb
{
namespace gui
{

class Plot : public Overlay
{
typedef Overlay Base;

private:
    // PRIVATE TYPES

    struct Data_range
    {
        Data_range()
        {
            reset();
        }

        float x_min;
        float x_max;
        float y_min;
        float y_max;
        bool initialized;

        /**
         * reset data range
         */
        void reset();

        void expand(const Data_range& r);

        void expand(float x, float y);
    };

    typedef std::map<double, double> Data_map;

    struct Data_set
    {
        Data_set() :
            data(),
            color(0.0, 0.0, 0.0),
            thickness(1.0),
            data_range_()
        {}

        template <class Iterator_x, class Iterator_y>
        void set(Iterator_x begin, Iterator_x end, Iterator_y ybegin)
        {
            data.clear();
            Iterator_y it_y = ybegin;
            for(Iterator_x it = begin; it != end; ++it, ++it_y)
            {
                data[*it] = *it_y;
            }
            update_data_range();
        }

        template <class Iterator_y>
        void set(Iterator_y ybegin, Iterator_y yend)
        {
            data.clear();
            size_t i = 0;
            for(Iterator_y it = ybegin; it != yend; ++it)
            {
                data[i] = *it;

                i++;
            }
            update_data_range(); 
        }

        void set(const Data_map& in)
        {
            data = in;
            update_data_range(); 
        }

        template <class Iterator_x, class Iterator_y>
        void append(Iterator_x begin, Iterator_x end, Iterator_y ybegin)
        {
            Data_range append_range;
            Iterator_y it_y = ybegin;
            for(Iterator_x it = begin; it != end; ++it, ++it_y)
            {
                data[*it] = *it_y;
                append_range.expand(*it, *it_y);
            }
            expand_data_range_(append_range); 
        }

        template <class Iterator_y>
        void append(Iterator_y ybegin, Iterator_y yend)
        {
            Data_range append_range;
            Iterator_y it_y = ybegin;
            size_t i;
            if(data.size() == 0)
                i = 0;
            else
                i = data.rbegin()->first + 1; // pick up where we left off

            for(Iterator_y it = ybegin; it != yend; ++it)
            {
                data[i] = *it_y;
                i++;
                append_range.expand(i, *it_y);
            }
            expand_data_range_(append_range); 
        }

        void append(const Data_map& in)
        {
            Data_range append_range;
            typedef Data_map::const_iterator Iterator;
            typedef Data_map::value_type Data_pair;

            for(Iterator it = in.begin(); it != in.end(); ++it)
            {
                data.insert(*it);
                append_range.expand(it->first, it->second);
            }

            expand_data_range_(append_range);
        }

        void render() const;

        const Data_range& get_data_range()
        {
            return data_range_;
        }

        void update_data_range();

        Data_map data;
        Vector3 color;
        float thickness;

    private:
        void expand_data_range_(const Data_range& range);

        Data_range data_range_;
    };

    
    // useful typedefs
    typedef std::list<Data_set> Data_sets;

    /**
     * Simple structure for storing a rectangular range defining the 
     * bounds of the data to be displayed.
     *
     * @invariant x_size > 0, y_size > 0
     */
    struct Axis
    {
        Axis();

        // returns whether this is valid
        operator bool() const
        {
            return (y_size > 0 && x_size > 0);
        }

        /** 
         * update x and y range to fit bounds of this data
         */
        template <class Iterator>
        void update_data_range(Iterator begin, Iterator end)
        {
            data_range_.reset();

            for(Iterator it = begin; it != end; ++it)
            {
                data_range_.expand(*it);
            }

            update_real_range();
        }

        /**
         * set axes
         */
        void set(float x_min_, float x_max_, float y_min_, float y_max_);

        /**
         * Update axes from data range.
         *
         * @param include_x_origin force x-axis to be included in plot
         * @param include_y_origin force y-axis to be included in plot
         */
        void auto_set(bool include_x_origin, bool include_y_origin);

        /**
         * Fixes current axes (disables auto mode)
         */
        void fixed()
        {
            auto_ = false;
        }

        /**
         * Enable automatic axis scalaing, and update axes
         *
         * @param include_x_origin force x-axis to be included in plot
         * @param include_y_origin force y-axis to be included in plot
         */
        void automatic(bool include_x_origin, bool include_y_origin)
        {
            auto_ = true;
            auto_include_x_ = include_x_origin;
            auto_include_y_ = include_y_origin;

            auto_set(auto_include_x_, auto_include_y_);
        }

        /**
         * Update hash spacing (e.g. when plot size changes).  sizes are specified
         * in normalized pane coordinates, i.e. size = a/b where a is the hash size
         * in pixels and b is the plot width in pixels.
         *
         * @param min_width minimum horizontal distance between hash marks, in normalized pane coordinates
         * @param min_height minimum vertical distance between hash marks, in normalized pane coordinates
         */
        void set_hash_spacing(float min_width, float min_height)
        {
            // in normalized coordinates, width of 0.5 means half the width
            // of the plot
            min_dx_normalized_ = min_width;
            min_dy_normalized_ = min_height;

            // need at least one subdivision
            if(min_dy_normalized_ > 1.0)
                min_dy_normalized_ = 1.0;

            if(min_dx_normalized_ > 1.0)
                min_dx_normalized_ = 1.0;

            update_ideal_range_x();
            update_ideal_range_y();
        }

        void update_real_range()
        {
            if(auto_)
                auto_set(auto_include_x_, auto_include_y_);
        }

        /**
         * This updates the "ideal" step-size in the x-direction
         * and might modify min/max of y-range so plot begins and ends
         * at clean values
         *
         * @pre real range is up-to-date
         */
        void update_ideal_range_y()
        {
            if(y_size < SIZE_EPSILON)
                return;

            // convert from normalized pane coordinates into axis-space coordinates
            float min_height = min_dy_normalized_ * y_size;

            dy = human_quantize_range_(y_min, y_max, auto_, 10, min_height);

            // range could have been re-quantized, so sizes need updating
            if(auto_)
            {
                y_size = y_max - y_min;

                assert(y_size >= 0);
                assert(y_size < DBL_MAX);
#ifdef TEST
                assert(y_size < 1e15);
#endif
            }
        }

        /**
         * This updates the "ideal" step-size in the x-direction
         * and might modify min/max of y-range so plot begins and ends
         * at clean values
         *
         * @pre real range is up-to-date
         */
        void update_ideal_range_x()
        {
            // TODO: this sucks because it's literally exactly the same as update dy, just with all the variables changed.
            // Best solution: refactor so Axis only represents ONE AXIS!
            // It was a really obviously dumb idea to conflate x and y axis,
            // not sure what I was thinking...
            //
            // ALSO this is the most bug-prone, tightly-coupled code in the entire file.
            // If there's a weird bug, its _definitely_ here.
            
            if(x_size < SIZE_EPSILON)
                return;

            // convert from normalized pane coordinates into axis-space coordinates
            float min_width = min_dx_normalized_ * x_size;

            // update dx and dy (and optionally *_min and *_max)
            //
            // if data range is sufficiently small, the quantize
            // method will fail, but since dx is ignored in this
            // case, we can just skip updating dx... 
            dx = human_quantize_range_(x_min, x_max, auto_, 10, min_width);

            // range could have been re-quantized, so sizes need updating
            if(auto_)
            {
                x_size = x_max - x_min;

                assert(x_size >= 0);
                assert(x_size < DBL_MAX);
            }
        }

        /** @brief Find a clean, human-friendly subdivision of axes */
        static double human_quantize_range_(float& xmin, float& xmax, bool quantize_bounds, size_t max_num_bins, float min_incr);

        /** 
         * Clamp range bounds to the nearest quantization boundary
         * that includes the old range.
         */
        static void quantize_range_(float& x_min, float& x_max, float incr)
        {
            x_min = std::floor(x_min / incr) * incr;
            x_max = std::ceil(x_max / incr) * incr;
        }

        /**
         * Return the axis region in axis coordinates.
         *
         * This is preferable to getting x_size and y_size 
         * directly, because it handles cases like horizontal
         * or nearly-horizontal lines
         */
        void get_view_range(
                float& left ,
                float& bottom,
                float& width,
                float& height,
                float& hash_dx,
                float& hash_dy) const;

    private:
        bool auto_;
        bool auto_include_x_;
        bool auto_include_y_;

        Data_range data_range_;

        float min_dx_normalized_;
        float min_dy_normalized_;

        float x_size;
        float y_size;
    public:
        float x_min;
        float x_max;
        float y_min;
        float y_max;

        float dx;
        float dy;

        static const float SIZE_EPSILON;
    };


public:
    typedef Data_sets::iterator Data_set_iterator;
    static const Vector3 DEFAULT_COLOR;
    Plot(float x, float y, float width, float height, const Vector3& bg_color = Vector3(1.0, 1.0, 1.0));

    void thin_mode()
    {
        thin_mode_ = true;
        update_data_pane_geometry();
    }

    void set_transparent_background()
    {
        bg_color_.reset();
    }

    virtual void set_size(int width, int height)
    {
        Base::set_size(width, height);
        update_geometry_();
    }

    virtual void set_position(int x, int y)
    {
        Base::set_position(x, y);
        update_geometry_();
    }

    /**
     * Create an empty dataset and return an iterator to it.
     */
    Data_set_iterator add_dataset(const Vector3& line_color = DEFAULT_COLOR);

    /**
     * Add dataset using a double->double map
     */
    Data_set_iterator add_dataset(const Data_map& data, const Vector3& line_color = DEFAULT_COLOR);

    /**
     * Update dataset using a double->double map
     */
    void update_dataset(Data_set_iterator data_set, const Data_map& data)
    {
        data_set->set(data);
        update_axis_data_range();
    }

    /**
     * Append to dataset using a double->double map
     */
    void append_dataset(Data_set_iterator data_set, const Data_map& data)
    {
        data_set->append(data);
        update_axis_data_range();
    }


    /**
     * Add dataset using a set if x and y values
     */
    template <class Iterator_x, class Iterator_y>
    Data_set_iterator add_dataset(Iterator_x begin, Iterator_x end, Iterator_y ybegin, const Vector3& line_color = DEFAULT_COLOR)
    {
        Vector3 color = line_color;
        if(color == DEFAULT_COLOR)
            color = COLOR_ORDER_[data_sets_.size() % COLOR_ORDER_.size()];

        data_sets_.push_front(Data_set());
        Data_set_iterator data_set = data_sets_.begin();

        data_set->color = color;
        update_dataset(data_set, begin, end, ybegin);
        return data_set;
    }

    /**
     * update dataset using a set if x and y values
     */
    template <class Iterator_x, class Iterator_y>
    void update_dataset(Data_set_iterator data_set, Iterator_x begin, Iterator_x end, Iterator_y ybegin)
    {
        data_set->set(begin, end, ybegin);
        update_axis_data_range();   
    }

    /**
     * Append dataset using a set if x and y values
     */
    template <class Iterator_x, class Iterator_y>
    void append_dataset(Data_set_iterator data_set, Iterator_x begin, Iterator_x end, Iterator_y ybegin)
    {
        data_set->append(begin, end, ybegin);
        update_axis_data_range();
    }

    /**
     * Add dataset using a set y values.  X values are assumed to be 0,1,2,...
     */
    template <class Iterator_y>
    Data_set_iterator add_dataset(Iterator_y ybegin, Iterator_y yend, const Vector3& line_color = DEFAULT_COLOR)
    {
        Vector3 color = line_color;
        if(color == DEFAULT_COLOR)
            color = COLOR_ORDER_[data_sets_.size() % COLOR_ORDER_.size()];

        data_sets_.push_front(Data_set());
        Data_set_iterator data_set = data_sets_.begin();

        data_set->color = color;
        update_dataset(data_set, ybegin, yend);
        return data_sets_.begin();
    }

    /**
     * Update dataset using a set y values.  X values are assumed to be 0,1,2,...
     */
    template <class Iterator_y>
    void update_dataset(Data_set_iterator data_set, Iterator_y ybegin, Iterator_y yend)
    {
        data_set->set(ybegin, yend);
        update_axis_data_range();
    }

    /**
     * append to dataset using a set y values.  X values are assumed to be 0,1,2,...
     */
    template <class Iterator_y>
    void append_dataset(Data_set_iterator data_set, Iterator_y ybegin, Iterator_y yend)
    {
        data_set->append(ybegin, yend);
        update_axis_data_range();
    }

    /**
     * Returns a handle to the i=th dataset
     */
    Data_set_iterator get_dataset(size_t index);

    void update_axis_data_range();

    /// Mark a specific point in the plot
    void set_mark(double x)
    {
        mark_ = x;
    }

    /// Remove mark from plot
    void unset_mark()
    {
        mark_ = boost::none;
    }

    /// Disable auto axis. Fix axes to their current range
    void fix_axis()
    {
        axis_.fixed();
    }

    /// Set data axes, similar to matlab's "axis" command.  Data points
    /// outside this range will be ignored
    void axis(float x_min, float x_max, float y_min, float y_max)
    {
        axis_.set(x_min, x_max, y_min, y_max);
    }

    /// return to using auto-scaled axis after having manually-set axis bounds
    void auto_axis(bool include_x = false, bool include_y = false)
    {
        axis_.automatic(include_x, include_y);
    }

    /// Set plot title
    void title(const std::string& title)
    {
        title_ = title;
        render_title_ = true;
        update_data_pane_geometry();
    }

    /// Set label for the x axis
    void xlabel(const std::string& label)
    {
        x_axis_label_ = label;
        render_x_axis_label_ = true;
        update_data_pane_geometry();
    }

    /// Set label for the y axis
    void ylabel(const std::string& label)
    {
        y_axis_label_ = label;
        render_y_axis_label_ = true;
        update_data_pane_geometry();
    }

    virtual void render() const;

    /// renders the unscaled data frame 
    /// Axes will be scaled to 1x1.
    /// Data may be rendered outside of the 1x1 square; Caller should use
    /// glScissor() to clip pixels outside this range.
    ///
    /// @post current color and modelview matrix will be modified.
    void render_data_frame() const;

    void box(bool enabled)
    {
        show_box_ = enabled;
    }

private:
    // PRIVATE METHODS
    static void draw_mark_(const Data_set& data_set, double x);

    static void draw_circle_(
            double x,
            double y,
            double radius,
            const Vector3& fill_color,
            const Vector3& line_color,
            size_t subdivisions = 10);
    
    void update_hash_mark_spacing();

    float data_pane_height() const
    {
        return data_pane_height_;
    }

    float data_pane_width() const
    {
        return data_pane_width_;
    }

    float data_pane_x_offset() const
    {
        return data_pane_x_offset_;
    }

    float data_pane_y_offset() const
    {
        return data_pane_y_offset_;
    }

    void update_geometry_()
    {
        update_data_pane_geometry();
    }

    void update_data_pane_geometry();
        
    static void centered_text_(const std::string& str, float x, float y) ;

    static void right_text_(const std::string& str, float x, float y) ;

    static void centered_vertical_text_(const std::string& str, float x, float y) ;

    
    /**
     * assum we're in screen coordinates, here
     */
    void draw_axes( const Axis& axis, bool full_box) const;


private:
    boost::optional<Vector3> bg_color_;
    std::list<Data_set> data_sets_;

    boost::optional<double> mark_;

    Axis axis_;
    bool auto_axis_;

    std::string title_;
    std::string y_axis_label_;
    std::string x_axis_label_;

    bool render_title_;
    bool render_y_axis_label_;
    bool render_x_axis_label_;

    float data_pane_x_offset_;
    float data_pane_y_offset_;

    float data_pane_height_;
    float data_pane_width_;

    bool show_box_;
    bool thin_mode_;

    static const float CHARACTER_WIDTH;
    static const float CHARACTER_HEIGHT;
    static const float DATA_PANE_PADDING;
    static const float TEXT_PADDING;
    static const float TITLE_HEIGHT;
    static const float X_AXIS_LABEL_HEIGHT;
    static const float Y_AXIS_LABEL_WIDTH;
    static const float HASH_LABEL_MAX_CHARS;
    static const float HASH_LABEL_WIDTH;
    static const float HASH_LABEL_HEIGHT;

#ifdef KJB_HAVE_GLUT
    static const void* CHARACTER_FONT;
#endif

    static const float MIN_HASH_LABEL_WIDTH;
    static const float MIN_HASH_LABEL_HEIGHT;
    static const float HASH_MARK_SIZE;

    static const boost::array<Vector3, 7> COLOR_ORDER_;
};
} // namespace gui
} // namespace kjb

#endif /* have_opengl */
#endif
