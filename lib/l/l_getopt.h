
/* $Id: l_getopt.h 8758 2011-02-24 18:06:31Z predoehl $ */

/*
// Kobus. GNU getopt code, with a few non-substantive changes (scan for
// "Kobus"). The terms of use from FSF are below.
//
// One major cosmetic change: Protoiz'ed.
//
// Note that this code does not normally get compiled when the gnu library is
// available, unless we #define FORCE_COMPILE
*/

#ifndef L_GNU_GETOPT_H
#define L_GNU_GETOPT_H

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#ifdef GLOBAL_DEF
#    undef GLOBAL_DEF
#endif

#ifdef GLOBAL_INIT
#    undef GLOBAL_INIT
#endif

#ifdef GNU_GETOPT_ALLOCATE_GLOBAL_STORAGE
#    define GLOBAL_DEF
#    define GLOBAL_INIT(x) x
#else
#    define GLOBAL_DEF extern
#    define GLOBAL_INIT(x)
#endif


/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

GLOBAL_DEF char *gnu_optarg  GLOBAL_INIT( = NULL );

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns EOF, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `gnu_optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

GLOBAL_DEF int gnu_optind    GLOBAL_INIT( = 0 );

/* Callers store zero here to inhibit the error message `getopt' prints
   for unrecognized options.  */

GLOBAL_DEF int gnu_opterr    GLOBAL_INIT( = 1 );

/* Set to an option character which was unrecognized.  */

GLOBAL_DEF int gnu_optopt    GLOBAL_INIT( = '?' );

/* Describe the long-named options requested by the application.
   The LONG_OPTIONS argument to getopt_long or getopt_long_only is a vector
   of `struct option' terminated by an element containing a name which is
   zero.

   The field `has_arg' is:
   no_argument          (or 0) if the option does not take an argument,
   required_argument    (or 1) if the option requires an argument,
   optional_argument    (or 2) if the option takes an optional argument.

   If the field `flag' is not NULL, it points to a variable that is set
   to the value given in the field `val' when the option is found, but
   left unchanged if the option is not found.

   To have a long-named option do something other than set an `int' to
   a compiled-in constant, such as set a value from `gnu_optarg', set the
   option's `flag' field to zero and its `val' field to a nonzero
   value (the equivalent single-letter option character, if there is
   one).  For long options that have a zero `flag' field, `getopt'
   returns the contents of the `val' field.  */

struct Option  /* Kobus: option -> Option */
{
#if defined (__STDC__) && __STDC__
  const char *name;
#else
  char *name;
#endif
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */
/* Kobus: Changed them to upper-case. */
#define NO_ARGUMENT     0
#define REQUIRED_ARGUMENT   1
#define OPTIONAL_ARGUMENT   2

int gnu_getopt(int argc, char** argv, const char* optstring);

extern int gnu_getopt_long
(
    int                  argc,
    char**               argv,
    const char*          shortopts,
    const struct Option* longopts,
    int*                 longind
);

extern int gnu_getopt_long_only
(
    int                  argc,
    char**               argv,
    const char*          shortopts,
    const struct Option* longopts,
    int*                 longind
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif /* L_GNU_GETOPT_H */

