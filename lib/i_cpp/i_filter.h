/**
 * @file
 * @brief Code for a wrapper class around the C struct KJB_Filter.
 * @author Ernesto Brau
 */
/*
 * $Id: i_filter.h 18347 2014-12-03 16:58:53Z jguan1 $
 */

#ifndef KJB_CPP_FILTER_H
#define KJB_CPP_FILTER_H

#include "m_cpp/m_matrix.h"
#include "l_cpp/l_exception.h"
#include "i_cpp/i_image.h"
#include <string>

namespace kjb 
{

/**
 * @addtogroup kjbImageProc
 * @{
 */

/**
 * @brief Filter class.
 *
 * This class represents an image filter. It is meant to be used
 * to convolve with other filters and with images. The reason I made
 * this class (as opposed to simply using a Matrix) is to be able
 * to overload the * operator to use as a convolution. More generally,
 * a filter is not really a matrix, but a signal, so it should not
 * have the functionality of a matrix.
 */
class Filter
{
public:

    typedef Matrix::Value_type Value_type;

    /// @brief Construct filter of specified size, defaults to zero by zero.
    Filter(int rows, int cols)
        : m_kernel(rows % 2 != 0 ? rows : rows + 1, cols % 2 != 0 ? cols : cols + 1)
    {}

    /// @brief Construct filter from matrix
    Filter(const Matrix& src) :
        m_kernel(src)
    {}

    /// @brief Read filter from a named file
    Filter(const char* fname) 
        : m_kernel(fname)
    {}

    /// @brief Read filter from a named file
    Filter(const std::string& fname) 
        : m_kernel(fname.c_str())
    {}

    /// @brief Copy ctor, performs a deep copy
    Filter(const Filter& src)
        : m_kernel(src.m_kernel)
    {}

    /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

    /// @brief Destroy filter's memory.
    ~Filter()
    {}

    /// @brief Swap the implementation of two filters
    void swap(Filter& other)
    {
        using std::swap;
        swap(m_kernel, other.m_kernel);
    }

    /// @brief Return the number of rows in the filter
    int get_num_rows() const
    {
        return m_kernel.get_num_rows();
    }

    /// @brief Return the number of columns in the filter
    int get_num_cols() const
    {
        return m_kernel.get_num_cols();
    }

    /// @brief Assignment of a matrix to a filter.
    Filter& operator=(const kjb::Matrix& src)
    {
        if(&m_kernel != &src)
        {
            m_kernel = src;
        }

        return *this;
    }

    /// @brief Deep copy assignment.
    Filter& operator=(const Filter& src)
    {
        if(this != &src)
        {
            m_kernel = src.m_kernel;
        }

        return *this;
    }

    /// @brief Lvalue value access at given row & column, no bounds-checking.
    Value_type& operator()(int row, int col)
    {
        // hit KJB(UNTESTED_CODE());
        return m_kernel(row, col);
    }

    /// @brief Rvalue value access at given row & column, no bounds-checking.
    Value_type operator()(int row, int col) const
    {
        // hit KJB(UNTESTED_CODE());
        return m_kernel(row, col);
    }

    /**
     * @brief Test whether row, column coordinates are valid
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    void check_bounds(int row, int col) const
    {
        // hit KJB(UNTESTED_CODE()); 
        if(row < 0 || get_num_rows() <= row || col < 0 || get_num_cols() <= col)
        {
            // hit KJB(UNTESTED_CODE());
            KJB_THROW(Index_out_of_bounds);
        }
    }

    /** @brief Access value lvalue at row, column coordinates.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    Value_type& at(int row, int col)
    {
        // hit KJB(UNTESTED_CODE());
        check_bounds(row, col);
        return operator()(row, col);
    }

    /** @brief Access value rvalue at row, column coordinates.
     *  @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    Value_type at(int row, int col) const
    {
        // hit KJB(UNTESTED_CODE());
        check_bounds(row, col);
        return operator()(row, col);
    }

    /// @brief Write to a file
    void write(std::string fname)
    {
        m_kernel.write(fname.c_str());
    }

    friend Image operator*(const Image&, const Filter&);
    friend Matrix operator*(const Matrix&, const Filter&);

private:
    Matrix m_kernel;
};

/** @brief   Create a Gaussian filter with given sigma and size. */
Filter gaussian_filter(double sigma, int size);

/** @brief   Create a Gaussian filter with given sigma. */
inline Filter gaussian_filter(double sigma)
{
    return gaussian_filter(sigma, 6 * static_cast<int>(sigma) + 1);
}

/** @brief   Create a Laplacian of Gaussian filter. */
Filter laplacian_of_gaussian_filter(int size, double sigma);

/** @brief   Convolve an image with a filter */
Image operator*(const Image& image, const Filter& kernel);

/// @brief this wraps C function kjb_c::gauss_sample_image (q.v.).
Image gauss_sample_image(const Image& in, int resolution, double sigma);

/// @}

} // namespace kjb

namespace std {

template<>
inline void swap(kjb::Filter& a, kjb::Filter& b) 
{
    a.swap(b);
}

} // namespace std

#endif /*KJB_CPP_FILTER_H */

