
/* $Id: s_spectra.c 6352 2010-07-11 20:13:21Z kobus $ */

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
#include "s/s_spectra.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                               create_spectra
 *
 * Creates a spectra of the specified characteristics
 *
 * This routine creates a spectra of the specified characteristics. Spectra
 * should be disposed of with free_spectra().
 *
 * Returns :
 *     On succes, a pointer to a freshly allocated spectra is returned. On
 *     failure, NULL is returned, and an error message is set.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Spectra* debug_create_spectra
(
    int            num_spectra,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type,
    const char*    file_name,
    int            line_number
)
{
    Spectra *sp;


    NRN(sp = DEBUG_TYPE_MALLOC(Spectra, file_name, line_number));

    sp->spectra_mp = DEBUG_create_matrix(num_spectra, num_freq_intervals,
                                         file_name, line_number);

    if (sp->spectra_mp == NULL)
    {
        kjb_free(sp);
        sp = NULL;
    }
    else
    {
        sp->offset = offset;
        sp->step = step;
        sp->type = type;
    }

    return sp;
}


        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Spectra* create_spectra
(
    int            num_spectra,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type
)
{
    Spectra *sp;


    NRN(sp = TYPE_MALLOC(Spectra));

    sp->spectra_mp = create_matrix(num_spectra, num_freq_intervals);

    if (sp->spectra_mp == NULL)
    {
        kjb_free(sp);
        sp = NULL;
    }
    else
    {
        sp->offset = offset;
        sp->step = step;
        sp->type = type;
    }

    return sp;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               free_spectra
 *
 * Frees spectra
 *
 * This routine frees allocted spectra, as created by create_spectra(),
 * copy_spectra(), convert_spectra(), etc. It is safe to pass a NULL.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
*/

void free_spectra(Spectra* spectra_ptr)
{


    if (spectra_ptr != NULL)
    {
        free_matrix(spectra_ptr->spectra_mp);
        kjb_free(spectra_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_target_spectra
 *
 * Gets target spectra for "building block" routines
 *
 * This routine implements the creation/over-writing g used in the KJB
 * library. If *target_sp_ptr is NULL, then this routine creates the spectra. If
 * it is not null, and it is the right size, then this routine does nothing. If
 * it is the wrong size, then it is resized.
 *
 * If an actual resize is needed, then a new spectra of the required size is
 * first created. If the creation is successful, then the old spectra is free'd.
 * The reason is that if the new allocation fails, a calling application should
 * have use of the old spectra. The alternate is to free the old spectra first.
 * This is more memory efficient. A more sophisticated alternative is to free
 * the old spectra if it can be deterimined that the subsequent allocation will
 * succeed. Although such approaches have merit, it is expected that
 * resizing will occur infrequently enought that it is not worth implementing
 * them. Thus the simplest method with good g under most conditions
 * has been used.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
*/


#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_target_spectra
(
    Spectra**      out_sp_ptr,
    int            num_spectra,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type,
    const char*    file_name,
    int            line_number
)
{
    Spectra *out_sp = *out_sp_ptr;


    if ((num_spectra < 0) || (num_freq_intervals < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_sp == NULL)
    {
        NRE(out_sp = debug_create_spectra(num_spectra, num_freq_intervals,
                                          offset, step, type, file_name,
                                          line_number));
        *out_sp_ptr = out_sp;
    }
    else
    {
        ERE(debug_get_target_matrix(&(out_sp->spectra_mp), num_spectra,
                                    num_freq_intervals, file_name,
                                    line_number));
        out_sp->offset = offset;
        out_sp->step   = step;
        out_sp->type   = type;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_spectra
(
    Spectra**      out_sp_ptr,
    int            num_spectra,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type
)
{
    Spectra *out_sp = *out_sp_ptr;


    if ((num_spectra < 0) || (num_freq_intervals < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_sp == NULL)
    {
        NRE(out_sp = create_spectra(num_spectra, num_freq_intervals, offset,
                                    step, type));
        *out_sp_ptr = out_sp;
    }
    else
    {
        ERE(get_target_matrix(&(out_sp->spectra_mp), num_spectra,
                              num_freq_intervals));
        out_sp->offset = offset;
        out_sp->step   = step;
        out_sp->type   = type;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  copy_spectra
 *
 * Copies spectra
 *
 * This routine copies the spectra pointed to by "sp" to that pointed to by
 * "*target_sp_ptr", If *target_sp_ptr is NULL, then the target spectra is
 * created. If it already exists, but is the wrong size, then it is resized.
 * Finally, if it is the correct size, it is over-written.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
 */

int copy_spectra(Spectra** target_sp_ptr, const Spectra* sp)
{
    Spectra*       target_sp;
    double*          cur_target_row_pos;
    double*          cur_row_pos;
    int            num_rows;
    int            num_cols;
    int            i;
    int            j;
    Spectra_origin type;
    double         step;
    double         offset;



    if (sp == NULL)
    {
        free_spectra(*target_sp_ptr);
        *target_sp_ptr = NULL;
        return NO_ERROR;
    }

    num_rows = sp->spectra_mp->num_rows;
    num_cols = sp->spectra_mp->num_cols;
    offset   = sp->offset;
    step     = sp->step;
    type     = sp->type;

    ERE(get_target_spectra(target_sp_ptr, num_rows, num_cols,
                           offset, step, type));

    target_sp = *target_sp_ptr;

    target_sp->type   = sp->type;
    target_sp->step   = sp->step;
    target_sp->offset = sp->offset;

    for (i=0; i<num_rows; i++)
    {
        cur_target_row_pos = target_sp->spectra_mp->elements[ i ];
        cur_row_pos        = sp->spectra_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            *cur_target_row_pos++ = *cur_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  convert_spectra
 *
 * Converts a spectra to one with different characteristics
 *
 * This routine converts a spectra to one with different characteristics. The
 * resulting spectra will have the specified characteristics. If interpolation
 * is necessary, then cubic spline is used. The resulting spectra is pointed to
 * by *target_sp_ptr.
 *
 * If *target_sp_ptr is NULL, then the target spectra is created. If it already
 * exists, but is the wrong size, then it is resized.  Finally, if it is the
 * correct size, it is over-written.
 *
 * Returns:
 *    On success, NO_ERROR is returned. On failure, ERROR is returned, with an
 *    error message being set.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
*/

int convert_spectra
(
    Spectra**      target_sp_ptr,
    const Spectra* original_sp,
    int            count,
    double         offset,
    double         step
)
{


    if (original_sp == NULL)
    {
        free_spectra(*target_sp_ptr);
        *target_sp_ptr = NULL;
        return NO_ERROR;
    }

    verbose_pso(20, "Converting spectra from (%d, %.1f, %.1f) ",
                original_sp->spectra_mp->num_cols, original_sp->offset,
                original_sp->step);
    verbose_pso(20, "to (%d, %.1f, %.1f)\n", count, offset, step);

    if (    (count == original_sp->spectra_mp->num_cols)
         && (IS_EQUAL_DBL(offset, original_sp->offset))
         && (IS_EQUAL_DBL(step, original_sp->step))
       )
    {
        verbose_pso(20, "No conversion required, so just copying.\n");
        ERE(copy_spectra(target_sp_ptr, original_sp));
    }
    else  /* Bummer! Have to convert the original */
    {
        int      num_spectra     = original_sp->spectra_mp->num_rows;
        Spectra_origin type      = original_sp->type;
        double   temp_offset     = original_sp->offset;
        int      temp_count      = original_sp->spectra_mp->num_cols;
        double   temp_step       = original_sp->step;
        double   temp_upper      = temp_offset + (temp_count - 1) * temp_step;
        double   upper;
        int      extra_beg_count = 0;
        int      extra_end_count = 0;
        Spectra* temp_sp;
        int      new_count;
        int      i;
        int      j;
        Spectra *target_sp;


        verbose_pso(20, "Converting with cubic splines.\n");

        ERE(get_target_spectra(target_sp_ptr, num_spectra, count, offset, step,
                               type));

        target_sp = *target_sp_ptr;

        /*
         * The derived spectra will be a spline of the original. However, we
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

            temp_offset -= (extra_beg_count * original_sp->step);
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

        NRE(temp_sp = create_spectra(num_spectra, new_count, temp_offset,
                                     temp_step, type));

        /*
        * Fill in zeros on either side of the "temp" copy of the original.
        */
        for (i=0; i<num_spectra; i++)
        {
            int j_count = 0;

            for (j=0; j<extra_beg_count; j++)
            {
                (temp_sp->spectra_mp->elements)[ i ][ j_count++ ] = 0.0;
            }

            for (j=0; j<temp_count; j++)
            {
                (temp_sp->spectra_mp->elements)[ i ][ j_count++ ] =
                          (original_sp->spectra_mp->elements)[ i ][ j ];
            }

            for (j=0; j<extra_end_count; j++)
            {
                (temp_sp->spectra_mp->elements)[ i ][ j_count++ ] = 0.0;
            }
        }

        /*
        ** Now do the spline.
        */
        for (i=0; i<num_spectra; i++)
        {
            Vector in_vector;
            Vector out_vector;


            in_vector.length = new_count;
            in_vector.elements = (temp_sp->spectra_mp->elements)[ i ];

            out_vector.length = count;
            out_vector.elements = (target_sp->spectra_mp->elements)[ i ];

            NRE(cs_interpolate_vector(&in_vector, temp_offset, temp_step,
                                      &out_vector, count, offset, step));

            for (j=0; j<count; j++)
            {
                if (out_vector.elements[ j ] < DBL_EPSILON)
                {
                    out_vector.elements[ j ] = 0.0;
                }
            }
        }

        free_spectra(temp_sp);
    }

    verbose_pso(20, "Conversion complete.\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             multiply_spectra
 *
 * Multiplies spectra elementwise
 *
 * This routine multiplies all the spectra pointed to by the parameter sp1 by
 * the spectra in sp2, specifed by sp2_index. The parameter sp2_index can be a simple
 * integer, in which case each spectra in sp1 is multiplied by the spectra in
 * sp2 with that sp2_index, producing a result of the same dimension as sp1.
 * Alternatively, if sp2_index is USE_ALL_SPECTRA, then each spectra in sp1 is
 * multiplied by each spectra in sp2, producing a result with the product of the
 * number of spectra in sp1 and sp2.
 *
 * The result is put into (*output_sp_ptr), which is created or resized as
 * necessary.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR of failure, with an appropriate error
 *     message being set.
 *
 * Index: spectra, spectra arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int multiply_spectra
(
    Spectra**      output_sp_ptr,
    const Spectra* sp1,
    int            sp2_index,
    const Spectra* sp2
)
{
    int     num_spectra;
    Vector* row1_vp         = NULL;
    Vector* row2_vp         = NULL;
    Vector* product_vp      = NULL;
    Matrix* output_spect_mp;
    int     i;
    int     j;
    int     num1;
    int     num2;
    int     first;


    ERE(check_spectra_are_comparable(sp1, sp2));

    if (sp2_index == USE_ALL_SPECTRA)
    {
        num1 = sp1->spectra_mp->num_rows;
        first = 0;
    }
    else
    {
        num1 = 1;
        first = sp2_index;
    }

    num2 = sp2->spectra_mp->num_rows;
    num_spectra = num1 * num2;

    ERE(get_target_spectra(output_sp_ptr, num_spectra,
                           sp1->spectra_mp->num_cols, sp1->offset, sp1->step,
                           GENERIC_SPECTRA));

    output_spect_mp = (*output_sp_ptr)->spectra_mp;

    for (i=0; i<num1; i++)
    {
        ERE(get_matrix_row(&row1_vp, sp1->spectra_mp, first + i));

        for (j=0; j<num2; j++)
        {
            ERE(get_matrix_row(&row2_vp, sp2->spectra_mp, j));
            ERE(multiply_vectors(&product_vp, row1_vp, row2_vp));
            ERE(put_matrix_row(output_spect_mp, product_vp, i * num2 + j));
        }
    }

    free_vector(row1_vp);
    free_vector(row2_vp);
    free_vector(product_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           check_spectra_are_comparable
 *
 * Checks that spectra are comparable
 *
 * This routine checks that the number of intervals, the step size and the
 * offset of spectra sp1 and sp2 are the same. If they are not, then ERROR is
 * returned, with an error message being set. If these characteristics are the
 * same, the spectra are considered to be comparable, and NO_ERROR is returned.
 * The "type" of the spectra need not be the same for them to be considered
 * comparable.
 *
 * Note:
 *     This routine does not test whether or not the number of spectra in the
 *     two spectra sets are the same.
 *
 * Related:
 *     Spectra
 *
 * Index: spectra, spectra error routines
 *
 * -----------------------------------------------------------------------------
*/

int check_spectra_are_comparable(const Spectra* sp1, const Spectra* sp2)
{


    if (sp1->spectra_mp->num_cols != sp2->spectra_mp->num_cols)
    {
        set_error("Spectra are not comparable.");
        add_error("One has %d intervals, whereas the other has %d.",
                  sp1->spectra_mp->num_cols, sp2->spectra_mp->num_cols);
        return ERROR;
    }

    if ( ! IS_EQUAL_DBL(sp1->step, sp2->step))
    {
        set_error("Spectra are not comparable.");
        add_error("One has step size %.2f, whereas the other has %.2f.",
                  sp1->step, sp2->step);
        return ERROR;
    }


    if ( ! IS_EQUAL_DBL(sp1->offset, sp2->offset))
    {
        set_error("Spectra are not comparable.");
        add_error("One has offset %.2f, whereas the other has %.2f.",
                  sp1->offset, sp2->offset);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

