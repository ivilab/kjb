/* $Id */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
|  Author: Jinyan Guan
* =========================================================================== */

#include <edge_cpp/collinear_segment_chain.h>
#include <sstream>
#include <algorithm>

namespace kjb {

/**
 * @brief   Get end-point with lowest y-value.
 */
inline
const Vector& get_bottom_y(const Line_segment& seg)
{
    return seg.get_start_y() < seg.get_end_y() ? seg.get_start() : seg.get_end();
}

/**
 * @brief   Get end-point with highest y-value.
 */
inline
const Vector& get_top_y(const Line_segment& seg)
{
    return seg.get_start_y() > seg.get_end_y() ? seg.get_start() : seg.get_end();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Collinear_segment_chain::Collinear_segment_chain
(
    const std::vector<Line_segment>& segments 
):  
    m_segments(segments)
{
    IFT(!segments.empty(), Illegal_argument,
        "Cannot create collinear segment chain from empty segment range.");

    double min_x = std::numeric_limits<double>::max();
    double max_x = -std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_y = -std::numeric_limits<double>::max();
    const Vector* min_x_pt = NULL;
    const Vector* max_x_pt = NULL;
    const Vector* min_y_pt = NULL;
    const Vector* max_y_pt = NULL;
    for(size_t i = 0; i < m_segments.size(); i++)
    {
        const Line_segment& cur_seg = m_segments[i];
        if(cur_seg.get_start_x() < min_x)
        {
            min_x = cur_seg.get_start_x();
            min_x_pt = &cur_seg.get_start();
        }

        if(cur_seg.get_end_x() > max_x)
        {
            max_x = cur_seg.get_end_x();
            max_x_pt = &cur_seg.get_end();
        }

        const Vector& bot_y_pt = get_bottom_y(cur_seg);
        double bot_y = bot_y_pt[1];
        if(bot_y < min_y)
        {
            min_y = bot_y;
            min_y_pt = &bot_y_pt;
        }

        const Vector& top_y_pt = get_top_y(cur_seg);
        double top_y = top_y_pt[1];
        if(top_y > max_y)
        {
            max_y = top_y;
            max_y_pt = &top_y_pt;
        }
    }

    if(max_x - min_x > max_y - min_y)
    {
        init_from_end_points((*min_x_pt)[0], (*min_x_pt)[1],
                             (*max_x_pt)[0], (*max_x_pt)[1]);
    }
    else
    {
        init_from_end_points((*min_y_pt)[0], (*min_y_pt)[1],
                             (*max_y_pt)[0], (*max_y_pt)[1]);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Collinear_segment_chain::read(std::istream& in)
{
    Line_segment::read(in);
  
    using std::istringstream;
    const char* field_value;
    size_t num_segments;

    if(!(field_value = read_field_value(in, "num_segments")))
    {
        KJB_THROW_2(Illegal_argument,
                    "Collinear line segment set, Could not read number of segments");
    }
    istringstream ist(field_value);
    ist >> num_segments;
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment set");
    }
    ist.clear(std::ios_base::goodbit);

    m_segments.clear();
    for(size_t i = 0; i < m_segments.size(); i++)
    {
        m_segments[i].read(in);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Collinear_segment_chain::write(std::ostream& out) const
{
    
    Line_segment::write(out);
    out << "num_segments:" << m_segments.size() << '\n';
    for(size_t i = 0; i < m_segments.size(); i++)
    {
        m_segments[i].write(out);
    }

}

} //namespace kjb

