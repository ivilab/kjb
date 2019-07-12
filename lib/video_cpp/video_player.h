/* $Id: video_player.h 18791 2015-04-07 18:51:57Z ernesto $ */
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
#ifndef KJB_CPP_GR_PLAYER_H
#define KJB_CPP_GR_PLAYER_H

#ifdef KJB_HAVE_OPENGL

#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif

#include <gr_cpp/gr_opengl.h>
#include <i_cpp/i_image.h>
#include <video_cpp/video.h>

namespace kjb
{
    /**
     * A simple function that renders a video frame to opengl
     */
    void render_video_frame(const Video& video, size_t frame, bool reset_pos = true)
    {
        size_t num_cols = kjb::opengl::get_viewport_width();
        size_t num_rows = kjb::opengl::get_viewport_height();
        if(num_cols != video.get_width() || num_rows != video.get_height()) return;

        if(reset_pos) glRasterPos2f(0, 0);
        glDrawPixels(num_cols, num_rows, GL_RGB, GL_UNSIGNED_BYTE, video.get_buffer(frame));

        GL_ETX();
    }

/**
 * A rudimentary video player in GLUT/OpenGL, indended as a base class for
 * custom interactive visualizations.  Subclass and re-implement the
 * render() method to draw custom video overlays.
 *
 * Implements movie decoding with FFMPEG, play/pause, and stepping forward/reverse.
 *
 * This won't display anything until a Glut window is attached and glutMainLoop is called. Example
 *
 * <code>
 *     Player player;
 *     player.load("movie.avi");
 *     Glut_window wnd;
 *     player.attach(wnd);
 *     glutMainLoop();
 * </code>
 *
 * @TODO: write() method for saving visualizations as compressed movie files.
 *
 * @Note: This class reads the entire video uncompressed pixels into memory.  As a result, it probably isn't ideal for movies longer than 20-30 seconds.  Someday might implement lazy loading of frames (i.e. only decompress a frame when you're ready to display it).  This requires more knowledge of FFMPEG than I have time for at the moment.
 *
 * @author Kyle Simek.
 */
class Video_player
{
typedef Video_player Self;

struct null_deleter
{
    void operator()(void const*) const
    { }
};

public:
    Video_player() :
        video_(),
        cur_frame_(0),
        playing_(false),
        timer_registered_(false)
    {}

    Video_player(const std::string& movie_fname) :
        video_(),
        cur_frame_(0),
        playing_(false),
        timer_registered_(false)
    {
        load(movie_fname);
    }

    Video_player(const std::vector<std::string>& fnames) :
        video_(),
        cur_frame_(0),
        playing_(false),
        timer_registered_(false)
    {
        load(fnames, 30.0);
    }

    const Video& get_video() const
    {
        return *video_;
    }

    void load(const std::string& movie_fname)
    {
        boost::shared_ptr<Video> tmp(new Video());
        video_ = tmp;

        video_->decode_video(movie_fname);
    }

    void load(const std::vector<std::string>& fnames, float fps)
    {
        boost::shared_ptr<Video> tmp(new Video());
        video_ = tmp;
        video_->load_images(fnames, fps);
    }

    void set(boost::shared_ptr<Video> video)
    {
        video_ = video;
    }

    void set(Video* video)
    {
        video_ = boost::shared_ptr<Video>(video, null_deleter());
    }

#ifdef KJB_HAVE_GLUT
    void attach(kjb::opengl::Glut_window& wnd)
    {
        using namespace boost;
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        wnd.set_display_callback(bind(&Self::display, this));
        wnd.set_special_callback(bind(&Self::special_key, this, _1, _2, _3));
        wnd.set_keyboard_callback(bind(&Self::key, this, _1, _2, _3));
        wnd.set_size(video_->get_width(), video_->get_height());

    }
#endif

    void render_image(size_t frame) const
    {
        render_video_frame(*video_, frame);
    }

#ifdef KJB_HAVE_GLUT
    void display() const
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        render();
        glutSwapBuffers();
    }
#endif

    virtual void render() const
    {
        render_image(cur_frame_);
    }

    void special_key(int k, int, int)
    {
#ifdef KJB_HAVE_GLUT
        switch(k)
        {
            case GLUT_KEY_UP:
                cur_frame_ = (cur_frame_ + 10) % video_->size();
                break;
            case GLUT_KEY_DOWN:
                cur_frame_ = (cur_frame_ + video_->size() - 10) % video_->size();
                break;
            case GLUT_KEY_RIGHT:
                cur_frame_ = (cur_frame_ + 1) % video_->size();
                break;
            case GLUT_KEY_LEFT:
                cur_frame_ = (cur_frame_ + video_->size() - 1) % video_->size();
                break;
            default:
                return;
                break;
        }
        redisplay();
#else
        KJB_THROW_2(Missing_dependency, "glut");
#endif
    }

    void init_timer()
    {
#ifdef KJB_HAVE_GLUT
        if(timer_registered_ == false)
        {
            float period = 1000.0 / video_->get_frame_rate();
            kjb::opengl::Glut::add_timer_callback(period, boost::bind(&Self::timer, this));
            timer_registered_ = true;
        }
#else
        KJB_THROW_2(Missing_dependency, "glut");
#endif
    }

    void redisplay()
    {
#ifdef KJB_HAVE_GLUT
        glutPostRedisplay();
#endif
    }

    void timer()
    {
        timer_registered_ = false;
        if(playing_)
        {
            advance_frame();
            redisplay();
            init_timer();
        }
    }

    void advance_frame()
    {
        cur_frame_ = (cur_frame_ + 1) % video_->size();
    }

    void play()
    {
        playing_ = true;
        init_timer();
    }

    void key(unsigned int k, int, int)
    {
        switch(k)
        {
            case 27: // ESC
            case 'q':
            case 'Q':
                exit(0);
                break;
            case ' ':
                playing_ = !playing_;
                if(playing_)
                {
                    init_timer();
                }
                break;
            default:
                break;
        }
    }

    int current_frame_index() const
    {
        return cur_frame_;
    }

    boost::shared_ptr<kjb::Video> video_;

    int cur_frame_;
        
    bool playing_;
    bool timer_registered_;
};


} // namespace kjb
#endif /* HAVE_OPENGL */
#endif
