/**
 * @file
 * @brief Code for a wrapper class around the C struct KJB_Image.
 * @author Kyle Simek
 * @author Andrew Predoehl
 */
/*
 * $Id: i_image.h 19820 2015-09-24 20:56:35Z predoehl $
 */

#ifndef KJB_CPP_IMAGE_H
#define KJB_CPP_IMAGE_H

//#include "m_cpp/m_concept.h"
#include "i/i_matrix.h"
#include "i/i_float.h"
#include "i/i_float_io.h"
#include "i/i_transform.h"
#include "i/i_arithmetic.h"
#include "i/i_draw.h"
#include "i2/i2_draw_text.h"

// if not for intensity_histogram(), we could remove all 
// dependencies to m_cpp from i_image.h .  Consider moving to 
// i2_cpp.
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <boost/shared_ptr.hpp>
//#include <boost/concept_check.hpp>

//define YES_WE_WANT_GARBAGE_POLICE /* for debugging */

#include <string>
#include <exception>
#include <stdexcept>
#ifdef YES_WE_WANT_GARBAGE_POLICE
#include <sstream>
#endif

namespace kjb
{
    class Matrix;
    class Int_matrix;
    class Vector;

/**
 * @defgroup kjbImageProc Image Processing
 *
 * This group include the Image class and all closely-related classes and
 * functions pertaining to low-level operations on and manipulation of images.
 *
 * @{
 */


// some trivial forward declaration stuff we need
class Image;
Image scale_image(const Image&, double);
Image get_inverted(const Image&);
Image rgb_matrices_to_image(const Matrix&, const Matrix&, const Matrix&);



/**
 * @brief Wrapped version of the C struct KJB_image
 *
 * Due to memory leaks associated with the kjb_display_image function -- leaks
 * that are still unresolved -- this class has some %debug features I've
 * wrapped up under the heading "Garbage Police."  Look for the above macro.
 * If the Garbage Police are turned off, then this is a very thin wrapper on
 * the KJB_image class.  Otherwise there is some static tracking of the number
 * of instances of this class.  This class does not seem to leak; the leaks
 * must be elsewhere (i.e., not my fault).
 */
class Image
{
public:
    typedef kjb_c::KJB_image    Impl_type;
    typedef kjb_c::Pixel        Pixel_type;

protected:
    Impl_type* m_image;

private:
    static const char* BAD_CHANNEL;

    static
    void sort2( int& smaller, int& bigger )
    {
        if ( smaller > bigger )
        {
            std::swap( smaller, bigger );
        }
    }

    void compute_row_col( int index, int* row, int* col ) const
    {
        // Test program was HERE.
        *row = index / get_num_cols();
        *col = index - *row * get_num_cols();
    }

    void compute_row_col_carefully( int index, int* row, int* col ) const
    {
        if ( 0 == get_num_cols() )
        {
            *row = *col = 0;
        }
        else
        {
            // Test program was HERE.
            compute_row_col( index, row, col );
        }
    }

#ifdef YES_WE_WANT_GARBAGE_POLICE
    mutable std::string info;
    static int live_object_counter;
    static int serial_counter;
#endif

    void call_me_in_every_ctor()
    {
#ifdef YES_WE_WANT_GARBAGE_POLICE
        ++live_object_counter;
        ++serial_counter;
        std::ostringstream s;
        s << '(' << serial_counter << ')';
        info = s.str() + info;
#endif
    }



public:


    /// @brief Constants for accessing color channels via number, with at().
    enum RGB_channel { RED, GREEN, BLUE, END_CHANNELS };

    /// @brief Construct image of specified size, defaults to zero by zero.
    Image(int rows = 0, int cols = 0);

    /// @brief Construct grayscale image from c-style matrix
    explicit Image( const kjb_c::Matrix& src );

    /// @brief Construct grayscale image from matrix
    explicit Image( const Matrix& src );

    /// @brief Copy ctor, performs a deep copy (use sparingly)
    Image( const Image& src );

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move ctor
     */
    Image(Image&& src)
        : m_image( nullptr )
    {
        m_image = src.m_image;
        src.m_image = 0;
    }
#endif /* KJB_HAVE_CXX11 */



