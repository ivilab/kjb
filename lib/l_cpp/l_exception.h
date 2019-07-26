/**
 * @file
 * @brief Support for error handling exception classes in libKJB
 * @author Kobus Barnard
 * @author Joseph Schlect
 * @author Kyle Simek
 * @author Luca Del Pero
 * @author Andrew Predoehl
 */

/*
 * $Id: l_exception.h 21596 2017-07-30 23:33:36Z kobus $
 */

/* =========================================================================== *
|
|  Copyright (c) 1994-2010
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
|    Kobus Barnard, Kyle Simek, Joseph Schlecht, Luca Del Pero, Andrew Predoehl
|
* =========================================================================== */

#ifndef KJB_EXCEPTION_H
#define KJB_EXCEPTION_H

#include "l/l_sys_err.h"
#include "l/l_sys_str.h"
#include "l/l_string.h"
#include <boost/preprocessor/seq.hpp>
#include <exception>
#include <string>
#include <iosfwd>

#define KJB_THROW(ex) throw ex(__FILE__, __LINE__)

#define KJB_THROW_2(ex, msg) throw ex(msg, __FILE__, __LINE__)

/*
 * The do { ... } while(0) structure below avoids the mistake known as
 * "swallowing the semicolon."
 */

// usage: KJB_THROW_3(Exception, format_string, (param1)(param2)(param3)(...))
#define KJB_THROW_3(ex, fmt, params)                            \
    do                                                          \
    {                                                           \
        char buffer[ERROR_MESS_BUFF_SIZE];                      \
        kjb_c::kjb_sprintf(buffer, ERROR_MESS_BUFF_SIZE, fmt,   \
                            BOOST_PP_SEQ_ENUM(params));         \
        throw ex(buffer, __FILE__, __LINE__);                   \
    }                                                           \
    while(0)

// "On error throw exception" -- in this case, "error" is ANY nonzero value.
#define ETX(a)                                                  \
    do                                                          \
    {                                                           \
        if (a)                                                  \
        {                                                       \
            kjb::throw_kjb_error( "", __FILE__, __LINE__ );     \
        }                                                       \
    }                                                           \
    while(0)


#define ETX_2(a, msg)                                           \
    do                                                          \
    {                                                           \
        if (a)                                                  \
        {                                                       \
            kjb::throw_kjb_error(msg, __FILE__, __LINE__);      \
        }                                                       \
    }                                                           \
    while(0)

// "On null throw exception"
#define NTX(a)                                                  \
    do                                                          \
    {                                                           \
        if ((a) == NULL)                                        \
        {                                                       \
            kjb::throw_kjb_error( "Null result",                \
                                        __FILE__, __LINE__ );   \
        }                                                       \
    }                                                           \
    while(0)

// "If False Throw (exception)" -- throws 'ex' if 'a' is false.
#define IFT(a, ex, msg)                                         \
    do                                                          \
    {                                                           \
        if(!(a))                                                \
        {                                                       \
            KJB_THROW_2(ex, msg);                               \
        }                                                       \
    }                                                           \
    while(0)                                                    \

// "If False Throw (exception) with details" -- throws 'ex' if 'a' is false.
#define IFTD(a, ex, msg, params)                                \
    do                                                          \
    {                                                           \
        if(!(a))                                                \
        {                                                       \
            KJB_THROW_3(ex, msg, params);                       \
        }                                                       \
    }                                                           \
    while(0)                                                    \




namespace kjb {

/**
 * @class Exception
 *
 * @brief Base class of all exceptions in the jwsc++ library.
 */
class Exception  : public std::exception
{
public:

