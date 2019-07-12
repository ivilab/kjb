/**
 * @file
 * @brief Def of function operator of class Spot_detector
 * @author Ernesto Brau
 */
/*
 * $Id: blob_spot_detector.cpp 17797 2014-10-21 04:41:57Z predoehl $
 */

#include <l/l_int_matrix.h>
#include <blob_cpp/blob_spot_detector.h>
#include <m/m_vector.h>
#include <seg/seg_spots.h> /* decl. of find_bright_spots_in_image */

namespace
{

// RAII object automatically releases Vector_vector
struct VecVec
{
    kjb_c::Vector_vector* m_vvp;
    VecVec(kjb_c::Vector_vector* p) : m_vvp(p) {}
    ~VecVec() { kjb_c::free_vector_vector(m_vvp); }
};

// RAII object automatically releases Int_matrix_vector
struct IntMatVec
{
    kjb_c::Int_matrix_vector* m_imvp;
    IntMatVec(kjb_c::Int_matrix_vector* p) : m_imvp(p) {}
    ~IntMatVec() { kjb_c::free_int_matrix_vector(m_imvp); }
};

}

namespace kjb
{

const Spot_detector::Centroid_set& Spot_detector::operator()
(
    const Image& img
)   const
{
    // temporary containers for spot 
    kjb_c::Vector_vector* centroids = NULL;
    kjb_c::Int_matrix_vector* spots = NULL;

    // The key functionality is in this C function in lib/seg
    ETX(kjb_c::find_bright_spots_in_image(
                img.c_ptr(),
                m_background.get_c_matrix(),
                m_thresholds.get_c_matrix(),
                m_min_brightness,
                m_min_size,
                m_max_size,
                m_similarity,
                &spots,
                &centroids
            ));

    // RAII insurance that the above allocations will be freed.
    VecVec auto_centroids(centroids);
    IntMatVec auto_spots(spots);

	// copy centroids and spots into different containers
    spot_centroids.resize(centroids -> length, Vector(2, 0.0));
    spot_coordinates.resize(spots -> length);

    for (size_t i = 0; i < spot_centroids.size(); ++i)
    {
        spot_centroids[i].set(centroids->elements[i]->elements[0],
                              centroids->elements[i]->elements[1]);
        spot_coordinates[i].resize(spots->elements[i]->num_rows);

        for (size_t j = 0; j < spot_coordinates[i].size(); ++j)
        {
            spot_coordinates[i][j].first = spots->elements[i]->elements[j][0];
            spot_coordinates[i][j].second = spots->elements[i]->elements[j][1];
        }
    }

    return spot_centroids;
}

} //namespace kjb

