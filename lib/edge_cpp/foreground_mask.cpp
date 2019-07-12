/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Luca Del Pero, Kyle Simek, Andrew Predoehl.         |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: foreground_mask.cpp 9519 2011-05-18 17:54:30Z ksimek $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Yuanliu Liu
 *
 * @load the mask of foreground object into an integer matrix
 *
 *
 */

#include "edge_cpp/foreground_mask.h"
#include "m_cpp/m_matrix.h"
#include "i_cpp/i_image.h"
#include <cmath>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <string>

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


namespace kjb {

Foreground_mask::Foreground_mask(const std::string& file_name)
    : mask_matrix(file_name.c_str())
{
    total_size = mask_matrix.get_num_cols()*mask_matrix.get_num_rows();
}

double Foreground_mask::calc_overlap_ratio(const Int_matrix & predicted_mask)
{
	double total_overlap = 0.0;
	for(int i = 0; i < predicted_mask.get_num_rows(); i++)
	{
		for(int j = 0; j < predicted_mask.get_num_cols(); j++)
		{
            int entity = predicted_mask(i, j)&0x7f;
            if(entity > 0)
            {
            	entity = 1;
            }
            if(entity == mask_matrix(i, j))
            {
            	total_overlap += 1.0;
            }
		}
	}
	return (total_overlap/total_size);
}

void Foreground_mask::draw_foreground_image(const std::string & output_path)
{
    kjb::Image img(mask_matrix.get_num_rows(), mask_matrix.get_num_cols());
    for(int i = 0; i < mask_matrix.get_num_rows(); i++)
    {
    	for(int j = 0; j < mask_matrix.get_num_cols(); j++)
    	{
            if(mask_matrix(i, j) == 0)
            {
            	img(i, j, 0) = 0.0;
            	img(i, j, 1) = 0.0;
            	img(i, j, 2) = 0.0;
            }
            else
            {
            	img(i, j, 0) = 255.0;
            	img(i, j, 1) = 255.0;
            	img(i, j, 2) = 255.0;
            }
    	}
    }
    img.write(output_path.c_str());
}

}
