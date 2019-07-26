/**
 * @file
 * @brief C++ wrapper on GSL some multimin features, to prevent resource leaks
 * @author Andrew Predoehl
 *
 * GSL is the GNU Scientific Library.
 */

/*
 * $Id: gsl_multimin.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef GSL_MULTIMIN_WRAP_H_KJBLIB_UARIZONAVISION
#define GSL_MULTIMIN_WRAP_H_KJBLIB_UARIZONAVISION

#include <l_cpp/l_exception.h>
#include <gsl_cpp/gsl_util.h>
#include <gsl_cpp/gsl_vector.h>

#include <boost/function/function1.hpp>

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_multimin.h" /* no need for extern "C" */
#else
#warning "Gnu Scientific Library is absent, yet essential to this program."

/*
 * Here is some fakery to make client programs compile even when GSL is absent.
 */
typedef void gsl_multimin_fdfminimizer_type;
typedef void gsl_multimin_fdfminimizer;
#define gsl_multimin_fdfminimizer_conjugate_pr ((void*)0)
#define gsl_multimin_fdfminimizer_vector_bfgs2 ((void*)0)
#define gsl_multimin_fdfminimizer_alloc(x,z) 0
#define gsl_set_error_handler_off() (0)
struct gsl_multimin_function_fdf {
    int n; void *params;
    double (*f)( const gsl_vector*, void* );
    void (*df)( const gsl_vector*, void*, gsl_vector* );
    void (*fdf)( const gsl_vector*, void*, double*, gsl_vector* );
};

typedef void gsl_multimin_fminimizer_type;
typedef void gsl_multimin_fminimizer;
#define gsl_multimin_fminimizer_nmsimplex ((void*)0)
#define gsl_multimin_fminimizer_nmsimplex2 ((void*)0)
#define gsl_multimin_fminimizer_alloc(x,z) 0
struct gsl_multimin_function {
    size_t n; void *params;
    double (*f)( const gsl_vector*, void* );
};
#define GSL_NAN (1.0/0.0)

#endif



namespace kjb {



/**
 * @brief Wrapper for GSL's multidimensional minimizer, when you have gradient
 *
 * GSL is the GNU Scientific Library.
 *
 * This object is not copyable or assignable, but it does have a swap() method.
 */
class Gsl_Multimin_fdf {

#ifdef KJB_HAVE_GSL
    /**
     * I split up the GSL multimin wrapper into two objects, namely
     * basic_Gsl_Multimin_fdf (henceforth "basic_") and
     * Gsl_Multimin_fdf (henceforth "fancy")
     * so that alloc / free functions were paired naturally in the basic_
     * version and the fancy version performs the standard initialization; but
     * if the fancy initialization fails, then I can just throw; the basic_
     * object by that time is already constructed and thus its dtor will fire.
     * No leakage is possible.
     *
     * If I had done it all in one, then a failure in fancy initialization
     * would oblige me to call the free() function before
     * throwing, which makes the code harder to maintain.
     */
    struct basic_Gsl_Multimin_fdf {
        gsl_multimin_fdfminimizer* p;

        basic_Gsl_Multimin_fdf( 
            const gsl_multimin_fdfminimizer_type* type,
            size_t dim
        )
        :   p( dim ? gsl_multimin_fdfminimizer_alloc( type, dim ) : 00 )
        {
            ETX_2( 0 == dim, "basic_Gsl_Multimin_fdf(): Zero dimensions" );
            ETX_2( 00 == p, "basic_Gsl_Multimin_fdf(): Memory alloc failed" );
        }

        ~basic_Gsl_Multimin_fdf()
        {
            gsl_multimin_fdfminimizer_free( p );
        }

        void swap( basic_Gsl_Multimin_fdf& that )
        {
            using std::swap;
            swap( p, that.p );
        }

