/* $Id: color_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#include "likelihood_cpp/color_likelihood.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_stat.h"

#include <iostream>

namespace kjb {

std::map<int, Region_vector> get_regions_from_mask(const Int_matrix& mask, int region_length)
{
    /* Image tempimage(50 * mask);
    tempimage.write("lalalala.jpg"); */

    std::cout << "AHAHAHAAHAHAHAHAHAHAHAHAHAH**************************************************************\n\n\n\n";

    std::map<int, Region_vector> region_vectors;
    for(int i = 0; i < mask.get_num_rows(); i++)
    {
        for(int j = 0; j < mask.get_num_cols(); j++)
        {
            int region_num = mask(i, j);
            if(region_num == 0)
            {
                continue;
            }

            int base_region_num = ((region_num - 1) / region_length) + 1;
            if(region_vectors.count(base_region_num) == 0)
            {
                region_vectors[base_region_num] = Region_vector(region_length);
            }
            region_vectors[base_region_num][(region_num - 1) % region_length].push_back(std::make_pair(i, j));

            //std::cout << region_vectors.size();
        }
    }

    return region_vectors;
}

double Color_likelihood::operator()(const std::map<int, Region_vector>& object_regions)
{
    double logp = -100000000000.0;
    for(std::map<int, Region_vector>::const_iterator rv_p = object_regions.begin(); rv_p != object_regions.end(); rv_p++)
    {
        const Region_vector& rv = rv_p->second;
        std::vector<kjb::Vector> histograms(rv.size());
        std::vector<kjb::Vector>::iterator hist_p = histograms.begin();

        for(Region_vector::const_iterator region_p = rv.begin(); region_p != rv.end(); region_p++)
        {
            const Region& reg = *region_p;
            if(!reg.empty())
            {
                //std::cout << "distance: " << std::distance(reg.begin(), reg.end()) << std::endl;
                *hist_p = m_image.intensity_histogram(reg.begin(), reg.end());
            }
            hist_p++;
        }

        for(size_t i = 0; i < histograms.size() - 1; i++)
        {
            if(!rv[i].empty())
            {
                for(size_t j = i + 1; j < histograms.size(); j++)
                {
                    if(!rv[j].empty())
                    {
                        double chi_sq_st = chi_square_statistic(histograms[i].begin(), histograms[i].end(), histograms[j].begin());
                        double p = kjb::pdf(Chi_square_distribution(2), chi_sq_st);
                        logp += log(p);
                    }
                }
            }
        }
    }

    return logp;
}

} // namespace kjb

