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
|  Author: Luca Del Pero
* =========================================================================== */

#ifndef EDGE_VANISHING_POINT_H
#define EDGE_VANISHING_POINT_H

#include "m_cpp/m_vector.h"
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>
#include <i_cpp/i_image.h>
#include <edge_cpp/line_segment.h>

namespace kjb {

/**
 * @class Vanishing_point
 *
 * @brief A vanishing point for a set of parallel lines in an image
 */
class Vanishing_point : public Readable, public Writeable
{
    public:
      /** @brief the possible types of vanishing point.
       *  We need this to differentiate between vanishing points
       *  with finite coordinates and vanishing points at infinity
       */
      enum Vanishing_point_type {
            REGULAR = 0,
            INFINITY_UP,
            INFINITY_DOWN,
            INFINITY_LEFT,
            INFINITY_RIGHT
        };

        /** @brief Default constructor, it initialize the vanishing point in (0,0) */
        Vanishing_point(Vanishing_point_type itype = REGULAR) :
            Readable(), Writeable(), _point(2,0.0), _type(itype) { }

        Vanishing_point(double x, double y) : Readable(), Writeable(), _point(2,0.0)
        {
            _type = REGULAR;
            _point(0) = x;
            _point(1) = y;
        }

        Vanishing_point(const kjb::Vector & position) :  Readable(), Writeable(), _point(2,0.0)
        {
            if(position.size() != 2)
            {
                throw kjb::Illegal_argument("Vanishing point vector size must be 2");
            }
             _type = REGULAR;
            _point = position;
        }

        Vanishing_point(const char *filename) : Readable(), Writeable(), _point(2,0.0)
        {
            Readable::read(filename);
        }

        Vanishing_point(std::istream& in) : Readable(), Writeable(), _point(2,0.0)
        {
            read(in);
        }


        Vanishing_point(const Vanishing_point & src)
                   : Readable(), Writeable(), _point(src._point), _type(src._type) { }

        Vanishing_point & operator=(const Vanishing_point & src)
        {
            _point = src._point;
            _type = src._type;
            return (*this);
        }

        bool operator==(const Vanishing_point & vp);


        ~Vanishing_point() { }

        /** @brief returns true is the vanishing point is at infinity */
        bool is_at_infinity() const
        {
            return(_type);
        }

        /** @brief returns the vanishing point type */
        const Vanishing_point_type & get_type() const
        {
            return _type;
        }

        /** @brief returns the x coordinate of this vanishing point */
        double get_x() const
        {
            return _point(0);
        }

        /** @brief returns the y coordinate of this vanishing point */
        double get_y() const
        {
            return _point(1);
        }

        void set_x(double ix)
        {
            _point(0) = ix;
        }

        void set_y(double iy)
        {
            _point(1) = iy;
        }

        /** @brief Sets the type of this Vanishing point */
        void set_type(const Vanishing_point_type & itype)
        {
            _type = itype;
            if(_type > REGULAR)
            {
                _point(0) = 0.0;
                _point(1) = 0.0;
            }
        }

        /** @brief Sets the position of this Vanishing point */
        void set_point(double x, double y)
        {
           _type = REGULAR;
           _point(0) = x;
           _point(1) = y;
        }

        /** @brief Sets the position of this Vanishing point */
        void set_position(const kjb::Vector & position)
        {
            if(position.size() != 2)
            {
                throw kjb::Illegal_argument("Vanishing point vector size must be 2");
            }
             _type = REGULAR;
            _point = position;
        }

        /** @brief Reads this Vanishing point from an input stream. */
        void read(std::istream& in);

        /** @brief Writes this Vanishing point to an output stream. */
        void write(std::ostream& out) const;

    private:

#warning "[Code police] Please don't start identifiers with underscore."
        /** The coordinates (x,y) of this vanishing point. This field is meaningful only
         * if the vanishing point has finite coordinates (_type = REGULAR)
         */
        kjb::Vector _point;

        /** The type of this vanishing point. It can have finite coordinates stored
         * in _point, or be at infinity in four different directions (UP, DOWN, LEFT, RIGHT)
         */
        Vanishing_point_type _type;


};

void read_CMU_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    std::string file_name
);


void draw_mid_point_to_vanishing_point
(
    kjb::Image & img,
    const Line_segment & segment,
    const Vanishing_point & vpt,
    double ir,
    double ig,
    double ib,
    double width
);

bool find_vanishing_point_given_one_and_line
(
    const kjb::Vanishing_point & vp1,
    double focal,
    unsigned int img_cols,
    unsigned int img_rows,
    const Line_segment & ls,
    Vanishing_point & vpt
);

bool find_third_vanishing_point
(
    const kjb::Vanishing_point & vp1,
    const kjb::Vanishing_point & vp2,
    double focal,
    unsigned int img_rows,
    unsigned int img_cols,
    Vanishing_point & vpt
);

bool read_hedau_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal,
    const std::string & file_path,
    unsigned int num_cols,
    unsigned int num_rows
);

bool find_vertical_vanishing_point
(
    Vanishing_point & vp1,
    Vanishing_point & vp2,
    Vanishing_point & estimated_vertical,
    unsigned int img_cols,
    unsigned int img_rows
);

}
#endif /*VANISHING_POINT_DETECTION */
