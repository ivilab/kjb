/*
 * $Id: test_image.cpp 15996 2013-11-13 21:54:27Z predoehl $
 */

#include "i/i_display.h"
#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>

namespace {

const kjb::PixelRGBA    red(100,0,0),
                        green(0,100,0),
                        blue(0,0,100),
                        black(0,0,0),
                        white(240, 240, 240);

void test1()
{
    kjb::Image i0;
    bool did_we_catch_ioob = false;

    try {
        i0.at( 0,0 ) = green;
    }
    catch( kjb::Index_out_of_bounds& )
    {
        did_we_catch_ioob = true;
    }
    assert( true == did_we_catch_ioob );

    bool did_we_catch_ioob2 = false;
    try {
        i0.at( 0 ) = green;
    }
    catch( kjb::Index_out_of_bounds& )
    {
        did_we_catch_ioob2 = true;
    }
    assert( true == did_we_catch_ioob2 );
}

int test2( bool showme )
{
    kjb::Image im(100,100);
    im.draw_aa_rectangle(0,0,99,99, black);
    for( int i = 0; i < 100; ++i )
    {
        im.at(i,i) = red; // red slash
    }
    return showme && kjb_c::is_interactive() ? im.display( "Red slash" ) : 0;
}


/// @brief this makes horizontal bands of (dim) colors
int test3()
{
    kjb::Image im(100,100);
    const kjb::Image &constima = im;
    int k = 0;
    for( int j = 0; j < 1000; ++j ) im.at( k++ ) = red;
    for( int j = 0; j < 1000; ++j ) im.at( k++ ) = green;
    for( int j = 0; j < 1000; ++j ) im.at( k++ ) = blue;
    for( int j = 0; j < 1000; ++j ) im.at( k++ ) = black;

    for( int j = 0; j < 1000; ++j, ++k ) im( k ) = constima( k-4000 );
    for( int j = 0; j < 1000; ++j, ++k ) im( k ) = constima.at( k-4000 );
    for( int j = 0; j < 1000; ++j, ++k ) im( k ) = constima( k-4000 );
    for( int j = 0; j < 3000; ++j, ++k ) im( k ) = black;

    return kjb_c::is_interactive() ? im.display( "Horizontal RGB bars" ) : 0;
}


int test4()
{
    kjb::Image im(100,100);
    im.draw_aa_rectangle(0,0,99,99, black);
    for( int j =   0; j < 20; ++j ) im( j, j, kjb::Image::RED   ) = 10*(j   );
    for( int j =  20; j < 40; ++j ) im( j, j, kjb::Image::GREEN ) = 10*(j-20);
    for( int j =  40; j < 60; ++j ) im( j, j, kjb::Image::BLUE  ) = 10*(j-40);

    const kjb::Image &constima = im;
    for ( int j = 0; j < 60; ++j ) im.at( j, 20+j ) = constima.at( j, j );
    for ( int j = 0; j < 60; ++j )
    {
        for( int c = 0; c < kjb::Image::END_CHANNELS; ++c )
            im( 20+j,j, c ) = constima( j,j, kjb::Image::END_CHANNELS-1-c );
    }
    for ( int j = 0; j < 60; ++j )
    {
        for( int c = 0; c < kjb::Image::END_CHANNELS; ++c )
            im.at(40+j,j,c) = constima.at( j,j, kjb::Image::END_CHANNELS-1-c );
    }
    return kjb_c::is_interactive() ? im.display("RGB shooting stars") : 0;
}

int test5()
{
    kjb::Image im("../input/test.tiff");
    kjb::Image im2(std::string("../input/test.tiff"));
    return kjb_c::is_interactive() ? im.display( "Test image" ) : 0;
}

int test6(double a)
{
    kjb::Image im("../input/test.tiff");
    kjb::Image im2 = kjb::scale_image(im, a);
    char temp[100];
    sprintf(temp, "Scaled by %.2f%%", a * 100);
    return kjb_c::is_interactive() ? im2.display(temp) : 0;
}

int test7(double a)
{
    kjb::Image im("../input/test.tiff");
    im.scale(a);
    char temp[100];
    sprintf(temp, "Scaled in-place by %.2f%%", a * 100);
    return kjb_c::is_interactive() ? im.display(temp) : 0;
}


void null_bug_handler(const char*) {}


int test8()
{
    const int SZ = 100, LL = 20, HH = (SZ - LL)/2;
    kjb::Image im;
    kjb::Matrix r(SZ, SZ, 0.0), g=r, b=r, wrong(SZ/2, SZ/2, 0.0);

    for (int i=0; i<LL; ++i)
    {
        for (int j=0; j<SZ; ++j)
        {
            r.at(i+HH, j) = 250;
            g.at(j, i+HH) = 250;
            b.at(j, std::min(SZ-1, std::max(0, j+i-LL/2))) = 250;
        }
    }
    im.from_color_matrices(r, g, b);

    // --------------------------------------------

    // If the inputs are not all the same size, the bug handler is called, and
    // after that an exception is thrown.  Let's verify this is the case.

    bool caught_it = false;
    try
    {
        kjb_c::set_bug_handler( &null_bug_handler );
        im.from_color_matrices(r, g, wrong);
    }
    catch( kjb::KJB_error &e )
    {
        caught_it = true;
    }
    assert(caught_it);
    kjb_c::set_bug_handler( &kjb_c::default_bug_handler );

    // ------------------------------------------

    const char *title = "test 8: from_color_matrices()";
    return kjb_c::is_interactive() ? im.display(title) : 0;
}


// test the silent clipping feature of rectangle drawing.
// output should be the french flag speckled with little green hollow squares
int test9()
{
    kjb::Image im(400, 400);
    im.draw_aa_rectangle(500, 500, -3, -3, red);        // background

    im.draw_aa_rectangle(500, 500, 450, 450, black);    // total clip to SE
    im.draw_aa_rectangle(10, 500, 350, 450, black);     // total clip to E
    im.draw_aa_rectangle(-10, 500, -5, 450, black);     // total clip to NE
    im.draw_aa_rectangle(-10, 10, -5, 350, black);      // total clip to N
    im.draw_aa_rectangle(-10, -10, -5, -50, black);     // total clip to NW
    im.draw_aa_rectangle(10, -10, 350, -50, black);     // total clip to W
    im.draw_aa_rectangle(450, -10, 550, -50, black);    // total clip to SW
    im.draw_aa_rectangle(450, 10, 550, 350, black);     // total clip to S

    im.draw_aa_rectangle(-100, 133, 600, 266, white);   // clip top and bottom
    im.draw_aa_rectangle(-100, 1266, 600, 266, blue);   // clip three sides

    // clipped green outlines
    im.draw_aa_rectangle_outline(-100, 150, 50, 250, green);  // N
    im.draw_aa_rectangle_outline(350, 150, 450, 250, green);  // S
    im.draw_aa_rectangle_outline(150, 350, 250, 450, green);  // E
    im.draw_aa_rectangle_outline(150, -100, 250, 50, green);  // W
    im.draw_aa_rectangle_outline(-100, -150, 50, 50, green);  // NW
    im.draw_aa_rectangle_outline(-100, 350, 50, 550, green);  // NE
    im.draw_aa_rectangle_outline(350, -150, 450, 50, green);  // SW
    im.draw_aa_rectangle_outline(350, 350, 450, 550, green);  // SE
    im.draw_aa_rectangle_outline(-1, -1, 550, 550, green);    // total clip

    // not clipped for once
    im.draw_aa_rectangle_outline(150, 150, 250, 250, green);  // center

    return kjb_c::is_interactive() ? im.display("clipping") : 0;
}


int main2()
{
    std::vector<int> handles;

    test1();
    test2( false );

    int h;

    h = test2( true );  handles.push_back( h );
    h = test3();        handles.push_back( h );
    h = test4();        handles.push_back( h );
    h = test5();        handles.push_back( h );
    h = test6(0.5);     handles.push_back( h );
    h = test6(2.0);     handles.push_back( h );
    h = test7(0.5);     handles.push_back( h );
    h = test7(2.0);     handles.push_back( h );
    h = test8();        handles.push_back( h );
    h = test9();        handles.push_back( h );

    if ( kjb_c::is_interactive() )
    {
        kjb_c::nap(10000);
        std::for_each( handles.begin(), handles.end(),
                                std::ptr_fun(kjb_c::close_displayed_image) );
    }

    #ifdef YES_WE_WANT_GARBAGE_POLICE
    fprintf(    stderr,
                "Garbage police report that %d Images were (ever) constructed "
                "via C++\nand %d of them are still alive now.\n", 
                kjb::Image::query_serial_counter(),
                kjb::Image::query_live_counter()
            );
    #endif

    return EXIT_SUCCESS;
}

}

int main()
{
    try
    {
        return main2();
    }
    catch( kjb::Exception &e )
    {
        e.print_details_exit();
    }
}
