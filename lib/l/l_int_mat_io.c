
/* $Id: l_int_mat_io.c 21520 2017-07-22 15:09:04Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed to the documentation of this code).
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

#include "l/l_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "l/l_config.h"
#include "l/l_int_mat_io.h"
#include "l/l_io.h"
#include "l/l_lib.h"
#include "l/l_parse.h"
#include "l/l_string.h"
#include "l/l_verbose.h"
#include "l/l_sys_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define KJB_DATA_HEAD_SIZE  64
#define KJB_RAW_INT_MATRIX_STRING  "kjb raw int matrix\n\n\f\n"

/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                        read_int_matrix_from_config_file
 *
 * Reads an integer matrix from a config file
 *
 * This routine reads an integer matrix from a configuration file, checking
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
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_mpp is freed and set to NULL.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Related:
 *     get_config_file, kjb_get_error
 *
 * Index: configuration files, I/O, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_matrix_from_config_file(Int_matrix** result_mpp,
                                     const char* env_var,
                                     const char* directory,
                                     const char* file_name,
                                     const char* message_name,
                                     char* config_file_name,
                                     size_t config_file_name_size)
{
    char    temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int     result;


    UNTESTED_CODE();

    if (is_no_value_word(file_name))
    {
        free_int_matrix(*result_mpp);
        *result_mpp = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    result = read_int_matrix(result_mpp, temp_config_file_name);

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
 *                             read_int_matrix
 *
 * Reads an integer matrix from a file
 *
 * This routine reads an integer matrix from a file. If the file_name is NULL
 * or a null string, then stdin is assumed. If this is the case, and if the
 * source is not a file (i.e. a pipe), then this routine will fail.
 *
 * Several read strategies are tried until one succeeds. The first strategy is
 * to assume the file is a matlab integer matrix (see
 * fp_read_int_matrix_from_matlab_file(3)). The second strategy is to assume
 * that the file has a header file (see fp_read_int_matrix_with_header(3)).The
 * final strategy is to assume that the file is a formatted ascii file, and the
 * matrix dimensions are deduced from the number of rows and columns. The
 * number of columns in each row must be the same.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_matrix(Int_matrix** result_mpp, const char* file_name)
{
    FILE* fp;
    int   result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_int_matrix(result_mpp, fp);

    /*
    //  The file pointer based read routines can return EOF because they are
    //  used as building blocks. But for name based read routines, we expect at
    //  least one matrix. This comment does not apply to the routines below
    //  which read a matrix of specified size. Those will fail if there is not
    //  enough data, so an anologous check is not needed.
    */
    if (result == EOF)
    {
        set_error("Unable to read an integer matrix file from %F.", fp);
        result = ERROR;
    }

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fp_read_int_matrix
 *
 * Reads an integer matrix from a file
 *
 * This routine reads an integer matrix from a file. If the file_name is NULL
 * or a null string, then stdin is assumed. If this is the case, and if the
 * source is not a file (i.e. a pipe), then this routine will fail.
 *
 * Several read strategies are tried until one succeeds.  The first strategy is
 * to assume the file is a matlab integer matrix (see
 * fp_read_int_matrix_from_matlab_file(3)). The second strategy is to assume
 * that the file has a header file (see fp_read_int_matrix_with_header(3)).The
 * final strategy is to assume that the file is a formatted ascii file, and the
 * matrix dimensions are deduced from the number of rows and columns. The number
 * of columns in each row must be the same.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_matrix(Int_matrix** result_mpp, FILE* fp)
{
    int       result    = NOT_FOUND;
    Path_type path_type;
    long      save_file_pos;


    path_type = fp_get_path_type(fp);

    if (path_type == PATH_ERROR)
    {
        result = ERROR;
    }

    if (path_type != PATH_IS_REGULAR_FILE)
    {
        set_error("Input %F is not a regular file.", fp);
        add_error("Read of intgeger matrix aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_int_matrix(result_mpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        /*
        // If it is not a Matlab file, then NOT_FOUND is returned. If there is
        // an error reading a Matlab file, then ERROR is returned.
        */
        result = fp_read_int_matrix_from_matlab_file(result_mpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));;
        }
    }

    if (result == NOT_FOUND)
    {
        /*
        // If it is not an integer matrix file with a header, then NOT_FOUND is
        // returned. If there is an error reading the file, then ERROR is
        // returned.
        */
        result = fp_read_int_matrix_with_header(result_mpp,fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_formatted_int_matrix(result_mpp, fp);
        EPE(result);

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_read_raw_int_matrix
 *
 * Reads kjb raw int matrix
 *
 * This routine reads kjb raw matrix integer data from a file pointed to by a
 * FILE object into a matrix.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE object as returned by "kjb_fopen".
 *
 * Returns:
 *     If the file does not seem to be a kjb raw matrix file, then NOT_FOUND is
 *     returned. NO_ERROR is returned on success, and ERROR is returned on
 *     success, with an appropriate error message being set.
 *
 * Note:
 *     This routine is currently NOT byte order safe! (It's on the TODO list).
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_int_matrix(Int_matrix** result_mpp, FILE* fp)
{
    int     num_rows, num_cols;
    long    bytes_used_so_far;
    int     byte_order;
    char    head_str[ KJB_DATA_HEAD_SIZE ];
    int     pad;
    off_t num_bytes;


    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (num_bytes < KJB_DATA_HEAD_SIZE) return NOT_FOUND;

    ERE(kjb_fread_exact(fp, head_str, sizeof(head_str)));
    head_str[ sizeof(head_str) - 1 ] = '\0';

    if ( ! STRCMP_EQ(head_str, KJB_RAW_INT_MATRIX_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, num_rows));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, num_cols));
    ERE(FIELD_READ(fp, pad));

    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if ((num_rows == 0) && (num_cols == 0))
    {
        /* Special case: The matrix is mean to be null. */
        free_int_matrix(*result_mpp);
        *result_mpp = NULL;
    }
    else if (num_rows < 0)
    {
        set_error("Invalid data in matrix file %F.");
        add_error("The number of rows is negative ( %d ).", num_rows);
        return ERROR;
    }
    else if (num_cols < 0)
    {
        set_error("Invalid data in matrix file %F.");
        add_error("The number of cols is negative ( %d ).", num_cols);
        return ERROR;
    }
    else if (num_bytes < (off_t)(num_rows * num_cols * sizeof(int)))
    {
        set_error("Invalid data in matrix file %F.");
        add_error("The number of bytes should be at least %d.",
                  bytes_used_so_far + num_rows * num_cols * sizeof(int));
        return ERROR;
    }
    else
    {
        ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));

        ERE(kjb_fread_exact(fp, (*result_mpp)->elements[ 0 ],
                            num_rows * num_cols * sizeof(int)));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_read_int_matrix_from_matlab_file
 *
 * Reads an integer matrix from matlab file
 *
 * This routine reads an integer matrix from a matlab file. If the file is not
 * a matlab file, then NOT_FOUND is returned. Currently, we support exactly one
 * matlab file format, namely 4 byte integers, PC format. This will be extended
 * at needed, but likely not before.
 *
 * "fp" points to a FILE object specifying the file to read the data from.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     NO_ERROR on success, NOT_FOUND if the file is not a matlab integer file,
 *     and ERROR on failure, with an appropriate error message being set.
 *
 * Documentor:
 *     Kobus Barnard
 *
 * Related:
 *     read_int_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_matrix_from_matlab_file(Int_matrix** result_mpp, FILE* fp)
{
    kjb_int32 type;
    kjb_int32 num_rows, num_cols;
    kjb_int32 imaginary_componants;
    kjb_int32 name_len;
    off_t num_bytes;
    int   reversed_int;
    Bool  reversed_data_flag = FALSE;
    Int_matrix* mp;
    int   i, j;
    long  bytes_used_so_far;


    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (num_bytes  < 20) return NOT_FOUND;

    ERE(kjb_fread_4_bytes(fp, &type));

    if (type != 20)
    {
        reverse_four_bytes(&type, &reversed_int);
        type = reversed_int;

        if (type == 20)
        {
            reversed_data_flag = TRUE;
        }
        else
        {
            return NOT_FOUND;
        }
    }

    if (reversed_data_flag)
    {
        ERE(kjb_fread_4_bytes_backwards(fp, &num_rows));
        ERE(kjb_fread_4_bytes_backwards(fp, &num_cols));
        ERE(kjb_fread_4_bytes_backwards(fp, &imaginary_componants));
        ERE(kjb_fread_4_bytes_backwards(fp, &name_len));
    }
    else
    {
        ERE(kjb_fread_4_bytes(fp, &num_rows));
        ERE(kjb_fread_4_bytes(fp, &num_cols));
        ERE(kjb_fread_4_bytes(fp, &imaginary_componants));
        ERE(kjb_fread_4_bytes(fp, &name_len));
    }

    if (    (imaginary_componants != 0)
         || (num_bytes != 20 + name_len + 4 * num_rows * num_cols)
       )
    {
        return NOT_FOUND;
    }

    ERE(kjb_fseek(fp, (long)name_len, SEEK_CUR));

    ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));
    mp = *result_mpp;

    if (reversed_data_flag)
    {
        /*
        // Fortran style.
        */
        for (j = 0; j < num_cols; j++)
        {
            for (i = 0; i < num_rows; i++)
            {
                ERE(kjb_fread_4_bytes_backwards(fp, &(mp->elements[ i ][ j ])));
            }
        }
    }
    else
    {
        /*
        // Does not work. Fortran style!
        //
        // ERE(kjb_fread_exact(fp, &(mp->elements[ 0 ][ 0 ]),
        //                    (size_t)(4 * num_rows * num_cols)));
        */
        for (j = 0; j < num_cols; j++)
        {
            for (i = 0; i < num_rows; i++)
            {
                ERE(kjb_fread_4_bytes(fp, &(mp->elements[ i ][ j ])));
            }
        }

    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               fp_read_int_matrix_with_header
 *
 * Reads data row-wise from a FILE into a new integer matrix
 *
 * This routine reads data in a file pointed to by a FILE object into an
 * integer matrix whose number of columns is not known to the calling routine.
 * The data file MUST contain a matrix size header otherwise NOT_FOUND is
 * returned.  The routine then reads <num_rows>*<num_cols> data into the matrix
 * row wise. The restrictions on formatting are minimal.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE object as returned by "kjb_fopen".
 *
 * The file is scanned from the current position for the presence of a matrix
 * size header of the form:
 * |   #! rows=<num-matrix-rows> cols=<num-matrix-cols>
 * where <num-matrix-rows>,<num-matrix-cols> are positive integers.
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * Data is read from the first matrix size header found until the next matrix
 * or spectra header (if present) is located. The current file position is NOT
 * reset to the starting after the function returns, so this function can be
 * used to read multiple matrices from the same file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Authors:
 *     Lindsay Martin and Kobus Barnard
 *
 * Documentors:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_matrix_with_header(Int_matrix** result_mpp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Int_matrix* mp;
    char    line[ LARGE_IO_BUFF_SIZE ];
    char    num_buff[ 200 ];
    int     num_rows = NOT_SET;
    int     num_cols = NOT_SET;
    int     i, j;
    char*   line_pos;
    int     result = fp_read_matrix_size_header(fp, &num_rows, &num_cols);


    if (result == ERROR)
    {
        set_error("Error reading matrix size header from %F", fp);
        return ERROR;
    }
    else if (result == NOT_FOUND)
    {
        return NOT_FOUND;
    }

    /*
    // Because this routine is interuptable, we have to initialize it.
    // Otherwise, on interuption the initialization checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_int_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    line[ 0 ] = '\0';
    line_pos = line;

    for (i=0; i<num_rows; i++)
    {
        for (j = 0; j<num_cols; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff,
                                        " ,\t"))
            {
                ERE(result = BUFF_GET_REAL_LINE(fp, line));

                if (result == EOF)
                {
                    set_error("Unexpected EOF in %F.", fp);
                    return ERROR;
                }
                line_pos = line;

                if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff,
                                            " ,\t"))
                {
                    set_error("Missing data on row %d of %F.", (i+1), fp);
                    return ERROR;
                }
            }

            result = ss1i(num_buff, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading an integer from %F.",fp);
                return ERROR;
            }
            else if (io_atn_flag)
            {
                halt_all_output = FALSE;
                set_error("Processing interrupted.");
                return ERROR;
            }
        }
    }

    while (BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
    {
        warn_pso("Extra element %q in matrix in %F is ignored.\n",
                 num_buff, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_read_formatted_int_matrix
 *
 * Reads an integer matrix from data in a formatted FILE
 *
 * This routine reads an integer matrix from a file containing data formatted
 * into fixed rows and columns. The matrix dimensions are controlled by the
 * formatting of the data in the file. For example, if the input file contains
 * 5 rows of data, each with 3 columns, then the output matrix will be 5 X 3.
 *
 * "fp" points to a FILE object specifying the file to read the data from.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix, kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_formatted_int_matrix(Int_matrix** result_mpp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Int_matrix*         mp;
    char                line[LARGE_IO_BUFF_SIZE  ];
    char                num_buff[ 200 ];
    int                 num_cols, num_rows;
    int                 i, j;
    char*               line_pos;
    int                 scan_res;


    num_rows = count_real_lines(fp);

    if (num_rows == ERROR)
    {
        insert_error("Read of %F as a formatted integer matrix failed.", fp);
        return ERROR;
    }

    if ((num_rows == 0) || (num_rows == EOF))
    {
        return EOF;
    }

    ERE(BUFF_GET_REAL_LINE(fp, line));
    line_pos = line;

    num_cols = 0;

    while (BUFF_GEN_GET_TOKEN_OK(&line_pos,num_buff, " ,\t"))
    {
        num_cols++;
    }

    if (num_cols == 0)
    {
        set_error("Unable to read at least one matrix colunmn from from %F.",
                  fp);
        return ERROR;
    }

    /*
    // Because this routine is interuptable, we have to initialize it, otherwise
    // on interuption the initialization checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_int_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    for (i=0; i<num_rows; i++)
    {
        /* Already have first line, so skip read if i is 0. */
        if (i != 0)
        {
            ERE(BUFF_GET_REAL_LINE(fp, line));
        }

        line_pos = line;

        for (j=0; j<num_cols; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos,num_buff,
                                        " ,\t"))
            {
                set_error("Missing data on row %d of %F.", (i+1), fp);
                return ERROR;
            }

            scan_res = ss1i(num_buff, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading an integer from %F.", fp);
                return ERROR;
            }
            else if (io_atn_flag)
            {
                halt_all_output = FALSE;
                set_error("Processing interrupted.");
                return ERROR;
            }
        }
    }

    /*
    // Suck up remaining lines to position file pointer after soft EOF, in the
    // case that the read stopped at a soft EOF.
    */

    /*CONSTCOND*/
    while (TRUE)
    {
        int read_res = BUFF_GET_REAL_LINE(fp, line);

        if (read_res == EOF) break;

        if (read_res < 0)
        {
            return read_res;
        }

        if (read_res > 0)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             read_int_matrix_by_rows
 *
 * Reads data row-wise from a file into a integer matrix
 *
 * This routine reads data contained in a file specified by a file name into an
 * integer matrix whose number of columns is known.  Currently there must be
 * exactly one number per line in the file.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL or the
 * first character is null, input is expected from STDIN instead of a file.
 *
 * "num_cols" indicates the number of columns the created matrix will
 * contain. The number of rows in the new matrix depends on the number
 * of data elements in the file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix_by_cols, fp_read_int_matrix_by_rows,
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_matrix_by_rows(Int_matrix** result_mpp, const char* file_name,
                            int num_cols)
{
    FILE* fp;
    int   result;


    UNTESTED_CODE();

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_int_matrix_by_rows(result_mpp, fp, num_cols);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   fp_read_int_matrix_by_rows
 *
 * Reads data row-wise from a FILE into a new integer matrix
 *
 * This routine reads data in a file pointed to by a FILE object into an
 * integer matrix whose number of columns is known. The matrix will be created.
 * Currently there must be exactly one number per line in the file.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE object as returned by "kjb_fopen".
 *
 * "num_cols" indicates the number of columns the new matrix will
 * contain. The number of rows in the new matrix depends on the number
 * of data elements in the file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix_by_cols, fp_read_int_matrix_by_rows
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_matrix_by_rows(Int_matrix** result_mpp, FILE* fp, int num_cols)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Int_matrix* mp;
    char    line[ 200 ];
    int     num_lines;
    int     num_rows;
    int     i,j;
    int     result = NO_ERROR;


    UNTESTED_CODE();

    ERE(num_lines = count_real_lines(fp));

    if (num_lines == 0)
    {
        set_error("Expecting at least one number in %F.", fp);
        return ERROR;
    }

    if ((num_lines % num_cols) != 0)
    {
        set_error("The number of data lines in %F is not divisible by %d.",
                  fp, num_cols);
        return ERROR;
    }

    num_rows = num_lines / num_cols;

    /*
    // Because this routine is interuptable, we have to initialize it, otherwise
    // on interuption the initialization checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_int_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            result = BUFF_GET_REAL_LINE(fp, line);

            if (result == EOF)
            {
                result = ERROR;
                set_error("Not enough data in %F for reading matrix by rows.",
                          fp);
            }
            else if (io_atn_flag)
            {
                set_error("Processing interrupted.");
                halt_all_output = FALSE;
                result = ERROR;
            }

            if (result == ERROR)
            {
                break;
            }

            result = ss1i(line, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading an integer from %F.",fp);
                break;
            }
        }
    }

    if (result == ERROR) return result;

#ifndef __lint
    /*
    // Suck up remaining lines to position file pointer after soft EOF, in the
    // case that the read stopped at a soft EOF.
    */
    while (BUFF_GET_REAL_LINE(fp, line) >= 0)
    {
        ; /* Do nothing. */
    }
#endif

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        read_int_matrix_by_cols
 *
 * Reads data column-wise from a file into a new integer matrix
 *
 * This routine reads data contained in a file specified by a file name into an
 * integer matrix whose number of rows is known. The matrix will be created.
 * Currently there must be exactly one number per line in the file.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL or the
 * first character is null, input is expected from STDIN instead of a file.
 *
 * "num_rows" indicates the number of rows the created matrix will
 * contain. The number of columns in the new matrix depends on the number
 * of data elements in the file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix_by_rows, fp_read_int_matrix_by_cols,
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_matrix_by_cols(Int_matrix** result_mpp, const char* file_name,
                            int num_rows)
{
    FILE* fp;
    int   result;


    UNTESTED_CODE();

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_int_matrix_by_cols(result_mpp, fp, num_rows);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   fp_read_int_matrix_by_cols
 *
 * Reads data column-wise from a FILE into a new integer matrix
 *
 * This routine reads data in a file pointed to by a FILE object into an
 * integer matrix whose number of rows is known. The matrix will be created.
 * Currently there must be exactly one number per line in the file.
 *
 * The integer matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE object as returned by "kjb_fopen".
 *
 * "num_rows" indicates the number of rows the new matrix will
 * contain. The number of columns in the new matrix depends on the number
 * of data elements in the file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix_by_rows, fp_read_int_matrix_by_cols
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_matrix_by_cols(Int_matrix** result_mpp, FILE* fp, int num_rows)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Int_matrix* mp;
    char    line[ 200 ];
    int     num_lines;
    int     num_cols;
    int     i,j;
    int     result = NO_ERROR;


    UNTESTED_CODE();

    ERE(num_lines = count_real_lines(fp));

    if (num_lines == 0)
    {
        set_error("Expecting at least one number in %F.", fp);
        return ERROR;
    }

    if ((num_lines % num_rows) != 0)
    {
        set_error("The number of data lines in %F is not divisible by %d.",
                  num_rows);
        return ERROR;
    }

    num_cols = num_lines / num_rows;

    /*
    // Because this routine is interuptable, we have to initialize it, otherwise
    // on interuption the initializatio checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_int_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_int_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    for (j=0; j<num_cols; j++)
    {
        for (i=0; i<num_rows; i++)
        {
            result = BUFF_GET_REAL_LINE(fp, line);

            if (result == EOF)
            {
                set_error("Not enough data reading matrix by columns from %F.",
                          fp);
                result = ERROR;
            }
            else if (io_atn_flag)
            {
                set_error("Processing interrupted.");
                halt_all_output = FALSE;
                result = ERROR;
            }


            if (result == ERROR)
            {
                break;
            }

            result = ss1i(line, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading an integer from %F.", fp);
                break;
            }
        }
     }

    if (result == ERROR) return ERROR;

#ifndef __lint
    /*
    // Suck up remaining lines to position file pointer after soft EOF, in the
    // case that the read stopped at a soft EOF.
    */
    while (BUFF_GET_REAL_LINE(fp, line) >= 0)
    {
        ; /* Do nothing. */
    }
#endif

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_ow_read_formatted_int_matrix
 *
 * Reads an integer matrix from data in a formatted FILE
 *
 * This routine reads an integer matrix of a specific size from a file
 * containing data formatted into fixed rows and columns. The matrix dimensions
 * are assumed to be identical to those of the input matrix which must exist.
 * The data in the file must have one line of data per row.
 *
 * "fp" points to a FILE object specifying the file to read the data from. "mp"
 * points to the matrix which will recieve the data.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_int_matrix, kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_formatted_int_matrix(Int_matrix* mp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    char                line[ 10000 ];
    char                num_buff[ 200 ];
    int                 num_cols        = mp->num_cols;
    int                 num_rows        = mp->num_rows;
    int                 i, j;
    char*               line_pos;
    int                 scan_res;


    for (i=0; i<num_rows; i++)
    {
        ERE(BUFF_GET_REAL_LINE(fp, line));

        line_pos = line;

        for (j=0; j<num_cols; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
            {
                set_error("Missing data on row %d of %F.", (i+1), fp);
                add_error("Expecting %d items on a line but found %d.",
                          num_cols, j);
                return ERROR;
            }

            scan_res = ss1i(num_buff, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading an integer from %F.", fp);
                return ERROR;
            }
            /* If this is set, a signal handler might have halted output. */
            else if (io_atn_flag)
            {
                halt_all_output = FALSE;
                set_error("Processing interrupted.");
                return ERROR;
            }

        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_ow_read_int_matrix_by_rows
 *
 * Reads data row-wise from a file into an existing integer matrix
 *
 * This routine reads an integer matrix from a file as specified by a file
 * pointer into a matrix. The matrix must already exist. Currently there must
 * be exactly one number per line in the file.
 *
 * "mp" is a pointer to a Int_matrix object. This matrix must already
 * exist, and have the predefined size 'mp->num_rows' X 'mp->num_cols'.
 *
 * "fp" points to a FILE structure associated with the input stream as
 * returned by "kjb_fopen".
 *
 * Returns:
 *     ERROR, if "mp" is NULL, if EOF is encountered before num_rows X num_cols
 *     elements have been read, or if an error occurs reading a floating point
 *     number.  EOF, if the contents of the file is empty.  NO_ERROR on success.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_and create_int_matrix_bu_rows, fp_ow_read_int_matrix_by_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_int_matrix_by_rows(Int_matrix* mp, FILE* fp)
{
    char  line[ LARGE_IO_BUFF_SIZE ];
    int   num_cols;
    int   num_rows;
    int   i,j;
    long  read_res;
    int   scan_res;


    UNTESTED_CODE();

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
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
                    set_error("Not enough data reading matrix by rows from %F.",
                              fp);
                    add_error("Expecting a %d by %d integer matrix.",
                              mp->num_rows, mp->num_cols);
                    return ERROR;
                }
            }

            scan_res = ss1i(line, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading integer from %F.", fp);
                return ERROR;
            }
        }
     }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_ow_read_int_matrix_by_cols
 *
 * Reads data column-wise from a file into an integer matrix
 *
 * This routine reads an integer matrix from a file as specified by a file
 * pointer into a matrix. The matrix must already exist. Currently there must
 * be exactly one number per line in the file.
 *
 * "mp" is a pointer to a Int_matrix object. This matrix must already
 * exist, and have the predefined size 'mp->num_rows' X 'mp->num_cols'.
 *
 * "fp" points to a FILE structure associated with the input stream as
 * returned by "kjb_fopen".
 *
 * Returns:
 *     ERROR, if "mp" is NULL, if EOF is encountered before num_rows X num_cols
 *     elements have been read, or if an error occurs reading a floating point
 *     number.  EOF, if the file is empty.  NO_ERROR on success.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_and_create_int_matrix_by_columns, fp_ow_read_int_matrix_by_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_int_matrix_by_cols(Int_matrix* mp, FILE* fp)
{
    char  line[ 200 ];
    int   num_cols;
    int   num_rows;
    int   i,j;
    long  read_res;
    int   scan_res;


    UNTESTED_CODE();

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (j=0; j<num_cols; j++)
    {
        for (i=0; i<num_rows; i++)
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
                    set_error(
                         "Not enough data reading matrix by columns from %F.",
                         fp);
                    add_error("Expecting a %d by %d integer matrix.",
                              mp->num_rows, mp->num_cols);
                    return ERROR;
                }
            }
            scan_res = ss1i(line, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading integer from %F.", fp);
                return ERROR;
            }
        }
     }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       fp_read_matrix_size_header
 *
 * Reads matrix size header in matrix file.
 *
 * This routine reads the matrix size header in file indicated by the file
 * pointer indicated by the argument "fp". If the header contains information
 * about the number of matrix rows, the number of matrix columns, or both,
 * then the corresponding variables whose pointers are arguments are set.
 * Variables are not changed unless there is information in the header.
 *
 * The variables corresponding to the pointer arguments "num_rows_ptr" and
 * "num_cols_ptr" can be tested on returnare assigned a value of NOT_SET before calling this function,
 * the values eturn to see if a matrix size header was found
 * in the file.
 *
 * The matrix size header has the format:
 *|    #! rows=<num-matrix-rows> cols=<num-matrix-cols>
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * where <num-matrix-rows> and <num-matrix-cols> are positive
 * integers. Note that one or both of these values may be present in a header.
 *
 * Returns:
 *    Either NO_ERROR if a matrix size header was successfully read,
 *    or ERROR on a file error or if no header is present.
 *
 * Note:
 *    This routine can fail if the stream is a pipe. The argument fp must point
 *    to something that can be "seeked". Exaclty what that is can depend on the
 *    OS.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_size_header(FILE* fp, int* num_rows_ptr, int* num_cols_ptr)
{
    IMPORT int kjb_comment_char;
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 100 ];
    char*          line_pos;
    long           start_file_pos;   /* File position at function call */
    int            num_rows = NOT_SET;
    int            num_cols = NOT_SET;
    int            result   = NO_ERROR;


    ERE(start_file_pos = kjb_ftell(fp));

    while ((num_rows == NOT_SET) || (num_cols == NOT_SET))
    {
        int read_result;

        read_result  = BUFF_FGET_LINE(fp, line);

        if (read_result == ERROR) result = ERROR;

        if (read_result < 0) break; /* Bail out of while (TRUE) */

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

            if (*line_pos == '!')
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
                    /* Look for flag indicating the number of matrix rows */
                    if (IC_STRCMP_EQ(*option_list_pos,"rows"))
                    {
                        if (ss1pi(*value_list_pos, &num_rows) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }

                    /* Look for flag indicating the number of matrix columns */
                    else if (IC_STRCMP_EQ(*option_list_pos,"cols"))
                    {
                        if (ss1pi(*value_list_pos, &num_cols) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }
                    value_list_pos++;
                    option_list_pos++;
                } /* for (i=0; i<num_options; i++) */

                free_options(num_options, option_list, value_list);

                if (result == ERROR)
                {
                    break;
                }
            } /* if (*line_pos == '!') */
        } /* else if (*line_pos == kjb_comment_char) */
        else
        {
            result = NO_ERROR;
            break;
        }
    } /* end while ( TRUE ) - all "breaks" wind up here */

    if (result == ERROR)
    {
        /* Rewind file to starting position */
        (void)kjb_fseek(fp, start_file_pos, SEEK_SET);
        return ERROR;
    }
    else if ((num_rows == NOT_SET) || (num_cols == NOT_SET))
    {
        /* Rewind file to starting position */
        ERE(kjb_fseek(fp, start_file_pos, SEEK_SET));
        return NOT_FOUND;
    }
    else
    {
        *num_rows_ptr = num_rows;
        *num_cols_ptr = num_cols;
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   fp_write_int_matrix_with_title
 *
 * Debugging routine that outputs an integer matrix
 *
 * This routine outputs the integer matrix pointed to by "mp" to the file pointer
 * indicated by "fp", and labels the output with a user defined "title".
 *
 * Use as a quick-and-dirty debugging aid only.
 *
 * Returns:
 *     NO_ERROR on success, ERROR otherwise.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix
 *
 * Index:  I/O, matrices, debugging, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix_with_title(const Int_matrix* mp, FILE* fp,
                                   const char* title)
{
    IMPORT int kjb_comment_char;
    int        i, j;


    if (mp == NULL)
    {
        if (title != NULL)
        {
            ERE(kjb_fprintf(fp, title));
            ERE(kjb_fprintf(fp, ": "));
        }
        ERE(kjb_fprintf(fp, "NULL\n"));

        return NO_ERROR;
    }

    if (title != NULL)
    {
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
        ERE(kjb_fprintf(fp, "%c ", kjb_comment_char));
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
    }

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, "%5d", mp->elements[i][j]));

            if (j < (mp->num_cols-1))
            {
                ERE(kjb_fprintf(fp," "));
            }
            else
            {
                ERE(kjb_fprintf(fp,"\n"));
            }
        }
    }

    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_int_matrix
 *
 * Writes an integer matrix to a file specified by name
 *
 * This routine outputs an integer matrix to a file specified by the input file
 * name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or the first character is null, then output is directed to STDOUT.
 * Otherwise, the file is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Int_matrix object whose contents are to be
 * written. If it is NULL, then this routine is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     fp_write_int_matrix, read_int_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_matrix(const Int_matrix* mp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_int_matrix(mp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

        if (write_result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_write_int_matrix
 *
 * Write an integer matrix to a file specified by FILE pointer
 *
 * This routine outputs an integer matrix to a file specified by a pointer to a
 * FILE object.
 *
 * "fp" is a pointer to a FILE object as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Int_matrix object to output. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix, fp_read_int_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix(const Int_matrix* mp, FILE* fp)
{
    int i, j;


    if (mp == NULL) return NO_ERROR;

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, "%5d", mp->elements[i][j]));

            if (j < (mp->num_cols-1))
            {
                ERE(kjb_fprintf(fp," "));
            }
            else
            {
                ERE(kjb_fprintf(fp,"\n"));
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_raw_int_matrix
 *
 * Writes a matrix to a file specified by name as raw data
 *
 * This routine outputs an Int_atrix to a the specified file as raw data.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix object whose contents are to be written.
 * If it is NULL, then a matrix with the number of rows and columns is zero,
 * which will be read in as NULL by the corresponding read routine.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Related:
 *     fp_write_matrix, read_matrix
 *
 * Note:
 *     This routine is currently NOT byte order safe! (It's on the TODO list).
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_int_matrix(const Int_matrix* mp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    if (kjb_isatty(fileno(fp)))
    {
        set_error("Attempt to write a raw matrix to a terminal aborted.\n");
        write_result = ERROR;
    }
    else
    {
        write_result = fp_write_raw_int_matrix(mp, fp);
    }

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

        if (write_result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        fp_write_raw_int_matrix
 *
 * Write a matrix to a file specified by FILE pointer as raw data.
 *
 * This routine outputs a Matrix to a file specified by a pointer to a
 * FILE object ias raw data.
 *
 * "fp" is a pointer to a FILE object as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Matrix object to output. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Note:
 *     This routine is currently NOT byte order safe! (It's on the TODO list).
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_int_matrix(const Int_matrix* mp, FILE* fp)
{
    int i;
    int num_rows   = 0;
    int num_cols   = 0;
    int byte_order = 1;
    char head_str[ KJB_DATA_HEAD_SIZE ];
    int pad = 0;

    if (mp != NULL)
    {
        num_rows = mp->num_rows;
        num_cols = mp->num_cols;
    }

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_INT_MATRIX_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, num_rows));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, num_cols));
    ERE(FIELD_WRITE(fp, pad));

    if ((num_rows > 0) && (num_cols > 0))
    {
        ERE(kjb_fwrite_2(fp, &(mp->elements[ 0 ][ 0 ]),
                         (sizeof(int) * num_rows * num_cols),
                         NULL));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_int_matrix_rows
 *
 * Outputs an integer matrix in row order to a file specified by name
 *
 * This routine outputs an integer matrix in row order to a file specified by
 * its file name. All columns of a given row are output before advancing to the
 * next row.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the integer matrix whose contents are to be
 * written. If it ss NULL, then this is a NOP.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the matrix elements are written in fixed format
 * according to "8.4f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     fp_write_int_matrix_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_matrix_rows(const Int_matrix* mp, const char* file_name)
{
    FILE* fp;
    int   result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_int_matrix_rows(mp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

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

/* =============================================================================
 *                      fp_write_int_matrix_rows
 *
 * Outputs an integer matrix in row order to a FILE specified by a pointer
 *
 * This routine outputs an integer matrix in row order to a file specified by a
 * pointer to a FILE object. All columns of a given row are output before
 * advancing to the next row.
 *
 * "fp" is a pointer to a FILE object as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the Int_matrix object whose contents are to be
 * written.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix_rows, fp_write_int_matrix_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix_rows(const Int_matrix* mp, FILE* fp)
{
    int   i, j;

    if (mp == NULL) return NO_ERROR;

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, "%6d", mp->elements[i][j]));
        }
        ERE(kjb_fprintf(fp, "\n"));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_int_matrix_cols
 *
 * Outputs an integer matrix in column order to a file specified by name
 *
 * This routine outputs an integer matrix in column order to a file specified
 * by its file name. All row entries of a given column are output before
 * advancing to the next column.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Int_matrix object whose contents are to be written.
 * written. If it is NULL, then this routine is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix_rows, fp_write_int_matrix_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_matrix_cols(const Int_matrix* mp, const char* file_name)
{
    FILE* fp;
    int   result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_int_matrix_cols(mp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

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

/* =============================================================================
 *                      fp_write_int_matrix_cols
 *
 * Outputs an integer matrix in column order to a pointer to FILE
 *
 * This routine outputs an integer matrix in column order to a file specified by
 * a pointer to a FILE object. All row entriess of a given column are
 * output before advancing to the next column.
 *
 * "fp" is a pointer to a FILE object as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the Int_matrix object whose contents are to be
 * written.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix_cols, fp_write_int_matrix_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix_cols(const Int_matrix* mp, FILE* fp)
{
    int   i, j;


    if (mp == NULL) return NO_ERROR;

    for (j=0; j<mp->num_cols; j++)
    {
        for (i=0; i<mp->num_rows; i++)
        {
            ERE(kjb_fprintf(fp, "%6d", mp->elements[i][j]));
        }

        ERE(kjb_fprintf(fp, "\n"));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                    write_int_matrix_with_header
 *
 * Writes an integer matrix to a file row-wise with header
 *
 * This routine outputs an integer matrix to a file specified by the input file
 * name row by row with a header.
 *
 * "file_name" is a pointer to a character array containing the name of the file
 * to write the matrix contents to. If "file_name" is NULL or equal to '\0',
 * output is directed to STDOUT. Otherwise, the file is created or the existing
 * copy is overwritten.
 *
 * "mp" is a pointer to the matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine writes a matrix with 0 rows and 0 columns
 * which will become a NULL matrix with the anologous read routine.
 *
 * The matrix * size header has the form:
 * |   #! rows=<num-matrix-rows> cols=<num-matrix-cols>
 * where <num-matrix-rows>,<num-matrix-cols> are positive integers.
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_matrix_with_header
(
    const Int_matrix* mp,
    const char*       file_name
)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    UNTESTED_CODE();

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_int_matrix_with_header(mp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        if (write_result == ERROR)
        {
            push_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        close_result = kjb_fclose(fp);

        if (write_result == ERROR)
        {
            pop_error_action();
        }
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   fp_write_int_matrix_with_header
 *
 * Writes data row-wise to a FILE prefaced by an integer matrix header
 *
 * This routine writes data in an integer matrix to a file. Data is prefaced by
 * a matrix size header indicating the number of rows and columns in the matrix
 * being output.
 *
 * "fp" points to a FILE object as returned by "kjb_fopen".
 *
 * A matrix size header is written at the current file position. The matrix
 * size header has the form:
 * |   #! rows=<num-matrix-rows> cols=<num-matrix-cols>
 * where <num-matrix-rows>,<num-matrix-cols> are positive integers.
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * Data is then written in row order to the file. The current file position
 * is NOT reset to the starting after the function returns, so this function
 * can be used to write multiple matrices from the same file.
 *
 * Returns:
 *     NO_ERROR on success,
 *     ERROR on failure, with "kjb_error" set to a descriptive message.
 *
 * Authors:
 *     Lindsay Martin and Kobus Barnard
 *
 * Documentors:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix_with_header(const Int_matrix* mp, FILE* fp)
{
    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(fp_write_matrix_size_header(fp, (mp == NULL) ? 0: mp->num_rows, (mp == NULL) ? 0: mp->num_cols));

    ERE(fp_write_int_matrix_rows(mp, fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       fp_write_matrix_size_header
 *
 * Writes matrix size header to a matrix file.
 *
 * This routine writes the matrix size information contined in num_rows and
 * num_cols to the file indicated by the file pointer "fp". The matrix size is
 * taken from the "num_rows" and "num_cols" fields of the input Int_matrix.
 *
 * The matrix size header has the format:
 *|    #! rows=<num-matrix-rows> cols=<num-matrix-cols>
 * where <num-matrix-rows> and <num-matrix-cols> are positive integers.
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * "num_rows" and "num_cols" fields are tested to ensure they are positive
 * integers. If one or both of the values are negative, ERROR is returned and
 * the error message set.
 *
 * NOTE:
 *     This function is not normally required unless more than one integer
 *     matrix is stored in the file, or if the calling program can make no
 *     assumptions about the matrix size.
 *
 * Returns:
 *    Either NO_ERROR on success, or ERROR, with an appropriate error message
 *    being set.
 *
 * Related:
 *     read_matrix_size_header
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_size_header(FILE* fp, int num_rows, int num_cols)
{

    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        result;


    if ( (num_rows < 0) || (num_cols < 0) )
    {
        result = ERROR;
        set_error("Error writing matrix size header to: \"%F\"", fp);

        if (num_rows <= 0)
        {
            add_error("Number of rows in input matrix is negative or zero.");
        }
        else
        {
            add_error("Number of columns in input matrix is negative or zero.");
        }
    }
    else
    {
        result = kjb_fprintf(fp, "\n%c%c rows=%d cols=%d\n\n",
                             kjb_comment_char, kjb_header_char,
                             num_rows, num_cols);
    }

    if (result > 0)
        result = NO_ERROR;
    else
        result = ERROR;

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                    write_int_matrix_to_matlab_file
 *
 * Writes an integer matrix to a matlab file
 *
 * This routine outputs an integer matrix to a matlab file specified by the
 * input file name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the integer matrix whose contents are to be written. If
 * it is NULL, then this is treated as bug because we don't have a concept of a
 * null matlab matrix.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     fp_write_int_matrix, read_int_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_matrix_to_matlab_file(const Int_matrix* mp,
                                    const char* file_name)
{
    static const char* suffixes[ 2 ] = {"mat", NULL};
    FILE*              fp;
    int                write_result;
    int                close_result = NO_ERROR;
    char               matlab_variable_name[ 1000 ];
    char               dir_str[ 1000 ];
    char               suffix_str[ 1000 ];



    if ((file_name == NULL) || (*file_name == '\0') || (mp == NULL))
    {
        /*
        // The normal convection is not supported for matlab files.
        */
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_base_name(file_name,
                      dir_str, sizeof(dir_str),
                      matlab_variable_name, sizeof(matlab_variable_name),
                      suffix_str, sizeof(suffix_str),
                      suffixes));

    if (skip_because_no_overwrite(file_name)) return NO_ERROR;

    if (FIND_CHAR_YES(matlab_variable_name, ' '))
    {
        set_error("Write of matlab file aborted due to spaces in file name.");
        return ERROR;
    }

    if (strlen(matlab_variable_name) > 31)
    {
        warn_pso("Write of matlab file with more than 31 characters ");
        warn_pso("in variable name.\n");
        warn_pso("I have no idea what will happen as a result!\n");
    }

    NRE(fp = kjb_fopen(file_name, "w"));

    write_result = fp_write_int_matrix_to_matlab_file(mp, fp,
                                                      matlab_variable_name);

    /*
    // Since we don't implement the normal convention in the case of matlab
    // output, this  must be TRUE, but keep it in case we do it differently
    // later.
    */
    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

        if (write_result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_write_int_matrix_to_matlab_file
 *
 * Write an integer matrix to a matlab file
 *
 * This routine outputs an integer matrix to a matlab file specified by a
 * pointer to a FILE object.
 *
 * "fp" is a pointer to a FILE object as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Int_matrix object to output. If it is NULL, then
 * this is treated as bug because we don't have a concept of a null matlab
 * matrix.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_int_matrix, fp_read_int_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_matrix_to_matlab_file(const Int_matrix* mp,
                                       FILE* fp,
                                       const char* matlab_variable_name)
{
    int i, j;
    int type = 20;
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int imaginary_componants = 0;
    size_t name_size = strlen(matlab_variable_name) + 1;
    size_t extra_nulls = 0;
    int name_with_extra_nulls_size = name_size;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    FIELD_WRITE(fp, type);
    FIELD_WRITE(fp, num_rows);
    FIELD_WRITE(fp, num_cols);
    FIELD_WRITE(fp, imaginary_componants);

    while (name_with_extra_nulls_size %4 != 0)
    {
        name_with_extra_nulls_size++;
        extra_nulls++;
    }

    FIELD_WRITE(fp, name_with_extra_nulls_size);

    kjb_fwrite_2(fp, matlab_variable_name, name_size, NULL);

    ASSERT(extra_nulls < 4);

    kjb_fwrite(fp, "\0\0\0", extra_nulls);

    for (j=0; j<mp->num_cols; j++)
    {
        for (i=0; i<mp->num_rows; i++)
        {
            FIELD_WRITE(fp, mp->elements[ i ][ j ]);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_int_matrix
(
    const char*       output_dir,
    const char*       file_name,
    const Int_matrix* mp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_int_matrix(mp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_int_matrix
(
    const char*       output_dir,
    const char*       file_name,
    const Int_matrix* mp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_raw_int_matrix(mp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

