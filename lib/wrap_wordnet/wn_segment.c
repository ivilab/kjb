/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
| Authors:
|     Luca Del Pero
|
* =========================================================================== */

#include "wrap_wordnet/wn_segment.h"

#ifdef __cplusplus
extern "C" {
#endif

static int fs_human_seg_stripped_rows = 0;
static int fs_human_seg_stripped_cols = 0;
static int fs_machine_seg_stripped_rows = 0;
static int fs_machine_seg_stripped_cols = 0;

int set_human_seg_stripped_rows
(
    int rows
)
{
    if(rows < 0)
    {
        add_error("# of stripped rows should be greater than 0!");
        return ERROR;
    }

    fs_human_seg_stripped_rows = rows;

    return NO_ERROR;
}

int set_human_seg_stripped_cols
(
    int cols
)
{
    if(cols < 0)
    {
        add_error("# of stripped cols should be greater than 0!");
        return ERROR;
    }

    fs_human_seg_stripped_cols = cols;

    return NO_ERROR;
}

int set_machine_seg_stripped_rows
(
    int rows
)
{
    if(rows < 0)
    {
        add_error("# of stripped rows should be greater than 0!");
        return ERROR;
    }

    fs_machine_seg_stripped_rows = rows;

    return NO_ERROR;
}

int set_machine_seg_stripped_cols
(
    int cols
)
{
    if(cols < 0)
    {
        add_error("# of stripped cols should be greater than 0!");
        return ERROR;
    }

    fs_machine_seg_stripped_cols = cols;

    return NO_ERROR;
}

/* read the segmentation mask for an image. It puts the stripped-off pixels back
 * and set their values as '-1'. */
int read_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file,
    int seg_type
)
{
    int result;
    Int_matrix *imp = NULL;
    Int_matrix *segment_imp = NULL;
    int num_rows;
    int num_cols;
    int row;
    int col;
    int        num_stripped_rows;
    int        num_stripped_cols;

    if(seg_type == MACHINE_SEGMENTATION)
    {
        ERE(read_int_matrix(&imp, seg_file));
        num_stripped_rows = fs_machine_seg_stripped_rows;
        num_stripped_cols = fs_machine_seg_stripped_cols;
    }
    else
    {
        ERE(read_UCB_segmentation(&imp, seg_file));
        num_stripped_rows = fs_human_seg_stripped_rows;
        num_stripped_cols = fs_human_seg_stripped_cols;
    }
 
    num_rows = imp->num_rows;
    num_cols = imp->num_cols;

    num_rows += (2 * num_stripped_rows);
    num_cols += (2 * num_stripped_cols);

    ERE(get_initialized_int_matrix(&segment_imp, num_rows, num_cols,
        -1));
    
    for(row=0; row<imp->num_rows; row++)
    {
        for(col=0; col<imp->num_cols; col++)
        {
            segment_imp->elements[row+num_stripped_rows][col+num_stripped_cols] = imp->elements[row][col];
        }
    }
    
    if(segment_impp != NULL)
    {
        ERE(copy_int_matrix(segment_impp, segment_imp));
    }

    free_int_matrix(imp);
    free_int_matrix(segment_imp);

    return NO_ERROR;
}

int read_Corel_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file
)
{
    /*Int_matrix *imp = NULL;
    Int_matrix *segment_imp = NULL;
    int num_rows;
    int num_cols;
    int row;
    int col;*/
    if(read_int_matrix(segment_impp, seg_file) == ERROR)
    {
        return ERROR;
    }

    /*we are not done yet. we need to put the cutoff boundary back*/
   /* num_rows = imp->num_rows;
    num_cols = imp->num_cols;

    ERE(get_initialized_int_matrix(&segment_imp, num_rows + 20, num_cols + 20,
        -1));
    
    for(row=0; row<num_rows; row++)
    {
        for(col=0; col<num_cols; col++)
        {
            segment_imp->elements[row+10][col+10] = imp->elements[row][col];
        }
    }

    if(segment_impp != NULL)
    {
        ERE(copy_int_matrix(segment_impp, segment_imp));
    }

    free_int_matrix(imp);
    free_int_matrix(segment_imp);*/

    return NO_ERROR;
}

