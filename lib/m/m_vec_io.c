
/* $Id: m_vec_io.c 21522 2017-07-22 15:14:27Z kobus $ */

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
#include "m/m_vec_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define KJB_DATA_HEAD_SIZE  64
#define KJB_RAW_VECTOR_STRING  "kjb raw vector\n\n\f\n"
#define KJB_RAW_VECTOR_VECTOR_STRING  "kjb raw vector vector\n\n\f\n"

/* -------------------------------------------------------------------------- */

static int fp_read_ascii_vector_2(Vector** result_vpp, FILE* fp, int length);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                        read_vector_from_config_file
 *
 * Reads a vector from a config file
 *
 * This routine reads a vector from a configuration file, checking several
 * possible locations for the file. If the parameter "env_var" is not NULL, then
 * it first tries using this string as the configuration file name. Next it
 * looks for "file_name" in the current directory, then the user's home
 * directory, and, depending on how the library was built, then the "shared"
 * home directory and/or the programmer's home directory. If "directory" is not
 * NULL, then it is used as a sub-directory in all paths starting from a home
 * directory (i.e, all paths except the current directory).
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
 * The vector *result_vpp is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_vpp is freed and set to NULL.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_vector_from_config_file
(
    Vector** result_vpp /* Double pointer to Vector result.  */,
    const char* env_var/* Config file name from environment. */,
    const char* directory /* Directory to scan for config file. */,
    const char* file_name  /* Name of config file to search for. */,
    const char* message_name  /* File name for error messages.      */,
    char* config_file_name /* Name of actual config file found.  */,
    size_t config_file_name_size /* Size of actual config file name.   */
)
{
    char    temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int     result;


    if (is_no_value_word(file_name))
    {
        free_vector(*result_vpp);
        *result_vpp = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    result = read_vector(result_vpp, temp_config_file_name);

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
 *                       read_vector
 *
 * Reads data from a file specified by name into a new Vector
 *
 * This routine reads data contained in a file specified by its name into a
 * new vector that is created dynamically. The length of the new vector is
 * determined by the number of data elements contained in the input file.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, input is expected from STDIN instead of a file.
 *
 * Returns:
 *     A valid pointer to the new vector on success, or
 *     NULL on failure, with "kjb_error" set to a descriptive message.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_vector(Vector** result_vpp, const char* file_name)
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

    result = fp_read_vector(result_vpp, fp);

    /*
    //  The file pointer based read routines can return EOF because they are
    //  used as building blocks. But for name based read routines, we expect at
    //  least one vector.
    */
    if (result == EOF)
    {
        set_error("Unable to read a vector from %F.", fp);
        result = ERROR;
    }

    if (file_name != NULL)
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_vector
 *
 * Reads data from a file into a vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into a vector that is created or resized if needed. The length of the result
 * vector is determined by the number of data elements contained in the input
 * file.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_vector(Vector** result_vpp, FILE* fp)
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
        add_error("Read of vector aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_vector(result_vpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_vector_with_header(result_vpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_ascii_vector(result_vpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_raw_vector
 *
 * Reads raw data from a file into a vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into a vector that is created dynamically if needed. The length of the result
 * vector is determined by the number of data elements contained in the input
 * file.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success, NOT_FOUND if the file does not appear to contain a
 *     valid KJB library format raw vector, and ERROR on failure, with an error
 *     message being set.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_vector(Vector** result_vpp, FILE* fp)
{
    int     length;
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

    if ( ! STRCMP_EQ(head_str, KJB_RAW_VECTOR_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, length));
    ERE(FIELD_READ(fp, pad));

    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (length == 0)
    {
        /* Special case: The matrix is mean to be null. */
        free_vector(*result_vpp);
        *result_vpp = NULL;
    }
    else if (length < 0)
    {
        set_error("Invalid data in vector file %F.");
        add_error("The length is negative ( %d ).", length);
        return ERROR;
    }
    else if (num_bytes < (off_t)(length * sizeof(double)))
    {
        set_error("Invalid data in vector file %F.");
        add_error("The number of bytes should be at least %d.",
                  bytes_used_so_far + length * sizeof(double));
        return ERROR;
    }
    else
    {
        ERE(get_target_vector(result_vpp, length));

        if (length > 0)
        {
            ERE(kjb_fread_exact(fp, (*result_vpp)->elements,
                                length * sizeof(double)));
        }
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_vector_with_header
 *
 * Reads ascii data from an input stream into a vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into an integer vector. The length of the new vector is determined by the
 * number of data elements contained in the input file.  The vector *vpp is
 * created or resized as necessary.
 *
 * "vpp" is a pointer to a vector pointer.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Related:
 *     read_vector, fp_read_vector
 *
 * Index: I/O,  vectors,  vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_vector_with_header(Vector** vpp, FILE* fp)
{
    int     num_elements;
    int     result = fp_read_vector_length_header(fp, &num_elements);


    if (result == ERROR)
    {
        set_error("Error reading vector length header from %F.", fp);
        return ERROR;
    }
    else if (result == NOT_FOUND)
    {
        return NOT_FOUND;
    }
    else if (num_elements == 0)
    {
        free_vector(*vpp);
        *vpp = NULL;
        return NO_ERROR;
    }
    else
    {
        return fp_read_ascii_vector_2(vpp, fp, num_elements);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         fp_read_ascii_vector
 *
 * Reads ascii data from a file into a vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into vector that is created dynamically if needed. The length of the result
 * vector is determined by the number of data elements contained in the input
 * file.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_ascii_vector(Vector** result_vpp, FILE* fp)
{
    int     num_elements;
    char    line[ LARGE_IO_BUFF_SIZE ];
    long    read_res = NO_ERROR;


    ERE(num_elements = gen_count_tokens_in_file(fp, " ,\t"));

    if (num_elements == 0)
    {
        set_error("Expecting at least one number in %F.", fp);
        return ERROR;
    }

    ERE(fp_read_ascii_vector_2(result_vpp, fp, num_elements));

    /*
     * Since we counted out the number of tokens that we expected, and stopped
     * reading when we got all of them, there can be junk left that we have to
     * swallow.
    */
    while (TRUE)
    {
        ERE(read_res = BUFF_GET_REAL_LINE(fp, line));

        if (read_res < 0) break;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fp_read_ascii_vector_2(Vector** result_vpp, FILE* fp, int length)
{


    ERE(get_target_vector(result_vpp, length));

    if (length == 0) return NO_ERROR;

    return fp_ow_read_vector(*result_vpp, fp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               fp_ow_read_vector
 *
 * Reads data from an input stream into a Vector
 *
 * This routine reads data into a vector from a file specified by a file
 * pointer.  The vector must already exist, and the file must already be opened.
 *
 * "fp" points to a file structure associated with the input stream as returned
 * by "kjb_fopen".
 *
 * "vp" is a pointer to a vector. This vector must already exist, and
 * have length "vp->length".
 *
 * The routine attempts to read "vp->length" data elements from the input file.
 * If no elements can be read from the file, the routine returns EOF. If the
 * number of elements read is not equal to the vector length, an error is
 * indicated.
 *
 * Returns:
 *     NO_ERROR on success,
 *     EOF if no more data elements are in the file, or
 *     ERROR on failure.
 *
 * Doccumentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_ow_read_vector(Vector* vp, FILE* fp)
{
    char  line[ LARGE_IO_BUFF_SIZE ];
    char  element_buff[ 200 ];
    char* line_pos;
    int   i;
    long  read_res;
    int   scan_res;


    i = 0;

    while (i<vp->length)
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
                set_error("Not enough data read from %F for vector of size %d.",
                          fp, vp->length);
                return ERROR;
            }
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,\t"))
        {
            if (i < vp->length)
            {
                scan_res = ss1snd(element_buff, (vp->elements)+i);

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

int fp_write_row_vector_with_title
(
    const Vector* vp,
    FILE*         fp,
    const char*   title
)
{
    int i;
    const char* format_str;
#ifdef FOR_AUTO_FORMAT
    double      max;
#endif


    if (vp == NULL)
    {
         if (title != NULL)
         {
             ERE(kjb_fprintf(fp, title));
             ERE(kjb_fprintf(fp, ": "));
         }

         ERE(kjb_fprintf(fp, "NULL\n"));

         return NO_ERROR;
     }

#ifdef FOR_AUTO_FORMAT
    max = max_abs_vector_element(vp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%10.3e";
    }
    else
    {
        format_str = "%9.5f";
    }
#endif

    format_str = "%e";

    if (title != NULL)
    {
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
    }

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));

        if (i < (vp->length-1))
        {
            ERE(kjb_fprintf(fp,"  "));
        }
        else
        {
            ERE(kjb_fprintf(fp,"\n"));
        }
    }
    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_write_col_vector_with_title
(
    const Vector* vp,
    FILE*         fp,
    const char*   title
)
{
    IMPORT int kjb_comment_char;
    int   i;
#ifdef FOR_AUTO_FORMAT
    double  max;
#endif
    const char* format_str;


    if (title != NULL)
    {
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
        ERE(kjb_fprintf(fp, "%c ", kjb_comment_char));
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
        ERE(kjb_fprintf(fp, "%c\n", kjb_comment_char));
    }

    if (vp == NULL)
    {
        ERE(kjb_fprintf(fp, "NULL\n"));
        return NO_ERROR;
    }

#ifdef FOR_AUTO_FORMAT
    max = max_abs_vector_element(vp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%10.3e\n";
    }
    else
    {
        format_str = "%9.5f\n";
    }
#endif

    format_str = "%e\n";

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));
    }
    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_row_vector
 *
 * Writes a vector as a row to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified
 * by its name. The vector is output as a row of data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 * If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_row_vector(const Vector* vp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_row_vector(vp, fp);

    if (file_name != NULL)
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
 *                          fp_write_row_vector
 *
 * Writes a vector as a row to an output stream
 *
 * This routine writes data contained in a vector to a file specified
 * by a pointer to FILE. The vector is output as a row of data.
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the vector whose contents are to be written.
 *  If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_row_vector(const Vector* vp, FILE* fp)
{
    int         i;
    const char* format_str;
    double      max;

    if (vp == NULL) return NO_ERROR;

    max = max_abs_vector_element(vp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%10.3e";
    }
    else
    {
        format_str = "%9.5f";
    }

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));

        if (i < (vp->length-1))
        {
            ERE(kjb_fprintf(fp,"  "));
        }
        else
        {
            ERE(kjb_fprintf(fp,"\n"));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                          write_indexed_vector
 *
 * Writes an Indexed vector to a file specified by name
 *
 * This routine writes data contained in an indexed vector to a file specified
 * by its name. The indexed vector is output as two columns of data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "ivp" points to the indexed vector whose contents are to be written.  If it
 * is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_indexed_vector(const Indexed_vector* ivp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) 
        {
            return NO_ERROR;
        }

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_indexed_vector(ivp, fp);

    if (file_name != NULL)
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
 *                          fp_write_indexed_vector
 *
 * Writes an indexed vector to an output stream
 *
 * This routine writes data contained in an indexed vector to a file specified
 * by a pointer to FILE. The indexed vector is output as two columns of data.
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "ivp" points to the vector whose contents are to be written.
 *  If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_indexed_vector(const Indexed_vector* ivp, FILE* fp)
{
    int         i;
    double      max;
    const char* format_str;

    if (ivp == NULL) return NO_ERROR;

    if (ivp->length == 0) return NO_ERROR;

    max = ivp->elements[0].element;

    for (i=1; i<ivp->length; i++)
    {
        if (ivp->elements[i].element > max)
        {
            max = ivp->elements[i].element;
        }
    }

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%5d   %10.3e\n";
    }
    else
    {
        format_str = "%5d   %9.5f\n";
    }

    for (i=0; i<ivp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, ivp->elements[i].index, ivp->elements[i].element));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_col_vector
 *
 * Writes a vector as a col to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified
 * by its name. The vector is output as a column of data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 *  If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_col_vector(const Vector* vp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) 
        {
            return NO_ERROR;
        }

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_col_vector(vp, fp);

    if (file_name != NULL)
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
 *                          fp_write_col_vector
 *
 * Writes a vector as a column to an output stream
 *
 * This routine writes data contained in a vector to a file specified
 * by a pointer to FILE. The vector is output as a column of data.
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the vector whose contents are to be written.
 *  If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_col_vector(const Vector* vp, FILE* fp)
{
    int         i;
    double      max;
    const char* format_str;

    if (vp == NULL) return NO_ERROR;

    max = max_abs_vector_element(vp);

    if ((max < 0.01) || (max > 100000.0 ))
    {
        format_str = "%10.3e\n";
    }
    else
    {
        format_str = "%9.5f\n";
    }

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_col_vector_with_header
 *
 * Writes a vector as a column to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified by its
 * name. The vector is output as a column of data with a header. Since vector
 * files with headers are usually components of more complex data structures, we
 * write the data at full precision.
 *
 * The vector length header has the format:
 * |    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is acutally
 * the header char, also user settable).
 *
 * "file_name" points to a character array specifying the name of the file to
 * read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 * If it is NULL, then a vector of length zero is written which will be read
 * back as NULL by the corresponding read routine.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_col_vector_with_header(const Vector* vp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    UNTESTED_CODE();

    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_col_vector_with_header(vp, fp);

    if (file_name != NULL)
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
 *                          fp_write_col_vector_with_header
 *
 * Writes a vector as a column to an output stream with a header
 *
 * This routine writes data contained in a vector to a file specified by a
 * pointer to FILE. The vector is output as a column of data. Since vector files
 * with headers are usually components of more complex data structures, we write
 * the data at full precision.
 *
 * The vector length header has the format:
 * |    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is acutally
 * the header char, also user settable).
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_col_vector_with_header(const Vector* vp, FILE* fp)
{

    UNTESTED_CODE();

    ERE(fp_write_vector_length_header(fp, (vp == NULL) ? 0 :  vp->length));
    ERE(fp_write_col_vector_full_precision(vp, fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_raw_vector
 *
 * Writes a vector to a file specified by name as raw data
 *
 * This routine writes data contained in a vector to a file specified by
 * its name. The vector is output as raw data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_vector(const Vector* vp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    UNTESTED_CODE();

    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_raw_vector(vp, fp);

    if (file_name != NULL)
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
 *                          fp_write_raw_vector
 *
 * Writes a vector to an output stream as raw data
 *
 * This routine writes a vectorto a file specified by a pointer to FILE. The
 * vector is output as raw data.
 *
 * "fp" points to a FILE associated with the output stream.  "vp" points
 * to the vector to be written.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_vector(const Vector* vp, FILE* fp)
{
    int i;
    int         byte_order = 1;
    char        head_str[ KJB_DATA_HEAD_SIZE ];
    int         pad = 0;
    int         length = 0;


    if (vp != NULL)
    {
        length = vp->length;
    }

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_VECTOR_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, length));
    ERE(FIELD_WRITE(fp, pad));

    if (length > 0)
    {
        ERE(kjb_fwrite_2(fp, vp->elements, sizeof(double) * length, NULL));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     write_row_vector_full_precision
 *
 * Writes a vector as a row to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified
 * by its name. The vector is output as a row of data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_row_vector_full_precision
(
    const Vector* vp,
    const char*   file_name
)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_row_vector_full_precision(vp, fp);

    if (file_name != NULL)
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
 *                    fp_write_row_vector_full_precision
 *
 * Writes a vector as a row to an output stream
 *
 * This routine writes data contained in a vector to a file specified
 * by a pointer to FILE. The vector is output as a row of data.
 *
 * "fp" points to a FILE associated with the output stream.
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_row_vector_full_precision(const Vector* vp, FILE* fp)
{
    int  i;
    char format_str[30];


    ERE(kjb_sprintf(format_str, sizeof(format_str), "%%.%de", DBL_DIG));

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));

        if (i < (vp->length-1))
        {
            ERE(kjb_fprintf(fp,"  "));
        }
        else
        {
            ERE(kjb_fprintf(fp,"\n"));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                     write_col_vector_full_precision
 *
 * Writes a vector as a col to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified
 * by its name. The vector is output as a column of data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_col_vector_full_precision
(
    const Vector* vp,
    const char*   file_name
)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_col_vector_full_precision(vp, fp);

    if (file_name != NULL)
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
 *                          fp_write_col_vector_full_precision
 *
 * Writes a vector as a column to an output stream
 *
 * This routine writes data contained in a vector to a file specified
 * by a pointer to FILE. The vector is output as a column of data.
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the vector whose contents are to be written.
 *  If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_col_vector_full_precision(const Vector* vp, FILE* fp)
{
    int   i;
    char format_str[30];


    if (vp == NULL) return NO_ERROR;

    ERE(kjb_sprintf(format_str, sizeof(format_str), "%%.%de\n", DBL_DIG));

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp,format_str, vp->elements[i]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Vector_vector IO
*/

/* =============================================================================
 *                        read_vector_vector_from_config_file
 *
 * Reads a vector vector from a config file
 *
 * This routine reads a vector vector from a configuration file, checking
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
 * The vector vector *result_vvpp is created or resized as necessary.
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *result_vvpp is freed and set to NULL.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_vector_vector_from_config_file
(
    Vector_vector** result_vvpp  ,
    const char* env_var /* Config file name from envirnonment. */,
    const char* directory /* Directory to scan for config file.  */,
    const char* file_name /* Name of config file to search for.  */,
    const char* message_name /* File name for error messages.       */,
    char* config_file_name  /* Name of actual config file found.   */,
    size_t config_file_name_size  /* Size of actual config file name.*/
)
{
    char    temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    int     result;


    if (is_no_value_word(file_name))
    {
        free_vector_vector(*result_vvpp);
        *result_vvpp = NULL;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    ERE(get_config_file(env_var, directory, file_name, message_name,
                        temp_config_file_name, sizeof(temp_config_file_name)));

    result = read_vector_vector(result_vvpp, temp_config_file_name);

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
 *                             read_vector_vector
 *
 * Reads a vector vector from file
 *
 * This routine reads a vector vector from a file.  If the file_name is NULL or
 * a null string, then stdin is assumed. If this is the case, and if the source
 * is not a file (i.e. a pipe), then this routine will fail.
 *
 * In theory, and in parallel with other routines such as read_matrix, several
 * read strategies are tried until one succeeds.  However, only one strategy is
 * currently implemented. Specifically we assume that
 * the file is a formatted ascii file, and the vector vector dimensions are
 * deduced from the number of rows and columns (see also
 * fp_read_formatted_vector_vector(3)).
 *
 * The vector vector *result_vvpp is created or resized as necessary.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL or the
 * first character is null, input is retrieved from STDIN. If STDIN is
 * not a redirected file, an error message will be generated.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_vector_vector
(
    Vector_vector** result_vvpp,
    const char*     file_name
)
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

    result = fp_read_vector_vector(result_vvpp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fp_read_vector_vector
 *
 * Reads a vector vector from data in a formated file
 *
 * This routine reads a vector vector from a file.  If the file_name is NULL or
 * a null string, then stdin is assumed. If this is the case, and if the ource
 * is not a file (i.e. a pipe), then this routine will fail.  If there are
 * multiple matrices in the file separated with soft EOFs, then the reading
 * continues to the next soft (or hard) EOF.
 *
 * In theory, and in parallel with other routines such as read_matrix, several
 * read strategies are tried until one succeeds.  However, only one strategy is
 * currently implemented. Specifically we assume that
 * the file is a formatted ascii file, and the vector vector dimensions are
 * deduced from the number of rows and columns. Each row is assumed to be a
 * vector.
 *
 * The vector vector *result_vvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_vector_vector(Vector_vector** result_vvpp, FILE* fp)
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
        add_error("Read of vector vector aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_vector_vector(result_vvpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_formatted_vector_vector(result_vvpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_raw_vector_vector
 *
 * Reads a raw vector vector from a stream
 *
 * This routine reads a raw (binary format) vector vector from the stream
 * pointed to by fp.  string, then stdin is assumed. If this is the case, and if
 * the source is not a file (i.e. a pipe), then this routine will fail.
 *
 * The vector vector *result_vvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_vector_vector(Vector_vector** vvpp, FILE* fp)
{
    int     num_vectors;
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

    if ( ! STRCMP_EQ(head_str, KJB_RAW_VECTOR_VECTOR_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, num_vectors));
    ERE(FIELD_READ(fp, pad));

    if (num_vectors < 0)
    {
        set_error("Invalid data in vector vector file %F.");
        add_error("The number of vectors is negative ( %d ).", num_vectors);
        return ERROR;
    }

    ERE(get_target_vector_vector(vvpp, num_vectors));

    for (i = 0; i < num_vectors; i++)
    {
        if (fp_read_raw_vector(&((*vvpp)->elements[ i ]), fp) == ERROR)
        {
            add_error("Failed reading vector %d.\n", i + 1);
            result = ERROR;
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_read_formatted_vector_vector
 *
 * Reads a vector vector from data in a formatted FILE
 *
 * This routine reads a vector vector from a file containing data formatted into
 * fixed rows and columns. The vector vector dimensions are controlled by the
 * formatting of the data in the file. The number of vectors is the number of
 * rows, and the number of elements in a given element is the number of elements
 * in the column.
 *
 * The vector vector *result_vvpp is created or resized as necessary.
 *
 * "fp" points to a FILE specifying the file to read the data from.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_formatted_vector_vector
(
    Vector_vector** result_vvpp,
    FILE*           fp
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Vector_vector*      vvp;
    char                line[ LARGE_IO_BUFF_SIZE ];
    char                num_buff[ 10000 ];
    int                 length;
    int                 num_vectors;
    int                 i, j;
    char*               line_pos;
    int                 scan_res;



    ERE(num_vectors = count_real_lines(fp));

    if ((num_vectors == 0) || (num_vectors == EOF))
    {
        return EOF;
    }

    ERE(get_target_vector_vector(result_vvpp, num_vectors));
    vvp = *result_vvpp;

    for (i=0; i<num_vectors; i++)
    {
        ERE(BUFF_GET_REAL_LINE(fp, line));

        length = 0;
        line_pos = line;

        trim_beg(&line_pos);
        trim_end(line_pos);

        if (STRCMP_EQ(line_pos, "NULL"))
        {
            /* Leave this vector NULL. */
            continue;
        }

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
        {
            length++;
        }

        ERE(get_target_vector(&(vvp->elements[ i ]), length));

        line_pos = line;

        for (j=0; j<length; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
            {
                set_error("Missing data on row %d of %F.", (i+1), fp);
                return ERROR;
            }

            scan_res = ss1snd(num_buff, &(vvp->elements[i]->elements[j]));

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
 *                   fp_write_vector_vector_with_title
 *
 * Debugging routine that outputs a vector vector
 *
 * This routine outputs the vector vector pointed to by "vvp" to the file pointer
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
 * Index:  I/O, matrices, debugging, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_vector_vector_with_title
(
    const Vector_vector* vvp,
    FILE*                fp,
    const char*          title
)
{
    int   i;


    if (vvp == NULL)
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

    for (i=0; i<vvp->length; i++)
    {
        Vector* vp = vvp->elements[ i ];

        if (vp == NULL)
        {
            ERE(kjb_fprintf(fp, "NULL\n"));
        }
        else
        {
            ERE(fp_write_row_vector(vp, fp));
        }
    }

    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_vector_vector
 *
 * Writes a vector vector to a file specified by name
 *
 * This routine outputs a Vector_vector to a file specified by the input file
 * name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the vector vector contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "vvp" is a pointer to the Vector_vector whose contents are to be
 * written. It cannot be NULL.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the vector vector elements are written in fixed format
 * according to "9.5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_vector_vector(const Vector_vector* vvp, const char* file_name)
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

    write_result = fp_write_vector_vector(vvp, fp);

    kjb_fflush(fp);

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
 *                           fp_write_vector_vector
 *
 * Write a vector vector to a file specified by FILE pointer
 *
 * This routine outputs a Vector_vector to a file specified by a pointer to a
 * FILE.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "vvp" is a pointer to the Vector_vector to output. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_vector_vector(const Vector_vector* vvp, FILE* fp)
{
    int   i;


    for (i=0; i<vvp->length; i++)
    {
        Vector* vp = vvp->elements[ i ];

        if (vp == NULL)
        {
            ERE(kjb_fprintf(fp, "NULL\n"));
        }
        else
        {
            ERE(fp_write_row_vector(vp, fp));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                    write_raw_vector_vector
 *
 * Writes a vector vector to a file specified by name as raw data
 *
 * This routine outputs a vector vector to a file specified by the input file
 * name as raw (binary) data.
 *
 * "file_name" is a pointer to a character array containing the name of the file
 * to write the vector vector contents to. If "file_name" is NULL or equal to
 * '\0', output is directed to STDOUT. Otherwise, the file is created or the
 * existing copy is overwritten.
 *
 * "vvp" is a pointer to the vector vector to be written.
 *
 * The output format depends on the magnitude of the maximum value in the
 * matrix. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the vector vector elements are written in fixed format
 * according to "9.5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_vector_vector(const Vector_vector* vvp, const char* file_name)
{
    FILE* fp;
    int   write_raw_result;
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

    write_raw_result = fp_write_raw_vector_vector(vvp, fp);

    kjb_fflush(fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        if (write_raw_result == ERROR)
        {
            push_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        close_result = kjb_fclose(fp);

        if (write_raw_result == ERROR)
        {
            pop_error_action();
        }
    }

    if ((write_raw_result == ERROR) || (close_result == ERROR))
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
 *                           fp_write_raw_vector_vector
 *
 * Write a vector vector to a file specified by FILE pointer as raw data
 *
 * This routine outputs a vector vector to a file specified by a pointer to
 * FILE as raw data.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "vvp" is a pointer to the vector vector to output. The pointer is assumed to
 * be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_vector_vector(const Vector_vector* vvp, FILE* fp)
{
    int num_vectors = vvp->length;
    int i;


    UNTESTED_CODE();

    ERE(fp_write_raw_vector_vector_header(num_vectors, fp));

    for (i=0; i<num_vectors; i++)
    {
        Vector* vp = vvp->elements[ i ];

        ERE(fp_write_raw_vector(vp, fp));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                      fp_write_raw_vector_vector_header
 *
 * Writes the header for a raw vector vector to a stream
 *
 * This routine outputs the header for a raw vector vector to a stream pointed
 * to by fp. This routine exists so that vector vectors can be written
 * sequentially, one vector at a time, so that the entire vector vector does not
 * need to be in memory).
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, matrices, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_vector_vector_header(int num_vectors, FILE* fp)
{
    int byte_order = 1;
    char head_str[ KJB_DATA_HEAD_SIZE ];
    int  pad = 0;
    int  i;


    UNTESTED_CODE();

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_VECTOR_VECTOR_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, num_vectors));
    ERE(FIELD_WRITE(fp, pad));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     write_vector_vector_full_precision
 *
 * Writes a vector vector to a file specified by name
 *
 * This routine outputs a Vector_vector to a file specified by the input file
 * name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the vector vector contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "vvp" is a pointer to the Vector_vector whose contents are to be
 * written.
 *
 * The output format depends on the magnitude of the values in the vector
 * vector. If the maximum is < 0.01 or greater than 10000, the data is written
 * in exponential format according to the format string "%10.3e". Otherwise the
 * vector vector elements are written in fixed format according to "%9.5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_vector_vector_full_precision
(
    const Vector_vector* vvp,
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

    write_result = fp_write_vector_vector_full_precision(vvp, fp);

    kjb_fflush(fp);

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
 *                       fp_write_vector_vector_full_precision
 *
 * Write a vector vector to a file specified by FILE pointer
 *
 * This routine outputs a Vector_vector to a file specified by a pointer to a
 * FILE.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "vvp" is a pointer to the Vector_vector to output. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, matrices, vector vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_vector_vector_full_precision
(
    const Vector_vector* vvp,
    FILE*                fp
)
{
    int   i;


    for (i=0; i<vvp->length; i++)
    {
        Vector* vp = vvp->elements[ i ];

        if (vp == NULL)
        {
            ERE(kjb_fprintf(fp, "NULL\n"));
        }
        else
        {
            ERE(fp_write_row_vector_full_precision(vp, fp));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int write_v3(const V_v_v* vvvp, const char* file_name)
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

    write_result = fp_write_v3(vvvp, fp);

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

int fp_write_v3(const V_v_v* vvvp, FILE* fp)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        num_vector_vectors = vvvp->length;
    int        i;


    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(kjb_fprintf(fp, "%c%c num_vector_vectors=%d\n\n",
                    kjb_comment_char, kjb_header_char, vvvp->length));

    for (i=0; i<num_vector_vectors; i++)
    {
        ERE(fp_write_vector_vector(vvvp->elements[ i ], fp));

        ERE(kjb_fprintf(fp, "\n%c%ceof\n\n",
                        kjb_comment_char, kjb_header_char));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int write_v3_full_precision(const V_v_v* vvvp, const char* file_name)
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
        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_v3_full_precision(vvvp, fp);

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

int fp_write_v3_full_precision(const V_v_v* vvvp, FILE* fp)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        num_vector_vectors = vvvp->length;
    int        i;


    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(kjb_fprintf(fp, "%c%c num_vector_vectors=%d\n\n",
                    kjb_comment_char, kjb_header_char, vvvp->length));

    for (i=0; i<num_vector_vectors; i++)
    {
        ERE(fp_write_vector_vector_full_precision(vvvp->elements[ i ], fp));

        ERE(kjb_fprintf(fp, "\n%c%ceof\n\n",
                        kjb_comment_char, kjb_header_char));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_v3(V_v_v** vvvpp, const char* file_name)
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

    result = fp_read_v3(vvvpp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_v3(V_v_v** vvvpp, FILE* fp)
{
    IMPORT int kjb_comment_char;
    V_v_v* vvvp;
    int            num_vector_vectors = NOT_SET;
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 5000 ];
    char*          line_pos;
    int            result          = ERROR;


    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    while (num_vector_vectors == NOT_SET)
    {
        int read_result;

        ERE(read_result = BUFF_FGET_LINE(fp, line));

        if (read_result == EOF)
        {
            set_error("Unexpected EOF reached reading vector vector array from %F.\n",
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
                    /*
                    // Look for flag indicating the number of vector vector rows
                    */
                    if (IC_STRCMP_EQ(*option_list_pos,"num_vector_vectors"))
                    {
                        if (ss1pi(*value_list_pos, &num_vector_vectors) == ERROR)
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
            set_error("Cannot find header with num_vector_vectors in %F.", fp);
            return ERROR;
        }
    } /* end while ( num_vector_vectors == NOT_SET )  */


    ERE(get_target_v3(vvvpp, num_vector_vectors));
    vvvp = *vvvpp;

    for (i=0; i<num_vector_vectors; i++)
    {
        result = fp_read_vector_vector(&(vvvp->elements[ i ]), fp);

        if (result == EOF)
        {
            set_error("Unexpected end of file reading vector vector vector from %F.",
                      fp);
            result = ERROR;
        }

        if (result == ERROR) break;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_vector
(
    const char*   output_dir,
    const char*   file_name,
    const Vector* dist_vp
)
{
    char dist_out_file_name[ MAX_FILE_NAME_SIZE ];

    if (dist_vp == NULL) return NO_ERROR;

    BUFF_CPY(dist_out_file_name, output_dir);
    BUFF_CAT(dist_out_file_name, DIR_STR);
    BUFF_CAT(dist_out_file_name, file_name);

    return write_col_vector_full_precision(dist_vp, dist_out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_vector
(
    const char*   output_dir,
    const char*   file_name,
    const Vector* dist_vp
)
{
    char dist_out_file_name[ MAX_FILE_NAME_SIZE ];

    if (dist_vp == NULL) return NO_ERROR;

    BUFF_CPY(dist_out_file_name, output_dir);
    BUFF_CAT(dist_out_file_name, DIR_STR);
    BUFF_CAT(dist_out_file_name, file_name);

    return write_raw_vector(dist_vp, dist_out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

