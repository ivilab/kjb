/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for handling Manhattan world objects
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef EDGE_MANHATTAN_WORLD_H
#define EDGE_MANHATTAN_WORLD_H

#include "edge_cpp/vanishing_point_detector.h"
#include "edge_cpp/line_segment.h"
#include "edge_cpp/line_segment_set.h"
#include "edge_cpp/vanishing_point.h"
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>
#include <fstream>



#define MW_OUTLIER_THRESHOLD 0.16
#define MAX_CORNER_SEGMENT_STRETCH 35 /*25 */
#define MAX_CORNER_PERP_SEGMENT_STRETCH 3 /*3 */
#define MW_FOCAL_INIT_VALUE 150
#define MW_MIN_FOCAL_INIT_VALUE 50

namespace kjb {

/**
 * @class Manhattan_segment
 *
 * @brief A manhattan segment defined by a line segment and the vanishing
 *        point it converges to
 */
class Manhattan_segment
{
    public:
        Manhattan_segment(const Edge_segment & isegment, Vanishing_point * ivp = 0)
                      : _segment(isegment), _vp(ivp)
        {
            if(!ivp)
            {
                _outlier = true;
                alpha = 0.0;
            }
            else
            {
                alpha = Vanishing_point_detector::compute_alpha( *_vp, &_segment);
                _outlier = false;
            }
        }

        Manhattan_segment(const Manhattan_segment & src)
                      : _segment(src._segment), _vp(src._vp),
                        alpha(src.alpha), _outlier(src._outlier)  { }

        /** @brief returns the edge segment defining this manhattan segment */
        const Edge_segment & get_edge_segment() const { return _segment;}

        /** @brief Returns the vanishing point this Manhattan segment converges to */
        const Vanishing_point * get_vanishing_point() const { return _vp; }

        void set_vanishing_point(Vanishing_point * ivp)
        {
            if(!ivp)
            {
                _outlier = true;
                _vp = NULL;
                alpha = 0;
            }
            else
            {
                _outlier = false;
                _vp = ivp;
                alpha = Vanishing_point_detector::compute_alpha( *_vp, &_segment);
            }
        }

        /** @brief Marks this segment as an outlier
         */
        void mark_as_outlier()
        {
            _outlier = true;
            _vp = 0;
            alpha = 0.0;
        }

        /** @brief Returns true if this segment is an outlier,
         * ie it is not aligned with one of the three orthogonal
         * directions defining Manhattan world
         */
        bool is_outlier() const
        {
            return _outlier;
        }

        /** @brief returns alpha */
        double get_alpha() const
        {
            return alpha;
        }

        ~Manhattan_segment() { }

        /** @brief Draws this Manhattan segment */
        void draw( kjb::Image & img, double ir, double ig, double ib, double width = 1.0)  const
        {
            _segment.draw(img, ir, ig, ib, width);
        }

        /** @brief Randomly colors this Manhattan segment on an image */
        void randomly_color(kjb::Image & img, double width = 1.0)  const
        {
            _segment.randomly_color(img, width);
        }

         /** @brief Draws a line between the line segment mid point and the vanishing point it converges to
          *         Mostly for debug purposes */
        void draw_mid_point_to_vanishing_point(kjb::Image & img, double ir, double ig, double ib, double width = 1.0)  const;

        friend std::ostream & operator<<(std::ostream& out, const Manhattan_segment& ms);
    private:

#warning "[Code police] Please don't start identifiers with underscore."
        /** @brief The edge_segment this Manhattan world points to */
        Edge_segment _segment;
        
        /** @brief The vanishing point this segment converges to */
        Vanishing_point * _vp;

        /** @brief The difference in orientation between the line segment
         * and the segment between its mid point and the vanishing point it
         * converges to */
        double alpha;

        /** @brief Tells whether this edge is an outlier, meaning that
         * it is not aligned with one of the three main orthogonal
         * directions defining Manhattan world
         */
        bool _outlier;

};

/* @class Manhattan_corner_segment This class contains a line segment part
 * of a corner in Manhattan world. Such corners are formed by three line segments,
 * each converging to a different vanishing point (orthogonal corner).
 */
class Manhattan_corner_segment
{
public:

    Manhattan_corner_segment(const Manhattan_segment & isegment,
            double idist = 0.0, double iperp_dist = 0.0) :
        segment(isegment), distance_to_centre(idist), perpendicular_distance(iperp_dist)
    {

    }


