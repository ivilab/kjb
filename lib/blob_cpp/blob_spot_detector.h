/**
 * @file
 * @brief Declaration of Spot_detector functor
 * @author Ernesto Brau
 * @author Andrew Predoehl
 */
/*
 * $Id: blob_spot_detector.h 17797 2014-10-21 04:41:57Z predoehl $
 */

#ifndef BLOB_SPOT_DETECTOR_H_INCLUDED
#define BLOB_SPOT_DETECTOR_H_INCLUDED

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <i_cpp/i_image.h>
#include <vector>

namespace kjb
{

/**
 * @brief   A spot detector functor, comparable to a blob detector.
 *
 * This class detects "spots" in images, where a spot is a set of
 * 8-connected pixels that pass certain tests:
 * - After the background is subtracted out, the brightest pixel
 *   must be at least a certain brightness (in absolute value).
 * - Pixels in a blob must be at least a certain fraction
 *   (the "similarity" parameter) as bright as the brightest pixel
 *   above background.
 *
 * Use the operator() to apply this detector to an image.
 */
class Spot_detector
{
public:
    typedef std::vector<Vector> Centroid_set;
    typedef std::vector<std::pair<int, int> > Pair_vector;

private:
    Matrix m_background, m_thresholds;
    int m_min_brightness, m_min_size, m_max_size;
    double m_similarity;
    mutable Centroid_set spot_centroids;
    mutable std::vector<Pair_vector> spot_coordinates;

    void swap(Spot_detector& sd)
    {
        using std::swap;

        m_background.swap(sd.m_background);
        m_thresholds.swap(sd.m_thresholds);
        swap(m_min_brightness, sd.m_min_brightness);
        swap(m_min_size, sd.m_min_size);
        swap(m_max_size, sd.m_max_size);
        swap(m_similarity, sd.m_similarity);
        spot_centroids.swap(sd.spot_centroids);
        spot_coordinates.swap(sd.spot_coordinates);
    }

public:
    /**
     * @brief   Construct a spot detector with the given parameters.
     *
     * @param background    Background image. Will be subtracted from the
     *                      operand image. In applications involving sequences
     *                      of images, it is usually the pixelwise average of
     *                      all frames.  With noisier image sequences, a matrix
     *                      of pixelwise medians might work better.
     *
     * @param thresholds    Image of thresholds.  The brightest pixel in the
     *                      operand image must be this much brighter than bg.
     *                      In applications 
     *                      involving sequence of images, it is usually
     *                      a multiple of the standard deviation of the
     *                      images.
     *
     * @param min_brightness The location of the brightest pixel above
     *                      background must be at least this bright
     *                      in the original input image.
     *
     * @param min_size      Spots with fewer pixels than this will be
     *                      ignored.
     *
     * @param max_size      Spots with more pixels than this will be
     *                      ignored.
     *
     * @param similarity    Pixels in a blob must be at least this
     *                      fraction as bright as the image's brightest
     *                      pixel above background.
     */
    Spot_detector
    (
        const Matrix& background,
        const Matrix& thresholds,
        int min_brightness,
        int min_size,
        int max_size,
        double similarity
    ) :
        m_background(background),
        m_thresholds(thresholds),
        m_min_brightness(min_brightness),
        m_min_size(min_size),
        m_max_size(max_size),
        m_similarity(similarity)
    {}

    /**
     * @brief   Construct a spot detector using constant background, threshold
     *
     * This ctor is similar to the ctor that takes Matrix background and
     * and threshold inputs, for the case that the background is a uniform
     * value across all pixels, and ditto for the thresholds.
     *
     * @param background    Background value. Background image (see above)
     *                      will be created from this value.
     *
     * @param thresholds    Threshold value. Threshold image (see above)
     *                      will be created from this value.
     *
     * @param min_brightness (see other ctor)
     * @param min_size      (see other ctor)
     * @param max_size      (see other ctor)
     * @param similarity    (see other ctor)
     */
    Spot_detector
    (
        int num_rows,
        int num_cols,
        double background,
        double  threshold,
        int min_brightness,
        int min_size,
        int max_size,
        double similarity
    )
    {
        Spot_detector sd(
                Matrix(num_rows, num_cols, background),
                Matrix(num_rows, num_cols, threshold),
                min_brightness,
                min_size,
                max_size,
                similarity
            );
        swap(sd);
    }

    /**
     * @brief   Applies this spot detector to the given image.
     *
     * Detects the spots in image and stores the spots in its internal state.
     *
     * @return  A const-ref to the set of spot centroids.
     */
    const Centroid_set& operator()(const Image&) const;

    /**
     * @brief   Retrieves the spots computed in the last call to operator().
     *
     * @return  A set of spots computed via operator(), or an empty set if
     *          the operator() has not been called.
     */
    const std::vector<Pair_vector>& get_spot_coordinates() const
    {
        return spot_coordinates;
    }

    /**
     * @brief   Retrieves the spot centroids computed in last operator() call.
     *
     * @return  A set of spot centroids computed via operator(), or an empty
     *          set if the operator() has not been called.
     */
    const Centroid_set& get_spot_centroids() const
    {
        return spot_centroids;
    }
};

} //namespace kjb

#endif /* BLOB_SPOT_DETECTOR_H_INCLUDED */

