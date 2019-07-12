/* $Id: gsl_multifit.h 20312 2016-02-02 05:20:31Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef KJB_CPP_GSL_MULTIFIT_H
#define KJB_CPP_GSL_MULTIFIT_H

/**
 * @file
 *
 * @brief Wrappers for GSL's nonlinear least-squares library.
 * 
 * If your error function takes the form of a squared distance metric,
 * you're probably better using this than multimin, which is for minimization
 * of general functions.
 *
 * @author Kyle Simek
 * @author Andrew Predoehl
 */

#include <l_cpp/l_exception.h>
#include <gsl_cpp/gsl_util.h>
#include <gsl_cpp/gsl_vector.h>
#include <gsl_cpp/gsl_matrix.h>
#include <boost/function/function1.hpp>

#ifdef KJB_HAVE_GSL
#include <gsl/gsl_multifit_nlin.h>
#else
#warning "Gnu Scientific Library is absent, yet essential to this program."

/*
 * Here is some fakery to make client programs compile even when GSL is absent.
 */
typedef void gsl_multifit_fdfsolver_type;
typedef void gsl_multifit_fdfsolver;
#define gsl_multifit_fdfsolver_lmsder ((void*)0)
#define gsl_multifit_fdfsolver_lmder ((void*)0)
#define gsl_multifit_fdfsolver_alloc(x,z) 0
#define gsl_set_error_handler_off() (0)
struct gsl_multifit_function_fdf {
    double (*f)( const gsl_vector*, void* , gsl_vector * );
    void (*df)( const gsl_vector*, void*, gsl_matrix* );
    void (*fdf)( const gsl_vector*, void*, gsl_vector*, gsl_matrix* );
    int n;
    size_t p;
    void *params;
};

typedef void gsl_multifit_fsolver_type;
typedef void gsl_multifit_fsolver;
// #define gsl_multifit_fsolver_nmsimplex2 ((void*)0)
#define gsl_multifit_fsolver_alloc(x,z) 0
//struct gsl_multifit_function {
//    size_t n; void *params;
//    double (*f)( const gsl_vector*, void* );
//};

#endif


namespace kjb
{

/**
 * @brief wrapper for GSL-1.x nonlinear least squares derivative solver.
 * 
 * BROKEN.  FIXME:  works on version 1.6 but not version 2.x.
 * Jacobian field J was removed from the API.  New gradient field g is similar.
 * 2016-02-01 Predoehl.
 */
#define HOW_IT_WAS_JANUARY_2016 0
#if HOW_IT_WAS_JANUARY_2016 
class Gsl_multifit_fdf
{

#ifdef KJB_HAVE_GSL
    /**
     * See Gsl_Multimin_fdf for rationale for this class. Basically, it avoids
     * memory leaks.
     * @sa Gsl_Multimin_fdf::basic_Gsl_Multimin_fdf
     */
    struct basic_Gsl_multifit_fdf {
        gsl_multifit_fdfsolver* p;
        size_t dim;
        size_t num_obs;

        basic_Gsl_multifit_fdf( 
            const gsl_multifit_fdfsolver_type* type,
            size_t n,
            size_t p
        )
        :   p( (n>0 && p>0) ? gsl_multifit_fdfsolver_alloc(type, n, p) : 00 ),
            dim( p ),
            num_obs( n )
        {
            ETX_2( 0 == n, "basic_Gsl_multifit_fdf(): zero observations" );
            ETX_2( 0 == p, "basic_Gsl_multifit_fdf(): Zero dimensions" );
            ETX_2( 00 == p, "basic_Gsl_multifit_fdf(): Memory alloc failed" );
        }

        ~basic_Gsl_multifit_fdf()
        {
            gsl_multifit_fdfsolver_free( p );
        }

        void swap( basic_Gsl_multifit_fdf& that )
        {
            using std::swap;
            swap( p, that.p );
        }

    private:
        // this class is not copyable, nor assignable, nor has a default ctor.
        basic_Gsl_multifit_fdf( const basic_Gsl_multifit_fdf& );
        basic_Gsl_multifit_fdf();
        basic_Gsl_multifit_fdf& operator=( const basic_Gsl_multifit_fdf& );
    };

    basic_Gsl_multifit_fdf m_fit;
#endif

    bool m_verbose;

    boost::function1<double, const gsl_vector*> boost_f_;
    bool m_initialized;

