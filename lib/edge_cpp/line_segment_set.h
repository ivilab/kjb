/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line segment set classes
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef EDGE_EDGE_SEGMENT_SET_H
#define EDGE_EDGE_SEGMENT_SET_H

#include "edge_cpp/line_segment.h"
#include <vector>

namespace kjb {

/**
 * @class Line_segment_set
 *
 * @brief Class to manipulate a set of line segments
 */
class Line_segment_set: public Readable, public Writeable
{
    public:
        Line_segment_set() : Readable(), Writeable() {}

        Line_segment_set(const std::vector<Line_segment> & isegments )
         : Readable(), Writeable(), _segments(isegments){}

        Line_segment_set(std::istream& in)
        {
            read(in);
        }

        Line_segment_set(const char * file_name)
        {
           Readable::read(file_name);
        }

        Line_segment_set(const Line_segment_set & src) :
            Readable(), Writeable(), _segments(src._segments) { }

        Line_segment_set & operator=(const Line_segment_set & iseg)
        {
            _segments = iseg._segments;
            return (*this);
        }

        ~Line_segment_set() {}

        void read_from_Kovesi_format(const std::string path_to_file);

        void read_from_Kovesi_formatb(const std::string path_to_file);

        /** @brief Returns a vector containing the fitted line segments */
        inline const std::vector<Line_segment> & get_segments() { return _segments; }

        /** @brief Returns the number of segments in this set */
        inline unsigned int size() const { return _segments.size(); }

        /** @brief Adds a line segment to this collection */
        inline void add_segment(const Line_segment & isegment)
        {
            _segments.push_back(isegment);
        }

        /** @brief returns the ith fitted line segment */
        inline const Line_segment & get_segment(unsigned int i) const { return _segments[i]; }

        /** @brief Draws this line segment set */
        void draw( kjb::Image & img, double ir, double ig, double ib, double width = 1.0)  const;

        /** @brief Randomly colors this line segment set on an image */
        void randomly_color(kjb::Image & img, double width = 1.0)  const;

        /** @brief Reads this Edge_segment_set from an input stream. */
        void read(std::istream& in);

        /** @brief Writes this Edge_segment_set to an output stream. */
        void write(std::ostream& out) const;

        /** @brief Reads this Edge_segment_set from an input file. */
        void read(const char* fname)
        {
            Readable::read(fname);
        }

         /** @brief Writes this Edge_segment_set to a file. */
        void write(const char* fname) const
        {
            Writeable::write(fname);
        }

        kjb::Edge_set * convert_to_edge_set(unsigned int num_rows, unsigned int num_cols);

        void remove_frame_segments(unsigned int num_rows, unsigned int num_cols);

    private:
        std::vector<Line_segment> _segments;

};

/*
 * In order to use this function, you have to set two
 * environment variables.
 * First, PATH_TO_MATLAB, must point to the directory where matlab is installed.
 * Second, you need to compile APPgetLargeConnectedEdges.m, which
 * you can find in this directory, using Matlab with the command
 * mcc -o APPgetLargeConnectedEdges -m APPgetLargeConnectedEdges.m.
 * Last, setPATH_TO_GET_LONG_STRAIGHT_EDGES, must point to the directory
 * containing script run_APPgetLargeConnectedEdges.sh generated
 * by the MATLAB compiler
 */
void detect_long_connected_segments(Line_segment_set & segments, const std::string & img_path, int max_length);


/**
 * @class Edge_segment_set
 *
 * @brief Class to manipulate a set of edge segments
 */
class Edge_segment_set: public Readable, public Writeable
{
    public:
        Edge_segment_set() : Readable(), Writeable() {}

        Edge_segment_set(const std::vector<Edge_segment> & isegments )
         : Readable(), Writeable(), _segments(isegments){}

        Edge_segment_set(const kjb::Edge_set * edge_set,const std::vector<Line_segment> & isegments );

        Edge_segment_set(const kjb::Edge_set * edge_set, bool use_num_pts_as_length = true );

        Edge_segment_set(const kjb::Edge_set * edge_set, const char * filename);

        Edge_segment_set(const kjb::Edge_set * edge_set, std::istream& in);

        Edge_segment_set(const Edge_segment_set & src) :
            Readable(), Writeable(), _segments(src._segments) { }

        Edge_segment_set & operator=(const Edge_segment_set & iseg)
        {
            _segments = iseg._segments;
            return (*this);
        }

        ~Edge_segment_set() {}

        inline void init_from_edge_set(const kjb::Edge_set * edge_set, bool use_num_pts_as_length = true);

        /** Returns a vector containing the fitted line segments */
        inline const std::vector<Edge_segment> & get_segments() { return _segments; }

        /** Returns the number of segments in this set */
        inline unsigned int size() const { return _segments.size(); }

        /** @brief returns the ith fitted line segment */
        inline const Edge_segment & get_segment(unsigned int i) const { return _segments[i]; }
        
        /** @brief Adds a line segment to this collection */
        inline void add_segment(const Edge_segment & isegment)
        {
            _segments.push_back(isegment);
        }

        /** @brief returns the ith collection of edge points the segments were fitted to */
        inline const kjb::Edge & get_edge(unsigned int i)
        {
            return _segments[i].get_edge();
        }

        
        /** @brief Draws this line segment set */
        void draw( kjb::Image & img, double ir, double ig, double ib, double width = 1.0)  const;

        /** @brief Randomly colors this line segment set on an image */
        void randomly_color(kjb::Image & img, double width = 1.0)  const;

         /** @brief Reads this Edge_segment_set from an input stream. */
        void read(std::istream& in, const kjb::Edge_set * edge_set );

         /** @brief Reads this Edge_segment_set from an input file. */
        void read(const char *filename, const kjb::Edge_set * edge_set );

        /** @brief Reads this Edge_segment_set from an input stream. */
        void read(std::istream& in);

        /** @brief Reads this Edge_segment_set from an input file. */
        void read(const char* fname)
        {
            Readable::read(fname);
        }

        /** @brief Writes this Edge_segment_set to an output stream. */
        void write(std::ostream& out) const;

         /** @brief Writes this Edge_segment_set to a file. */
        void write(const char* fname) const
        {
            Writeable::write(fname);
        }

        /** @brief In case there are multiple roughly collinear edge segments,
         *         only the longest one is kept. This function is mostly
         *         for debug purposes */
        void remove_overlapping_segments
        (
            kjb::Edge_set & edges,
            double collinear_threshold = 0.12,
            double overlapping_threshold = 10
        );

        void remove_non_straight_segments(kjb::Edge_set & edges, double threshold);

        void remove_frame_segments(unsigned int num_rows, unsigned int num_cols, kjb::Edge_set & edges);

        kjb::Edge_set * convert_to_edge_set(unsigned int num_rows, unsigned int num_cols);

        void find_vertical_segment_pairs(std::vector<Segment_pair> & pairs, int num_rows) const;

    private:

        /** @brief This function will change the edge set thie edge segment set
         *         was built from, according to the changes that were made to
         *         this edge_segment set */
        void update_edge_set(kjb::Edge_set & edges);

        /** @brief A collection of edge segments */
        std::vector<Edge_segment> _segments;

};

}


#endif
