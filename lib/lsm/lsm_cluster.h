
/* $Id: lsm_cluster.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef LSM_CLUSTER_H
#define LSM_CLUSTER_H


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
// NOTE: Repetative use of the phrase "kmeans" is intentional. The module may
//       turn into "cluster_lib.h" with more than one clustering algorithm
//       implemented. Then again, it may not.
*/

/* Definition of limits for k-means clustering algorithm                 */
#define KMEANS_DEFAULT_MAX_ITERATIONS 50
#define KMEANS_DEFAULT_EPSILON        ((double)1e-3)
#define KMEANS_DEBUG_VERBOSE_LEVEL    500

/* 3D Histogram Clustering bin resolution                                */
#define CLUSTER_HISTOGRAM_NUM_BINS     20

/* GLOBAL CLUSTER ALGORITHM FUNCTIONS -----------------------------------*/
int set_clustering_options(char* option, char* value);

/* K-MEANS CLUSTERING FUNCTIONS -----------------------------------------*/

int set_kmeans_max_iterations(const int new_max_iterations);
int get_kmeans_max_iterations(void);
int set_kmeans_epsilon(const double new_epsilon);
double get_kmeans_epsilon(void);

int get_kmeans_clusters
(
    const Matrix* data_mp,
    int           num_clusters,
    int           (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* distance_ptr),
    Matrix**      output_cluster_mpp,
    Vector**      output_weights_vpp,
    Vector**      output_classes_vpp
);

/* 3D HISTOGRAM CLUSTERING FUNCTIONS ------------------------------------*/

int set_3D_histogram_num_bins(const int new_max_iterations);
int get_3D_histogram_num_bins(void);

int get_3D_histogram_clusters
(
    const Matrix* input_data_mp,
    Matrix**      output_cluster_mpp,
    Vector**      output_weights_vpp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

