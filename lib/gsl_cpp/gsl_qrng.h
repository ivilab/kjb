/**
 * @file
 * @brief C++ wrapper on GSL quasi-random sequence generators, to prevent leaks
 * @author Andrew Predoehl
 *
 * GSL is the GNU Scientific Library.
 */

/*
 * $Id: gsl_qrng.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef GSL_QRNG_WRAP_H_KJBLIB_UARIZONAVISION
#define GSL_QRNG_WRAP_H_KJBLIB_UARIZONAVISION

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <gsl_cpp/gsl_util.h>

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_qrng.h"
#endif


namespace kjb {

#ifndef KJB_HAVE_GSL
#warning "Compiling GNU GSL wrapper without GNU GSL; it will not run properly"
typedef void gsl_qrng_type;
typedef void gsl_qrng;
gsl_qrng_type   *gsl_qrng_niederreiter_2,
                *gsl_qrng_sobol,
                *gsl_qrng_halton,
                *gsl_qrng_reversehalton;
#endif


enum {
    GSL_QRNG_NIEDER,
    GSL_QRNG_SOBOL,
    GSL_QRNG_HALTON,
    GSL_QRNG_RVSHALTON
};


/**
 * @brief Wrapper for one of GSL's quasi-random generators
 *
 * A quasi-random generator is deterministic, but it acts like a random number
 * generator that gives you unnaturally "uniform" samples in D-dimensional
 * space R(0,1)^D.
 *
 * @warning I recommend you use one of the type-specific classes instead of
 *          this basic one, such as Gsl_Qrng_Halton.
 *
 * GSL is the GNU Scientific Library.
 *
 * - No default ctor
 * - Copyable, assignable, swappable
 * - You are advised to use the specializations below instead of this class
 */
template< unsigned KIND >
class Gsl_Qrng_basic {

    unsigned m_dimensions;

    gsl_qrng *m_qrng;

public:

    /**
     * @brief ctor builds the quasi-random generator 
     *
     * @param dimensions    Number of dimensions the QRNG is asked to "sample"
     *
     * @throws KJB_error if input is bad (dimensions is zero or too big) or
     *          cannot allocate memory.
     */
    Gsl_Qrng_basic(
        unsigned dimensions,
        const gsl_qrng_type* qtype,
        unsigned maxdim
    )
#ifdef KJB_HAVE_GSL
    :   m_dimensions( dimensions ),
        m_qrng( m_dimensions ? gsl_qrng_alloc( qtype, m_dimensions ) : 00 )
    {
        ETX_2( 0 == m_dimensions, "Gsl_Qrng primitive ctor: zero dimensions" );
        ETX_2( 00 == m_qrng, "Gsl_Qrng primitive ctor:  allocation failed" );
        ETX_2( dimensions > maxdim, "Gsl_Qrng ctor:  too many dimensions" );
        gsl_qrng_init( m_qrng );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    ~Gsl_Qrng_basic()
    {
#ifdef KJB_HAVE_GSL
        gsl_qrng_free( m_qrng );
#endif
    }

    /**
     * @brief The old-fashioned way to read a quasi-random sample
     * @param [in] destination  must be an array of size at least that of the
     *              QRNG's dimensionality.
     * @throws KJB_error if something goes wrong
     */
    void read( double* destination )
    {
#ifdef KJB_HAVE_GSL
        ETX_2( 00 == destination, "Call to Gsl_Qrng::read( NULL )" );
        GSL_ETX( gsl_qrng_get( m_qrng, destination ) );
#endif
    }

    /// @brief Best way to read a single random sample of known dimensionality
    Vector read()
    {
        std::vector< double > sample( get_dimensions() );
        read( & sample[ 0 ] );  // Herb Sutter says this is legitimate
        return Vector( sample.begin(), sample.end() );
    }

    const char* name() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_qrng_name( m_qrng );
#endif
    }

    unsigned get_dimensions() const
    {
        return m_dimensions;
    }

    /// @brief copy ctor
    Gsl_Qrng_basic( const Gsl_Qrng_basic< KIND >& that )
#ifdef KJB_HAVE_GSL
    :   m_dimensions( that.get_dimensions() ),
        m_qrng( gsl_qrng_clone( that.m_qrng ) )
    {
        ETX_2( 0 == m_dimensions, "Gsl_Qrng copy ctor: zero dimensions" );
        ETX_2( 00 == m_qrng, "Gsl_Qrng copy ctor:  allocation failed" );
    }
#else
    {}
#endif

