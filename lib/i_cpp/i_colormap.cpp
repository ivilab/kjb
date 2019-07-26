/* $Id: i_colormap.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "i_cpp/i_colormap.h"
#include "i_cpp/i_hsv.h"
#include "l_cpp/l_algorithm.h"

#include <boost/assign/list_of.hpp>

namespace kjb
{

std::map<std::string, Colormap::Preset_map_func>
Colormap::presets_ = boost::assign::map_list_of<std::string, Colormap::Preset_map_func>
    ("jet", Colormap::jet)
    ("hot", Colormap::hot)
    ("hsv", Colormap::hsv)
    ("gray", Colormap::gray)
    ("cool", Colormap::cool)
    ("lines", Colormap::lines)
    ;

std::vector<PixelRGBA> Colormap::jet(int n)
{
    if(n == 0)
        return std::vector<PixelRGBA>();
    if(n == 1)
    {
        return std::vector<PixelRGBA>(1, PixelRGBA(0, 0, 127.5, 255.0));
    }
    else if (n > 1)
    {
        std::vector<PixelRGBA> map(n);
        std::vector<double> range(n);
        linspace( 0.0, 1.0, n, range.begin());

        for(int i = 0; i < n; i++)
        {
            double x = range[i];
            float& r = map[i].r;
            r = 0.0;
            if(x >= 3./8 && x < 5./8)
                r += (4 * x - 3./2);
            if(x >= 5./8 && x < 7./8)
                r += 1.0;
            if(x >= 7./8)
                r += (-4 * x + 9./2);
            r *= 255.;


            float& g = map[i].g;
            g = 0.0;
            if(x >= 1./8 && x < 3./8)
                g += (4 * x - 1./2);
            if(x >= 3./8 && x < 5./8) 
                g += 1.0;
            if(x >= 5./8 && x < 7./8)
                g += (-4 * x + 7./2);
            g *= 255;

            float& b = map[i].b;
            b = 0.0;
            if(x < 1./8)
                b += (4 * x + 1./2);
            if(x >= 1./8 && x < 3./8)
                b += 1.0;
            if(x >= 3./8 && x < 5./8)
                b += (-4 * x + 5./2);
            b *= 255;

            map[i].extra.alpha = 255;
        }
        return map;
    }

    abort(); // can't get here
}

std::vector<PixelRGBA> Colormap::hot(int n)
{
    if(n == 0)
        return std::vector<PixelRGBA>();
    if(n == 1)
    {
        return std::vector<PixelRGBA>(1, PixelRGBA(0, 0, 0.0, 255.0));
    }
    else if (n > 1)
    {
        std::vector<PixelRGBA> map(n);
        std::vector<double> range(n);
        linspace( 0.0, 1.0, n, range.begin());

        for(int i = 0; i < n; i++)
        {
            double x = range[i];
            float& r = map[i].r;
            r = 0.0;
            if(x < 2./5)
                r += (5./2 * x);
            if(x >= 2./5)
                r += 1.0;
            r *= 255;



            float& g = map[i].g;
            g = 0.0;
            if(x >= 2./5 && x < 4./5) 
                g += (5./2 * x - 1);
            if(x >= 4./5)
                g += 1.0;
            g *= 255;


            float& b = map[i].b;
            b = 0.0;
            if(x >= 4./5)
                b += (5*x - 4);
            b *= 255;

            map[i].extra.alpha = 255;
        }
        return map;
    }

    abort(); // can't get here

}

std::vector<PixelRGBA> Colormap::hsv(int n)
{
    if(n == 0)
        return std::vector<PixelRGBA>();
    if(n == 1)
    {
        return std::vector<PixelRGBA>(1, PixelRGBA(255.0, 0, 0.0, 255.0));
    }
    else if (n > 1)
    {
        std::vector<PixelRGBA> map(n);
        std::vector<double> range(n);
        linspace( 0.0, 1.0, n, range.begin());

        for(int i = 0; i < n; i++)
        {
            double x = range[i];
//            map[i] = static_cast<kjb_c::Pixel>(PixelHSVA(x, 1.0, 1.0));
            PixelHSVA color(x, 1.0, 1.0);
            map[i] = PixelRGBA(color.r, color.g, color.b);
        }
        return map;
    }

    abort(); // can't get here
}

std::vector<PixelRGBA> Colormap::gray(int n)
{
    if(n == 0)
        return std::vector<PixelRGBA>();
    else
    {
        std::vector<PixelRGBA> map(n);
        std::vector<double> range(n);
        linspace( 0.0, 255.0, n, range.begin());

        for(int i = 0; i < n; i++)
        {
            double x = range[i];
            map[i] = PixelRGBA(x, x, x);
        }
        return map;
    }

    abort(); // can't get here

}

std::vector<PixelRGBA> Colormap::lines(int n)
{
    std::vector<PixelRGBA> map(n);
    int i = 0;

    while(true)
    {
        map[i].r = 0.0; map[i].g = 0.0; map[i].b = 255.0; i++;
        if(i >= n) break;
        map[i].r = 0.0; map[i].g = 127.0; map[i].b = 0.0; i++;
        if(i >= n) break;
        map[i].r = 255.0; map[i].g = 0.0; map[i].b = 0.0; i++;
        if(i >= n) break;
        map[i].r = 0.0; map[i].g = 191.0; map[i].b = 191.0; i++;
        if(i >= n) break;
        map[i].r = 191.0; map[i].g = 0.0; map[i].b = 191.0; i++;
        if(i >= n) break;
        map[i].r = 191.0; map[i].g = 191.0; map[i].b = 0.0; i++;
        if(i >= n) break;
        map[i].r = 63.0; map[i].g = 63.0; map[i].b = 63.0; i++;
        if(i >= n) break;
    }

    return map;
}

std::vector<PixelRGBA> Colormap::cool(int n)
{

    if(n == 0)
        return std::vector<PixelRGBA>();
    if(n == 1)
    {
        return std::vector<PixelRGBA>(1, PixelRGBA(0, 255.0, 255.0, 255.0));
    }
    else if (n > 1)
    {
        std::vector<PixelRGBA> map(n);
        std::vector<double> range(n);
        linspace( 0.0, 1.0, n, range.begin());

        for(int i = 0; i < n; i++)
        {
            double x = range[i];
            float& r = map[i].r;
            r = x;
            r *= 255;

            float& g = map[i].g;
            g = 255 - r;

            float& b = map[i].b;
            b = 255;

            map[i].extra.alpha = 255.0;
        }
        return map;
    }

    abort(); // can't get here
}

} // namespace kjb
