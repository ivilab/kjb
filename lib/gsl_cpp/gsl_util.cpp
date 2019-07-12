/**
 * @file
 * @brief GSL utility stuff to help those using the C++ wrapper on GSL code.
 * @author Andrew Predoehl
 *
 * GSL is the GNU Scientific Library.
 */

/*
 * $Id: gsl_util.cpp 21357 2017-03-30 05:35:22Z kobus $
 */

#include <l/l_sys_io.h>
#include <l_cpp/l_exception.h>

#include "gsl_cpp/gsl_util.h"

#include <sstream>

namespace kjb {

void report_gsl_failure_and_throw_kjb_error(
    int gsl_code,
    const char* file,
    unsigned line_no
)
{
#ifdef KJB_HAVE_GSL
    if ( gsl_code != GSL_SUCCESS )
    {
        std::ostringstream oss;
        oss << "GSL Error E" << gsl_code << ", "
                                        << gsl_strerror( gsl_code ) << '\n';
        kjb::throw_kjb_error( oss.str().c_str(), file, line_no );
    }
#endif
}


void gsl_iterate_EPE(int gsl_error_code)
{
#ifdef KJB_HAVE_GSL
    if (gsl_error_code != GSL_SUCCESS)
    {
        kjb_c::p_stderr("iterate failure E%d: %s\n", gsl_error_code,
                                                     gsl_strerror(gsl_error_code));
    }
#endif
}


}
