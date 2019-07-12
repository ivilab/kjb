
/* $Id: m_io.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_io.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int read_fixed_array(FILE* fp, double* array, int length)
{
    char  line[ LARGE_IO_BUFF_SIZE ];
    char  element_buff[ 200 ];
    char* line_pos;
    int   i;
    long  read_res;
    int   scan_res;


    if ((fp == NULL) || (array == NULL))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    i = 0;

    while (i<length)
    {
        ERE(read_res = BUFF_GET_REAL_LINE(fp, line));

        if (read_res == EOF)
        {
            if (i==0)
            {
                return EOF;
            }
            else
            {
                set_error("Not enough data read from %F (%d numbers needed).",
                          fp, length);
                return ERROR;
            }
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            if (i < length)
            {
                if (is_scientific_notation_number(element_buff))
                {
                    scan_res = ss1snd(element_buff, &(array[ i ]));
                }
                else
                {
                    scan_res = ss1d(element_buff, &(array[ i ]));
                }

                if (scan_res == ERROR)
                {
                    insert_error("Error reading floating point number from %F.",
                                 fp);
                    return ERROR;
                }
                i++;
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_and_create_array(char* file_name, int* num_elements_ptr,
                          double** array_ptr)
{
    FILE* data_fp;
    int   num_elements;
    double* data_array;
    double* data_array_pos;
    char  line[ LARGE_IO_BUFF_SIZE ];
    char* line_pos;
    char  element_buff[ 200 ];
    int   scan_res, read_res;


    NRE(data_fp = kjb_fopen(file_name, "r"));

    num_elements = 0;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            num_elements++;
        }
    }

    if (num_elements == 0)
    {
        Error_action save_error_action = get_error_action();

        set_error("Expecting at least one number in file %F.", data_fp);
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        (void)kjb_fclose(data_fp);
        set_error_action(save_error_action);

        return ERROR;
    }
    else
    {
        rewind(data_fp);
    }

    NRE(data_array = DBL_MALLOC(num_elements));

    data_array_pos = data_array;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);
            kjb_free(data_array);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            if (is_scientific_notation_number(element_buff))
            {
                scan_res = ss1snd(element_buff, data_array_pos);
            }
            else
            {
                scan_res = ss1d(element_buff, data_array_pos);
            }

            if (scan_res == ERROR)
            {
                Error_action save_error_action = get_error_action();

                insert_error("Error reading floating point number from %F.",
                             data_fp);
                set_error_action(FORCE_ADD_ERROR_ON_ERROR);
                (void)kjb_fclose(data_fp);
                kjb_free(data_array);
                set_error_action(save_error_action);

                return ERROR;
            }
            data_array_pos++;
        }
    }

    ERE(kjb_fclose(data_fp));

    *num_elements_ptr = num_elements;
    *array_ptr = data_array;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

