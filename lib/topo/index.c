/**
 * @file
 * @brief Functions and structures specific to file and memory pool indexing
 * @author Scott Morris
 * @author Alan Morris
 *
 * Originally from TopoFusion.
 *
 * This file is C for real, just like the filename indicates.
 */

/*
 * $Id: index.c 22174 2018-07-01 21:49:18Z kobus $
 *
 * Recommended tab width:  4
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_def.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_str.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_io.h"
#include "l/l_error.h"
#include "l/l_global.h"
#include "l/l_debug.h"
#include "l/l_string.h"

#include "topo/master.h"
#include "topo/index.h"

#ifdef __cplusplus
extern "C" {
#endif


#define startIndexSize  500     /**< num slots of index entries in new file */

#define VERSION_STAMP "INDEX32" /**< write this at the top of the index file */


/**
 * @brief definition of indexEntry
 */
typedef struct
{
    int     x;      /**< x coordinate of location (UTM easting)             */
    int     y;      /**< y coordinate of location (UTM northing)            */
    int     file;   /**< index of file storing the tile                     */
    int     offset; /**< byte offset within file of the tile                */
    int     size;   /**< size in bytes of the tile                          */
    char    typ;    /**< tile type (identifies its parent tileset)          */
    char    zone;   /**< zone of the location (UTM zone)                    */
} indexEntry;   /**< data structure used for cache of aerial imagery tiles. */


/**
 * @brief the tileset mapping
 *
 * we have this because we initially didn't use 2, 8, and 16.  We want the
 * numbers to go in order from 2,4,8,16,32, etc, but everyone's index file has
 * 4,16,64 as 0,1,2 for tileset, so we need to map them back to their old
 * tileset number, then we add 2,8, and 16 on the end
 */
extern int tileMapping[NUM_TILESETS];

#if 0
static indexEntry *sortedIndex = NULL;      /**< The sorted file Index */
static int numSortedIndexEntries = 0;       /**< Num of sorted entries loaded*/
#endif

static indexEntry *fileIndex = NULL;    /**< stores all entries of tiles    */
static int numIndexEntries = 0;         /**< Number of entries loaded       */
static int numMapFiles;                 /**< Number of MapXX.dat files in use*/
static int indexLimit;                  /**< Upper limit on fileIndex array */
static FILE *theIndexFile;              /**< File pointer for MapIndex.dat  */
static FILE **mapFiles=NULL;            /**< Arr. of ptrs for MapXX.dat files*/



static void gen_map_fn( char* buf, size_t bsz, int index )
{
    kjb_sprintf( buf, bsz, "%s%sMaps%d.dat", gTheFileName, DIR_STR, index );
}

#define GEN_MAP_FN(buf, index) gen_map_fn((buf), sizeof(buf), index)



