/**
 * @file
 * @brief Class defs for C++ wrapper on GNU GSL random number distributions
 * @author Andrew Predoehl
 */
/*
 * $Id: gsl_randist.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef GSL_RANDIST_H_INCLUDED_LIBKJB_UOFARIZONAVISION
#define GSL_RANDIST_H_INCLUDED_LIBKJB_UOFARIZONAVISION

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <gsl_cpp/gsl_rng.h>

#ifdef KJB_HAVE_GSL
// these are GSL headers, not our wrapper
#include "gsl/gsl_randist.h"
#else
#warning "Compiling GNU GSL wrapper without GNU GSL; it will not run properly"
#endif


namespace kjb {

/**
 * @brief Randomly sample discrete events with an empirical distribution.
 *
 * This is an RAII wrapper on the GNU GSL code to support sampling from an
 * empirically-defined distribution of discrete events.
 *
 * @throws Missing_dependency if compiled with NO_LIBS.
 *
 * @see http://www.gnu.org/software/gsl/manual/html_node/General-Discrete-Distributions.html
 */
class Gsl_ran_discrete {

#ifdef KJB_HAVE_GSL
    gsl_ran_discrete_t* m_opaque;
#endif

public:

    /**
     * @brief build the sampler from an array of event weights or probabilities
     *
     * Event weights do not need to be normalized to sum to 1.
     */
    Gsl_ran_discrete( size_t event_count, const double* event_probs )
#ifdef KJB_HAVE_GSL
    :   m_opaque( gsl_ran_discrete_preproc( event_count, event_probs ) )
    {
        ETX_2( 00 == m_opaque, "Gsl_ran_discrete ctor:  bad alloc" );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    /**
     * @brief build the sampler from a Vector event weights or probabilities
     *
     * Event weights do not need to be normalized to sum to 1.
     */
    Gsl_ran_discrete( const Vector& event_probs )
#ifdef KJB_HAVE_GSL
    :   m_opaque( 00 )
    {
        Gsl_ran_discrete grd(
                            event_probs.size(),
                            event_probs.get_c_vector() -> elements
                        );
        swap( grd );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif


    /**
     * @brief build the sampler from a std::vector< double > of event weights
     *
     * Event weights do not need to be normalized to sum to 1.
     */
    Gsl_ran_discrete( const std::vector< double >& event_probs )
#ifdef KJB_HAVE_GSL
    :   m_opaque( 00 )
    {
        // Herb Sutter tells me the following code is legit:
        // http://herbsutter.com/2008/04/07/cringe-not-vectors-are-guaranteed-to-be-contiguous/
        Gsl_ran_discrete grd( event_probs.size(), & event_probs[ 0 ] );
        swap( grd );
    }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
    }
#endif

    /**
     * @brief sample from the discrete distribution
     * @param rng   This is a GSL random number generator pointer.  I recommend
     *              that you instantiate a class such as Gsl_rng_mt19937 and
     *              pass in that object, which will be transparently converted
     *              to the correct type.  To prevent correlations, you should
     *              not instantiate a new RNG object each time you sample or
     *              even each time you instantiate this class.  You should
     *              minimize
     *              the number of instantiations of random number generators.
     */
    double sample( const gsl_rng* rng )
    {
#ifdef KJB_HAVE_GSL
        return gsl_ran_discrete( rng, m_opaque );
#endif
    }

    /**
     * @brief Recompute the probability mass (or density) for event k.
     * @warning this is sort of an expensive operation:  if you can save the
     *          original array of probabilities (or densities) that were used
     *          in the ctor, that would be quicker.
     */
    double pdf( size_t k )
    {
#ifdef KJB_HAVE_GSL
        return gsl_ran_discrete_pdf( k, m_opaque );
#endif
    }


    /// @brief swap the internal state of two of these objects
    void swap( Gsl_ran_discrete& that )
    {
#ifdef KJB_HAVE_GSL
        using std::swap;

        swap( m_opaque, that.m_opaque );
#endif
    }


#ifdef KJB_HAVE_GSL
    /// @brief dtor frees the resources used by internal tables, etc.
    ~Gsl_ran_discrete()
    {
        gsl_ran_discrete_free( m_opaque );
    }
#endif

};


} // end ns kjb

#endif /* GSL_RANDIST_H_INCLUDED_LIBKJB_UOFARIZONAVISION */
