/*!
 * @file spear_train.cpp
 *
 * @author Colin Dawson 
 * $Id: spear_train.cpp 16950 2014-06-04 06:28:43Z cdawson $ 
 */

#include "spear/spear_train.h"

namespace spear {
void print_filename(const std::string& s, std::ostream& os)
{
    int i = 0;
    int start = -1;

    if((i = s.find_last_of('/')) > 0 && i < (int) s.size())
    {
	start = i;
    }

    os << s.substr(start + 1, s.size() - start) << " ";
    // exit(1);
}

void train_collins(
    const std::string& input_filename,
    const std::string& output_dir,
    const int&         lf_thresh,
    std::ostream&      os
    )
{
    spear::LF_WORD_THRESHOLD = lf_thresh;

    // input stream for parse trees
    std::ifstream input_file(input_filename.c_str()); 
    print_filename(input_filename, os);

    if(! input_file)
    {
	os << "Can not open " << input_filename << std::endl;
    }

    // creates a new model object
    spear::ModelPtr model(new Model(spear::LF_WORD_THRESHOLD)); 
    spear::Trainer trainer(model); // creates a trainer object
    trainer.setVerbose(Trainer::VERBOSE_MEDIUM);

    trainer.learn(input_file, output_dir.c_str());
}

};
