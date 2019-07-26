/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line segment class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef EDGE_CORNER_H
#define EDGE_CORNER_H

#include "l_cpp/l_exception.h"
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>
#include "i_cpp/i_image.h"
#include "m_cpp/m_vector.h" 
#include "edge_cpp/line_segment.h"
#include <limits>

namespace kjb {

/**
 * @class Corner
 *
 * @brief Class to manipulate a 2D corner.
 * The corener is defined in terms of a set of
 * line segments all intersecting at a point in
 * the image, which is the corner position.
 * No consistency controls are performed here
 */
class Corner : public Readable, public Writeable
{
    public:

        /** @brief Constructor without initializations */
        Corner() : Readable(), Writeable(), position(3,0.0) {}

        /** @brief Constructor without initializations */
        Corner(const kjb::Vector & iposition) : Readable(), Writeable(), position(3,0.0)
        {
            set_position(iposition);
        }
        
        Corner(const Corner & src) : Readable(), Writeable(), position(src.position), segments(src.segments)
        {

        }

        Corner (const char * filename)
        {
            Readable::read(filename);
        }

        Corner (std::istream & in)
        {
            read(in);
        }

        Corner & operator=(const Corner & src)
        {
            position = src.position;
            segments = src.segments;
            return (*this);
        }

        inline const kjb::Vector & get_position() const
        {
            return position;
        }

        inline const std::vector<kjb::Line_segment> & get_segments() const
        {
            return segments;
        }

        inline void set_position(const kjb::Vector & iposition)
        {
            if(iposition.size() == 2)
            {
                position(0) = iposition(0);
                position(1) = iposition(1);
            }
            else if(iposition.size() == 3)
            {
                position = iposition;
            }
            else
            {
                KJB_THROW_2(Illegal_argument, "Invalid size of vector containing corner position");
            }
        }

        inline void add_segment(const kjb::Line_segment & isegment)
        {
            segments.push_back(isegment);
        }

        inline const kjb::Line_segment & get_segment(unsigned int i) const
        {
            if(i >= segments.size())
            {
                KJB_THROW_2(Illegal_argument, "Corner: requested segment index out of bounds");
            }
            return segments[i];
        }


        inline void set_segment(unsigned int i,  const kjb::Line_segment isegment)
        {
            if(i >= segments.size())
            {
                KJB_THROW_2(Illegal_argument, "Corner: requested segment index out of bounds");
            }
            segments[i] = isegment;
        }

        inline unsigned int get_num_segments() const
        {
            return segments.size();
        }

        /** @brief Reads this Line segment from an input stream. */
        void read(std::istream& in);

        /** @brief Writes this Line segment to an output stream. */
        void write(std::ostream& out) const;
       
        /** @brief Draws this line segment */
        virtual void draw( kjb::Image & img, double ir, double ig, double ib, double width = 1.0)  const;

        /** @brief Randomly colors this line segment on an image */
        virtual void randomly_color(kjb::Image & img, double width = 1.0)  const;

    protected:

        /** @brief position of this 2D corner */
        kjb::Vector position;

        /** The segments forming this corner */
        std::vector<kjb::Line_segment> segments;

};


}

#endif
