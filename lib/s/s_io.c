
/* $Id: s_io.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int get_spectra_output_name
(
    const Spectra* sp,
    const char*    file_name,
    char*          modified_file_name,
    size_t         modified_file_name_max_size
);


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                        read_spectra_from_config_file
 *
 * Reads spectra from a config file
 *
 * This routine reads spectra from a configuration file, checking several
 * possible locations for the file. If the parameter "env_var" is not NULL, then
 * it first tries using this string as the configuration file name. Next it
 * looks for "file_name" in the current directory, then the user's home
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
 * The spectra *result_sp_ptr is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_sp_ptr is freed and set to NULL.
 *
 * Note:
 *     Spectra is plural! This routine will read as many spectra from the file
 *     as there are. Normally the file is in Kobus's spectra file format which
 *     has a line including the number for intervals, the interval size, and the
 *     first interval wavelength in nano-meterst. If no such line is present,
 *     then the values for the PhotoResearch PR-650 are used which is equivalent
 *     to having the following header:
 * |            #! t=s n=101 o=380 s=4
 *     (Note that the "#" is resettable by the user option "comment-char", and
 *     the "!" is resettable by the user option "header-char").
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_spectra_from_config_file
(
    Spectra**   result_sp_ptr,
    const char* env_var,
    const char* directory,
    const char* file_name,
    const char* message_name,
    char*       config_file_name,
    size_t      config_file_name_size
)
{
    char     temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int      result;


    if (is_no_value_word(file_name))
    {
        free_spectra(*result_sp_ptr);
        *result_sp_ptr = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    /*
    // Clear the error since there is an execution path which uses
    // "insert_error" without a previous "set_error". Although it should be
    // guaranteed that the error should be set if "read_spectra", fails, we
    // are better off not taking changes.
    */
    kjb_clear_error();

    result = read_spectra(result_sp_ptr, temp_config_file_name);

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
 *                             read_reflectance_spectra
 *
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_reflectance_spectra
(
    Spectra**   result_sp_ptr,
    const char* file_name
)
{


    ERE(read_spectra(result_sp_ptr, file_name));

    if (    ((*result_sp_ptr)->type != GENERIC_SPECTRA)
         && ((*result_sp_ptr)->type != REFLECTANCE_SPECTRA)
       )
    {
        warn_pso("Previously indictated spectra type for %s is ",file_name);
        warn_pso("being over-ridden.\n");
        warn_pso("         Spectra in %s are being used as reflectances.\n",
                 file_name);
    }

    (*result_sp_ptr)->type = REFLECTANCE_SPECTRA;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             read_illuminant_spectra
 *
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_illuminant_spectra
(
    Spectra**   result_sp_ptr,
    const char* file_name
)
{


    ERE(read_spectra(result_sp_ptr, file_name));

    if (    ((*result_sp_ptr)->type != GENERIC_SPECTRA)
         && ((*result_sp_ptr)->type != ILLUMINANT_SPECTRA)
       )
    {
        warn_pso("Previously indictated spectra type for %s is ",file_name);
        warn_pso("being over-ridden.\n");
        warn_pso("         Spectra in %s are being used as illuminants.\n",
                 file_name);
    }

    (*result_sp_ptr)->type = ILLUMINANT_SPECTRA;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             read_sensor_spectra
 *
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_sensor_spectra(Spectra** result_sp_ptr, const char* file_name)
{


    ERE(read_spectra(result_sp_ptr, file_name));

    if (    ((*result_sp_ptr)->type != GENERIC_SPECTRA)
         && ((*result_sp_ptr)->type != SENSOR_SPECTRA)
       )
    {
        warn_pso("Previously indictated spectra type for %s is ",
                 file_name);
        warn_pso("being over-ridden.\n");
        warn_pso("         Spectra in %s are being used as illuminants.\n",
                 file_name);
    }

    (*result_sp_ptr)->type = SENSOR_SPECTRA;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               read_spectra
 *
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_spectra(Spectra** result_sp_ptr, const char* file_name)
{
    FILE*        fp;
    Error_action save_error_action = get_error_action();
    int          result;


    NRE(fp = kjb_fopen(file_name, "r"));

    result = fp_read_spectra(result_sp_ptr, fp);

    if (result == ERROR)
    {
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

    (void)kjb_fclose(fp);

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_spectra(Spectra** result_sp_ptr, FILE* fp)
{
    double         step               = SPECTROMETER_STEP;
    double         offset             = SPECTROMETER_OFFSET;
    int            num_freq_intervals = NUM_SPECTROMETER_INTERVALS;
    Spectra_origin type               = GENERIC_SPECTRA;
    int num_spectra = 0;


    ERE(fp_read_spectra_file_header(fp, &num_freq_intervals, &offset, &step,
                                    &type));

    if (IS_LESSER_DBL(offset, 380.0))
    {
        warn_pso("Offset less than 380.0 in %F.\n", fp);
    }

    if (IS_GREATER_DBL(offset, 400.0))
    {
        warn_pso("Offset less than 400.0 in %F.\n", fp);
    }

    if (IS_LESSER_DBL(offset + (step * (num_freq_intervals - 1)), 650.0))
    {
        warn_pso("Max frequency less than 650.0 in %F.\n", fp);
    }

    if (IS_GREATER_DBL(offset + (step * (num_freq_intervals - 1)), 780.0))
    {
        warn_pso("Max frequency greater than 780.0 in %F.\n", fp);
    }

    if ((*result_sp_ptr) != NULL)
    {
        num_spectra = (*result_sp_ptr)->spectra_mp->num_rows;
    }

    ERE(get_target_spectra(result_sp_ptr, num_spectra, num_freq_intervals,
                           offset, step, type));

    if (fp_read_matrix_by_rows(&((*result_sp_ptr)->spectra_mp), fp,
                               num_freq_intervals)
        == ERROR)
    {
        free_spectra(*result_sp_ptr);
        *result_sp_ptr = NULL;
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           read_spectra_file_header
 *
 * Reads header in spectra file.
 *
 * This routine reads the spectra header in file_name. The file is opened,
 * processed from the begining, and then closed. If the header contains
 * information about the number of frequency intervals, the offset, or the step
 * size, then the corresponding variables whose pointers are arguments are set.
 * Variables are not changed unless there is information in the header. The type
 * information works similarly, except that the file suffix is also considered a
 * source of information.
 *
 * Note:
 *     This routine is not normally needed. It is used for code that use
 *     different forms of handling spectra (which explains also why the
 *     parameters needed are low-level concepts, rather than the Spectra type).
 *
 * Related:
 *      fp_read_spectra_file_header
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int read_spectra_file_header
(
    char*           file_name,
    int*            num_freq_intervals_ptr,
    double*         offset_ptr,
    double*         step_ptr,
    Spectra_origin* type_ptr
)
{
    FILE*        fp;
    int          result;
    Error_action save_error_action = get_error_action();


    NRE(fp = kjb_fopen(file_name, "r"));

    result = fp_read_spectra_file_header(fp, num_freq_intervals_ptr,
                                         offset_ptr, step_ptr, type_ptr);

    if (result == ERROR)
    {
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

    if (kjb_fclose(fp) == ERROR)
    {
        result = ERROR;
    }

    set_error_action(save_error_action);

    return result;
}

/* =============================================================================
 *                             fp_read_spectra
 *
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
*/

/*
 * =============================================================================
 *                       fp_read_spectra_file_header
 *
 * Reads header in spectra file.
 *
 * This routine reads the spectra header in file_name. If the header contains
 * information about the number of frequency intervals, the offset, or the step
 * size, then the corresponding variables whose pointers are arguments are set.
 * Variables are not changed unless there is information in the header. The type
 * information works similarly, except that the file suffix is also considered a
 * source of information.
 *
 * Note:
 *     This routine is not normally needed, as it is accessed by
 *     fp_read_spectra (or indirectly via read_spectra). It is exported for
 *     code that use different forms of handling spectra (which explains also
 *     why the parameters needed are low-level concepts, rather than the Spectra
 *     type).
 *
 * Related:
 *      fp_read_spectra_file_header
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int fp_read_spectra_file_header
(
    FILE*           fp,
    int*            num_freq_intervals_ptr,
    double*         offset_ptr,
    double*         step_ptr,
    Spectra_origin* type_ptr
)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    static const char *suffixes[ ] =
    {
        "spect", "spectra", "illum", "reflect", "sensor", "sensors", NULL
    };
    char           file_name[ MAX_FILE_NAME_SIZE ];
    char           base_name[ MAX_FILE_NAME_SIZE ];
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 200 ];
    char*          line_pos;
    int            result             = NO_ERROR;
    char           suffix[ 100 ];
    Spectra_origin suffix_type        = SPECTRA_TYPE_NOT_SET;
    Spectra_origin type               = SPECTRA_TYPE_NOT_SET;
    int            num_freq_intervals = NOT_SET;
    double           offset             = DBL_NOT_SET;
    double           step               = DBL_NOT_SET;
    long           save_file_pos;


    BUFF_GET_FD_NAME(fileno(fp), file_name);

    ERE(save_file_pos = kjb_ftell(fp));

    ERE(get_base_path(file_name, base_name, sizeof(base_name), suffix,
                      sizeof(suffix), suffixes));

    if (STRCMP_EQ(suffix, "reflect"))
    {
        suffix_type = REFLECTANCE_SPECTRA;
    }
    else if (    (STRCMP_EQ(suffix, "spect"))
              || (STRCMP_EQ(suffix, "spectra"))
            )
    {
        suffix_type = GENERIC_SPECTRA;
    }
    else if (STRCMP_EQ(suffix, "illum"))
    {
        suffix_type = ILLUMINANT_SPECTRA;
    }
    else if (    (STRCMP_EQ(suffix, "sensor"))
              || (STRCMP_EQ(suffix, "sensors"))
            )
    {
        suffix_type = SENSOR_SPECTRA;
    }

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
                    if (IC_STRCMP_EQ(*option_list_pos,"n"))
                    {
                        if (ss1pi(*value_list_pos,
                                  &num_freq_intervals) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }
                    else if (IC_STRCMP_EQ(*option_list_pos,"o"))
                    {
                        if (ss1d(*value_list_pos, &offset) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }
                    else if (IC_STRCMP_EQ(*option_list_pos,"s"))
                    {
                        if (ss1d(*value_list_pos, &step) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }
                    else if (IC_STRCMP_EQ(*option_list_pos,"t"))
                    {
                        if (**value_list_pos == 'i')
                        {
                            type= ILLUMINANT_SPECTRA;
                        }
                        else if (**value_list_pos == 'r')
                        {
                            type= REFLECTANCE_SPECTRA;
                        }
                        else if (**value_list_pos == 's')
                        {
                            type= SENSOR_SPECTRA;
                        }
                        else if (**value_list_pos == 'g')
                        {
                            type= GENERIC_SPECTRA;
                        }
                        else
                        {
                            set_error("%q is an invalid type.",
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

                if (result == ERROR)
                {
                    break;
                }
            }
        }
        else
        {
            result =  NO_ERROR;
            break;
        }
    }

    if (result != ERROR)
    {
        if (    (type != SPECTRA_TYPE_NOT_SET)
             && (suffix_type != SPECTRA_TYPE_NOT_SET)
           )
        {
            if (type != suffix_type)
            {
                pso("Warning: Internal spectra type contradicts that ");
                pso("implied by file suffix.\n");
                pso("File suffix type overides.\n");
            }
            *type_ptr = suffix_type;
        }
        else if (type != SPECTRA_TYPE_NOT_SET)
        {
            *type_ptr = type;
        }
        else if (suffix_type != SPECTRA_TYPE_NOT_SET)
        {
            *type_ptr = suffix_type;
        }

        if (KJB_IS_SET(num_freq_intervals))
        {
            *num_freq_intervals_ptr = num_freq_intervals;
        }

        if (KJB_IS_SET(offset)) *offset_ptr = offset;
        if (KJB_IS_SET(step))   *step_ptr   = step;
    }

    ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          write_spectra_full_precision
 *
 * Writes spectra to a file with full precision.
 *
 * This routine writes a spectra to a file. If the file name does not have a
 * spectra type suffix, then one is added, based on the spectra type. The data
 * is written out in full precision.
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int write_spectra_full_precision
(
    const Spectra* sp,
    const char*    file_name
)
{
    char  modified_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp;
    int result;
    Error_action save_error_action = get_error_action();


    ERE(get_spectra_output_name(sp, file_name, modified_file_name,
                                sizeof(modified_file_name)));

    NRE(fp = kjb_fopen(modified_file_name, "w"));

    result = fp_write_spectra(sp, fp);

    if (result == ERROR)
    {
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

    if (kjb_fclose(fp) == ERROR)
    {
        result = ERROR;
    }

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       fp_write_spectra_full_precision
 *
 * Writes spectra to a file with full precision.
 *
 * This routine writes a spectra to a file pointed to by fp with full precision.
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int fp_write_spectra_full_precision(const Spectra* sp, FILE* fp)
{
    int result;


    result = fp_write_spectra_file_header(fp, sp->spectra_mp->num_cols,
                                          sp->offset, sp->step, sp->type);

    if (result != ERROR)
    {
        result = fp_write_matrix_rows_full_precision(sp->spectra_mp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                write_spectra
 *
 * Writes spectra to a file.
 *
 * This routine writes a spectra to a file. If the file name does not have a
 * spectra type suffix, then one is added, based on the spectra type.
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int write_spectra(const Spectra* sp, const char* file_name)
{
    char  modified_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp;
    int result;
    Error_action save_error_action = get_error_action();


    ERE(get_spectra_output_name(sp, file_name, modified_file_name,
                                sizeof(modified_file_name)));

    NRE(fp = kjb_fopen(modified_file_name, "w"));

    result = fp_write_spectra(sp, fp);

    if (result == ERROR)
    {
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

    if (kjb_fclose(fp) == ERROR)
    {
        result = ERROR;
    }

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             fp_write_spectra
 *
 * Writes spectra to a file.
 *
 * This routine writes a spectra to a file pointed to by fp.
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int fp_write_spectra(const Spectra* sp, FILE* fp)
{
    int result;


    result = fp_write_spectra_file_header(fp, sp->spectra_mp->num_cols,
                                          sp->offset, sp->step, sp->type);

    if (result != ERROR)
    {
        result = fp_write_matrix_rows(sp->spectra_mp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_spectra_output_name
(
    const Spectra* sp,
    const char*    file_name,
    char*          modified_file_name,
    size_t         modified_file_name_max_size
)
{
    char base_name[ MAX_FILE_NAME_SIZE ];
    char suffix[ MAX_FILE_NAME_SIZE ];
    static const char *suffixes[ ] =
    {
        "sensor", "sensors", "reflect", "illum", "spectra", "spect", NULL
    };


    suffix[ 0 ] = '\0';

    ERE(get_base_path(file_name, base_name, sizeof(base_name),
                      suffix, sizeof(suffix), suffixes));

    if (sp->type == SENSOR_SPECTRA)
    {
        if (suffix[ 0 ] == '\0')
        {
            BUFF_CPY(suffix, "sensor");
        }
        else if (    ( ! STRCMP_EQ(suffix, "sensor"))
                  && ( ! STRCMP_EQ(suffix, "sensors"))
                  && ( ! STRCMP_EQ(suffix, "spectra"))
                  && ( ! STRCMP_EQ(suffix, "spect"))
                )
        {
            pso("Warning: File suffix %s contradicts spectra type (sensor).\n",
                suffix);
        }
    }
    else if (sp->type == REFLECTANCE_SPECTRA)
    {
        if (suffix[ 0 ] == '\0')
        {
            BUFF_CPY(suffix, "reflect");
        }
        else if (    ( ! STRCMP_EQ(suffix, "reflect"))
                  && ( ! STRCMP_EQ(suffix, "spectra"))
                  && ( ! STRCMP_EQ(suffix, "spect"))
                )
        {
            pso("Warning: File suffix %s contradicts spectra type (reflect).\n",
                suffix);
        }
    }
    else if (sp->type == ILLUMINANT_SPECTRA)
    {
        if (suffix[ 0 ] == '\0')
        {
            BUFF_CPY(suffix, "illum");
        }
        else if (    ( ! STRCMP_EQ(suffix, "illum"))
                  && ( ! STRCMP_EQ(suffix, "spectra"))
                  && ( ! STRCMP_EQ(suffix, "spect"))
                )
        {
            pso("Warning: File suffix %s contradicts spectra type (illum).\n",
                suffix);
        }
    }
    else if (suffix[ 0 ] == '\0')
    {
        BUFF_CPY(suffix, "spectra");
    }

    kjb_strncpy(modified_file_name, base_name, modified_file_name_max_size);
    kjb_strncat(modified_file_name, ".", modified_file_name_max_size);
    kjb_strncat(modified_file_name, suffix, modified_file_name_max_size);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          fp_write_spectra_file_header
 *
 * Writes spectra file header information
 *
 * This routine writes the spectra header file information contained in the
 * parameters. Any parameter which is not set (has a negative value) is not
 * written.
 *
 * Note:
 *     This routine is not normally needed, as it is accessed by
 *     fp_write_spectra (or indirectly via write_spectra). It is exported for
 *     code that use different forms of handling spectra (which explains also
 *     why the parameters needed are low-level concepts, rather than the Spectra
 *     type).
 *
 * Index: spectra, spectra I/O
 *
 * -----------------------------------------------------------------------------
 */

int fp_write_spectra_file_header
(
    FILE*          output_fp,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type
)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        type_char;


    ERE(kjb_fprintf(output_fp, "\n%c%c", kjb_comment_char, kjb_header_char));

    if      (type == REFLECTANCE_SPECTRA)     type_char = 'r';
    else if (type == ILLUMINANT_SPECTRA)      type_char = 'i';
    else if (type == SENSOR_SPECTRA)          type_char = 's';
    else if (type == GENERIC_SPECTRA)         type_char = 'g';
    else /* (type == SPECTRA_TYPE_NOT_SET) */ type_char = '\0';

    if (type_char != '\0')
    {
        ERE(kjb_fprintf(output_fp, " t=%c", type_char));
    }

    if (num_freq_intervals > 0)
    {
        ERE(kjb_fprintf(output_fp, " n=%d", num_freq_intervals));
    }

    if (offset > 0.0)
    {
        ERE(kjb_fprintf(output_fp, " o=%.1f", offset));
    }

    if (step > 0.0)
    {
        ERE(kjb_fprintf(output_fp, " s=%.1f", step));
    }


    ERE(kjb_fprintf(output_fp, "\n\n"));

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Spectra_origin get_spectra_type_from_str(char* type_str)
{

    if (STRCMP_EQ(type_str, "i"))
    {
        return ILLUMINANT_SPECTRA;
    }
    else if (STRCMP_EQ(type_str, "r"))
    {
        return REFLECTANCE_SPECTRA;
    }
    else if (STRCMP_EQ(type_str, "s"))
    {
        return SENSOR_SPECTRA;
    }
    else if (STRCMP_EQ(type_str, "g"))
    {
        return GENERIC_SPECTRA;
    }
    else
    {
        set_error("%q is an invalid spectra type.", type_str);
        return SPECTRA_TYPE_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int put_spectra_type_into_str
(
    Spectra_origin spectra_type,
    char*          buff,
    size_t         buff_size
)
{
    const char* type_str;


    if      (spectra_type == ILLUMINANT_SPECTRA)  type_str = "i";
    else if (spectra_type == REFLECTANCE_SPECTRA) type_str = "r";
    else if (spectra_type == SENSOR_SPECTRA)      type_str = "s";
    else if (spectra_type == GENERIC_SPECTRA)     type_str = "g";
    else                                          type_str = "<unknown>";


    kjb_strncpy(buff, type_str, buff_size);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

