/**
 * @file
 * @author Ernesto Brau
 */
/*
 * $Id$
 */

#include "blob_cpp/blob_detector.h"
#include <iterator>
#include <utility>

std::vector<Matrix_vector> dog_scale_space(const GSS& gss)
{
    std::vector<Matrix_vector> dogss;

    for (int i_o = 0; i_o < gss.get_num_octaves(); ++i_o)
    {
        dogss.push_back(Matrix_vector());
        typedef std::vector<kjb::Image>::const_iterator VICI;
        std::pair<VICI, VICI> oct_its = gss.get_x_octave_at_index(i_o);
        for (VICI p = oct_its.first + 1; p != oct_its.second; ++p)
        {
            dogss.back().push_back(
                p -> to_grayscale_matrix() - (p - 1) -> to_grayscale_matrix());
        }
    }

    return dogss;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

Vec_vec_vec matrix_vector_to_vvv(const Matrix_vector& mv)
{
    Vec_vec_vec vvv(mv.size());
    for (size_t m = 0; m < mv.size(); ++m)
    {
        vvv[m] = std::vector<std::vector<double> >(mv[m].get_num_rows());
        for (int i = 0; i < mv[m].get_num_rows(); ++i)
        {
            vvv[m][i] = std::vector<double>(mv[m].get_num_cols());
            for (int j = 0; j < mv[m].get_num_cols(); ++j)
            {
                vvv[m][i][j] = mv[m](i, j);
            }
        }
    }

    return vvv;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

const std::set<Blob>& Blob_detector::operator()(const kjb::Image& I)
{
    /*const int num_levels = 3;
    const int min_level = -1;
    const int max_level = num_levels + 1;
    const double sigma_0 = 1.6 * std::pow(2, 1.0 / num_levels);
    const double thresh = 2.5 * num_levels;
    const int min_octave = round(log2(min_blob_size) - log2(1.6) - ((min_level + 1.0) / num_levels) - 1.0);
    const int max_octave = round(log2(max_blob_size) - log2(1.6) - (1.0 / num_levels) - 1.0);
    int num_octaves = max_octave - min_octave + 1;
    */
    
    const int min_level = -1; 
    const int max_level = num_levels + 1; 
    const double sigma_0 = 1.6 * std::pow(2, 1.0 / num_levels); 
    const int min_octave = round(log2(min_blob_size) - log2(1.6) - ((min_level + 1.0) / num_levels) - 1.0);
    double sigma = sigma_0 * std::pow(2, min_octave + (double)max_level/num_levels); 
    int h = I.get_num_rows();
    int w = I.get_num_cols();
    if(sigma > std::min(h, w) )
    {
        KJB_THROW_2(kjb::Illegal_argument, "min blob size is too big\n");
    }

    const int max_octave = round(log2(max_blob_size) - log2(1.6) - ((max_level + 1.0) / num_levels) - 1.0);
    double sigma_max = sigma_0 * std::pow(2, max_octave + max_level/num_levels);

    int num_octaves = max_octave - min_octave + 1;
    if(sigma_max > std::min(h, w)) num_octaves--; 

    GSS_generator compute_gss(num_octaves, min_octave, num_levels, min_level, max_level, sigma_0, 0.0);
    GSS gss = compute_gss(I);
    std::vector<Matrix_vector> dogss = dog_scale_space(gss);

    blobs.clear();
    for (int i_o = 0; i_o < num_octaves; i_o++)
    {
        local_argoptima_3D(matrix_vector_to_vvv(dogss[i_o]), threshold, std::insert_iterator<std::set<Blob> >(blobs, blobs.begin()), Compute_blob(&gss, i_o));
    }

    if(blobs.empty())
    {
        return blobs;
    }

    /*for (std::set<Blob>::const_iterator p = ++blobs.begin(); p != blobs.end(); p++)
    {
        std::set<Blob>::const_iterator q = p;
        Blob blob = *(--q);
        std::set<Blob>::const_iterator r = std::find_if(p, blobs.end(), std::bind1st(std::ptr_fun(blob_center_contained), blob));
        if(r != blobs.end())
        {
            blobs.erase(q);
        }
    }*/
    
    
    for (std::set<Blob>::const_iterator p = ++blobs.begin(); p != blobs.end(); p++)
    {
        std::set<Blob>::const_iterator q = p;
        Blob blob = *(--q);
        std::set<Blob>::const_iterator r = std::find_if(p, blobs.end(), std::bind1st(std::ptr_fun(blob_center_contains), blob));
        if(r != blobs.end())
        {
            blobs.erase(q);
        }
    }

    return blobs;
}