    /**
     * @brief Read image from a named file
     *
     * This wraps C-library function kjb_read_image_2().
     * @throws KJB_error if the underlying C function returns an error.
     */
    explicit Image(const std::string& fname);

    /// @brief Wrap up an "unsafe" image so it is certain to be destroyed.
    explicit Image( Impl_type* wrap_me );

    /**
     * @brief   Create an image with pixels initialized to the given value.
     */
    Image(int num_rows, int num_cols, int r, int g, int b);

    /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

    /*---------------------------------------------------------------------*
     * "NAMED CONSTRUCTORS"
     *---------------------------------------------------------------------*/

    /**
     * @brief Create an empty image of valid zero pixels, r=g=b=0.
     */
    static
    Image create_zero_image( int rows, int cols )
    {
        return create_initialized_image( rows, cols, 0, 0, 0 );
    }

    /**
     * @brief Create an empty image with pixels initialized to (r, g, b)
     */
    static
    Image create_initialized_image( int rows, int cols, int r, int g, int b );

    /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


    /// @brief Dtor simply wraps kjb_free_image()
    virtual ~Image()
    {
        #ifdef YES_WE_WANT_GARBAGE_POLICE
        fprintf( stderr, "DTOR: %s\n", info.c_str() );
        --live_object_counter;
        #endif

        kjb_c::kjb_free_image(m_image);
    }

    /// @brief Swap the implementation of two images
    void swap( Image& other )
    {
        using std::swap;
        swap( m_image, other.m_image );
    }

    friend void swap(kjb::Image& a, kjb::Image& b)
    {
        a.swap(b);
    }

    /// @brief Scale the image by factor using imagemagick (deprecated).
    /// @deprecated use non-member function scale_image().
    Image& scale(double factor)
    {
        Image i(scale_image(*this, factor));
        swap(i);
        return *this;
    }

    /// @brief Crop this image.
    Image& crop(int r, int c, int num_rows, int num_cols);

    #ifdef YES_WE_WANT_GARBAGE_POLICE
    /// @brief Add to an object's identifying string for debug
    void infocat( const std::string& concat_me ) const { info += concat_me; }
    std::string get_info() const        { return info;                  }
    static int  query_live_counter()    { return live_object_counter;   }
    static int  query_serial_counter()  { return serial_counter;        }
    #endif


    /// @brief Write to a file
    void write(const std::string& fname) const;

    /// @brief Return the number of rows in the image
    int get_num_rows() const
    {
        return m_image->num_rows;
    }

    /// @brief Return the number of columns in the image
    int get_num_cols() const
    {
        return m_image->num_cols;
    }

    /// @brief Return the number of elements (pixels) in the image
    int get_length() const
    {
        return get_num_rows() * get_num_cols();
    }

    /// @brief Deep copy assignment from C-type image.
    Image& operator=(const kjb_c::KJB_image& src);

    /// @brief Deep copy assignment.
    Image& operator=(const Image& src);

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move assignment
     */
    Image& operator=(Image&& other)
    {
        if(this == &other)
            return *this;

        kjb_c::kjb_free_image( m_image );
        m_image = other.m_image;
        other.m_image = 0;
        return *this;
    }
#endif /* KJB_HAVE_CXX11 */

    /// @brief Add an image from this image, in place
    Image& operator+=(const Image& op2);

    /// @brief Subtract an image from this image, in place
    Image& operator-=(const Image& op2);

    /**
     * @brief   Multiply this image by a scalar; i.e., scale
     *          this image in channel space.
     * @return  an lvalue to this matrix
     */
    Image& operator*=(double op2);

    /**
     * @brief   Divide this image by a scalar; i.e., scale
     *          this image in channel space.
     * @return  an lvalue to this matrix
     * @throws  Divide_by_zero if the scalar value is zero
     */
    Image& operator/=(double op2);

    /**
     * @brief Unchecked access of lvalue at given row, column, and RGB_channel
     * @see at() method, which provides bounds checking.
     * @warn The coordinates are row,column NOT x,y !
     */
    float& operator()(int row, int col, int channel)
    {
        // Test program was HERE.
        switch(channel)
        {
        case RED:
            return operator()( row, col ).r;
        case GREEN:
            return operator()( row, col ).g;
        case BLUE:
            return operator()( row, col ).b;
        default:
            throw std::out_of_range("Invalid channel");
        }
    }

