/* $Id: edge.h 11995 2012-03-29 20:19:58Z ksimek $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#ifndef KJB_CPP_SEGMENT_PAIR_H
#define KJB_CPP_SEGMENT_PAIR_H

#include <l/l_incl.h>
#include <edge/line_segment.h>
#include <i_cpp/i_image.h>
#include <string>

namespace kjb
{

class Segment_pair
{
public:
    Segment_pair(const Line_segment & is1, const Line_segment & is2) : ls1(is1), ls2(is2)
    {

    }

    Segment_pair(const Segment_pair & src) : ls1(src.ls1), ls2(src.ls2)
    {

    }

    Segment_pair & operator=(const Segment_pair & src)
    {
        ls1 = src.ls1;
        ls2 = src.ls2;
        return (*this);
    }

    ~Segment_pair()
    {

    }

    const Line_segment & get_segment1() const
    {
        return ls1;
    }

    const Line_segment & get_segment2() const
    {
        return ls2;
    }

    void draw(Image & img, double ir = 255.0, double ig = 0.0, double ib = 0.0) const
    {
        ls1.draw(img, ir, ig, ib);
        ls2.draw(img, ir, ig, ib);
    }

    void draw_extremities(Image & img, double ir = 255.0, double ig = 0.0, double ib = 0.0) const
    {
        Vector v1 = get_extremity_1();
        Vector v2 = get_extremity_2();
        img(v1(1), v1(0), 0) = 255.0;
        img(v1(1), v1(0), 1) = 0.0;
        img(v1(1), v1(0), 2) = 0.0;
        img(v2(1), v2(0), 0) = 255.0;
        img(v2(1), v2(0), 1) = 0.0;
        img(v2(1), v2(0), 2) = 0.0;
    }

    const Vector & get_extremity_1() const
    {
        if(ls1.get_start_y() > ls1.get_end_y())
        {
            return ls1.get_start();
        }
        return ls1.get_end();
    }

    const Vector & get_extremity_2() const
    {
        if(ls2.get_start_y() > ls2.get_end_y())
        {
            return ls2.get_start();
        }
        return ls2.get_end();
    }



private:
    Line_segment ls1;
    Line_segment ls2;

    int lowest;

}; // namespace kjb
}
#endif
