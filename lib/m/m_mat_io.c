
/* $Id: m_mat_io.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_mat_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define KJB_DATA_HEAD_SIZE  64
#define KJB_RAW_MATRIX_STRING  "kjb raw matrix\n\n\f\n"
#define KJB_RAW_MATRIX_VECTOR_STRING  "kjb raw matrix vector\n\n\f\n"

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                        read_matrix_from_config_file
 *
 * Reads a matrix from a config file
 *
 * This routine reads a matrix from a configuration file, checking several
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
 *     shared directory is "~kobus" (compiled in constant) and the programmer's
 *     directory is "~kobus" (another compiled in constant). Then the following
 *     files are tried in the order listed.
 * |            ./y
 * |            ~/x/y
 * |            ~kobus/x/y
 * |            ~kobus/x/y
 *
 * The file name actually used is put into the buffer config_file_name whose
 * size must be passed in via max_len;
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_mpp is freed and set to NULL.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Related:
 *     get_config_file
 *
 * Index: configuration files, I/O, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_matrix_from_config_file
(
    Matrix**    result_mpp,            /* Double pointer to Matrix result.    */
    const char* env_var,               /* Config file name from envirnonment. */
    const char* directory,             /* Directory to scan for config file.  */
    const char* file_name,             /* Name of config file to search for.  */
    const char* message_name,          /* File name for error messages.       */
    char*       config_file_name,      /* Name of actual config file found.   */
    size_t      config_file_name_size  /* Size of actual config file name.    */
)
{
    char    temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int     result;


    if (is_no_value_word(file_name))
    {
        free_matrix(*result_mpp);
        *result_mpp = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    result = read_matrix(result_mpp, temp_config_file_name);

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
 *                             read_matrix
 *
 * Reads a matrix from file
 *
 * This routine reads a matrix from a file.  If the file_name is NULL or a null
 * string, then stdin is assumed. If this is the case, and if the source is not
 * a file (i.e. a pipe), then this routine will fail.
 *
 * Several read strategies are tried until one succeeds.  The first strategy is
 * to assume that the matrix is in raw (binary) format (see fp_read_matrix(3)).
 * If that fails, the rouint assume that the file has a header file (see
 * fp_read_matrix_with_header(3)). The final strategy is to assume that the file
 * is a formatted ascii file, and the matrix dimensions are deduced from the
 * number of rows and columns (see fp_read_formatted_matrix(3)). The number of
 * columns in each row must be the same.  For example, if the input file
 * contains 5 rows of data, each with 3 columns, then the output matrix will be
 * 5 X 3.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_matrix(Matrix** result_mpp, const char* file_name)
{
    FILE*     fp;
    int       result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_matrix(result_mpp, fp);

    /*
    //  The file pointer based read routines can return EOF because they are
    //  used as building blocks. But for name based read routines, we expect at
    //  least one matrix. This comment does not apply to the routines below
    //  which read a matrix of specified size. Those will fail if there is not
    //  enough data, so an anologous check is not needed.
    */

    /* if ((result == EOF) || (result == NOT_FOUND))
    {
        set_error("Unable to read a matrix from %F.", fp);
        result = ERROR;
    } */

    if (result == NOT_FOUND)
    {
        set_error("Unable to read a matrix from %F.", fp);
        result = ERROR;
    }

    if (result == EOF)
    {
        get_target_matrix(result_mpp, 0, 0);
        result = NO_ERROR;
    }

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fp_read_matrix
 *
 * Reads a matrix from data in a formated file
 *
 * This routine reads a matrix from a file.  If the file_name is NULL or a null
 * string, then stdin is assumed. If this is the case, and if the source is not
 * a file (i.e. a pipe), then this routine will fail.  If there are multiple
 * matrices in the file separated with soft EOFs, then the reading continues to
 * the next soft (or hard) EOF.
 *
 * Several read strategies are tried until one succeeds.  The first strategy is
 * to assume that the matrix is in raw (binary) format (see fp_read_matrix(3)).
 * If that fails, the rouint assume that the file has a header file (see
 * fp_read_matrix_with_header(3)). The final strategy is to assume that the file
 * is a formatted ascii file, and the matrix dimensions are deduced from the
 * number of rows and columns (see fp_read_formatted_matrix(3)). The number of
 * columns in each row must be the same.  For example, if the input file
 * contains 5 rows of data, each with 3 columns, then the output matrix will be
 * 5 X 3.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_matrix, write_matrix, fp_read_matrix_with_header,
 *     fp_read_formatted_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix(Matrix** result_mpp, FILE* fp)
{
    int       result        = NOT_FOUND;
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
        add_error("Read of matrix aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_matrix(result_mpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        /*
        // If it is not a matrix file with a header, then NOT_FOUND is
        // returned. If there is an error reading the file, then ERROR is
        // returned.
        */
        result = fp_read_matrix_with_header(result_mpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_formatted_matrix(result_mpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            fp_read_raw_matrix
 *
 * Reads kjb raw matrix from a stream
 *
 * This routine reads kjb raw matrix data from a file pointed to by fp.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to type FILE as returned by "kjb_fopen".
 *
 * Returns:
 *     If the file does not seem to be a kjb raw matrix file, then NOT_FOUND is
 *     returned. NO_ERROR is returned on success, and ERROR is returned on
 *     success, with an appropriate error message being set.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_matrix(Matrix** result_mpp, FILE* fp)
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

    if ( ! STRCMP_EQ(head_str, KJB_RAW_MATRIX_STRING))
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
        free_matrix(*result_mpp);
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
    else if (num_bytes < (off_t)(num_rows * num_cols * sizeof(double)))
    {
        set_error("Invalid data in matrix file %F.");
        add_error("The number of bytes should be at least %d.",
                  bytes_used_so_far + num_rows * num_cols * sizeof(double));
        return ERROR;
    }
    else
    {
        ERE(get_target_matrix(result_mpp, num_rows, num_cols));

        if ((num_rows > 0) && (num_cols > 0))
        {
            ERE(kjb_fread_exact(fp, (*result_mpp)->elements[ 0 ],
                                num_rows * num_cols * sizeof(double)));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_read_matrix_with_header
 *
 * Reads data row-wise from a FILE into a new matrix
 *
 * This routine reads data in a file pointed to by a FILE into a matrix whose
 * number of columns is not known to the calling routine. The data file MUST
 * contain a matrix size header otherwise NOT_FOUND is returned. The routine
 * then reads <num_rows>*<num_cols> data into the matrix row wise. The
 * restrictions on formatting are minimal.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE as returned by "kjb_fopen".
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
 * If both the number of rows and the number of cols are zero, then it is
 * assumed that a NULL matrix is to be read.
 *
 * Returns:
 *     If the file does not seem to be a matrix file with a header, then
 *     NOT_FOUND is returned. NO_ERROR is returned on success, and ERROR is
 *     returned on success, with an appropriate error message being set.
 *
 * Documentors:
 *     Lindsay Martin, Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_with_header(Matrix** result_mpp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* mp;
    char    line[ LARGE_IO_BUFF_SIZE ];
    char    num_buff[ 200 ];
    int     num_rows;
    int     num_cols;
    int     i, j;
    char*   line_pos;
    int     result = fp_read_matrix_size_header(fp, &num_rows, &num_cols);


    if (result == ERROR)
    {
        set_error("Error reading matrix size header from %F when", fp);
        add_error("reading matrix by rows. Matrix size header not found.");
        return ERROR;
    }
    else if (result == NOT_FOUND)
    {
        return NOT_FOUND;
    }
    else if ((num_rows == 0) && (num_cols == 0))
    {
        free_matrix(*result_mpp);
        *result_mpp = NULL;
        return NO_ERROR;
    }

    /*
    // Because this routine is interuptable, we have to initialize it.
    // Otherwise, on interuption the initialization checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    line[ 0 ] = '\0';
    line_pos = line;

    for (i = 0; i<num_rows; i++)
    {
        for (j = 0; j<num_cols; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
            {
                ERE(result = BUFF_GET_REAL_LINE(fp, line));

                if (result == EOF)
                {
                    set_error("Unexpected EOF in %F.", fp);
                    return ERROR;
                }
                line_pos = line;

                if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
                {
                    set_error("Missing data for matrix element (%d,%d) from matrix in %F.",
                              (i+1), (j+1), fp);
                    return ERROR;
                }
            }

            result = ss1snd(num_buff, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading floating point number from %F.",fp);
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
 *                     fp_read_formatted_matrix
 *
 * Reads a matrix from data in a formatted FILE
 *
 * This routine reads a matrix from a file containing data formatted into fixed
 * rows and columns. The matrix dimensions are controlled by the formatting of
 * the data in the file. For example, if the input file contains 5 rows of
 * data, each with 3 columns, then the output matrix will be 5 X 3.  If there
 * are multiple matrices in the file separated with soft EOFs, then the reading
 * continues to the next soft (or hard) EOF.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to type FILE as returned by "kjb_fopen".
 *
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     read_matrix, kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_formatted_matrix(Matrix** result_mpp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* mp;
    char    line[ LARGE_IO_BUFF_SIZE ];
    char    num_buff[ 200 ];
    int     num_cols;
    int     num_rows;
    int     i,j;
    char*    line_pos;
    int     scan_res;


    ERE(num_rows = count_real_lines(fp));

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
    // on interuption the initializatio checker will kick in.
    */
#ifdef TRACK_MEMORY_ALLOCATION
    ERE(get_zero_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(result_mpp, num_rows, num_cols));
#endif

    mp = *result_mpp;

    for (i=0; i<num_rows; i++)
    {
        /* Already have first one. */
        if (i != 0)
        {
            ERE(BUFF_GET_REAL_LINE(fp, line));
        }

        line_pos = line;

        for (j=0; j<num_cols; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff,
                                        " ,\t"))
            {
                set_error("Missing data on row %d of %F.", (i+1), fp);
                return ERROR;
            }

            scan_res = ss1snd(num_buff, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
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
 *                           read_matrix_by_rows
 *
 * Reads data row-wise from a file into a matrix
 *
 * This routine reads data contained in a file specified by a file name into a
 * matrix whose number of columns is known. The matrix will be created or
 * resized if necessary. Currently there must be exactly one number per line in
 * the file.
 *
 * The matrix *result_mpp is created or resized as necessary.
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
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_matrix_by_cols, fp_read_matrix_by_rows,
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_matrix_by_rows
(
    Matrix**    result_mpp,
    const char* file_name,
    int         num_cols
)
{
    FILE*   fp;
    int result;


    UNTESTED_CODE();

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_matrix_by_rows(result_mpp, fp, num_cols);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_read_matrix_by_rows
 *
 * Reads data row-wise from a FILE
 *
 * This routine reads data in a file pointed to by a FILE into a matrix
 * whose number of columns is known. The matrix will be created or resized if
 * necessary. Currently there must be exactly one number per line in the file.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE as returned by "kjb_fopen".
 *
 * "num_cols" indicates the number of columns the new matrix will
 * contain. The number of rows in the new matrix depends on the number
 * of data elements in the file.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_matrix_by_cols, fp_read_matrix_by_rows
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_by_rows(Matrix** result_mpp, FILE* fp, int num_cols)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* mp;
    char    line[ LARGE_IO_BUFF_SIZE ];
    int     num_lines;
    int     num_rows;
    int     i,j;
    int     result = NO_ERROR;


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
    ERE(get_zero_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(result_mpp, num_rows, num_cols));
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

            result = ss1snd(line, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading floating point number from %F.",fp);
                break;
            }
        }
    }

    if (result == ERROR) return result;

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

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           read_matrix_by_cols
 *
 * Reads data column-wise from a file into a matrix
 *
 * This routine reads data contained in a file specified by a file name into a
 * matrix whose number of rows is known. The matrix will be created or resized
 * if necessary. Currently there must be exactly one number per line in the
 * file.
 *
 * The matrix *result_mpp is created or resized as necessary.
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
 *     read_matrix_by_rows, fp_read_matrix_by_cols,
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_matrix_by_cols
(
    Matrix**    result_mpp,
    const char* file_name,
    int         num_rows
)
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

    result = fp_read_matrix_by_cols(result_mpp, fp, num_rows);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   fp_read_matrix_by_cols
 *
 * Reads data column-wise from a FILE into a new matrix
 *
 * This routine reads data in a file pointed to by a FILE into a matrix
 * whose number of rows is known. The matrix will be created or resized if
 * necessary. Currently there must be exactly one number per line in the file.
 *
 * The matrix *result_mpp is created or resized as necessary.
 *
 * "fp" points to a FILE as returned by "kjb_fopen".
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
 *     read_matrix_by_rows, fp_read_matrix_by_cols
 *     kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_by_cols(Matrix** result_mpp, FILE* fp, int num_rows)

{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* mp;
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
    ERE(get_zero_matrix(result_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(result_mpp, num_rows, num_cols));
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

            result = ss1snd(line, ((mp->elements)[i])+j);

            if (result == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
                break;
            }
        }
     }

    if (result == ERROR)  return ERROR;

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
            result = read_res;
            break;
        }

        if (read_res > 0)
        {
            SET_CANT_HAPPEN_BUG();
            result = ERROR;
        }
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_ow_read_formatted_matrix
 *
 * Reads a matrix from data in a formatted FILE
 *
 * This routine reads a matrix of a specific size from a file containing data
 * formatted into fixed rows and columns. The matrix dimensions are assumed to
 * be identical to those of the input matrix which must exist. The data in the
 * file must have one line of data per row.
 *
 * "fp" points to a FILE specifying the file to read the data from. "mp"
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
 *     read_matrix, kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_formatted_matrix(Matrix* mp, FILE* fp)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    char    line[ 10000 ];
    char    num_buff[ 200 ];
    int     num_cols = mp->num_cols;
    int     num_rows = mp->num_rows;
    int     i,j;
    char*    line_pos;
    int     scan_res;


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

            scan_res = ss1snd(num_buff, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_ow_read_matrix_by_rows
 *
 * Reads data row-wise from a file into an existing matrix
 *
 * This routine reads a matrix from a file as specified by a file pointer
 * into a matrix. The matrix must already exist. Currently there must be
 * exactly one number per line in the file.
 *
 * "fp" points to a FILE structure associated with the input stream as
 * returned by "kjb_fopen".
 *
 * "mp" is a pointer to a Matrix . This matrix must already
 * exist, and have the predefined size 'mp->num_rows' X 'mp->num_cols'.
 *
 * Returns:
 *     ERROR, if EOF is encountered before num_rows X num_cols elements have
 *     been read, or if an error occurs reading a floating point number.  EOF,
 *     if the contents of the file is empty.  NO_ERROR on success.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     read_and create_matrix_bu_rows, fp_ow_read_matrix_by_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_matrix_by_rows(Matrix* mp, FILE* fp)
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
                    add_error("Expecting a %d by %d matrix.", mp->num_rows,
                              mp->num_cols);
                    return ERROR;
                }
            }

            scan_res = ss1snd(line, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
                return ERROR;
            }
        }
     }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_ow_read_matrix_by_cols
 *
 * Reads data column-wise from a file into a matrix
 *
 * This routine reads a matrix from a file as specified by a file pointer
 * into a matrix. The matrix must already exist. Currently there must be
 * exactly one number per line in the file.
 *
 * "fp" points to a FILE structure associated with the input stream as
 * returned by "kjb_fopen".
 *
 * "mp" is a pointer to a Matrix . This matrix must already
 * exist, and have the predefined size 'mp->num_rows' X 'mp->num_cols'.
 *
 * Returns:
 *     ERROR, if EOF is encountered before num_rows X num_cols elements have
 *     been read, or if an error occurs reading a floating point number.  EOF,
 *     if the file is empty.  NO_ERROR on success.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     read_and_create_matrix_by_columns, fp_ow_read_matrix_by_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_matrix_by_cols(Matrix* mp, FILE* fp)
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
                    add_error("Expecting a %d by %d matrix.", mp->num_rows,
                              mp->num_cols);
                    return ERROR;
                }
            }
            scan_res = ss1snd(line, ((mp->elements)[i])+j);

            if (scan_res == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
                return ERROR;
            }
        }
     }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_matrix
 *
 * Writes a matrix to a file specified by name
 *
 * This routine outputs a Matrix to a file specified by the input file
 * name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or the first character is null, then output is directed to STDOUT.
 * Otherwise, the file is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be
 * written. If the matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the matrix elements are written in fixed format
 * according to "8.4f".
 *
 * Warning:
 *     This routine does not write at full precision! For full precision use
 *     either write_matrix_full_precision() or write_raw_matrix(). 
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     fp_write_matrix, read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix(const Matrix* mp, const char* file_name)
{


    return write_matrix_2(mp, file_name, (Word_list*)NULL, (Word_list*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_matrix_2
 *
 * Writes a matrix to a file specified by name with labels
 *
 * This routine outputs a Matrix to a file specified by the input file
 * name with labels
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be
 * written. If the matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the matrix elements are written in fixed format
 * according to "8.4f".
 *
 * If row_labels is not null, then the routine produces an extra column with
 * those labels. (NOT IMPLEMENTED YET).
 *
 * If col_labels is not null, then the routine produces an extra row with
 * those labels.
 *
 * Warning:
 *     This routine does not write at full precision! For full precision use
 *     either write_matrix_full_precision() or write_raw_matrix(). 
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     fp_write_matrix, read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_2
(
    const Matrix*    mp,
    const char*      file_name,
    const Word_list* row_labels,
    const Word_list* col_labels
)
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

    write_result = fp_write_matrix_2(mp, fp, row_labels, col_labels);

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
 *                           fp_write_matrix
 *
 * Write a matrix to a file specified by FILE pointer
 *
 * This routine outputs a Matrix to a file specified by a pointer to a
 * FILE.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Matrix to output. If the matrix is NULL, then this
 * routine is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Warning:
 *     This routine does not write at full precision! For full precision use
 *     either write_matrix_full_precision() or write_raw_matrix(). 
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix, fp_read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix
(
    const Matrix*    mp,
    FILE*            fp
)
{

    return fp_write_matrix_2(mp, fp, (Word_list*)NULL, (Word_list*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                           fp_write_matrix_2
 *
 * Write a matrix to a file specified by FILE pointer with labels
 *
 * This routine outputs a Matrix to a file specified by a pointer to a
 * FILE with labels.
 *
 * "fp" is a pointer to FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Matrix to output. If the matrix is NULL, then this
 * routine is a NOP.
 *
 * If row_labels is not null, then the routine produces an extra column with
 * those labels. (NOT IMPLEMENTED YET).
 *
 * If col_labels is not null, then the routine produces an extra row with
 * those labels.
 *
 * Warning:
 *     This routine does not write at full precision! For full precision use
 *     either write_matrix_full_precision() or write_raw_matrix(). 
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix, fp_read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_2
(
    const Matrix*    mp,
    FILE*            fp,
    __attribute__((unused)) const Word_list* dummy_row_labels,
    const Word_list* col_labels
)
{
    int    i, j;
    double max = DBL_MAX;
    char format_str[ 100 ];
    unsigned int width = 9;
    int num_rows;
    int num_cols;

    if (mp == NULL) return NO_ERROR;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if (col_labels != NULL)
    {
        if (col_labels->num_words != mp->num_cols)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        for (i = 0; i < col_labels->num_words; i++)
        {
            width = MAX_OF(width, strlen(col_labels->words[ i ]));
        }
    }

    /*
     * Kobus, 06/10/18. 
     *
     * We used to do this. But it is safer to ensure that each row and each
     * column has an element that does not loose precision.

    max = max_abs_matrix_element(mp);

    */

    for (i=0; i<num_rows; i++)
    {
        double row_max = 0.0;

        for (j=0; j<num_cols; j++)
        {
            row_max = MAX_OF(row_max, ABS_OF(mp->elements[i][j]));
        }

        max = MIN_OF(max, row_max);
    }

    for (j=0; j<num_cols; j++)
    {
        double col_max = 0.0;

        for (i=0; i<num_rows; i++)
        {
            col_max = MAX_OF(col_max, ABS_OF(mp->elements[i][j]));
        }

        max = MIN_OF(max, col_max);
    }


    ERE(kjb_sprintf(format_str, sizeof(format_str), "%s%d.5", "%", width));

    if ((max < 0.01) || (max > 100000.0 ))
    {
        BUFF_CAT(format_str, "e");
    }
    else
    {
        BUFF_CAT(format_str, "f");
    }

    if (col_labels != NULL)
    {
        for (i = 0; i < col_labels->num_words; i++)
        {
            unsigned int len = strlen(col_labels->words[ i ]);

            kjb_fputs(fp, col_labels->words[ i ]);
            print_blanks(fp, 1 + width - len);
        }
        kjb_fputs(fp, "\n");
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double val = mp->elements[i][j];
            int int_val = (int)(val + ((val < 0.0) ? -0.5 : 0.5));

            if(!isnand(val))
            {
                ASSERT_IS_NUMBER_DBL(val);
                ASSERT_IS_FINITE_DBL(val);
            }

            if ((ABS_OF(int_val) < 1000) && (IS_EQUAL_DBL(val, (double)int_val)))
            {
                ERE(kjb_fprintf(fp, "%5d    ", int_val));
            }
            else
            {
                ERE(kjb_fprintf(fp,format_str, val));
            }

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
 *                    write_matrix_with_header
 *
 * Writes a matrix to a file row-wise with header
 *
 * This routine outputs a Matrix to a file specified by the input file name row
 * by row with a header. Since matrix files with headers are usually components
 * of more complex data structures, we write the data at full precision.
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

int write_matrix_with_header
(
    const Matrix*    mp,
    const char*      file_name
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

    write_result = fp_write_matrix_with_header(mp, fp);

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
 *                   fp_write_matrix_with_header
 *
 * Writes data row-wise to a FILE prefaced by a matrix header
 *
 * This routine writes data in a matrix to a file. Data is prefaced by a matrix
 * size header indicating the number of rows and columns in the matrix being
 * output. Since matrix files with headers are usually components of more
 * complex data structures, we write the data at full precision.
 *
 * "fp" points to a FILE as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
 *
 * The matrix size header has the form:
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
 * Related:
 *     fp_read_matrix_with_header
 *     fp_write_matrix_size_header
 *     count_data_lines_until_next_header, kjb_print_error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_with_header(const Matrix* mp, FILE* fp)
{
    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(fp_write_matrix_size_header(fp,
                                    (mp == NULL) ? 0: mp->num_rows,
                                    (mp == NULL) ? 0: mp->num_cols));
    ERE(fp_write_matrix_full_precision(mp, fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_matrix_full_precision
 *
 * Writes a matrix to a file specified by name
 *
 * This routine outputs a Matrix to a file specified by the input file
 * name with lots of digits.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the matrix elements are written in fixed format
 * according to "8.4f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     write_matrix, fp_write_matrix_full_precision, read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_full_precision(const Matrix* mp, const char* file_name)
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

    write_result = fp_write_matrix_full_precision(mp, fp);

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
 *                    fp_write_matrix_full_precision
 *
 * Write a matrix to a file specified by FILE pointer
 *
 * This routine outputs a Matrix with lots of digits to a file specified by a
 * pointer to a FILE.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "mp" is a pointer to the Matrix to output.  If the matrix is NULL, then this
 * routine is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix_full_precision, fp_write_matrix, fp_read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_full_precision(const Matrix* mp, FILE* fp)
{
    int   i, j;
    char format_str[30];

    if (mp == NULL) return NO_ERROR;

    ERE(kjb_sprintf(format_str, sizeof(format_str), "%%.%de", DBL_DIG));

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, format_str, mp->elements[i][j]));

            if (j < (mp->num_cols-1))
            {
                ERE(kjb_fprintf(fp,"  "));
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
 *                          write_matrix_rows
 *
 * Outputs a matrix in row order to a file specified by name
 *
 * This routine outputs a matrix in row order to a file specified by
 * its file name. All columns of a given row are output before advancing
 * to the next row.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
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
 *     Lindsay Martin
 *
 * Related:
 *     fp_write_matrix_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_rows(const Matrix* mp, const char* file_name)
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

    write_result = fp_write_matrix_rows(mp, fp);

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
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      fp_write_matrix_rows
 *
 * Outputs a matrix in row order to a FILE specified by a pointer
 *
 * This routine outputs a matrix in row order to a file specified by
 * a pointer to a FILE. All columns of a given row are output
 * before advancing to the next row.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the Matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum absolute value in
 * the matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string "%.5e".
 * Otherwise the matrix elements are written in fixed format according to
 * ".5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix_rows, fp_write_matrix_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_rows(const Matrix* mp, FILE* fp)
{
    int         i;
    int         j;
    double      max;
    const char* format_str;


    if (mp == NULL) return NO_ERROR;

    max = max_abs_matrix_element(mp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%.5e\n";
    }
    else
    {
        format_str = "%.5f\n";
    }

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, format_str, mp->elements[i][j]));
        }
        ERE(kjb_fprintf(fp, "\n"));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_write_matrix_rows_full_precision(const Matrix* mp, FILE* fp)
{
    int   i, j;
    char format_str[30];


    ERE(kjb_sprintf(format_str, sizeof(format_str), "%%.%de\n", DBL_DIG));

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp, format_str, mp->elements[i][j]));
        }
        ERE(kjb_fprintf(fp, "\n"));
    }

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* =============================================================================
 *                          write_matrix_cols
 *
 * Outputs a matrix in column order to a file specified by name
 *
 * This routine outputs a matrix in column order to a file specified by
 * its file name. All row entries of a given column are output before
 * advancing to the next column.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum absolute value in
 * the matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string "%10.3e".
 * Otherwise the matrix elements are written in fixed format according to
 * "8.4f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix_rows, fp_write_matrix_cols
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_cols(const Matrix* mp, const char* file_name)
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

    write_result = fp_write_matrix_cols(mp, fp);

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
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      fp_write_matrix_cols
 *
 * Outputs a matrix in column order to a pointer to FILE
 *
 * This routine outputs a matrix in column order to a file specified by a
 * pointer to a FILE. All row entriess of a given column are output before
 * advancing to the next column.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the Matrix hose contents are to be written.  If the
 * matrix is NULL, then this routine is a NOP.
 *
 * The output format depends on the magnitude of the maximum absolute value in
 * the matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string "%.5".
 * Otherwise the matrix elements are written in fixed format according to
 * ".5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix_cols, fp_write_matrix_rows
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_cols
(
   const Matrix* mp   /* Pointer to output FILE . */,
   FILE* fp           /* Pointer to Matrix to output. */
)
{
    int         i;
    int         j;
    double      max;
    const char* format_str;


    if (mp == NULL) return NO_ERROR;

    max = max_abs_matrix_element(mp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%.5e\n";
    }
    else
    {
        format_str = "%.5f\n";
    }

    for (j=0; j<mp->num_cols; j++)
    {
        for (i=0; i<mp->num_rows; i++)
        {
            ERE(kjb_fprintf(fp, format_str, mp->elements[i][j]));
        }

        ERE(kjb_fprintf(fp, "\n"));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_matrix_vector
 *
 * Writes a matrix vector to a file specified by name
 *
 * This routine outputs a matrix vector to a file specified by the file name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 * If it is NULL, then this routine will write a matrix vector with the number
 * of matrices set to zero, which will read in as NULL with the analogous read
 * routine.
 *
 * The output begins with a line specifying the number of matrices as follows:
 * |         #! num_matrices=999
 * Each matrix is proceeded by line indicating its size using the following
 * format:
 * |         #! rows=10 cols=10
 * A NULL matrix will be indicated by
 * |         #! rows=0 cols=0
 *
 * The output format of the numbers depends on the magnitude of the maximum
 * value in the matrix. If the maximum is < 0.01 or greater than 10000, the data
 * is written in exponential format according to the format string "%10.3e".
 * Otherwise the matrix elements are written in fixed format according to
 * "8.4f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_vector(const Matrix_vector* mvp, const char* file_name)
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

    write_result = fp_write_matrix_vector(mvp, fp);

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
 *                           fp_write_matrix_vector
 *
 * Writes a matrix vector to a stream
 *
 * This routine outputs a matrix vector to a stream pointed to by fp.
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 * If it is NULL, then this routine will write a matrix vector with the number
 * of matrices set to zero, which will read in as NULL with the analogous read
 * routine.
 *
 * The output begins with a line specifying the number of matrices as follows:
 * |         #! num_matrices=999
 * Each matrix is proceeded by line indicating its size using the following
 * format:
 * |         #! rows=10 cols=10
 * A NULL matrix will be indicated by
 * |         #! rows=0 cols=0
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the matrix elements are written in fixed format
 * according to "8.4f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_vector(const Matrix_vector* mvp, FILE* fp)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        num_matrices     = mvp->length;
    int        i;


    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(kjb_fprintf(fp, "%c%c num_matrices=%d\n\n",
                    kjb_comment_char, kjb_header_char, (mvp == NULL) ? 0: mvp->length));

    for (i=0; i<num_matrices; i++)
    {
        Matrix* mp = mvp->elements[ i ];

        if (mp == NULL)
        {
            ERE(kjb_fprintf(fp, "%c%c rows=0 cols=0\n\n",
                            kjb_comment_char, kjb_header_char));
        }
        else
        {
            ERE(kjb_fprintf(fp, "%c%c rows=%d cols=%d\n\n",
                            kjb_comment_char, kjb_header_char,
                            mp->num_rows, mp->num_cols));
            ERE(fp_write_matrix(mp, fp));
            ERE(kjb_fputs(fp, "\n\n"));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       write_matrix_vector_full_precision
 *
 * Writes a matrix vector to a file specified by name in full precision
 *
 * This routine outputs a matrix vector to a file specified by the file name
 * with full precission. Otherwise this routine is very similar to
 * write_matrix_vector(3).
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 * If it is NULL, then this routine will write a matrix vector with the number
 * of matrices set to zero, which will read in as NULL with the analogous read
 * routine.
 *
 * The output begins with a line specifying the number of matrices as follows:
 * |         #! num_matrices=999
 * Each matrix is proceeded by line indicating its size using the following
 * format:
 * |         #! rows=10 cols=10
 * A NULL matrix will be indicated by
 * |         #! rows=0 cols=0
 *
 * The output format of the numbers is sufficient to output double precision
 * numbers without loss of precision.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_matrix_vector_full_precision
(
    const Matrix_vector* mvp,
    const char*          file_name
)
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

    write_result = fp_write_matrix_vector_full_precision(mvp, fp);

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
 *                       fp_write_matrix_vector_full_precision
 *
 * Writes a matrix vector to a stream with full precision
 *
 * This routine outputs a matrix vector to a stream pointed to by fp
 * with full precission. Otherwise this routine is very similar to
 * fp_write_matrix_vector(3).
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 * If it is NULL, then this routine will write a matrix vector with the number
 * of matrices set to zero, which will read in as NULL with the analogous read
 * routine.
 *
 * The output begins with a line specifying the number of matrices as follows:
 * |         #! num_matrices=999
 * Each matrix is proceeded by line indicating its size using the following
 * format:
 * |         #! rows=10 cols=10
 * A NULL matrix will be indicated by
 * |         #! rows=0 cols=0
 *
 * The output format of the numbers is sufficient to output double precision
 * numbers without loss of precision.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_vector_full_precision
(
    const Matrix_vector* mvp,
    FILE*                fp
)
{
    int num_matrices = mvp->length;
    int i;


    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(kjb_fprintf(fp, "%c%c num_matrices=%d\n\n",
                    kjb_comment_char, kjb_header_char, (mvp == NULL) ? 0: mvp->length));

    for (i=0; i<num_matrices; i++)
    {
        Matrix* mp = mvp->elements[ i ];

        if (mp == NULL)
        {
            ERE(kjb_fprintf(fp, "%c%c rows=0 cols=0\n\n",
                            kjb_comment_char, kjb_header_char));
        }
        else
        {
            ERE(kjb_fprintf(fp, "%c%c rows=%d cols=%d\n\n",
                            kjb_comment_char, kjb_header_char,
                            mp->num_rows, mp->num_cols));
            ERE(fp_write_matrix_full_precision(mp, fp));
            ERE(kjb_fputs(fp, "\n\n"));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fp_write_matrix_with_title
 *
 * Debugging routine that outputs a matrix
 *
 * This routine outputs the matrix pointed to by "mp" to the file pointer
 * indicated by "fp", and labels the output with a user defined "title".
 *
 * Use as a quick-and-dirty debugging aid only.
 *
 * Returns:
 *     NO_ERROR on success, ERROR otherwise.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     write_matrix
 *
 * Index:  I/O, matrices, debugging, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_with_title
(
    const Matrix* mp,
    FILE*         fp,
    const char*   title
)
{
    int    i, j;
    double max;
    const char* format_str;


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

    max = max_abs_matrix_element(mp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%14.7e";
    }
    else
    {
        format_str = "%12.8f";
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
            ERE(kjb_fprintf(fp,format_str, mp->elements[i][j]));

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
 *                     fp_write_matrix_full_precision_with_title
 *
 * Debugging routine that outputs a matrix
 *
 * This routine outputs the matrix pointed to by "mp" to the file pointer
 * indicated by "fp", and labels the output with a user defined "title". The
 * output is full precision scientific notation.
 *
 * Use as a quick-and-dirty debugging aid only.
 *
 * Returns:
 *     NO_ERROR on success, ERROR otherwise.
 *
 * Documentor:
 *     Lindsay Martin, Kobus Barnard
 *
 * Index:  I/O, matrices, debugging, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_matrix_full_precision_with_title
(
    const Matrix* mp,
    FILE*         fp,
    const char*   title
)
{
    int   i, j;
    char format_str[30];


    ERE(kjb_sprintf(format_str, sizeof(format_str), "%%.%de", DBL_DIG));

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
        ERE(kjb_fprintf(fp, "\n"));
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
        ERE(kjb_fprintf(fp, "%c", kjb_comment_char));
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
    }

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(fp,format_str, mp->elements[i][j]));

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
 *                           write_raw_matrix
 *
 * Writes a matrix to a file specified by name as raw data
 *
 * This routine outputs a Matrix to a the specified file as raw data.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mp" is a pointer to the Matrix whose contents are to be written. If mp is
 * NULL, then the number of rows and columns are both set to 0, indicating to
 * the corresponding read routine that the matrix should be NULL. Otherwise, the
 * numer of rows and columns are both assumed to be positive. Note that there is
 * no way to to reliably read/write non-NULL matrices with either 0 rows or 0
 * cols (even though they can exist in memory).
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     fp_write_matrix, read_matrix
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_matrix(const Matrix* mp, const char* file_name)
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
        write_result = fp_write_raw_matrix(mp, fp);
    }

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
 *                           fp_write_raw_matrix
 *
 * Write a matrix to a file specified by FILE pointer as raw data.
 *
 * This routine outputs a Matrix to a file specified by a pointer to a FILE ias
 * raw data.
 *
 * "fp" is a pointer to the the FILE as returned by "kjb_fopen".
 *
 * "mp" is a pointer to the Matrix to output.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_matrix(const Matrix* mp, FILE* fp)
{
    int i;
    int num_rows   = 0;
    int num_cols   = 0;
    int byte_order = 1;
    char head_str[ KJB_DATA_HEAD_SIZE ];
    int  pad = 0;

    if (mp != NULL)
    {
        num_rows = mp->num_rows;
        num_cols = mp->num_cols;
    }

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_MATRIX_STRING);

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
                         (sizeof(double) * num_rows * num_cols),
                         NULL));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_indexed_matrix_vector
(
    Matrix_vector** mvpp,
    const char*     index_file_name,
    const char*     file_name
)
{
    FILE*       index_fp = NULL;
    FILE*       fp = NULL;
    int         result = NO_ERROR;
    Matrix*     mp       = NULL;
    Int_vector* index_vp = NULL;
    int num_cols = NOT_SET, i, num_matrices = 0;
    int cur_row = 0;


    NRE(fp = kjb_fopen(file_name, "r"));

    if ((index_file_name != NULL) && (*index_file_name != '\0'))
    {
        index_fp = kjb_fopen(index_file_name, "r");

        if (index_fp ==NULL)
        {
            kjb_fclose(fp);
            return ERROR;
        }

        result = fp_read_int_vector(&index_vp, index_fp);
    }

    if (result != ERROR)
    {
        result = fp_read_matrix(&mp, fp);
    }

    if (result != ERROR)
    {
        if (index_vp == NULL)
        {
            num_matrices = mp->num_rows;
        }
        else
        {
            num_matrices = index_vp->length;
        }

        num_cols = mp->num_cols;
        result = get_target_matrix_vector(mvpp, num_matrices);
    }

    for (i = 0; i < num_matrices; i++)
    {
        int num_rows;

        if (result == ERROR) break;

        ASSERT(KJB_IS_SET(num_cols));

        if (index_vp == NULL)
        {
            num_rows = 1;
        }
        else
        {
            num_rows = index_vp->elements[ i ];
        }

        if (cur_row + num_rows > mp->num_rows)
        {
            set_error("Not enough rows in %s as determined from %s.",
                      file_name, index_file_name);
            result = ERROR;
            break;
        }

        result = copy_matrix_block(&((*mvpp)->elements[ i ]), mp,
                                   cur_row, 0, num_rows, num_cols);

        cur_row += num_rows;
    }

    free_matrix(mp);
    free_int_vector(index_vp);

    (void)kjb_fclose(fp);        /* Ignore return: Only reading. */
    (void)kjb_fclose(index_fp);  /* Ignore return: Only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             read_matrix_vector
 *
 * Reads a matrix vector from file
 *
 * This routine reads a matrix vector from a file.  If the file_name is NULL or
 * a null string, then stdin is assumed. If this is the case, and if the source
 * is not a file (i.e. a pipe), then this routine will fail.
 *
 * Several read strategies are tried until one succeeds. The first strategy is
 * to assume that the matrix is in raw, binary format. If that fails, then it is
 * assumed that the file has a header file (see
 * fp_read_matrix_vector_with_header(3)).
 *
 * The matrix *result_mvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_matrix_vector(Matrix_vector** mvpp, const char* file_name)
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

    result = fp_read_matrix_vector(mvpp, fp);

    /*
    //  The file pointer based read routines can return EOF because they are
    //  used as building blocks. But for name based read routines, we expect at
    //  least one data item.
    */
    if ((result == EOF) || (result == NOT_FOUND))
    {
        set_error("Unable to read a matrix vector from %F.", fp);
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
 *                           fp_read_matrix_vector
 *
 * Reads a matrix vector from a stream
 *
 * This routine reads a matrix vector from the stream pointed to by fp. If the
 * stream is not a file (for example a pipe), then this routine will fail.
 *
 * Several read strategies are tried until one succeeds. The first strategy is
 * to assume that the matrix is in raw, binary format (see
 * fp_read_raw_matrix_vector(3)). If that fails, then it is assumed that the
 * file has a header file (see fp_read_matrix_vector_with_header(3)).
 *
 * The matrix *result_mvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_vector(Matrix_vector** mvpp, FILE* fp)
{
    int       result        = NOT_FOUND;
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
        add_error("Read of matrix vector aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_matrix_vector(mvpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_matrix_vector_with_headers(mvpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_raw_matrix_vector
 *
 * Reads a raw matrix vector from a stream
 *
 * This routine reads a raw (binary format) matrix vector from the stream
 * pointed to by fp.  string, then stdin is assumed. If this is the case, and if
 * the source is not a file (i.e. a pipe), then this routine will fail.
 *
 * The matrix *result_mvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_matrix_vector(Matrix_vector** mvpp, FILE* fp)
{
    int     num_matrices;
    int     i;
    long    bytes_used_so_far;
    int     byte_order;
    char    head_str[ KJB_DATA_HEAD_SIZE ];
    int     pad;
    off_t   num_bytes;
    int     result = NO_ERROR;


    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (num_bytes < KJB_DATA_HEAD_SIZE) return NOT_FOUND;

    ERE(kjb_fread_exact(fp, head_str, sizeof(head_str)));
    head_str[ sizeof(head_str) - 1 ] = '\0';

    if ( ! STRCMP_EQ(head_str, KJB_RAW_MATRIX_VECTOR_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, num_matrices));
    ERE(FIELD_READ(fp, pad));

    if (num_matrices < 0)
    {
        set_error("Invalid data in matrix vector file %F.");
        add_error("The number of matrices is negative ( %d ).", num_matrices);
        return ERROR;
    }

    ERE(get_target_matrix_vector(mvpp, num_matrices));

    for (i = 0; i < num_matrices; i++)
    {
        if (fp_read_raw_matrix(&((*mvpp)->elements[ i ]), fp) == ERROR)
        {
            add_error("Failed reading matrix %d.\n", i + 1);
            result = ERROR;
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        fp_read_matrix_vector_with_headers
 *
 * Reads an ascii matrix vector with headers from a stream
 *
 * This routine reads an ascii matrix vector from the stream pointed to by fp.
 * string, then stdin is assumed. If this is the case, and if the source is not
 * a file (i.e. a pipe), then this routine will fail.
 *
 * The matrix *result_mvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_matrix_vector_with_headers(Matrix_vector** mvpp, FILE* fp)
{
    IMPORT int kjb_comment_char;
    Matrix_vector* mvp;
    int            num_matrices = NOT_SET;
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 5000 ];
    char*          line_pos;
    int            result          = ERROR;


    while (num_matrices == NOT_SET)
    {
        int read_result;

        ERE(read_result = BUFF_FGET_LINE(fp, line));

        if (read_result == EOF)
        {
            set_error("Unexpected EOF reached reading matrix array from %F.\n",
                      fp);
            return ERROR;
        }

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
                    free_options(num_options, option_list, value_list);
                    return ERROR;
                }

                for (i=0; i<num_options; i++)
                {
                    /* Look for flag indicating the number of matrix rows */
                    if (IC_STRCMP_EQ(*option_list_pos,"num_matrices"))
                    {
                        if (ss1pi(*value_list_pos, &num_matrices) == ERROR)
                        {
                            free_options(num_options, option_list, value_list);
                            return ERROR;
                        }
                    }

                    value_list_pos++;
                    option_list_pos++;
                } /* for (i=0; i<num_options; i++) */

                free_options(num_options, option_list, value_list);

            } /* if (*line_pos == '!') */
        } /* else if (*line_pos == kjb_comment_char) */
        else
        {
            set_error("Cannot find header with num_matrices in %F.", fp);
            return ERROR;
        }
    } /* end while ( num_matrices == NOT_SET )  */

    if (num_matrices == 0)
    {
        UNTESTED_CODE();

        free_matrix_vector(*mvpp);
        *mvpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix_vector(mvpp, num_matrices));
    mvp = *mvpp;

    for (i=0; i<num_matrices; i++)
    {
        result = fp_read_matrix(&(mvp->elements[ i ]), fp);

        if (result == EOF)
        {
            set_error("Unexpected end of file reading matrix vector from %F.",
                      fp);
            result = ERROR;
        }

        if (result == ERROR)
        {
            insert_error("Problem reading matrix %d.", i + 1);
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_raw_matrix_vector
 *
 * Writes a matrix vector to a file specified by name in raw (binary) format
 *
 * This routine outputs a matrix vector to a file specified by the file name in
 * raw (binary) format.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the matrix contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_matrix_vector(const Matrix_vector* mvp, const char* file_name)
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
        write_result = fp_write_raw_matrix_vector(mvp, fp);
    }

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
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_write_raw_matrix_vector
 *
 * Writes a matrix vector to a stream in raw (binary) format
 *
 * This routine outputs a matrix vector to a stream pointed to by fp in raw
 * (binary) format.
 *
 * "mvp" is a pointer to the Matrix_vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_matrix_vector(const Matrix_vector* mvp, FILE* fp)
{
    int num_matrices = mvp->length;
    int i;

    ERE(fp_write_raw_matrix_vector_header(num_matrices, fp));

    for (i=0; i<num_matrices; i++)
    {
        Matrix* mp = mvp->elements[ i ];

        ERE(fp_write_raw_matrix(mp, fp));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                      fp_write_raw_matrix_vector_header
 *
 * Writes the header for a raw matrix vector to a stream
 *
 * This routine outputs the header for a raw matrix vector to a stream pointed
 * to by fp. This routine exists so that matrix vectors can be written
 * sequentially, one matrix at a time, so that the entire matrix vector does not
 * need to be in memory).
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, matrices, matrix I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_matrix_vector_header(int num_matrices, FILE* fp)
{
    int byte_order = 1;
    char head_str[ KJB_DATA_HEAD_SIZE ];
    int  pad = 0;
    int  i;

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_MATRIX_VECTOR_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, num_matrices));
    ERE(FIELD_WRITE(fp, pad));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_matrix
(
    const char*   output_dir,
    const char*   file_name,
    const Matrix* mp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_matrix_full_precision(mp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_matrix
(
    const char*   output_dir,
    const char*   file_name,
    const Matrix* mp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_raw_matrix(mp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_matrix_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Matrix_vector* mvp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mvp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_matrix_vector_full_precision(mvp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_matrix_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Matrix_vector* mvp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (mvp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_raw_matrix_vector(mvp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Vector_vector* vvp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (vvp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_vector_vector_full_precision(vvp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Vector_vector* vvp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (vvp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_raw_vector_vector(vvp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

