/**
 * @file
 * @brief Implementation of class DOrthoQuad and class Tile_manager
 * @author Scott Morris
 * @author Andrew Predoehl
 */
/* ======================================================================== *
|                                                                           |
| Copyright (c) 2007, by members of University of Arizona Computer Vision   |
| group (the authors) including                                             |
|        Scott Morris, Kobus Barnard, Andrew Predoehl.                      |
|                                                                           |
| For use outside the University of Arizona Computer Vision group please    |
| contact Kobus Barnard.                                                    |
|                                                                           |
* ========================================================================= *
*
* $Id: dorthoquad.cpp 15331 2013-09-16 08:33:56Z predoehl $
*/

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_tsig.h"
#include "l/l_string.h"
#include "i_cpp/i_image.h"
#include "l_cpp/l_stdio_wrap.h"
#include "l_cpp/l_exception.h"
#include "i_cpp/i_pixel.h" /* debug only! */

// TopoFusion-derived code.
#include "topo/index.h"
#include "topo/master.h"
#include "topo_cpp/download.h"
#include "topo_cpp/dorthoquad.h"

#include <algorithm>
#include <limits>
#include <iostream>
#include <cmath>


namespace
{

const bool DOQ_VERBOSE = false;

const bool TILE_MANAGER_CTOR_IS_VERBOSE = false; // good for debug



/**
 * @brief try to decompress the JFIF bytestream in buf, writing into tile
 *
 * @param[in] buf       A buffer possibly containing a compressed image.  The
 *                      compression format is JPEG, formally known as JFIF.
 * @param[in] buflen    Length of the array of valid bytes in buf.
 * @param[out] tile     Decompressed pixels are written to this image.
 *                      However, the number of rows and columns of *tile must
 *                      be set equal to the dimensions of the image in buf
 *                      prior to calling.  I.e., you must know them in advance.
 *                      The pixel contents of *tile are clobbered.
 *
 * @return  True iff successful expanding buf into tile.  Return value is false
 *          if buf cannot successfully be parsed or if *tile is the wrong size.
 *
 * Historical note:  this used to rely on the Corona image library for
 * decompression, but now we write to a temporary file and load that way.
 */
bool decompressJpegBuffer(
    const char *buf,
    int buflen,
    kjb::Image* tile
)
{
    using namespace kjb_c;
    ASSERT( tile );

    int pos = 0;
    const char SEEK4[] = "JFIF"; // formal name of the JPEG compression format
    const int POSMAX = buflen - sizeof(SEEK4);
    while (pos < POSMAX && kjb_strncmp(SEEK4, buf + pos, sizeof(SEEK4)-1) != 0)
    {
        ++pos;
    }

    if ( pos < 6 || POSMAX <= pos ) return false; // input is not JPEG

    pos -= 6;   // A properly formatted file has 6 bytes before its "JFIF"

    // Write this data out to a temporary file, so class kjb::Image can read it
    // back in.  That really is the easiest way, despite its clunkiness.
    kjb::Temporary_File tf;
    long ct = kjb_fwrite( tf, buf + pos, buflen - pos );
    if ( ct != buflen - pos )
    {
        TEST_PSE(("Bad or short write of tile to /tmp.\n"));
        return false;
    }
    rewind( tf );
    if ( ERROR == kjb_fflush( tf ) )
    {
        TEST_PSE(("Unable to flush tile to /tmp.\n"));
        return false;
    }

    // use the KJB library loader to perform JFIF decompression
    try
    {
        kjb::Image jpeg( tf.get_filename() );
        if  (       jpeg.get_num_rows() == tile -> get_num_rows()
                &&  jpeg.get_num_cols() == tile -> get_num_cols()
            )
        {
            tile -> swap( jpeg );
            return true; // successful exit is from here
        }
    }
    catch ( kjb::KJB_error& e )
    {
        e.print_details();
        TEST_PSE(("Download buffer is not JPEG format.\n"));
        return false;
    }

    TEST_PSE(("Download buffer dimensions are incorrect.\n"));
    return false;
}



/**
 * @brief try to decompress buf data as a GIF, writing into tile
 *
 * @param[in] buf       A buffer possibly containing a GIF image.
 * @param[in] buflen    Length of the array of valid bytes in buf.
 * @param[out] tile     Decompressed pixels are written to this image.
 *                      However, the number of rows and columns of *tile must
 *                      be set equal to the dimensions of the image in buf
 *                      prior to calling.  I.e., you must know them in advance.
 *                      The pixel contents of *tile are clobbered.
 *
 * @return  True iff successful expanding buf into tile.  Return value is false
 *          if buf cannot successfully be parsed or if *tile is the wrong size.
 */
bool decompress_gif_buffer(
    const char *buf,
    int buflen,
    kjb::Image* tile
)
{
    if (0 == tile) return false;

    // verify the header says that the buffer contains GIF data
    const std::string raw(buf, buflen);
    const size_t typecheck = raw.find("Content-Type: image/gif\r\n");
    if (std::string::npos == typecheck) return false;

    // read the size of the payload from the length field
    const std::string CLPAT("Content-Length: ");
    size_t sizecheck = raw.find(CLPAT);
    if (std::string::npos == sizecheck) return false;
    sizecheck += CLPAT.size();
    size_t gif_size = 0;
    while (std::isdigit(raw[sizecheck]))
    {
        gif_size *= 10;
        gif_size += raw[sizecheck++] - '0';
    }
    if (    sizecheck+1 >= raw.size()
        ||  raw[sizecheck+0] != '\r'
        ||  raw[sizecheck+1] != '\n'
       )
    {
        return false;
    }

    // find the end-of-header string
    const std::string EOHPAT("\r\n\r\n");
    const size_t eoh = raw.find(EOHPAT);
    if (std::string::npos == eoh) return false;
    if (eoh < typecheck || eoh < sizecheck) return false;

    // make sure the size is exactly correct
    const std::vector<char> payload(raw.begin()+eoh+EOHPAT.size(), raw.end());
    if (payload.size() != gif_size) return false;

    // Write this data out to a temporary file, so class kjb::Image can read it
    // back in.  That really is the easiest way, despite its clunkiness.
    kjb::Temporary_Recursively_Removing_Directory td;
    const std::string fn(td.get_pathname() + DIR_STR + "topo_tile.gif");
    kjb::File_Ptr_Write tf(fn);
    const long ct = kjb_c::kjb_fwrite(tf, & payload.front(), payload.size());
    if (ct != long(payload.size()))
    {
        KJB(TEST_PSE(("Bad or short write of tile to /tmp.\n")));
        return false;
    }
    if ( kjb_c::ERROR == tf.close() )
    {
        KJB(TEST_PSE(("Unable to close temporary GIF file")));
        return false;
    }

    // use the KJB library loader to read the file
    try
    {
        kjb::Image i( fn );
        if  (       i.get_num_rows() == tile -> get_num_rows()
                &&  i.get_num_cols() == tile -> get_num_cols()
            )
        {
            tile -> swap(i);
            return true; // successful exit is from here
        }
    }
    catch ( kjb::KJB_error& e )
    {
        e.print_details();
        KJB(TEST_PSE(("Download buffer is not GIF format.\n")));
        return false;
    }

    KJB(TEST_PSE(("Download buffer dimensions are incorrect.\n")));
    return false;
}



inline int getTileFromDisk(
    const kjb::TopoFusion::tile_entry& e,
    char* buf,
    size_t bufsz
)
{
    using kjb_c::TopoFusion::getTileFromDisk;
    return getTileFromDisk( e.x, e.y, e.tileset, e.zone, buf, bufsz );
}


inline
int addIndexEntry(
    const kjb::TopoFusion::tile_entry& e,
    const char* buf,
    int bufln
)
{
    using kjb_c::TopoFusion::addIndexEntry;
    return addIndexEntry( e.x, e.y, e.tileset, e.zone, buf, bufln );
}


inline int invalidateEntry( const kjb::TopoFusion::tile_entry& e )
{
    return kjb_c::TopoFusion::invalidateEntry( e.x, e.y, e.tileset, e.zone );
}


const int MAX_RETRY = 1000;
int retry_test( int retry_ct )
{
    if ( MAX_RETRY <= retry_ct )
    {
        kjb_c::set_error( "Unable to download tile" );
        return kjb_c::ERROR;
    }
    return kjb_c::NO_ERROR;
}


kjb::TopoFusion::DOrthoQuad::Pixel grayscale_average(const kjb_c::Pixel& p)
{
    return static_cast<kjb::TopoFusion::DOrthoQuad::Pixel>(
                                               0.5 + (p.r + p.g + p.b) / 3.0f);
}


int plus_margin(int edge_size_px)
{
    return edge_size_px + 3 * kjb_c::TopoFusion::TILE_SIZE;
}


} // end anonymous namespace