    Manhattan_corner_segment(const Manhattan_corner_segment & src) :
        segment(src.segment),
        distance_to_centre(src.distance_to_centre),
        perpendicular_distance(src.perpendicular_distance)
    {

    }

    ~Manhattan_corner_segment()
    {

    }

    inline const Manhattan_segment & get_manhattan_segment() const
    {
        return segment;
    }

    inline const Edge_segment & get_edge_segment() const
    {
        return segment.get_edge_segment();
    }

    inline double get_fitting_error() const
    {
        return segment.get_edge_segment().get_least_squares_fitting_error();
    }

    inline double get_strength() const
    {
        return segment.get_edge_segment().get_strength();
    }

    /** @breif returns the distance between the corner position and
     * the line_segment closest point. We allow line segments that do not
     * intersect exactly to form a corner
     */
    inline double get_distance() const
    {
        return distance_to_centre;
    }

    /** @breif returns the perpendicular distance between the corner position and
     * the line the line_segment lies on. This is zero is the corner position
     * lies on this line
     */
    inline double get_perpendicular_distance() const
    {
        return perpendicular_distance;
    }

    //TODO to be implemented
    double compute_penalty() const;

    friend std::ostream & operator<<(std::ostream& out, const Manhattan_corner_segment& mcs);

private:

    Manhattan_corner_segment & operator=(const Manhattan_corner_segment & src)
    {
        /** This is private because this is not meant to be used! */
        distance_to_centre = src.distance_to_centre;
        perpendicular_distance = src.perpendicular_distance;
        return (*this);
    }

    const Manhattan_segment & segment;

    double distance_to_centre;

    double perpendicular_distance;

};

/* @class Manhattan_cornert This class represents a corner in Manhattan world.
 * Such corners are formed by three line segments, each converging to a different
 * vanishing point (orthogonal corner). Each corner is created when two or three
 * line_segments detected in the image and converging to different vanishing points
 * intersect. If there are only two segments intersecting, we compute the third
 * as the line between the corner position and the only vanishing point that was
 * not used in creating this corner (such segment is marked as unavailable)
 */
class Manhattan_corner
{
public:
    Manhattan_corner(std::vector<Vanishing_point> * ivpts) :
        position(2, 0.0), segments(3,(Manhattan_corner_segment *) NULL), available(3, false), indexes(3, 0),
        orthogonal_segments(3), v_pts(ivpts)
    {

    }

    Manhattan_corner(const Manhattan_corner & src) :
        position(src.position), segments(3, (Manhattan_corner_segment *)NULL), available(src.available),
        indexes(src.indexes), orthogonal_segments(src.orthogonal_segments), v_pts(src.v_pts)
    {
        for(unsigned int i = 0; i < 3; i++)
        {
            if(available[i])
            {
                segments[i] = new Manhattan_corner_segment(*(src.segments[i]));
            }
        }
    }

    Manhattan_corner operator=(const Manhattan_corner & src)
    {
        available = src.available;
        for(unsigned int i = 0; i < 3; i++)
        {
            if(available[i])
            {
                segments[i] = new Manhattan_corner_segment(*(src.segments[i]));
            }
        }
        orthogonal_segments = src.orthogonal_segments;
        v_pts = src.v_pts;
        position = src.position;
        indexes = src.indexes;
        return (*this);
    }

    ~Manhattan_corner()
    {
        for(unsigned int i = 0; i < 3; i++)
        {
            if(available[i])
            {
                delete segments[i];
            }
        }
    }

    /** @brief Returns true if the ith corner segment was detected in
     * the image, false if it is missing
     */
    bool is_available(unsigned int i) const
    {
        if(i >= 3)
        {
            KJB_THROW_2(Illegal_argument,"Manhattan corners only have 3 segments");
        }
        return available[i];
    }

    const Manhattan_corner_segment * get_segment(unsigned int i) const
    {
        if(!is_available(i))
        {
            KJB_THROW_2(Illegal_argument,"Manhattan corners, the requested segment is not available!");
        }
        return segments[i];
    }

    /** @brief Returns the index in the edge_segments vector stored in the Manahattan_world
     * class corresponding to the ith corner_segment in this corner i=[0,2] */
    unsigned int get_index(unsigned int i) const
    {
        if(!is_available(i))
        {
            KJB_THROW_2(Illegal_argument,"Manhattan corners, the requested segment is not available!");
        }
        return indexes[i];
    }

    /** @brief Returns the position of the corner in the image plane */
    const kjb::Vector & get_position() const
    {
        return position;
    }

