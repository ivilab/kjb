
/* $Id: calibration_utilities.c 10666 2011-09-29 19:52:00Z predoehl $ */

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

#include <wrap_wordnet/calibration_utilities.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DETECTORS_IS_MORPHED 0
#define DETECTORS_IS_SENSED 1
/* =============================================================================
 *                           find_bin
 *
 * Given min-max values, the number of bins, and a value, this function returns
 * the bin the value falls in (from 0 to n_bins-1). If the value is smaller
 * than the min (larger than the max), different actions are taken according to the value
 * of check_boundaries. If this is true, an error is returned, otherwise the value
 * is considered to be in the first (last) bin.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * -----------------------------------------------------------------------------
*/
int find_bin(double *bin_number, double ivalue, int num_bins, double imin, double imax, int check_boundaries)
{
	double step_size = (imax-imin)/(num_bins);
	if(imax <= imin)
	{
		add_error("find bin, max <= min");
		return ERROR;
	}

	if(ivalue < imin)
	{
		if(check_boundaries)
		{
			add_error("find bin, value < min");
			return ERROR;
		}
		(*bin_number) = 0;
	}
	else if(ivalue < imax)
	{
        ivalue -= imin;
        (*bin_number) = (ivalue/step_size);

	}
	else
	{
		if(check_boundaries)
		{
			add_error("find bin, value > max");
			return ERROR;
		}
		(*bin_number) = num_bins - 1;
	}

    if((*bin_number) >= num_bins)
    {
        return ERROR;
    }

	return NO_ERROR;
}

