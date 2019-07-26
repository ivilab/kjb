/* $Id: color_likelihood.h 10622 2011-09-29 19:50:53Z predoehl $ */
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

/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author: Ernesto Brau
 * =========================================================================== */

#ifndef KJB_COLOR_LIKELIHOOD_H_
#define KJB_COLOR_LIKELIHOOD_H_

#include "i_cpp/i_image.h"
#include "m_cpp/m_int_matrix.h"

#include <vector>
#include <list>
#include <map>
#include <utility>

/**
 * @file    color_likelihood.h  This file contains functions
 * used to compute the likelihood of a set of block faces
 * given the grayscale intensities of the corresponding image regions.
 */
namespace kjb {

/**
 * @brief   Later...
 */
typedef std::list<std::pair<int, int> > Region;

/**
 * @brief   Later...
 */
typedef std::vector<Region> Region_vector;

/**
 * @brief   Later...
 */
std::map<int, Region_vector> get_regions_from_mask(const Int_matrix& mask, int region_length);

/**
 * @brief   Functor that computes the likelihood of a set of projected
 *          faces onto an image, using the color distribution of each
 *          projected face. More later...
 */
class Color_likelihood
{
private:
    Image m_image;

public:
   /** @brief Constructor. */
    Color_likelihood(const Image& image) : m_image(image)
    {}

   /** @brief Copy constructor. */
    Color_likelihood(const Color_likelihood& cl) : m_image(cl.m_image)
    {}

   /** @brief Assignment operator. */
    Color_likelihood& operator=(const Color_likelihood& cl)
    {
        if(&cl != this)
        {
            m_image = cl.m_image;
        }

        return *this;
    }

   /** @brief Destructor. */
    ~Color_likelihood(){}

   /** @brief Calculates the log likelihood of the image edge set. */
   double operator()(const std::map<int, Region_vector>& object_regions);

};

} // namespace kjb

#endif /* KJB_COLOR_LIKELIHOOD_H_ */

