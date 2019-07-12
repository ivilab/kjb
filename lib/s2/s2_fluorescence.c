
/* $Id: s2_fluorescence.c 6352 2010-07-11 20:13:21Z kobus $ */

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

#include "s/s_gen.h"     /* Only safe as first include in a ".c" file. */
#include "s/s_io.h"
#include "n2/n2_quadratic.h"
#include "s2/s2_fluorescence.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                         create_fluorescent_database
 *
 * Creates a fluorescent database of the specified size
 *
 * This routine creates a fluorescent database of the specified size.
 * Fluorescent databases should be disposed of with free_fluorescent_database().
 *
 * Returns :
 *     On succes, a pointer to a freshly allocated fluorescent_database is
 *     returned. On failure, NULL is returned, and an error message is set.
 *
 * Related:
 *     Fluorescent_database
 *
 * Index: fluorescence
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Fluorescent_database* debug_create_fluorescent_database
(
    int         num_fluorescent_surfaces,
    const char* file_name,
    int         line_number
)
{
    Fluorescent_database *fl_db_ptr;
    int i;


    if (num_fluorescent_surfaces < 1)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(fl_db_ptr = DEBUG_TYPE_MALLOC(Fluorescent_database,
                                      file_name, line_number));

    fl_db_ptr->fluorescent_surfaces = DEBUG_N_TYPE_MALLOC(Fluorescent_surface,
                                                       num_fluorescent_surfaces,
                                                       file_name, line_number);

    if (fl_db_ptr->fluorescent_surfaces == NULL)
    {
        kjb_free(fl_db_ptr);
        return NULL;
    }

    for (i=0; i<num_fluorescent_surfaces; i++)
    {
        (fl_db_ptr->fluorescent_surfaces)[ i ].illum_sp = NULL;
        (fl_db_ptr->fluorescent_surfaces)[ i ].response_sp = NULL;
    }

    fl_db_ptr->num_fluorescent_surfaces = num_fluorescent_surfaces;

    return fl_db_ptr;
}


        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Fluorescent_database* create_fluorescent_database
