
/* $Id: i_convert.c 20697 2016-06-12 00:22:46Z kobus $ */

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
#include "i/i_convert.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int convert_image_file_to_raster
(
    const char* in_file_name_and_sub_image,
    const char* out_file_name
)
{
    char         exec_string[ 1000 ];
    static const char* convert_programs[ ]  = {"convert +compress -type truecolor",  "convert -type truecolor", "convert" };
    static const char* target_prefixes[ ]   = { "", "sun:" };
    char         in_file_name_and_sub_image_copy[ MAX_FILE_NAME_SIZE ];
    char*        in_file_name_and_sub_image_pos;
    char         in_file_name[ MAX_FILE_NAME_SIZE ];
    char         in_sub_image[ MAX_FILE_NAME_SIZE ];
    char         expanded_in_file_name[ MAX_FILE_NAME_SIZE ];
    char         expanded_in_file_name_and_sub_image[ MAX_FILE_NAME_SIZE ];
    int          num_convert_programs = sizeof(convert_programs)/sizeof(char*);
    int          i;
    int          result;


    BUFF_CPY(in_file_name_and_sub_image_copy, in_file_name_and_sub_image);
    in_file_name_and_sub_image_pos = in_file_name_and_sub_image_copy;
    BUFF_GEN_GET_TOKEN(&in_file_name_and_sub_image_pos, in_file_name, "[");
    BUFF_CPY(in_sub_image, in_file_name_and_sub_image_pos);

    ERE(BUFF_REAL_PATH(in_file_name, expanded_in_file_name));

    BUFF_CPY(expanded_in_file_name_and_sub_image, expanded_in_file_name);
    BUFF_CAT(expanded_in_file_name_and_sub_image, in_sub_image);

    for (i=0; i<num_convert_programs; i++)
    {
        EPETE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s %s%s",
                          convert_programs[ i ],
                          expanded_in_file_name_and_sub_image,
                          target_prefixes[ i ], out_file_name));

        result = kjb_system(exec_string);

        if (    (result == NO_ERROR)
             && (get_path_type(out_file_name) == PATH_IS_REGULAR_FILE)
           )
        {
            verbose_pso(5, "Conversion succeeded: %s.\n", exec_string);
            return NO_ERROR;
        }
    }

    set_error("Failure converting image in %s with all conversion programs.", 
             expanded_in_file_name_and_sub_image);
    add_error("Tried \"%s\"", convert_programs[ 0 ]);

    for (i=1; i<num_convert_programs; i++)
    {
        cat_error(", \"%s\"", convert_programs[ i ]);
    }

    cat_error(" in PATH.");

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int convert_image_file_from_raster
(
    const char* in_file_name,
    const char* out_file_name
)
{
    char               exec_string[ 1000 ];
    static const char* convert_programs[ ]  = { "convert +compress -type truecolor", "convert -type truecolor", "convert" };
    int                num_convert_programs = sizeof(convert_programs) /sizeof(char*);
    int                i;
    int                result;


    for (i=0; i<num_convert_programs; i++)
    {
        ERE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s %s",
                        convert_programs[ i ], in_file_name, out_file_name));

        result = kjb_system(exec_string);

        if (    (result == NO_ERROR)
             && (get_path_type(out_file_name) == PATH_IS_REGULAR_FILE)
           )
        {
            verbose_pso(5, "Conversion succeeded : %s.\n", exec_string);
            return NO_ERROR;
        }
    }

    set_error("Failure converting image with all conversion programs.");
    add_error("Tried \"%s\"", convert_programs[ 0 ]);

    for (i=1; i<num_convert_programs; i++)
    {
        cat_error(", \"%s\"", convert_programs[ i ]);
    }

    cat_error(".");

    return ERROR;
}


#ifdef __cplusplus
}
#endif

