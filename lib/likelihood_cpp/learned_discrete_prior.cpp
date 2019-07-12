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

#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <inttypes.h>
#include "likelihood_cpp/learned_discrete_prior.h"
#include "m/m_convolve.h"
#include "p/p_plot.h"
#include <typeinfo>

#include <fstream>
#include <sstream>

#define ONE_D_SOFT_BIN_RESOLUTION   (10)
#define SOFT_BINNING_SIGMA 1.0

using namespace kjb;

/** 
 * @param numBins  Number of bins in the histogram.
 * @param max  Double representing the maximum value of the histogram.
 * @param min  Double representing the minimum value of the histogram.
 * @param fname  Input file to read the data to build the histogram from.
 * @param softBin  1 if want to create histogram using soft-binning (default), 
 *                 0 if not.
 */
Learned_discrete_prior::Learned_discrete_prior
(
    int         numBins, 
    double      max, 
    double      min, 
    const char* fname, 
    int         softBin
) : Readable(), Writeable()
{
    double point;
    double b;
    int bin;
    double count = 0.0;

    num_bins = numBins;
    histo_max = max;
    histo_min = min;

    kjb_c::Vector* gauss_vp = NULL;
    double sigma = SOFT_BINNING_SIGMA;
    int half_mask_width = MAX_OF(1, (int)(6.0*ONE_D_SOFT_BIN_RESOLUTION*sigma));

    Vector tmp_bins(numBins, 0.0);

    // Read the file and count the number of values in each bin.
    std::ifstream file(fname);
    while(file >> point)
    {
    	// Check point is within max and min bounds.
        if(point < histo_min || point > histo_max)
        {
            KJB_THROW_3(Illegal_argument, "Error: the value %d doesn't fall "
                   "within the provided maximum and minimum bounds.", (point));
        }
        
        // Compute and store in the double b the bin that the point goes in.
        find_bin(b, point, num_bins, histo_min, histo_max);

        if(softBin == 0)
        {
            // If point == max, add 1 to last bin.
            if(fabs(histo_max - point) < DBL_EPSILON)
            {
                tmp_bins(num_bins - 1) += 1;
            }
            // Otherwise, take the floor of b to get the bin number and add 1 
            // to it.
            else
            {
                bin = std::floor(b);
                tmp_bins(bin) += 1;
            }
            // Count the number of points so we can normalize the histogram.
            count++;
        }
        else    // softBin == 1, so implement soft-binning.
        {
            // Create a gaussian mask with length 123 where the peak is located
            // at index 61.
            ETX(kjb_c::get_1D_gaussian_mask(&gauss_vp, 
                                            3 + 2 * half_mask_width, 
                                            ONE_D_SOFT_BIN_RESOLUTION * sigma));

            // Each bin in the histogram is covered by 10 bins (given by 
            // ONE_D_SOFT_BIN_RESOLUTION) in the gaussian mask.
            // The gaussian mask is centered over the portion of the bin that 
            // the point belongs in. Then the values of the mask bins are added
            // to the histogram bins that they are over. 
            for(int k = -half_mask_width; k <= half_mask_width; k++)
            {
                double d_bin = b+((double)k/(double)ONE_D_SOFT_BIN_RESOLUTION);

                if(d_bin >= 0.0)
                {
                    bin = (int)d_bin;

                    // This ensures that the values = histo_max are put in the 
                    // last bin.
                    if(d_bin == num_bins)                            
                    {
                        bin -= 1;
                    }

                    if((bin >= 0 ) && (bin < num_bins))
                    {
                        tmp_bins(bin) += gauss_vp->elements[half_mask_width 
                                                                    + k + 1];
                        
                        // count keeps track of the sum of the values in the 
                        // histogram bins so that we can normalize the histogram
                        // after all the points have been added.
                        count += gauss_vp->elements[half_mask_width + k + 1];
                    }
                }
            }
        }
    }

    // Normalize the histogram by dividing each bin by the sum of the bins.
    for(int i = 0; i < num_bins; i++)
    {
        tmp_bins(i) = tmp_bins(i) / count;
    }

    histo_bins = tmp_bins;

    kjb_c::free_vector(gauss_vp);
    file.close();
}

/** 
 * @param dp  the Learned_discrete_prior object to copy into this one.
 */
Learned_discrete_prior::Learned_discrete_prior(const Learned_discrete_prior& dp)
:   Readable(dp),
    Writeable(dp),
	num_bins(dp.num_bins),
    histo_max(dp.histo_max),
    histo_min(dp.histo_min),
    histo_bins(dp.histo_bins)
{

}

/** Frees all space allocated by this object. */
Learned_discrete_prior::~Learned_discrete_prior()
{
}

/** 
 * Performs a deep copy of the histogram bins.
 * 
 * @param dp  Learned_discrete_prior to copy into this one.
 *
 * @return A reference to this Learned_discrete_prior.
 */
Learned_discrete_prior& Learned_discrete_prior::operator=(
    const Learned_discrete_prior& dp
)
{
    if(this == &dp) return *this;

    this->num_bins = dp.get_num_bins();
    this->histo_max = dp.get_histo_max();
    this->histo_min = dp.get_histo_min();
    this->histo_bins = dp.get_histo_bins();
    
    return *this;
}