    /**
     * @brief Unchecked access of rvalue at given row, column, and RGB_channel
     * @see at() method, which provides bounds checking.
     * @warn The coordinates are row,column NOT x,y !
     */
    float operator()(int row, int col, int channel) const
    {
        switch(channel)
        {
        case RED:
            return operator()( row, col ).r;
        case GREEN:
            return operator()( row, col ).g;
        case BLUE:
            return operator()( row, col ).b;
        default:
            throw std::out_of_range("Invalid channel");
        }
    }

    /**
     * @brief Lvalue pixel access at given row & column, no bounds-checking.
     * @warn The coordinates are row,column NOT x,y !
     */
    Pixel_type& operator()( int row, int col )
    {
        return m_image -> pixels[ row ][ col ];
    }

    /**
     * @brief Rvalue pixel access at given row & column, no bounds-checking.
     * @warn The coordinates are row,column NOT x,y !
     */
    const Pixel_type& operator()( int row, int col ) const
    {
        return m_image -> pixels[ row ][ col ];
    }

    /// @brief Access lvalue at row-major index, without bounds-checking
    Pixel_type& operator()( int index )
    {
        int row, col;
        compute_row_col( index, &row, &col );
        return operator()( row, col );
    }

    /// @brief Access rvalue at row-major index, without bounds-checking.
    const Pixel_type& operator()( int index ) const;

    /// @brief Invert this image; i.e., dark becomes light and vice-versa.
    /// @note This simply wraps a call to kjb_c::ow_invert_image().
    void invert();

    /// @brief Generate an inverted version of this image (deprecated).
    /// @deprecated use non-member function get_inverted(const Image&) instead.
    Image get_inverted() const
    {
        return kjb::get_inverted(*this);
    }

    /**
     * @note    DEPRECATED!! Use to_grayscale_matrix(double, double, double)
     */
    Matrix get_channel(int index) const;

    /**
     * @brief Test whether row, column coordinates are valid
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    void check_bounds( int row, int col ) const;


    /**
     * @brief Access lvalue of pixel channel at row, column coordinates.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    float& at(int row, int col, int channel);
    /**
     * @brief Access rvalue of pixel channel at row, column coordinates.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     * @warn The coordinates are row,column NOT x,y !
     */
    float at(int row, int col, int channel) const;

    /**
     * @brief Access pixel lvalue at row, column coordinates.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     * @warn The coordinates are row,column NOT x,y !
     */
    Pixel_type& at( int row, int col );

    /**
     * @brief Access pixel rvalue at row, column coordinates.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     * @warn The coordinates are row,column NOT x,y !
     */
    const Pixel_type& at( int row, int col ) const;

    /** @brief Access pixel rvalue at row-major index
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    const Pixel_type& at( int index ) const;

    /** @brief Access pixel lvalue at row-major index.
     * @throw Index_out_of_bounds if the coordinates are out of bounds.
     */
    Pixel_type& at( int index );

    /// @brief Access a pointer to the underlying implementation
    const Impl_type* c_ptr() const;

    /// DEPRECATED
    ///@brief Access a pointer to the underlying implementation,
    ///        use with care
    /// @deprecated
    Impl_type* non_const_c_ptr() const
    {
        return m_image;
    }

    /**
     * DEPRECATED
     * @brief set this matrix to point to a different c memory,
     * freeing the previously pointed area
     *
     * @param iimage the new c matrix to be wrapped by this c++ class
     *
     * @deprecated
     */
    void set_c_ptr(Impl_type * iimage)
    {
#ifdef YES_WE_WANT_GARBAGE_POLICE
       fprintf( stderr, "DTOR: %s\n", info.c_str() );
       --live_object_counter;
#endif

       kjb_c::kjb_free_image(m_image);
       m_image = iimage;
    }

    /// @brief Convert this image to a single matrix by simple averaging
    ///        of the pixels.
    Matrix to_grayscale_matrix() const;

