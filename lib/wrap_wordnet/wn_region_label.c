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

#include "wrap_wordnet/wn_region_label.h"

#ifdef KJB_HAVE_WN
#include "wn.h"
#else 
#include "wrap_wordnet/wn_dont_have.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

double fs_area_weight_cutoff = 0.01;

static int merge_labels_from_regions
(
    const Array *src_label_array_ptr,
    const Vector *counter_vp,
    Region_label **label_ptr_ptr
);

static void replace_space_by_underscore
(
    const char *source_str,
    char       *target_str
);

static int ow_remove_ancestors_in_label_array_1
(
    Array       *label_array_ptr,
    const Array *word_array_ptr,
    const Array *tree_array_ptr,
    int          parent_type
);

static int ow_remove_ancestors_in_label_1
(
    Region_label *label_ptr,
    const Array *word_array_ptr,
    const Array  *tree_array_ptr,
    int          parent_type
);

static int is_ancestor_1
(
    const Word_sense *word1_wp, 
    int              index,
    const Word_sense *word2_wp,
    const Array      *tree_array_ptr,
    int              parent_type
);

Region_label *construct_region_label
(
    int length 
)
{
    int i;
    Region_label *label_ptr = NULL;
    
    label_ptr = (Region_label*)malloc(sizeof(Region_label));
    if(label_ptr == NULL) return NULL;

    label_ptr->words = (Word_sense**)malloc(sizeof(Word_sense*)*length);
    if(label_ptr->words == NULL)
    {
        free(label_ptr);
        return NULL;
    }
    
    for(i=0; i<length; i++)
    {
        label_ptr->words[i] = NULL;
    }

    label_ptr->length = length;

    return label_ptr;
    
}

void print_region_label
(
    const void *label_ptr
)
{
    int i;
    int length;
    Region_label *ptr = NULL;
    Word_sense *word_wp = NULL;
    
    ptr = (Region_label*)label_ptr;
    if(ptr == NULL) return;

     length = ptr->length;
     for(i=0; i<length; i++)
     {
         word_wp = ptr->words[i];
         if(i != length - 1)
         {
             pso("[%d]%-12s(%2d)#%8.4f : ", word_wp->index, word_wp->word, word_wp->sense,
                 word_wp->value);
         }
         else
         {
             pso("[%d]%-12s(%2d)#%8.4f\n", word_wp->index, word_wp->word, word_wp->sense,
                 word_wp->value);
         }
     }
}

void free_region_label
(
    void *label_ptr
)
{
    int i;
    int length;
    Region_label *ptr = NULL;

    ptr = (Region_label*)label_ptr;

    if(ptr == NULL) return;

    length = ptr->length;
    if(ptr->words != NULL)
    {
        for(i=0; i<length; i++)
        {
            free(ptr->words[i]);
        }
        
        free(ptr->words);
    }

    free(ptr);
    ptr = NULL;
}

void free_region_label_words_only
(
    void *label_ptr
)
{
    int i;
    int length;
    Region_label *ptr = NULL;

    ptr = (Region_label*)label_ptr;

    if(ptr == NULL) return;

    length = ptr->length;
    if(ptr->words != NULL)
    {
        for(i=0; i<length; i++)
        {
            free(ptr->words[i]);
        }
        
        free(ptr->words);
        ptr->words = NULL;
    }
}


int read_words_to_array
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Array **array_ptr_ptr
)
{
   int length;

   Linkedlist *list_ptr = NULL;

   if(array_ptr_ptr == NULL)
   {
       return NO_ERROR;
   }

   ERE(read_words_to_list(vocabulary_file, is_morphed, is_sensed, &list_ptr));
 
   length = list_ptr->length;

   ERE(from_list_to_array(list_ptr, array_ptr_ptr));
   free_linkedlist(list_ptr, NULL);

   return length;
}

int read_words_to_array_2
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Array **array_ptr_ptr
)
{
   int length;

   Linkedlist *list_ptr = NULL;

   if(array_ptr_ptr == NULL)
   {
       return NO_ERROR;
   }

   ERE(read_words_to_list(vocabulary_file, is_morphed, is_sensed, &list_ptr));

   length = list_ptr->length;

   ERE(from_list_to_array(list_ptr, array_ptr_ptr));
   free_linkedlist(list_ptr, NULL);

   return NO_ERROR;
}

int read_words_to_list
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Linkedlist **list_ptr_ptr
)
{
    FILE *fp = NULL;
    char line[MAX_WORD_LENGTH];
    int res = NO_ERROR;
    Word_sense *word_wp = NULL;
    Linkedlist *list_ptr = NULL;
    int count = 0;
    int len;
    char c;

    if(list_ptr_ptr == NULL)
    {
      return NO_ERROR;
    }

    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        return ERROR;
    }
    *list_ptr_ptr = list_ptr;

    if((fp = fopen(vocabulary_file, "r")) == NULL)
    {
        free_linkedlist(list_ptr, NULL);
        add_error("Failed to open file: %s!\n", vocabulary_file);
        return ERROR;
    }

    count = 0;
    while(fgets(line, MAX_WORD_LENGTH, fp) != NULL)
    {
        len = strlen(line);
        c = line[len-1];
        if(c == '\n' || c =='\r') /* strip off RETURN */
        {
            line[len-1]='\0';
        }

        ERE(parse_word_sense_from_str(line, is_morphed, is_sensed, &word_wp));
        word_wp->index = count++;  /* index the word */
        ERE(append_element_with_contents(list_ptr, (void*)word_wp));
        word_wp = NULL; /* needed as the pointer is reused if not set to NULL */
    }

    if(res == ERROR)
    {
        free_linkedlist(list_ptr, free);
        return ERROR;
    }

    fclose(fp);

    return NO_ERROR;
}

