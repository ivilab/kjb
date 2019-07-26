/**
 * @file
 * @brief Constants and definitions for TopoFusion tile code.
 * @author Scott Morris
 * @author Alan Morris
 *
 * This file is really kept as pure C code; nothing is plus plus.
 * Originally from TopoFusion code.
 */
/*
 * $Id: master.h 22174 2018-07-01 21:49:18Z kobus $
 */

#ifndef MASTER_H_INCLUDED_UOFARIZONAVISION
#define MASTER_H_INCLUDED_UOFARIZONAVISION

/* C2MAN */
#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
namespace TopoFusion {
#endif
#endif



/*
#define BILINEAR            0
#define BICUBIC_SMOOTH      1
#define BICUBIC_ACCURATE    2
#define GET_ELE_FROM_TRACKS 3

#define DISP_2D         0
#define DISP_GENERATING 1
#define DISP_3D         2

#define WP_NAME         0
#define WP_COMMENT      1
#define WP_DESC         2
#define WP_SYM          3
*/


/*      init stuff          */
int init_master( const char* map_cache_path );


enum
{
    TILE_SIZE = 200 /**< edge length, in pixels, of each (square) tile */
};

/** index value for TileSet table. */
enum TileSetId
{
    TOPO_2M,   /**< topographic diagram, 2 m per pixel resolution */
    TOPO_4M,   /**< topographic diagram, 4 m per pixel resolution */
    TOPO_8M,   /**< topographic diagram, 8 m per pixel resolution */
    TOPO_16M,  /**< topographic diagram, 16 m per pixel resolution */
    TOPO_32M,  /**< topographic diagram, 32 m per pixel resolution */
    TOPO_64M,  /**< topographic diagram, 64 m per pixel resolution */
    TOPO_256M, /**< topographic diagram, 256 m per pixel resolution */
    TOPO_512M, /**< topographic diagram, 512 m per pixel resolution */

    AIR_1M,    /**< aerial imagery, 1 m per pixel resolution (use this one) */
    AIR_4M,    /**< aerial imagery, 4 m per pixel resolution */
    AIR_16M,   /**< aerial imagery, 16 m per pixel resolution */
    AIR_64M,   /**< aerial imagery, 64 m per pixel resolution */

    URBAN_POINT25M, /**< urban (color) aerial imagery, 25 cm per pixel res */
    URBAN_1M,  /**< urban (color) aerial imagery, 1 m per pixel res */
    URBAN_4M,  /**< urban (color) aerial imagery, 4 m per pixel res */
    URBAN_16M, /**< urban (color) aerial imagery, 16 m per pixel res */
    URBAN_64M, /**< urban (color) aerial imagery, 64 m per pixel res */

    NASA_16M,
    NASA_64M,
    NASA_256M,

    COMBO_1M,
    COMBO_4M,
    COMBO_16M,
    COMBO_64M,

    NO_MAP,                     /**< sentinel value for the "not-a" tileset */
    NUM_TILESETS,               /**< number of tilesets                     */

    TOPO_MIN =  TOPO_2M,        /**< min value of TOPO_* tilesets   */
    TOPO_MAX =  TOPO_512M,      /**< max value of TOPO_* tilesets   */
    AIR_MIN =   AIR_1M,         /**< min value of AIR_* tilesets    */
    AIR_MAX =   AIR_64M,        /**< max value of AIR_* tilesets    */
    URBAN_MIN = URBAN_POINT25M, /**< min value of URBAN_* tilesets  */
    URBAN_MAX = URBAN_64M,      /**< max value of URBAN_* tilesets  */
    NASA_MIN =  NASA_16M,       /**< min value of NASA_* tilesets   */
    NASA_MAX =  NASA_256M,      /**< max value of NASA_* tilesets   */
    COMBO_MIN = COMBO_1M,       /**< min value of COMBO_* tilesets  */
    COMBO_MAX = COMBO_64M,      /**< max value of COMBO_* tilesets  */

    TILESET_ID_MIN = TOPO_MIN   /**< min value of all tileset codes */
};


/*
#define OK 0
#define ERR 1
*/

/** definition of st_TileSource */
typedef struct
{
    char yoffset;           /**< written to but never read -- just ignore!  */
    char s_val;             /**< "scale," 10 + log_2( meters_per_pixel )    */
    char t_val;             /**< "theme": photoimagery, topo drawing, etc.  */
    float metersPerPixel;   /**< resolution, in meters per pixel            */
    long UTM_Size;          /**< apparently eq. to tile edge size in meters */

    /*float deg;    degrees per pixel.. */
    /*bool active;  whether the tileset should be displayed/downloaded */
} st_TileSource;             /**< TopoFusion descr. of source of imagery    */

/** global table of imagery source */
extern st_TileSource TileSource[ NUM_TILESETS ];

/**
 * tileset mapping, we have this because we initially didn't use 2, 8, and 16.
 * We want the numbers to go in order from 2,4,8,16,32, etc, but everyone's
 * index file has 4, 16, 64 as 0, 1, 2 for tileset, so we need to map them back
 * to their old tileset number, then we add 2, 8, and 16 on the end.
 */
extern int tileMapping[ NUM_TILESETS ];

/** global name of index file covering tile storage (abs. path + filename)  */
extern char gTheIndexFileName[2048];

/** abs. path + filename prefix of map data files */
extern char gTheFileName[2048];


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
} /* namespace TopoFusion */
} /* namespace kjb_c */
#endif
} /* extern "C" */
#endif



#endif  /* MASTER_H_INCLUDED_UOFARIZONAVISION */
