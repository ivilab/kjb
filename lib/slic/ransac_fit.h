/* $Id: ransac_fit.h 21596 2017-07-30 23:33:36Z kobus $
 */
#ifndef SLIC_RANSAC_DEFINED_H_
#define SLIC_RANSAC_DEFINED_H_

#include "m/m_incl.h"
#include "n/n_incl.h" 
#include "i/i_incl.h" 
#include "i/i_float.h" 

#include "slic/affine.h"
#include "slic/similarity.h"
#include "slic/homography.h"
#include "slic/fundamental_matrix.h"
#include "slic/image_interp.h"


#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#define DEFAULT_MINIMUM_PERCENTAGE_OF_INLIERS 0.2

/*
static int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *);
static int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **);
static int (*degen_func)(const Matrix *);
*/

int set_ransac_options(const char* option, const char* value);

int ransac_fit
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
);

int ransac_fit_basic
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const int          min_num_samples,
    const double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
);


int ransac_fit_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int          *num_inliers_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
);

/* int ransac_fit */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int          max_num_tries, */
/*     int          min_num_samples, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ); */

int ransac_fit_constrained
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_affine
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_constrained_affine
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_constrained_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_constrained_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int ransac_fit_fundamental_matrix
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int iterative_fit
(
    const Matrix *x_mp,
    const Matrix *y_mp,
   /* int (*validate_constraints)(const Matrix *),*/
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
);

int get_inliers
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double       good_fit_threshold,
    Int_vector   *index_ivp,
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **)
);

int get_unique_inliers
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double       good_fit_threshold,
    Int_vector   *index_ivp,
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **)
);

int get_random_samples
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          m,
    Matrix       **rand_x_mpp,
    Matrix       **rand_y_mpp,
    int (*degen_func)(const Matrix *)    
);

int get_fitting_data
(
    const Matrix     *x_mp,
    const Matrix     *y_mp,
    const Int_vector *index_ivp,
    Matrix       **fit_x_mpp,
    Matrix       **fit_y_mpp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
