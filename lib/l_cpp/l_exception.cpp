
/* $Id: l_exception.cpp 19990 2015-10-29 16:06:00Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Kyle Simek, Joseph Schlecht, Luca Del Pero, Andrew Predoehl
|
* =========================================================================== */

#include <l/l_sys_lib.h>
#include "l_cpp/l_exception.h"
#include <sstream>
#include <iostream>

#include <vector>
#include <algorithm>


namespace kjb
{

/* --------------------------  Exception  ----------------------------------- */

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Exception::Exception(const char* msg, const char* file, unsigned line)
:   m_msg(msg)
{
    m_file = file;
    m_line = line;
#ifdef TEST
    m_msg = get_details(); // show file and line number in debugging mode
#endif // TEST
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Exception::Exception(const std::string& msg, const char* file, unsigned line)
:   m_msg(msg)
{
    m_file = file;
    m_line = line;
#ifdef TEST
    m_msg = get_details(); // show file and line number in debugging mode
#endif // TEST
}

/**
 * @param  e  Exception to copy into this one.
 */
Exception::Exception(const Exception& e)
:   m_msg(e.m_msg)
{
    m_file = e.m_file;
    m_line = e.m_line;
}


std::string Exception::get_details() const
{
    std::ostringstream ost;
    ost << m_file << ' ' << m_line << ": " << m_msg;
    return ost.str();
}

/**
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 */
void Exception::print(
        std::ostream& out,
        bool newline) const
{
    out << m_msg;

    if (newline) out << "\n";
}

void Exception::print() const
{
    return print(std::cerr, false);
}


/**
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 */
void Exception::print_details(
        std::ostream& out,
        bool newline) const

{
    out << get_details();
    if(newline)
        out << '\n';
}
void Exception::print_details() const 
{
    return print_details(std::cerr, false);
}


/**
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 */
void Exception::print_abort(
        std::ostream& out,
        bool newline) const
{
    print(out, newline);
    kjb_c::kjb_abort();
}

void Exception::print_abort() const
{
    return print_abort(std::cerr, false);
}

/**
 * @brief print exception message to given stream, and then abort the program.
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 */
void Exception::print_details_abort(
        std::ostream& out,
        bool newline) const
{
    print_details(out, newline);
    kjb_c::kjb_abort();
}

void Exception::print_details_abort() const
{
    return print_details_abort(std::cerr, false);
}


/**
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 * @param  status   Exit status.
 */
void Exception::print_exit(
        std::ostream& out,
        bool newline,
        int status) const
{
    print(out, newline);
    kjb_c::kjb_exit(status);
}

void Exception::print_exit() const
{
    return print_exit(std::cerr, false, EXIT_FAILURE);
}


/**
 * @param  out      Output stream to print to.
 * @param  newline  Whether to print a newline character.
 * @param  status   Exit status.
 */
void Exception::print_details_exit(
    std::ostream& out,
    bool newline,
    int status
)   const
{
    print_details(out, newline);
    kjb_c::kjb_exit(status);
}

void Exception::print_details_exit() const
{
    print_details_exit(std::cout, false, EXIT_FAILURE);
}
/* -------------------------------------------------------------------------- */


/* ----------------------  KJB_exception  ----------------------------------- */

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
KJB_error::KJB_error(const char* msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
KJB_error::KJB_error(const std::string& msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */


/* --------------------------  Index_out_of_bounds -------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Index_out_of_bounds::Index_out_of_bounds(const char* file, unsigned line)
:    Illegal_argument("Index out of bounds.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Index_out_of_bounds::Index_out_of_bounds(
    const char* msg,
    const char* file,
    unsigned line
)
:   Illegal_argument(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Index_out_of_bounds::Index_out_of_bounds(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Illegal_argument(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */


/* --------------------------  Not_implemented ------------------------------ */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Not_implemented::Not_implemented(const char* file, unsigned line)
:    Exception("Not implemented.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Not_implemented::Not_implemented(
    const char* msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Not_implemented::Not_implemented(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */


/* --------------------------  Runtime_error -------------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Runtime_error::Runtime_error(const char* file, unsigned line)
:    Exception("Unknown runtime error", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Runtime_error::Runtime_error(const char* msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Runtime_error::Runtime_error(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */
/* --------------------------  Cant_happen ---------------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Cant_happen::Cant_happen(const char* file, unsigned line)
:    Exception("Reached a block of unreachable code.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Cant_happen::Cant_happen(const char* msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Cant_happen::Cant_happen(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */

/* --------------------------  Illegal_argument ----------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Illegal_argument::Illegal_argument(const char* file, unsigned line)
:    Runtime_error("Illegal argument.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Illegal_argument::Illegal_argument(
    const char* msg,
    const char* file,
    unsigned line
)
:   Runtime_error(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Illegal_argument::Illegal_argument(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Runtime_error(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */


/* --------------------------  Dimension_mismatch --------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Dimension_mismatch::Dimension_mismatch(const char* file, unsigned line)
:    Illegal_argument("Dimension mismatch.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Dimension_mismatch::Dimension_mismatch(
    const char* msg,
    const char* file,
    unsigned line
)
:   Illegal_argument(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Dimension_mismatch::Dimension_mismatch(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Illegal_argument(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */

/* --------------------------  Divide_by_zero ------------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Divide_by_zero::Divide_by_zero(const char* file, unsigned line)
:    Runtime_error("Divide by zero.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Divide_by_zero::Divide_by_zero(const char* msg, const char* file, unsigned line)
:    Runtime_error(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Divide_by_zero::Divide_by_zero(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Runtime_error(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */

/* --------------------------  Serialization_error -------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Serialization_error::Serialization_error(const char* file, unsigned line)
:    Runtime_error("Serialization error.", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Serialization_error::Serialization_error(
    const char* msg,
    const char* file,
    unsigned line
)
:   Runtime_error(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Serialization_error::Serialization_error(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Runtime_error(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */

/* --------------------------  IO_error ----------------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
IO_error::IO_error(const char* file, unsigned line)
:    Exception("IO error", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
IO_error::IO_error(const char* msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
IO_error::IO_error(const std::string& msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/* -------------------------------------------------------------------------- */

/* --------------------------  IO_error ----------------------------------- */

/**
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Result_error::Result_error(const char* file, unsigned line)
:    Exception("Result error", file, line)
{
}

/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Result_error::Result_error(const char* msg, const char* file, unsigned line)
:    Exception(msg, file, line)
{
}


/**
 * @param  msg   Error message.
 * @param  file  File where the error occurred.
 * @param  line  Line approximately where the error occurred.
 */
Result_error::Result_error(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}


Missing_dependency::Missing_dependency(const char* file, unsigned line)
:   Exception("Missing dependency", file, line)
{
}


Missing_dependency::Missing_dependency(
    const char* dependency,
    const char* file,
    unsigned line
)
:   Exception(
        std::string("Operation failed due to a missing dependency: ")
            + dependency,
        file,
        line
    )
{
}

Missing_dependency::Missing_dependency(
    const std::string& dependency,
    const char* file,
    unsigned line
)
:   Exception(
        std::string("Operation failed due to a missing dependency: ")
            + dependency,
        file,
        line
    )
{
}

/* -------------------------------------------------------------------------- */

Option_exception::Option_exception(
    const std::string& msg,
    const char* file,
    unsigned line
)
:   Exception(msg, file, line)
{
}

/* -------------------------------------------------------------------------- */

Missing_option::Missing_option(
    const std::string& option,
    const char* file,
    unsigned line
)
:   Option_exception(
        "Option \"" + option + "\" required but not specified",
        file,
        line
    )
{
}

/* -------------------------------------------------------------------------- */

#if 0
Cant_happen_exception::Cant_happen_exception(const char* file, unsigned line)
:    Runtime_error("Reached a block of unreachable code.", file, line)
{
}
#endif

/* -------------------------------------------------------------------------- */

Stack_overflow::Stack_overflow(const char* file, unsigned line)
:    Runtime_error("Stack overflow", file, line)
{
}

/* -------------------------------------------------------------------------- */

Stack_underflow::Stack_underflow(const char* file, unsigned line)
:    Runtime_error("Stack underflow", file, line)
{
}

/* -------------------------------------------------------------------------- */


/// @brief similar to kjb_c::kjb_get_error(), but this returns std::string.
std::string kjb_get_error()
{
    /*
     * This is very low level code because it deals with reading a C-style
     * string of unknown size, via a C function.
     */
    const int err_bufsize = 1 + kjb_c::kjb_get_strlen_error();

    if ( err_bufsize <= 1 ) /* it should not be l.t. 1 but let's be paranoid */
    {
        return "";
    }

    std::vector< char > err_buf( err_bufsize, 0 );
    kjb_c::kjb_get_error( & err_buf.front(), err_bufsize );
    return & err_buf.front();
}


// "On error throw exception
void throw_kjb_error(const char* msg, const char* file, unsigned line)
{
    std::string kjb_err_msg = kjb_get_error();

    if ( 00 == msg || 0 == msg[ 0 ] )
    {
        throw kjb::KJB_error( kjb_err_msg, file, line );
    }

    throw kjb::KJB_error( msg + kjb_err_msg, file, line );
}


}
