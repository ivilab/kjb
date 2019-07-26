/**
 * @file
 * @brief Interactive tests for the class DOrthoQuad
 * @author Andrew Predoehl
 *
 * This class hits the MSR Maps server for a few (around two to twelve) DOQ
 * images, which is around 200 - 1400 tiles.  Each tile is a few kilobytes,
 * so this can easily slurp around 1 - 7 MB from them.  So don't run it very
 * often, please.
 */

/*
 * $Id: test_doq.cpp 16002 2013-11-14 00:52:59Z predoehl $
 */

#include <l/l_incl.h>
#include <l_cpp/l_stdio_wrap.h>
#include <i_cpp/i_image.h>

#include <topo/index.h>
#include <topo/master.h>
#include <topo_cpp/dorthoquad.h>
#include <topo_cpp/LatLong-UTMconversion.h>


namespace {

const int NOEL = kjb::TopoFusion::NO_ELEV;

struct Places {
    const char* name;
    kjb::TopoFusion::pt utm;
} places[] = {
    { "DM Boneyard, Tucson AZ",             { 513286, 3556544, 12, NOEL } },
    { "U of Arizona campus",                { 504800, 3566200, 12, NOEL } },
    { "Bright Angel Trail",                 { 397233, 3991310, 12, NOEL } },
    { "Golden Gate bridge, CA",             { 546092, 4185730, 10, NOEL } },
    { 0, { 0, 0, 0, NOEL } }, // SENTINEL END OF LIST (OFFICIAL)

    // The following all work, but let's not open too many.
    { "Wall Street & Brooklyn bridge, NY",  { 584200, 4506900, 18, NOEL } },
    { "Tooth of Time, NM",                  { 499278, 4033646, 13, NOEL } },
    { "Taliesin West, Scottsdale AZ",       { 421500, 3718500, 12, NOEL } },
    { "French quarter, New Orleans LA",     { 783500, 3317500, 15, NOEL } },
    { "A-mountain, Tucson AZ",              { 500703, 3563567, 12, NOEL } },
    { "Reed Park, Tucson AZ",               { 507843, 3564093, 12, NOEL } },
    { "Space Needle, Lake Union, Seattle",  { 549480, 5274940, 10, NOEL } },
    { "Galveston Island Lake, TX",          { 320433, 3239000, 15, NOEL } },
    { 0, { 0, 0, 0, NOEL } }, // Extra end-of-list (after final valid location)

    // It only works inside the United States of America, so this one fails:
    { "Formerly-seven bridges of Konigsberg", { 486991, 6045900, 34, NOEL } },
    { 0, { 0, 0, 0, NOEL } } // Extra extra end-of-list (to prevent segfaults)
};

/* This test is deprecated since the "as_matrix" method has unpredictable
 * alignment -- it is aligned on tile boundaries, up to 100 meters away from
 * the indicated UTM coordinates.  Better to use test2() and test3().
 */
int test1()
{
    kjb::TopoFusion::DOrthoQuad doq( 2000 );

    kjb::Int_matrix mmm;

    for( const Places* p = places; p -> name; ++p ) {
        doq.fill( p -> utm );
        KJB( ERE( doq.as_matrix( &mmm ) ) ); // alignment is unpredictable!

        kjb::Image imago( mmm );

        pid_t pid = kjb_c::kjb_fork();
        if (0 == pid)
        {
            imago.display( p -> name );
            while(true) { kjb_c::nap(1000); }
        }
    }

    return kjb_c::NO_ERROR;
}


// display aerial imagery
int test2()
{
    const int SIZE = 2000; // length of square edge, in pixels

    for( const Places* p = places; p -> name; ++p ) {
        kjb::TopoFusion::pt northwest = p -> utm;
        northwest.x -= SIZE/2;
        northwest.y += SIZE/2;

        kjb::Int_matrix mmm(
                kjb::TopoFusion::get_aerial_image(northwest, SIZE, SIZE));
        kjb::Image imago( mmm );

        if (0 == kjb_c::kjb_fork())
        {
            imago.display( p -> name );
            while(true) { kjb_c::nap(1000); }
        }
    }

    return kjb_c::NO_ERROR;
}


// display detail of a topographic map (digitized topographic quad)
int test3()
{
    const int SIZE = 1000; // length of square edge, in pixels
    const int SIZE_METERS = SIZE * 2; // length of square edge, in meters

    for( const Places* p = places; p -> name; ++p ) {
        kjb::TopoFusion::pt northwest = p -> utm;
        northwest.x -= SIZE_METERS/2;
        northwest.y += SIZE_METERS/2;

        const kjb::Image imago( 
           kjb::TopoFusion::get_topographic_map_detail(northwest, SIZE, SIZE));

        if (0 == kjb_c::kjb_fork())
        {
            imago.display( p -> name );
            while(true) { kjb_c::nap(1000); }
        }
    }

    return kjb_c::NO_ERROR;
}

} // anonymous namespace

int main()
{
    kjb_c::kjb_init();

    try
    {
        KJB(EPETE(kjb::TopoFusion::validate_ellipsoid_table()));

        /* The "force" string here is tested to see if it is the name of a
         * local directory.  Since it almost certainly is not, the tile master
         * object will almost certainly use a temporary cache for its local
         * tile storage.
         */
        kjb::TopoFusion::Tile_manager tm("/force it to use a temporary cache");
        KJB(ASSERT( tm.is_using_a_temporary_cache() ));

        /*
         * We do the same test twice, on purpose.  The first time is to test
         * downloading the images from MSR Maps.  The images are cached
         * locally.  The second time is to test reloading from the cache.
         */
        KJB( EPETE( test2() ) );
        //KJB( EPETE( test2() ) );
        KJB( EPETE( test3() ) );
        //KJB( EPETE( test3() ) );
    }
    catch( kjb::Exception& e )
    {
        e.print_details_exit();
    }

    kjb_c::kjb_cleanup();
    return EXIT_SUCCESS;
}
