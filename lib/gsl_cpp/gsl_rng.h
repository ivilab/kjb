/**
 * @file
 * @brief Class defs for C++ wrapper on GNU GSL random number generators
 * @author Andrew Predoehl
 */
/*
 * $Id: gsl_rng.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef GSL_RNG_H_INCLUDED_LIBKJB_UOFARIZONAVISION
#define GSL_RNG_H_INCLUDED_LIBKJB_UOFARIZONAVISION

#include <l_cpp/l_exception.h>
#include <gsl_cpp/gsl_util.h>
#include <vector>
#include <string>

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_rng.h"    /* this is a GSL header, not our wrapper */
#else
#warning "Compiling GNU GSL wrapper without GNU GSL; it will not run properly"
typedef void gsl_rng;
typedef void gsl_rng_type;
gsl_rng_type    *gsl_rng_mt19937,
                *gsl_rng_ranlxs0,
                *gsl_rng_ranlxs1,
                *gsl_rng_ranlxs2,
                *gsl_rng_ranlxd1,
                *gsl_rng_ranlxd2,
                *gsl_rng_cmrg,
                *gsl_rng_mrg,
                *gsl_rng_taus2,
                *gsl_rng_gfsr4;
#endif


namespace kjb {


/**
 * @brief constants used to select random number generators
 *
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 *
 * GSL supports many other legacy algorithms that I have not bothered to add.
 */
enum {
    GSL_RNG_MT19937,        ///< Matsumoto's & Nishimura's "Mersenne Twister"
    GSL_RNG_RANLXS0,        ///< RANLUX single-precision level 0
    GSL_RNG_RANLXS1,        ///< RANLUX single-precision level 1
    GSL_RNG_RANLXS2,        ///< RANLUX single-precision level 2
    GSL_RNG_RANLXD1,        ///< RANLUX double-precision level 1
    GSL_RNG_RANLXD2,        ///< RANLUX double-precision level 2
    GSL_RNG_CMRG,           ///< L'Ecuyer's '96 generator, period ~ 10^56
    GSL_RNG_MRG,            ///< L'Ecuyer et al. '93 generator, period ~ 10^46
    GSL_RNG_TAUS2,          ///< Tausworthe generator version 2
    GSL_RNG_GFSR4,          ///< Four-tap-XOR shift generator
};


#ifdef KJB_HAVE_GSL
std::string gsl_rng_serialize_implementation( const gsl_rng* );

void gsl_rng_deserialize_implementation( gsl_rng*, const std::string& );
#endif


/**
 * @brief Basic RAII wrapper for GNU GSL random number generators
 *
 * @warning I recommend you not use this basic class directly; instead use the
 *          specializations below, defined via Gsl_rng_template.
 *
 * Most of the gsl functions having names with prefix "gsl_rng_" are wrapped
 * below.  The methods that are NOT wrapped are the following:
 * - gsl_rng_state (consider the serialize() method instead)
 * - gsl_rng_size
 * - gsl_rng_types_setup
 * - gsl_rng_env_setup
 *
 * The functiosn gsl_rng_fread() and gsl_rng_fwrite() are wrapped up inside
 * the methods serialize() and deserialize().  You should use that interface
 * since the methods also perform the type checking that fread requires.
 */
template< unsigned KIND >
class Gsl_rng_basic {

    gsl_rng *m_rng;

    // I assume there will be fewer than, say, 100 kinds of RNG.
    static char kind()
    {
        return char( '0' + KIND );
    }

public:

