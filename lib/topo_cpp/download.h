/**
 * @file
 * @brief Shared declarations for DOQ tile-downloading code (from TopoFusion).
 * @author Scott Morris
 * @author Alan Morris
 *
 * @warning Apparently this no longer works:  msrmaps.com seems unresponsive.
 */
/*
 * $Id: download.h 20300 2016-01-29 19:40:59Z predoehl $
 */

#ifndef DOWNLOAD_H_TOPOFUSION_INCLUDED
#define DOWNLOAD_H_TOPOFUSION_INCLUDED

namespace kjb
{
namespace TopoFusion
{

/// @brief data structure for downloaded UTM tiles, like a pt
struct tile_entry
{
    int x;          ///< easting
    int y;          ///< northing
    char tileset;   ///< also known as "type"
    char zone;      ///< UTM zone

    /** @brief ctor to fill in the fields */
    tile_entry( int xx, int yy, char ts, char zz )
    :   x(xx),
        y(yy),
        tileset(ts),
        zone(zz)
        /* ,isLoading(ld), count(ct),next(nn) */
    {}

};

/**
 * @brief download a 200x200 terraserver tile using the internet.
 * @param[in]  entry      UTM-style location, and database-number, to fetch
 * @param[out] buffer     region of bytes where your tile will be written.
 * @param[in]  bufsize    size of buffer -- around 500000 should be enough.
 * @return If successful, result is positive, equal to the number of
 *         characters written to buffer.  If unsuccessful, result is negative,
 *         indicating the type of problem (see source for details).
 * @warning This function is not thread-safe; it uses lots of static data.
 *
 * A typical tile is somewhat less than 10 kilobytes, but why be stingy?
 * Scott chose a safety factor of 50 in the trails code.  That means buffer is
 * 500000 chars in size, and bufsize contains the value 500000, in the trails
 * code.
 */
int download_tile ( const tile_entry *entry, char *buffer, int bufsize );

} // end namespace TopoFusion
}

#endif /* DOWNLOAD_H_TOPOFUSION_INCLUDED */
