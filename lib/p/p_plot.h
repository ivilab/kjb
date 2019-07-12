
/* $Id: p_plot.h 7576 2010-11-21 06:05:51Z kobus $ */

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

#ifndef P_PLOT_INCLUDED
#define P_PLOT_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define CURRENT_PLOT             (-1000)
#define PLOT_USE_DEFAULT_RANGE   (-1e30)

/* -------------------------------------------------------------------------- */

int set_colour_plot(void);
int set_monochrome_plot(void);
int set_max_num_plots(int max_num_plots);
int push_display_plot_flag(int flag);
int pop_display_plot_flag(void);
int plot_open(void);
int plot_open3(void);

int plot_set_attached_data
(
    int   plot_id,
    void* data_ptr,
    void  (*data_disposal_fn) (void *)
);

int plot_get_attached_data(int plot_id, void** data_ptr_ptr);

int special_plot_open
(
    int x_size,
    int y_size,
    int tlc_x,
    int tlc_y,
    int dim
);

int save_plot(int plot_id, const char* file_name);
int save_plot_as_pbm(int plot_id, const char* file_name);
int save_plot_dir(int plot_id, const char* dir_name);

int send_command_to_plot(int plot_id, const char* command);

int plot_set_title
(
    int         plot_id,
    const char* title,
    int         x_offset,
    int         y_offset
);

int plot_set_x_legend(int plot_id, const char* legend);
int plot_set_y_legend(int plot_id, const char* legend);

int plot_add_label(int plot_id, const char* label, double x, double y);
int plot_add_label_2(int plot_id, const char* label, double x, double y);

int plot_set_range
(
    int    plot_id,
    double x_min,
    double x_max,
    double y_min,
    double y_max
);

int plot_get_range
(
    int     plot_id,
    double* x_min_ptr,
    double* x_max_ptr,
    double* y_min_ptr,
    double* y_max_ptr
);

int plot_set_range3
(
    int    plot_id,
    double x_min,
    double x_max,
    double y_min,
    double y_max,
    double z_min,
    double z_max
);

int plot_function_string
(
    int    plot_id,
#ifdef DEF_OUT
    double x_min,
    double x_max,
    double y_min,
    double y_max,
#endif
    const char* function_string,
    int         width, 
    const char* name
);


int plot_selected_multiple_histograms
(
    int                  plot_id,
    const Vector_vector* vvp,
    int                  num_bins,
    double               sigma,
    const Word_list*     names_ptr,
    const Int_vector*    enable_vp, 
    double*              bin_size_ptr
);

int plot_multiple_histograms
(
    int                  plot_id,
    const Vector_vector* vvp,
    int                  num_bins,
    double               sigma,
    const Word_list*     names_ptr,
    double*              bin_size_ptr
);

int plot_histogram
(
    int           plot_id,
    const Vector* vp,
    int           num_bins,
    double        sigma,
    const char*   name
);

int plot_multiple_bars
(
    int                  plot_id,
    const Vector_vector* y_vvp,
    double      offset,
    double      step,
    const Word_list*     names_ptr
);

int plot_multiple_bars_2
(
    int                  plot_id,
    const Vector_vector* x_vvp,
    const Vector_vector* y_vvp,
    double               bar_width,
    const Word_list*     names_ptr
);

int plot_bars
(
    int           plot_id,
    const Vector* y_vp,
    double        offset,
    double        step,
    const char*   label_str
);

int plot_bars_2
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    double        bar_width,
    double        stripe_width,
    const char*   label_str
);

int plot_vector
(
    int           plot_id,
    const Vector* vp,
    double        x_offset,
    double        x_step,
    const char*   name
);

int plot_point_list(int plot_id, const Matrix* point_mp, char** names);

int plot_vector_point
(
    int           plot_id,
    const Vector* point_vp,
    const char*   name
);

int plot_point(int plot_id, double x, double y, const char* name);

int plot_matrix_row_points
(
    int           plot_id,
    const Matrix* mp,
    const char*   name
);

int plot_points
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
);

int plot_curve
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
);

int plot_line
(
    int         plot_id,
    double      x1,
    double      y1,
    double      x2,
    double      y2,
    const char* name
);

int plot_multi_segment_curve
(
    int                  plot_id,
    const Matrix_vector* segments,
    const char*          name
);

int plot_matrix_vector_list_cols
(
    int             plot_id,
    int             list_len,
    Matrix_vector** matrix_vector_list,
    const char*     name
);

int plot_matrix_vector_cols
(
    int                  plot_id,
    const Matrix_vector* matrix_vector,
    const char*          name
);

int plot_segments
(
    int           plot_id,
    const Vector* x1_vp,
    const Vector* y1_vp,
    const Vector* x2_vp,
    const Vector* y2_vp,
    const char*   name
);

int plot_matrix_cols
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char**  name_list,
    const int*    line_types
);

int plot_matrix_rows
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char**  name_list,
    const int*    line_types
);

int plot_multi_matrix_rows
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char*   name
);

int plot_matrix_values
(
    int           plot_id,
    const Matrix* mp
);

int plot_matrix_values_2
(
    int           plot_id,
    const Matrix* mp,
    double        x_min,
    double        x_max,
    double        y_min,
    double        y_max
);

int plot_update(int plot_id);
int plot_clear(int plot_id);
int plot_close(int plot_id);
void plot_close_all(void);

int plot_write(int plot_id, const char* buff);

int plot3_points
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
);

int plot3_curve
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

