/**
 * @file
 * @brief test program shows result of palette choosing "random" colors
 * @author Andrew Predoehl
 */
/*
 * $Id: test_palette.cpp 18148 2014-11-09 21:36:11Z predoehl $
 *
 * Tab size:  4
 */

#include <l_cpp/l_int_vector.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_hsv.h>
#include <c_cpp/c_palette.h>
#include <m_cpp/m_vector.h>

namespace {

/// @brief permute the palette so similar colors are close together in the grid
void sort( kjb::Palette& p )
{
    kjb::Vector vk;

    for( size_t working_len = p.size(); 1 < working_len; --working_len )
    {
        // compute nearest color to each color
        kjb::Vector dist2(int(working_len), DBL_MAX);
        kjb::Int_vector winner(working_len, int(p.size()));
        //winner.assign( working_len, p.size() );
        for( size_t kix = 0; kix < working_len; ++kix )
        {
            vk = kjb::hsluma_space( p[ kix ] );
            for( size_t jix = 0; jix < working_len; ++jix )
            {
                if ( kix == jix ) continue;
                double d2 =
                    ( vk - kjb::hsluma_space( p[ jix ] ) ).magnitude_squared();
                if ( d2 < dist2[ kix ] )
                {
                    dist2[ kix ] = d2;
                    winner[ kix ] = jix;
                }
            }
            KJB(ASSERT( winner[ kix ] < int(p.size()) ));
        }

        // find which 2 colors are closest
        size_t mcix = 0;
        for( size_t kix = 1; kix < working_len; ++kix )
        {
            if ( dist2[ kix ] < dist2[ mcix ] )
            {
                mcix = kix;
            }
#if 0
            if ( dist2[ mcix ] != dist2[ winner[ mcix ] ] )
            {
                std::cout << "mcix=" << mcix
                    << ", winner[mcix]=" << winner[mcix]
                    << ", d2mcix=" << dist2[ mcix ] << ", d2win="
                    << dist2[winner[mcix]]
                    << ", WL=" << working_len
                    << '\n';
            }
#endif
        }
        KJB(ASSERT( dist2[ mcix ] == dist2[ winner[ mcix ] ] ));
        KJB(ASSERT( int(mcix) != winner[ mcix ] ));

        // move results to the end of the table
        // Does p[working_len - 1] already hold a color we need to be there?
        if ( p.size() == working_len )
        {
            // no;
            p.swap( mcix, p.size() - 1 );
            p.swap( winner[ mcix ], p.size() - 2 );
        }
        else
        {
            // yes; it is either about to get one or two new friends
            size_t  mxix = std::max( mcix, size_t(winner[ mcix ]) ),
                    mnix = std::min( mcix, size_t(winner[ mcix ]) );
            KJB(ASSERT( mxix != mnix ));
            KJB(ASSERT( mxix <= working_len - 1 ));
            if ( mxix < working_len - 1 )
            {
                // two new friends closer to each other than p[wl-1]
                p.swap( mxix, working_len - 2 );
                p.swap( mnix, working_len - 3 );
                --working_len;
            }
            else // one new friend also very close to p[wl-1]
            {
                p.swap( mnix, working_len - 2 );
            }
        }
    }
}

} // end anonymous ns

/// @brief generate random palette and display it graphically
int main()
{
    KJB( EPETE( kjb_signal( SIGCHLD, SIG_IGN ) ) );
    const int BLOCK = 50, NUMBLOCK = 10;
    kjb::Image im( BLOCK * NUMBLOCK, BLOCK * NUMBLOCK );
    kjb::Palette pal( 100 );

    sort( pal );

    int ii = 0;
    for( int rr = 0; rr < NUMBLOCK; ++rr )
    {
        for( int cc = 0; cc < NUMBLOCK; ++cc )
        {
            im.draw_aa_rectangle(
                        rr * BLOCK,             cc * BLOCK,
                        (1 + rr) * BLOCK - 1,   (1 + cc) * BLOCK - 1,
                        pal[ ii++ ]
                    );
        }
    }
    im.write("random_colors.png");

    if (0 == kjb_c::kjb_fork())
    {
        im.display( "your random color" );
        while (true) kjb_c::nap(1000);
    }
    return EXIT_SUCCESS;
}

