/**
 * @file
 * @brief Contains definition for class DOrthoQuad
 * @author Scott Morris
 * @author Andrew Predoehl
 *
 * Definition for the DOrthoQuad class, which retrieves MSR Maps imagery from
 * a local filesystem cache or from Microsoft Research via the internet.
 */
/*
 * $Id: dorthoquad.h 15331 2013-09-16 08:33:56Z predoehl $
 */

#ifndef DORTHOQUAD_H_UOFARIZONAVISION
#define DORTHOQUAD_H_UOFARIZONAVISION

#include <i_cpp/i_image.h>
#include <l_cpp/l_int_matrix.h>
#include <l_cpp/l_stdio_wrap.h>
#include <topo/master.h>
#include <topo_cpp/layer.h>

#include <vector>
#include <boost/scoped_ptr.hpp>


namespace kjb
{
namespace TopoFusion
{


/// @brief hardcoded filesystem path to the /data_3 directory storing DEM tiles
extern const char* IVILAB_DOQ_TILE_CACHE_DIRECTORY;


/**
 * @brief RAII tool for opening and closing the DOQ master index.
 *
 * This object manages the DOQ master index of tile files, which should be
 * opened prior to generating a DOQ, and closed when it is no longer needed.
 * DOQ tiles are available two places:  from msrmaps.com and from the local
 * cache stored on an IVI Lab fileserver, specifically the one called "data_3."
 *
 * The data_3 fileserver is only visible inside the IVI lab network.
 * If you are attached to the network (by wire or by SSH tunnel) then you
 * will probably get fastest response using our fileserver.
 * Any locations absent from the fileserver will be downloaded from the
 * internet and stored in the fileserver's cache.  The cache size has no
 * explicit limit, as of this writing (July 2013).
 *
 * If you are outside the network (i.e., not attached via wire or SSH or any
 * other tunnel) you can download files using the internet, provided that
 * msrmaps.com is still serving the tiles, something they won't do forever.
 * This will then store tiles locally, in your /tmp directory (or equivalent
 * temporary location), and delete them when this object is destroyed.
 * Please note that it will NOT help you build up a local cache of tiles!
 * To repeat:  unless you connect to an existing tile cache in a local
 * filesystem, this object will delete whatever it downloads.
 *
 * If you want to keep the tiles you download, then you must use the C
 * interface directly.  Specifically, call the init_master() function in a
 * properly-prepared directory of your choice.  If you do all that, you can
 * (after you've set it up) instantiate one of these objects and point it at
 * the top-level directory, and the object will grow the cache for you as
 * you create DOrthoquad objects pointed hither and yon.
 *
 * This class is not copyable, not assignable, and is meant to be singleton,
 * i.e., at most one instance exists in any run.  The tile index relies on
 * statically-stored structures behind the scenes, so you cannot safely open
 * multiple indexes if accessing a filesystem cache (neither with threads nor
 * with separate processes).  Singleton semantics is enforced:  if you try to
 * instantiate a second time in one process, the ctor will throw.
 * You can instantiate multiple managers if they are attached to temporary
 * caches; however, be careful not to stress out msrmaps.com.
 *
 * @warning msrmaps.com is a free service but their goodwill cannot possibly
 * be unlimited.  If we download lots of tiles from them, we could exhaust
 * their generosity of bandwidth.
 */
class Tile_manager
{
    boost::scoped_ptr< Temporary_Recursively_Removing_Directory > td;

    // singleton-enforcement:  you can instantiate this class at most once.
    static bool m_instantiated;

    /// status:  true if the cache is temporary rather than preexisting
    bool m_temp_cache;

