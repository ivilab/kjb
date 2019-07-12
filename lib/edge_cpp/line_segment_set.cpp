
/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Line segment set class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#include <edge_cpp/line_segment_set.h>
#include "edge_cpp/segment_pair.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace kjb;

/**
 * Read an edge segment set from an input stream.
 * The input edge_set is necessary to bind the fitted segments to the
 * original detected edges
 *
 * @param edge_set set of edges fitted to the line segments to be read
 * @param in the input stream to read this edge segment set from
 */
void Line_segment_set::read(std::istream& in )
{
    using std::istringstream;

    const char* field_value;

    unsigned int _size;

    // Camera centre
    if (!(field_value = read_field_value(in, "num_segments")))
    {
        KJB_THROW_2(Illegal_argument, "Edge segment set, Could not read number of segments");
    }
    istringstream ist(field_value);
    ist >> _size;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment set");
    }
    ist.clear(std::ios_base::goodbit);

    _segments.clear();
    for(unsigned int i = 0; i < _size; i++)
    {
        _segments.push_back( Line_segment(in));
    }
}

/** @brief Writes this Edge_segment_set to an output stream. */
void Line_segment_set::write(std::ostream& out) const
{
    out << "num_segments:" << size() << '\n';

    for(unsigned int i = 0; i < size(); i++)
    {
        _segments[i].write(out);
    }
}

void Line_segment_set::read_from_Kovesi_format(const std::string path_to_file)
{
    using std::istringstream;
    std::ifstream ifs(path_to_file.c_str());
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open Line_segment_set file in Kovesi format");
    }

    try
    {
        std::string seg_line;
        getline(ifs, seg_line);
        _segments.clear();
        while( !(ifs.fail()))
        {
            istringstream iss(seg_line, istringstream::in);
            double a, b, c, d;
            iss >> a >> b >> c >> d;
            Line_segment ls;
            try
            {
                ls.init_from_end_points(b, a, d, c);
               _segments.push_back(ls);
            }
            catch(...)
            {

            }
            getline(ifs, seg_line);
        }
        ifs.close();
    }
    catch(...)
    {
        KJB_THROW_2(IO_error, "Could not read Line_segment_set in Kovesi format");
    }
}

void Line_segment_set::read_from_Kovesi_formatb(const std::string path_to_file)
{
    using std::istringstream;
    std::ifstream ifs(path_to_file.c_str());
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open Line_segment_set file in Kovesi format");
    }

    try
    {
        std::string seg_line;
        getline(ifs, seg_line);
        _segments.clear();
        while( !(ifs.fail()))
        {
            istringstream iss(seg_line, istringstream::in);
            double a, b, c, d;
            iss >> a >> b >> c >> d;
            Line_segment ls;
            try
            {
                ls.init_from_end_points(a, b, c, d);
               _segments.push_back(ls);
            }
            catch(...)
            {

            }
            getline(ifs, seg_line);
        }
        ifs.close();
    }
    catch(...)
    {
        KJB_THROW_2(IO_error, "Could not read Line_segment_set in Kovesi format");
    }
}

/**
 * Draws the line segment on an image
 *
 * @param img the image
 * @param ir the red channel
 * @param ig the green channel
 * @param ib the blue channel
 * @param width the width of the segment to be drawn (in pixel)
 */
void Line_segment_set::draw( kjb::Image & img, double ir, double ig, double ib, double width)  const
{
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        _segments[i].draw(img, ir, ig, ib, width);
    }
}

/**
 * Draws the line segment set using a different random color for each segment
 *
 * @param img the image
 * @param width the width of the segment to be drawn (in pixel)
 */
void Line_segment_set::randomly_color(kjb::Image & img, double width)  const
{
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        _segments[i].randomly_color(img, width);
    }
}

/**
 * Constructs a fitted_edge_segment_set from a set of detected edges
 *
 * @param edge_set the detected edges
 */
Edge_segment_set::Edge_segment_set(const kjb::Edge_set * edge_set, bool use_num_pts_as_length )
: Readable(), Writeable()
{
    init_from_edge_set(edge_set, use_num_pts_as_length);
}

/**
 * Constructs an edge segment set by reading it from an input stream.
 * The input edge_set is necessary to bind the fitted segments to the
 * original detected edges
 *
 * @param edge_set set of edges fitted to the line segments to be read
 * @param in the input stream to read this edge segment set from
 */
Edge_segment_set::Edge_segment_set(const kjb::Edge_set * edge_set, std::istream& in)
: Readable(), Writeable()
{
    read(in, edge_set);
}

/**
 * Constructs an edge segment set by reading it from an input stream.
 * The input edge_set is necessary to bind the fitted segments to the
 * original detected edges
 *
 * @param edge_set set of edges fitted to the line segments to be read
 * @param filename the input file to read this edge segment set from
 */