int read_UCB_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file
)
{
    FILE *fp = NULL;
    char line[MAX_LINE_LENGTH];
    char *token;
    int len;
    char c;
    int i;
    int data_start = FALSE;
    int height = -1;
    int width = -1;
    int segment;
    int row;
    int start_col, end_col;
    Int_matrix *segment_imp = NULL;

    if((fp = fopen(seg_file, "r")) == NULL)
    {
        add_error("Failed to open file: %s!\n", seg_file);
        return ERROR;
    }

    while(fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        len = strlen(line);
        c = line[len-1];
        if(c == '\n' || c =='\r') /* strip off RETURN */
        {
            line[len-1]='\0';
        }
        
        if(data_start == FALSE)
        {
            token = strtok(line, " ");
            while(token != NULL)
            {
                if(strcmp(token, "width") == 0)
                {
                    token = strtok(NULL, " ");
                    width = atoi(token);
                }
            
                if(strcmp(token, "height") == 0)
                {
                    token = strtok(NULL, " ");
                    height = atoi(token);
                }
                
                if(strncmp(token, "data",4) == 0)
                {
                    ASSERT(width > 0 && height > 0);
                   /* pso("Height: %d Width: %d\n", height+1, width+1);*/
                    ERE(get_target_int_matrix(&segment_imp, height+1, width+1));
                    data_start = TRUE;
                }
                token = strtok(NULL, " ");
            }
        }
        else
        {
            fscanf(fp, "%d %d %d %d", &segment, &row, &start_col, &end_col);
            /*pso("%d %d %d %d\n", segment, row, start_col, end_col);*/
            for(i=start_col; i<=end_col; i++)
            {
                segment_imp->elements[row][i] = segment;
            }
        }
    }

    fclose(fp);

    if(segment_impp != NULL)
    {
        ERE(copy_int_matrix(segment_impp, segment_imp));
    }

    free_int_matrix(segment_imp);

    return NO_ERROR;
}

/* resize a bitmap(matrix) into a given size */
int resize_segmentation_bitmap
(
    const Int_matrix *src_imp,
    int              dst_height,
    int              dst_width,
    Int_matrix **dst_impp
)
{
    int src_height;
    int src_width;
    double w_scale;
    double h_scale;
    int row;
    int col;
    int x;
    int y;
    Int_matrix *dst_imp = NULL;

    ASSERT(dst_width > 0 || dst_height > 0);
    
    src_width = src_imp->num_cols;
    src_height = src_imp->num_rows;
    w_scale = ((double)src_width) / dst_width;
    h_scale = ((double)src_height) / dst_height;

    ERE(get_target_int_matrix(&dst_imp, dst_height, dst_width));
    for(row=0; row<dst_height; row++)
    {
        for(col=0; col<dst_width; col++)
        {
            x = (int)(col * w_scale);
            if(x >= src_width)
            {
                x = src_width-1;
            }

            y = (int)(row * h_scale);
            if(y >= src_height)
            {
                y = src_height-1;
            }

            dst_imp->elements[row][col] = src_imp->elements[y][x];
        }
    } 
   
    if(dst_impp != NULL)
    {
        ERE(copy_int_matrix(dst_impp, dst_imp));
    }

    free_int_matrix(dst_imp);

    return NO_ERROR;
}