    /// @brief Convert this image to a single matrix by weighted averaging
    ///        of the pixels.
    ///
    /// @warning this routine is sensitive to the value of the
    /// Pixel.extra.invalid
    /// field, and will return a matrix full of -99s (DBL_MISSING) if that
    /// field does not contain value VALID_PIXEL.  However, this field is a
    /// union with alpha values, so you can't use alpha in such images.
    ///
    /// @param  r_w     Weight of red channel.
    /// @param  g_w     Weight of green channel.
    /// @param  b_w     Weight of blue channel.
    ///
    Matrix to_grayscale_matrix(double r_w, double g_w, double b_w) const;

    /// @brief Convert this image to a single matrix by simple averaging
    ///        of the pixels.
    ///
    /// @note  Not sure if this should exist. Perhaps to_grayscale_matrix is
    ///        enough? Might disappear.
    Matrix to_channel_matrix(RGB_channel channel) const;


    /**
     * @brief Convert this image to a single matrix containing RGBA values.
     *        The first eight least significant bits are red, the next least
     *        least significant octet is green, and so on.
     */
    Int_matrix to_color_matrix(double scale = 1.0) const;

    /**
     * @brief Construct an image from a matrix containing RGBA values.
     *        Each element of matrix m is an integer which consists of ...
     * @todo finish the documentation here
     */
    void from_color_matrix(const Int_matrix& m);

    /**
     * @brief Contruct an image from three matrices representing red, green,
     *        blue channels.
     * @throws KJB_error (possibly) if matrices not the same size (see below).
     * @deprecated Use rgb_matrices_to_image in new code.
     */
    void from_color_matrices(
        const Matrix& red,
        const Matrix& green,
        const Matrix& blue
    );

    /**
     * @brief   %Computes the intensity histogram of a region --
     *          given by a range of pairs -- of this image.
     *
     * @tparam  InputIterator   An iterator whose value_type is
     *                          a pair.
     */
    template<class InputIterator>
    Vector intensity_histogram(InputIterator first, InputIterator last) const;

    /**
     * @brief   Draw a point on the image
     *
     *          Draw a point on the image in the given
     *          location, with the given width and pixel
     *          color.
     */
    void draw_point(
        int row,      ///< Index of the first filled row of the rectangle
        int col,      ///< Index of the first filled col. of rectangle
        int width,    ///< Width of point
        Pixel_type p  ///< Pixel (color) with which to fill
    );

    /**
     * @brief  Draw a line segment on the image
     */
    void draw_line_segment(
        int row_from, ///< Index of starting row
        int col_from, ///< Index of starting column
        int row_to,   ///< Index of ending row
        int col_to,   ///< Index of ending column
        int width,    ///< Width of segment
        Pixel_type p  ///< Pixel (color) with which to fill
    );

    /**
     * @brief  Draw an arrow from src to dest
     * @param src source location as an (x,y) value, within image boundaries
     * @param dest destination location as an (x,y) value, within image bounds
     * @param pixel color of the arrow
     * @warning src and dest have (x,y) semantics, where x is column, y is row.
     */
    void draw_arrow
    (
        const Vector& src,
        const Vector& dest,
        const Image::Pixel_type pixel
    );

    /**
     * @brief  Draw a polyline on the image, i.e., a chain of line segments.
     *
     * Sequence is provided as an iterator range, which points to vectors
     * representing (u,v) coordinates (i.e., (col, row) coordinates).
     * Suitable vector types are any that implement the kjb::SimpleVector
     * concept.  This includes kjb::Vector and std::vector<double>.
     */
    template <class Iterator>
    void draw_polyline(
            Iterator begin,
            Iterator end,
            int width,
            Pixel_type p  ///< Pixel (color) with which to fill
    )
    {
        /*
        typedef typename std::iterator_traits<Iterator>::value_type value_type;
        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<value_type>));
        */
        if(begin == end) return; // empty range
        for (Iterator it2 = begin, it = it2++; it2 != end; ++it, ++it2)
        {
            draw_line_segment((*it)[1], (*it)[0], (*it2)[1], (*it2)[0], width, p);
        }
    }