    /** @brief Constructs an Exception. */
    Exception(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception. */
    Exception(const std::string& msg, const char* file=0, unsigned line=0);

    /** @brief Constructs an Exception from another */
    Exception(const Exception& e);


    /** @brief Deletes an Exception. */
    virtual ~Exception() throw() { }


    /** @brief Returns the error message for the Exception. */
    const std::string& get_msg() const throw()
    {
        return m_msg;
    }

    /** @brief Returns the error message for the Exception. */
    const char* what() const throw()
    {
        return m_msg.c_str();
    }


    /** @brief Returns the file name where the Exception occurred. */
    const char* get_file() const
    {
        return m_file;
    }


    /** @brief Returns the line number near where the Exception occurred. */
    unsigned get_line() const
    {
        return m_line;
    }

    /** @brief Return string containing filename, line number, and message. */
    std::string get_details() const;


    /** @brief Prints the Exception::msg to an ostream. */
    virtual void print(
        std::ostream& out, // = std::cerr
        bool newline = false
    )   const;

    virtual void print() const;



    /** @brief Prints all the fields of the Exception to an ostream. */
    virtual void print_details(
        std::ostream& out, // = std::cerr
        bool newline = false
    )   const;

    virtual void print_details() const;

    /** @brief Prints the Exception::msg to an ostream and aborts. */
    virtual void print_abort(
        std::ostream& out, // = std::cerr
        bool newline = false
    )   const;

    virtual void print_abort() const; 

    /**
     * @brief Prints all the fields of the Exception to an ostream and
     * aborts.
     */
    virtual void print_details_abort
    (
        std::ostream& out, // = std::cerr
        bool     newline = false
    )
    const;

    virtual void print_details_abort() const;


    /** @brief Prints the Exception::msg to an ostream and exits. */
    virtual void print_exit
    (
        std::ostream& out,
        bool     newline, // = std::cerr
        int      status = false
    )
    const;

    virtual void print_exit() const;


    /**
     * @brief Prints all the fields of the Exception to an ostream and
     * exits.
     */
    virtual void print_details_exit
    (
        std::ostream& out, 
        bool     newline = false,
        int      status = EXIT_FAILURE
    )
    const;

    virtual void print_details_exit() const;


protected:

    /** @brief Message associated with the error causing the exception. */
    std::string m_msg;

    /** @brief File name where the Error occurred. */
    const char* m_file;

    /** @brief Line number where the Error occurred. */
    unsigned m_line;
};


/// @brief Exception often thrown when wrapped C functions return error codes.
class KJB_error : public Exception
{
public:
    /// @brief Constructs Exception caused by an error in the KJB C library
    KJB_error(const char* msg, const char* file=0, unsigned line=0);


    /// @brief Constructs Exception caused by an error in the KJB C library
    KJB_error(const std::string& msg, const char* file=0, unsigned line=0);


