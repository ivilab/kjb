/*!
 * @file Categorical_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Categorical_event.cpp 17097 2014-07-05 00:13:22Z cdawson $ 
 */

#include "semantics/Categorical_event.h"

namespace semantics
{

std::ostream& operator<<(std::ostream& os, const Categorical_event_base& e)
{
    e.print_to(os);
    return os;
}

};
