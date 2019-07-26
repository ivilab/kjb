
#ifndef COLLINS_PARSER_H
#define COLLINS_PARSER_H

#include <string>

/* Kobus. */ 
#include "spear/Word.h"
#include "spear/Tree.h"

namespace spear
{

    class Collins
    {
    public:
	static bool initialize(
	    const std::string&  modelDirectory,
	    double              beam = 10000
	    );
	
	static spear::TreePtr parse(
	    const std::vector<spear::Word>&  sentence,
	    double&                          prob,
	    bool                             discardNpbFlag = true, 
	    bool                             traceFlag = true);

    private:
	static bool initialized;
    };

}

#endif