    /** @brief Deletes a KJB_error. */
    virtual ~KJB_error() throw() { }

};



/// @brief Object thrown when attempting to use unimplemented functionality.
class Not_implemented : public Exception
{
public:
    /** @brief Constructs an Exception caused by attempting
     * to use un-implemented functionality. */
    Not_implemented(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by attempting
     * to use un-implemented functionality. */
    Not_implemented(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by attempting
     * to use un-implemented functionality. */
    Not_implemented(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );


    /** @brief Deletes a Not_implemented. */
    virtual ~Not_implemented() throw() { }

};

/**
 * @brief Object thrown when computation fails somehow during execution.
 *
 * This class is primarily meant to be a base class to other exceptions.
 *
 * In a sense, all thrown exceptions represent a run-time condition, because
 * you cannot throw before your program is running, or after it is running.
 * However, sometimes we think of certain tasks as central to a computation
 * (such as linear algebra, fourier transforms, etc.) and other tasks as
 * ancillary (such as option-parsing, filesystem IO, or execution as mediated
 * by macros like KJB_HAVE_LAPACK).  Failures in the "central" computational
 * tasks of a program are represented by throwing Runtime_error objects,
 * preferably more-specialized descendant classes than this base class.
 */
class Runtime_error : public Exception
{
public:
    /** @brief Constructs a runtime exception */
    Runtime_error(const char* file, unsigned line);

    /** @brief Constructs a runtime exception */
    Runtime_error(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs a runtime exception */
    Runtime_error(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes a Runtime_error. */
    virtual ~Runtime_error() throw() { }

};

/**
 * @brief Object thrown when the program does something thought impossible.
 *
 * Throwing this object is appropriate when control reaches a lexical
 * location of the source code that was thought to be unreachable.
 * Alternatively, it might be helpful to think of throwing this when the
 * program's internal state changes in a way thought to be impossible.
 *
 * @code
 *  int rc = fclose( fp ); // as everyone knows, EOF if error, or 0 for success
 *  if ( 0 == rc )         cout << "closed the file\n";
 *  else if ( EOF == rc )  cerr << "cannot close the file\n";
 *  else                   throw Cant_happen(__FILE__,__LINE__);
 * @endcode
 */
class Cant_happen : public Exception
{
public:
    /// @brief Ctor of Exception caused reaching presumably unreachable code
    Cant_happen(const char* file, unsigned line);

    /// @brief Ctor of Exception caused reaching presumably unreachable code
    Cant_happen(const char* msg, const char* file=0, unsigned line=0);

    /// @brief Ctor of Exception caused reaching presumably unreachable code
    Cant_happen(const std::string& msg, const char* file=0, unsigned line=0);

    /// @brief Deletes a Cant_happen.
    virtual ~Cant_happen() throw() { }

};


/// @brief Object thrown when an argument to a function is not acceptable
class Illegal_argument : public Runtime_error
{
public:
    /// @brief Ctor for passing an illegal argument to a method.
    Illegal_argument(const char* file, unsigned line);

    /// @brief Ctor for passing an illegal argument to a method.
    Illegal_argument(const char* msg, const char* file=0, unsigned line=0);

    /// @brief Ctor for passing an illegal argument to a method.
    Illegal_argument(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /// @brief Deletes a Illegal_argument. */
    virtual ~Illegal_argument() throw() { }
};


/// @brief Object thrown when an index argument exceeds the size of a container
class Index_out_of_bounds : public Illegal_argument
{
public:
    /** @brief Constructs an Exception caused by attempting to
     * access an index outside the bounds of a collection. */
    Index_out_of_bounds(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by attempting to
     * access an index outside the bounds of a collection. */
    Index_out_of_bounds(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by attempting to
     * access an index outside the bounds of a collection. */
    Index_out_of_bounds(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );


    /** @brief Deletes a Index_out_of_bounds. */
    virtual ~Index_out_of_bounds() throw() { }
};


/// @brief Object thrown when an argument is of the wrong size or dimensions.
class Dimension_mismatch : public Illegal_argument
{
public:
    /** @brief Constructs an Exception caused by dimension mismatch */
    Dimension_mismatch(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by dimension mismatch */
    Dimension_mismatch(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by dimension mismatch */
    Dimension_mismatch(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes a Dimension_mismatch. */
    virtual ~Dimension_mismatch() throw() { }

};


/// @brief Object thrown when an division is attempted with a zero divisor
class Divide_by_zero : public Runtime_error
{
public:
    /** @brief Constructs an Exception caused by dividing by zero */
    Divide_by_zero(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by dividing by zero */
    Divide_by_zero(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by dividing by zero */
    Divide_by_zero(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes a Divide_by_zero. */
    virtual ~Divide_by_zero() throw() { }

};

/// @brief Object thrown when serialization or deserialization fails
class Serialization_error : public Runtime_error
{
public:
    /** @brief Constructs an Exception caused by a serialization error. */
    Serialization_error(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by a serialization error. */
    Serialization_error(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by a serialization error. */
    Serialization_error(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes a Serialization_error. */
    virtual ~Serialization_error() throw() { }

};

/// @brief Object thrown when input or output fails
class IO_error : public Exception
{
public:
    /** @brief Constructs an Exception caused by an IO error. */
    IO_error(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by an IO error. */
    IO_error(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by an IO error */
    IO_error(const std::string& msg, const char* file=0, unsigned line=0);


    /** @brief Deletes an IO error. */
    virtual ~IO_error() throw() { }

};

/// @brief Object thrown when a function cannot generate a valid result.
class Result_error : public Exception
{
public:
    /** @brief Constructs an Exception caused by a function generating an
     * invalid result. */
    Result_error(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by a function generating an
     * invalid result. */
    Result_error(const char* msg, const char* file=0, unsigned line=0);


    /** @brief Constructs an Exception caused by a function generating an
     * invalid result. */
    Result_error(const std::string& msg, const char* file=0, unsigned line=0);


    /** @brief Deletes a Result error. */
    virtual ~Result_error() throw() { }

};

/// @brief Object thrown when a program lacks required resources or libraries
class Missing_dependency : public Exception
{
public:
    /** @brief Constructs an Exception caused by a missing dependency. */
    Missing_dependency(const char* file, unsigned line);

    /** @brief Constructs an Exception caused by a missing dependency. */
    Missing_dependency(
        const char* dependency,
        const char* file=0,
        unsigned line=0
    );


    /** @brief Constructs an Exception caused by a missing dependency. */
    Missing_dependency(
        const std::string& dependency,
        const char* file=0,
        unsigned line=0
    );


    /** @brief Deletes exception. */
    virtual ~Missing_dependency() throw() { }

};

/// @brief Object thrown for exceptions related to command-line options
class Option_exception : public Exception
{
public:
    /** @brief Constructs an Exception related to command-line options. */
    Option_exception(
        const std::string& msg,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes exception. */
    virtual ~Option_exception() throw() { }
};

/// @brief Object thrown when an obligatory command-line option is absent
class Missing_option : public Option_exception
{
public:
    /// @brief Constructs an Exception caused by a missing command-line option.
    Missing_option(
        const std::string& option,
        const char* file=0,
        unsigned line=0
    );

    /** @brief Deletes exception. */
    virtual ~Missing_option() throw() { }
};

/// @deprecated Use Cant_happen instead.
typedef Cant_happen Cant_happen_exception;
#if 0
class Cant_happen_exception : public Runtime_error
{
public:
    /** @brief Constructs an Exception caused by reaching a line of code
     * that shouldn't be reachable. */
    Cant_happen_exception(const char* file=0, unsigned line=0);

    /** @brief Deletes exception. */
    virtual ~Cant_happen_exception() { } throw()
};
#endif

/// @brief Object thrown if a finite size stack is full and pushed farther
class Stack_overflow : public Runtime_error
{
public:
    /** @brief Constructs an Exception caused by a stack overflow. */
    Stack_overflow(const char* file=0, unsigned line=0);

    /** @brief Deletes exception. */
    virtual ~Stack_overflow() throw() { }
};


/// @brief Object thrown if stack is empty and popped.
class Stack_underflow : public Runtime_error
{
public:
    /** @brief Constructs an Exception caused by a stack underflow. */
    Stack_underflow(const char* file=0, unsigned line=0);

    Stack_underflow(const char* msg, const char* file=0, unsigned line=0)
    :   Runtime_error(msg, file, line)
    {}

    /** @brief Deletes exception. */
    virtual ~Stack_underflow() throw() {}
};


/// @brief Object thrown if a resource allocation failed (memory, fp, etc.)
class Resource_exhaustion : public Runtime_error
{
public:
    /// @brief Constructs an Exception caused by a stack underflow. */
    Resource_exhaustion(const char* file=0, unsigned line=0)
    :   Runtime_error("Resource exhaustion", file, line)
    {}

    /// @brief Deletes exception.
    virtual ~Resource_exhaustion() throw() {}
};


std::string kjb_get_error();

void throw_kjb_error( const char* msg, const char* file, unsigned line);

} // namespace kjb


#endif
