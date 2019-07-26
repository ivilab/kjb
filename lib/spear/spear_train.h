#ifndef SPEAR_TRAIN_H_
#define SPEAR_TRAIN_H_

/*!
 * @file spear_train.h
 *
 * @author Mihai Surdeanu
 * $Id: spear_train.h 16810 2014-05-16 00:12:58Z cdawson $ 
 */

#include <iostream>
#include <fstream>

#include "spear/Trainer.h"
#include "spear/Wide.h"

namespace spear
{
    void print_filename(const std::string& s, std::ostream& os);
    void train_collins(
	const std::string& input_filename,
	const std::string& output_dir,
	const int&         lf_thresh = 0,
	std::ostream&      os = std::cerr
	);
};

#endif