namespace kjb
{
namespace TopoFusion
{


/// @brief intentionally-long pathname to where we cache DOQ tiles at the lab.
const char* IVILAB_DOQ_TILE_CACHE_DIRECTORY="/net/v04/data_3/trails/doq_cache";

/// flag to force singleton behavior (i.e., only one instantiation per run).
bool Tile_manager::m_instantiated = false;



Tile_manager::Tile_manager(const char* dir) // common case: dir eq. to default
:   m_temp_cache(false)
{
    if (m_instantiated)
    {
        KJB_THROW_2(Runtime_error, "class Tile_manager permits only "
          "singleton instantiation (i.e., do not try to create two of them).");
    }

    std::string master_path(dir);
    if (! kjb_c::is_directory(dir))
    {
        td.reset(new Temporary_Recursively_Removing_Directory());
        m_temp_cache = true;
        master_path = td -> get_pathname();
        KJB(ETX(kjb_mkdir((master_path + DIR_STR + "maps.dat").c_str())));
        if (TILE_MANAGER_CTOR_IS_VERBOSE)
        {
            KJB(TEST_PSE(("class Tile_manager ctor: open temp cache %s\n",
                                                        master_path.c_str())));
        }
    }
    else if (TILE_MANAGER_CTOR_IS_VERBOSE)
    {
        using namespace kjb_c;
        TEST_PSE(("class Tile_manager ctor: using default cache %s\n", dir));
    }

    ETX(kjb_c::TopoFusion::init_master(master_path.c_str()));

    // To avoid nuisance warnings, we let libkjb close the file if it wants to.
    ETX(kjb_c::add_cleanup_function( & kjb_c::TopoFusion::closeIndexFile ));

    m_instantiated = true;
}


Tile_manager::~Tile_manager()
{
    /*
     * Possibly by this time, libkjb might have closed the file already; or
     * maybe not.  We do not know the order.  Fortunately, the closing function
     * here is "idempotent," meaning that it is safe to call a second or third
     * time or whatever.  That is good -- it's essential -- because both the
     * dtor and libkjb (via "add_cleanup_function") are bound to call it.
     */
    kjb_c::TopoFusion::closeIndexFile();

    m_instantiated = false;
}



/// @brief Construct a digital orthoquad image buffer; must be square.
DOrthoQuad::DOrthoQuad( unsigned edge_length_pix )
:   m_tileset_code(kjb_c::TopoFusion::AIR_1M),
    m_doq(plus_margin(edge_length_pix), plus_margin(edge_length_pix), 0, 0, 0)
{}



char DOrthoQuad::select_tileset( char tileset )
{
    if (    tileset < kjb_c::TopoFusion::TILESET_ID_MIN
        ||  tileset >= kjb_c::TopoFusion::NO_MAP
       )
    {
        KJB_THROW_2(Illegal_argument,
                            "argument to select_tileset() is out of bounds");
    }

    // trivial case:  no change
    if (m_tileset_code == tileset) return m_tileset_code;

    // nontrivial change
    char old_tileset = m_tileset_code;
    m_tileset_code = tileset;
    m_current = Loc();
    return old_tileset;
}


void DOrthoQuad::clearCache(
    const Loc& nw_request,
    int numtilebuf                  // Input:  Amount (in tiles) of context
                                    // desired around pos.  more = bigger image
                                    // (Each tile is 200x200 pixels.)
)
{
    m_current = Loc();
    ASSERT( ! isReady() );

    const int tile_ct = numtilebuf * 2 + 1;
    const int &MAX = std::numeric_limits< int >::max();
    ASSERT( nw_request.left + tile_ct < MAX );
    ASSERT( nw_request.top < MAX );

    for ( long x = nw_request.left; x < nw_request.left + tile_ct; ++x )
    {
        for( long y = nw_request.top; y > nw_request.top - tile_ct; --y )
        {
            KJB(EPE(invalidateEntry(tile_entry(
                    static_cast< int >( x ),
                    static_cast< int >( y ),
                    m_tileset_code,
                    nw_request.zone
                ))));
        }
    }
}


/**
 * @brief takes a point and fills the doq buffer with image data (unsigned
    chars) around it.

    Uses the TopoFusion mapsX.dat and mapindex.dat format to store maps locally

    if a map tile needed is not found it will download (syncronously) the tile
    from terraserver and store it in the map database for future iterations

    Original version written by Scott Morris.

    Revised by Andrew Predoehl:
        * doqvec, doqleft, doqtop are no longer globals.
        * doqvec is now a C++ STL container rather than an C-style array
        * TFB now clips at the edges of the doqvec buffer instead of segfault
        * download loop calls nap() before retrying its download.
        * tile_entry concept unifies indexing metadata and improves readability
        * Misc. style improvements like using C++ scope rules to shrink scopes.

    Note that the download loop NEVER gives up.  Good or bad?  I have seen it
    happen that downloading a single DOQ takes half an hour of retries.
    However, intuitively, having an infinite loop there seems bad.

    @param[in] pos          Position of point of interest, in UTM:  easting,
                            northing, and zone.  It will be roughly centered in
                            doqvec.
    @param[in] numtilebuf   Amount (in tiles) of context desired around 'pos';
                            more means bigger image.  Area downloaded is a
                            square of side length 2 x numtilebuf + 1 tiles.
                            (Each tile is 200x200 pixels.)

    @return kjb_c::ERROR or kjb_c::NO_ERROR indicating outcome.
*/
int DOrthoQuad::fillDoq(
    const Loc& nw_request,
    int numtilebuf
)
{
    using kjb_c::TopoFusion::TILE_SIZE; // tile size in pixels
    ASSERT( ! nw_request.is_unused() );

    /*
     * Tile buffer.
     * The capacity here is overkill: actual size is < 10k over 90% of the time.
     */
    std::vector<char> vbuf( 500000 );

    if (DOQ_VERBOSE)
    {
        std::cout << "Entering " << __func__ <<" with numtilebuf="<< numtilebuf
                  << " and nw_request(l,t,z) = (" << nw_request.left << ", "
                  << nw_request.top << ", " << int(nw_request.zone) << ")\n";
    }

#if 0
    const long x_size = m_doq.get_num_cols(), y_size = m_doq.get_num_rows();
#endif

    // See if the doq buffer already contains the desired tile; if so, exit.
    if ( m_current == nw_request )
    {
        return kjb_c::NO_ERROR;  // doq is already filled with correct pixels
    }

    m_current = nw_request;

    Image tile1( TILE_SIZE, TILE_SIZE, 0, 0, 0 );

    // Units on x, below, are TILES not pixels.
    const int tile_ct_end = numtilebuf * 2 + 1;
    for ( long x = m_current.left; x < m_current.left + tile_ct_end; ++x )
    {
        // Units on y, below, are also tiles.
        for ( long y = m_current.top; y > m_current.top - tile_ct_end; --y )
        {
            if (DOQ_VERBOSE)
            {
                std::cout << __func__ << " tile fill, "
                    "x=" << x << " of " << m_current.left + tile_ct_end << ", "
                    "y=" << y << " of " << m_current.top - tile_ct_end << '\n';
            }
            const tile_entry entry( x, y, m_tileset_code, m_current.zone );
            int retry_ct;
            for (retry_ct = 0; retry_ct < MAX_RETRY; ++retry_ct)
            {
                long int buflen;

                // Zero the buffer, for good data hygiene
                std::fill(vbuf.begin(), vbuf.end(), 0);
                char* const buf = & vbuf.front();

                // Retrieve the image tile into the buffer
                if (0 == (buflen = getTileFromDisk(entry, buf, vbuf.size())))
                {
                    //download the tile
                    while ((buflen=download_tile(&entry, buf, vbuf.size()))<0)
                    {
                        KJB(TEST_PSE(("Download fail!  Sleep, then retry\n")));
                        kjb_c::nap(10000); // ten seconds
                        ++retry_ct;
                        KJB( ERE( retry_test( retry_ct ) ) );
                    }
                    ASSERT( buflen > 0 );
                    int rc = addIndexEntry( entry, buf, buflen );
                    ASSERT( rc != -1 );
                }

                // Expand the data into a bitmap, exit (if successful)
                // Inputs may be JPEG or GIF format, so we try each.
                if (decompressJpegBuffer( buf, buflen, &tile1 )) break;
                if (decompress_gif_buffer( buf, buflen, &tile1 )) break;
                // Try more image formats here, and break if successful.

                // Backtrack if the last step failed (uncommon case)
                KJB(ERE( invalidateEntry( entry )));
                KJB(TEST_PSE(( "Bad image buffer!  Sleep, then retry\n" )));
                kjb_c::nap(10000); // ten seconds
            }
            KJB( ERE( retry_test( retry_ct ) ) );

#if 0
            // Fill the array, clipping perhaps.  Units of i,j below are pixels
            for ( long j = 0; j < TILE_SIZE; ++j )
            {
                const long Yix = j + (m_current.top - y) * TILE_SIZE;
                // test if edge of tile falls off north, south edges of doq
                if ( Yix >= y_size ) break; // southernmost row?  time to stop.
                if ( Yix < 0 ) continue; // northernmost row?  try next row.
                for ( long i = 0; i < TILE_SIZE; ++i )
                {
                    const long Xix = i + (x - m_current.left) * TILE_SIZE;
                    // if edge of tile falls off east, west edges of doq
                    if ( Xix >= x_size ) break;
                    if ( Xix < 0 ) continue;
                    // All channels have equal intensity, so let's read Red.
                    m_doq[ Xix ][ Yix ] = tile1.at( j, i ).r;
                }
            }
#else
            m_doq.draw_image(
                    tile1,
                    (m_current.top - y) * TILE_SIZE,
                    (x - m_current.left) * TILE_SIZE
                );
#endif
        }
    }

    return kjb_c::NO_ERROR;
}



/**
 * @brief Compute how many tiles are needed to cover X number of meters.
 *
 * How many tiles does it take to cover a given number of (linear) meters?
 * This will tell you.  Basically it computes the "ceiling" of the length
 * in meters, divided by tile size in meters.
 *
 * @return that number of tiles
 */
int DOrthoQuad::num_tiles_to_cover( int meters ) const
{
    const long TILE_SIZE_METERS
               = kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].UTM_Size;