Edge_segment_set::Edge_segment_set(const kjb::Edge_set * edge_set, const char * filename)
: Readable(), Writeable()
{
    read(filename, edge_set);
}

Edge_segment_set::Edge_segment_set
(
    const kjb::Edge_set * edge_set,
    const std::vector<Line_segment> & isegments
) : Readable(), Writeable()
{
    if(isegments.size() != edge_set->num_edges())
    {
        KJB_THROW_2(KJB_error, "Constructor of edge_segment_set, size of line_segment_set must match number of edges");
    }
    for(unsigned int i = 0; i < isegments.size(); i++)
    {
        _segments.push_back(Edge_segment(edge_set->get_edge(i), isegments[i]));
    }
}

/**
 * Initialize this edge_segment_set from an edge_set.
 * It fit an edge segment to each of the input edges
 * in the input Edge_set
 *
 * @param edge_set the input Edge_set
 */
void Edge_segment_set::init_from_edge_set(const kjb::Edge_set * edge_set, bool use_num_pts_as_length)
{
    _segments.clear();
    for(unsigned int i = 0; i < edge_set->num_edges(); i++ )
    {
        _segments.push_back( Edge_segment( edge_set->get_edge(i), use_num_pts_as_length ) );
    }
}

/**
 * Read an edge segment set from an input stream.
 * The input edge_set is necessary to bind the fitted segments to the
 * original detected edges
 *
 * @param edge_set set of edges fitted to the line segments to be read
 * @param in the input stream to read this edge segment set from
 */
void Edge_segment_set::read(std::istream& in, const kjb::Edge_set * edge_set )
{
    using std::istringstream;

    const char* field_value;

    unsigned int _size;

    // Camera centre
    if (!(field_value = read_field_value(in, "num_segments")))
    {
        KJB_THROW_2(Illegal_argument, "Edge segment set, Could not read number of segments");
    }
    istringstream ist(field_value);
    ist >> _size;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment set");
    }
    if (_size != edge_set->num_edges())
    {
        KJB_THROW_2(Illegal_argument, "Edge segment set and edge set size do not match, could"
                                    "not read Edge segment set");
    }
    ist.clear(std::ios_base::goodbit);

    _segments.clear();
    for(unsigned int i = 0; i < _size; i++)
    {
        _segments.push_back( Edge_segment(edge_set->get_edge(i), in));
    }
}

/**
 * Read an edge segment set from an input file.
 * The input edge_set is necessary to bind the fitted segments to the
 * original detected edges
 *
 * @param edge_set set of edges fitted to the line segments to be read
 * @param in the input file to read this edge segment set from
 */
void Edge_segment_set::read(const char * filename, const kjb::Edge_set * edge_set )
{
    std::ifstream in;
    std::ostringstream ost;

    in.open(filename);
    if (in.fail())
    {
        ost << filename << ": Could not open edge segments file";
        KJB_THROW_2(IO_error,ost.str());
    }

    try
    {
        read(in, edge_set);
        }
        catch (IO_error e)
        {
            ost << filename << ": " << e.get_msg();
            KJB_THROW_2(IO_error,ost.str());
        }

    in.close();
    if (in.fail())
    {
        ost << filename << ": Could not close edge segments file";
        KJB_THROW_2(IO_error,ost.str());
    }
}

/** Reads this Edge_segment_set from an input stream.
 *  Not implemented
 */
void Edge_segment_set::read(std::istream& /* in */)
{
    KJB_THROW_2(KJB_error, "Read method requires for edge segment set requires an input edge_set");
}

/** @brief Writes this Edge_segment_set to an output stream. */
void Edge_segment_set::write(std::ostream& out) const
{
    out << "num_segments:" << size() << '\n';

    for(unsigned int i = 0; i < size(); i++)
    {
        _segments[i].write(out);
    }
}

/**
 * Draws the line segment on an image
 *
 * @param img the image
 * @param ir the red channel
 * @param ig the green channel
 * @param ib the blue channel
 * @param width the width of the segment to be drawn (in pixel)
 */
void Edge_segment_set::draw( kjb::Image & img, double ir, double ig, double ib, double width)  const
{
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        if(_segments[i].get_least_squares_fitting_error() < 0.02)
            _segments[i].draw(img, ir, ig, ib, width);
    }
}

/**
 * Draws the line segment set using a different random color for each segment
 *
 * @param img the image
 * @param width the width of the segment to be drawn (in pixel)
 */
void Edge_segment_set::randomly_color(kjb::Image & img, double width)  const
{
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        _segments[i].randomly_color(img, width);
    }
}

