
/* $Id: m_vec_stat.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_vec_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static Vector* fs_data_sum_vp            = NULL;
static Vector* fs_data_min_vp            = NULL;
static Vector* fs_data_max_vp            = NULL;
static Vector* fs_data_squared_sum_vp    = NULL;
static int     fs_num_vector_data_points = NOT_SET;
static int     fs_dirty_vector_stat_data = FALSE;

/* -------------------------------------------------------------------------- */

static int init_vector_data_analysis(int length);

/* -------------------------------------------------------------------------- */


/*
 * =============================================================================
 *                          get_vector_mean_square
 *
 * Computes the average of the squared elements of a vector
 *
 * This routine calculates the average of the squared elements of a vector and
 * puts the result into (*result_ptr). If result_ptr is NULL, this routine does
 * nothing.  If vp is NULL, or vp has zero length, then ERROR is returned, with
 * an error message being set. (This may change as more sophisticated handling
 * of missing values is implemented), The routine average_vector_elements() can
 * be used to for different behaviour in these circumstances. 
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int get_vector_mean_square(const Vector* vp, double *result_ptr)
{
   

    if (result_ptr == NULL)
    {
        return NO_ERROR;
    }
    else if (vp == NULL)
    {
        set_error("Vector to average is NULL."); 
        return ERROR;
    }
    else if (vp->length <= 0)
    {
        set_error("Vector to average has no elements."); 
        return ERROR;
    }
    else 
    {
        *result_ptr = average_vector_squared_elements(vp);
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          get_vector_mean
 *
 * Computes the average of the elements of a vector
 *
 * This routine calculates the average of the elements of a vector and puts the
 * result into (*result_ptr). If result_ptr is NULL, this routine does nothing.
 * If vp is NULL, or vp has zero length, then ERROR is returned, with an error
 * message being set. (This may change as more sophisticated handling of missing
 * values is implemented), The routine average_vector_elements() can be used to
 * for different behaviour in these circumstances. 
 *
 * Note: 
 *     This routine has been changed recently. The previous behaviour is
 *     availabe from the routine  average_vector_elements(). 
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int get_vector_mean(const Vector* vp, double *result_ptr)
{
   

    if (result_ptr == NULL)
    {
        return NO_ERROR;
    }
    else if (vp == NULL)
    {
        set_error("Vector to average is NULL."); 
        return ERROR;
    }
    else if (vp->length <= 0)
    {
        set_error("Vector to average has no elements."); 
        return ERROR;
    }
    else 
    {
        *result_ptr = average_vector_elements(vp);
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_vector_stats
(
 const Vector* vp,
 const Int_vector* skip_vp,
 double*       mean_ptr,
 double*       stdev_ptr,
 int*          n_ptr,
 double*       min_ptr,
 double*       max_ptr
)
{
    int len, i;

    len  = vp->length;

    if ((skip_vp != NULL) && (skip_vp->length != len))
    {
        set_bug("Mismatched vector lengths in get_vector_stats().\n");
        return ERROR;
    }

    ERE(clear_data_stats());

    for (i=0; i<len; i++)
    {
        if ((skip_vp != NULL) && (skip_vp->elements[ i ])) continue;

        ERE(add_data_point(vp->elements[ i ]));
    }

    return get_data_stats(mean_ptr, stdev_ptr, n_ptr, min_ptr, max_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_vector_stats_2(Stat* stat_ptr, const Vector* vp)
{
    int len, i;
    double* pos;


    ERE(clear_data_stats());

    len  = vp->length;
    pos = vp->elements;

    for (i=0; i<len; i++)
    {
        ERE(add_data_point(*pos));
        pos++;
    }

    return get_data_stats_2(stat_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Stat* get_vector_stats_3(const Vector* vp)
{
    int len, i;
    double* pos;


    ERN(clear_data_stats());

    len  = vp->length;
    pos = vp->elements;

    for (i=0; i<len; i++)
    {
      ERN(add_data_point(*pos));
      pos++;
     }

    return get_data_stats_3();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int init_vector_data_analysis(int length)
{
    static int first_time = TRUE;

    if (first_time)
    {
        add_cleanup_function(cleanup_vector_stats);
        first_time = FALSE;
    }

    fs_num_vector_data_points = 0;

    NRE(fs_data_sum_vp = create_zero_vector(length));
    NRE(fs_data_min_vp = create_zero_vector(length));
    NRE(fs_data_max_vp = create_zero_vector(length));
    NRE(fs_data_squared_sum_vp = create_zero_vector(length));

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int add_vector_data_point(const Vector* vp)
{
    int     i;
    int     length;
    double* pos;
    double* sum_pos;
    double* squared_sum_pos;
    double* min_pos;
    double* max_pos;


    if (fs_num_vector_data_points == NOT_SET)
    {
        ERE(init_vector_data_analysis(vp->length));
    }
    else if (vp->length != fs_data_sum_vp->length)
    {
        set_bug("Incorrect vector length in add_vector_data_point.");
        return ERROR;
    }

    length = vp->length;
    pos = vp->elements;

    sum_pos = fs_data_sum_vp->elements;
    min_pos = fs_data_min_vp->elements;
    max_pos = fs_data_max_vp->elements;
    squared_sum_pos = fs_data_squared_sum_vp->elements;

    fs_num_vector_data_points++;

    for (i=0; i<length; i++)
    {
        (*sum_pos) += (*pos);

        if (fs_num_vector_data_points == 1)
        {
            *min_pos = *pos;
            *max_pos = *pos;
        }
        else
        {
            if (*pos < *min_pos) *min_pos = *pos;
            if (*pos > *max_pos) *max_pos = *pos;
        }

        (*squared_sum_pos) += (*pos) * (*pos);

        pos++;
        sum_pos++;
        squared_sum_pos++;
        min_pos++;
        max_pos++;
    }

    fs_dirty_vector_stat_data = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Vector_stat* get_vector_data_stats(void)
{
    Vector_stat* vector_stat_ptr;
    double       temp;
    int          i;
    int          length;
    double*      mean_pos;
    double*      stdev_pos;
    double*      sum_pos;
    double*      squared_sum_pos;


    if (fs_num_vector_data_points == NOT_SET)
    {
        set_bug("Call to get_data_stats before init_vector_data_analysis.");
        return NULL;
    }

    if (fs_num_vector_data_points == 0)
    {
        set_bug("Call to get_vector_data_stats before add_vector_data_point.");
        return NULL;
    }

    if (fs_num_vector_data_points == 1)
    {
        /*
        // CHECK
        //
        // Should this be a bug?
        */
        set_bug("Call to get_vector_data_stats for only one point.");
        return NULL;
    }

    NRN(vector_stat_ptr = TYPE_MALLOC(Vector_stat));

    length = fs_data_sum_vp->length;

    NRN(vector_stat_ptr->mean_vp = create_vector(length));
    NRN(vector_stat_ptr->min_vp = create_vector_copy(fs_data_min_vp));
    NRN(vector_stat_ptr->max_vp = create_vector_copy(fs_data_max_vp));
    NRN(vector_stat_ptr->stdev_vp = create_vector(length));

    vector_stat_ptr->n = fs_num_vector_data_points;

    mean_pos = vector_stat_ptr->mean_vp->elements;
    stdev_pos = vector_stat_ptr->stdev_vp->elements;

    sum_pos = fs_data_sum_vp->elements;
    squared_sum_pos = fs_data_squared_sum_vp->elements;

    for (i=0; i<length; i++)
    {
        *mean_pos = (*sum_pos) / ((double) fs_num_vector_data_points);
        temp = *squared_sum_pos;
        temp -= (*sum_pos) * (*sum_pos) / fs_num_vector_data_points;
        temp /= (fs_num_vector_data_points - 1);

        if (temp < 0.0)
        {
            if (temp < -DBL_EPSILON)
            {
                /*
                // FIX
                //
                // Perhaps it is time we make this a bug ? Perhaps set
                // the threshold on the basis of lenght?
                */

                dbe(temp);
                temp = 0.0;
            }
            temp = 0.0;
        }

        *stdev_pos = sqrt(temp);

        mean_pos++;
        stdev_pos++;
        sum_pos++;
        squared_sum_pos++;
    }

    fs_dirty_vector_stat_data = FALSE;

    return vector_stat_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_vector_stat(Vector_stat* vector_stat_ptr)
{


    if (vector_stat_ptr != NULL)
    {
        free_vector(vector_stat_ptr->mean_vp);
        free_vector(vector_stat_ptr->min_vp);
        free_vector(vector_stat_ptr->max_vp);
        free_vector(vector_stat_ptr->stdev_vp);

        kjb_free(vector_stat_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int clear_vector_stats(void)
{
    if (fs_dirty_vector_stat_data)
    {
        set_bug("Attempt to clear vector stats with dirty stat data.");
        return ERROR;
    }

    fs_num_vector_data_points = NOT_SET;

    free_vector(fs_data_sum_vp);
    free_vector(fs_data_min_vp);
    free_vector(fs_data_max_vp);
    free_vector(fs_data_squared_sum_vp);

    fs_data_sum_vp = NULL;
    fs_data_min_vp = NULL;
    fs_data_max_vp = NULL;
    fs_data_squared_sum_vp = NULL;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void cleanup_vector_stats(void)
{
    fs_dirty_vector_stat_data = FALSE;

    EPE(clear_vector_stats());
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             ow_square_vector_elemenets
 *
 * Squares the elements of a vector
 *
 * This routine calculates element-wise square of a vector, overwriting the input
 * vector with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     negative.
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_square_vector_elements(Vector* source_vp)
{
    int     i;
    int     length;
    double*  source_ptr;


    source_ptr = source_vp->elements;

    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        (*source_ptr) = (*source_ptr)*(*source_ptr);
        source_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

