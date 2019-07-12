/* $Id$ */
/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

/*=========================================================================== *
  |
  | Copyright (c) 1994-2010 by Kobus Barnard (author)
  |
  | Personal and educational use of this code is granted, provided that this
  | header is kept intact, and that the authorship is not misrepresented, that
  | its use is acknowledged in publications, and relevant papers are cited.
  |
  | For other use contact the author (kobus AT cs DOT arizona DOT edu).
  |
  | Please note that the code in this file has not necessarily been adequately
  | tested. Naturally, there is no guarantee of performance, support, or fitness
  | for any particular task. Nonetheless, I am interested in hearing about
  | problems that you encounter.
  |
  | Author: Emily Hartley
 *=========================================================================== */

#ifndef KJB_LEARNED_DISCRETE_PRIOR_H_
#define KJB_LEARNED_DISCRETE_PRIOR_H_

#include "m_cpp/m_vector.h"
#include "l_cpp/l_readable.h"
#include "l_cpp/l_writeable.h"
#include <cmath>

namespace kjb {

/**
 * @brief   This class creates a histogram of a list of points and 
 *          stores the number of bins in num_bins, the maximum and 
 *          minimum values allowed in histo_max and histo_min,
 *          respectively, and the count of the number of points in
 *          each bin in histo_bins, which is a Vector of size
 *          num_bins.
 */
class Learned_discrete_prior:  public Readable, public Writeable
{
private:
    int num_bins;
    double histo_max;
    double histo_min;
    Vector histo_bins;

protected:
    /**
     * @brief Determines which bin in the histogram the given element should 
     *        be put in. 
     */
    void find_bin
    (
        double& bin, 
        double  element, 
        int     numBins, 
        double  min, 
        double  max
    ) const;

public:
   /** @brief Constructor. */
    Learned_discrete_prior
    (
        int         numBins, 
        double      max, 
        double      min,
        const char* fname, 
        int         softBin=1
    );

    Learned_discrete_prior()
    {
    	num_bins = 1;
    	histo_max = 1.0;
    	histo_min = 0.0;
    }

   /** @brief Copy constructor. */
    Learned_discrete_prior(const Learned_discrete_prior& dp);

    /** @brief Constructs a parametric_parapiped from an input file. */
    Learned_discrete_prior(const char* fname) throw (kjb::Illegal_argument,
                kjb::IO_error)
	{
        read(fname);
	}

    /** @brief Constructs a parametric_parapiped from an input stream. */
    Learned_discrete_prior(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error)
	{
        read(in);
	}

   /** @brief Destructor. */
    ~Learned_discrete_prior();

   /** @brief Assignment operator. */
    Learned_discrete_prior& operator=(const Learned_discrete_prior& dp);

    /** @brief Reads this parametric_parapiped from an input stream. */
    virtual void read(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error);

    /** @brief Reads this parametric_parapiped from a file */
    virtual void read(const char * fname) throw (kjb::IO_error,  kjb::Illegal_argument)
    {
        Readable::read(fname);
    }

    /** @brief Writes this parametric_parapiped to a file. */
    virtual void write(std::ostream& out) const
       throw (kjb::IO_error);

    /** @brief Writes this parametric_parapiped to an output stream. */
    virtual void write(const char * fname) const
       throw (kjb::IO_error)
    {
        Writeable::write(fname);
    }

    /** @brief Returns the number of bins. */
    int get_num_bins() const;

    /** @brief Returns the maximum value of the histogram. */
    double get_histo_max() const;

    /** @brief Returns the minimum value of the histogram. */
    double get_histo_min() const;

    /** @brief Returns the Vector containing the count of points in each bin. */
    const Vector& get_histo_bins() const;
    
    /** 
     * @brief Creates a plot of the histogram and saves it in ps format in a 
     *        file with the provided name or plot.ps if no name is provided. 
     */
    int plot_histogram(const char* name=NULL) const;

    static void find_min_max(double & hmax, double & hmin, const char* fname);

    inline double evaluate_prior(double value) const
    {
    	double dbin = 0.0;
    	find_bin(dbin, value, num_bins, histo_min, histo_max);
    	int bin = std::floor(dbin);
    	return histo_bins(bin);
    }

};

} // namespace kjb

#endif /* KJB_LEARNED_DISCRETE_PRIOR_H_ */