static int initialize_existing_index(void)
{
    off_t fsize_o;
    int fsize_i;
    long rsize;
    char buf[8], mapFileName[4096];

    int i=0, STAMP_SIZE = signed_strlen(VERSION_STAMP);
    size_t ct = kjb_fread(theIndexFile, buf, sizeof(buf));

    ASSERT(STAMP_SIZE + 1 == sizeof(buf));
    if ( ct != sizeof(buf) ) return ERROR;
    buf[STAMP_SIZE] = '\0';

    /* Check if the index is the old kind. */
    if (kjb_strcmp(buf, VERSION_STAMP) != EQUAL_STRINGS)
    {
        /* need to convert */
        add_error("Index file has the wrong prefix (it might be corrupt).");
        return ERROR;
    }

    /* Get the size of the file. */
    ERE(fp_get_byte_size(theIndexFile, &fsize_o));
    if ( fsize_o < STAMP_SIZE )
    {
        add_error("Index file is too small:  expected size at least %d, "
                "actual size was %ld\n", STAMP_SIZE, (long) fsize_o);
        return ERROR;
    }
    ASSERT(fsize_o < INT_MAX);
    fsize_i = (int)fsize_o - STAMP_SIZE;

    numIndexEntries = fsize_i / sizeof(indexEntry);
    /*TEST_PSE(("Number of tiles in index = %d / %d = %d\n",fsize_i,sizeof(indexEntry),numIndexEntries)); */

    if (fsize_i % sizeof(indexEntry) != 0)
    {
        /* Force file to be a multiple of sizeof(IndexEntry). */
        indexEntry dummy = { -1, -1, 0, 0, 0, '\0', '\0' };
        ERE(kjb_fseek(theIndexFile,
                STAMP_SIZE + numIndexEntries * sizeof(indexEntry), SEEK_SET));
        ERE(kjb_fwrite(theIndexFile, &dummy, sizeof(indexEntry)));
    }

    /* Seek to the front of the payload (just beyond the version stamp). */
    ERE(kjb_fseek(theIndexFile, STAMP_SIZE, SEEK_SET));

    /* Make a sequential index. */
    indexLimit = numIndexEntries*2;
    if (indexLimit <= 0)
    {
        indexLimit = startIndexSize;
    }
    ASSERT( fileIndex==NULL );
    NRE( fileIndex = N_TYPE_MALLOC( indexEntry, indexLimit ));

    /* Read the entire index in one fread, plus error-checking. */
    rsize = kjb_fread( theIndexFile, fileIndex,
                                sizeof(indexEntry) * numIndexEntries );
    if (rsize != (long)sizeof(indexEntry) * numIndexEntries)
    {
        TEST_PSE(("initIndexFile: rsize != numIndexEntries"));
        add_error("fread failure reading index: requested %ld, received %ld",
                (long)sizeof(indexEntry) * numIndexEntries, rsize);
        kjb_free( fileIndex );
        fileIndex=NULL;
        return ERROR;
    }

    /* Find the last map file referenced */
    for (i=0; i<numIndexEntries; ++i)
    {
        if (fileIndex[i].file > numMapFiles)
        {
            numMapFiles = fileIndex[i].file;
        }
    }

    /* add one (first is Maps0.dat) and allocate array of FILE ptrs. */
    numMapFiles += 1;
    ASSERT( mapFiles==NULL );
    mapFiles = N_TYPE_MALLOC( FILE*, numMapFiles );
    if ( NULL == mapFiles )
    {
        kjb_free( fileIndex );
        fileIndex=NULL;
        add_error("unable to malloc file ptrs for DOQ tile index.");
        NRE(mapFiles);
    }

    /* Test for existence of each map file.
     * If one of the files is missing we would like to find out now, before we
     * open any of them.
     */
    for (i=0; i<numMapFiles; ++i)
    {
        GEN_MAP_FN( mapFileName, i );
        if (! is_file(mapFileName))
        {
            kjb_free( mapFiles );
            mapFiles = NULL;
            kjb_free( fileIndex );
            fileIndex = NULL;
            return ERROR;
        }
    }

    /* Open each map file. */
    for (i=0; i<numMapFiles; ++i)
    {
        GEN_MAP_FN( mapFileName, i );
        if ((mapFiles[i] = kjb_fopen(mapFileName, "ab+")) == NULL)
        {
            TEST_PSE(("Error opening %s for appending", mapFileName));
            add_error("Error opening %s for appending", mapFileName);
            while( i ) kjb_fclose( mapFiles[--i] );
            kjb_free( mapFiles );
            mapFiles = NULL;
            kjb_free( fileIndex );
            fileIndex = NULL;
            return ERROR;
        }
    }

    return NO_ERROR;
}