int read_region_labels_to_array
(
    const char *label_file,
    int        is_morphed,
    int        is_sensed,
    Array **label_array_ptr_ptr
)
{
    FILE *fp = NULL;
    char line[MAX_WORD_LENGTH];
    int res = NO_ERROR;
    Region_label *label_ptr = NULL;
    Linkedlist *list_ptr = NULL;
    int length;
    int len;
    char c;
    int label_start = TRUE;

    if(label_array_ptr_ptr == NULL)
    {
      return NO_ERROR;
    }

    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        return ERROR;
    }

    if((fp = fopen(label_file, "r")) == NULL)
    {
        printf("Label file does not exist\r\n");
        free_linkedlist(list_ptr, NULL);
        add_error("Failed to open file: %s!\n", label_file);
        return ERROR;
    }

    printf("Start reading labels\r\n");
    while(fgets(line, MAX_WORD_LENGTH, fp) != NULL)
    {
        len = strlen(line);
        c = line[len-1];
        if(c == '\n' || c =='\r') /* strip off RETURN */
        {
            line[len-1]='\0';
        }

        c = line[0];
        if(c == '#') 
        {
            if(strncmp(line, LABEL_START_SIGN_STR,8) == 0)
            {
                label_start = TRUE;
                continue;
            }

            label_start = FALSE; 
        }
        if(label_start == FALSE) continue;  /*not labels yet */
        
        ERE(parse_region_label_from_str(line, is_morphed, is_sensed, &label_ptr));
        ERE(append_element_with_contents(list_ptr, (void*)label_ptr));
       /* print_region_label(label_ptr);*/
        label_ptr = NULL; /* needed as the pointer is reused if not set to NULL */
    }

    if(res == ERROR)
    {
        free_linkedlist(list_ptr, free_region_label);
        return ERROR;
    }

    length = list_ptr->length;
    ERE(from_list_to_array(list_ptr, label_array_ptr_ptr));

    free_linkedlist(list_ptr, NULL);
    fclose(fp);

    /*return length;*/
    return NO_ERROR;
}

int count_word_frequency
(
    const Int_matrix *word_imp,
    int              num,
    Vector           **freq_vpp
)
{
    int i, j;
    /*int max_num = -1;*/
    Vector *freq_vp = NULL;
    int tot_count;
    int entry;

  /*  ERE(get_max_int_matrix_element(word_imp, &max_num, NULL, NULL));
    ASSERT(max_num >= 0);*/

    ERE(get_zero_vector(&freq_vp, num));

    tot_count = 0;
    for(i=0; i<word_imp->num_rows; i++)
    {
        for(j=0; j<word_imp->num_cols; j++)
        {
            entry = word_imp->elements[i][j];
            if(entry >= 0 && entry < num)
            {
                (freq_vp->elements[entry]) += 1.0;
                tot_count++;
            }
        }
    }

    ERE(ow_divide_vector_by_scalar(freq_vp, (double)tot_count));
    /*write_col_vector(freq_vp, 0);*/

    if(freq_vpp != NULL)
    {
        ERE(copy_vector(freq_vpp, freq_vp));
    }

    free_vector(freq_vp);
    return NO_ERROR;
}
    
/* find the new words that are not in the vocabulary.*/
int get_new_words
(
    const Array *vocabulary_array_ptr,
    const Array *label_array_ptr,
    Array **new_array_ptr_ptr
)
{
    int i, j;
    Region_label *label_ptr = NULL;
    Linkedlist *list_ptr = NULL;
    Word_sense *word_wp = NULL;
    /*Word_sense *new_word_wp = NULL;*/
    int length, length1, length2;
    int res = NO_ERROR;

    if(new_array_ptr_ptr == NULL) return NO_ERROR;

    if((list_ptr = create_linkedlist()) == NULL)
    {
        add_error("Failed to create linked list!\n");
        return ERROR;
    }

    length1 = label_array_ptr->length;
    for(i = 0; i < length1; i++)
    {
        label_ptr = (Region_label*)label_array_ptr->elements[i];
        if(label_ptr == NULL)
        {
            res = ERROR;
            break;
        }

        length2 = label_ptr->length;
        for(j = 0; j < length2; j++)
        {
            word_wp = label_ptr->words[j];
            if(word_wp == NULL)
            {
                res = ERROR;
                break;
            }
            if(search_array(vocabulary_array_ptr, compare_word_sense, word_wp) < 0
              && !search_linkedlist(list_ptr, compare_word_sense, word_wp))
            {
               /* ERE(copy_word_sense(&new_word_wp, word_wp));*/
                if(append_element_with_contents(list_ptr, word_wp) == ERROR)
                {
                    res = ERROR; 
                    break;
                }
            }
        }
        
        if(res == ERROR) break;
    } 
    
    if(res != ERROR)
    {
        res = from_list_to_array(list_ptr, new_array_ptr_ptr);
    }

    length = 0;
    if(list_ptr != NULL)
    {
        length = list_ptr->length;
    }

    free_linkedlist(list_ptr, NULL);

    if(res == ERROR) return ERROR;

    return length;
}

int parse_region_label_from_str
(
    const char *line_str,
    int        is_morphed,
    int        is_sensed,
    Region_label **label_ptr_ptr
)
{
    
    Region_label *label_ptr = NULL;
    Word_sense *word_wp = NULL;
    Linkedlist *list_ptr = NULL;
    List_element *current = NULL;
    char copied_line_str[MAX_WORD_LENGTH] ;
    const char *delimit=":";
    char *token;
    char *split_str;
    int length;
    int count;

    if(label_ptr_ptr == NULL)
    {
       return NO_ERROR;
    }
    
    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        add_error("Failed to create linked list!\n");
        return ERROR;      
    }

    /*determine the number of words. Ugly code, but avoid the nortorish
      problem of the 'strtok' */
    strcpy(copied_line_str, line_str);
    token = strtok(copied_line_str, delimit);
    while(token != NULL)
    {
        /* split the token into two parts if '_' occurs in the string*/
         split_str = strchr(token, '_');
        if(split_str != NULL)
        {
            ERE(append_element_with_contents(list_ptr, strdup(split_str+1)));
        }
        else
        {
            ERE(append_element_with_contents(list_ptr, strdup(token)));
        }
       
        token = strtok(NULL, delimit);
    }

    length = list_ptr->length;
    if(length == 0)
    {
        free_linkedlist(list_ptr, free);
        return NO_ERROR;
    }

    label_ptr = construct_region_label(length);
    if(label_ptr == NULL)
    {
        add_error("Failed to create 'Region_label' structure!\n");
        return ERROR;
    }
    *label_ptr_ptr = label_ptr;

    count = 0;
    current = list_ptr->head;
    while(current != NULL)
    {
        token = (char*)current->contents;
        ERE(parse_word_sense_from_str(token, is_morphed, is_sensed, &word_wp));
        label_ptr->words[count++] = word_wp;
        word_wp = NULL;
        current = current->next;
    }
    
    free_linkedlist(list_ptr, free);
    return length;
}
    