    /** @brief Sets the position of the corner in the image plane */
    void set_position(const kjb::Vector & iposition)
    {
        if(iposition.size() < 2)
        {
            KJB_THROW_2(Illegal_argument, "Corner position vector must be of size 2");
        }
        position = iposition;
        for(unsigned int i =0; i < 3; i++)
        {
            delete_segment(i);
        }
    }

    void set_segment(const Manhattan_corner_segment & segment, unsigned int i)
    {
        if(is_available(i))
        {
            delete segments[i];
        }
        segments[i] = new Manhattan_corner_segment(segment);
        available[i] = true;
        compute_orthogonal_segment(i);
    }

    void set_segment(unsigned int i,const Manhattan_segment & segment, double dist = 0.0, double perp_dist = 0)
    {
        if(is_available(i))
        {
            delete segments[i];
        }
        segments[i] = new Manhattan_corner_segment(segment, dist, perp_dist);
        available[i] = true;
        compute_orthogonal_segment(i);
    }

    void set_index(unsigned int i, unsigned int iindex)
    {
        if(is_available(i))
        {
            indexes[i] = iindex;
        }
    }


    void delete_segment(unsigned int i)
    {
        if(is_available(i))
        {
            delete segments[i];
        }
        segments[i] = NULL;
        available[i] = false;
        indexes[i] = 0;
    }

    inline const Line_segment & get_orthogonal_segment(unsigned int i) const
    {
        if(i > 2)
        {
            KJB_THROW_2(Illegal_argument,"Manhattan corner, get orthogonal segment, index out of bounds");
        }
        return orthogonal_segments[i];
    }

    unsigned int num_available_segments() const;

    void draw(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void get_3D_corner
    (
        double z_distance,
        double focal_length,
        double princ_x,
        double princ_y,
        kjb::Vector & corner3D_1,
        kjb::Vector & corner3D_2,
        kjb::Vector & corner3D_3,
        kjb::Vector & position_3D
    ) const;

    friend std::ostream & operator<<(std::ostream& out, const Manhattan_corner& mc);

    void get_direction(unsigned int segment_index, kjb::Vector & direction) const;

    bool is_up_corner() const;

    double get_avg_segment_size() const;

    void compute_orthogonal_segments();

    void init_missing_orthogonal_segment(int iindex, double start_x, double start_y, double end_x, double end_y);

    void compute_orthogonal_segment(unsigned int i);

private:

    bool is_right_segment(unsigned int i);

    kjb::Vector position;

    std::vector<Manhattan_corner_segment *> segments;

    std::vector<bool> available;

    std::vector<unsigned int> indexes;

    std::vector<Line_segment> orthogonal_segments;

    std::vector<Vanishing_point> * v_pts;
};


/**
 * @class Manhattan_world
 *
 * @brief This class contains the three orthogonal vanishing points
 * defining a Manhattan scene, where most or all planes are aligned
 * with three main orthogonal directions. This class optionally
 * contains a set of segments from the scene, assigned to the
 * correct vanishing point.
 */
class Manhattan_world
{
public:

    Manhattan_world(double ifocal = MW_FOCAL_INIT_VALUE) : _assignments(4), _vpts(3), focal_length(ifocal) { }

    Manhattan_world(const std::vector<Vanishing_point> & ivpts, double ifocal = MW_FOCAL_INIT_VALUE )
    : _assignments(4), _vpts(ivpts), focal_length(ifocal)
    {
        if(focal_length < MW_MIN_FOCAL_INIT_VALUE)
        {
            focal_length = MW_MIN_FOCAL_INIT_VALUE;
        }
        if(ivpts.size() != 3)
        {
            KJB_THROW_2(Illegal_argument,"Manhattan world requires three vanishing points");
        }
    }

    Manhattan_world(const Edge_segment_set & isegments, const std::vector<Vanishing_point> & ivpts,
                    double ifocal = MW_FOCAL_INIT_VALUE, double outlier_threshold = MW_OUTLIER_THRESHOLD);

    Manhattan_world(const char *filename, const Edge_segment_set & isegments,
                    double outlier_threshold = MW_OUTLIER_THRESHOLD) :
                        _assignments(4), _vpts(3), focal_length(MW_FOCAL_INIT_VALUE)
    {
        read(filename, isegments, outlier_threshold);
    }

    Manhattan_world(const char *filename) : _assignments(4), _vpts(3), focal_length(MW_FOCAL_INIT_VALUE)
    {
        read(filename);
    }