    // Not copyable, not assignable
    Tile_manager& operator=(const Tile_manager&); // teaser
    Tile_manager(const Tile_manager&); // teaser

public:
    /**
     * @brief Open the tile cache index, if possible on the IVI lab fileserver.
     * @param dir OPTIONAL pathname to tile cache.  Usually omitted; see below.
     * @throws KJB_error if we cannot initialize (and lots of steps can fail).
     *
     * In typical use, you should NOT supply a dir argument; instead, you
     * should accept the default.  The default will test whether the files we
     * want are already available on the local filesystem, in the "usual
     * place."  On an IVI lab machine, the "usual place" will indeed hold tile
     * files.  If "the usual place" does not even exist, nothing bad happens!
     * Instead, we create a temporary directory for tile files, and download
     * them from msrmaps.com.  We will delete the temporary directory and its
     * contents at destructor-time.
     *
     * The idea here is to make life easy for you, the caller!  Ideally you
     * need not add any logic to test whether you're on the SISTA network.
     * This class will handle it for you.  On the network we use cached tiles,
     * off the network (hopefully a rare situation) we download them.
     *
     * The only time you need to supply an argument for 'dir' is if you have
     * (on your own) built up your own cache of tile files in TopoFusion format
     * and you want to use that cache or expand it.  If so, you should
     * pass in the absolute pathname to the directory that contains the files
     * mapindex.dat file and the maps.dat subdirectory.  Caches of this kind
     * easily grow to a few gigabytes.  This class will not help you get it
     * started, but if you've already gotten started this class could help you
     * grow it.
     */
    Tile_manager(const char* dir = IVILAB_DOQ_TILE_CACHE_DIRECTORY);

    /// @brief dtor closes the tile cache index, maybe deleting the index.
    ~Tile_manager();

    /// @brief returns whether the cache is stored in a temporary directory.
    bool is_using_a_temporary_cache() const
    {
        return m_temp_cache;
    }

    /// @brief return whether the (singleton) object already exists
    static bool is_already_instantiated_elsewhere()
    {
        return m_instantiated;
    }
};



/**
 * @brief Digital orthoquad buffer
 *
 * This class lets you build grayscale digital orthoquad images.  You choose
 * the size, and then you can "fill" it centered at a given UTM coordinate.
 * This downloads the data for you; then you can read pixels out of the buffer
 * at will.  The "fill" process queries an on-disk cache, and if any desired
 * tiles are absent, it will request the missing tiles over the internets.
 *
 * Centering is approximate.  There is a granularity of about 200 pixels; the
 * resulting image can have an offset of about plus or minus 100 pixels in each
 * direction.  The best way to cope with centering jitter is to use the
 * read_abs() method to read pixels into a matrix, instead of the as_matrix
 * method.
 *
 * Default tileset:  aerial imagery at 1 meter per pixel resolution, i.e.,
 * the tileset coded AIR_1M in header file lib/topo/master.h.
 * You can change the tileset with the select_tileset() method.  This is a
 * new method, and
 * much of the code and documentation assumes a 1 meter per pixel resolution.
 * But many other tilesets have a different resolution, and so the code might
 * not work as expected or the documentation might be wrong for such tilesets.
 * It is a (low-priority) work in progress to make the code and documentation
 * consistent with the recent possibility for other resolutions.
 */
class DOrthoQuad
{

public:
    typedef unsigned char Pixel;    ///< a single pixel is a gray level 0-255

private:

    /// UTM point with tile-size resolution
    struct Loc
    {
        enum { UNUSED = -1 };
        long    left,   ///< tile-index coordinate of western edge of doq
                top;    ///< tile-index coordinate of northern edge of doq
        char    zone;   ///< UTM zone of doq

        /// default ctor marks left field with UNUSED sentinel
        Loc()
        :   left( UNUSED )
        {}

        /// default basic initialization ctor
        Loc( long l, long t, char z )
        :   left( l ),
            top( t ),
            zone( z )
        {}

        /// test whether the object is still in its primordial, unused state
        bool is_unused() const
        {
            return UNUSED == left;
        }

        /// test whether two locs are equal -- integer comparison is legit, yo
        bool operator==( const Loc& that ) const
        {
            return left == that.left && top == that.top && zone == that.zone;
        }