    if (meters < 0) KJB_THROW_2(Illegal_argument, "negative cover distance");
    ASSERT( 0 < TILE_SIZE_METERS );

    int tiles_2_cover = meters / TILE_SIZE_METERS; // want "ceil" of this
    const long meters_of_whole_tiles = TILE_SIZE_METERS * tiles_2_cover;
    // "ceiling"-ifiy the above result.
    if ( meters_of_whole_tiles < meters ) tiles_2_cover += 1;

    return tiles_2_cover;
}


/// @return (linear) number of tiles needed to cover the given width and height
int DOrthoQuad::num_tile_buf() const
{
    return std::max(    num_tiles_to_cover( width() ),
                        num_tiles_to_cover( height() ) ) / 2;
}


int DOrthoQuad::fill( const kjb::TopoFusion::pt& center )
{
    const long TILE_SIZE_METERS
               = kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].UTM_Size;
    const int n = num_tile_buf();

    ASSERT( long(width()) <= TILE_SIZE_METERS * (1 + 2 * n) );
    ASSERT( long(height()) <= TILE_SIZE_METERS * (1 + 2 * n) );

    if (DOQ_VERBOSE)
    {
        std::cout << "In " << __func__ << " with center at (e,n,z)=("
                  << center.x << ", " << center.y << ", " << int(center.zone)
                  << ") and number of tiles is " << n << '\n';
    }

