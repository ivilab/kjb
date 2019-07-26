/* $Id: i_colormap.h 16764 2014-05-09 23:17:12Z ksimek $ */
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

#ifndef KJB_I_CPP_I_COLORMAP_H
#define KJB_I_CPP_I_COLORMAP_H

#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <l_cpp/l_algorithm.h>
#include <i/i_float.h>
#include <l_cpp/l_exception.h>
#include <i_cpp/i_pixel.h>

namespace kjb
{

/**
 * @addtogroup kjbImageProc
 * @{
 */

class Colormap
{
public:

    Colormap(const std::string& name = "jet", int n = 64)
    {
        preset(name, n);
    }

    Colormap(const char* name, int n = 64)
    {
        preset(name, n);
    }

    void preset(const std::string& name, int n = 64)
    {
        if(presets_.count(name) == 0)
        {
            KJB_THROW_3(Illegal_argument, "Unknown colormap preset: %s", (name.c_str()));
        }

        colors_ = presets_[name](n);
    }

    /**
     * Convert value in [0.0, 1.0] to corresponding color in map.
     * Values will be clamped to [0.0, 1.0]
     */
    PixelRGBA operator()(double x) const
    {
        return lerp(colors_.begin(), colors_.end(), x);
    }

    size_t size() const
    {
        return colors_.size();
    }

    typedef std::vector<PixelRGBA> (*Preset_map_func)(int);

    static std::vector<PixelRGBA> jet(int n);
    static std::vector<PixelRGBA> hot(int n);
    static std::vector<PixelRGBA> hsv(int n);
    static std::vector<PixelRGBA> gray(int n);
    static std::vector<PixelRGBA> cool(int n);
    static std::vector<PixelRGBA> lines(int n);
    static std::map<std::string, Preset_map_func> presets_;

    std::vector<PixelRGBA> colors_;
};

/// @}

} // namespace kjb
#endif
