/**
 * @file
 * @brief GSL utility stuff to help those using the C++ wrapper on GSL code.
 * @author Andrew Predoehl
 *
 * GSL is the GNU Scientific Library.
 */

/*
 * $Id: gsl_util.h 14342 2013-05-02 02:35:41Z predoehl $
 */

#ifndef GSL_UTIL_H_KJBLIB_UARIZONAVISION
#define GSL_UTIL_H_KJBLIB_UARIZONAVISION

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_errno.h"
#else
#warning "GNU GSL is absent"
#define GSL_SUCCESS 0
#endif

/*
 * @brief This throws an exception if its argument is a GSL error.
 * @throws KJB_error if the gsl_code is not equal to GSL_SUCCESS
 *
 * This macro is intended to be used like other KJB-library macros like ETE
 * or EPE:  it surrounds executable code that might return a GSL error.
 * In the common case the expression is GSL_SUCCESS and the macro does nothing.
 * When the argument is not GSL_SUCCESS, this will generate an error string
 * describing the flavor of GSL error, and throw a kjb::KJB_error object
 * containing the error string.
 *
 * This macro invokes an implementation function that is designed to be as
 * lightweight as possible, so that each use injects a minimum of code inline.
 */
#define GSL_ETX( gsl_expr ) kjb::Gsl_Etx_impl( (gsl_expr), __FILE__, __LINE__ )


namespace kjb {


/**
 * @brief Implements the error-handling activities of Gsl_Etx_impl().
 * @throws KJB_error if the gsl_code is not equal to GSL_SUCCESS
 *
 * Please use the GSL_ETX macro instead of calling this directly.
 */
void report_gsl_failure_and_throw_kjb_error(
    int gsl_code,
    const char* file,
    unsigned line_no
);


/**
 * @brief On GSL error of some kind, throw an KJB_error exception.
 * @throws KJB_error if the gsl_code is not equal to GSL_SUCCESS
 *
 * Please consider using the GSL_ETX macro instead of calling this directly.
 *
 * GSL is the GNU Scientific Library.
 * GSL routines usually return a status code that is either GSL_SUCCESS or
 * some sort of coded value describing the lack of success.
 * GSL error codes are describe at the link below:
 * http://www.gnu.org/software/gsl/manual/html_node/Error-Codes.html
 *
 * The design goal of this function is to be as efficient as possible to
 * inline, and therefore to defer as much as possible to the helper function 
 * report_gsl_failure_and_throw_kjb_error, which implements the error action.
 * That's why there are no local variables.
 */
inline
void Gsl_Etx_impl( int gsl_code, const char* file, unsigned line_no )
{
    if ( GSL_SUCCESS != gsl_code )
    {
        report_gsl_failure_and_throw_kjb_error( gsl_code, file, line_no );
    }
}

/// @brief On error, print a GSL iteration error message (like EPE)
void gsl_iterate_EPE(int gsl_error_code);

}

#endif