    /// @brief ctor defines type -- not meant to be called directly by users.
    Gsl_rng_basic( const gsl_rng_type* type )
#ifdef KJB_HAVE_GSL
    :   m_rng( gsl_rng_alloc( type ) )
    {
        ETX_2( 00 == m_rng, "Gsl_rng_basic ctor: bad_alloc" );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    /// @brief copy ctor of an RNG of the same kind
    Gsl_rng_basic< KIND >( const Gsl_rng_basic< KIND >& rng )
#ifdef KJB_HAVE_GSL
    :   m_rng( gsl_rng_clone( rng.m_rng ) )
    {
        ETX_2( 00 == m_rng, "Gsl_rng_basic copy ctor: bad_alloc" );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    /// @brief assignment operator from an RNG of the same kind
    Gsl_rng_basic< KIND >& operator=( const Gsl_rng_basic< KIND >& that )
    {
        if ( this != &that )
        {
#ifdef KJB_HAVE_GSL
            gsl_rng_memcpy( m_rng, that.m_rng );
#endif
        }
        return *this;
    }

    /// @brief seed the generator to determine its future values
    void seed( unsigned long seed_val ) const
    {
#ifdef KJB_HAVE_GSL
        gsl_rng_set( m_rng, seed_val );
#endif
    }

    /// @brief take a snapshot of the generator state right now
    std::string serialize() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_serialize_implementation( m_rng ) + kind();
#endif
    }

    /**
     * @brief return generator to the state "snapshot" from serialize().
     * @param state Output from method serialize() from a RNG of the same type
     *              as this one.
     * @throws KJB_error corresponding to GSL_EFAILED if input is from an
     *         invalid string.
     */
    void deserialize( const std::string& state ) const
    {
#ifdef KJB_HAVE_GSL
        if ( state.size() < 2 || state[ state.size() - 1 ] != kind() )
        {
            GSL_ETX( GSL_EFAILED );
        }

        gsl_rng_deserialize_implementation( m_rng,
                            std::string( state.begin(), state.end() - 1 ) );
#endif
    }

    /**
     * @brief Sample uniformly distributed integers over a large range
     * @see max() for the maximum possible value -- a value that IS possible.
     * @see min() for the minimum possible value -- a value that IS possible.
     */
    unsigned long get() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_get( m_rng );
#endif
    }

    /// @brief return C-style string of the name of the generator algorithm.
    const char* name() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_name( m_rng );
#endif
    }

    /// @brief maximum possible value that get() can potentially return.
    unsigned long max() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_max( m_rng );
#endif
    }

    /// @brief minimum possible value that get() can potentially return.
    unsigned long min() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_min( m_rng );
#endif
    }

    /// @brief Sample uniformly distributed float in interval [0,1)
    double uniform() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_uniform( m_rng );
#endif
    }

    /// @brief Sample uniformly distributed positive float in interval (0,1)
    double uniform_pos() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_rng_uniform_pos( m_rng );
#endif
    }

    /**
     * @brief Sample uniformly dist. non-neg. integers less than end_value
     * @warning end_value must not be too big:  see GSL documentation
     * @throws KJB_error corresponding to GSL_EINVAL if end_value is invalid.
     */
    unsigned long uniform_int( unsigned long end_value ) const
    {
#ifdef KJB_HAVE_GSL
        unsigned long dev = gsl_rng_uniform_int( m_rng, end_value );

        if ( 0 == dev && end_value > max() )
        {
            GSL_ETX( GSL_EINVAL );
        }

        return dev;
#endif
    }

    /// @brief return raw pointer to generator -- needed by other GSL methods
    operator const gsl_rng*() const
    {
        return m_rng;
    }

#ifdef KJB_HAVE_GSL
    /// @brief dtor releases memory allocated by generator
    ~Gsl_rng_basic()
    {
        gsl_rng_free( m_rng );
    }
#endif

};


/// @brief this macro is used to define random number generator classes
#define Gsl_rng_template( Foo, foo, FOO )       \
    struct Foo : public Gsl_rng_basic< FOO >    \
    {                                           \
        Foo() : Gsl_rng_basic< FOO >( foo ) {}  \
    }

