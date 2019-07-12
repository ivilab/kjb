/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test for the methods of kjb::Image using Matrix and Int_matrix.
 *
 * kjb::Image gives an interface to build an image from three Matrix objects,
 * (R, G, B) and to decompose an image into them.  Obviously those two
 * operations should be complementary.  Likewise one can turn an image whose
 * pixels are integer valued in the R,G,B channels in the range 0-255 into an
 * Int_matrix and vice versa.  Those operations should be complementary.
 * So this test generates a random image, and tests that the Matrix and
 * Int_matrix interfaces are complementary.
 */
/*
 * $Id: test_mat_comp.cpp 18292 2014-11-25 23:47:42Z ksimek $
 */

#include <l/l_def.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>
#include <l_cpp/l_int_matrix.h>

#include <iostream>

namespace {

int random_byte()
{
    return 255 * kjb_c::kjb_rand();
}

bool are_different_pix( const kjb::PixelRGBA& p1, const kjb::PixelRGBA& p2 )
{
    return      p1.r != p2.r
            ||  p1.g != p2.g
            ||  p1.b != p2.b;
    // other fields are ignored
}


bool different( const kjb::Image& im1, const kjb::Image& im2 )
{
    const int rows = im1.get_num_rows(), cols = im1.get_num_cols();

    if ( rows != im2.get_num_rows() || cols != im2.get_num_cols() )
    {
        std::cout << "size mismatch\n";
        return true;
    }

    for ( int row = 0; row < rows; ++row )
    {
        for ( int col = 0; col < cols; ++col )
        {
            if ( are_different_pix( im1.at( row, col ), im2.at( row, col ) ) )
            {
                std::cout << "row=" << row << ", col=" << col
                        << " orig there rgb: "
                        << im1(row,col).r << ' '
                        << im1(row,col).g << ' '
                        << im1(row,col).b
                        << " copy there rgb: "
                        << im2(row,col).r << ' '
                        << im2(row,col).g << ' '
                        << im2(row,col).b << '\n';
                return true;
            }
        }
    }
    return false;
}


}


int main()
{
    const int EDGE = 512;
    kjb_c::kjb_init();

    kjb::Image img( EDGE, EDGE, 0,0,0 );

    for ( int row = 0; row < EDGE; ++row )
    {
        for ( int col = 0; col < EDGE; ++col )
        {
            kjb::PixelRGBA ppp( random_byte(), random_byte(), random_byte() );
            img( row, col ) = ppp;
            // This is required to make to_grayscale_matrix succeed:
            img( row, col ).extra.invalid.pixel = VALID_PIXEL;
        }
    }

    const kjb::Int_matrix ma1( img.to_color_matrix() );

    kjb::Image im2;
    im2.from_color_matrix( ma1 );

    // floating point comparison -- danger!
    if ( different( img, im2 ) )
    {
        std::cout << "fail at " << __LINE__ << '\n';
        return EXIT_FAILURE;
    }

    const kjb::Matrix   reds( img.to_grayscale_matrix(1,0,0) ),
                        grns( img.to_grayscale_matrix(0,1,0) ),
                        blus( img.to_grayscale_matrix(0,0,1) );

    kjb::Image im3;
    im3.from_color_matrices( reds, grns, blus );

    if ( different( img, im3 ) )
    {
        std::cout << "fail at " << __LINE__ << '\n';
        return EXIT_FAILURE;
    }

    /*------------------------------------------------*/

    if ( kjb_c::is_interactive() )
    {
        std::cout << "Success!\n";
    }

    return EXIT_SUCCESS;
}