    Manhattan_world(std::istream& in, const Edge_segment_set & isegments,
            double outlier_threshold = MW_OUTLIER_THRESHOLD) :
                _assignments(4), _vpts(3), focal_length(MW_FOCAL_INIT_VALUE)
    {
        read(in, isegments, outlier_threshold);
    }

    Manhattan_world(const Manhattan_world & src) : _assignments(src._assignments),
                     _corners2(src._corners2),  _corners3(src._corners3),
                     _vpts(src._vpts), focal_length(src.focal_length) { }

    Manhattan_world & operator=(const Manhattan_world & src)
    {
        _assignments = src._assignments;
        _vpts = src._vpts;
        _corners2 = src._corners2;
        _corners3 = src._corners3;
        focal_length = src.focal_length;
        return (*this);
    }

    /** @brief returns the focal length */
    inline double get_focal_length() const
    {
        return focal_length;
    }

    /** @brief returns the focal length */
    inline void set_focal_length(double ifocal)
    {
        focal_length = ifocal;
    }

    void reset_vanishing_points(const std::vector<Vanishing_point> & vpts)
    {
        _vpts = vpts;
        for(unsigned int i = 0; i < 4; i++)
        {
            _assignments[i].clear();
        }
        _corners2.clear();
        _corners3.clear();
    }

    /** @brief Assigns each input segment to the vanishing point it converges to, or mark
     * it as an outlier if it does not converge to any vanishing point */
    void assign_segments_to_vpts(const Edge_segment_set & , double outlier_threshold = MW_OUTLIER_THRESHOLD );

    /** @brief returns the vector of the segments assigned to the correct vanishing point (outliers
     *         in the last vector)  */
    const std::vector < std::vector<Manhattan_segment> > & get_assignments() const {return _assignments; }

    /**@brief returns the length of the vector of the segments assigned to the vanishing point*/
    unsigned num_lines_assigned_to_vp(unsigned vpindex) const { return _assignments[vpindex].size(); } 

    /** @brief Returns the vanishing points */
    const std::vector<Vanishing_point> & get_vanishing_points() const { return _vpts; }

    const std::vector<Vanishing_point> get_value_vanishing_points() const { return _vpts; }

    /** @brief draws the line segments with different colors according to the
     * vanishing point they are assigned to (green and red for horizontal,
     * blue for vertical, black for outliers).
     */
    void draw_segments_with_vp_assignments(kjb::Image & img, double width = 1.0) const;

    /** @brief Draws a line between the mid point of each segment (excluding outliers)
     *  and the vanishing point the segment converges to. Line segments
     *  are drawn with different colors according to the
     *  vanishing point they are assigned to (green and red for horizontal,
     *  blue for vertical, black for outliers).
     */
    void draw_lines_from_segments_midpoint_to_vp(kjb::Image & img, double width = 1.0) const;

    /** @brief Reads this Manhattan world from an input stream. */
    void read(std::istream& in, const Edge_segment_set & isegments, double outlier_threshold = MW_OUTLIER_THRESHOLD);

    /** @brief Reads this Manhattan world from an input file. */
    void read(const char *filename, const Edge_segment_set & isegments, double outlier_threshold = MW_OUTLIER_THRESHOLD);

    void read(const char * filename);
    void read(std::istream& in);

    /** @brief Writes this Manhattan world to an output stream. */
    void write(std::ostream& out) const;

    /** @brief Writes this Manhattan world to an output file. */
    void write(const char * out_file) const
    {
        std::ofstream ofs(out_file);
        if(ofs.fail())
        {
            KJB_THROW_2(kjb::IO_error, "Could not open file to write Manhattan world");
        }
        write(ofs);
        ofs.close();
    }

    /** @brief Returns the ith corner consisting of the intersection of two line segments */
    const Manhattan_corner & get_corner_2(unsigned int i) const
    {
        if(i >= _corners2.size())
        {
            KJB_THROW_2(Illegal_argument,"Corner2 index out of bounds");
        }
        return _corners2[i];
    }

    /** @brief Returns the ith corner consisting of the intersection of three line segments */
    const Manhattan_corner & get_corner_3(unsigned int i) const
    {
        if(i >= _corners3.size())
        {
            KJB_THROW_2(Illegal_argument,"Corner3 index out of bounds");
        }
        return _corners3[i];
    }

    const Manhattan_corner & get_extra_corner(unsigned int i) const
    {
        if(i >= _extra_corners.size())
        {
            KJB_THROW_2(Illegal_argument,"Extra corner index out of bounds");
        }
        return _extra_corners[i];
    }



