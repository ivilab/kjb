/**
 * @file
 * @brief implementation of wrapper on KJB_Image
 *
 * @author Ernesto Brau
 * @author Sumin Byeon
 * @author Luca Del Pero
 * @author Jinyan Guan
 * @author Andy Predoehl
 * @author Kyle Simek
 */

/* $Id: i_image.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "i/i_float_io.h"
#include "l_cpp/l_exception.h"
#include "i_cpp/i_image.h"
#include "m_cpp/m_matrix.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_int_matrix.h"

#include <string>
#include <cmath>

namespace kjb {


#ifdef YES_WE_WANT_GARBAGE_POLICE
int Image::live_object_counter = 0;
int Image::serial_counter = 0;
#endif

const char* Image::BAD_CHANNEL = "Invalid channel number";

Image::Image(int rows, int cols)
    : m_image( 0 )
{
    ETX( kjb_c::get_target_image( &m_image, rows, cols ) );
    call_me_in_every_ctor();
}

Image::Image( const kjb_c::Matrix& src ) :
    m_image(0)
{
    ETX( kjb_c::matrix_to_bw_image(&src, &m_image) );
    call_me_in_every_ctor();
}

Image::Image( const Matrix& src ) :
    m_image(0)
{
    ETX( kjb_c::matrix_to_bw_image(src.get_c_matrix(), &m_image) );
    call_me_in_every_ctor();
}

Image::Image( const Image& src )
    : m_image( 0 )
        #ifdef YES_WE_WANT_GARBAGE_POLICE
        , info( std::string("(COPY-OF") + src.info + std::string(")") )
        #endif
{
    ETX( kjb_c::kjb_copy_image( &m_image, src.m_image ) );
    call_me_in_every_ctor();
}

Image::Image(const std::string& fname)
    : m_image(0)
{
    ETX(kjb_read_image_2(&m_image, fname.c_str()));
    call_me_in_every_ctor();
}

Image::Image( Impl_type* wrap_me )
    : m_image( wrap_me )
{
    call_me_in_every_ctor();
}

Image::Image(int num_rows, int num_cols, int r, int g, int b) :
    m_image(0)
{
    ETX(kjb_c::get_initialized_image_2(&m_image, num_rows, num_cols, r, g, b));
    call_me_in_every_ctor();
}

Image Image::create_initialized_image( int rows, int cols, int r, int g, int b )
{
    // suddenly I'm worried this will leak
    Image result( rows, cols );
    if ( 0 < rows && 0 < cols )
    {
        ETX( kjb_c::get_initialized_image_2( &result.m_image, rows, cols, r, g, b ) );
    }
    return result;
}

/**
 * @brief Write to a file
 * @param fname Filename to write to.  Include a format suffix, please!
 * @throws KJB_error, if anything goes wrong in kjb_write_image().
 *
 * This wraps C function kjb_c::kjb_write_image(), and infers the proper
 * output format from the filename suffix; or it uses a Sun format if no suffix
 * is recognized.
 */
void Image::write(const std::string& fname) const
{
    ETX(kjb_c::kjb_write_image(m_image, fname.c_str()));
}

/// @brief Crop this image.
Image& Image::crop(int r, int c, int num_rows, int num_cols)
{
    if(r == 0 && c == 0
              && num_rows == get_num_rows()
              && num_cols == get_num_cols())
    {
        return *this;
    }

    Impl_type* im = NULL;
    ETX(kjb_c::get_image_window(&im, m_image, r, c, num_rows, num_cols));
    Image i(im);
    swap(i);
    return *this;
}

/// @brief Deep copy assignment from C-type image.
Image& Image::operator=(const kjb_c::KJB_image& src)
{
    if (m_image != & src)
    {
        ETX(kjb_c::kjb_copy_image(&m_image, &src));
    }

    return *this;
}

/// @brief Deep copy assignment.
Image& Image::operator=(const Image& src)
{
    return operator=(*(src.m_image));
}

/// @brief Add an image to this image, in place
Image& Image::operator+=(const Image& op2)
{
    ETX(kjb_c::ow_add_images(m_image, op2.m_image));
    return *this;
}

/// @brief Subtract an image from this image, in place
Image& Image::operator-=(const Image& op2)
{
    ETX(kjb_c::ow_subtract_images(m_image, op2.m_image));
    return *this;
}

/**
 * @brief   Multiply this image by a scalar; i.e., scale
 *          this image in channel space.
 * @return  an lvalue to this matrix
 */