int my_round(double number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

/* =============================================================================
 *                           read_min_max
 * Reads a min and a max value (double) from a file. This routine expects that
 * the values are stored one per line
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * -----------------------------------------------------------------------------
*/
int read_min_max(double *imin, double *imax, char *filename)
{
    char * line = NULL;
    int nbytes = 0;
    FILE * fp = NULL;

    printf("The min max file is %s\r\n", filename);
    line = (char *) malloc((MAX_FILE_NAME_SIZE*2) + 100);
    if(line == NULL)
    {
        add_error("Read min-max, could not allocate memory");
        return ERROR;
    }
    fp = kjb_fopen(filename,"r");
    if(fp == NULL)
    {
        add_error("Min-max file does not exist");
        return ERROR;
    }
    if(getline(&line, &nbytes, fp) > 0)
    {
        (*imin) = atof(line);
    }
    else
    {
        add_error("Could not read min from min-max file");
        free(line);
        kjb_fclose(fp);
        return ERROR;
    }
    if(getline(&line, &nbytes, fp) > 0)
    {
        (*imax) = atof(line);
    }
    else
    {
        add_error("Could not read max from min-max file");
        free(line);
        kjb_fclose(fp);
        return ERROR;
    }
    free(line);
    kjb_fclose(fp);

    return NO_ERROR;
}

/* =============================================================================
 *                           read_padding
 * Reads the x padding and the y padding from a file. This routine expects that
 * the values are stored one per line
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * -----------------------------------------------------------------------------
*/
int read_padding(int * x_padding, int * y_padding, char * path_to_padding)
{
    FILE * fp = NULL;
    Int_vector * pads = NULL;

    fp = kjb_fopen(path_to_padding,"r");
    if(fp == NULL)
    {
    	add_error("Padding file does not exist");
    	return ERROR;
    }

    if(fp_read_int_vector(&pads, fp) != NO_ERROR)
    {
    	add_error("Padding file is corrupted");
    	kjb_fclose(fp);
    	return ERROR;
    }

    if( (pads->length != 2) || (pads->elements[0] < 0) || (pads->elements[1] < 0))
    {
    	add_error("Padding file is corrupted");
		kjb_fclose(fp);
		free_int_vector(pads);
		return ERROR;
    }

    (*x_padding) = pads->elements[0];
    (*y_padding) = pads->elements[1];

    free_int_vector(pads);
    kjb_fclose(fp);
	return NO_ERROR;
}

int map_pixel_to_grid
(
	int * grid_row,
	int * grid_col,
	int p_row,
	int p_col,
	int num_grid_rows,
	int num_grid_cols,
	int cell_width,
	int cell_height
)
{
    (*grid_row) = p_row/cell_height;
    (*grid_col) = p_col/cell_width;
    if((*grid_row) >= num_grid_rows)
    {
    	return ERROR;
    }
    if((*grid_col) >= num_grid_cols)
    {
	    return ERROR;
    }

    return NO_ERROR;
}

void print_detector_information
(
	const char *** detector_lists,
	const V_v_v * calibrations,
	const Array * words
)
{
	int i = 0;
	int j = 0;
	int k = 0;
	Word_sense * ws = NULL;
	printf("DETECTOR INFORMATION\r\n");

	if(calibrations != NULL)
	{
		for(i = 0; i < calibrations->length; i++)
		{
			ws = (Word_sense *)words->elements[i];
			printf("Files for word %s\r\n", ws->word);
			if(calibrations->elements[i] != NULL)
			{
				for(j = 0; j < calibrations->elements[i]->length; j++)
				{
					printf("FILE: %s\r\n", detector_lists[i][j]);
					printf("CALIBRATION:");
					for(k = 0; k < calibrations->elements[i]->elements[j]->length; k++)
					{
						printf("%f ", calibrations->elements[i]->elements[j]->elements[k]);
					}
					printf("\r\n");
				}
			}
		}
	}
}

int is_file_for_detector(const char * detector_file, const char * detector_word)
{
	char * detector_name = NULL;
	char * copy = NULL;
	int result = 0;
	copy = (char *)malloc(sizeof(char) * (strlen(detector_file) + 1) );
	if(!copy)
	{
		add_error("Is file for detector, could not allocate enough memory");
		return result;
	}
	strcpy(copy, detector_file);
	detector_name = strtok(copy,"_");
	if(detector_name != NULL)
	{
		result = !strncmp(detector_name, detector_word, strlen(detector_name));
	}

	free(copy);
	return result;
}

void free_detector_lists(char *** detector_lists, const V_v_v * calibrations)
{
    int i = 0;
    int j = 0;

    if(calibrations != NULL)
    {
        for(i = 0; i < calibrations->length; i++)
        {
        	if(calibrations->elements[i] != NULL)
        	{
        		for(j = 0; j < calibrations->elements[i]->length; j++)
        		{
        			free(detector_lists[i][j]);
        		}
        	}
        	free(detector_lists[i]);
        }
    }
    free(detector_lists);
}


int read_image_list
(
    char *** image_list,
    int * num_images,
    char * path
)
{
    FILE * data_fp = NULL;
	char * line = NULL;
	int nbytes = 0;

	(*num_images) = 0;
	data_fp = kjb_fopen(path,"r");
	if(!data_fp)
	{
		add_error("Add object detectors, could not read image list");
		return ERROR;
	}

	line = (char *) malloc((MAX_FILE_NAME_SIZE*2) + 100);
	if(line == NULL)
	{
		kjb_fclose(data_fp);
		add_error("Add object detectors, read image list, could not allocate memory for line buffer");
		return ERROR;
	}

	while(getline(&line, &nbytes, data_fp) > 0)
	{
		if((*num_images) == 0)
		{
			(*image_list) = (char**)malloc(sizeof(char *));
		}
		else
		{
			(*image_list) = (char**)realloc((*image_list), sizeof(char *)*((*num_images) + 1) );
		}
		if( (!image_list) )
		{
			kjb_fclose(data_fp);
			free(line);
			add_error("Read image list, could not allocate memory for detector list");
			return ERROR;
		}
		(*image_list)[*num_images] = (char *)malloc(sizeof(char)*(strlen(line)));
		if( !(*image_list)[*num_images] )
		{
			kjb_fclose(data_fp);
			free(line);
			add_error("Read image list, could not allocate memory for detector list");
			return ERROR;
		}
		(*num_images)++;
		strncpy((*image_list)[(*num_images) -1], line, strlen(line) - 1);
		((*image_list)[(*num_images) -1])[strlen(line) - 1] = '\0';
	}


	free(line);
	kjb_fclose(data_fp);
	return NO_ERROR;
}


int read_detector_list
(
	const char * path_to_detector_list,
	char *** detector_list,
	int *num_detectors
)
{
	FILE * data_fp = NULL;
	char * line = NULL;
	int nbytes = 0;

	(*num_detectors) = 0;
	data_fp = kjb_fopen(path_to_detector_list,"r");
	if(!data_fp)
	{
		add_error("Add object detectors, could not read detector list");
		return ERROR;
	}

	line = (char *) malloc((MAX_FILE_NAME_SIZE*2) + 100);
	if(line == NULL)
	{
		kjb_fclose(data_fp);
		add_error("Add object detectors, read detector list, could not allocate memory for line buffer");
		return ERROR;
	}

	while(getline(&line, &nbytes, data_fp) > 0)
	{
		if((*num_detectors) == 0)
		{
			(*detector_list) = (char**)malloc(sizeof(char *));
		}
		else
		{
			(*detector_list) = (char**)realloc((*detector_list), sizeof(char *)*((*num_detectors) + 1) );
		}
		if( (!detector_list) )
		{
			kjb_fclose(data_fp);
			free(line);
			add_error("Add object detectors, read detector list, could not allocate memory for detector list");
			return ERROR;
		}
		(*detector_list)[*num_detectors] = (char *)malloc(sizeof(char)*(strlen(line)));
		if( !(*detector_list)[*num_detectors] )
		{
			kjb_fclose(data_fp);
			free(line);
			add_error("Add object detectors, read detector list, could not allocate memory for detector list");
			return ERROR;
		}
		(*num_detectors)++;
		strncpy((*detector_list)[(*num_detectors) -1], line, strlen(line) - 1);
		((*detector_list)[(*num_detectors) -1])[strlen(line) - 1] = '\0';
	}


	free(line);
	kjb_fclose(data_fp);
	return NO_ERROR;
}

int read_detector_words(const char * path_to_detector_words, Array ** detectors)
{
	if(read_words_to_array_2
	(
	    path_to_detector_words,
	    DETECTORS_IS_MORPHED,
	    DETECTORS_IS_SENSED,
	    detectors
	) != NO_ERROR )
	{
	    add_error("Could not read detector words");
	    return ERROR;
	}
	return NO_ERROR;
}

int read_detector_information
(
	const char * path_to_detector_list,
	const char * path_to_detector_dir,
	const char * path_to_min_max,
    const Array * detectors,
	V_v_v ** calibrations,
	char **** detector_lists,
    Vector_vector ** min_maxs
)
{
	Int_vector * counts = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	char ** detector_files = 0;
	int num_files;
	Word_sense * ws = 0;
	char detector_file_name[MAX_FILE_NAME_SIZE];
	char min_max_file_name[MAX_FILE_NAME_SIZE];
	FILE * fp = 0;

	ERE(get_target_int_vector(&counts, detectors->length));
	if(get_target_v3(calibrations, detectors->length) != NO_ERROR)
	{
		add_error("Read detector information, could not allocate calibration vectors");
		free_int_vector(counts);
		return ERROR;
	}

    if(get_target_vector_vector(min_maxs, detectors->length) != NO_ERROR)
    {
        add_error("Read detector information, could not allocate min max vector vector");
        free_int_vector(counts);
        free_v3(*calibrations);
        return ERROR;
    }

    for(i = 0; i < detectors->length; i++)
    {
        (*min_maxs)->elements[i] = NULL;
    }


    for(i = 0; i < detectors->length; i++)
    {
        if(get_zero_vector(&((*min_maxs)->elements[i]),2) != NO_ERROR)
        {
            add_error("Read detector information, could not allocate min max vector vector");
            free_int_vector(counts);
            free_v3(*calibrations);
            free_vector_vector(*min_maxs);
        }
        ws = (Word_sense *)detectors->elements[i];
        sprintf(min_max_file_name,"%s/%s",path_to_min_max,ws->word);
        if(read_min_max(&((*min_maxs)->elements[i]->elements[0]),&((*min_maxs)->elements[i]->elements[1]), min_max_file_name) != NO_ERROR)
        {
            add_error("Read detector information, could not read min max files");
            free_int_vector(counts);
            free_v3(*calibrations);
            free_vector_vector(*min_maxs);
        }
    }

	if( read_detector_list
	(
	    path_to_detector_list,
	    &detector_files,
	    &num_files
	) != NO_ERROR)
	{
		free_int_vector(counts);
		free_v3(*calibrations);
        free_vector_vector(*min_maxs);
		return ERROR;
	}


	for(i = 0; i < detectors->length; i++)
	{
		counts->elements[i] = 0;
		for(j = 0; j < num_files; j++)
		{
			ws = (Word_sense *)detectors->elements[i];
			if( is_file_for_detector(detector_files[j], ws->word) )
			{
                counts->elements[i]++;
			}
		}
	}

	(*detector_lists) = (char ***)malloc(sizeof(char **)*detectors->length);
	if(!(*detector_lists))
	{
		add_error("Read detector information, could not allocate memory for detector lists");
		free_int_vector(counts);
		free_v3(*calibrations);
        free_vector_vector(*min_maxs);
		for(i = 0; i < num_files; i++)
		{
			free(detector_files[i]);
		}
		free(detector_files);
		return ERROR;
	}

	for(i = 0; i < detectors->length; i++)
	{
		(*detector_lists)[i] = (char **)malloc(sizeof(char *)*(counts->elements[i]));
		if(!(*detector_lists))
		{
			add_error("Read detector information, could not allocate memory for detector lists");
			free_int_vector(counts);
            free_vector_vector(*min_maxs);
			free_detector_lists(*detector_lists, *calibrations);
			free_v3(*calibrations);
			for(i = 0; i < num_files; i++)
			{
				free(detector_files[i]);
			}
			free(detector_files);
			return ERROR;
		}
		for(k = 0; k < counts->elements[i]; k++)
		{
			((*detector_lists)[i])[k] = NULL;
		}
	    if( get_target_vector_vector( &((*calibrations)->elements[i]), counts->elements[i]) != NO_ERROR)
	    {
            add_error("Read detector information, could not allocate calibration vectors");
            free_detector_lists(*detector_lists, *calibrations);
            free_vector_vector(*min_maxs);
            free_v3(*calibrations);
            for(i = 0; i < num_files; i++)
			{
				free(detector_files[i]);
			}
            free(detector_files);
            return ERROR;
	    }
		for(j = 0; j < num_files; j++)
		{
			ws = (Word_sense *)detectors->elements[i];
			if( is_file_for_detector(detector_files[j], ws->word) )
			{
				sprintf(detector_file_name, "%s/%s", path_to_detector_dir, detector_files[j]);
				for(k = 0; k < counts->elements[i]; k ++)
				{
					if(((*calibrations)->elements[i])->elements[k] == NULL)
					{
						break;
					}
				}

				if(k == counts->elements[i])
				{
					add_error("Memory corruption while reading detector calibrations");
					for(i = 0; i < num_files; i++)
					{
						free(detector_files[i]);
					}

					free(detector_files);
					free_detector_lists(*detector_lists, *calibrations);
					free_v3(*calibrations);
					free_int_vector(counts);
                    free_vector_vector(*min_maxs);
					return ERROR;
				}

				fp = kjb_fopen(detector_file_name, "r");

				if(fp == NULL)
				{
					printf("%s is the culprit\r\n", detector_file_name);
					add_error("Could not open one of the calibration files");
					for(i = 0; i < num_files; i++)
					{
						free(detector_files[i]);
					}

					free(detector_files);
					free_detector_lists(*detector_lists, *calibrations);
					free_v3(*calibrations);
					free_int_vector(counts);
                    free_vector_vector(*min_maxs);
					return ERROR;
				}

				if( (fp_read_vector( &(((*calibrations)->elements[i])->elements[k]), fp))  != NO_ERROR)
				{
					add_error("Could not read one of the calibration files");
					for(i = 0; i < num_files; i++)
					{
						free(detector_files[i]);
					}

					free(detector_files);
					free_detector_lists(*detector_lists, *calibrations);
					free_v3(*calibrations);
					free_int_vector(counts);
                    free_vector_vector(*min_maxs);
					kjb_fclose(fp);
					return ERROR;
				}

				kjb_fclose(fp);

				for(k = 0; k < counts->elements[i]; k++)
				{
					if( (((*detector_lists)[i])[k]) == NULL)
					{
						break;
					}
				}

				if(k == counts->elements[i])
				{
					add_error("Memory corruption while allocating detector lists");
					for(i = 0; i < num_files; i++)
					{
						free(detector_files[i]);
					}

					free(detector_files);
					free_detector_lists(*detector_lists, *calibrations);
					free_v3(*calibrations);
					free_int_vector(counts);
                    free_vector_vector(*min_maxs);
					return ERROR;
				}

				(((*detector_lists)[i])[k]) = (char *)malloc(sizeof(char)*(strlen(detector_files[j]) +1));
				if((((*detector_lists)[i])[k]) == NULL)
				{
					add_error("Memory corruption while allocating detector lists");
					for(i = 0; i < num_files; i++)
					{
						free(detector_files[i]);
					}

					free(detector_files);
					free_detector_lists(*detector_lists, *calibrations);
					free_v3(*calibrations);
					free_int_vector(counts);
                    free_vector_vector(*min_maxs);
					return ERROR;
				}
				strcpy((((*detector_lists)[i])[k]),detector_files[j]);

			}
		}
	}



	for(i = 0; i < detectors->length; i++)
	{
		ws = (Word_sense *)detectors->elements[i];
		printf("Num detectors for word %s is: %d\r\n", ws->word, counts->elements[i] );
	}


	for(i = 0; i < num_files; i++)
	{
	    free(detector_files[i]);
	}

    free(detector_files);
	free_int_vector(counts);
	return NO_ERROR;
}

int read_response_to_backgrounds
(
    const char * path_to_detector_list,
    const char * min_max_dir,
    const char * background_filename,
    const char * min_max_filename,
    Vector ** calibration,
    Vector ** min_max
)
{
    char path_to_detector[MAX_FILE_NAME_SIZE];
    char path_to_min_max[MAX_FILE_NAME_SIZE];

    sprintf(path_to_detector,"%s/%s", path_to_detector_list, background_filename);
    sprintf(path_to_min_max, "%s/%s", min_max_dir, min_max_filename);

    ERE(read_vector(calibration, path_to_detector));

    if(get_target_vector(min_max, 2) != NO_ERROR)
    {
    	free_vector(*calibration);
    	return ERROR;
    }

    if(read_min_max(&(*min_max)->elements[0], &(*min_max)->elements[1], path_to_min_max) != NO_ERROR)
    {
        free_vector(*calibration);
        free_vector(*min_max);
        return ERROR;
    }

    return NO_ERROR;
}

int free_image_list(char ** image_list, int num_lists)
{
	int i;
	for(i = 0; i < num_lists; i++)
	{
		free(image_list[i]);
		image_list[i] = 0;
	}

	free(image_list);
	image_list = 0;
}

int write_image_list
(
    const char ** image_list,
    int num_images,
    char * path
)
{
	FILE * fp = 0;
	int i = 0;

	fp = kjb_fopen(path,"w");
	if(fp == 0)
	{
		add_error("Could not open file for writing image list");
		return ERROR;
	}

	for(i = 0; i < num_images; i++)
	{
		fprintf(fp,"%s\n",image_list[i]);
	}

	return ERROR;

}

#ifdef MAC_OSX
/* PASTE REMAINDER AT BOTTOM OF FILE */
ssize_t getline(char **linep, size_t *np, FILE *stream)
{
  char *p = NULL;
  size_t i = 0;
  int ch;

  if (!linep || !np) {
    errno = EINVAL;
    return -1;
  }

  if (!(*linep) || !(*np)) {
    *np = 120;
    *linep = (char *)malloc(*np);
    if (!(*linep)) {
      return -1;
    }
  }

  flockfile(stream);

  p = *linep;
  for (ch = 0; (ch = getc_unlocked(stream)) != EOF;) {
    if (i > *np) {
      /* Grow *linep. */
      size_t m = *np * 2;
      char *s = (char *)realloc(*linep, m);

      if (!s) {
        int error = errno;
        funlockfile(stream);
        errno = error;
        return -1;
      }

      *linep = s;
      *np = m;
    }

    p[i] = ch;
    if ('\n' == ch) break;
    i += 1;
  }
  funlockfile(stream);

  /* Null-terminate the string. */
  if (i > *np) {
    /* Grow *linep. */
      size_t m = *np * 2;
      char *s = (char *)realloc(*linep, m);

      if (!s) {
        return -1;
      }

      *linep = s;
      *np = m;
  }

  p[i + 1] = '\0';
  return ((i > 0)? i : -1);
}
#endif

#ifdef __cplusplus
}
#endif