(
    int num_fluorescent_surfaces
)
{
    Fluorescent_database* fl_db_ptr;
    int                   i;


    if (num_fluorescent_surfaces < 1)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(fl_db_ptr = TYPE_MALLOC(Fluorescent_database));

    fl_db_ptr->fluorescent_surfaces = N_TYPE_MALLOC(Fluorescent_surface,
                                                    num_fluorescent_surfaces);

    if (fl_db_ptr->fluorescent_surfaces == NULL)
    {
        kjb_free(fl_db_ptr);
        return NULL;
    }

    for (i=0; i<num_fluorescent_surfaces; i++)
    {
        (fl_db_ptr->fluorescent_surfaces)[ i ].illum_sp = NULL;
        (fl_db_ptr->fluorescent_surfaces)[ i ].response_sp = NULL;
    }

    fl_db_ptr->num_fluorescent_surfaces = num_fluorescent_surfaces;

    return fl_db_ptr;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         free_fluorescent_database
 *
 * Frees fluorescent_database
 *
 * This routine frees a fluorescent database, as created by
 * get_target_fluorescent_database, create_fluorescent_database(),
 * copy_fluorescent_database(), etc. It is safe to pass a NULL.
 *
 * Related:
 *     Fluorescent_database
 *
 * Index: fluorescence
 *
 * -----------------------------------------------------------------------------
*/

void free_fluorescent_database(Fluorescent_database* fl_db_ptr)
{
            Fluorescent_surface* fluorescent_surface_ptr;
    int i;


    if (fl_db_ptr != NULL)
    {
        fluorescent_surface_ptr = fl_db_ptr->fluorescent_surfaces;

        for (i=0; i<fl_db_ptr->num_fluorescent_surfaces; i++)
        {
            if (fluorescent_surface_ptr != NULL)
            {
                free_spectra(fluorescent_surface_ptr->illum_sp);
                free_spectra(fluorescent_surface_ptr->response_sp);
            }
            fluorescent_surface_ptr++;

        }
        kjb_free(fl_db_ptr->fluorescent_surfaces);
        kjb_free(fl_db_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_target_fluorescent_database
 *
 * Gets target fluorescent_database for "building block" routines
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library. If *target_fl_db_ptr_ptr is NULL, then this routine creates the
 * fluorescent_database. If it is not null, and it is the right size, then this
 * routine does nothing. If it is the wrong size, then it is resized.
 *
 * If an actual resize is needed, then a new fluorescent_database of the
 * required size is first created. If the creation is successful, then the old
 * fluorescent_database is free'd.  The reason is that if the new allocation
 * fails, a calling application should have use of the old fluorescent_database.
 * The alternate is to free the old fluorescent_database first.  This is more
 * memory efficient. A more sophisticated alternative is to free the old
 * fluorescent_database if it can be deterimined that the subsequent allocation
 * will succeed. Although such approaches have merit, it is expected that
 * resizing will occur infrequently enought that it is not worth implementing
 * them. Thus the simplest method with good semantics under most conditions has
 * been used.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Fluorescent_database
 *
 * Index: fluorescence
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_target_fluorescent_database
(
    Fluorescent_database** out_fl_db_ptr_ptr,
    int                    num_fluorescent_surfaces,
    const char*            file_name,
    int                    line_number
)
{
    Fluorescent_database *out_fl_db_ptr = *out_fl_db_ptr_ptr;


    if (num_fluorescent_surfaces < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (out_fl_db_ptr == NULL)
         || (out_fl_db_ptr->num_fluorescent_surfaces !=num_fluorescent_surfaces)
       )
    {
        Fluorescent_database *new_fl_db_ptr;

        NRE(new_fl_db_ptr = debug_create_fluorescent_database(
                                                     num_fluorescent_surfaces,
                                                     file_name, line_number));
        free_fluorescent_database(out_fl_db_ptr);
        *out_fl_db_ptr_ptr = new_fl_db_ptr;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_fluorescent_database
(
    Fluorescent_database** out_fl_db_ptr_ptr,
    int                    num_fluorescent_surfaces
)
{
    Fluorescent_database *out_fl_db_ptr = *out_fl_db_ptr_ptr;


    if (num_fluorescent_surfaces < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (out_fl_db_ptr == NULL)
         || (out_fl_db_ptr->num_fluorescent_surfaces !=num_fluorescent_surfaces)
       )
    {
        Fluorescent_database *new_fl_db_ptr;

        NRE(new_fl_db_ptr =
                         create_fluorescent_database(num_fluorescent_surfaces));

        free_fluorescent_database(out_fl_db_ptr);

        *out_fl_db_ptr_ptr = new_fl_db_ptr;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_fluorescent_database
(
    Fluorescent_database** target_fl_db_ptr_ptr,
    Fluorescent_database*  fl_db_ptr
)
{
    int i;
    int num_fluorescent_surfaces;
    Fluorescent_surface* target_fl_ptr;
    Fluorescent_surface* fl_ptr;


    if (fl_db_ptr == NULL)
    {
        free_fluorescent_database(*target_fl_db_ptr_ptr);
        *target_fl_db_ptr_ptr = NULL;
        return NO_ERROR;
    }

    num_fluorescent_surfaces = fl_db_ptr->num_fluorescent_surfaces;

    ERE(get_target_fluorescent_database(target_fl_db_ptr_ptr,
                                        num_fluorescent_surfaces));


    target_fl_ptr = (*target_fl_db_ptr_ptr)->fluorescent_surfaces;
    fl_ptr = fl_db_ptr->fluorescent_surfaces;

    for (i=0; i<num_fluorescent_surfaces; i++)
    {
        ERE(copy_spectra(&(target_fl_ptr->illum_sp), fl_ptr->illum_sp));
        ERE(copy_spectra(&(target_fl_ptr->response_sp), fl_ptr->response_sp));

        target_fl_ptr++;
        fl_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           read_fl_db_from_config_file
 *
 * Reads a fluorescent database from a config file
 *
 * This routine reads a fluorescent database from a configuration file, checking
 * several possible locations for the file. If the parameter "env_var" is not
 * NULL, then it first tries using this string as the configuration file name.
 * Next it looks for "file_name" in the current directory, then the user's home
 * directory, and, depending on how the library was built, then the "shared"
 * home directory and/or the programmer's home directory. If "directory" is not
 * NULL, then it is used as a sub-directory in all paths starting from a home
 * directory (i.e., all paths except the current directory).
 *
 * The parameter message_name can be used to specify a name for the
 * configuration file to be used in error and verbose output messages. If
 * message_name is NULL, then "configuration" is used.
 *
 * If the buffer config_file_name is not NULL, then the file name actually used
 * is copied into it. If it is used, then its size must be passed in via
 * config_file_name_size;
 *
 * Example:
 *     Suppose directory is "x" and file_name is "y". Also supposed that the
 *     shared directory is "~iis" (compiled in constant) and the programmer's
 *     directory is "~kobus" (another compiled in constant). Then the following
 *     files are tried in the order listed.
 * |            ./y
 * |            ~/x/y
 * |            ~iis/x/y
 * |            ~kobus/x/y
 *
 * The file name actually used is put into the buffer config_file_name whose
 * size must be passed in via max_len;
 *
 * The flourescent database *fl_db_ptr_ptr is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *fl_db_ptr_ptr is freed and set to NULL.
 *
 * Note:
 *     The form of the flourescent database file is a header of the form:
 * |        #! t=fluorescent_db c=<num_fluorescent_surfaces>
 *     Followed by <num_fluorescent_surfaces> groups of spectra. Each group of
 *     spectra must be started with the spectra header, and end with the special
 *     soft end of file:
 * |        #! eof
 *     (Note that the "#" is resettable by the user option "comment-char", and
 *     the "!" is resettable by the user option "header-char").
 *
 *     Each group of spectra must contain an even number of spectra which are
 *     the spectra of an illuminant, followed by a spectra of the response of
 *     the given fluorescent surface to that illuminant. A reasonable number of
 *     relatively different illuminat spectra are required to adequately
 *     characterize a fluorescent surface.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O, fluorescence
 *
 * -----------------------------------------------------------------------------
*/

int read_fl_db_from_config_file
(
    Fluorescent_database** fl_db_ptr_ptr,
    const char*            env_var,
    const char*            directory,
    const char*            file_name,
    const char*            message_name,
    char*                  config_file_name,
    size_t                 config_file_name_size
)
{
    char     temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int      result;


    if (is_no_value_word(file_name))
    {
        free_fluorescent_database(*fl_db_ptr_ptr);
        *fl_db_ptr_ptr = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    result = read_fluorescent_database(fl_db_ptr_ptr, temp_config_file_name);

    if (result == ERROR)
    {
        insert_error("Error reading data from config file %s.",
                     temp_config_file_name);
        return ERROR;
    }
    else if (config_file_name != NULL)
    {
        kjb_strncpy(config_file_name, temp_config_file_name,
                    config_file_name_size);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           read_fluorescent_database
 *
 * Reads a fluorescent database
 *
 * This routine reads a fluorescent database from a file.
 *
 * The flourescent database *fl_db_ptr is created or resized as necessary.
 *
 * Note:
 *     The form of the flourescent database file is a header of the form:
 * |        #! t=fluorescent_db c=<num_fluorescent_surfaces>
 *     Followed by <num_fluorescent_surfaces> groups of spectra. Each group of
 *     spectra must be started with the spectra header, and end with the special
 *     soft end of file:
 * |        #! eof
 *     (Note that the "#" is resettable by the user option "comment-char", and
 *     the "!" is resettable by the user option "header-char").
 *
 *     Each group of spectra must contain an even number of spectra which are
 *     the spectra of an illuminant, followed by a spectra of the response of
 *     the given fluorescent surface to that illuminant. A reasonable number of
 *     relatively different illuminat spectra are required to adequately
 *     characterize a fluorescent surface.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, fluorescence
 *
 * -----------------------------------------------------------------------------
*/

int read_fluorescent_database
(
    Fluorescent_database** fl_db_ptr_ptr,
    char*                  file_name
)
{
    int result;
    FILE*fp;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_fluorescent_database(fl_db_ptr_ptr, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);   /* Ignore return--only reading */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_fluorescent_database
(
    Fluorescent_database** fl_db_ptr_ptr,
    FILE*                  fp
)
{
    int i, num_fluorescent_surfaces;
    Fluorescent_surface* fl_ptr;


    ERE(fp_read_fluorescent_database_header(fp, &num_fluorescent_surfaces));

    ERE(get_target_fluorescent_database(fl_db_ptr_ptr,
                                        num_fluorescent_surfaces));

    fl_ptr = (*fl_db_ptr_ptr)->fluorescent_surfaces;

    for (i=0; i<num_fluorescent_surfaces; i++)
    {
        if (fp_read_fluorescent_database_surface(fp, fl_ptr) == ERROR)
        {
            insert_error("Unable to read expected fluorescent surface %d.",
                         i + 1);
            return ERROR;
        }

        fl_ptr++;
    }

    if (count_real_lines(fp) > 0)
    {
        set_error("Extra lines in fluorescent database file.");
        add_error("Data for exactly %d fluorescent surfaces is expected.",
                  num_fluorescent_surfaces);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_fluorescent_database_header
(
    FILE* fp,
    int*  num_fluorescent_surfaces_ptr
)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 200 ];
    char*          line_pos;
    int            result             = NO_ERROR;
    int num_fluorescent_surfaces = NOT_SET;
    int type_found = FALSE;


    /*CONSTCOND*/
    while ( TRUE )
    {
        int read_result;

        read_result  = BUFF_FGET_LINE(fp, line);

        if (read_result == ERROR) result = ERROR;

        if (read_result < 0) break;

        line_pos = line;

        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ;  /* Do nothing. */
        }
        else if (*line_pos == kjb_comment_char)
        {
            line_pos++;

            trim_beg(&line_pos);

            if (*line_pos == kjb_header_char)
            {
                line_pos++;

                num_options = parse_options(line_pos, &option_list,
                                            &value_list);
                option_list_pos = option_list;
                value_list_pos  = value_list;

                if (num_options == ERROR)
                {
                    result = ERROR;
                    break;
                }

                for (i=0; i<num_options; i++)
                {
                    if (IC_STRCMP_EQ(*option_list_pos,"c"))
                    {
                        if (ss1pi(*value_list_pos,
                                  &num_fluorescent_surfaces) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }
                    else if (IC_STRCMP_EQ(*option_list_pos,"t"))
                    {
                        if (IC_STRCMP_EQ(*value_list_pos, "fluorescent_db"))
                        {
                            type_found = TRUE;
                        }
                        else
                        {
                            set_error(
                                  "t=%q is invalid for fluorescent databases.",
                                  *value_list_pos);
                            result = ERROR;
                            break;
                        }
                    }
                    else
                    {
                        set_error("%q is an invalid option.", *option_list_pos);
                        result = ERROR;
                        break;
                    }

                    value_list_pos++;
                    option_list_pos++;
                }

                free_options(num_options, option_list, value_list);

                break; /* We have found and processed the header line. */
            }
        }
        else
        {
            set_error("Header is missing from fluorescent database in %F.", fp);
            result = ERROR;
            break;
        }
    }

    if (result != ERROR)
    {
        if (num_fluorescent_surfaces == NOT_SET)
        {
            set_error("Error using %F as fluorescent database.", fp);
            add_error("The number of surfaces is not specified.");
            result = ERROR;
        }
        else
        {
            *num_fluorescent_surfaces_ptr = num_fluorescent_surfaces;
        }

        if (type_found == FALSE)
        {
            pso("Warning: Fluorescent database type tag is missing.\n");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_fluorescent_database_surface
(
    FILE*                fp,
    Fluorescent_surface* fl_ptr
)
{
    Spectra* input_sp = NULL;
    int      num_illuminants;
    int      result = NO_ERROR;


    ERE(fp_read_spectra(&input_sp, fp));

    if (IS_ODD(input_sp->spectra_mp->num_rows))
    {
        set_error("An odd number of spectra was read for a surface in %F.\n",
                  fp);
        num_illuminants = NOT_SET;   /* For debugging. */
        result = ERROR;
    }
    else
    {
        num_illuminants = input_sp->spectra_mp->num_rows / 2;
    }

    if (result != ERROR)
    {
        result = get_target_spectra(&(fl_ptr->illum_sp), num_illuminants,
                                     input_sp->spectra_mp->num_cols,
                                     input_sp->offset, input_sp->step,
                                     ILLUMINANT_SPECTRA);
    }

    if (result != ERROR)
    {
        result = get_target_spectra(&(fl_ptr->response_sp), num_illuminants,
                                     input_sp->spectra_mp->num_cols,
                                     input_sp->offset, input_sp->step,
                                     REFLECTANCE_SPECTRA);
    }

    if (result != ERROR)
    {
        Vector* spectra_row_vp = NULL;
        int i;

        for (i=0; i<num_illuminants; i++)
        {
            result = get_matrix_row(&spectra_row_vp, input_sp->spectra_mp,
                                    2 * i);

            if (result == ERROR) break;

            result = put_matrix_row(fl_ptr->illum_sp->spectra_mp,
                                    spectra_row_vp, i);

            if (result == ERROR) break;

            result = get_matrix_row(&spectra_row_vp, input_sp->spectra_mp,
                                    2 * i + 1);

            if (result == ERROR) break;

            result = put_matrix_row(fl_ptr->response_sp->spectra_mp,
                                    spectra_row_vp, i);

            if (result == ERROR) break;
        }

        free_vector(spectra_row_vp);
    }


    free_spectra(input_sp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_fluorescent_database
 *
 * Writes a fluorescent database
 *
 * This routine writes a fluorescent database from a file.
 *
 * Note:
 *     The form of the flourescent database file is a header of the form:
 * |        #! t=fluorescent_db c=<num_fluorescent_surfaces>
 *     Followed by <num_fluorescent_surfaces> groups of spectra. Each group of
 *     spectra must be started with the spectra header, and end with the special
 *     soft end of file:
 * |        #! eof
 *     (Note that the "#" is resettable by the user option "comment-char", and
 *     the "!" is resettable by the user option "header-char").
 *
 *     Each group of spectra must contain an even number of spectra which are
 *     the spectra of an illuminant, followed by a spectra of the response of
 *     the given fluorescent surface to that illuminant. A reasonable number of
 *     relatively different illuminat spectra are required to adequately
 *     characterize a fluorescent surface.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, fluorescence
 *
 * -----------------------------------------------------------------------------
*/

int write_fluorescent_database
(
    char*                 file_name,
    Fluorescent_database* fl_db_ptr
)
{
    int result;
    FILE*fp;
    Error_action save_error_action = get_error_action();


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_fluorescent_database(fp, fl_db_ptr);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        if (result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        if (kjb_fclose(fp) == ERROR)
        {
            result = ERROR;
        }

        set_error_action(save_error_action);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_write_fluorescent_database
(
    FILE*                 fp,
    Fluorescent_database* fl_db_ptr
)
{
    int i, num_fluorescent_surfaces;
    Fluorescent_surface* fl_ptr;


    num_fluorescent_surfaces = fl_db_ptr->num_fluorescent_surfaces;
    ERE(fp_write_fluorescent_database_header(fp, num_fluorescent_surfaces));

    fl_ptr = fl_db_ptr->fluorescent_surfaces;

    for (i=0; i<num_fluorescent_surfaces; i++)
    {
        ERE(fp_write_fluorescent_database_surface(fp, fl_ptr));
        fl_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_write_fluorescent_database_header
(
    FILE* fp,
    int   num_fluorescent_surfaces
)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;


    ERE(kjb_fprintf(fp, "\n%c%c t=fluorescent_db c=%d\n\n",
                    kjb_comment_char,
                    kjb_header_char,
                    num_fluorescent_surfaces));
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_write_fluorescent_database_surface
(
    FILE*                fp,
    Fluorescent_surface* fl_ptr
)
{
    Vector* spectra_row_vp = NULL;
    int i;
    int num_illuminants = fl_ptr->illum_sp->spectra_mp->num_rows;
    Spectra* output_sp = NULL;
    int result = NO_ERROR;


    ERE(get_target_spectra(&output_sp, 2 * num_illuminants,
                           fl_ptr->illum_sp->spectra_mp->num_cols,
                           fl_ptr->illum_sp->offset,
                           fl_ptr->illum_sp->step,
                           GENERIC_SPECTRA));

    for (i=0; i<num_illuminants; i++)
    {
        result = get_matrix_row(&spectra_row_vp, fl_ptr->illum_sp->spectra_mp,
                                i);

        if (result == ERROR) break;

        result = put_matrix_row(output_sp->spectra_mp, spectra_row_vp, 2*i);

        if (result == ERROR) break;

        result = get_matrix_row(&spectra_row_vp,
                                fl_ptr->response_sp->spectra_mp, i);

        if (result == ERROR) break;

        result = put_matrix_row(output_sp->spectra_mp, spectra_row_vp, 2*i + 1);

        if (result == ERROR) break;
    }

    if (result != ERROR)
    {
        result = fp_write_spectra(output_sp, fp);
    }

    if (result != ERROR)
    {
        result = kjb_fprintf(fp, "\n%c%c eof\n\n",
                             kjb_comment_char, kjb_header_char);
    }

    free_spectra(output_sp);
    free_vector(spectra_row_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_fluorescent_response_spectra
(
    Spectra**             response_sp_ptr,
    Fluorescent_database* fl_db_ptr,
    Spectra*              illum_sp,
    int                   index
)
{
    int     num_fluorescent_surfaces = fl_db_ptr->num_fluorescent_surfaces;
    int     result = NO_ERROR;
    int     i;
    Vector* illum_vp                 = NULL;
    Vector* response_vp              = NULL;
    Matrix* response_spect_mp;
    int     num_illuminants;
    int     first;
    int     count;
    int     num_spectra;
    Fluorescent_surface* fl_ptr;


    fl_ptr = fl_db_ptr->fluorescent_surfaces;

    for (count=0; count<num_fluorescent_surfaces; count++)
    {
        ERE(check_spectra_are_comparable(fl_ptr->illum_sp, illum_sp));
        fl_ptr++;
    }

    if (index == USE_ALL_SPECTRA)
    {
        UNTESTED_CODE();

        num_illuminants = illum_sp->spectra_mp->num_rows;
        first = 0;
    }
    else
    {
        num_illuminants = 1;
        first = index;
    }

    num_spectra = num_illuminants * num_fluorescent_surfaces;

    ERE(get_target_spectra(response_sp_ptr, num_spectra,
                           illum_sp->spectra_mp->num_cols, illum_sp->offset,
                           illum_sp->step, GENERIC_SPECTRA));

    response_spect_mp = (*response_sp_ptr)->spectra_mp;

    fl_ptr = fl_db_ptr->fluorescent_surfaces;

    for (count=0; count<num_fluorescent_surfaces; count++)
    {
        for (i=0; i<num_illuminants; i++)
        {
            result = get_matrix_row(&illum_vp, illum_sp->spectra_mp, first + i);

            if (result != ERROR)
            {
                result = get_fluorescent_surface_response(&response_vp, fl_ptr,
                                                          illum_vp,
                                                          (Vector**)NULL);
            }

            if (result != ERROR)
            {
                result = put_matrix_row(response_spect_mp, response_vp,
                                        count * num_illuminants + i);
            }
        }

        if (result == ERROR) break;

        fl_ptr++;
    }

    free_vector(illum_vp);
    free_vector(response_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_fluorescent_surface_response
(
    Vector**             response_vpp,
    Fluorescent_surface* fl_ptr,
    Vector*              illum_vp,
    Vector**             estimated_illum_vpp
)
{
    Matrix* transposed_illum_mp = NULL;
    Matrix* transposed_response_mp = NULL;
    Vector* illum_weight_vp = NULL;
    Vector* lb_vp = NULL;
    int result;


    result = get_transpose(&transposed_illum_mp, fl_ptr->illum_sp->spectra_mp);


    if (result != ERROR)
    {
        result = get_transpose(&transposed_response_mp,
                               fl_ptr->response_sp->spectra_mp);
    }

    UNTESTED_CODE();  /* Since rework of constrained_least_squares. */

    if (result != ERROR)
    {
        result = get_zero_vector(&lb_vp, transposed_illum_mp->num_rows);
    }

    if (result != ERROR)
    {
        result = constrained_least_squares(&illum_weight_vp,
                                           transposed_illum_mp,
                                           illum_vp,
                                           (const Matrix*)NULL,
                                           (const Vector*)NULL,
                                           (const Matrix*)NULL,
                                           (const Vector*)NULL,
                                           lb_vp,
                                           (const Vector*)NULL);
    }

    if (result != ERROR)
    {
        result = multiply_matrix_and_vector(response_vpp,
                                            transposed_response_mp,
                                            illum_weight_vp);
    }


    if ((result != ERROR) && (estimated_illum_vpp != NULL))
    {
        result = multiply_matrix_and_vector(estimated_illum_vpp,
                                            transposed_illum_mp,
                                            illum_weight_vp);
    }

    free_vector(lb_vp);
    free_vector(illum_weight_vp);
    free_matrix(transposed_illum_mp);
    free_matrix(transposed_response_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