Image& Image::operator*=(double op2)
{
    ETX(kjb_c::ow_scale_image(m_image, op2));
    return *this;
}

/**
 * @brief   Divide this image by a scalar; i.e., scale
 *          this image in channel space.
 * @return  an lvalue to this matrix
 * @throws  Divide_by_zero if the scalar value is zero
 */
Image& Image::operator/=(double op2)
{
    if( 0 == op2 )
    {
        KJB_THROW(Divide_by_zero);
    }

    return operator*=( 1.0 / op2 );
}

void Image::draw_aa_rectangle(
    int first_row,
    int first_col,
    int last_row,
    int last_col,
    Pixel_type p
)
{
    sort2( first_row, last_row );
    sort2( first_col, last_col );

    // Total clipping . . .
    if (    last_row < 0                // . . . by top edge
        ||  first_row >= get_num_rows() // . . . by bottom edge
        ||  last_col < 0                // . . . by left edge
        ||  first_col >= get_num_cols() // . . . by right edge
       )
    {
        return;
    }

    // Partial clipping
    first_row = std::max(first_row, 0);
    last_row = std::min(last_row, get_num_rows()-1);
    first_col = std::max(first_col, 0);
    last_col = std::min(last_col, get_num_cols()-1);

    // Fill the rectangle.
    for( int row = first_row; row <= last_row; ++row )
    {
        for( int col = first_col; col <= last_col; ++col )
        {
            at( row, col ) = p;
        }
    }
}


void Image::draw_aa_rectangle_outline(
    int first_row,
    int first_col,
    int last_row,
    int last_col,
    Pixel_type p
)
{
    // top and bottom edges of box
    draw_aa_rectangle(first_row, first_col, first_row, last_col, p);
    draw_aa_rectangle( last_row, first_col,  last_row, last_col, p);

    // left, right edges of box
    draw_aa_rectangle(first_row, first_col, last_row, first_col, p);
    draw_aa_rectangle(first_row,  last_col, last_row,  last_col, p);
}




Int_matrix Image::to_color_matrix(double scale) const
{
    int rows = get_num_rows();
    int cols = get_num_cols();

    Int_matrix m(rows, cols);

    for(int i=0; i < rows; ++i )
    {
        for(int j=0; j < cols; ++j )
        {
            int red = static_cast< int >( m_image -> pixels[i][j].r * scale ),
                grn = static_cast< int >( m_image -> pixels[i][j].g * scale ),
                blu = static_cast< int >( m_image -> pixels[i][j].b * scale ),
                rgba;
            rgba = blu & 0xFF;
            rgba <<= 8;
            rgba |= grn & 0xFF;
            rgba <<= 8;
            rgba |= red & 0xFF;
            m(i, j) = rgba;
        }
    }

    return m;
}


void Image::from_color_matrix(const Int_matrix& m)
{
    int rows = m.get_num_rows();
    int cols = m.get_num_cols();

    ETX( kjb_c::get_initialized_image_2( &m_image, rows, cols, 0, 0, 0 ) );

    for(int i=0; i < rows; ++i )
    {
        for(int j=0; j < cols; ++j )
        {
            register int rgba = m(i, j);
            /*
            uint8_t red = rgba & 0xFF;
            uint8_t green = (rgba >> 8) & 0xFF;
            uint8_t blue = (rgba >> 16) & 0xFF;

            m_image->pixels[i][j].r = red;
            m_image->pixels[i][j].g = green;
            m_image->pixels[i][j].b = blue;
            */
            m_image->pixels[i][j].r = rgba & 0xFF;
            rgba >>= 8;
            m_image->pixels[i][j].g = rgba & 0xFF;
            rgba >>= 8;
            m_image->pixels[i][j].b = rgba & 0xFF;
        }
    }
}


/// @brief Access rvalue at row-major index, without bounds-checking.
const Image::Pixel_type& Image::operator()( int index ) const
{
    // hit KJB(UNTESTED_CODE());
    int row, col;
    compute_row_col( index, &row, &col );
    return operator()( row, col );
}

void Image::invert()
{
    ETX(kjb_c::ow_invert_image(m_image));
}

Matrix Image::get_channel(int index) const
{
    kjb_c::Matrix *r = 0, *g = 0, *b = 0;
    KJB(ETX(image_to_rgb_matrices(m_image, &r, &g, &b)));
    kjb::Matrix wrapped_r(r), wrapped_g(g), wrapped_b(b);

    switch(index)
    {
        case RED:   return wrapped_r;
        case GREEN: return wrapped_g;
        case BLUE:  return wrapped_b;
        default:    KJB_THROW_2(Illegal_argument, "Channel index must be "
                                    "0, 1, or 2 (or RED, GREEN, or BLUE).");
    }
}