int parse_word_sense_from_str
(
    const char *line_str,
    int        is_morphed,
    int        is_sensed,
    Word_sense **word_wpp
)
{
#ifdef KJB_HAVE_WN 
    char copied_str[MAX_WORD_LENGTH] ;
    char word_str[MAX_WORD_LENGTH] ;
    char new_word_str[MAX_WORD_LENGTH] ;
    char sense_str[MAX_WORD_LENGTH];
    char value_str[MAX_WORD_LENGTH];
    const char *delimit = "()#";
    char *morphed_word = NULL;
    char *token;
    double value;
    int sense;
    int res = NO_ERROR;
    word_str[0] = '\0';
    sense_str[0] = '\0';
    value_str[0] = '\0';
     
    strcpy(copied_str, line_str);
    token = strtok(copied_str, delimit);
    if(token == NULL)
    {
        res = ERROR;
    }

    if(res != ERROR) /* token the word */
    {
        strcpy(word_str, token);
        token = strtok(NULL, delimit);
        if(token == NULL)
        {
            res = ERROR;
        }
    }
    
    if(res != ERROR) /* token the sense */
    {
        strcpy(sense_str, token);
        token = strtok(NULL, delimit);
        if(token == NULL)
        {
            res = ERROR;
        }    
    }
    
    if(res != ERROR) /* token the value */
    {
        strcpy(value_str, token);
        token = strtok(NULL, delimit);
        if(token != NULL)  /* nothing should be followed! */
        {
            res = ERROR;
        }    
    }
    
    if(token != NULL)
    {
        add_error("The format should be '<word>(<#sense>)#<value>!\n");
        return ERROR;
    }

    if(strlen(word_str) == 0)
    {
        add_error("The format should be '<word>(<#sense>)#<value>!\n");
        return ERROR;
    }
   
    replace_space_by_underscore(word_str, new_word_str);
    ToLowerCase(new_word_str);
    if(is_morphed)
    {
        /* morph the word */
        morphed_word = morphword(new_word_str, NOUN);
       if(morphed_word != NULL)
       {
        strcpy(new_word_str, morphed_word);
       }
    }

    if(is_sensed)
    {
        sense = (strlen(sense_str) > 0) ? atoi(sense_str) : 1;
    }
    else
    {
        sense = -1;
    }
    
    if(strlen(value_str) > 0)
    {
        value = atof(value_str);
    }
    else
    {
        value = 1.0; /* use 1.0 */
    }

    if(word_wpp != NULL)
    {
        ERE(get_target_word_sense(word_wpp, DEFAULT_WORD_INDEX, 
            new_word_str, sense, value));
    }

    return NO_ERROR;
#else 
    set_dont_have_wn_error();
    return ERROR; 
#endif 
}

/* replace the space between words by '_' */
static void replace_space_by_underscore
(
    const char *source_str,
    char       *target_str
)
{
    char *token;
    char copied_str[MAX_WORD_LENGTH];

    target_str[0] = '\0';
    strcpy(copied_str, source_str);
    token = strtok(copied_str, " ");
    while(token != NULL)
    {
        strcat(target_str, token);
        token = strtok(NULL, " ");
        if(token != NULL)
        {
            strcat(target_str, "_");
        }
    }

}

/* merge the occurrence of the duplicates, which is caused by
   plurals. */
int merge_word_occurrence
(
    const Array      *vocabulary_array_ptr,
    const Int_matrix *word_occurrence_mp,
    Array            **new_array_ptr_ptr,
    Int_matrix       **new_occurrence_mpp
)
{
#ifdef KJB_HAVE_WN 
    int i, k1, k2;
    int index;
    int length;
    char *morphed_word;
    char *word;
    Word_sense *word_wp = NULL;
    Word_sense *tmp_word_wp = NULL;
    Array *new_array_ptr = NULL;
    Int_matrix *new_occurrence_mp = NULL;
    int res = NO_ERROR;
    
    length = vocabulary_array_ptr->length;
    ERE(copy_int_matrix(&new_occurrence_mp, word_occurrence_mp));
    new_array_ptr = create_array(length);
    for(i = 0; i < length; i++)
    {
        word_wp = (Word_sense*)vocabulary_array_ptr->elements[i];
        morphed_word = morphword(word_wp->word, NOUN);
        if(morphed_word != NULL)
        {
            word = morphed_word;
        }
        else
        {
            word = word_wp->word;
        }
        tmp_word_wp = create_word_sense(word_wp->index, 
                      word, word_wp->sense, word_wp->value);
        if(tmp_word_wp == NULL)
        {
            res = ERROR;
            break;
        } 

        new_array_ptr->elements[i] = tmp_word_wp;
        if(morphed_word != NULL) /* check if the singular exists or not */
        {
            index = is_word_in_vocabulary(tmp_word_wp, vocabulary_array_ptr);
            if(index >= 0)
            {
                /*keep the same name, but remove entry in the occurrence matrix*/
                strcpy(tmp_word_wp->word, word_wp->word);

                for(k1 = 0; k1 < new_occurrence_mp->num_rows; k1++)
                {
                    for(k2 = 0; k2 < new_occurrence_mp->num_cols; k2++)
                    {
                        if(new_occurrence_mp->elements[k1][k2] == i)
                        {
                            new_occurrence_mp->elements[k1][k2] = index;
                        }
                    }
                }
            }
        }
    }

    if(res == ERROR)
    {
        free_array(new_array_ptr, free);
        free_int_matrix(new_occurrence_mp);
        return ERROR;
    }

    if(new_array_ptr_ptr == NULL)
    {
        free_array(new_array_ptr, free);
    }
    else
    {
        *new_array_ptr_ptr = new_array_ptr;
    }

    if(new_occurrence_mpp == NULL)
    {
        free_int_matrix(new_occurrence_mp);
    }
    else
    {
        *new_occurrence_mpp = new_occurrence_mp;
    }

    return NO_ERROR;
#else 
    set_dont_have_wn_error();
    return ERROR; 
#endif 
}