    /** @brief Draw a solid, axis-aligned rectangle on the image
     *
     * This would be more elegant if the first row were filled and the last row
     * was just BEYOND the fill, but we don't do it that way.  The first row,
     * last row, first column, and last column are all affected.  If the
     * first and last indices are not increasing they are silently swapped.
     *
     * This performs clipping silently.  If any of the row or column indices
     * are out of bounds, nothing bad happens; the box is simply clipped.
     *
     * The "aa" is for "axis-aligned."
     */
    void draw_aa_rectangle(
        int first_row,      ///< Index of the first filled row of the rectangle
        int first_col,      ///< Index of the first filled col. of rectangle
        int last_row,       ///< Index of the last filled row of the rectangle
        int last_col,       ///< Index of the last filled col. of the rectangle
        Pixel_type p        ///< Pixel (color) with which to fill
    );

    /** @brief Draw the outline of an, axis-aligned rectangle on the image
     *
     * This would be more elegant if the first row were filled and the last row
     * was just BEYOND the fill, but we don't do it that way.  The first row,
     * last row, first column, and last column are all affected.  If the
     * first and last indices are not increasing they are silently swapped.
     *
     * This performs clipping silently.  If any of the row or column indices
     * are out of bounds, nothing bad happens; the box is simply clipped.
     *
     * The "aa" is for "axis-aligned."
     */
    void draw_aa_rectangle_outline(
        int first_row,      ///< Index of the first filled row of the rectangle
        int first_col,      ///< Index of the first filled col. of rectangle
        int last_row,       ///< Index of the last filled row of the rectangle
        int last_col,       ///< Index of the last filled col. of the rectangle
        Pixel_type p        ///< Pixel (color) with which to fill
    );

    /**
     * @brief Draw a circle on the image
     *
     * Draws a circle (an empty circle .... not a disk!) of the specified radius
     * and line width on the image, with center at (center_row, center_col).
     *
     * @see draw_disk
     */
    void draw_circle(
        int center_row,     ///< Index of the center row of the circle
        int center_col,     ///< Index of the center column of the circle
        int radius,         ///< Index of the radius of the circle
        int line_width,     ///< approx. thickness of the circle (0 = hairline)
        Pixel_type p        ///< Pixel (color) with which to draw
    );


    /**
     * @brief draw a disk (a filled circle) on the image, of a given color.
     * @see draw_circle
     *
     * Clipping occurs silently:  disk need not lie fully within the image.
     */
    void draw_disk(
        int center_row,     ///< Index of the center row of the circle
        int center_col,     ///< Index of the center column of the circle
        int radius,         ///< Index of the radius of the circle
        Pixel_type p        ///< Pixel (color) with which to draw
    );


    /**
     * @brief Draw the text on the image
     * @see kjb_c::image_draw_text_center
     * @returns kjb_c::ERROR or kjb_c::NO_ERROR to indicate success or failure.
     *
     * Draws the text on the image with the center at location (row, col).
     */
    int draw_text_center(
        int row,
        int col,
        const std::string& text,
        const std::string& font_file = "times14"
    );

    /**
     * @brief Draw the text on the image
     * @see kjb_c::image_draw_text_top_left
     * @returns kjb_c::ERROR or kjb_c::NO_ERROR to indicate success or failure.
     *
     * Draws the text on the image with the top-left at location (row, col)
     */
    int draw_text_top_left(
        int row,
        int col,
        const std::string& text,
        const std::string& font_file = "times14"
    );

    /**
     * @brief Overlay another image on this image, with an optional offset
     *
     * This just forwards a call to kjb_c::image_draw_image().
     *
     * @param overlay   Points to an image that will be drawn onto this image
     * @param row       Index of the row of this image where the top of the
     *                  overlay will be drawn
     * @param col       Index of the column of this image where the left edge
     *                  of the overlay will be drawn
     * @param scale     Optional shrink factor of the overlay image; if equal
     *                  to 1, the overlay is drawn full-size.  If equal to 2,
     *                  the overlay is drawn at half-size, etc.  This number
     *                  must be positive.
     *
     * Clipping occurs silently:  the whole overlay need not fit; nor does row
     * or col need to be within the bounds of this image.  For example, you
     * could crop the top edge of overlay by using a negative value for row.
     */
    void draw_image(
        const kjb_c::KJB_image* overlay,
        int row = 0,
        int col = 0,
        int scale = 1
    );

