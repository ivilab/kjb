
/* $Id: i_lib.c 20918 2016-10-31 22:08:27Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_lib.h"


#define IMAGE_FILE_SUFFIXES                                                    \
    {                                                                          \
        "kiff", "kif", "mid", "miff", "tiff", "tif", "viff", "gif", "gif87", "png",   \
        "ps", "eps", "sun", "ras", "jpeg", "jpg", "pcd", "pict", "pgm", "ppm", \
        "pnm", "sgi", "xbm", "xpm", "r16",                                     \
        "KIFF", "KIF", "MID", "MIFF", "TIFF", "TIF", "VIFF", "GIF", "GIF87", "PNG",  \
        "PS", "EPS", "SUN", "RAS", "JPEG", "JPG", "PCD", "PICT", "PGM", "PPM", \
        "PNM", "SGI", "XBM", "XPM", "R16",                                     \
        NULL                                              \
    }

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int get_image_file_base_path
(
    const char* file_path,
    char*       base_path,
    size_t      base_path_size,
    char*       suffix,
    size_t      suffix_size
)
{
    static const char* suffixes[ ]  = IMAGE_FILE_SUFFIXES;

    int result = get_base_path(file_path, base_path, base_path_size,
                               suffix, suffix_size, suffixes);

    if (result == ERROR)
    {
        insert_error("%q is not a valid image path.", file_path);
    }

    return result;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_image_file_base_name
(
    const char* file_name,
    char*       dir_str,
    size_t      dir_str_size,
    char*       base_name,
    size_t      base_name_size,
    char*       suffix,
    size_t      suffix_size
)
{
    static const char* suffixes[ ]  = IMAGE_FILE_SUFFIXES;

    int result = get_base_name(file_name, dir_str, dir_str_size,
                               base_name, base_name_size,
                               suffix, suffix_size, suffixes);

    if (result == ERROR)
    {
        insert_error("%q is not a valid image name.",
                     file_name);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_image_files
(
    Word_list** paths_ptr_ptr, 
    Word_list** base_names_ptr_ptr,
    const char* dir, 
    const char* suffix
)
{
    static const char* suffixes[ ]  = IMAGE_FILE_SUFFIXES;
    const int          num_suffixes = sizeof(suffixes) / sizeof(const char*);
    static Word_list*  paths_ptr_list[ sizeof(suffixes) / sizeof(const char*) ];
    static Word_list*  base_names_ptr_list[ sizeof(suffixes) / sizeof(const char*) ];
    int                i;
    int                result = NO_ERROR;
    char dot_suffix[ 100 ];


    if (suffix != NULL)
    {
        if (suffix[ 0 ] == '.') 
        {
            BUFF_CPY(dot_suffix, suffix); 
        }
        else 
        {
            BUFF_CPY(dot_suffix, "."); 
            BUFF_CAT(dot_suffix, suffix); 
        }

        return kjb_simple_glob(paths_ptr_ptr, 
                               base_names_ptr_ptr,
                               dir, dot_suffix,
                               (int (*)(const char*))NULL);
    }

    for (i = 0; i < num_suffixes; i++)
    {
        if (suffixes[ i ] == NULL) continue;

        if (suffixes[ i ][ 0 ] == '.') 
        {
            BUFF_CPY(dot_suffix, suffixes[ i ]); 
        }
        else 
        {
            BUFF_CPY(dot_suffix, "."); 
            BUFF_CAT(dot_suffix, suffixes[ i ]); 
        }

        result = kjb_simple_glob(&(paths_ptr_list[ i ]), 
                                 &(base_names_ptr_list[ i ]),
                                 dir, dot_suffix,
                                 (int (*)(const char*))NULL);

        if (result == ERROR) break;

    }

    if ((paths_ptr_ptr != NULL) && (result != ERROR))
    {
        result = concat_word_lists(paths_ptr_ptr, num_suffixes,
                                   (const Word_list * const *) paths_ptr_list);
    }

    if ((base_names_ptr_ptr != NULL) && (result != ERROR))
    {
        result = concat_word_lists(base_names_ptr_ptr, num_suffixes, 
                                   (const Word_list * const *) base_names_ptr_list);
    }

    for (i = 0; i < num_suffixes; i++)
    {
        free_word_list(paths_ptr_list[ i ]);
        paths_ptr_list[ i ] = NULL; 

        free_word_list(base_names_ptr_list[ i ]);
        base_names_ptr_list[ i ] = NULL; 
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

