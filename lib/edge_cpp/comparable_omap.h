/**
 * @file
 * @author Josh Bowdish
 * @brief definition of Comparable_omap class
 */

//#include "Comparable_omap.h"
#ifndef COMPAREABLE_OMAP_H
#define COMPAREABLE_OMAP_H

#include "i_cpp/i_image.h"
#include "i/i_float_io.h"
#include "i/i_float.h"
#include <stdio.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_int_matrix.h>
//#include < boost/multi_array.hpp >


namespace kjb{

/**
 * @brief - creates an omap from an image that can call compare_omap(compareto)
 *  to see how similar it is.
 */
class Comparable_omap{

public:

Comparable_omap(const Image &basein)
{
    //base = basein;
    //make_opposite_omap();
    _num_cols = basein.get_num_cols();
    _num_rows = basein.get_num_rows();
    prepare_int_maps(basein);
    totalbaseblack = get_black(basein);
    total = basein.get_num_rows()*basein.get_num_cols();
}

Comparable_omap(const std::string& fname) // : base(fname.c_str())
{
    Image tempimg(fname.c_str());
    _num_cols = tempimg.get_num_cols();
    _num_rows = tempimg.get_num_rows();
    //make_opposite_omap();
    prepare_int_maps(tempimg);
    totalbaseblack = get_black(tempimg);
    total = tempimg.get_num_rows()*tempimg.get_num_cols();
}

void prepare_int_maps(const kjb::Image & base);

//double compare_omap(const Image & comparetoin) const;

double compare_omap(const Int_matrix & imap) const;

/*double match_pixels
(
    const Image  & basein,
    const Image  & comparetoin
) const;*/

/*const kjb::Image & get_base()
{
    return base;
}

const kjb::Image & get_reversed()
{
    return reversedbase;
}*/

void convert_map_to_image
(
    const Int_matrix & imap,
    Image & img
);

double compare_omap_integral
(
    const std::vector<kjb::Int_vector> & surface_changes,
    int surface_counter
) const;


//void make_opposite_omap();

private: 
    //Image base;
    //Image reversedbase;

    Int_matrix map_base;

    Int_matrix accum_hor;
    Int_matrix accum_ver1;
    Int_matrix accum_ver2;


    double percentright;
    int totalbaseblack;
    int total;

#warning "[Code police] Please don't create identifier names that start with"
#warning "[Code police] an underscore.  Such identifiers are reserved."
    int _num_cols;
    int _num_rows;

int get_black(const Image & howblack);

}; //class Comparable_omap

}//namespace kjb

#endif 
