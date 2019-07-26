
/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Line segment class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#include "edge_cpp/corner.h"
#include <sstream>

using namespace kjb;

/** @brief Reads this Line segment from an input stream. */
void Corner::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    // Line segment centre
    if (!(field_value = read_field_value(in, "position")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Corner Position");
    }
    istringstream ist(field_value);
    ist >> position(0) >> position(1) >> position(2);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid corner position");
    }
    ist.clear(std::ios_base::goodbit);

    unsigned int num_segments;

    // Line segment number of segments
    if (!(field_value = read_field_value(in, "num_segments")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Corner Number of Segments");
    }
    ist.str(field_value);
    ist >> num_segments;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Corner number of segments");
    }
    ist.clear(std::ios_base::goodbit);

    for(unsigned int i = 0; i < num_segments; i++)
    {
        segments.push_back(Line_segment(in));
    }

}

/** @brief Writes this Line segment to an output stream. */
void Corner::write(std::ostream& out) const
{
    out <<  "position" << position(0) << " " << position(1) << " " << position(2) << "\n" <<
            " num_segments: " << segments.size() << "\n";
    for(unsigned int i = 0; i < segments.size(); i++)
    {
        segments[i].write(out);
    }
}

/** @brief Draws this line segment */
void Corner::draw( kjb::Image & img, double ir, double ig, double ib, double width)  const
{
    for(unsigned int i = 0; i < segments.size(); i++)
    {
        segments[i].draw(img, ir, ig, ib, width);
    }
}

/** @brief Randomly colors this line segment on an image */
void Corner::randomly_color(kjb::Image & img, double width)  const
{
    for(unsigned int i = 0; i < segments.size(); i++)
    {
        segments[i].randomly_color(img, width);
    }
}