    private:
        // this class is not copyable, nor assignable, nor has a default ctor.
        basic_Gsl_Multimin_fdf( const basic_Gsl_Multimin_fdf& );
        basic_Gsl_Multimin_fdf();
        basic_Gsl_Multimin_fdf& operator=( const basic_Gsl_Multimin_fdf& );
    };

    basic_Gsl_Multimin_fdf m_min;
#endif

    bool m_verbose;

public:

    /**
     * @brief ctor builds the minimizer by allocating and setting up params
     *
     * @param type  indicates the minimization algorithm you would like to use;
     *              supported choices are listed at the link below:
     *              http://www.gnu.org/software/gsl/manual/html_node/Multimin-Algorithms-with-Derivatives.html
     *
     * @param x0    Initial vector location in domain from which search begins.
     *
     * @param step_size please see the GSL documentation, or use clarivoyance,
     *                  to determine what this should be set to; apparently it
     *                  controls the size of (only) the first step.
     *
     * @param tol       please see the GSL documentation for this one too.
     *
     * @param verbosity If true, then a bad iterate() call will emit a message
     *                  to stderr.
     *
     * @throws KJB_error if the input is bad:  e.g., x0 equal to NULL or
     *          cannot allocate memory.
     */
    Gsl_Multimin_fdf(
        const gsl_multimin_fdfminimizer_type* type,
        gsl_multimin_function_fdf* fdf,
        const gsl_vector* x0,
        double step_size,
        double tol,
        bool verbosity = true
    )
#ifdef KJB_HAVE_GSL
    :   m_min( type, x0 ? x0 -> size : 0 ),
        m_verbose( verbosity )
    {
        GSL_ETX( gsl_multimin_fdfminimizer_set( m_min.p, fdf, x0, step_size,
                                                                    tol ) );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    /**
     * @brief Perform one iteration of the minimizer.
     * @returns GSL_SUCCESS if successful, or a GSL error code otherwise:
     *        http://www.gnu.org/software/gsl/manual/html_node/Error-Codes.html
     *
     * If you want to throw an exception in the case of a failed iteration,
     * consider using the GSL_ETX macro.
     */
    int iterate()
    {
#ifdef KJB_HAVE_GSL
        int rc = gsl_multimin_fdfminimizer_iterate( m_min.p );
        if (m_verbose) gsl_iterate_EPE(rc);
        return rc;
#endif
    }

    /// @brief query the current best argmin of the minimizer
    gsl_vector* argmin() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fdfminimizer_x( m_min.p );
#endif
    }

    /// @brief query the current best min value of the function to be minimized
    double min() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fdfminimizer_minimum( m_min.p );
#endif
    }

    /// @brief query the gradient of the function at the current location
    gsl_vector* gradient() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fdfminimizer_gradient( m_min.p  );
#endif
    }

    /// @brief access a string describing the algorithm
    const char* name() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fdfminimizer_name( m_min.p );
#endif
    }

    /**
     * @brief check the magnitude the gradient of the function now
     * @return GSL_SUCCESS if gradient magnitude is less than epsabs, or
     *          GSL_CONTINUE otherwise.
     */
    int test_gradient( double epsabs ) const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_test_gradient( m_min.p -> gradient, epsabs );
#endif
    }

    /**
     * @brief Restart the minimizer at the current argmin value.
     * @throws KJB_error if an error occurs (not sure what that would be)
     *
     * Bad news:
     * If you want to restart the whole thing at a new point in the domain, I'm
     * afraid you must instantiate a new object.  If you alter argmin() (which
     * you CAN do) then subsequent calls to iterate() will tell you that you
     * are not making progress towards the solution.
     *
     * Hint: the swap() method helps you resume work after instantiating a new
     * object for the above purpose.
     */
    void restart()
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_multimin_fdfminimizer_restart( m_min.p ) );
#endif
    }

    /**
     * @brief Swap the contents of this minimizer and another (fast).
     *
     * This method is useful if, for example, you want to restart the minimizer
     * at a new location; just instantiate a new minimizer object at the new
     * location and swap() it with the old minimizer object.
     */
    void swap( Gsl_Multimin_fdf& that )
    {
#ifdef KJB_HAVE_GSL
        if ( this == &that )
        {
            return;
        }
        m_min.swap( that.m_min );
        using std::swap;
        swap( m_verbose, that.m_verbose );
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
};


