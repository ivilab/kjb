
/* $Id: m_lut.c 20918 2016-10-31 22:08:27Z kobus $ */

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

#include "m/m_gen.h"     /* Only safe as first include in a ".c" file. */

#include "m/m_spline.h"
#include "m/m_lut.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int fp_read_lut_file_header
(
    FILE*   fp,
    int*    lut_len_ptr,
    double* offset_ptr,
    double* step_ptr
);

static int fp_write_lut_file_header
(
    FILE*  output_fp,
    int    lut_len,
    double offset,
    double step
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                               create_lut
 *
 * Creates a lut of the specified characteristics
 *
 * This routine creates a lut of the specified characteristics. The lut
 * should be disposed of with free_lut().
 *
 * Returns :
 *     On succes, a pointer to a freshly allocated lut is returned. On
 *     failure, NULL is returned, and an error message is set.
 *
 * Related:
 *     Lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Lut* debug_create_lut(int lut_len, double offset, double step,
                      const char* file_name, int line_number)
{
    Lut* lut_ptr;


    NRN(lut_ptr = DEBUG_TYPE_MALLOC(Lut, file_name, line_number));

    lut_ptr->lut_vp = debug_create_vector(lut_len, file_name, line_number);

    if (lut_ptr->lut_vp == NULL)
    {
        kjb_free(lut_ptr);
        lut_ptr = NULL;
    }
    else
    {
        lut_ptr->offset = offset;
        lut_ptr->step = step;
    }

    return lut_ptr;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Lut* create_lut(int lut_len, double offset, double step)
{
    Lut* lut_ptr;


    NRN(lut_ptr = TYPE_MALLOC(Lut));

    lut_ptr->lut_vp = create_vector(lut_len);

    if (lut_ptr->lut_vp == NULL)
    {
        kjb_free(lut_ptr);
        lut_ptr = NULL;
    }
    else
    {
        lut_ptr->offset = offset;
        lut_ptr->step = step;
    }

    return lut_ptr;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               free_lut
 *
 * Frees lut
 *
 * This routine frees allocted lut, as created by create_lut(),
 * copy_lut(), convert_lut(), etc. It is save to pass a NULL.
 *
 * Related:
 *     Lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
*/

void free_lut(Lut* lut_ptr)
{


    if (lut_ptr != NULL)
    {
        free_vector(lut_ptr->lut_vp);
        kjb_free(lut_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_target_lut
 *
 * Gets target lut for "building block" routines
 *
 * This routine implements the creation/over-writing symantics used in the KJB
 * library. If *target_lp_ptr is NULL, then this routine creates the lut. If
 * it is not null, and it is the right size, then this routine does nothing. If
 * it is the wrong size, then it is resized.
 *
 * If an actual resize is needed, then a new lut of the required size is
 * first created. If the creation is successful, then the old lut is free'd.
 * The reason is that if the new allocation fails, a calling application should
 * have use of the old lut. The alternate is to free the old lut first.
 * This is more memory efficient. A more sophisticated alternative is to free
 * the old lut if it can be deterimined that the subsequent allocation will
 * succeed. Although such approaches have merit, it is expected that
 * resizing will occur infrequently enought that it is not worth implementing
 * them. Thus the simplest method with good symantics under most conditions
 * has been used.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
*/


#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_target_lut(Lut** out_lp_ptr, int lut_len, double offset,
                         double step, const char* file_name, int line_number)
{
    Lut* out_lp = *out_lp_ptr;


    if (lut_len < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_lp == NULL)
    {
        NRE(out_lp = debug_create_lut(lut_len, offset, step, file_name,
                                      line_number));
        *out_lp_ptr = out_lp;
    }
    else
    {
        ERE(debug_get_target_vector(&(out_lp->lut_vp), lut_len, file_name,
                                    line_number));
        out_lp->offset = offset;
        out_lp->step   = step;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_lut(Lut** out_lp_ptr, int lut_len, double offset, double step)
{
    Lut* out_lp = *out_lp_ptr;


    if (lut_len < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_lp == NULL)
    {
        NRE(out_lp = create_lut(lut_len, offset, step));
        *out_lp_ptr = out_lp;
    }
    else
    {
        ERE(get_target_vector(&(out_lp->lut_vp), lut_len));
        out_lp->offset = offset;
        out_lp->step   = step;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  copy_lut
 *
 * Copies lut
 *
 * This routine copies the lut pointed to by "lut_ptr" to that pointed to by
 * "*target_lp_ptr", If *target_lp_ptr is NULL, then the target lut is
 * created. If it already exists, but is the wrong size, then it is resized.
 * Finally, if it is the correct size, it is over-written.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
 */

int copy_lut(Lut** target_lp_ptr, Lut* lp)
{


    UNTESTED_CODE();

    if (lp == NULL)
    {
        free_lut(*target_lp_ptr);
        *target_lp_ptr = NULL;
        return NO_ERROR;
    }

    ERE(get_target_lut(target_lp_ptr, lp->lut_vp->length, lp->offset,
                       lp->step));
    ERE(copy_vector(&((*target_lp_ptr)->lut_vp), lp->lut_vp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  convert_lut
 *
 * Converts a lut to one with different characteristics
 *
 * This routine converts a lut to one with different characteristics. The
 * resulting lut will have the specified characteristics. If interpolation
 * is necessary, then cubic spline is used. The resulting lut is pointed to
 * by *target_lp_ptr.
 *
 * If *target_lp_ptr is NULL, then the target lut is created. If it already
 * exists, but is the wrong size, then it is resized.  Finally, if it is the
 * correct size, it is over-written.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
*/

int convert_lut(Lut** target_lp_ptr, Lut* original_lp, int count, double offset,
                double step)
{


    UNTESTED_CODE();

    if (original_lp == NULL)
    {
        free_lut(*target_lp_ptr);
        *target_lp_ptr = NULL;
        return NO_ERROR;
    }

    verbose_pso(20, "Converting lut from (%d, %.1f, %.1f) ",
                original_lp->lut_vp->length, original_lp->offset,
                original_lp->step);
    verbose_pso(20, "to (%d, %.1f, %.1f)\n", count, offset, step);

    if (    (count == original_lp->lut_vp->length)
         && (IS_EQUAL_DBL(offset, original_lp->offset))
         && (IS_EQUAL_DBL(step, original_lp->step))
       )
    {
        verbose_pso(20, "No conversion required, so just copying.\n");
        ERE(copy_lut(target_lp_ptr, original_lp));
    }
    else  /* Bummer! Have to convert the original */
    {
        double temp_offset     = original_lp->offset;
        int    temp_count      = original_lp->lut_vp->length;
        double temp_step       = original_lp->step;
        double temp_upper      = temp_offset + (temp_count - 1) * temp_step;
        double upper;
        int    extra_beg_count = 0;
        int    extra_end_count = 0;
        Lut*   temp_lp;
        int    new_count;
        int    j;
        Lut*   target_lp;
        int    index           = 0;
        Vector in_vp;
        Vector out_vp;


        verbose_pso(20, "Converting with cubic splines.\n");

        ERE(get_target_lut(target_lp_ptr, count, offset, step));

        target_lp = *target_lp_ptr;

        /*
         * The derived lut will be a spline of the original. However, we
         * must first ensure that the range of the original contains the
         * desired. To do this, we pad on either side with zeros as
         * required.
         *
         * First determine number of extra begining zeros needed.
         */
        if (IS_GREATER_DBL(temp_offset, offset))
        {
            double extra_offset = temp_offset - offset;


            extra_beg_count = (int)(extra_offset / temp_step);

            if (IS_GREATER_DBL(extra_offset,
                                temp_step * extra_beg_count)
               )
            {
                extra_beg_count++;
            }

            temp_offset -= (extra_beg_count * original_lp->step);
        }

        /*
         *
         *  Next determine number of extra ending zeros needed.
         */
        upper = offset + (count - 1 ) * step;

        if (IS_GREATER_DBL(upper, temp_upper))
        {
            double extra_upper = upper - temp_upper;


            extra_end_count = (int)(extra_upper / temp_step);

            if (IS_GREATER_DBL(extra_upper, temp_step* extra_end_count)
               )
            {
                extra_end_count++;
            }
        }

        new_count = temp_count + extra_beg_count + extra_end_count;

        NRE(temp_lp = create_lut(new_count, temp_offset, temp_step));

        /*
        * Fill in zeros on either side of the "temp" copy of the original.
        */
        for (j=0; j<extra_beg_count; j++)
        {
            (temp_lp->lut_vp->elements)[ index++ ] = 0.0;
        }

        for (j=0; j<temp_count; j++)
        {
            (temp_lp->lut_vp->elements)[ index++ ] =
                      (original_lp->lut_vp->elements)[ j ];
        }

        for (j=0; j<extra_end_count; j++)
        {
            (temp_lp->lut_vp->elements)[ index++ ] = 0.0;
        }

        /*
        ** Now do the spline.
        */
        in_vp.length = new_count;
        in_vp.elements = temp_lp->lut_vp->elements;

        out_vp.length = count;
        out_vp.elements = target_lp->lut_vp->elements;

        NRE(cs_interpolate_vector(&in_vp, temp_offset, temp_step,
                                  &out_vp, count, offset, step));

        for (j=0; j<count; j++)
        {
            if (out_vp.elements[ j ] < DBL_EPSILON)
            {
                out_vp.elements[ j ] = 0.0;
            }
        }

        free_lut(temp_lp);
    }

    verbose_pso(20, "Conversion complete.\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        read_lut_from_config_file
 *
 * Reads lut from a config file
 *
 * This routine reads a lut from a configuration file, checking several
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
 * The lut *result_lp_ptr is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_lp_ptr is freed and set to NULL.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O, lut
 *
 * -----------------------------------------------------------------------------
*/

int read_lut_from_config_file
(
    Lut**       result_lp_ptr,
    const char* env_var,
    const char* directory,
    const char* file_name,
    const char* message_name,
    char*       config_file_name,
    size_t      config_file_name_size
)
{
    char temp_config_file_name[ MAX_FILE_NAME_SIZE ];


    if (is_no_value_word(file_name))
    {
        free_lut(*result_lp_ptr);
        *result_lp_ptr = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    /*
    // Clear the error since there is an execution path which uses
    // "insert_error" without a previous "set_error". Although it should be
    // guaranteed that the error should be set if "read_lut", fails, we
    // are better off not taking changes.
    */
    kjb_clear_error();

    if (read_lut(result_lp_ptr, temp_config_file_name) == ERROR)
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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               read_lut
 *
 *
 * -----------------------------------------------------------------------------
*/

int read_lut(Lut** lut_ptr_ptr, char* file_name)
{
    FILE*        fp;
    int          result;
    Error_action save_error_action = get_error_action();


    NRE(fp = kjb_fopen(file_name, "r"));

    result = fp_read_lut(lut_ptr_ptr, fp);

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

/* =============================================================================
 *                             fp_read_lut
 *
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_lut(Lut** lut_ptr_ptr, FILE* fp)
{
    double  step = 0.0;
    double  offset = 0.0;
    int     lut_len = NOT_SET;


    ERE(fp_read_lut_file_header(fp, &lut_len, &offset, &step));
    ERE(get_target_lut(lut_ptr_ptr, lut_len, offset, step));

    if (fp_ow_read_vector((*lut_ptr_ptr)->lut_vp, fp) == ERROR)
    {
        add_error("Unable to read LUT from %F.", fp);
        free_lut(*lut_ptr_ptr);
        *lut_ptr_ptr = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fp_read_lut_file_header(FILE* fp, int* lut_len_ptr,
                                   double* offset_ptr, double* step_ptr)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int    i;
    int    num_options;
    char** option_list;
    char** option_list_pos;
    char** value_list;
    char** value_list_pos;
    char   line[ 200 ];
    char*  line_pos;
    int    result       = NO_ERROR;
    int    lut_len      = NOT_SET;
    double offset;
    double step;
    int    offset_found = FALSE;
    int    step_found   = FALSE;
    long   save_file_pos;



    ERE(save_file_pos = kjb_ftell(fp));

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
                        if (ss1pi(*value_list_pos, &lut_len) == ERROR)
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
                        else
                        {
                            offset_found = TRUE;
                        }
                    }
                    else if (IC_STRCMP_EQ(*option_list_pos,"s"))
                    {
                        if (ss1d(*value_list_pos, &step) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                        else
                        {
                            step_found = TRUE;
                        }
                    }
                    else
                    {
                        set_error("%q is an invalid LUT header option.",
                                  *option_list_pos);
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
            break;
        }
    }

    if (result != ERROR)
    {
        if ( ! ((lut_len  > 0) && (offset_found) && (step_found)))
        {
            set_error("Incomplete or missing header in LUT %F.", fp);
            result = ERROR;
        }
        else
        {
            *lut_len_ptr = lut_len;
            *offset_ptr = offset;
            *step_ptr   = step;
        }
    }

    ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                write_lut
 *
 * Writes lut to a file.
 *
 * This routine writes a lut to a file.
 *
 * Index: lut, lut I/O
 *
 * -----------------------------------------------------------------------------
 */

int write_lut(const Lut* lp, const char* file_name)
{
    FILE*        fp;
    int          result;
    Error_action save_error_action = get_error_action();


    NRE(fp = kjb_fopen(file_name, "w"));

    result = fp_write_lut(lp, fp);

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
 *                             fp_write_lut
 *
 * Writes lut to a file.
 *
 * This routine writes a lut to a file pointed to by fp.
 *
 * Index: lut, lut I/O
 *
 * -----------------------------------------------------------------------------
 */

int fp_write_lut(const Lut* lut_ptr, FILE* fp)
{
    int result;


    result = fp_write_lut_file_header(fp, lut_ptr->lut_vp->length,
                                      lut_ptr->offset, lut_ptr->step);

    if (result != ERROR)
    {
        result = fp_write_col_vector(lut_ptr->lut_vp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fp_write_lut_file_header(FILE* output_fp, int lut_len, double offset,
                                    double step)
{


    ERE(kjb_fprintf(output_fp, "\n#!"));


    ERE(kjb_fprintf(output_fp, " n=%d", lut_len));
    ERE(kjb_fprintf(output_fp, " o=%.4f", offset));
    ERE(kjb_fprintf(output_fp, " s=%.6f", step));


    ERE(kjb_fprintf(output_fp, "\n\n"));

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int apply_lut(const Lut* lut_ptr, double x, double* y_ptr)
{

    double d_index = SUB_DBL_EPSILON((x - lut_ptr->offset) / lut_ptr->step);
    int    lb      = (int)d_index;
    int    ub      = (int)d_index + 1;
    double upper_weight = d_index - lb;
    double lower_weight = 1.0 - upper_weight;

    if ((lb < 0) || (ub >= lut_ptr->lut_vp->length))
    {
        set_error("Value %.3f outside lookup table range [%.3f, %.3f].",
                  x, lut_ptr->offset,
               lut_ptr->offset + lut_ptr->step * (lut_ptr->lut_vp->length - 1));
        return ERROR;
    }

    ASSERT(upper_weight <= 1.0);
    ASSERT(upper_weight >= 0.0);
    ASSERT(lower_weight <= 1.0);
    ASSERT(lower_weight >= 0.0);

    *y_ptr = lower_weight * lut_ptr->lut_vp->elements[ lb ];
    *y_ptr += upper_weight * lut_ptr->lut_vp->elements[ ub ];

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int apply_lut_2(const Lut* lut_ptr, double x, double* y_ptr)
{

    double d_index = SUB_DBL_EPSILON((x - lut_ptr->offset) / lut_ptr->step);
    int    lb      = kjb_floor(d_index);
    int    ub      = kjb_floor(d_index) + 1;

    ASSERT_IS_NUMBER_DBL(x);

    if (lb < 0)
    {
        dbw();
        dbe(x);
        *y_ptr = lut_ptr->lut_vp->elements[ 0 ];
    }
    else if (ub >= lut_ptr->lut_vp->length)
    {
        dbw();
        dbe(x);
        *y_ptr = lut_ptr->lut_vp->elements[ lut_ptr->lut_vp->length - 1 ];
    }
    else
    {
        double upper_weight = d_index - lb;
        double lower_weight = 1.0 - upper_weight;

        ASSERT(upper_weight <= 1.0);
        ASSERT(upper_weight >= 0.0);
        ASSERT(lower_weight <= 1.0);
        ASSERT(lower_weight >= 0.0);

        *y_ptr = lower_weight * lut_ptr->lut_vp->elements[ lb ];
        *y_ptr += upper_weight * lut_ptr->lut_vp->elements[ ub ];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// For now, we don't assume anything about the LUT, so we cannot even do
// interpolation. Perhaps different kinds of LUT's are in order?
//
// THIS ROUTINE MAKES NO SENSE IF THE LUT IS NOT MONTONIC!
//
*/

int apply_lut_inverse(const Lut* lut_ptr, double y, double* x_ptr)
{
    int     i;
    Vector* lut_vp    = lut_ptr->lut_vp;
    int     len       = lut_vp->length;


    for (i = 0; i < len; i++)
    {
        if (lut_vp->elements[ i ] > y) break;
    }

    *x_ptr = lut_ptr->offset + lut_ptr->step * i;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

