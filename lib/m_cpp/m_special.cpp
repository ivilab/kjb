/* $Id: m_special.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include "m_cpp/m_special.h"
#include <boost/math/special_functions/factorials.hpp>

double kjb::log_binomial_coefficient(size_t n, size_t k)
{
   if((k == 0) || (k == n))
      return 0;
   if((k == 1) || (k == n-1))
      return log(n);

   if(n <= boost::math::max_factorial<double>::value)
   {
      double iresult; 
      // Use fast table lookup:
      iresult = boost::math::unchecked_factorial<double>(n);
      iresult /= boost::math::unchecked_factorial<double>(n-k);
      iresult /= boost::math::unchecked_factorial<double>(k);

      // these should all be within reasonable limits of precision, so we do log after all operations
      return log(ceil(iresult + 0.5));
   }
   else
   {
        double total;
        total  = lgamma(n+1);
        total -= lgamma(k+1);
        total -= lgamma(n - k+1);
        return total;
   }
}