int is_word_in_vocabulary
(
    const   Word_sense *word_wp,
    const   Array      *vocabulary_array_ptr
)
{
    int i;
    int length;
    
    if(vocabulary_array_ptr == NULL) return -1;

    length = vocabulary_array_ptr->length;
    for(i=0; i<length; i++)
    {
        if(compare_word_sense(vocabulary_array_ptr->elements[i], (void*)word_wp)
            == 1)
        {
            return i;
        }
    }

    return -1;
}

int relabel_image_by_segmentation
(
    const Int_matrix *src_segment_imp,
    const Int_matrix *dst_segment_imp,
    const Array      *src_label_array_ptr,
    Array            **dst_label_array_ptr_ptr
)
{
    int num_rows;
    int num_cols;
    int src_segments;
    int dst_segments;
    int num_pixels;
    int seg;
    int row;
    int col;
    int src_seg;
    Vector *counter_vp = NULL;
    Array *dst_label_array_ptr = NULL;
    Region_label *region_label_ptr = NULL;

    if(dst_label_array_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    num_rows = src_segment_imp->num_rows;
    num_cols = src_segment_imp->num_cols;

    ASSERT(num_rows == dst_segment_imp->num_rows &&
           num_cols == dst_segment_imp->num_cols);

    ERE(get_max_int_matrix_element(src_segment_imp, &src_segments, NULL, NULL));
    ERE(get_max_int_matrix_element(dst_segment_imp, &dst_segments, NULL, NULL));
    src_segments++;
    dst_segments++;

    dst_label_array_ptr = create_array(dst_segments);
    if(dst_label_array_ptr == NULL)
    {
        return ERROR;
    }

    for(seg=0; seg<dst_segments; seg++)
    {
        num_pixels = 0;
        ERE(get_zero_vector(&counter_vp, src_segments));
        for(row=0; row<num_rows; row++)
        {
            for(col=0; col<num_cols; col++)
            {
                if(dst_segment_imp->elements[row][col] == seg)
                {
                    src_seg = src_segment_imp->elements[row][col];
                    (counter_vp->elements[src_seg])++;
                    num_pixels++;
                }
            }
        }
        pso("seg: %d\n", seg);
        if(num_pixels > 0)  /* if # of pixels is 0, then set it as NULL */
        {
            ERE(ow_divide_vector_by_scalar(counter_vp, (double)num_pixels));
            write_row_vector(counter_vp, 0);

            ERE(merge_labels_from_regions(src_label_array_ptr, counter_vp,
                &region_label_ptr));
        }

        dst_label_array_ptr->elements[seg] = region_label_ptr;
        region_label_ptr = NULL;
    }

    *dst_label_array_ptr_ptr = dst_label_array_ptr;
    free_vector(counter_vp);

    return NO_ERROR;
}

static int merge_labels_from_regions
(
    const Array *src_label_array_ptr,
    const Vector *counter_vp,
    Region_label **label_ptr_ptr
)
{
    Linkedlist *list_ptr = NULL;
    int num_regions;
    int num_words;
    double weight;
    int word;
    int region;
    Region_label *label_ptr = NULL;
    Array *word_array_ptr = NULL;
    Word_sense *word_wp = NULL;
    Word_sense *tmp_word_wp = NULL;

    if(label_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        return ERROR;
    }

   /* write_col_vector(counter_vp,0);*/
    num_regions = src_label_array_ptr->length;
    for(region=0; region<num_regions; region++)
    {
        weight = counter_vp->elements[region];
        if(weight >= fs_area_weight_cutoff)
        {
            label_ptr = (Region_label*)src_label_array_ptr->elements[region];
            /*print_region_label(label_ptr);*/
            num_words = label_ptr->length;
            for(word=0; word<num_words; word++)
            {
                word_wp = label_ptr->words[word];
                tmp_word_wp = (Word_sense*)search_linkedlist(list_ptr,
                    compare_word_sense, word_wp);
                if(tmp_word_wp == NULL)
                {
                    ERE(copy_word_sense(&tmp_word_wp, word_wp));
                    tmp_word_wp->value = weight;
                    ERE(append_element_with_contents(list_ptr, tmp_word_wp));
                   /* print_word_sense(tmp_word_wp);*/
                }
                else /* already in the list */
                {
                    tmp_word_wp->value += weight; 
                }
            }
        }
    }

    ERE(from_list_to_array(list_ptr, &word_array_ptr));

    num_words = list_ptr->length;
    label_ptr = construct_region_label(num_words);
    if(label_ptr == NULL)
    {
        free_linkedlist(list_ptr, NULL);
        free_array(word_array_ptr, free);
        return ERROR;
    }

    for(word=0; word<num_words; word++)
    {
        ERE(copy_word_sense(&(label_ptr->words[word]),
            (Word_sense*)word_array_ptr->elements[word]));
    }

    *label_ptr_ptr = label_ptr;
   /* print_region_label(label_ptr);*/

    free_linkedlist(list_ptr, NULL);
    free_array(word_array_ptr, free);

    return NO_ERROR;
}

/*int parse_word_sense_from_str1
(
    const char *line_str,
    int        is_morphed,
    Word_sense **word_wpp
)
{
    int sense_started = 0;
    int sense_ended = 0;
    char c;
    char word_str[MAX_WORD_LENGTH];
    char sense_str[MAX_WORD_LENGTH];
    int sense;
    char *sense_ptr;
    char *word_ptr;
    char *morphed_word = NULL;
    
    sense_ptr = sense_str;
    word_ptr = word_str;

    while((c = *line_str++) != '\0')
    {
        if(c == '(')
        {
            sense_started = 1;
        }
        else if(c == ')')
        {
           sense_ended = 1;
           break;
        }
        else 
        {
            if(sense_started)
            {
                *sense_ptr++ = c;
            }
            else
            {
                *word_ptr++ = c;
            }
        }
    }

    *sense_ptr = '\0'; 
    *word_ptr = '\0';

    if(sense_started)
    {
        if(!sense_ended)
        {
            add_error("fortmat error!\n");
            return ERROR;
        }

        sense = atoi(sense_str);
    }
    else 
    {
        sense = 1;
    }

    if(is_morphed)
    {
        morphed_word = morphword(word_str, NOUN);
    }

    if(morphed_word != NULL)
    {
        strcpy(word_str, morphed_word);
    }

    if(word_wpp != NULL)
    {
        ERE(get_target_word_sense(word_wpp, DEFAULT_WORD_INDEX, 
            word_str, sense, 0.0));
    }

    return NO_ERROR;
}*/
int read_filename_list
(
    const char  *list_filename,
    Array      **file_array_ptr_ptr
)
{
    char filename[MAX_FILE_NAME_SIZE];
    Array *file_array_ptr = NULL;
    FILE *fp = NULL;
    int i;
    int count;

    if(file_array_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }
    
    fp = fopen(list_filename, "r");
    if(fp == NULL)
    {
        add_error("Failed to open %s\n", list_filename);
        return ERROR;
    }
    
    count = 0;
    while(fscanf(fp, "%s", filename) != EOF)
    {
        count++;
    }


    if(count == 0)
    {
        fclose(fp);
        add_error("Empty file %s\n", list_filename);
        return ERROR;
    }
    
    fseek(fp, 0L, SEEK_SET);
    file_array_ptr = create_array(count);
    if(file_array_ptr == NULL)
    {
        fclose(fp);
        return ERROR;
    }

    for(i=0; i<count; i++)
    {
        fscanf(fp, "%s", filename);
        file_array_ptr->elements[i] = strdup(filename);
    }

    fclose(fp);
    *file_array_ptr_ptr = file_array_ptr;

    return NO_ERROR;
}

/* read the labels of a list of images into an array-array data strutcture.
   Each array element is an array that stores the region labels for an image.*/
int read_images_labels
(
    const char *list_filename,
    int        is_morphed,
    int        is_sensed,
    Array      **label_array_array_ptr_ptr
)
{
    Array *file_array_ptr = NULL;
    Array *label_array_ptr = NULL;
    Array *label_array_array_ptr = NULL;
    int num_files;
    int file;
    char *filename;
    int i;
    int res = NO_ERROR;

    if(label_array_array_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    ERE(read_filename_list(list_filename, &file_array_ptr));

    if(file_array_ptr == NULL)
    {
        return NO_ERROR;
    }
    num_files = file_array_ptr->length;

    label_array_array_ptr = create_array(num_files);
    if(label_array_array_ptr == NULL)
    {
        free_array(file_array_ptr, free);
        return ERROR;
    }

    for(file=0; file<num_files; file++)
    {
        filename = (char*)file_array_ptr->elements[file];
        if(read_region_labels_to_array(filename, is_morphed, is_sensed,
            &label_array_ptr) == ERROR)
        {
            res = ERROR;
            break;
        }

        label_array_array_ptr->elements[file] = label_array_ptr;
        label_array_ptr = NULL;
    }

    if(res == ERROR)
    {
        for(i=0; i<num_files; i++)
        {
            free_array((Array*)(label_array_array_ptr->elements[i]), free_region_label);
        }
        free_array(label_array_array_ptr, NULL);
    }

    if(res != ERROR)
    {
        *label_array_array_ptr_ptr = label_array_array_ptr;
    }

    free_array(file_array_ptr, free);   

    return res;
}

int relabel_images
(
    const Array *label_array_array_ptr,
    const Array *srcfile_array_ptr,
    const Array *dstfile_array_ptr,
    Array      **newlabel_array_array_ptr_ptr
)
{
   Array *label_array_ptr = NULL;
   Array *newlabel_array_ptr = NULL;
   Array *newlabel_array_array_ptr = NULL;
   Int_matrix *src_imp = NULL;
   Int_matrix *dst_imp = NULL;
   Int_matrix *resize_dst_imp = NULL;
   char *filename;
   int  num_files;
   int file;
   int i;
   int res = NO_ERROR;

    if(label_array_array_ptr == NULL ||
       srcfile_array_ptr == NULL ||
       dstfile_array_ptr == NULL)
    {
       return NO_ERROR;
    }

    num_files = srcfile_array_ptr->length;
    ASSERT(num_files == label_array_array_ptr->length && 
           num_files == dstfile_array_ptr->length);
    
    newlabel_array_array_ptr = create_array(num_files);
    if(newlabel_array_array_ptr == NULL)
    {
        return ERROR;
    }

    for(file=0; file<num_files; file++)
    {
         filename = (char*)srcfile_array_ptr->elements[file];
         /*if(read_UCB_segmentation(&src_imp, filename) == ERROR)*/
         if(read_segmentation(&src_imp, filename, HUMAN_SEGMENTATION) == ERROR)
         {
             res = ERROR;
             break;
         }
        /* pso("%s %d %d\n", filename, src_imp->num_rows, src_imp->num_cols);*/

         filename = (char*)dstfile_array_ptr->elements[file];
         /*if(read_int_matrix(&dst_imp, filename) == ERROR)
         {
             res = ERROR;
             break;
         }*/
         /*if(read_Corel_segmentation(&dst_imp, filename) == ERROR)*/
         if(read_segmentation(&dst_imp, filename, MACHINE_SEGMENTATION) == ERROR)
         {
             res = ERROR;
             break;
         }

        /* pso("%s %d %d\n", filename,dst_imp->num_rows, dst_imp->num_cols);*/

         ERE(resize_segmentation_bitmap(dst_imp, src_imp->num_rows,
             src_imp->num_cols, &resize_dst_imp));
 
         label_array_ptr = (Array*)label_array_array_ptr->elements[file];
         if(relabel_image_by_segmentation(src_imp,
                                           resize_dst_imp,
                                           label_array_ptr,
                                           &newlabel_array_ptr) == ERROR)
         {
             kjb_print_error();
             res = ERROR;
             break;
         }
         /*print_array(newlabel_array_ptr, print_region_label);*/
         newlabel_array_array_ptr->elements[file] = newlabel_array_ptr;
         newlabel_array_ptr = NULL;
    }

    if(res == ERROR)
    {
        for(i=0; i<num_files; i++)
        {
            free_array((Array*)(newlabel_array_array_ptr->elements[i]),
                free_region_label);
        }
        free_array(newlabel_array_array_ptr, NULL);
    }

    *newlabel_array_array_ptr_ptr = newlabel_array_array_ptr;

    free_int_matrix(src_imp);
    free_int_matrix(dst_imp);
    free_int_matrix(resize_dst_imp);

    return res;
}

int get_sorted_segment_by_area
(
    const char *list_filename
)
{
    Array *file_array_ptr = NULL;
    int i, j;
    Int_matrix *imp = NULL;
    Int_vector *index_ivp = NULL;

    ERE(read_filename_list(list_filename, &file_array_ptr));

    for(i=0; i<file_array_ptr->length; i++)
    {
        /*ERE(read_Corel_segmentation(&imp,  (char*)(file_array_ptr->elements[i])));*/
        ERE(read_segmentation(&imp, (char*)(file_array_ptr->elements[i]), MACHINE_SEGMENTATION));
        ERE(sort_segment_by_area(imp, &index_ivp, NULL));
        for(j=0; j<index_ivp->length; j++)
        {
            pso("%d ", index_ivp->elements[j]);
        }
        pso("\n");
    }

    free_int_matrix(imp);
    free_int_vector(index_ivp);


    return NO_ERROR;
}

/* get a list of truth words without duplicates from the region labels */
int get_vocabulary_from_labels
(
    const Array *label_array_array_ptr, 
    Array       **word_array_ptr_ptr
)
{
    int image;
    int region;
    int word;
    int num_images;
    int num_regions;
    int num_words;
    int count;
    Linkedlist *list_ptr = NULL;
    Array *label_array_ptr = NULL;
    Region_label *label_ptr = NULL;
    Word_sense *tmp_word_wp = NULL;
    Word_sense *word_wp = NULL;

    if(label_array_array_ptr == NULL || word_array_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        return ERROR;
    }

    count = 0;
    num_images = label_array_array_ptr->length;
    for(image=0; image<num_images; image++)
    {
        /*pso("Image %d\n", image);*/
        label_array_ptr = (Array*)label_array_array_ptr->elements[image];
        num_regions = label_array_ptr->length;
        for(region=0; region<num_regions; region++)
        {
            label_ptr = (Region_label*)label_array_ptr->elements[region];
            num_words = label_ptr->length;
           /* print_region_label(label_ptr);*/
            for(word=0; word<num_words; word++)
            {
                word_wp = label_ptr->words[word];
                if(search_linkedlist(list_ptr, compare_word_sense, word_wp) == NULL)
                {
                   /* pso("%s(%d) added\n", word_wp->word, word_wp->sense);*/
                    ERE(copy_word_sense(&tmp_word_wp, word_wp));
                    tmp_word_wp->index = (count++);
                    ERE(append_element_with_contents(list_ptr, tmp_word_wp));
                    tmp_word_wp = NULL;
                }
            }
        }
    }
    
    ERE(from_list_to_array(list_ptr, word_array_ptr_ptr));

    free_linkedlist(list_ptr, NULL);

    return NO_ERROR;
}

void free_label_array_array
(
   Array *label_array_array_ptr
)
{
    int i;
    int len;

    if(label_array_array_ptr == NULL) return;

    len = label_array_array_ptr->length;
    for(i=0; i<len; i++)
    {
        free_array((Array*)(label_array_array_ptr->elements[i]), free_region_label);
    }
    free_array(label_array_array_ptr, NULL);
}

/* merge two word arrays into one that has no duplicates*/
int merge_word_arrays
(
    const Array *word_array1_ptr, 
    const Array *word_array2_ptr,
    Array       **merged_array_ptr_ptr
)
{
    int len1 = 0;
    int len2 = 0;
    int i;
    int len;
    Linkedlist *list_ptr = NULL;
    Word_sense *tmp_word_wp = NULL;
    Word_sense *word_wp = NULL;
    int count;

    if(word_array1_ptr == NULL || word_array2_ptr == NULL)
    {
        return NO_ERROR;
    }

    if(merged_array_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    len1 = word_array1_ptr->length;
    len2 = word_array2_ptr->length;

    len = len1 + len2;

    list_ptr = create_linkedlist();
    if(list_ptr == NULL)
    {
        return ERROR;
    }

    count = 0;
    for(i=0; i<len1; i++)
    {
        word_wp = (Word_sense*)word_array1_ptr->elements[i];
        if(search_linkedlist(list_ptr, compare_word_sense, word_wp) == NULL)
        {
            ERE(copy_word_sense(&tmp_word_wp, word_wp));
            tmp_word_wp->index = (count++);
            ERE(append_element_with_contents(list_ptr, tmp_word_wp));
            tmp_word_wp = NULL;
        }
    }

    for(i=0; i<len2; i++)
    {
        word_wp = (Word_sense*)word_array2_ptr->elements[i];
        if(search_linkedlist(list_ptr, compare_word_sense, word_wp) == NULL)
        {
            ERE(copy_word_sense(&tmp_word_wp, word_wp));
            tmp_word_wp->index = (count++);
            ERE(append_element_with_contents(list_ptr, tmp_word_wp));
            tmp_word_wp = NULL;
        }
    }

    ERE(from_list_to_array(list_ptr, merged_array_ptr_ptr));

    free_linkedlist(list_ptr, NULL);

    return NO_ERROR;
}

/* Replace the plurals in an array into theirs singulars. */
int replace_plurals_with_singulars
(
    Array *words_array_ptr
)
{
#ifdef KJB_HAVE_WN 
    int num_words;
    int i;
    char *morphed_word;
    Word_sense *word_wp = NULL;

    if(words_array_ptr == NULL)
    {
        return NO_ERROR;
    }

    num_words = words_array_ptr->length;
    for(i=0; i<num_words; i++)
    {
        word_wp = (Word_sense*)words_array_ptr->elements[i];
        morphed_word = morphword(word_wp->word, NOUN);
       if(morphed_word != NULL)
       {
            strcpy(word_wp->word, morphed_word);
       }
    }

    return NO_ERROR;
#else 
    set_dont_have_wn_error();
    return ERROR; 
#endif 
}

int ow_remove_synonyms_in_label
(
    Region_label *label_ptr
)
{
    int num_words;
    int word1, word2;
    Word_sense *word1_wp = NULL;
    Word_sense *word2_wp = NULL;
    Word_sense **words_wpp = NULL;
    int num_syns;
    int count;

    if(label_ptr == NULL)
    {
        return NO_ERROR;
    }
 
    num_words = label_ptr->length;
    if(num_words <= 1)
    {
        return NO_ERROR;
    }

    num_syns = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index == NOT_USED_INDEX ||
           strcmp(word1_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;

        for(word2=word1+1; word2<num_words; word2++)
        {
            word2_wp = label_ptr->words[word2];
            if(word2_wp->index == NOT_USED_INDEX ||
               strcmp(word2_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;

            if(is_synonym(word1_wp, word2_wp))
            {
                num_syns++;
                word2_wp->index = NOT_USED_INDEX;
            }
        }
    }
 
    if(num_syns == 0) return NO_ERROR;

    
    words_wpp = (Word_sense**)malloc(sizeof(Word_sense*)*(num_words - num_syns));
    if(words_wpp == NULL)
    {
        add_error("Failed to malloc memory!\n");
        return ERROR;
    }

    count = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index != NOT_USED_INDEX)
        {
            words_wpp[count] = NULL;
            ERE(copy_word_sense(&(words_wpp[count]), word1_wp)); 
            count++;
        } 
    }

    free_region_label_words_only(label_ptr);
    label_ptr->words = words_wpp;
    label_ptr->length -= num_syns;
    
    return NO_ERROR;
}

int ow_remove_synonyms_in_label_array
(
    Array *label_array_ptr
)
{
    int length;
    int i;

    if(label_array_ptr == NULL) return NO_ERROR;

    length = label_array_ptr->length;
    for(i=0; i<length; i++)
    {
        ERE(ow_remove_synonyms_in_label((Region_label*)label_array_ptr->elements[i]));
    }

    return NO_ERROR;
}

int ow_remove_synonyms_in_label_array_array
(
    Array *label_array_array_ptr
)
{
    int length;
    int i;

    if(label_array_array_ptr == NULL) return NO_ERROR;

    length = label_array_array_ptr->length;
    for(i=0; i<length; i++)
    {
        ERE(ow_remove_synonyms_in_label_array(
           (Array*)label_array_array_ptr->elements[i]));
    }

    return NO_ERROR;
}

int ow_remove_ancestors_in_label
(
    Region_label *label_ptr,
    int          parent_type
)
{
    int num_words;
    int word1, word2;
    Word_sense *word1_wp = NULL;
    Word_sense *word2_wp = NULL;
    Word_sense **words_wpp = NULL;
    int num_syns;
    int count;

    if(label_ptr == NULL)
    {
        return NO_ERROR;
    }
 
    num_words = label_ptr->length;
    if(num_words <= 1)
    {
        return NO_ERROR;
    }

    num_syns = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index == NOT_USED_INDEX ||
           strcmp(word1_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;

        for(word2=0; word2<num_words; word2++)
        {
            if(word1 == word2) continue;
            word2_wp = label_ptr->words[word2];
            if(word2_wp->index == NOT_USED_INDEX ||
               strcmp(word2_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;
            if(is_ancestor(word1_wp, word2_wp, parent_type))
            {
                num_syns++;
                word2_wp->index = NOT_USED_INDEX;
            }
        }
    }
 
    if(num_syns == 0) return NO_ERROR;

    
    words_wpp = (Word_sense**)malloc(sizeof(Word_sense*)*(num_words - num_syns));
    if(words_wpp == NULL)
    {
        add_error("Failed to malloc memory!\n");
        return ERROR;
    }

    count = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index != NOT_USED_INDEX)
        {
            words_wpp[count] = NULL;
            ERE(copy_word_sense(&(words_wpp[count]), word1_wp)); 
            count++;
        } 
    }

    free_region_label_words_only(label_ptr);
    label_ptr->words = words_wpp;
    label_ptr->length -= num_syns;
    
    return NO_ERROR;
}

int ow_remove_ancestors_in_label_array
(
    Array *label_array_ptr,
    int          parent_type
)
{
    int length;
    int i;

    if(label_array_ptr == NULL) return NO_ERROR;

   /* pso("**************************************\n");
    print_array(label_array_ptr, print_region_label);*/
    length = label_array_ptr->length;
    for(i=0; i<length; i++)
    {
        ERE(ow_remove_ancestors_in_label(
           (Region_label*)label_array_ptr->elements[i], parent_type));
    }
    /*print_array(label_array_ptr, print_region_label);
    pso("**************************************\n");*/

    return NO_ERROR;
}

/*int ow_remove_ancestors_in_label_array_array
(
    Array *label_array_array_ptr,
    int   parent_type
)
{
    int length;
    int i;

    if(label_array_array_ptr == NULL) return;

    length = label_array_array_ptr->length;
    for(i=0; i<length; i++)
    {
       pso("  Remove ancestors: %d\n", i);
        ERE(ow_remove_ancestors_in_label_array(
           (Array*)label_array_array_ptr->elements[i], parent_type));
    }

    return NO_ERROR;
}*/

int ow_remove_ancestors_in_label_array_array
(
    Array *label_array_array_ptr,
    int   parent_type
)
{
    int length;
    int i;
    Array *word_array_ptr = NULL;
    Array *tree_array_ptr = NULL;

    if(label_array_array_ptr == NULL) return NO_ERROR;

    ERE(get_vocabulary_from_labels(label_array_array_ptr, &word_array_ptr));
    ERE(index_word_array(word_array_ptr));
    ERE(construct_semantic_tree_for_vocabulary(word_array_ptr, word_array_ptr,
        &tree_array_ptr));

    length = label_array_array_ptr->length;
    for(i=0; i<length; i++)
    {
       pso("  Remove ancestors: %d\n", i);
        ERE(ow_remove_ancestors_in_label_array_1(
           (Array*)label_array_array_ptr->elements[i], word_array_ptr, tree_array_ptr, parent_type));
    }

    free_array(word_array_ptr, free);
    free_tree_array(tree_array_ptr);

    return NO_ERROR;
}

static int ow_remove_ancestors_in_label_array_1
(
    Array       *label_array_ptr,
    const Array *word_array_ptr,
    const Array *tree_array_ptr,
    int          parent_type
)
{
    int length;
    int i;

    if(label_array_ptr == NULL) return NO_ERROR;

    /*pso("**************************************\n");
    print_array(label_array_ptr, print_region_label);*/
    length = label_array_ptr->length;
    for(i=0; i<length; i++)
    {
        ERE(ow_remove_ancestors_in_label_1(
           (Region_label*)label_array_ptr->elements[i], word_array_ptr, tree_array_ptr, parent_type));
    }
    /*print_array(label_array_ptr, print_region_label);
    pso("**************************************\n");*/

    return NO_ERROR;
}

static int ow_remove_ancestors_in_label_1
(
    Region_label *label_ptr,
    const Array  *word_array_ptr,
    const Array  *tree_array_ptr,
    int          parent_type
)
{
    int num_words;
    int word1, word2;
    Word_sense *word1_wp = NULL;
    Word_sense *word2_wp = NULL;
    Word_sense **words_wpp = NULL;
    int num_syns;
    int count;
    int index;
    int res = NO_ERROR;

    if(label_ptr == NULL)
    {
        return NO_ERROR;
    }
 
    num_words = label_ptr->length;
    if(num_words <= 1)
    {
        return NO_ERROR;
    }

    num_syns = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index == NOT_USED_INDEX ||
           strcmp(word1_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;
        index = search_array(word_array_ptr, compare_word_sense, word1_wp);
        if(index < 0)
        {
            add_error("%s is not in the list!\n", word1_wp->word);
            res = ERROR;
            break;
        }

        for(word2=0; word2<num_words; word2++)
        {
            if(word1 == word2) continue;
            word2_wp = label_ptr->words[word2];
            if(word2_wp->index == NOT_USED_INDEX ||
               strcmp(word2_wp->word, UNKNOWN_LABEL_STR) == 0 ) continue;
            if(is_ancestor_1(word1_wp, index, word2_wp, tree_array_ptr, parent_type))
            {
                num_syns++;
                word2_wp->index = NOT_USED_INDEX;
            }
        }
    }
 
    if(res == ERROR) return ERROR;

    if(num_syns == 0) return NO_ERROR;

    
    words_wpp = (Word_sense**)malloc(sizeof(Word_sense*)*(num_words - num_syns));
    if(words_wpp == NULL)
    {
        add_error("Failed to malloc memory!\n");
        return ERROR;
    }

    count = 0;
    for(word1=0; word1<num_words; word1++)
    {
        word1_wp = label_ptr->words[word1];
        if(word1_wp->index != NOT_USED_INDEX)
        {
            words_wpp[count] = NULL;
            ERE(copy_word_sense(&(words_wpp[count]), word1_wp)); 
            count++;
        } 
    }

    free_region_label_words_only(label_ptr);
    label_ptr->words = words_wpp;
    label_ptr->length -= num_syns;
    
    return NO_ERROR;
}

static int is_ancestor_1
(
    const Word_sense *word1_wp, 
    int              index,
    const Word_sense *word2_wp,
    const Array      *tree_array_ptr,
    int              parent_type
)
{
    Tree *tree_ptr = NULL;

    tree_ptr = (Tree*)tree_array_ptr->elements[index];
    if(tree_ptr == NULL) return 0;

    if(search_tree(tree_ptr, compare_word_sense, word2_wp) != NULL)
    {
        return 1;
    }

    return 0;
}

int index_word_array
(
    const Array *array_ptr
)
{
    int i;
    int len;
    Word_sense *word_wp = NULL;

    if(array_ptr == NULL) return NO_ERROR;

    len = array_ptr->length;
    for(i=0; i<len; i++)
    {
        word_wp = (Word_sense*)array_ptr->elements[i];
        word_wp->index = i;
    }

    return NO_ERROR;
}

#ifdef __cplusplus
}
#endif
