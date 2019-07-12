
/* $Id: l_4D_arr.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_4D_arr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

double**** allocate_4D_double_array(int num_blocks, int num_planes,
                                    int num_rows, int num_cols)
{
    double**** array;
    double**** array_pos;
    double***  plane_ptr;
    double**   row_ptr;
    double*    col_ptr;
    int       i;
    int       j;
    int       k;


    if ((num_planes <= 0) || (num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(double***, num_blocks));

    plane_ptr = N_TYPE_MALLOC(double**, num_planes * num_blocks);

    if (plane_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    row_ptr = N_TYPE_MALLOC(double*, num_blocks * num_planes * num_rows);

    if (row_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        return NULL;
    }

    col_ptr = N_TYPE_MALLOC(double,
                            num_blocks * num_planes * num_rows * num_cols);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        kjb_free(row_ptr);
        return NULL;
    }

    array_pos = array;

    for (i = 0; i < num_blocks; i++)
    {
        *array_pos = plane_ptr;
        array_pos++;

        for (j = 0; j < num_planes; j++)
        {
            *plane_ptr = row_ptr;
            plane_ptr++;

            for (k = 0; k < num_rows; k++)
            {
                *row_ptr = col_ptr;
                col_ptr += num_cols;
                row_ptr++;
            }
        }
    }

    return array;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

float**** allocate_4D_float_array(int num_blocks, int num_planes,
                                  int num_rows, int num_cols)
{
    float**** array;
    float**** array_pos;
    float***  plane_ptr;
    float**   row_ptr;
    float*    col_ptr;
    int       i;
    int       j;
    int       k;


    if ((num_planes <= 0) || (num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(float***, num_blocks));

    plane_ptr = N_TYPE_MALLOC(float**, num_planes * num_blocks);

    if (plane_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    row_ptr = N_TYPE_MALLOC(float*, num_blocks * num_planes * num_rows);

    if (row_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        return NULL;
    }

    col_ptr = N_TYPE_MALLOC(float,
                            num_blocks * num_planes * num_rows * num_cols);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        kjb_free(row_ptr);
        return NULL;
    }

    array_pos = array;

    for (i = 0; i < num_blocks; i++)
    {
        *array_pos = plane_ptr;
        array_pos++;

        for (j = 0; j < num_planes; j++)
        {
            *plane_ptr = row_ptr;
            plane_ptr++;

            for (k = 0; k < num_rows; k++)
            {
                *row_ptr = col_ptr;
                col_ptr += num_cols;
                row_ptr++;
            }
        }
    }

    return array;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void***** allocate_4D_ptr_array(int num_blocks, int num_planes,
                                int num_rows, int num_cols)
{
    void***** array;
    void***** array_pos;
    void****  plane_ptr;
    void***   row_ptr;
    void**    col_ptr;
    int       i;
    int       j;
    int       k;


    if ((num_planes <= 0) || (num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(void****, num_blocks));

    plane_ptr = N_TYPE_MALLOC(void***, num_planes * num_blocks);

    if (plane_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    row_ptr = N_TYPE_MALLOC(void**, num_blocks * num_planes * num_rows);

    if (row_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        return NULL;
    }

    col_ptr = N_TYPE_MALLOC(void*,
                            num_blocks * num_planes * num_rows * num_cols);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        kjb_free(plane_ptr);
        kjb_free(row_ptr);
        return NULL;
    }

    array_pos = array;

    for (i = 0; i < num_blocks; i++)
    {
        *array_pos = plane_ptr;
        array_pos++;

        for (j = 0; j < num_planes; j++)
        {
            *plane_ptr = row_ptr;
            plane_ptr++;

            for (k = 0; k < num_rows; k++)
            {
                *row_ptr = col_ptr;
                col_ptr += num_cols;
                row_ptr++;
            }
        }
    }

    return array;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_4D_ptr_array(void***** array)
{


    if (array == NULL) return;

    kjb_free(***array);
    kjb_free(**array);
    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_4D_double_array(double**** array)
{


    if (array == NULL) return;

    kjb_free(***array);
    kjb_free(**array);
    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_4D_float_array(float**** array)
{


    if (array == NULL) return;

    kjb_free(***array);
    kjb_free(**array);
    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */



#ifdef __cplusplus
}
#endif

