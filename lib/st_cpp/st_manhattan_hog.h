/* $Id: edge.h 13438 2012-12-10 18:27:38Z predoehl $ */
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

#ifndef KJB_CPP_EDGE_MANHATTAN_HOG_H
#define KJB_CPP_EDGE_MANHATTAN_HOG_H

#include <string>

#include <edge_cpp/hog.h>
#include "st_cpp/st_parapiped.h"
#include "st_cpp/st_perspective_camera.h"
#include "edge_cpp/line_segment.h"

namespace kjb
{

void compute_HOG_features(const Image & img, int bin_size);

class Manhattan_hog: public Readable, public Writeable
{
public:
    Manhattan_hog
    (
        const Hog_responses & hog_responses,
        const Image & image,
        const Perspective_camera & camera,
        const Parametric_parapiped & parapiped
    );

    Manhattan_hog(const std::string & file_name)
    {
        Readable::read(file_name.c_str());
    }

    Manhattan_hog(const Manhattan_hog & src) : Readable(src), Writeable(src), manhattan_hog_size(src.manhattan_hog_size)
    {
        hog_responses_horizontal = new float[manhattan_hog_size];
        memcpy(hog_responses_horizontal, src.hog_responses_horizontal, manhattan_hog_size*sizeof(float));
        hog_responses_vertical_1 = new float[manhattan_hog_size];
        memcpy(hog_responses_vertical_1, src.hog_responses_vertical_1, manhattan_hog_size*sizeof(float));
        hog_responses_vertical_2 = new float[manhattan_hog_size];
        memcpy(hog_responses_vertical_2, src.hog_responses_vertical_2, manhattan_hog_size*sizeof(float));
        num_hog_cols = src.num_hog_cols;
        num_hog_rows = src.num_hog_rows;
        hog_maxs = src.hog_maxs;
        bin_size = src.bin_size;
    }

    ~Manhattan_hog()
    {
        delete[] hog_responses_horizontal;
        delete[] hog_responses_vertical_1;
        delete[] hog_responses_vertical_2;
    }

    Manhattan_hog& operator=(const Manhattan_hog& src)
    {
        delete[] hog_responses_horizontal;
        delete[] hog_responses_vertical_1;
        delete[] hog_responses_vertical_2;
        hog_responses_horizontal = NULL;
        hog_responses_vertical_1 = NULL;
        hog_responses_vertical_2 = NULL;
        hog_responses_horizontal = new float[manhattan_hog_size];
        memcpy(hog_responses_horizontal, src.hog_responses_horizontal, manhattan_hog_size*sizeof(float));
        hog_responses_vertical_1 = new float[manhattan_hog_size];
        memcpy(hog_responses_vertical_1, src.hog_responses_vertical_1, manhattan_hog_size*sizeof(float));
        hog_responses_vertical_2 = new float[manhattan_hog_size];
        memcpy(hog_responses_vertical_2, src.hog_responses_vertical_2, manhattan_hog_size*sizeof(float));
        num_hog_cols = src.num_hog_cols;
        num_hog_rows = src.num_hog_rows;
        hog_maxs = src.hog_maxs;
        bin_size = src.bin_size;
        return (*this);
    }

    /** @brief Reads this Line segment from an input stream. */
    void read(std::istream& in);

    /** @brief Writes this Line segment to an output stream. */
    void write(std::ostream& out) const;

    int compute_manhattan_hog_size(int hog_size_2D)
    {
        return (hog_size_2D/32)*27;
    }

    void get_3D_manhattan_directions
    (
        std::vector<kjb::Vector> & manhattan_dir_3d_vertical,
        std::vector<kjb::Vector> & manhattan_dir_3d_vertical1,
        std::vector<kjb::Vector> & manhattan_dir_3d_vertical2,
        int num_directions
    ) const;

    void get_projected_manhattan_directions
    (
        const std::vector<kjb::Vector> & manhattan_dir_3d_vertical,
        const std::vector<kjb::Vector> & manhattan_dir_3d_vertical1,
        const std::vector<kjb::Vector> & manhattan_dir_3d_vertical2,
        const kjb::Vector & center_in_pp_coordinates,
        const kjb::Parametric_parapiped & pp,
        const kjb::Perspective_camera & camera,
        int img_height,
        std::vector<kjb::Vector> & projected_manhattan_dir_horizontal,
        std::vector<kjb::Vector> & projected_manhattan_dir_vertical1,
        std::vector<kjb::Vector> & projected_manhattan_dir_vertical2
    ) const;

    void interpolate_projected_directions
    (
        const std::vector<kjb::Vector> & projected_directions,
        const float * weights_2D,
        float * weights_3D,
        double angle_increment,
        int inum_bins
    );

    void get_line_segments_for_drawing
    (
        std::vector<Line_segment> & line_segments,
        const std::vector<kjb::Vector> & projected_directions,
        double center_x,
        double center_y,
        double length
    ) const;

    Image draw_image_with_vertical_segments
    (
        const Hog_responses & hog_responses,
        const Image & image,
        const Perspective_camera & camera,
        const Parametric_parapiped & parapiped
    ) const;

    void get_hog_pixels(std::vector<std::pair<int, int> > & hog_pixels) const;

    void get_hog_offsets(std::vector<int>& hog_offsets) const;

    inline int get_hog_cell_x_center(int hog_x_index) const
    {
        return 1.5 + bin_size + (bin_size*hog_x_index);
    }

    inline int get_hog_cell_y_center(int hog_y_index) const
    {
        return 1.5 + bin_size + (bin_size*hog_y_index);
    }

    inline const float * get_horizontal() const
	{
    	return hog_responses_horizontal;
	}

    inline const float * get_vertical1() const
	{
    	return hog_responses_vertical_1;
	}

    inline const float * get_vertical2() const
	{
    	return hog_responses_vertical_2;
	}

    inline const Vector & get_hog_maxs() const
    {
    	return hog_maxs;
    }

    inline int get_num_hog_rows() const
    {
    	return num_hog_rows;
    }

    inline int get_num_hog_cols() const
    {
    	return num_hog_cols;
    }

private:
    int manhattan_hog_size;
    int num_hog_rows;
    int num_hog_cols;
    float * hog_responses_horizontal;
    float * hog_responses_vertical_1;
    float * hog_responses_vertical_2;
    Vector hog_maxs;
    int bin_size;

    void find_2D_hog_directions(std::vector<kjb::Vector> & directions_2D, int num_angle_bins) const;

    double find_2D_weight_by_interpolation
    (
        const Vector & idirection,
        double angle_increment,
        int inum_bins,
        const float * weights
    ) const;
};

} // namespace kjb
#endif
