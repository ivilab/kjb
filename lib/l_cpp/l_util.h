
/* $Id: l_util.h 18962 2015-04-28 00:51:55Z jguan1 $ */

#ifndef KJB_WRAP_UTIL_H
#define KJB_WRAP_UTIL_H

#include <stdlib.h>

#define KJB(x) do {using namespace kjb_c; x;} while(0)

namespace kjb {

void alert_untested( int line, const char* file );

/** @brief  Counts the total number of elements in a 2D STL-style container. */
template<class C>
size_t length(const C& cner)
{
    size_t len = 0;
    for(typename C::const_iterator p = cner.begin(); p != cner.end(); ++p)
    {
        len += p->size();
    }

    return len;
}

}

#endif