/**
 * @brief Wrapper for GSL's multidimensional minimizer, without using gradient
 *
 * GSL is the GNU Scientific Library.
 *
 * This object is not copyable or assignable, but it does have a swap() method.
 */
class Gsl_Multimin_f {

#ifdef KJB_HAVE_GSL
    /**
     * I split up the GSL multimin wrapper into two objects, namely
     * basic_Gsl_Multimin_f (henceforth "basic_") and
     * Gsl_Multimin_f (henceforth "fancy")
     * so that alloc / free functions were paired naturally in the basic_
     * version and the fancy version performs the standard initialization; but
     * if the fancy initialization fails, then I can just throw; the basic_
     * object by that time is already constructed and thus its dtor will fire.
     * No leakage is possible.
     *
     * If I had done it all in one, then a failure in fancy initialization
     * would oblige me to call the free() function before
     * throwing, which makes the code harder to maintain.
     */
    struct basic_Gsl_Multimin_f {
        gsl_multimin_fminimizer* p;
        size_t dim;

        basic_Gsl_Multimin_f( 
            const gsl_multimin_fminimizer_type* type,
            size_t dimension
        )
        :   p( dimension ? gsl_multimin_fminimizer_alloc( type, dimension ) : 00 ),
            dim( dimension )
        {
            ETX_2( 0 == dimension, "basic_Gsl_Multimin_f(): Zero dimensions" );
            ETX_2( 00 == p, "basic_Gsl_Multimin_f(): Memory alloc failed" );
        }

        ~basic_Gsl_Multimin_f()
        {
            gsl_multimin_fminimizer_free( p );
        }

        void swap( basic_Gsl_Multimin_f& that )
        {
            using std::swap;
            swap( p, that.p );
        }

    private:
        // this class is not copyable, nor assignable, nor has a default ctor.
        basic_Gsl_Multimin_f( const basic_Gsl_Multimin_f& );
        basic_Gsl_Multimin_f& operator=( const basic_Gsl_Multimin_f& );
    };

    basic_Gsl_Multimin_f m_min;
#endif

    bool m_verbose;

    boost::function1<double, const gsl_vector*> boost_f_;
    gsl_multimin_function gsl_f_;
    bool m_initialized;

public:

