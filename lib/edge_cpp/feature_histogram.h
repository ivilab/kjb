/*
*@class - Geometric_context.cpp
*
*@author - Luca Del Pero
*
*@brief - creates an omap from an image that can call compare_omap(compareto)
*   to see how similar it is.
*/

//#include "Geometric_context.h"
#ifndef FEATURE_HISTOGRAM_H
#define FEATURE_HISTOGRAM_H

#include "i_cpp/i_image.h"
#include "i/i_float_io.h"
#include "i/i_float.h"
#include <stdio.h>
#include <l_cpp/l_exception.h>
#include <wrap_dtlib_cpp/texture.h>

namespace kjb{

#warning "[Code police] Please don't indent code using tabs, in libkjb."
#warning "[Code police] Please use a 4-space indent, instead."
class Fh_type
{
public:
    unsigned int element_1; // object_id or room
    unsigned int element_2; // object_polymesh, always the same for room
    unsigned int element_3; // polymesh_surface
    unsigned int extra_field; // for application purposes, for room it is the code
};

class Feature_histogram
{
public:
    class Image_dart
    {
    public:
        Image_dart(unsigned int irow, unsigned int icol)
        {
            _row = irow;
            _col = icol;
        }

        unsigned int _row;
        unsigned int _col;
    };

    Feature_histogram
    (
        DTLib::CImg<DTLib::FloatCHistogramPtr> & histo,
        unsigned int dart_window_size,
        unsigned int num_rows,
        unsigned int num_cols,
        int padding
    );

    void draw_darts(kjb::Image & img);

    inline unsigned int get_num_darts()
    {
        return darts.size();
    }

    void prepare_diff_matrix(DTLib::CImg<DTLib::FloatCHistogramPtr> & histo);

    double compute_score
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & previous_darts,
        std::vector<Fh_type> & new_darts,
        std::vector<bool> changed_darts,
        double previous_score
    );

    double compute_score
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts
    );

    double compute_score2
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & previous_darts,
        std::vector<Fh_type> & new_darts,
        std::vector<bool> changed_darts,
        double previous_score
    );

    double compute_score2
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts
    );

    double compute_score3
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts
    );

    double compute_score4
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts
    );

    const Matrix & get_diff_matrix() const
    {
        return diff_matrix;
    }

    void draw_room
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        kjb::Image & img
    );

    void draw_room_floor
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        kjb::Image & img
    );

    void draw_room_ceiling
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        kjb::Image & img
    );

    void draw_room_walls
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        kjb::Image & img
    );

    void draw_object
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        unsigned int obj_index,
        kjb::Image & img
    );


    void draw_object_polymesh
    (
        const kjb::Int_matrix & map,
        std::vector<Fh_type> & new_darts,
        unsigned int obj_index,
        unsigned int polymsh_index,
        kjb::Image & img
    ) const;

    double compute_differential
    (
        const kjb::Matrix & assignment_matrix
    ) const;

    const std::vector<Image_dart> & get_darts() const { return darts;}

private:

    double find_weight(const Fh_type & dart1, const Fh_type & dart2);

    double find_probability(const Fh_type & dart1, const Fh_type & dart2, double diff);

    bool find_probability2(const Fh_type & dart1, const Fh_type & dart2, double diff, double & prob);

    bool find_probability3(const Fh_type & dart1, const Fh_type & dart2, double diff, double & prob);

    std::vector<Image_dart> darts;
#warning "[Code police] Please don't start identifiers with underscore."
    unsigned int _num_rows;
    unsigned int _num_cols;
    unsigned int _padding;

    Matrix diff_matrix;
    Matrix differential_matrix;
    double all_different;

}; //class Feature_histogram

}//namespace kjb

#endif 