void Edge_segment_set::find_vertical_segment_pairs(std::vector<Segment_pair> & pairs, int num_rows) const
{
    int half_img_size = (int) (num_rows/2.0);
    std::cout << "Half img size: " << half_img_size << std::endl;
    for(unsigned k1 = 0; k1 < _segments.size(); k1++)
    {
         double end1 = _segments[k1].get_end_y();
         if(_segments[k1].get_start_y() > end1)
         {
             end1 = _segments[k1].get_start_y();
         }
         if(end1 < half_img_size)
         {
             continue;
         }
         for(unsigned k2 = (k1+1); k2 < _segments.size(); k2++)
         {
            double end2 = _segments[k2].get_end_y();
            if(_segments[k2].get_start_y() > end2)
            {
               end2 = _segments[k2].get_start_y();
            }
            if(end2 < half_img_size)
            {
                continue;
            }
            //Check if the segment is vertical
            if(fabs(_segments[k1].get_orientation() - M_PI_2) <= 0.1)
            {

                if(_segments[k1].is_overlapping(_segments[k2], 0.2, 10))
                {
                    //Check if they are collinear
                    if(fabs(end1 - end2) < 10)
                    {
                        pairs.push_back(Segment_pair(_segments[k1], _segments[k2]));
                    }
                }
            }
        }
    }
}

/** In case there are multiple roughly collinear edge segments,
 *  only the longest one is kept. This function is mostly
 *  for debug purposes
 *
 *  @param edges The edge_set this edge_segment_set was built from
 */
void Edge_segment_set::remove_overlapping_segments
(
    kjb::Edge_set & edges,
    double collinear_threshold,
    double overlapping_threshold
)
{
    std::vector<Edge_segment> copies = _segments;
    std::vector<bool> used(copies.size(), false);
    _segments.clear();

    for(unsigned int k = 0; k < copies.size(); k++)
    {
        used[k] = false;
    }
//  std::cout << "Copies size:" << copies.size() << std::endl;
    for(unsigned k = 0; k < copies.size(); k++)
    {
        if(!used[k])
        {
            used[k] = true;
            Edge_segment seg = copies[k];
            for(unsigned j = k+1; j < copies.size(); j++ )
            {
                if(seg.is_overlapping(copies[j], collinear_threshold, 
                                                 overlapping_threshold))
                {
                    used[j] = true;
                    if(copies[j].get_length() > seg.get_length() )
                    {
                        seg = copies[j];
                    }
                }
            }
            _segments.push_back(seg);
        }
    }
    update_edge_set(edges);
}

/** In case there are multiple roughly collinear edge segments,
 *  only the longest one is kept. This function is mostly
 *  for debug purposes
 *
 *  @param edges The edge_set this edge_segment_set was built from
 */
void Edge_segment_set::remove_frame_segments(unsigned int num_rows, unsigned int num_cols, kjb::Edge_set & edges)
{
    std::vector<Edge_segment> copies = _segments;
    std::vector<bool> used(copies.size(), false);
    _segments.clear();

    for(unsigned int k = 0; k < copies.size(); k++)
    {
        used[k] = false;
    }
//  std::cout << "Copies size:" << copies.size() << std::endl;
    for(unsigned k = 0; k < copies.size(); k++)
    {
         if(copies[k].is_horizontal())
         {
             if(copies[k].get_centre_y() < 5)
             {
                 std::cout << "Horizontal!" << std::endl;
                 std::cout << copies[k] << std::endl;
                 continue;
             }
             if(copies[k].get_centre_y() > (num_rows -10))
             {
                 continue;
             }
         }
         if(copies[k].is_vertical())
         {
             if(copies[k].get_centre_x() < 5)
             {
                 std::cout << "Vertical!" << std::endl;
                 std::cout << copies[k] << std::endl;
                 continue;
             }
             if(copies[k].get_centre_x() > (num_cols -10))
             {
                 continue;
             }
         }
        _segments.push_back(copies[k]);
    }
    update_edge_set(edges);
}

void Line_segment_set::remove_frame_segments(unsigned int num_rows, unsigned int num_cols)
{
    std::vector<Line_segment> copies = _segments;
    std::vector<bool> used(copies.size(), false);
    _segments.clear();

    for(unsigned int k = 0; k < copies.size(); k++)
    {
        used[k] = false;
    }
//  std::cout << "Copies size:" << copies.size() << std::endl;
    for(unsigned k = 0; k < copies.size(); k++)
    {
         if(copies[k].is_horizontal())
         {
             if(copies[k].get_centre_y() < 5)
             {
                 std::cout << "Horizontal!" << std::endl;
                 std::cout << copies[k] << std::endl;
                 continue;
             }
             if(copies[k].get_centre_y() > (num_rows -10))
             {
                 continue;
             }
         }
         if(copies[k].is_vertical())
         {
             if(copies[k].get_centre_x() < 5)
             {
                 std::cout << "Vertical!" << std::endl;
                 std::cout << copies[k] << std::endl;
                 continue;
             }
             if(copies[k].get_centre_x() > (num_cols -10))
             {
                 continue;
             }
         }
        _segments.push_back(copies[k]);
    }
}

