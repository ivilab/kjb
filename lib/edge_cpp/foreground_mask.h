/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Luca Del Pero, Andrew Predoehl.                                    |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: l_int_matrix.h 11118 2011-11-14 16:28:44Z ksimek $ */

#ifndef EDGE_CPP_FOREGROUND_MASK_H
#define EDGE_CPP_FOREGROUND_MASK_H

/** @file
 *
 * @author Luca Del Pero
 * @author Andrew Predoehl
 * @author Ernesto Brau
 *
 * @brief Definition for the Int_matrix class, a thin wrapper on the KJB
 *        Int_matrix struct and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * m_matrix.h,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 *
 * Although this class has much the same interface as class Matrix, they
 * are not derived from a common abstract interface, because (1) we want as
 * much speed as possible -- this code should be eligible to put inside a
 * tight inner loop; and (2) I don't know whether that would be useful.
 */

#include "m_cpp/m_int_matrix.h"
#include <ostream>
#define VERY_SMALL_POSITIVE_NUMBER 1e-20
/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


namespace kjb {

class Foreground_mask
{
public:
	typedef Int_matrix   Impl_type;  ///< the underlying implementation
	
	Foreground_mask(const std::string& file_name);
	
	~Foreground_mask()
	{

	}
	
	const Impl_type & get_c_matrix() const
    {
        // Test program was HERE.
        return mask_matrix;
    }

	void draw_foreground_image(const std::string & output_path);

	double calc_overlap_ratio(const Impl_type& predicted_mask);	
private:

	double total_size;
	Impl_type mask_matrix; // the mask matrix, 1 for foreground, 0 for background
};
}
	
#endif