    /* Convert position from UTM meters into tile indices
     * So, the units of doqleft and doqtop are tiles, each of size
     * TILE_SIZE pixels, or TILE_SIZE_METERS meters, on a side.
     * Also, we move from request center to northwest corner of whole image
     */
    kjb_c::kjb_disable_paging();
    const Loc l = Loc::tile_nw_from_utm_center(center, TILE_SIZE_METERS, n);
    const int rc = fillDoq(l, n);
    kjb_c::kjb_restore_paging();
    return rc;
}


int DOrthoQuad::refill( const kjb::TopoFusion::pt& center )
{
    const long TILE_SIZE_METERS
               = kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].UTM_Size;
    const int n = num_tile_buf();
    clearCache(Loc::tile_nw_from_utm_center(center, TILE_SIZE_METERS, n), n);
    return fill( center );
}


/// @brief Return the "easting" coordinate of the left edge of the DOQ.
long DOrthoQuad::left() const
{
    if ( ! isReady() ) KJB_THROW_2(Runtime_error, "DOQ not filled");
    const long TILE_SIZE_METERS
               = kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].UTM_Size;
    return TILE_SIZE_METERS * m_current.left;
}

/// @brief Return the "northing" coordinate of the top edge of the DOQ.
long DOrthoQuad::top() const
{
    if ( ! isReady() ) KJB_THROW_2(Runtime_error, "DOQ not filled");
    const long TILE_SIZE_METERS
               = kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].UTM_Size;
    return TILE_SIZE_METERS * m_current.top;
}