/**
 * @class Gsl_rng_mt19937
 * @extends Gsl_rng_basic< GSL_RNG_MT19937 >
 * @brief Random number generator using the "Mersenne Twister" algorithm.
 * This implements the "Mersenne Twister" of Matsumoto and Nishimura.
 * The period is about 10 ** 6000.
 * This is an all-around good PRNG.  Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_mt19937,    gsl_rng_mt19937,    GSL_RNG_MT19937 );

/**
 * @class Gsl_rng_ranlxs0
 * @extends Gsl_rng_basic< GSL_RNG_RANLXS0 >
 * @brief Random number generator using the "RANLUX" algorithm, 24 bits.
 * This implements the "RANLUX" algorithm of Luescher at single precision,
 * i.e., meant for a float not a double.  This is a "luxury random number"
 * algorithm, i.e., slow.  Nevertheless this one is "level 0" so it's the
 * entry-level luxury model.  Period is 10 ** 171.
 * Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_ranlxs0,    gsl_rng_ranlxs0,    GSL_RNG_RANLXS0 );

/**
 * @class Gsl_rng_ranlxs1
 * @extends Gsl_rng_basic< GSL_RNG_RANLXS1 >
 * @brief Random number generator using the "RANLUX" algorithm, 24 bits.
 * This implements the "RANLUX" algorithm of Luescher at single precision,
 * i.e., meant for a float not a double.  This is a "luxury random number"
 * algorithm, i.e., slow.  This one is "level 1" so it's the mid-level luxury
 * model.  Period is 10 ** 171.
 * Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_ranlxs1,    gsl_rng_ranlxs1,    GSL_RNG_RANLXS1 );

/**
 * @class Gsl_rng_ranlxs2
 * @extends Gsl_rng_basic< GSL_RNG_RANLXS2 >
 * @brief Random number generator using the "RANLUX" algorithm, 24 bits.
 * This implements the "RANLUX" algorithm of Luescher at single precision,
 * i.e., meant for a float not a double.  This is a "luxury random number"
 * algorithm, i.e., slow.  This one is "level 2" so it's the top-level luxury
 * model.  Period is 10 ** 171.
 * Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_ranlxs2,    gsl_rng_ranlxs2,    GSL_RNG_RANLXS2 );

/**
 * @class Gsl_rng_ranlxd1
 * @extends Gsl_rng_basic< GSL_RNG_RANLXD1 >
 * @brief Random number generator using the "RANLUX" algorithm, 48 bits, lvl. 1
 * This implements the "RANLUX" algorithm of Luescher at double precision.
 * This is a "luxury random number"
 * algorithm, i.e., slow.  This one is "level 1" so it's not as decorrelated 
 * as level 2.  Period is 10 ** 171.
 * Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_ranlxd1,    gsl_rng_ranlxd1,    GSL_RNG_RANLXD1 );

/**
 * @class Gsl_rng_ranlxd2
 * @extends Gsl_rng_basic< GSL_RNG_RANLXD2 >
 * @brief Random number generator using the "RANLUX" algorithm, 48 bits, lvl. 1
 * This implements the "RANLUX" algorithm of Luescher at double precision.
 * This is a "luxury random number"
 * algorithm, i.e., slow.  This one is "level 2" so it's the most decorrelated.
 * Period is 10 ** 171.
 * Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_ranlxd2,    gsl_rng_ranlxd2,    GSL_RNG_RANLXD2 );

/**
 * @class Gsl_rng_cmrg
 * @extends Gsl_rng_basic< GSL_RNG_CMRG >
 * @brief Random number generator using L'Ecuyer's 1996 algorithm.
 * This implements the Combined Multiple Recursive Generator algorithm of
 * L'Ecuyer (1996). Period is 10 ** 56.  Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_cmrg,       gsl_rng_cmrg,       GSL_RNG_CMRG    );

/**
 * @class Gsl_rng_mrg
 * @extends Gsl_rng_basic< GSL_RNG_MRG >
 * @brief Random number generator using 1993 algorithm of L'Ecuyer et al.
 * This implements the Multiple Recursive Generator algorithm of L'Ecuyer et
 * al. (1993).  Period is 10 ** 46.  Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_mrg,        gsl_rng_mrg,        GSL_RNG_MRG     );

/**
 * @class Gsl_rng_taus2
 * @extends Gsl_rng_basic< GSL_RNG_TAUS2 >
 * @brief Random number generator using Tausworthe's algorithm.
 * This is L'Ecuyer's version of Tausworthe's algorithm (or something like
 * that).  Period is 10 ** 26.  Defined using macro Gsl_rng_template.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_taus2,      gsl_rng_taus2,      GSL_RNG_TAUS2   );

/**
 * @class Gsl_rng_gfsr4
 * @extends Gsl_rng_basic< GSL_RNG_GFSR4 >
 * @brief Random number generator using a four-tap XOR using a shift register.
 * This uses Ziff's offsets (1998) and is very fast.
 * @see http://www.gnu.org/software/gsl/manual/html_node/Random-number-generator-algorithms.html
 */
Gsl_rng_template(   Gsl_rng_gfsr4,      gsl_rng_gfsr4,      GSL_RNG_GFSR4   );


/**
 * @brief An all-around good, fast, simulation-quality random number generator.
 *
 * If you don't know which random number generator to pick, I recommend
 * this one, the "Mersenne Twister." It is
 * what standard Python uses, it is fast, and has a period of ~ 10^6000.
 * It is competitive with the faster _TAU and _GFSR4 algorithms.
 *
 * If we discover flaws in the Mersenne Twister or if a faster algorithm
 * emerges in the future, then this "default" typedef will be changed to a 
 * better selection.  Application code that
 * uses this default type definition should be written to be robust against
 * such changes.
 */
typedef Gsl_rng_mt19937 Gsl_rng_default;

} // end ns kjb

#endif /* GSL_RNG_H_INCLUDED_LIBKJB_UOFARIZONAVISION */
