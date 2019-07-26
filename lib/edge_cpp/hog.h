/* $Id: hog.h 18278 2014-11-25 01:42:10Z ksimek $ */
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
   |  Author:  Luca del Pero
 * =========================================================================== */

#ifndef KJB_CPP_EDGE_HOG_H
#define KJB_CPP_EDGE_HOG_H

#include <l/l_incl.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>
#include <cmath>

#include <string>

// small value, used to avoid division by zero
#define eps 0.0001

namespace kjb
{

static inline float hog_min(float x, float y) { return (x <= y ? x : y); }
static inline float hog_max(float x, float y) { return (x <= y ? y : x); }

static inline int hog_min(int x, int y) { return (x <= y ? x : y); }
static inline int hog_max(int x, int y) { return (x <= y ? y : x); }

void compute_HOG_features(const Image & img, int bin_size);

class Hog_responses: public Readable, public Writeable
{
public:
    Hog_responses(const Image & img, int ibin_size);

    Hog_responses(const std::string & file_name)
    {
        hog_responses = NULL;
        Readable::read(file_name.c_str());
    }

    Hog_responses(const Hog_responses & src) : Readable(src), Writeable(src), bin_size(src.bin_size),
            num_rows(src.num_rows), num_cols(src.num_cols), hog_num_rows(src.hog_num_rows),
            hog_num_cols(src.hog_num_cols), hog_size(src.hog_size)
    {
        hog_responses = new float[hog_size];
        memcpy(hog_responses, src.hog_responses, hog_size*sizeof(float));
    }

    ~Hog_responses()
    {
        delete[] hog_responses;
    }

    Hog_responses& operator=(const Hog_responses& src)
    {
        bin_size = src.bin_size;
        num_rows = src.num_rows;
        num_cols = src.num_cols;
        hog_num_rows = src.hog_num_rows;
        hog_num_cols = src.hog_num_cols;
        hog_size = src.hog_size;
        delete[] hog_responses;
        hog_responses = NULL;
        hog_responses = new float[hog_size];
        memcpy(hog_responses, src.hog_responses, hog_size*sizeof(float));
        return (*this);
    }

    inline int get_hog_num_rows() const
    {
        return ((int)round((double)num_rows/(double)bin_size)) -2;
    }

    inline int get_hog_num_cols() const
    {
        return ((int)round((double)num_cols/(double)bin_size)) -2;
    }

    int get_num_rows() const
    {
        return num_rows;
    }

    inline int get_num_cols() const
    {
        return num_cols;
    }

    inline int get_hog_pix_row(int img_pix_row) const
    {
        return (int)std::floor(((double)img_pix_row + 0.5)/(double)bin_size - 0.5);
    }

    inline int get_hog_pix_col(int img_pix_col) const
    {
        return (int)std::floor(((double)img_pix_col + 0.5)/(double)bin_size - 0.5);
    }

    inline int get_hog_cell_x_center(int hog_x_index) const
    {
        return 1.5 + bin_size + (bin_size*hog_x_index);
    }

    inline int get_hog_cell_y_center(int hog_y_index) const
    {
        return 1.5 + bin_size + (bin_size*hog_y_index);
    }

    void get_HOG_picture(Image & positive_image, Image & negative_image, bool & negative_weights);

    void get_bims(std::vector<Matrix> & bim);

    /** @brief Reads this Line segment from an input stream. */
    void read(std::istream& in);

    /** @brief Writes this Line segment to an output stream. */
    void write(std::ostream& out) const;

    inline int get_hog_size() const
    {
        return hog_size;
    }

    inline const float * get_hog_responses() const
    {
        return hog_responses;
    }

    inline int get_bin_size() const
    {
    	return bin_size;
    }

private:
    int bin_size;
    int num_rows;
    int num_cols;
    int hog_num_rows;
    int hog_num_cols;
    int hog_size;
    float * hog_responses;
};

void rotate_matrix_90_degrees(Matrix & m, int number_of_times);

/** angle is in degrees!! */
void rotateMatrix(Matrix & m, double angle);

} // namespace kjb
#endif
