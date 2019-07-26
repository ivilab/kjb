/**
 * @file
 * @brief Code for computing basic color histograms for an image.
 * @author Luca Del Pero
 * @author Andrew Predoehl
 */
/*
 * $Id: i_color_histogram.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef KJB_CPP_COLOR_HISTOGRAM_H
#define KJB_CPP_COLOR_HISTOGRAM_H

#define DEFAULT_COL_HIST_NUM_BINS 8

#include "i_cpp/i_image.h"
#include <m_cpp/m_vector.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>

#include <numeric>

namespace kjb 
{

/**
 * @addtogroup kjbImageProc
 * @{
 */

/**
 * @class Color_histogram
 *
 * @brief Class to compute an RGB colour histogram over an image
 * or a rectangular portion of it. The histogram is normalized.
 * We use the same number of bins for each of the channels (r,g,b).
 * It is easy to extend this class so that it computes such histogram
 * over a segment of a shape other than rectangular
 */
class Color_histogram : public Readable, public Writeable
{
public:

    /// @brief Constructor. Only memory for color histogram is allocated
    Color_histogram(unsigned int inum_bins = DEFAULT_COL_HIST_NUM_BINS);

    /// @brief Constructor. Color histogram for entire image is computed
    Color_histogram(unsigned int inum_bins, const kjb::Image & img);

    /// @brief Constructor. Constructs a color histogram by reading it from a file
    Color_histogram(const char * fname)
    {
        Readable::read(fname);
    }

    /// @brief Constructor. Constructs a color histogram by reading it from an input stream
    Color_histogram(std::istream &is)
    {
        read(is);
    }

    /// @brief Constructor. Color histogram for the specified portion of the image is computed
    Color_histogram
    (
        unsigned int inum_bins,
        const Image & img,
        int top_left_x,
        int top_left_y,
        int bottom_right_x,
        int bottom_right_y
    );

    /// @brief Copy constructor
    Color_histogram(const Color_histogram & src);

    /// @brief Assignment operator
    Color_histogram & operator =(const Color_histogram & src);

    /// @brief Computes a color histogram over the specified image region, and using the input number of bins
    void compute_histogram
    (
        const kjb::Image & img,
        unsigned int inum_bins,
        int top_left_x,
        int top_left_y,
        int bottom_right_x,
        int bottom_right_y
    );

    /// @brief Computes a color histogram over the entire image, and using the input number of bins 
    void compute_histogram(const kjb::Image & img, unsigned int inum_bins);

    /// @brief Computes a color histogram over the entire image, without changin the number of bins 
    void compute_histogram(const kjb::Image & img);

    /// @brief Computes a color histogram over the specified image portion, without changin the number of bins 
    void compute_histogram
    (
        const kjb::Image & img,
        int top_left_x,
        int top_left_y,
        int bottom_right_x,
        int bottom_right_y
    );

    /// @brief Returns the number of bins 
    unsigned int get_num_bins() const
    {
        return num_bins;
    }

    /// @brief Returns the value of the bin corresponding to the input (r,g,b) values 
    inline double get_value(float r, float g, float b) const
    {
        return m_histogram[compute_rgb_bin(r,g,b)];
    }

    inline double get_value(int ibin_number) const
    {
        return m_histogram[ibin_number];
    }

    void print_human_readable(std::ostream & out) const;

    double compare(const Color_histogram & ch) const;

    /// @brief compute sum of all the bins in the histogram
    double sum() const
    {
        /*
        double dsum = 0.0;
        for(int i = 0; i < m_histogram.size(); ++i)
        {
            dsum+= m_histogram[i];
        }
        return dsum;
        */
        return std::accumulate( m_histogram.begin(), m_histogram.end(), 0.0 );
    }

    /** @brief Reads this camera from an input stream. */
    virtual void read(std::istream& in);

    /** @brief Reads this parametric_parapiped from a file */
    virtual void read(const char * fname)
    {
        Readable::read(fname);
    }


    /** @brief Writes this camera to an output stream. */
    virtual void write(std::ostream& out) const
      ;

    /** @brief Writes this camera to an output stream. */
    virtual void write(const char * fname) const
    {
        Writeable::write(fname);
    }
private:
    Vector m_histogram;

    /// @brief Given a triplet (r,g,b), computes the position in the histogram 
    unsigned int compute_rgb_bin(float r, float g, float b) const;

    /// @brief Given a value between 0 and 255, computes the bin in the histogram 
    unsigned int find_bin(float ivalue) const;

    unsigned int num_bins;

    /// @brief This is redundant, but saves some computations 
    float bin_size;
};

void write_histograms(const std::vector<Color_histogram> & chs, const std::string & file_name);

void read_histograms(std::vector<Color_histogram> & chs, const std::string & file_name);

/// @}

}

#endif /*KJB_CPP_IMAGE_H */
