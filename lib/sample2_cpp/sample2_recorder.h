/* $Id: sample2_recorder.h 18301 2014-11-26 19:17:13Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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

#include <gui_cpp/gui_graph.h>
#include <sample_cpp/sample_recorder.h>
#include <sample_cpp/sample_base.h>
#include <gui_cpp/gui_viewer.h>
#include <gui_cpp/gui_graph.h>
#include <gr_cpp/gr_opengl.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


#ifdef KJB_HAVE_OPENGL
template <class Model>
class Target_plot_recorder
{
private:
    Target_plot_recorder(const Target_plot_recorder&){}
    Target_plot_recorder& operator=(const Target_plot_recorder&){ return *this;}
public:
    typedef Model Model_type;
    typedef kjb::gui::Plot Value_type;
    typedef std::map<double, double> Data_map;

    const kjb::gui::Plot& get() const{ return plot_;}

    Target_plot_recorder(size_t width, size_t height) :
        wnd_(),
        viewer_(),
        plot_(0,0,width, height),
        cull_begin_(0),
        cull_end_(0)
    {
        typedef kjb::opengl::Glut_window Glut_window;

        kjb::opengl::Glut::push_current_window();

        wnd_ = boost::shared_ptr<Glut_window>(new Glut_window(width, height, "Target Distribution vs. Iteration"));
        wnd_->set_current();

        viewer_ = boost::make_shared<kjb::gui::Viewer>(width, height);

        viewer_->attach(*wnd_);
        viewer_->attach_overlay(&plot_);
        dataset_ = plot_.add_dataset(Data_map());
        plot_.auto_axis(false, true);
        GL_ETX();

        kjb::opengl::Glut::pop_current_window();
    }

    void include_y_origin(bool b) 
    {
        plot_.auto_axis(false, b);
    }

    /**
     * Limit the number of elements in the plot by deleting old
     * elements.  Deleting elements requires re-calculation of the
     * data-range, so optional hysteresis can reduce the amount of
     * deletes that occur
     *
     * @param cull_begin When the number of elements exceeds this number, old elements will be deleted.  Set to zero to disable culling.
     * @param cull_end Delete enough elements so the size falls below this number.  Setting to zero or cull_begin will delete exactly one element every iteration, otherwise (cull_begin-cull_end) elements will be deleted.
     */
    void set_culling(size_t cull_begin, size_t cull_end = 0)
    {
        cull_begin_ = cull_begin;
        if(cull_end == 0)
            cull_end_ = cull_begin_;
        else
            cull_end_ = cull_end;
    }

    void operator()(const Model& model, const Step_log<Model>& step_log) const
    {
        typedef Data_map::iterator Iterator;
        // almost always, step_log will have only one entry.
        // In the rare case when there's multiple, this will only
        // take the last one
        double lt = step_log.back().lt;

        // append one value
        plot_.append_dataset(
                dataset_,
                &lt,
                &lt + 1);

        if(cull_begin_ && dataset_->data.size() > cull_begin_)
        {
            Data_map& data = dataset_->data;
            // UNTESTED
            int remove_count = (int) data.size() - (int) cull_end_;
            Iterator remove_begin = data.begin();
            Iterator remove_end = data.begin();
            std::advance(remove_end, remove_count);

            Iterator it = remove_begin;
            while(it != remove_end)
                data.erase(it++);
            dataset_->update_data_range();
        }

        GL_ETX();
        wnd_->redisplay();
        glFlush();
        GL_ETX();
    }

private:
    boost::shared_ptr<kjb::opengl::Glut_window> wnd_;
    boost::shared_ptr<kjb::gui::Viewer> viewer_;
    mutable kjb::gui::Plot plot_;
    kjb::gui::Plot::Data_set_iterator dataset_;
    size_t cull_begin_;
    size_t cull_end_;
};
#endif  /* KJB_HAVE_OPENGL */
