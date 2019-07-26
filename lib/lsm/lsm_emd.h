
/* $Id: lsm_emd.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef LSM_EMD_H
#define LSM_EMD_H


#include "s/s_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
    emd_lib.h

    Copyright (C) 1998 Yossi Rubner
    Computer Science Department, Stanford University
    E-Mail: rubner@cs.stanford.edu
    URL: http://vision.stanford.edu/~rubner

*/

/* Definition of limits for optimization */
/* #define EMD_CHECK_LEAKS    1 */
#define EMD_MAX_ITERATIONS 500000
#define EMD_INFINITY       1e20
#define EMD_EPSILON        1e-6
#define EMD_2D             2
#define EMD_3D             3

/* Modification Log:

   Feb 25
   - Change all references to Feature data type to a Vector
     "Feature* feature_ptr" becomes "Vector* feature_vp"

   - Change Signature definition for the array of features
     Old: Feature* feature_vec (an array of features)
     New: Vector** feature_vec (an array of pointers to Vectors)
*/

typedef struct Signature
{
    int       num_features; /* Number of features in the signature */
    Vector**  feature_vec;  /* Pointer to the vector of features   */
    double*     weights_vec;  /* Pointer to the feature weights      */
} Signature;


typedef struct Flow
{
    int   from_feature;    /* Feature number in signature 1       */
    int   to_feature;      /* Feature number in signature 2       */
    double  flow_amount;     /* Amount of flow                      */
} Flow;

typedef enum Emd_cluster_method
{
    EMD_NO_CLUSTERING,
    EMD_2D_HISTOGRAM,
    EMD_3D_HISTOGRAM,
    EMD_K_MEANS,
    EMD_CLARANS,
    EMD_CLUSTER_METHOD_NOT_SET = NOT_SET,
    EMD_CLUSTER_METHOD_ERROR = ERROR
} Emd_cluster_method;

typedef enum Emd_data_origin
{
    EMD_FROM_RGB,
    EMD_FROM_SPECTRA,
    EMD_DATA_ORIGIN_NOT_SET = NOT_SET,
    EMD_DATA_ORIGIN_ERROR = ERROR
} Emd_data_origin;

typedef union Emd_illum_data
{
    Matrix*  illum_mp;
    Spectra* illum_sp;
} Emd_illum_data;

typedef struct Signature_db
{
    int                num_signatures;   /* Number of signatures in database */
    Signature**        signature_ptr_vec; /* Array of pointers to signatures */
    Emd_cluster_method cluster_method;    /* Method used to make signatures  */
    Emd_data_origin    data_origin;       /* Illuminant origin (spectra/RGB) */
    Emd_illum_data     illum_data;        /* Either Spectra or RGB Matrix    */
} Signature_db;

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int get_earthmover_distance
(
    Signature* sig1,
    Signature* sig2,
    int        (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* dist_ptr),
    Flow*      flow_vec,
    int*       num_flows_ptr,
    double*    em_distance_ptr
);

int euclidean_distance(Vector* v1_vp, Vector* v2_vp, double* dist_ptr);

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int get_target_signature
(
    Signature** target_sig_ptr_ptr,
    const int   num_features
);

void free_signature(Signature* signature_ptr);

int copy_signature
(
    Signature**      output_sig_ptr_ptr,
    const Signature* source_sig_ptr
);

int set_signature_features
(
    Signature*    target_sig_ptr,
    const Matrix* feature_data_mp
);

int set_signature_weights
(
    Signature*    target_sig_ptr,
    const Vector* weight_data_vp
);

int put_matrix_data_into_signature
(
    Signature**   target_sig_ptr_ptr,
    const Matrix* source_sig_data_mp
);

int put_signature_data_into_matrix
(
    Matrix**         target_sig_data_mpp,
    const Signature* source_sig_ptr
);

int get_signature_from_RGB
(
    Signature**        output_sig_ptr_ptr,
    int                num_features,
    Matrix*            data_mp,
    int                (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* distance_ptr),
    Emd_cluster_method cluster_method
);

int get_clustered_data
(
    Emd_cluster_method cluster_method,
    Matrix*            input_data_mp,
    int                num_clusters,
    int                (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* distance_ptr),
    Matrix**           clusters_mpp,
    Vector**           weights_vpp
);

int convert_spectrum_to_signature
(
    Signature** target_sig_ptr_ptr,
    Spectra*    source_sp,
    int         spectrum_index
);

void verbose_psig(int cut_off, Signature* sig_ptr);


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int get_target_signature_db
(
    Signature_db** target_sig_db_ptr_ptr,
    const int      num_signatures
);

void free_signature_db(Signature_db* sig_db_ptr);

int copy_signature_db
(
    Signature_db**      target_sig_db_ptr_ptr,
    const Signature_db* source_sig_db_ptr
);

int add_signature_to_db
(
    Signature_db*    sig_db_to_update_ptr,
    const int        sig_index,
    const Signature* sig_to_add_ptr
);

int get_signature_from_db
(
    const Signature_db* sig_db_to_query_ptr,
    const int           sig_index,
    Signature**         output_sig_ptr_ptr
);

int set_signature_db_illum_data
(
    Signature_db*   target_sig_db_ptr,
    Emd_data_origin data_origin,
    Emd_illum_data* illum_data_ptr
);

int get_signature_db_illum_data
(
    const Signature_db* source_sig_db_ptr,
    Emd_data_origin*    illum_data_origin_ptr,
    Emd_illum_data*     illum_data_ptr
);

int read_signature_db
(
    Signature_db** target_sig_db_ptr_ptr,
    const char*    file_name,
    const char*    error_msg
);

int write_signature_db
(
    const char*         file_name,
    const Signature_db* source_sig_db_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