    /// @brief swap two generators
    void swap( Gsl_Qrng_basic< KIND >& that )
    {
#ifdef KJB_HAVE_GSL
        using std::swap;

        swap( m_dimensions, that.m_dimensions );
        swap( m_qrng, that.m_qrng );
#endif
    }

    /// @brief assignment operator
    Gsl_Qrng_basic< KIND >& operator=( const Gsl_Qrng_basic< KIND >& that )
    {
#ifdef KJB_HAVE_GSL
        if ( this != &that )
        {
            GSL_ETX( gsl_qrng_memcpy( m_qrng, that.m_qrng ) );
        }
#endif
        return *this;
    }

};


/**
 * @brief Quasi-random generator using the algorithm of Bratley et al.
 * 
 * This algorithm supports up to 12 dimensions.  The citation is found here:
 * http://www.gnu.org/software/gsl/manual/html_node/Quasi_002drandom-number-generator-algorithms.html
 *
 * I recommend you use this or a similar derived class instead of using the
 * Gsl_Qrng_basic base class.
 */
class Gsl_Qrng_Niederreiter : public Gsl_Qrng_basic< GSL_QRNG_NIEDER > {
public:
    /// @brief ctor for Niederreiter QRNG of given number of dimensions dims
    Gsl_Qrng_Niederreiter( unsigned dims )
    :   Gsl_Qrng_basic< GSL_QRNG_NIEDER >( dims, gsl_qrng_niederreiter_2, 12 )
    {}
};

/**
 * @brief Quasi-random generator using the algorithm of Antonov and Saleev.
 * 
 * This algorithm supports up to 40 dimensions.  The citation is found here:
 * http://www.gnu.org/software/gsl/manual/html_node/Quasi_002drandom-number-generator-algorithms.html
 *
 * I recommend you use this or a similar derived class instead of using the
 * Gsl_Qrng_basic base class.
 */
class Gsl_Qrng_Sobol : public Gsl_Qrng_basic< GSL_QRNG_SOBOL > {
public:
    /// @brief ctor for Sobol QRNG of given number of dimensions
    Gsl_Qrng_Sobol( unsigned dimensions )
    :   Gsl_Qrng_basic< GSL_QRNG_SOBOL >( dimensions, gsl_qrng_sobol, 40 )
    {}
};

/**
 * @brief Quasi-random generator using the algorithm of Halton
 * 
 * This algorithm supports up to 1229 dimensions.  The citation is found here:
 * http://www.gnu.org/software/gsl/manual/html_node/Quasi_002drandom-number-generator-algorithms.html
 *
 * I recommend you use this or a similar derived class instead of using the
 * Gsl_Qrng_basic base class.
 */
class Gsl_Qrng_Halton : public Gsl_Qrng_basic< GSL_QRNG_HALTON > {
public:
    /// @brief ctor for Halton QRNG of given number of dimensions
    Gsl_Qrng_Halton( unsigned dimensions )
    :   Gsl_Qrng_basic< GSL_QRNG_HALTON >( dimensions, gsl_qrng_halton, 1229 )
    {}
};

/**
 * @brief Quasi-random generator using the algorithm of Vandewoestyne et al.
 * 
 * This algorithm is also known as the "reverse Halton."
 *
 * This algorithm supports up to 1229 dimensions.  The citation is found here:
 * http://www.gnu.org/software/gsl/manual/html_node/Quasi_002drandom-number-generator-algorithms.html
 *
 * I recommend you use this or a similar derived class instead of using the
 * Gsl_Qrng_basic base class.
 */
class Gsl_Qrng_Rvs_Halton : public Gsl_Qrng_basic< GSL_QRNG_RVSHALTON > {
public:
    /// @brief ctor for Reverse Halton QRNG of dimensionality d
    Gsl_Qrng_Rvs_Halton( unsigned d )
    :   Gsl_Qrng_basic< GSL_QRNG_RVSHALTON >( d, gsl_qrng_reversehalton, 1229 )
    {}
};


} // namespace kjb

namespace std {

    /// @brief Swap two wrapped qrng objects.
    template< unsigned KIND >
    inline void swap(
        kjb::Gsl_Qrng_basic< KIND >& m1,
        kjb::Gsl_Qrng_basic< KIND >& m2
    )
    {
        m1.swap( m2 );
    }
}


#endif /* GSL_QRNG_WRAP_H_KJBLIB_UARIZONAVISION */