void Edge_segment_set::remove_non_straight_segments(kjb::Edge_set & edges, double threshold)
{
    std::vector<Edge_segment> copies;

    for(unsigned int k = 0; k < _segments.size(); k++)
    {
        if(_segments[k].get_least_squares_fitting_error() < threshold)
        {
            copies.push_back(_segments[k]);
        }
    }

    _segments.clear();
    _segments = copies;

    update_edge_set(edges);
}


/** This function will change the edge set thie edge segment set
 *  was built from, according to the changes that were made to
 *  this edge_segment set
 *
 *  @param edges The edge_set this edge_segment_set was built from
 */
void Edge_segment_set::update_edge_set(kjb::Edge_set & edges)
{
    /** First, we find all the edges that must be removed from
     * the edge_set (basically, all of those that are no longer
     * in the edge_segment_set) */
    unsigned int edges_size = edges.num_edges();
    std::vector<bool> _to_remove(edges_size, true);
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        unsigned int edge_index = edges.find_index(_segments[i].get_edge());
        if(! (_to_remove[edge_index] ))
        {
//          KJB_THROW_2(KJB_error, "Duplicated edges");
        }
        _to_remove[edge_index] = false;
    }

    /** We now remove the edges */
    unsigned int _num_removed = 0;
    for(unsigned int i = 0; i < edges_size; i++)
    {
        if(_to_remove[i])
        {
            edges.remove_edge( i - _num_removed);
//          std::cout << "Num edges:" << edges.num_edges() << std::endl;
            _num_removed++;
        }
    }

    /** We accordingly update the pointers */
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        kjb::Edge temp_edge = edges.get_edge(i);
        _segments[i].set_edge(temp_edge);
    }
}

kjb::Edge_set * Edge_segment_set::convert_to_edge_set(unsigned int num_rows, unsigned int num_cols)
{
    kjb::Vector lengths(_segments.size());

    unsigned int total_num_points = 0;
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        unsigned int length;
        kjb_c::find_edge_length_from_end_points(_segments[i].get_start_x(), _segments[i].get_start_y(),
                _segments[i].get_end_x(), _segments[i].get_end_y(),&length );
        total_num_points += length;
        lengths(i) = length;
    }

    kjb_c::Edge_set * edge_set = kjb_c::create_edge_set(total_num_points, _segments.size(),
                   lengths.get_c_vector(), num_rows, num_cols);
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        create_edge_from_end_points(_segments[i].get_start_x(), _segments[i].get_start_y(),
                //_segments[i].get_end_x(), _segments[i].get_end_y(), lengths(i), edge_set, i);
                _segments[i].get_end_x(), _segments[i].get_end_y(), edge_set, i);
    }
    return new kjb::Edge_set(edge_set);
}


kjb::Edge_set * Line_segment_set::convert_to_edge_set(unsigned int num_rows, unsigned int num_cols)
{
    kjb::Vector lengths(_segments.size());

    unsigned int total_num_points = 0;
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        unsigned int length;
        kjb_c::find_edge_length_from_end_points(_segments[i].get_start_x(), _segments[i].get_start_y(),
                _segments[i].get_end_x(), _segments[i].get_end_y(),&length );
        total_num_points += length;
        lengths(i) = length;
    }

    kjb_c::Edge_set * edge_set = kjb_c::create_edge_set(total_num_points, _segments.size(),
                   lengths.get_c_vector(), num_rows, num_cols);
    for(unsigned int i = 0; i < _segments.size(); i++)
    {
        create_edge_from_end_points(_segments[i].get_start_x(), _segments[i].get_start_y(),
                //_segments[i].get_end_x(), _segments[i].get_end_y(), lengths(i), edge_set, i);
                _segments[i].get_end_x(), _segments[i].get_end_y(), edge_set, i);
    }
    return new kjb::Edge_set(edge_set);
}


void kjb::detect_long_connected_segments(Line_segment_set & segments, const std::string & img_path, int max_length)
{
    char * path_to_edges = getenv ("PATH_TO_GET_LONG_STRAIGHT_EDGES");
    char * path_to_matlab = getenv ("PATH_TO_MATLAB");
    if(!path_to_edges)
    {
        KJB_THROW_2(KJB_error, "Could not read environment variable PATH_TO_GET_LONG_STRAIGHT_EDGES!!");
    }
    std::string command(path_to_edges);
    command.append("/run_APPgetLargeConnectedEdges.sh ");
    command.append(path_to_matlab);
    command.append(" ");
    command.append(img_path);
    command.append(" ./temp12345678.txt ");
    char number[100];
    sprintf(number, "%d", max_length);
    command.append(number);
    system(command.c_str());
    segments.read_from_Kovesi_format("./temp12345678.txt");
}
