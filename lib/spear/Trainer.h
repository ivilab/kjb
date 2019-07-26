#ifndef TRAINER_H_
#define TRAINER_H_

/*!
 * @file Trainer.h
 *
 * Generic parser trainer
 *
 * @author Mihai Surdeanu
 * $Id: Trainer.h 16866 2014-05-22 07:03:05Z cdawson $ 
 */

#include <iostream>

#include "spear/Lexicon.h"
#include "spear/Model.h"
#include "spear/Wide.h"

namespace spear {

    class Trainer : public RCObject
    {
    public:
	Trainer(const spear::ModelPtr & model);

	void setVerbose(int v) { verbose_ = v; };

	bool learn(
	    IStream&,
	    const std::string& modelDir
	    );

	static const int VERBOSE_LOW = 0;
	static const int VERBOSE_MEDIUM = 1;
	static const int VERBOSE_HIGH = 2;
  
    private:

	spear::LexiconPtr lexicon_;

	spear::ModelPtr model_;

	int verbose_;
    };

    typedef RCIPtr<spear::Trainer> TrainerPtr;

} // end namespace spear

#endif