    const Manhattan_segment & get_manhattan_segment(unsigned int vp_index, unsigned int segment_index) const;

    void write_manhattan_corner(const Manhattan_corner & corner, std::ostream& out) const;

    void read_manhattan_corner(Manhattan_corner & corner, std::istream& in) const;

    void write_corners(std::ostream& out) const;

    void read_corners(std::istream& in);

    void draw_corners(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners2(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners2up(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners3(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners3up(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners2smart(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_corners3smart(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;

    void draw_extra_corners(kjb::Image & img, bool draw_full_segments = false, double width = 1.0) const;


    /** @brief Creates a corner from two line segments.
     *  Returns false if the segments do not intersect
     */
    static bool create_corner
    (
        Manhattan_corner & corner,
        const Manhattan_segment & seg1,
        const Manhattan_segment & seg2,
        unsigned int index1,
        unsigned int index2,
        double max_stretch = MAX_CORNER_SEGMENT_STRETCH
    );

    /** @brief Creates a corner from an already existing corner and a line segment.
     *  Returns false if the segment does not intersect the corner position
    */
   static bool create_corner3
   (
       Manhattan_corner & corner,
       const Manhattan_segment & seg3,
       unsigned int index3,
       double max_stretch = MAX_CORNER_SEGMENT_STRETCH,
       double max_perp_stretch = MAX_CORNER_PERP_SEGMENT_STRETCH
   );

   bool create_corner3_from_incomplete
   (
      Manhattan_corner & corner,
      unsigned int index3,
      bool towards_vp,
      const kjb::Vector & position,
      bool check_consistency = true
   );


    /** @brief Creates corners by checking all possible intersections of
     *         two line segments */
    void create_corners2();

    /** @brief Creates corners by checking all possible intersections of
     *         three line segments */
    void create_corners3();

    /** @brief Creates corners by checking all possible intersections of
     *         two and three line segments */
    void create_corners();

    inline unsigned int get_num_corners2() const
    {
        return _corners2.size();
    }

    inline unsigned int get_num_corners3() const
    {
        return _corners3.size();
    }

    inline unsigned int get_num_extra_corners() const
    {
        return _extra_corners.size();
    }

    void print_corners(std::ostream& out) const
    {
        print_corners2(out);
        print_corners3(out);
    }

    void print_corners2(std::ostream& out) const;

    void print_corners3(std::ostream& out) const;

    void print_vanishing_points(std::ostream& out) const;

    void get_3D_corner
    (
        double z_distance,
        double princ_x,
        double princ_y,
        unsigned int index,
        bool usecorner3,
        kjb::Vector & corner3D_1,
        kjb::Vector & corner3D_2,
        kjb::Vector & corner3D_3,
        kjb::Vector & position_3D
    ) const;

    const std::vector<Manhattan_corner> & get_corners3() const
    {
        return _corners3;
    }

    const std::vector<Manhattan_corner> & get_corners2() const
    {
        return _corners2;
    }

    const std::vector<Manhattan_corner> & get_extra_corners() const
    {
        return _extra_corners;
    }

    void set_extra_corners_from_vertical_pairs(const std::vector<Segment_pair> & vpairs);

private:

    bool corner3_exists(const Manhattan_corner & c);

    /** A vector containing four vectors of edge segments.
     *  For n=1,2,3, the nth vector contains all the segments
     *  converging to the third vanishing point. The fourth
     *  vector contains the outliers
     */
    std::vector < std::vector<Manhattan_segment> > _assignments;

    std::vector<Manhattan_corner> _corners2;

    std::vector<Manhattan_corner> _corners3;

    std::vector<Manhattan_corner> _extra_corners;

    /** The three orthogonal vanishing points defining Manhattan world */
    std::vector <Vanishing_point> _vpts;

    double focal_length;

};

std::ostream& operator<<(std::ostream& out, const Manhattan_corner& mc);
std::ostream & operator<<(std::ostream& out, const Manhattan_corner_segment& mcs);
std::ostream & operator<<(std::ostream& out, const Manhattan_segment& ms);

Manhattan_world * create_manhattan_world_from_CMU_file(const std::string & file_name);

Manhattan_world * create_mw_from_CMU_file_and_compute_focal_length
(
    const std::string & file_name,
    const kjb::Edge_segment_set & iset,
    unsigned int num_rows,
    unsigned int num_cols
);

}

#endif