size_t DOrthoQuad::width() const
{
    return m_doq.get_num_cols() * meters_per_pixel();
}


size_t DOrthoQuad::height() const
{
    return m_doq.get_num_rows() * meters_per_pixel();
}


/**
 * @brief represent DOQ as a matrix of integers: row 0, col 0 is northwest
 * @return kjb_c::ERROR if DOQ is not yet filled, otherwise kjb_c::NO_ERROR.
 * @warning Remember the DOQ is not centered at the point given to the ctor.
 * @see read_abs() to get precise centering control over the DOQ imagery.
 */
int DOrthoQuad::as_matrix( Int_matrix *doq ) const
{
    KJB( NRE( doq ) );

    if ( ! isReady() )
    {
        kjb_c::set_error( "DOQ has not yet been pointed to a location." );
        return kjb_c::ERROR;
    }

    Int_matrix m = m_doq.to_grayscale_matrix().floor();
    doq -> swap(m);
    return kjb_c::NO_ERROR;
}


/// @brief Read pixel RELATIVE TO current left,top settings (meters offset)
DOrthoQuad::Pixel DOrthoQuad::read( unsigned xcoord, unsigned ycoord ) const
{
    return grayscale_average(read_color(xcoord, ycoord));
}


/// @brief Read pixel RELATIVE TO current left,top settings (meters offset)
kjb_c::Pixel DOrthoQuad::read_color(unsigned xcoord_m, unsigned ycoord_m) const
{
    if ( ! isReady() ) KJB_THROW_2(Runtime_error, "DOQ not filled");

    const float PPM = 1.0f / meters_per_pixel();
    return m_doq.at( ycoord_m * PPM, xcoord_m * PPM );
}