void Image::check_bounds( int row, int col ) const
{
    // hit KJB(UNTESTED_CODE());
    // No need to test whether m_image equals NULL; this will catch it.
    if (        row < 0 || get_num_rows() <= row
            ||  col < 0 || get_num_cols() <= col )
    {
        // hit KJB(UNTESTED_CODE());
        KJB_THROW(Index_out_of_bounds);
    }
}


/**
 * @brief Access lvalue of pixel channel at row, column coordinates.
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 */
float& Image::at(int row, int col, int channel)
{
    //hit KJB(UNTESTED_CODE());
    check_bounds( row, col );
    return operator()(row, col, channel);
}

/**
 * @brief Access rvalue of pixel channel at row, column coordinates.
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 * @warn The coordinates are row,column NOT x,y !
 */
float Image::at(int row, int col, int channel) const
{
    // hit KJB(UNTESTED_CODE());
    check_bounds( row, col );
    return operator()(row, col, channel);
}

/**
 * @brief Access pixel lvalue at row, column coordinates.
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 * @warn The coordinates are row,column NOT x,y !
 */
Image::Pixel_type& Image::at( int row, int col )
{
    // hit KJB(UNTESTED_CODE());
    check_bounds( row, col );
    return operator()( row, col );
}

/**
 * @brief Access pixel rvalue at row, column coordinates.
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 * @warn The coordinates are row,column NOT x,y !
 */
const Image::Pixel_type& Image::at( int row, int col ) const
{
    // hit KJB(UNTESTED_CODE());
    check_bounds( row, col );
    return operator()( row, col );
}

/** @brief Access pixel rvalue at row-major index
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 */
const Image::Pixel_type& Image::at( int index ) const
{
    // hit KJB(UNTESTED_CODE());
    int row, col;
    compute_row_col_carefully( index, &row, &col );
    check_bounds( row, col );
    return operator()( row, col );
}

/** @brief Access pixel lvalue at row-major index.
 * @throw Index_out_of_bounds if the coordinates are out of bounds.
 */
Image::Pixel_type& Image::at( int index )
{
    // hit KJB(UNTESTED_CODE());
    int row, col;
    compute_row_col_carefully( index, &row, &col );
    check_bounds( row, col );
    return operator()( row, col );
}

/// @brief Access a pointer to the underlying implementation
const Image::Impl_type* Image::c_ptr() const
{
    return m_image;
}

Matrix Image::to_grayscale_matrix() const
{
    kjb_c::Matrix* mat = NULL;
    ETX(kjb_c::image_to_matrix(m_image, &mat));
    return Matrix(mat);
}

Matrix Image::to_grayscale_matrix(double r_w, double g_w, double b_w) const
{
    kjb_c::Matrix* mat = NULL;
    ETX(kjb_c::image_to_matrix_2(m_image, r_w, g_w, b_w, &mat));
    return Matrix(mat);
}

Matrix Image::to_channel_matrix(RGB_channel channel) const
{
    return to_grayscale_matrix(channel == RED ? 1.0 : 0.0,
                               channel == GREEN ? 1.0 : 0.0,
                               channel == BLUE ? 1.0 : 0.0);
}

void Image::from_color_matrices(
    const Matrix& red,
    const Matrix& green,
    const Matrix& blue
)
{
    Image j(rgb_matrices_to_image(red, green, blue));
    swap(j);
}

void Image::draw_point(
    int row,      ///< Index of the first filled row of the rectangle
    int col,      ///< Index of the first filled col. of rectangle
    int width,    ///< Width of point
    Image::Pixel_type p  ///< Pixel (color) with which to fill
)
{
    ETX( kjb_c::image_draw_point_2(
                m_image,
                row,
                col,
                width,
                p.r,
                p.g,
                p.b
        ) );
}

void Image::draw_line_segment(
    int row_from, ///< Index of starting row
    int col_from, ///< Index of starting column
    int row_to,   ///< Index of ending row
    int col_to,   ///< Index of ending column
    int width,    ///< Width of segment
    Pixel_type p  ///< Pixel (color) with which to fill
)
{
    ETX( kjb_c::image_draw_segment_2(
                m_image,
                row_from,
                col_from,
                row_to,
                col_to,
                width,
                static_cast< int >(p.r),
                static_cast< int >(p.g),
                static_cast< int >(p.b)
        ) );
}

