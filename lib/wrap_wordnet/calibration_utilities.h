#ifndef KJB_CALIBRATION_UTILITIES
#define KJB_CALIBRATION_UTILITIES

#include <l/l_incl.h>
#include <m/m_vector.h>
#include <wrap_wordnet/wn_array.h>
#include <wrap_wordnet/wn_word_sense.h>

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int my_round (double number);
 
int find_bin(double *bin_number, double ivalue, int num_bins, double imin, double imax, int check_boundaries);

int read_min_max(double *imin, double *imax, char *filename);

int read_padding(int * x_padding, int * y_padding, char * path_to_padding);

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
);

void print_detector_information
(
	const char *** detector_lists,
	const V_v_v * calibrations,
	const Array * words
);

int is_file_for_detector(const char * detector_file, const char * detector_word);

void free_detector_lists(char *** detector_lists, const V_v_v * calibrations);

int read_image_list
(
    char *** image_list,
    int * num_images,
    char * path
);

int write_image_list
(
    const char ** image_list,
    int num_images,
    char * path
);

int read_detector_words(const char * path_to_detector_list, Array ** detectors);

int read_detector_information
(
	const char * path_to_detector_list,
	const char * path_to_detector_dir,
    const char * path_to_min_max,
	const Array * detectors,
	V_v_v ** calibrations,
	char **** detector_lists,
    Vector_vector ** min_maxs
);

int read_detector_list
(
	const char * path_to_detector_list,
	char *** detector_list,
	int *num_detectors
);

int read_response_to_backgrounds
(
    const char * path_to_detector_list,
    const char * min_max_dir,
    const char * background_filename,
    const char * min_max_filename,
    Vector ** calibration,
    Vector ** min_max
);

int free_image_list(char ** image_list, int num_lists);

#ifdef MAC_OSX
/* PASTE REMAINDER AT BOTTOM OF FILE */
ssize_t getline(char **linep, size_t *np, FILE *stream);
#endif

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif/* KJB_CALIBRATION_UTILITIES */