/**
 * @brief Return pixel at given UTM coordinates (zone assumed)
 * @param abs_xcoord UTM easting (in meters) in same zone as last fill.
 * @param abs_ycoord UTM northing (in meters) in same zone as last fill.
 * @return grayscale level of the imagery at that location
 * @throws Index_out_of_bounds if the query is outside the buffer contents
 *
 * This is at absolute location (not relative to current left,top), conditioned
 * on the current value of zone().  If you go out of bounds it will throw.
 */
DOrthoQuad::Pixel DOrthoQuad::read_abs(long abs_xcoord, long abs_ycoord) const
{
    return grayscale_average(read_abs_color(abs_xcoord, abs_ycoord));
}


/**
 * @brief Return color pixel at given UTM coordinates (zone assumed)
 * @param abs_xcoord UTM easting (in meters) in same zone as last fill.
 * @param abs_ycoord UTM northing (in meters) in same zone as last fill.
 * @return pixel value at the specified location
 * @throws Index_out_of_bounds if the query is outside the buffer contents
 *
 * This is at absolute location (not relative to current left,top).
 * The zone for the coordinates is the same as that of the last fill command,
 * but if you aren't sure what that was, just use the zone() method.
 * If you go out of bounds it will throw.
 */
kjb_c::Pixel DOrthoQuad::read_abs_color(long abs_xcoord, long abs_ycoord) const
{
    if ( abs_xcoord < left() || top() < abs_ycoord )
    {
        std::ostringstream oob;
        oob << "query x,y = " << abs_xcoord << ',' << abs_ycoord
            << "; left,top = " << left() << ',' << top();
        KJB_THROW_2(Index_out_of_bounds, oob.str());
    }
    return read_color( static_cast< unsigned >( abs_xcoord - left() ),
                       static_cast< unsigned >( top() - abs_ycoord  ) );
}


