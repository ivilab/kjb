/**
 * @file
 * @brief major TopoFusion routines and globals
 * @author Scott Morris
 * @author Alan Morris
 *
 * Originally from TopoFusion code.
 */

/*
 * $Id: master.c 20654 2016-05-05 23:13:43Z kobus $
 */

#include <l/l_sys_io.h>
#include <l/l_sys_err.h>
#include <l/l_sys_debug.h>
#include <l/l_error.h>
#include <l/l_string.h>
#include <l/l_global.h>

#include <topo/master.h>
#include <topo/index.h>

#ifdef __cplusplus
extern "C" {
#endif


/** basename of file used to store image cache          */
#define MAPDAT          "maps.dat"

/** basename of index file used to organize image cache */
#define MAPINDEX        "mapindex.dat"

int tileMapping[NUM_TILESETS];

char gTheFileName[2048];

char gTheIndexFileName[2048];

st_TileSource TileSource[NUM_TILESETS];


static int InitTileEngine( void )
{

    /*CRect rect; */

    /*GetClientRect(rect); */
    /*ClientToScreen(rect); */
    /*m_left_reset = (m_surface_width - rect.Width) / 2 */

    int i;

    /*TileSet = RELIEF_1KM; */

    TileSource[ AIR_1M      ].s_val = 10;
    TileSource[ AIR_4M      ].s_val = 12;
    TileSource[ AIR_16M     ].s_val = 14;
    TileSource[ AIR_64M     ].s_val = 16;

    TileSource[ URBAN_POINT25M].s_val = 8;
    TileSource[ URBAN_1M    ].s_val = 10;
    TileSource[ URBAN_4M    ].s_val = 12;
    TileSource[ URBAN_16M   ].s_val = 14;
    TileSource[ URBAN_64M   ].s_val = 16;

    TileSource[ COMBO_1M    ].s_val = 10;
    TileSource[ COMBO_4M    ].s_val = 12;
    TileSource[ COMBO_16M   ].s_val = 14;
    TileSource[ COMBO_64M   ].s_val = 16;


    TileSource[ TOPO_2M     ].s_val = 11;
    TileSource[ TOPO_4M     ].s_val = 12;
    TileSource[ TOPO_8M     ].s_val = 13;
    TileSource[ TOPO_16M    ].s_val = 14;
    TileSource[ TOPO_32M    ].s_val = 15;
    TileSource[ TOPO_64M    ].s_val = 16;
    TileSource[ TOPO_256M   ].s_val = 18;
    TileSource[ TOPO_512M   ].s_val = 19;

    TileSource[ NASA_16M    ].s_val = 14;
    TileSource[ NASA_64M    ].s_val = 16;
    TileSource[ NASA_256M   ].s_val = 18;
    /*TileSource[NASA_1024M].s_val = 20; */

    /*TileSource[NASA_16M].active =
    TileSource[NASA_64M].active =
    TileSource[NASA_256M].active = true;*/
    /*TileSource[NASA_1024M].active = true; */

    /* temp code!!! */

    /*TileSource[COMBO_1M].active =
    TileSource[COMBO_4M].active =
    TileSource[COMBO_16M].active =
    TileSource[COMBO_64M].active = true;*/

    tileMapping[ TOPO_4M    ] = 0;
    tileMapping[ TOPO_16M   ] = 1;
    tileMapping[ TOPO_64M   ] = 2;
    tileMapping[ TOPO_256M  ] = 3;
    tileMapping[ TOPO_512M  ] = 4;

    tileMapping[ AIR_1M     ] = 5;
    tileMapping[ AIR_4M     ] = 6;
    tileMapping[ AIR_16M    ] = 7;
    tileMapping[ AIR_64M    ] = 8;

    tileMapping[ TOPO_2M    ] = 9;
    tileMapping[ TOPO_8M    ] = 10;
    tileMapping[ TOPO_32M   ] = 11;

    tileMapping[ URBAN_POINT25M] = 12;
    tileMapping[ URBAN_1M   ] = 13;
    tileMapping[ URBAN_4M   ] = 14;
    tileMapping[ URBAN_16M  ] = 15;
    tileMapping[ URBAN_64M  ] = 16;

    tileMapping[ NASA_16M   ] = 17;
    tileMapping[ NASA_64M   ] = 18;
    tileMapping[ NASA_256M  ] = 19;
/*  tileMapping[ NASA_1024M] = 20; */



    for (i=TOPO_MIN;i<=TOPO_MAX;i++)
    {
        TileSource[i].yoffset = 0;
        TileSource[i].t_val = 2;
    }

    for (i=URBAN_MIN;i<=URBAN_MAX;i++)
    {
        TileSource[i].yoffset = 0;
        TileSource[i].t_val = 4;
    }

    /* set offsets */
    for (i=AIR_MIN;i<=AIR_MAX;i++)
    {
        TileSource[i].yoffset = 0;
        TileSource[i].t_val = 1;
    }

    TileSource[AIR_1M].yoffset = -1;
    TileSource[COMBO_1M].yoffset = -1;

    for( i = 0; i < NUM_TILESETS; ++i )
    {
        ASSERT( 0 == TileSource[i].s_val || 8 <= TileSource[i].s_val );
        ASSERT( TileSource[i].s_val != 9 );
        if ( 0 == TileSource[i].s_val )
        {
            TileSource[ i ].metersPerPixel = 0;
        }
        else if ( 8 == TileSource[i].s_val )
        {
            TileSource[ i ].metersPerPixel = 0.25;
        }
        else
        {
            unsigned short mpp = 1 << ( TileSource[ i ].s_val - 10 );
            ASSERT((double) mpp == pow( 2, TileSource[ i ].s_val - 10 ) );
            TileSource[i].metersPerPixel = mpp;
        }
        TileSource[i].UTM_Size = TILE_SIZE * TileSource[i].metersPerPixel;
    }

    TileSource[NO_MAP].UTM_Size = 200;
    TileSource[NO_MAP].metersPerPixel  = 1;

    return NO_ERROR;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             init_master
 *
 * Open the file cache and index
 *
 * Initialize the file and memory caches of USGS DOQ tiles.
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Returns:
 *    NO_ERROR on success, otherwise ERROR.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int init_master( const char* map_cache_path )
{
    const size_t p_len = strlen( map_cache_path ),
                 need_dat = strlen( MAPDAT ) + p_len + strlen( DIR_STR ),
                 need_ix = strlen( MAPINDEX ) + p_len + strlen( DIR_STR );

    if  (       sizeof(gTheFileName) <= need_dat + 1 
            ||  sizeof(gTheIndexFileName) <= need_ix + 1
        )
    {
        set_error( "init_master() received a pathname that is too long" );
        return ERROR;
    }

    /* Append the given path to the two global filename things */
    BUFF_CPY( gTheFileName, map_cache_path );
    BUFF_CPY( gTheIndexFileName, map_cache_path );

    /* Does the given path end in a slash? */
    if ( 0 < p_len && map_cache_path[ p_len-1 ] != DIR_CHAR )
    {
        BUFF_CAT( gTheFileName, DIR_STR );
        BUFF_CAT( gTheIndexFileName, DIR_STR );
    }

    /* Append the canonical names as suffixes */
    BUFF_CAT( gTheFileName, MAPDAT );
    BUFF_CAT( gTheIndexFileName, MAPINDEX );

    ERE( InitTileEngine() );
    ERE( initIndexFile() );
    return NO_ERROR;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