void Image::draw_circle(
    int center_row,     ///< Index of the first filled row of the rectangle
    int center_col,     ///< Index of the first filled col. of rectangle
    int radius,         ///< Index of the last filled row of the rectangle
    int line_width,     ///< Index of the last filled col. of the rectangle
    Pixel_type p        ///< Pixel (color) with which to fill
)
{
    kjb_c::image_draw_circle_2(m_image, center_row, center_col, radius, line_width, p.r, p.g, p.b);
}

void Image::draw_disk(
    int center_row,     ///< Index of the center row of the circle
    int center_col,     ///< Index of the center column of the circle
    int radius,         ///< Index of the radius of the circle
    Pixel_type p        ///< Pixel (color) with which to draw
)
{
    ETX(image_draw_disk_2(m_image, center_row, center_col, radius,
                                                        p.r, p.g, p.b));
}

int Image::draw_text_center(
    int row,
    int col,
    const std::string& text,
    const std::string& font_file
)
{
    return kjb_c::image_draw_text_center(m_image, text.c_str(),
                                                  row, col, font_file.c_str());
}

int Image::draw_text_top_left(
    int row,
    int col,
    const std::string& text,
    const std::string& font_file
)
{
    return kjb_c::image_draw_text_top_left(m_image, text.c_str(),
                                                  row, col, font_file.c_str());
}

void Image::draw_image(
    const kjb_c::KJB_image* overlay,
    int row,
    int col,
    int scale
)
{
    ETX( kjb_c::image_draw_image( m_image, overlay, row, col, scale ) );
}

void Image::draw_image(
    const Image& overlay,
    int row,
    int col,
    int scale
)
{
    draw_image( overlay.m_image, row, col, scale );
}

int Image::display( const std::string& title ) const
{
    if ( m_image->num_rows == 0 || m_image->num_cols == 0 )
    {
        KJB_THROW_2( Illegal_argument, "An empty image cannot be displayed" );
    }
    return kjb_c::kjb_display_image( m_image, title.c_str() );
}


void Image::draw_arrow
(
    const Vector& src,
    const Vector& dest,
    Pixel_type pixel
)
{
    Vector dir(dest - src);
    double mag = dir.magnitude();

    Vector l(0.8 * mag, -0.2 * mag);
    Vector r(0.8 * mag, 0.2 * mag);

    double theta = atan2(dir[1], dir[0]);

    Matrix R(2, 2);
    R(0, 0) = cos(theta); R(0, 1) = -sin(theta);
    R(1, 0) = sin(theta); R(1, 1) = cos(theta);

    Vector left = src + R*l;
    Vector right = src + R*r;

    draw_line_segment(src(1), src(0), dest(1), dest(0), 1.0, pixel);
    // left arrow half
    draw_line_segment(dest(1), dest(0), left(1), left(0), 1.0, pixel);
    // right arrow half
    draw_line_segment(dest(1), dest(0), right(1), right(0), 1.0, pixel);
}

Matrix to_grayscale_matrix(const Image& i)
{
    return i.to_grayscale_matrix();
}

Image scale_image(const Image& i, double factor)
{
    if(std::fabs(factor - 1.0) < 0.0001) return i;

    kjb_c::KJB_image* c_image = NULL;
    ETX(kjb_c::scale_image_size(&c_image, i.c_ptr(), factor));
    return Image(c_image);
}


/**
 * @brief Contruct an image from three matrices representing red, green,
 *        blue channels.
 * @throws KJB_error, possibly, if matrices not the same size (see below).
 *
 * This just wraps a call to kjb_c::rgb_matrices_to_image().
 * If the inputs are not all identically sized, the bug handler is called.
 * If the bug handler returns, a KJB_error is thrown.
 *
 * Note that if you want your Image to behave like with 8-bit channel values,
 * then you are obliged to scale the input matrices to the range
 * 0.0 to 255.0.
 */
Image rgb_matrices_to_image(
    const Matrix& red_channel,
    const Matrix& green_channel,
    const Matrix& blue_channel
)
{
    kjb_c::KJB_image* ip = 0;
    int rc = kjb_c::rgb_matrices_to_image(
        red_channel.get_c_matrix(),
        green_channel.get_c_matrix(),
        blue_channel.get_c_matrix(),
        &ip
    );
    Image j(ip);
    ETX(rc);
    return j;
}


}