float DOrthoQuad::meters_per_pixel() const
{
    return kjb_c::TopoFusion::TileSource[ int(m_tileset_code) ].metersPerPixel;
}


/**
 * @brief Return pixel at given UTM coordinates
 * @param p query location (absolute, not relative to current left,top)
 * @returns intensity of aerial imagery at query location p
 * @throws Index_out_of_bounds if the DOQ does not hold imagery at point p
 * @warning The query point zone must match the center point zone, even if
 *          the query point properly ought to be in a different zone.  In
 *          that case, you must "stretch" the center zone to cover the
 *          query point.
 */
DOrthoQuad::Pixel DOrthoQuad::read_abs(const pt& p) const
{
    if (p.zone != zone())
    {
        KJB_THROW_2(Index_out_of_bounds, "zone mismatch");
    }
    return read_abs(long(std::floor(p.x+0.5)), long(std::floor(p.y+0.5)));
}


/**
 * @brief return a matrix (like a monochrome image) of aerial imagery
 * @param nw_corner     Northwest corner location of the image
 * @param width         Width of the image, in meters (one meter per pixel)
 * @param height        Height of the image, in meters (one meter per pixel)
 * @returns an integer matrix with pixel intensities (north at top, east right)
 */
Int_matrix get_aerial_image(const pt& nw_corner, size_t width, size_t height)
{
    size_t edge = std::max(width, height);

    boost::scoped_ptr< Tile_manager > t;
    if (! Tile_manager::is_already_instantiated_elsewhere())
    {
        t.reset(new Tile_manager);
    }

    DOrthoQuad d(edge);
    pt center(nw_corner);
    center.x += width/2;
    center.y -= height/2;
    d.fill(center);

    Int_matrix out(height, width); // i.e., rows, columns
    for (size_t rrr = 0; rrr < height; ++rrr)
    {
        for (size_t ccc = 0; ccc < width; ++ccc)
        {
            out(rrr, ccc) = d.read_abs(nw_corner.x + ccc, nw_corner.y - rrr);
        }
    }
    return out;
}


/**
 * @brief return an image of a chunk of topographic map
 * @param nw_corner     Northwest corner location of the image
 * @param width         Width of the image, in pixels (two meters per pixel)
 * @param height        Height of the image, in pixels (two meters per pixel)
 * @returns a pixel matrix showing a detail of a USGS topographic orthoquad
 *
 * This works by instantiating a DOrthoQuad object (possibly preceded by a
 * Tile_manager object too, if necessary), setting the theme to TOPO_2M, and
 * extracting a chunk of the selected size.
 */