    /// @brief This is an overload of draw_image() taking a C-style pointer.
    void draw_image(
        const Image& overlay,
        int row = 0,
        int col = 0,
        int scale = 1
    );

    /**
     * @brief Show the image and return an image number (for closing later).
     *
     * @see close_displayed_image()
     *
     * This will try to show the image on the screen, in a window.  The window
     * title optionally may be (partially) specified with the 'title'
     * parameter.  Double quotes in the title will be altered, sorry.
     * The display will persist even after the Image object is destroyed; this
     * is a feature, not a bug.  You can kill it with close_displayed_image().
     *
     * @bug Whenever this function is called, a memory leak is reported.
     */
    int display( const std::string& title = std::string() ) const;

    /// @brief return copy of the "flags" field of underlying KJB_image object
    int get_flags() const
    {
        return m_image -> flags;
    }

    /// @brief set "flags" field of underlying KJB_image; return previous flags
    int set_flags(int new_flags)
    {
        int old_flags = m_image -> flags;
        m_image -> flags = new_flags;
        return old_flags;
    }
};

/**
 * Convenience typedef for boost::shared_ptr<Image>.
 */
typedef boost::shared_ptr<Image> Image_ptr;


/**
 * @brief Scale image size by factor, i.e., enlarge or shrink.
 * @param i input image to scale
 * @param factor the enlargement factor to apply, e.g., 2 means to double the
 *               width and height.  0.5 means to halve the width and height.
 * @return The scaled image.
 * @throws KJB_error if the scaling fails
 *
 * If the enlargement factor is in the range 0.9999 to 1.0001, this function
 * just returns a copy of the input, without any scaling.
 */
Image scale_image(const Image& i, double factor);



Matrix to_grayscale_matrix(const Image& i);

inline Image get_inverted(const Image& i)
{
    Image res(i);
    res.invert();
    return res;
}


//---------------------------------------------------------
// ARITHMETIC OPERATORS
//---------------------------------------------------------

/**
 * @brief   Scale an image in channel space, yielding a new
 *          image.
 */
inline
Image operator*(const Image& op1, double op2)
{
    return Image(op1) *= op2;
}

/**
 * @brief   Scale an image in channel space, yielding a new
 *          image.
 * @throws  Divide_by_zero if the given value is zero.
 *
 */
inline
Image operator/(const Image& op1, double op2)
{
    return Image(op1) /= op2;
}

/**
 * @brief   Add two images.
 */
inline
Image operator+(const Image& op1, const Image& op2)
{
    return Image(op1) += op2;
}

/**
 * @brief   Subtract two images.
 */
inline
Image operator-(const Image& im1, const Image& im2)
{
    return Image(im1) -= im2;
}


/// @brief set Image flag, to indicate that the 'alpha' channel is meaningful.
inline void enable_transparency(Image& i)
{
    i.set_flags(i.get_flags() | HAS_ALPHA_CHANNEL);
}


/// @brief clear Image flag, to indicate that the 'alpha' channel is not meaningful.
inline void disable_transparency(Image& i)
{
    i.set_flags(i.get_flags() & ~HAS_ALPHA_CHANNEL);
}


/// @brief test the Image flag, returning true if the 'alpha' channel is meaningful.
inline bool is_transparency_enabled(const Image& i)
{
    return i.get_flags() & HAS_ALPHA_CHANNEL;
}


//---------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE MEMBERS
//---------------------------------------------------------

template<class InputIterator>
Vector Image::intensity_histogram(InputIterator first, InputIterator last) const
{
    Matrix I = to_grayscale_matrix();
    Vector hist(static_cast<int>(256), static_cast<double>(get_length() / 256.0));

    for(; first != last; first++)
    {
        int val = static_cast<int>(I(first->first, first->second));
        hist[val] += 0.7;

        if(val != 0)
        {
            hist[val - 1] += 0.15;
        }

        if(val != 255)
        {
            hist[val + 1] += 0.15;
        }
    }

    return hist;
}


/// @}

} //namespace kjb

#endif /*KJB_CPP_IMAGE_H */
