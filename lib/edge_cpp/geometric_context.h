/*
*@class - Geometric_context.cpp
*
*@author - Josh Bowdish
*
*@brief - creates an omap from an image that can call compare_omap(compareto)
*   to see how similar it is.
*/

//#include "Geometric_context.h"
#ifndef GEOMETRIC_CONTEXT_H
#define GEOMETRIC_CONTEXT_H

#include "i_cpp/i_image.h"
#include "i/i_float_io.h"
#include "i/i_float.h"
#include <stdio.h>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_matrix.h>

#define MAX_NUM_ENTITIES 30

namespace kjb{

class Geometric_context{

enum Geometric_context_label {
      CENTRAL  = 0, // FLOOR
      LEFT,
      RIGHT,
      FLOOR,
      CEILING,
      OBJECT_1,
      OBJECT_2
  };

/* OLD:
 *       FLOOR  = 0,
      RIGHT,
      CENTRAL,
      LEFT,
      OBJECT_1,
      OBJECT_2,
      CEILING
 */


public:

Geometric_context(const Int_matrix & iseg_map, const Matrix & iprobabilities)
    : seg_map(iseg_map), probabilities(iprobabilities), num_segments(0)
{
    init_context();
}

Geometric_context(const std::string& seg_path, const std::string prob_path)
    : seg_map(seg_path.c_str()), probabilities(prob_path.c_str()), num_segments(0)
{
    init_context();
}

void draw_geometric_context(Image & img) const;

void draw_segmentation(Image & img) const ;

double compute_score(const kjb::Image & img) const;
double compute_score(const kjb::Int_matrix & map) const;
double compute_score
(
    const kjb::Int_matrix & map,
    kjb::Vector & individual_scores,
    int num_entities
) const;

void init_context();

void draw_segment(Image & img, int segment_index) const;

int get_num_segments() const
{
    return num_segments;
}

void convert_map_to_image
(
    const Int_matrix & imap,
    Image & img
);

void convert_map_to_edges
(
    const Int_matrix & imap,
    Image & img
);

private: 
  void count_segments();

    /*
     * We assume that the segment count starts from 1, not from zero
     */
    Matrix seg_map;
    Matrix probabilities;
    int num_segments;

}; //class Geometric_context

}//namespace kjb

#endif 