Image get_topographic_map_detail(
    const pt& nw_corner,
    size_t width, // pixels
    size_t height // pixels
)
{
    // We must have the tile management machinery set up, here or elsewhere.
    boost::scoped_ptr< Tile_manager > t;
    if (! Tile_manager::is_already_instantiated_elsewhere())
    {
        t.reset(new Tile_manager);
    }

    // Set up orthoquad data structure of proper size.
    DOrthoQuad d(std::max(width, height)); // units are pixels
    d.select_tileset(kjb_c::TopoFusion::TOPO_2M);

    // Compute the center location.
    const size_t    METERS_PER_PIXEL = d.meters_per_pixel(),
                    width_meters = METERS_PER_PIXEL * width,
                    height_meters = METERS_PER_PIXEL * height;
    ASSERT(2u == METERS_PER_PIXEL);
    pt center(nw_corner);
    center.x += 0.5 * width_meters;  // center is halfway past west edge
    center.y -= 0.5 * height_meters;

    // Read the image, from disk or internet.
    d.fill(center);

#if 1
    Image out(height, width);
    for (size_t rrr = 0; rrr < height; ++rrr)       // rrr indexes pixels
    {
        for (size_t ccc = 0; ccc < width; ++ccc)    // ccc indexes pixels
        {
            long x_abs = nw_corner.x + METERS_PER_PIXEL*ccc,  // in meters
                 y_abs = nw_corner.y - METERS_PER_PIXEL*rrr;  // in meters
            out.at(rrr, ccc) = d.read_abs_color(x_abs, y_abs);
        }
    }
#else

    /* DEBUG VERSION */
    const double PIX_PER_METER = 1.0 / METERS_PER_PIXEL;
    Image out( d.debug_access_internal_image_buffer() );
    int target_top_row = 0.5 + (d.top() - nw_corner.y) * PIX_PER_METER;
    int target_left_col = 0.5 + (nw_corner.x - d.left()) * PIX_PER_METER;
    int target_bottom_row = target_top_row + height;
    int target_right_col = target_left_col + width;

    const pt ctr = d.center();
    std::cerr << "bounding box (pixel coords):\nL, R = "
        << target_left_col << ", " << target_right_col
        << "\ntop, bot = " << target_top_row << ", " << target_bottom_row
        << "\nnw_corner utm (e,n,z) = (" << nw_corner.x << ", "
        << long(nw_corner.y) << ", " << int(nw_corner.zone)
        << ")\nDOQ internal buffer size, pix (height, width) = ("
        << out.get_num_rows() << ", " << out.get_num_cols()
        << ")\nDOQ internal buffer size, meters (height, width) = ("
        << d.height() << ", " << d.width()
        << ")\nDOQ internal buffer top northing = " << d.top()
        << "\nDOQ internal buffer left easting = " << d.left()
        << "\nDOQ internal buffer center (e,n,z) = (" << ctr.x
        << ", " << long(ctr.y) << ", " << int(ctr.zone)
        << ")\nRequest, pix (height, width) = ("
        << height << ", " << width << ")\n";

    if (target_top_row < 0)
    {
        std::cerr << "top neg: " << target_top_row << '\n';
        target_top_row=0;
    }
    if (target_left_col<0)
    {
        std::cerr << "left neg: " << target_left_col << '\n';
        target_left_col=0;
    }
    if (target_right_col>=out.get_num_cols())
    {
        std::cerr << "right over: " << target_right_col << " eq. or exceeds "
                    << out.get_num_cols() << '\n';
        target_right_col = out.get_num_cols() - 1;
    }
    if (target_bottom_row >= out.get_num_rows())
    {
        std::cerr << "bot over: " << target_bottom_row << " eq. or exceeds "
                  << out.get_num_rows() << '\n';
        target_bottom_row = out.get_num_rows() - 1;
    }
    out.draw_aa_rectangle_outline(target_top_row, target_left_col,
            target_bottom_row, target_right_col,
            kjb::PixelRGBA(180, 0, 180) /* purple */ );
#endif
    return out;
}


} // end namespace TopoFusion
} // end namespace kjb