void Learned_discrete_prior::read(std::istream& in) throw (kjb::Illegal_argument,
        kjb::IO_error)
{
    using std::ostringstream;
    using std::istringstream;

    const char* type_name = typeid(*this).name();
    const char* field_value;

    if (!(field_value = read_field_value(in, "num_bins")))
    {
        throw Illegal_argument("Missing num_bins");
    }

    istringstream ist(field_value);
    ist >> num_bins;
    if (ist.fail() || (num_bins <= 0))
    {
        throw Illegal_argument("Invalid num_bins");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "histo_max")))
    {
        throw Illegal_argument("Missing histo_max");
    }
    ist.str(field_value);
    ist >> histo_max;
    if (ist.fail())
    {
        throw Illegal_argument("Invalid histo_max");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "histo_min")))
    {
        throw Illegal_argument("Missing histo_min");
    }
    ist.str(field_value);
    ist >> histo_min;
    if (ist.fail() || (histo_max <= histo_min))
    {
        throw Illegal_argument("Invalid histo_min");
    }
    ist.clear(std::ios_base::goodbit);

    int histo_size;
    if (!(field_value = read_field_value(in, "histo_size")))
    {
        throw Illegal_argument("Missing histo_size");
    }
    ist.str(field_value);
    ist >> histo_size;
    if (ist.fail() || (histo_size < 0))
    {
        throw Illegal_argument("Invalid histo_size");
    }
    ist.clear(std::ios_base::goodbit);

    histo_bins.clear();
    double histo_val = 0.0;
    if (!(field_value = read_field_value(in, "histo_values")))
    {
        throw Illegal_argument("Missing histo_values");
    }
    ist.str(field_value);
    for(unsigned int i = 0; i < histo_size; i++)
    {
        ist >> histo_val;
        histo_bins.push_back(histo_val);
    }
    if (ist.fail())
    {
        throw Illegal_argument("Invalid histo_values");
    }
    ist.clear(std::ios_base::goodbit);

}

/** Writes this learn_discrete_prior to a file. */
void Learned_discrete_prior::write(std::ostream& out) const
       throw (kjb::IO_error)
{
    out << "num_bins: " << num_bins << '\n'
    << "histo_max: " << histo_max << '\n'
    << "histo_min: " << histo_min << '\n'
    << "histo_size: " << histo_bins.size() << '\n'
    << "histo_values: ";
    for(unsigned int  i = 0; i < histo_bins.size(); i++)
    {
        out << histo_bins(i) << ' ';
    }
    out << '\n';
}

/**
 * @return  The number of bins in the histogram.
 */
int Learned_discrete_prior::get_num_bins() const
{
    return num_bins;
}

/**
 * @return  The maximum value of the histogram.
 */
double Learned_discrete_prior::get_histo_max() const
{
    return histo_max;
}

/**
 * @return  The minimum value of the histogram.
 */
double Learned_discrete_prior::get_histo_min() const
{
    return histo_min;
}

/**
 * @return  The Vector containing the count of points in each bin.
 */
const Vector& Learned_discrete_prior::get_histo_bins() const
{
    return histo_bins;
}

/**
 * @param  name  The name of the ps file the histogram plot will be 
 *               saved to.  If NULL, the plot will be saved to a file
 *               called plot.ps
 *
 * @return  NO_ERROR on success and ERROR on failure.
 */
int Learned_discrete_prior::plot_histogram(const char* name) const
{
    double step;
    int result;
    kjb_c::Vector* hist_vp = NULL;
    int plot_id;
    
    hist_vp = kjb_c::create_zero_vector(num_bins);

    step = (histo_max - histo_min) / num_bins;
    
    if (step < 1.0e2 * DBL_MIN)
    {
        kjb_c::set_error("Bin size is too small for histogram plot.");
        return kjb_c::ERROR;
    }

    for(int i = 0; i < num_bins; i++)
    {
        hist_vp->elements[i] = histo_bins(i);
        std::cout << "Bin i:" << histo_bins(i) << std::endl;
    }

    double plot_min = histo_min + (step / 2.0);

    plot_id = kjb_c::plot_open();
    std::cout << "Plot_id is:" << result << std::endl;
    result = kjb_c::plot_bars(plot_id, hist_vp, plot_min, step, name);
    std::cout << "Result is:" << result << std::endl;

    kjb_c::save_plot(plot_id, name);
    kjb_c::plot_close(plot_id);

    kjb_c::free_vector(hist_vp);

    return result;
}

/** 
 * @param bin  Double to store the number of the bin to add the element to.
 * @param element  The element to be added to the histogram.
 * @param numBins  The number of bins in the histogram.
 * @param min  The minimum value that can be added to the histogram.
 * @param max  The maximum value that can be added to the histogram.
 */
void Learned_discrete_prior::find_bin
(
    double& bin, 
    double  element, 
    int     numBins, 
    double  min, 
    double  max
) const
{
	if(element <= min)
	{
		bin = 0;
	}
	else if(element >= max)
	{
		bin = numBins - 1;
	}
	else
	{
        bin = (element - min) * (numBins / (max - min));
	}
}

void Learned_discrete_prior::find_min_max
(
    double      & hmax,
    double      & hmin,
    const char* fname
)
{
	double point = 0.0;

    // Read the file and count the number of values in each bin.
    std::ifstream file(fname);
    int counter = 0;
    while(file >> point)
    {
    	if(counter == 0)
    	{
    		hmax = point;
    		hmin = point;
    	}
    	else
    	{
    		if(hmax < point)
    		{
    			hmax = point;
    		}
    		else if(hmin > point)
    		{
    			hmin = point;
    		}
    	}
    	counter++;
    }
    file.close();

}