    /**
     * @brief ctor builds the minimizer by allocating and setting up params
     *
     * @param type  indicates the minimization algorithm you would like to use;
     *              supported choices are listed at the link below:
     *              http://www.gnu.org/software/gsl/manual/html_node/Multimin-Algorithms-with-Derivatives.html
     *
     * @param x0    Initial vector location in domain from which search begins.
     *
     * @param step_size please see the GSL documentation, or use clarivoyance,
     *                  to determine what this should be set to; apparently it
     *                  controls the size of (only) the first step.
     *
     * @param tol       please see the GSL documentation for this one too.
     *
     * @param verbosity If true, then a bad iterate() call will emit a message
     *                  to stderr.
     *
     * @throws KJB_error if the input is bad:  e.g., x0 equal to NULL or
     *          cannot allocate memory.
     */
    Gsl_Multimin_f(
        const gsl_multimin_fminimizer_type* type,
        gsl_multimin_function* f,
        const gsl_vector* x0,
        const gsl_vector* step_size,
        bool verbosity = true
    )
#ifdef KJB_HAVE_GSL
    :   m_min( type, x0 ? x0 -> size : 0 ),
        m_verbose( verbosity ),
        m_initialized(false)
    {
        initialize(f, x0, step_size);
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    Gsl_Multimin_f(
            size_t size,
            const gsl_multimin_fminimizer_type* type = gsl_multimin_fminimizer_nmsimplex2)
#ifdef KJB_HAVE_GSL
    :   m_min( type, size),
        m_verbose( false ),
        m_initialized(false)
    {}
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    void initialize(
        gsl_multimin_function* f,
        const gsl_vector* x0,
        const gsl_vector* step_size)
    {
#ifdef KJB_HAVE_GSL
        if(x0->size != step_size->size)
            KJB_THROW(kjb::Dimension_mismatch);
        GSL_ETX( gsl_multimin_fminimizer_set( m_min.p, f, x0, step_size ) );
        m_initialized = true;
#endif
    }

    size_t dim() const
    {
#ifdef KJB_HAVE_GSL
        return m_min.dim;
#endif
    }

    /**
     * @brief Perform one iteration of the minimizer.
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
            KJB_THROW_2(Runtime_error, "Minimizer not initialized");
        }

        int rc = gsl_multimin_fminimizer_iterate( m_min.p );
        if (m_verbose) gsl_iterate_EPE(rc);
        return rc;
#endif
    }

    /// @brief query the current best argmin of the minimizer
    gsl_vector* argmin() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fminimizer_x( m_min.p );
#endif
    }

    /// @brief query the current best min value of the function to be minimized
    virtual double min() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fminimizer_minimum( m_min.p );
#endif
    }

    /// @brief access a string describing the algorithm
    const char* name() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_multimin_fminimizer_name( m_min.p );
#endif
    }

    /**
     * @brief check the magnitude the minimizer specific characteristic size 
     * against the tolerance epsabs.
     *
     * @return GSL_SUCCESS if the size is smaller than tolerance epsabs, or
     *          GSL_CONTINUE otherwise.
     */
    int test_size( double epsabs ) const
    {
#ifdef KJB_HAVE_GSL
        
        double size = gsl_multimin_fminimizer_size(m_min.p);
        return gsl_multimin_test_size( size, epsabs );
#endif
    }


    /**
     * @brief Swap the contents of this minimizer and another (fast).
     *
     * This method is useful if, for example, you want to restart the minimizer
     * at a new location; just instantiate a new minimizer object at the new
     * location and swap() it with the old minimizer object.
     */
    void swap( Gsl_Multimin_f& that )
    {
#ifdef KJB_HAVE_GSL
        if ( this == &that )
        {
            return;
        }
        m_min.swap( that.m_min );
        using std::swap;
        swap( m_verbose, that.m_verbose );
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
};



inline
double gsl_evaluate_Nd_boost_function(const gsl_vector* v, void* params)
{
    typedef boost::function1<double, const gsl_vector*> Func_type;
    Func_type* f = static_cast<Func_type*>(params);

    return (*f)(v);
}

template<class T>
class Generic_multimin : public Gsl_Multimin_f
{
public:
    typedef boost::function1<double, T> Evaluator;

    // @note this function can assume that T is initialized to the value from the previous optimization iteration
    typedef boost::function2<void, const gsl_vector*, T&> From_gsl;

    // @note this function can assume gsl_vector* is initialized to correct dimension
    typedef boost::function2<void, const T&, gsl_vector*> To_gsl;

private:

    typedef Gsl_Multimin_f Base;

    /**
     * Helper functor to convert a function of signature
     *      double f(const T&)
     * to a function of signature
     *      double f(const gsl_vector*)
     * 
     * This allows it to be passed to _another_ helper function,
     * gsl_evaluate_Nd_boost_function
     */
    template <class Eval_type>
    struct gsl_adapter_func
    {
        gsl_adapter_func(
                const Eval_type& e,
                From_gsl& from_gsl,
                bool negate = false) :
            f_(),
            from_gsl_(from_gsl),
            workspace_(),
            negate_(negate)
        {
            f_ = boost::ref(e);
        }

        /**
         * Construct with explicit initial model.
         * Useful when minimizer operates on a subset of 
         * parameters, and the other parameters remain constant;
         * the constant parameters are initialized
         * from the model passed-in here.
         */
        gsl_adapter_func(
                const Eval_type& e,
                From_gsl& from_gsl,
                const T& initial,
                bool negate = false) :
            f_(),
            from_gsl_(from_gsl),
            workspace_(initial),
            negate_(negate)
        { 
            f_ = boost::ref(e);
        }

        double operator()(const gsl_vector* x) const
        {
            from_gsl_(x, workspace_);
            if(negate_)
                return -f_(workspace_);
            else
                return f_(workspace_);
        }

        Evaluator f_;
        From_gsl& from_gsl_;
        mutable T workspace_;
        bool negate_;
    };
public:
    Generic_multimin(
            size_t size,
            const To_gsl& to_gsl,
            const From_gsl& from_gsl,
            const gsl_multimin_fminimizer_type* type = gsl_multimin_fminimizer_nmsimplex2) : 
        Base(size, type),
        boost_f_(),
        gsl_f_(),
        to_gsl_(to_gsl),
        from_gsl_(from_gsl),
        negate_evaluator_(false)
    {}

    template <class Eval_type, class VectorType>
    void initialize(
            const Eval_type& e,
            const T& guess,
            const VectorType& step_size,
            bool negate_evaluator = false)
    {
        negate_evaluator_ = negate_evaluator;
#ifdef KJB_HAVE_GSL
        size_t num_dimensions = dim();
        // WARNING: possibly guess and/or step_size need to exist throughout the life of the minimizer.  If this immediately segfaults, this is probably why.

        if(!reference_)
        {
            // save the initial guess in case immutable parameters
            // need to be restored at the end
            boost::shared_ptr<T> tmp(new T(guess));
            reference_ = tmp;
        }

        boost_f_ = gsl_adapter_func<Eval_type>(e, from_gsl_, *reference_, negate_evaluator);
        gsl_f_.f = gsl_evaluate_Nd_boost_function;
        gsl_f_.n = num_dimensions;
        gsl_f_.params = static_cast<void*>(&boost_f_);

        // TODO: find a way to avoid an extra allocation here.
        Gsl_Vector gsl_guess(num_dimensions);
        to_gsl_(guess, gsl_guess);

        Gsl_Vector gsl_step_size(step_size.begin(), step_size.end());

        Base::initialize(&gsl_f_, gsl_guess, gsl_step_size);
#else
        KJB_THROW_2(Missing_dependency, "gsl");
#endif
    }

    const T& get() const
    {
        if(!reference_)
            KJB_THROW_2(Runtime_error, "Not initialized");
        from_gsl_(argmin(), *reference_);
        return *reference_;
    }

    /// @brief query the current best min value of the function to be minimized
    double min() const
    {
#ifdef KJB_HAVE_GSL

        double result = Base::min();
        return (negate_evaluator_ ? -result : result);

#endif
    }


private:
    boost::function1<double, const gsl_vector*> boost_f_;
    gsl_multimin_function gsl_f_;
    To_gsl to_gsl_;
    From_gsl from_gsl_;
    bool negate_evaluator_;
    mutable boost::shared_ptr<T> reference_;
};


} // namespace kjb

namespace std {

    /// @brief Swap two wrapped multimin objects.
    template<>
    inline void swap( kjb::Gsl_Multimin_fdf& m1, kjb::Gsl_Multimin_fdf& m2 )
    {
        m1.swap( m2 );
    }

    /// @brief Swap two wrapped multimin objects.
    template<>
    inline void swap( kjb::Gsl_Multimin_f& m1, kjb::Gsl_Multimin_f& m2 )
    {
        m1.swap( m2 );
    }
}


#endif /* GSL_MULTIMIN_WRAP_H_KJBLIB_UARIZONAVISION */
