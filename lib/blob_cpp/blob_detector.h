/**
 * @file
 * @author Ernesto Brau
 */
/*
 * $Id$
 */

#ifndef BLOB_DETECTOR_INCLUDED
#define BLOB_DETECTOR_INCLUDED

#include "i_cpp/i_image.h"
#include "m_cpp/m_matrix.h"
#include "blob_cpp/blob_gss.h"
#include "blob_cpp/blob_local_optimize.h"
#include <vector>
#include <set>
#include <cmath>

/**
 * @brief   A simple class that represents a blob.
 */
class Blob
{
public:
    int row;
    int col;
    int size;

    /**
     * @brief   Construct a blob in (r,c) of size s;
     */
    Blob(int r, int c, int s) :
        row(r),
        col(c),
        size(s)
    {}

#if 0
    /**
     * @brief   Copy-constructor.
     */
    Blob(const Blob& b) :
        row(b.row),
        col(b.col),
        size(b.size)
    {}

    /**
     * @brief   Assignment operator; member-wise assignment.
     */
    Blob& operator=(const Blob& b)
    {
        if(&b != this)
        {
            row = b.row;
            col = b.col;
            size = b.size;
        }
        return *this;
    }
#endif
};


/**
 * @brief   Compare two blobs; member-wise comparison.
 */
inline
bool operator==(const Blob& b1, const Blob& b2)
{
    return b1.size == b2.size && b1.row == b2.row && b1.col == b2.col;
}

/**
 * @brief   Lexicographical less than comparison: size, row, col order.
 */
inline
bool operator<(const Blob& b1, const Blob& b2)
{
    if(b1.size != b2.size)
    {
        return b1.size < b2.size;
    }

    if(b1.row != b2.row)
    {
        return b1.row < b2.row;
    }

    if(b1.col != b2.col)
    {
        return b1.col < b2.col;
    }

    return false;
}

/**
 * @brief   Tests whether a blob's center is contained in another blob.
 *
 * @return  Returns true iff b1's center is inside the circle centered
 *          at b2 of radius b2.size.
 */
inline
bool blob_center_contained(Blob b1, Blob b2)
{
    double d = std::sqrt(((b1.row - b2.row) * (b1.row - b2.row)) + ((b1.col - b2.col) * (b1.col - b2.col)));
    return d <= b2.size / 2.0;
}

/**
 * @brief   Tests whether a blob's center is contained in another blob.
 *
 * @return  Returns true iff b1's center is inside the circle centered
 *          at b2 of radius b2.size.
 */
inline
bool blob_center_contains(Blob b1, Blob b2)
{
    double d = std::sqrt(((b1.row - b2.row) * (b1.row - b2.row)) + ((b1.col - b2.col) * (b1.col - b2.col)));
    return d <= b1.size / 2.0;
}

typedef std::vector<kjb::Matrix> Matrix_vector;

/**
 * @brief   Compute the difference of Gaussians (DoG) scale space
 */
std::vector<Matrix_vector> dog_scale_space(const GSS& gss);

/**
 * @brief   A blob detector class. Use operator() to apply to image.
 *
 * This class detects "blobs" in images, where a blob is detected
 * using the method in Lindenberg-98, which Lowe uses for SIFT as
 * well.
 */
class Blob_detector
{
private:
    int min_blob_size;
    int max_blob_size;
    int num_levels; 
    double threshold; 
    std::set<Blob> blobs;

public:
    /**
     * @brief   Construct a blob detector with the given max/min blob sizes.
     */
    Blob_detector(int minblobsize, int maxblobsize) :
        min_blob_size(minblobsize),
        max_blob_size(maxblobsize), 
        num_levels(3),
        threshold(7.5)
    {}

    /**
     * @brief   Construct a blob detector with the given max/min blob sizes.
     */
    Blob_detector
    (
        int minblobsize, 
        int maxblobsize,
        int numlevels, 
        double thresh
    ) :
        min_blob_size(minblobsize),
        max_blob_size(maxblobsize), 
        num_levels(numlevels),
        threshold(thresh)
    {}

    /**
     * @brief   Copy-ctor.
     */
    Blob_detector(const Blob_detector& bd) :
        min_blob_size(bd.min_blob_size),
        max_blob_size(bd.max_blob_size),
        num_levels(bd.num_levels),
        threshold(bd.threshold),
        blobs(bd.blobs)
    {}

    /**
     * @brief   Assignment operator. Member-wise assignment.
     */
    Blob_detector& operator=(const Blob_detector& bd)
    {
        if(&bd != this)
        {
            min_blob_size = bd.min_blob_size;
            max_blob_size = bd.max_blob_size;
            num_levels = bd.num_levels; 
            threshold = bd.threshold; 
            blobs = bd.blobs;
        }
        return *this;
    }

    /**
     * @brief   Applies this blob detector to the given image.
     *
     * Detects the blobs in I and stores the blobs in its internal state.
     *
     * @return  A const-ref to the blob set.
     */
    const std::set<Blob>& operator()(const kjb::Image& I);

    /**
     * @brief   Getter for the min blob size
     *
     * @return  The min blob size
     */
    int get_min_blob_size() const
    {
        return min_blob_size;
    }

    /**
     * @brief   Getter for the max blob size
     *
     * @return  The max blob size
     */
    int get_max_blob_size() const
    {
        return max_blob_size;
    }

    /**
     * @brief   Getter for the num_levels 
     */

    /**
     * @brief   Retrieves the blobs computed in the last call to operator().
     *
     * @return  A set of blobs computed via operator(), or an empty set if
     *          the operator() has not been called.
     */
    const std::set<Blob>& get_blobs() const
    {
        return blobs;
    }

    ~Blob_detector(){}
};

// helper typedef and function for use of local_argoptima
typedef std::vector<std::vector<std::vector<double> > > Vec_vec_vec;
Vec_vec_vec matrix_vector_to_vvv(const Matrix_vector& mv);

// computes blob parameters from local optima results; for use
// with std::transform
struct Compute_blob
{
    const GSS* m_gss;
    int m_octave_index;

    Compute_blob(const GSS* gss, int octave_index) :
        m_gss(gss),
        m_octave_index(octave_index)
    {}

    Compute_blob& operator=(const Compute_blob& cb)
    {
        if(&cb != this)
        {
            m_gss = cb.m_gss;
            m_octave_index = cb.m_octave_index;
        }

        return *this;
    }

    Blob operator()(int scale_index, int scaled_row, int scaled_col)
    {
        return Blob(static_cast<int>(scaled_row * std::pow(2.0, m_gss->get_octave_name(m_octave_index))),
                    static_cast<int>(scaled_col  * std::pow(2.0, m_gss->get_octave_name(m_octave_index))),
                    2 * static_cast<int>(m_gss->sigma_at_indices(m_octave_index, scale_index)));
    }
};


#endif /*BLOB_DETECTOR_INCLUDED */