int sort_segment_by_area
(
    const Int_matrix *segment_imp,
    Int_vector **segment_index_ivpp,
    Int_vector **segment_area_ivpp
)
{
    int num_segments;
    int segment;
    Indexed_int_vector *segment_indexed_ivp = NULL;
    Int_vector *segment_index_ivp = NULL;
    Int_vector *segment_area_ivp = NULL;
    int num_rows;
    int num_cols;
    int row;
    int col;

    if(segment_imp == NULL)
    {
        return NO_ERROR;
    }

    num_segments = max_int_matrix_element(segment_imp);
    num_segments++;

    if(num_segments == 0)
    {
        add_error("Number of segments is 0!\n");
        return ERROR;
    }

    ERE(get_zero_indexed_int_vector(&segment_indexed_ivp, num_segments));

    num_rows = segment_imp->num_rows;
    num_cols = segment_imp->num_cols;
    for(row=0; row<num_rows; row++)
    {
        for(col=0; col<num_cols; col++)
        {
            segment = segment_imp->elements[row][col];
            if(segment >= 0)
            {
                (segment_indexed_ivp->elements[segment]).element++;
            }
        }
    }

    ERE(descend_sort_indexed_int_vector(segment_indexed_ivp));
    
    ERE(get_target_int_vector(&segment_index_ivp, num_segments));
    ERE(get_target_int_vector(&segment_area_ivp, num_segments));
    for(segment=0; segment<num_segments; segment++)
    { 
        segment_index_ivp->elements[segment] =
            segment_indexed_ivp->elements[segment].index;
 /*           pso("%d\n",  segment_indexed_ivp->elements[segment].index);*/
        segment_area_ivp->elements[segment] =
            segment_indexed_ivp->elements[segment].element;
    }

    if(segment_index_ivpp != NULL)
    {
        ERE(copy_int_vector(segment_index_ivpp, segment_index_ivp));
    }
    
    if(segment_area_ivpp != NULL)
    {
        ERE(copy_int_vector(segment_area_ivpp, segment_area_ivp));
    }

    free_indexed_int_vector(segment_indexed_ivp);
    free_int_vector(segment_index_ivp);
    free_int_vector(segment_area_ivp);

    return NO_ERROR;
}

int get_segment_index
(
    const Array       *segfile_array_ptr,
    Int_vector_vector **segment_index_ivvpp
)
{
    int num_files;
    int file;
    char *filename;
    Int_matrix *segment_imp = NULL;
    Int_vector_vector *segment_index_ivvp = NULL;
    int res = NO_ERROR;

    if(segfile_array_ptr == NULL || segment_index_ivvpp == NULL)
    {
        return NO_ERROR;
    }

    num_files = segfile_array_ptr->length;
    ERE(get_target_int_vector_vector(segment_index_ivvpp, num_files));
    segment_index_ivvp = *segment_index_ivvpp;
    
    for(file=0; file<num_files; file++)
    {
        filename = (char*)segfile_array_ptr->elements[file];
        if(read_Corel_segmentation(&segment_imp, filename) == ERROR)
        {
            res = ERROR;
            break;
        }
        
        ERE(sort_segment_by_area(segment_imp, &(segment_index_ivvp->elements[file]),
           NULL));
    }

    if(res == ERROR)
    {
        free_int_vector_vector(segment_index_ivvp);
        *segment_index_ivvpp = NULL;
    }

    free_int_matrix(segment_imp);

    return res;
}

int read_UCB_num_segments
(
    int * num_segments,
    const char *seg_file
)
{
    FILE *fp = NULL;
    char line[MAX_LINE_LENGTH];
    char *token;
    int len;
    char c;
    int found_num_segs = FALSE;

    if((fp = fopen(seg_file, "r")) == NULL)
    {
        add_error("Failed to open file: %s!\n", seg_file);
        return ERROR;
    }

    while( (fgets(line, MAX_LINE_LENGTH, fp) != NULL) && (!found_num_segs))
    {
        len = strlen(line);
        c = line[len-1];
        if(c == '\n' || c =='\r') /* strip off RETURN */
        {
            line[len-1]='\0';
        }

        if(found_num_segs == FALSE)
        {
            token = strtok(line, " ");
            while(token != NULL)
            {
                if(strcmp(token, "segments") == 0)
                {
                    token = strtok(NULL, " ");
                    (*num_segments) = atoi(token);
                    found_num_segs = TRUE;
                }
                token = strtok(NULL, " ");
            }
        }
    }

    fclose(fp);

    return NO_ERROR;
}


#ifdef __cplusplus
}
#endif
