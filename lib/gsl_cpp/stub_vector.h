/**
 * @file
 * @brief stub for GSL vector (when GSL is unavailable) -- not executed.
 * @author Andrew Predoehl
 *
 * When GSL is unavailable, the gsl_cpp wrapper code is designed so that it
 * will still compile, and will fail (only) at run-time.  We do not want it
 * to fail at compile time, according to lab convention.  Instead of adding
 * millions of ifdef-endif pairs all over the GSL code, it is easier to
 * create a dummy type definition for gsl_vector, and its basic support
 * functions.  Again, they only are compiled when GSL is absent.  The code is
 * never executed, because the class constructors throw an exception, and no
 * such object is ever instantiated.  That is why the code is unreachable.
 * 
 * Therefore, this is an extremely unimportant file.  None of the code will be
 * executed.  It is here just as an attempt to trick the compiler not to
 * complain.  It's such a strange case that I wanted to isolate it
 * to a separate file that you, the user, can almost always ignore.
 *
 * Furthermore, you need never include this file; it is already conditionally
 * included by gsl_vector.h exactly when GSL is absent.  Just ignore it.
 */

/*
 * $Id: stub_vector.h 19607 2015-07-17 00:11:07Z predoehl $
 */

#ifndef GSL_STUB_VECTOR_H_KJBLIB_UARIZONAVISION
#define GSL_STUB_VECTOR_H_KJBLIB_UARIZONAVISION

#ifndef KJB_HAVE_GSL

/* This is a fake vector that is a standin for GSL vector when GSL is 
 * unavailable.  It might not be necessary, but at one point it seemed to
 * help compilation when NO_LIBS was active.  
 * The compiler apparently does a lot of inlining and it sees through most
 * transparent ruses.  So I'm basically giving it a working vector
 * implementation, although the class Gsl_Vector itself should not be able to
 * be instantiated (it should throw Missing_dependency in all ctors).
 *
 * (Later) I'm not sure this is really necessary, but if not it is no worse
 * than a harmless delusion.
 */
struct gsl_vector {
    double* p; size_t size;
    gsl_vector( size_t n ) : p( new double[ n ] ), size( n ) {}
    ~gsl_vector() { delete[] p; }
    double& at( size_t n ) { return p[n % size]; }
    const double& at( size_t n ) const { return p[n % size]; }
};
#define gsl_vector_alloc(x) (new gsl_vector(x)) /* don't actually use this! */

/*
 * It is sad but true:  because multimin foists upon us raw, unwrapped
 * gsl_vectors in its callback functions, it is simply easiest to use the
 * native access methods.  So it is also easiest to provide fake stand-ins when
 * the library is absent.  That is why the macros below are defined:  they
 * stand in when we compile without the library.
 * Although the code below looks plausible, remember IT WILL NEVER BE EXECUTED.
 * This is just elaborate scenery.
 */
#define gsl_vector_set(w,i,x) ((w)->at(i)=(x))
#define gsl_vector_get(w,i) ((w)->at(i))



/*
 * Here is similar treatment for GSL matrix, but less song and dance.
 * Here is some fakery to make client programs compile even when GSL is absent.
 */
struct gsl_matrix { size_t size1, size2; };
#define gsl_matrix_alloc(x, y) NULL


#endif
#endif /* GSL_STUB_VECTOR_H_KJBLIB_UARIZONAVISION */