        /// named ctor builds northwest corner loc from center, num tiles away
        static Loc tile_nw_from_utm_center(
            const pt& center, ///< desired center of orthoquad
            int tilesize_m,   ///< size of tiles, in meters (not pixels)
            int numtiles      ///< number of tiles from center to each edge
        )
        {
            return Loc(
                static_cast< long >( center.x / tilesize_m - numtiles ),
                static_cast< long >( center.y / tilesize_m + numtiles ),
                center.zone
            );
        }
    };

    Loc m_current; ///< top, left edge of current image, measured in tiles

    char m_tileset_code;

    Image m_doq; ///< pixel storage buffer

    int num_tile_buf() const;

    /// fill pixel storage buffer m_doq with imagery, from disk cache or net
    int fillDoq( const Loc&, int );

    /// clear a number of tiles out of the file cache, at a given location
    void clearCache( const Loc&, int );

public:

    int num_tiles_to_cover( int ) const;

    /// @brief Construct a digital orthoquad image buffer; must be square.
    DOrthoQuad( unsigned edge_length_pix = 0 );


    /**
     * @brief Fill the array with image data roughly around a given point.
     *
     * Use fill() under ordinary circumstances, since it will make use of
     * locally cached results.
     *
     * 3 Dec 2010:  It's unbelievable that I'm only discovering this after 2
     * years working with this code, but I now think the resulting image is
     * only ROUGHLY centered on the given location, with a granularity of
     * one "tile" (an internal unit of image tranfer that I thought was
     * encapsulated so that you, the client, would never have to know about
     * it).  A tile, at this writing, is 200 x 200 pixels, so the filled image
     * could, I think, be offset from your specified center by plus or minus
     * 100 pixels, in either axis.
     *
     * Use the read_abs() method to avoid the above granularity.
     *
     * @return ERROR or NO_ERROR indicating outcome.
     */
    int fill( const pt& );

    /**
     * @brief Like fill(), but force a download from the internet.
     *
     * Use refill() when an image is obviously corrupted, and it will clear
     * out the cache and force a fresh download.
     *
     * @return ERROR or NO_ERROR indicating outcome.
     */
    int refill( const pt& );


    /// @brief change the tileset "theme" -- see themes in lib/topo/master.h
    char select_tileset(char);



    /// @brief Return the "easting" coordinate of the left edge of the DOQ.
    long left() const;

    /// @brief Return the "northing" coordinate of the top edge of the DOQ.
    long top() const;

    /// @brief return UTM zone for the DOQ (which must lie entirely in a zone).
    char zone() const
    {
        return m_current.zone;
    }


    /// @brief Return the east-west size of the image, in meters
    size_t width() const;

    /// @brief Return the north-south size of the image, in meters
    size_t height() const;

    /// @brief Returns true iff the array has been filled with image data.
    bool isReady() const
    {
        return ! m_current.is_unused();
    }

    int as_matrix( Int_matrix* ) const;

    kjb_c::Pixel read_color( unsigned, unsigned ) const;

    kjb_c::Pixel read_abs_color( long, long ) const;

    Pixel read( unsigned, unsigned ) const;

    Pixel read_abs( long, long ) const;

    Pixel read_abs( const pt& ) const;

    /// @brief Return UTM coordinates of the center of the DOQ
    pt center() const
    {
        return make_pt(left()+width()/2, top()-height()/2, m_current.zone);
    }

    /// @brief access internal buffer, but with clunky name to discourage you
    const kjb::Image& debug_access_internal_image_buffer() const
    {
        return m_doq;
    }

    /// @brief retrieve the number of meters per pixel for chosen imagery theme
    float meters_per_pixel() const;

};


Int_matrix get_aerial_image(const pt&, size_t, size_t);

Image get_topographic_map_detail(const pt&, size_t, size_t);


} // end namespace TopoFusion
} // end namespace kjb

#endif