    mutable Gsl_Vector* m_gradient;

public:
    /// TODO: document
    Gsl_multifit_fdf(
        const gsl_multifit_fdfsolver_type* type,
        gsl_multifit_function_fdf* f,
        const gsl_vector* x0,
        size_t data_count,
        bool verbosity = true
    )
#ifdef KJB_HAVE_GSL
    :   m_fit( type, data_count, x0 ? x0 -> size : 0 ),
        m_verbose( verbosity ),
        m_initialized(false),
        m_gradient(NULL)
    {
        if(x0->size != f->p)
        {
            KJB_THROW(Dimension_mismatch);
        }

        initialize(f, x0);
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    Gsl_multifit_fdf(
            size_t data_count,
            size_t size,
            const gsl_multifit_fdfsolver_type* type = gsl_multifit_fdfsolver_lmsder)
#ifdef KJB_HAVE_GSL
    :   m_fit( type, data_count, size),
        m_verbose( false ),
        m_initialized(false),
        m_gradient(NULL)
    {}
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    ~Gsl_multifit_fdf()
    {
        delete m_gradient;
    }

    void initialize(
        gsl_multifit_function_fdf* f,
        const gsl_vector* x0
    )
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_multifit_fdfsolver_set( m_fit.p, f, x0 ) );
        m_initialized = true;
#endif
    }

    size_t dim() const
    {
#ifdef KJB_HAVE_GSL
        return m_fit.dim;
#endif
    }

    size_t num_observations() const
    {
#ifdef KJB_HAVE_GSL
        return m_fit.num_obs;
#endif
    }

    /**
     * @brief Perform one iteration of the solver.
     * @returns GSL_SUCCESS if successful, or a GSL error code otherwise:
     *        http://www.gnu.org/software/gsl/manual/html_node/Error-Codes.html
     *
     * If you want to throw an exception in the case of a failed iteration,
     * consider using the GSL_ETX macro.
     */
    int iterate()
    {
#ifdef KJB_HAVE_GSL
        if(!m_initialized)
        {
            KJB_THROW_2(Runtime_error, "Solver not initialized");
        }

        int rc = gsl_multifit_fdfsolver_iterate( m_fit.p );
        if (m_verbose) gsl_iterate_EPE(rc);
        return rc;
#endif
    }

    /// @brief query the current best argfit of the solver
    gsl_vector* position() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multifit_fdfsolver_position( m_fit.p );
#endif
    }

    gsl_matrix* jacobian() const
    {
#ifdef KJB_HAVE_GSL
        return m_fit.p->J;
#endif
    }


    /// @brief query the current best min value of the function to be minimized
    const gsl_vector* min() const
    {
#ifdef KJB_HAVE_GSL
        return m_fit.p->f;
#endif
    }

    /**
     * @return GSL_SUCCESS if the size is smaller than tolerance epsabs, or
     *          GSL_CONTINUE otherwise.
     */
    int test_delta( double epsabs, double epsrel) const
    {
#ifdef KJB_HAVE_GSL
        
        return gsl_multifit_test_delta( m_fit.p->dx, m_fit.p->x, epsabs, epsrel );
#endif
    }

    /**
     * @return GSL_SUCCESS if the size is smaller than tolerance epsabs, or
     *          GSL_CONTINUE otherwise.
     */
    int test_gradient( double epsabs = 1e-6) const
    {
#ifdef KJB_HAVE_GSL
        if(m_gradient == NULL)
            m_gradient = new Gsl_Vector(dim());

        gsl_multifit_gradient(m_fit.p->J, m_fit.p->f, *m_gradient);
        return gsl_multifit_test_gradient(*m_gradient, epsabs);
#endif
    }

    /**
     * @brief Swap the contents of this solver and another (fast).
     *
     * This method is useful if, for example, you want to restart the solver 
     * at a new location; just instantiate a new solver object at the new
     * location and swap() it with the old solver object.
     */
    void swap( Gsl_multifit_fdf& that )
    {
#ifdef KJB_HAVE_GSL
        if ( this == &that )
        {
            return;
        }
        m_fit.swap( that.m_fit );

        using std::swap;
        swap( m_verbose, that.m_verbose );
        swap( m_gradient, that.m_gradient );
#endif
    }

    /**
     * @brief Alter the verbosity attribute of the object.
     * @returns the previous value of the verbosity attribute.
     * Currently if the object has true-verbosity then you will get an error
     * message if iterate() fails.
     */
    bool verbosity( bool v )
    {
        bool oldv = m_verbose;
        m_verbose = v;
        return oldv;
    }

    /**
     * @returns the value of the verbosity attribute.
     */
    bool verbosity( )
    {
        return m_verbose;
    }
};
#endif /* HOW_IT_WAS_JANUARY_2016 */

} // namespace kjb

#endif