static int initialize_new_index(void)
{
    /* the file, gTheIndexFileName, does not exist */
    char mapFileName[4096];

    /* verify that the index stuff is in a sensible (closed) state. */
    if ( fileIndex != NULL || mapFiles != NULL )
    {
        add_error("Corrupt internal state in initialize_new_index()");
        return ERROR;
    }

    /* create a new index */
    indexLimit = startIndexSize;
    NRE(fileIndex = N_TYPE_MALLOC( indexEntry, indexLimit ));
    numIndexEntries = 0;

    /* Create ptr array for map file (an array of one element). */
    if (NULL == (mapFiles = TYPE_MALLOC(FILE*)))
    {
        add_error("Bad malloc opening map files' pointer array");
        kjb_free( fileIndex );
        fileIndex = NULL;
        return ERROR;
    }
    numMapFiles = 1;
    GEN_MAP_FN( mapFileName, 0 );

    /* Create a new index file, initialize it, and open the map file. */
    if (    NULL == (theIndexFile = kjb_fopen(gTheIndexFileName, "wb"))
         || kjb_fputs(theIndexFile, VERSION_STAMP) != strlen(VERSION_STAMP)
         || NULL == (mapFiles[0] = kjb_fopen(mapFileName, "ab+"))
       )
    {
        TEST_PSE(("Error opening map cache\n", gTheIndexFileName));
        add_error("Error opening map cache");
        numMapFiles = 0;
        kjb_free( mapFiles );
        mapFiles = NULL;
        kjb_free( fileIndex );
        fileIndex = NULL;
        EPE(kjb_fclose(theIndexFile)); /* ok to do this, even if NULL */
        theIndexFile = NULL;
        EPE(kjb_unlink(gTheIndexFileName));
        return ERROR;
    }

    return NO_ERROR;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/*
 * ============================================================================
 *                             initIndexFile
 *
 * Initialize the tile index file
 *
 * Initialized the index file, sets up converting if necessary.
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Returns:
 *    On success, returns NO_ERROR.  Otherwise returns ERROR and sets an
 *    error message.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int initIndexFile( void )
{
    int rc;

    if (fileIndex)
    {
        add_error("Attempted to double-open the TopoFusion index file.");
        return ERROR;
    }

    numMapFiles = 0;

    /* Try to open index file for reading and writing,
     * implicitly testing its existence.
     * If it does not exist, we will create a fresh one.
     */
    theIndexFile = kjb_fopen( gTheIndexFileName, "rb+" );

    rc = theIndexFile ? initialize_existing_index() : initialize_new_index();

    if (ERROR == rc)
    {
        /* It is safe, and simpler, to assume theIndexFile isn't NULL here. */
        kjb_fclose(theIndexFile);
        theIndexFile = NULL;
        add_error("Unable to initialize the index file");
        ERE(rc);
    }
    return NO_ERROR;
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             closeIndexFile
 *
 * Close the tile index file
 *
 * Closes up File index stuff; safe to do twice
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
void closeIndexFile( void )
{

    int iii;

    if ( numMapFiles )
    {
        ASSERT( mapFiles );
        for( iii = 0; iii < numMapFiles; ++iii )
        {
            EPE( kjb_fclose( mapFiles[ iii ] ) );
        }
        kjb_free( mapFiles );
        mapFiles = NULL;
        numMapFiles = 0;
    }

    if ( theIndexFile )
    {
        EPE( kjb_fclose( theIndexFile ) );
        theIndexFile = NULL;
    }

    kjb_free( fileIndex );
    fileIndex = NULL;
#if 0
    kjb_free( sortedIndex );
    sortedIndex = NULL;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             addIndexEntry
 *
 * Adds a tile to the cache and index
 *
 * Adds tile specified to MapsXX.dat, MapIndex.dat, and the Index.
 * Index is in memory.  Doubles fileIndex if necessary.
 * Creates new MapsXX.dat if current one is over 625MB.
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * FIXME: This function is liable to leak resources.
 *
 * Returns:
 *    On success, returns value in 'buflen' otherwise -1.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int addIndexEntry(
    int x,            /* Tile-size easting of location added to index  */
    int y,            /* Tile-size northing of location added to index */
    char typ,         /* Tileset type of this tile                     */
    char zone,        /* UTM zone of location                          */
    const char *buf,  /* buffer containing tile image data             */
    int buflen        /* length of buffer 'buf'                        */
)
{
    long hsize1, hsize2, offset=0, wsize=0;
    char header[1024];

    typ = tileMapping[(int)typ];

/*  TEST_PSE(("Adding index Entry for x=%d, y=%d, size=%d\n",x,y,buflen)); */

    if (numIndexEntries>=indexLimit-3)
    {  /* grow the array times two */
        /*TEST_PSE(("Adding index Entry: Doubling index\n"));*/
        indexEntry *temp = fileIndex;
        fileIndex = N_TYPE_MALLOC( indexEntry, indexLimit*2 );
        ASSERT( fileIndex );
        kjb_memcpy(fileIndex, temp, sizeof(indexEntry)*indexLimit);
        kjb_free(temp);
        indexLimit*=2;
    }


/*  TEST_PSE(("Position = %d\n",numIndexEntries)); */
    fileIndex[numIndexEntries].x = x;
    fileIndex[numIndexEntries].y = y;
    fileIndex[numIndexEntries].typ = typ;
    fileIndex[numIndexEntries].zone = zone;

    kjb_fseek(mapFiles[numMapFiles-1],0,SEEK_END);
    offset = kjb_ftell(mapFiles[numMapFiles-1]);
    ASSERT( offset < INT_MAX );

    EPE( offset );
    if ( ERROR == offset ) return -1;

    /* check if we are over 625MB, if so, branch off to a new map file */
    if (offset > 625L * 1024L * 1024L)
    {
        FILE **temp = mapFiles;
        char mapFileName[4096];
        TEST_PSE(("Maps.dat over 625MB, making new one\n"));
        numMapFiles++;
        mapFiles = N_TYPE_MALLOC( FILE*, numMapFiles );
        kjb_memcpy(mapFiles, temp, sizeof(FILE *)*(numMapFiles-1));
        kjb_free(temp);

        GEN_MAP_FN( mapFileName, numMapFiles-1 );

        if ((mapFiles[numMapFiles-1] = kjb_fopen(mapFileName,"ab+")) == NULL)
        {
            TEST_PSE(("Error opening %s",mapFileName));
            return -1;
        }
        offset = 0;
    }


    fileIndex[numIndexEntries].file = numMapFiles-1;
    fileIndex[numIndexEntries].offset = (int) offset;
    fileIndex[numIndexEntries].size = buflen;

/*  TEST_PSE(("fileIndex[%d].offset=%d\n",numIndexEntries,offset)); */

    /* write file index header, first to a string and then to the file */
    hsize1 = kjb_sprintf(header, sizeof(header),
                        "BEGIN TILE x=%d y=%d z=%d s=%d ", x, y, zone, typ);
    EPE( hsize1 );
    if ( ERROR == hsize1 ) return -1;
    hsize2 = kjb_fputs( mapFiles[ numMapFiles-1 ], header );
    EPE( hsize2 );
    if ( ERROR == hsize2 || hsize1 != hsize2 ) return -1;
    ASSERT( 0 < hsize1 && hsize1 < INT_MAX );
    if ( buflen <= (int) hsize1 )
    {
        TEST_PSE(( "Buffer (length %d) is too small for index header\n", buflen ));
        return -1;
    }

    /* AMP:  You might wonder why the following is OK.  It looks to me that
     * every downloaded tile buffer has a lengthy set of (irrelevant) headers
     * at the front, and the following clobbers some of it.  In the tiles that
     * I've observed, hsize1 is always much smaller than the headers.
     *
     * Follow up question:  But, why even do this at all?  A.:  It is probably
     * unnecessary but I'm doing it because Scott did something equivalent in
     * his original code and I wanted to follow suit.  But I had to reimplement
     * it to add const correctness to buf.  (Originally he just sprintf'd the
     * header onto the front of buf, and wrote the partially-clobbered buf to
     * the file).  Actually you could probably chop off many more bytes of
     * irrelevant trash from the front of the tile, and save space, but I'm not
     * sure how deep to cut.
     */
    buf += hsize1;
    buflen -= (int) hsize1;

    /* write the front-trimmed buf contents to the file */
    wsize = kjb_fwrite(mapFiles[numMapFiles-1], buf, buflen);
    if (wsize != buflen)
    {
        TEST_PSE(("Unable to write to Maps.dat!\n"));
        return -1;
    }

    kjb_fseek(theIndexFile,0,SEEK_END);
    wsize = kjb_fwrite( theIndexFile, &fileIndex[numIndexEntries],
                                                        sizeof(indexEntry));
    if (wsize != sizeof(indexEntry))
    {
        TEST_PSE(("Unable to write to MapIndex.dat!\n"));
        return -1;
    }

    numIndexEntries++;

    return buflen + hsize1;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             findTileOnDisk
 *
 * Looks for specified tile in the file Index
 *
 * This looks for specified tile in the file Index using linear time search.
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Returns:
 *    TRUE if found, FALSE otherwise
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int findTileOnDisk(
    int x,     /* UTM easting of location to test for tile presence  */
    int y,     /* UTM northing of location to test for tile presence */
    int typ,   /* Tileset of the tile we seek                        */
    char zone  /* UTM zone of location to test for tile presence     */
)
{
    int i;
    typ = tileMapping[typ];

    /* search the sequential 'fileIndex' */

    for (i=0;i<numIndexEntries;i++)
    {
        if  (       fileIndex[i].x==x
                &&  fileIndex[i].y==y
                && fileIndex[i].typ==typ
                && fileIndex[i].zone==zone
            )
        {
            return TRUE;
        }
    }

    /* not found, we don't have this tile on disk */
    return FALSE;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             getTileFromDisk
 *
 * Read a tile from file cache
 *
 * Retreives specified tile from disk, places it in 'buf'
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Returns:
 *    Return value is the size of the tile in bytes, if found.
 *    If not found, or if another error occurs, returns 0.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int getTileFromDisk(
    int x,         /* UTM easting of location whose tile we want      */
    int y,         /* UTM northing of location whose tile we want     */
    int typ,       /* Tileset of tile we want                         */
    char zone,     /* UTM zone of location whose tile we want         */
    char *buf,     /* buffer into which to write the tile image data  */
    size_t bufsize /* size of buffer 'buf' */
)
{
    int i;
    typ = tileMapping[typ];

    /*TEST_PSE(("x=%d, y=%d\n", x, y)); */

    /* search the sequential 'fileIndex' */

    for (i=0;i<numIndexEntries;i++)
    {
        if (   fileIndex[i].x==x && fileIndex[i].y==y
            && fileIndex[i].typ==typ && fileIndex[i].zone==zone
           )
        {
            size_t readsize = 0, fiisize = 0;
            kjb_fseek(mapFiles[fileIndex[i].file],fileIndex[i].offset,SEEK_SET);

            /* found it at position 'i' */
            fiisize = fileIndex[i].size;
            if ( bufsize < fiisize )
            {
                TEST_PSE(( "ERROR:  buffer supplied to %s is too small.\n", __func__ ));
                return 0;
            }

            readsize = kjb_fread(mapFiles[fileIndex[i].file], buf, fiisize);

            if (readsize != fiisize)
            {
                TEST_PSE(("getTileFromDisk: Bad Read: readsize = %d, filesize = %u\n", readsize, fiisize));

                EPE(invalidateEntry(x,y,typ,zone));
                return 0;
            }

            return readsize;
        }
    }

    /* not found, we don't have this tile on disk */
    return 0;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             invalidateEntry
 *
 * Mark a file cache entry as invalid
 *
 *   invalidates entry for a tile in both the fileIndex and the memory pool
 *   goes back into MapIndex.dat and overwrites offending entry.
 *
 * This and all file cache functions are NOT reentrant, because they access
 * static structures.
 *
 * Returns:
 *    NO_ERROR if the tile is found in the cache, otherwise ERROR.
 *
 * Author:  Scott Morris
 *
 * Documenter: Andrew Predoehl
 *
 * Index: aerial imagery
 *
 * ----------------------------------------------------------------------------
*/
int invalidateEntry(
    int x,     /* UTM easting location of the tile to invalidate  */
    int y,     /* UTM northing location of the tile to invalidate */
    int typ,   /* Tileset identifier of tile to invalidate */
    char zone  /* UTM zone of the location chosen whose tile to invalidate */
)
{
    int i;
    long nbytes;
    int numSortedIndexEntries = 0;

    typ = tileMapping[typ];

    for (i=0; i<numIndexEntries; i++)
    {
        if (    fileIndex[i].x==x && fileIndex[i].y==y
            &&  fileIndex[i].typ==typ && fileIndex[i].zone==zone
           )
        {
            fileIndex[i].x = -fileIndex[i].x;
            fileIndex[i].y = -fileIndex[i].y;
            fileIndex[i].zone = -fileIndex[i].zone;

            ERE( kjb_fseek(theIndexFile,
                 7+((numSortedIndexEntries+i)*sizeof(indexEntry)), SEEK_SET) );
            nbytes=kjb_fwrite(theIndexFile, &fileIndex[i], sizeof(indexEntry));

            if ((long)sizeof(indexEntry) != nbytes)
            {
                /* If nbytes equals ERROR we can return right now . . . */
                ERE(nbytes);
                /* otherwise we ought to explain the problem before we return.
                 */
                add_error("fwrite fail in invalidateEntry (attempted %u, "
                        "actually wrote %ld)", sizeof(indexEntry), nbytes);
                return ERROR;
            }
            break;
        }
    }

    /*
     * The following invariant should be true, unless somehow we called
     * invalidateEntry for a tile that we don't have.
     */
    return ( i < numIndexEntries ) ? NO_ERROR : ERROR;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

