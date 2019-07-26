
/* $Id: lsm_cluster.c 8780 2011-02-27 23:42:02Z predoehl $ */

#ifndef DONT_LINT_SHARED

/* Only safe as the first include of a .c file. */
#include "s/s_incl.h"  /* Includes l_incl.h */

#include "lsm/lsm_cluster.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* #define USE_HARDCODED_INITIAL_CENTRES 1 */

/*
// NOTE: Repetative use of the phrase "kmeans" is intentional. The module may
//       turn into "cluster_lib.c" with more than one clustering algorithm
//       implemented. Then again, it may not.
*/

static int     kmeans_max_iterations = KMEANS_DEFAULT_MAX_ITERATIONS;
static double    kmeans_epsilon        = KMEANS_DEFAULT_EPSILON;

/* Number of clusters. The "k" in k-means                                   */
static int     kmeans_num_clusters;

/* Cluster centre coordinates in data units. Output Value.
// Size  : (k clusters) X (number of columns in data_mp)                    */
static Matrix* kmeans_cluster_mp       = NULL;

/* Relative count of number of data points per cluster. Output Value.
// Counts are double-valued over range [0, 1].
// Length: k clusters.                                                      */
static Vector* kmeans_weights_vp       = NULL;

/* Index of cluster that the i-th data point belongs to.
// Length: n data points (number of rows in data_mp).                       */
static Vector* kmeans_classes_vp       = NULL;

/* Distance from current data point to the k clusters. Reused in loop.
// Length: k clusters.                                                      */
static Vector* kmeans_distance_vp      = NULL;

/* Sum of data point coordinates in cluster k. Reused in loop.
// Size  : (k clusters) X (number of columns in data_mp)                    */
static Matrix* kmeans_cluster_sum_mp   = NULL;

/* Number of data points assigned to current cluster. Reused in loop.
// Length: k clusters                                                       */
static Int_vector* kmeans_cluster_count_vp = NULL;

/* Boolean flag indicating whether cluster centre coordinates have changed. */
static int kmeans_clusters_changed;

/* Number of bins per axis for 3D histogram clustering                      */
static int cluster_3D_histogram_num_bins = CLUSTER_HISTOGRAM_NUM_BINS;

/*--------------------------------------------------------------------------*/

static int init_kmeans_static_data
(
    const Matrix* data_mp,
    int           num_clusters
);

static int get_initial_kmeans_clusters(const Matrix* data_mp);

static int get_distance_from_cluster
(
    const Matrix* data_mp,
    const int     data_index,
    const int     cluster_index,
    int           (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* distance_ptr),
    double*       dist_ptr
);

static int classify_data_by_kmeans_cluster
(
    const Matrix* data_mp,
    int           (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* distance_ptr)
);

static int update_kmeans_clusters(const Matrix* data_mp);

static int euclidean_distance
(
    Vector* v1_vp,
    Vector* v2_vp,
    double* distance_ptr
);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_kmeans_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       set_clustering_options
 *
 *
 *
 *
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_kmeans_max_iterations, get_kmeans_max_iterations,
 *     set_kmeans_epsilon, get_kmeans_epsilon,
 *     get_kmeans_clusters, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/

int set_clustering_options(char* option, char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;

    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "kmeans-max-iterations")
         || match_pattern(lc_option, "k-means-max-iterations")
       )
    {
        int temp_int_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("k-means-max-iterations = %d\n",
                    get_kmeans_max_iterations()));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("k-means maximum number of iterations is %d.\n",
                    get_kmeans_max_iterations()));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            result = set_kmeans_max_iterations(temp_int_value);
        }
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "kmeans-epsilon")
         || match_pattern(lc_option, "k-means-epsilon")
       )
    {
        double temp_real_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("k-means-epsilon = %f\n",
                    get_kmeans_epsilon()));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("k-means iteration termination epsilon is %f.\n",
                    get_kmeans_epsilon()));
        }
        else
        {
            ERE(ss1d(value, &temp_real_value));
            result = set_kmeans_epsilon(temp_real_value);
        }
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "cluster-3d-histogram-num-bins")
         || match_pattern(lc_option, "cluster-3d-hist-num-bins")
       )
    {
        int temp_int_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-3d-histogram-num-bins = %d\n",
                    get_3D_histogram_num_bins()));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("3D histogram clustering algorithm number of bins is %d.\n",
                    get_3D_histogram_num_bins()));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            result = set_3D_histogram_num_bins(temp_int_value);
        }
    }

    return result;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       set_kmeans_max_iterations
 *
 * Changes the setting of the maximum number of iterations.
 *
 * The k-means algorithm iterates until no cluster coordinate values
 * change by more than the epsilon value, or this maximum number of
 * iterations is reached, whichever is first.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     get_kmeans_max_iterations,
 *     set_kmeans_epsilon, get_kmeans_epsilon, set_kmeans_options,
 *     get_kmeans_clusters, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int set_kmeans_max_iterations(const int new_max_num_iterations)
{
    extern int kmeans_max_iterations;

    if (new_max_num_iterations < 0)
    {
        set_error("set_kmeans_max_iterations: max iterations < 0");
        add_error("  (new_max_num_iterations = %d)", new_max_num_iterations);
        return ERROR;
    }
    else
        kmeans_max_iterations = new_max_num_iterations;

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       get_kmeans_max_iterations
 *
 * Returns the current setting of the maximum number of iterations.
 *
 * The k-means algorithm iterates until no cluster coordinate values
 * change by more than the epsilon value, or this maximum number of
 * iterations is reached, whichever is first.
 *
 * Returns:
 *     The current maximum number of iterations allowed by the k-means
 *     clustering algorithm.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_kmeans_max_iterations,
 *     set_kmeans_epsilon, get_kmeans_epsilon, set_kmeans_options,
 *     get_kmeans_clusters, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int get_kmeans_max_iterations(void)
{
    extern int kmeans_max_iterations;

    return kmeans_max_iterations;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       set_kmeans_epsilon
 *
 * Changes the current setting of epsilon used by k-means algorithm.
 *
 * The k-means algorithm iterates until no cluster coordinate values
 * change by more than this epsilon value, or until the maximum number
 * of iterations is reached, whichever is first.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     get_kmeans_epsilon, set_kmeans_options,
 *     set_kmeans_max_iterations, get_kmeans_max_iterations,
 *     get_kmeans_clusters, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int set_kmeans_epsilon(const double new_epsilon)
{
    extern double  kmeans_epsilon;

    if ( IS_LESSER_DBL( (double)new_epsilon, 0.0 ) )
    {
        set_error("set_kmeans_epsilon: epsilon < 0");
        add_error("  ((new_epsilon = %f)", (double) new_epsilon);
        return ERROR;
    }
    else
        kmeans_epsilon = (double)new_epsilon;

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       get_kmeans_epsilon
 *
 * Returns the current setting of epsilon used by k-means algorithm.
 *
 * The k-means algorithm iterates until no cluster coordinate values
 * change by more than this epsilon value, or until the maximum number
 * of iterations is reached, whcihever is first.
 *
 * Returns:
 *     The current k-means clustering algorithm epsilon value.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_kmeans_epsilon, set_kmeans_options,
 *     set_kmeans_max_iterations, get_kmeans_max_iterations,
 *     get_kmeans_clusters, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
double get_kmeans_epsilon(void)
{
    extern double  kmeans_epsilon;

    return kmeans_epsilon;
}

/* ===========================================================================
 *                       get_kmeans_clusters
 *
 * Clusters data using the k-means method.
 *
 * Clusters data into a set number of cluster centres using the k-means
 * algorithm. Returns the cluster centres in a matrix, the number of data
 * points per cluster centre (normalized over [0, 1.0] range, and the cluster
 * centre each input data point is assigned to.
 *
 * The input data to be clustered is contained in "data_mp", an N x D matrix,
 * where N is the number of rows and D is the number of columns (dimensions).
 *
 * The argument "num_clusters" indicates how many cluster centres to compute from
 * the input data. This is the "k" in the k-means algorithm.
 *
 * The "distance_fn" argument allows the user to specify their own distance
 * metric that operates on vectors. If the "distance_fn" argument is NULL,
 * then the Euclidean distance will be used. To specify your own distance
 * function, you must use the following prototype:
 *
 * | int my_distance(Vector* v1_vp, Vector* v2_vp, double* distance_ptr)
 *
 * Your distance function should return NO_ERROR on success, and ERROR on
 * failure.
 *
 * The computed cluster centres are returned in "output_cluster_mpp", which
 * which is a double pointer to a Matrix of size "num_clusters" x D. If the
 * "output_cluster_mpp" matrix does not exist (*output_cluster_mpp == NULL)
 * or is the wrong size, the matrix will be created or resized as appropriate.
 *
 * The normalized number of data points assigned to each cluster is
 * returned in "output_weights_vpp", a Vector of length "num_clusters".
 * If this argument is NULL it will be ignored. If the vector does not exist
 * (*output_weights_vpp == NULL), or is the wrong length, the vector will
 * be created/resized. Note that this vector has benn normalized to sum to 1.0.
 *
 * The index of the cluster centre that each input data point has been assigned
 * to is returned in "output_classes_vpp", a Vector of length N. Each entry
 * is the index to "output_cluster_mpp" Matrix to which the data point has
 * been assigned. If this value is NULL, it will be ignored. If the vector does
 * not exist or is the wrong size, it will be created/resized.
 *
 * Finding the clusters is an iterative process. The iterations are controlled
 * by two parameters: the maximum number of iterations allowed, and the
 * difference a computed cluster centre and its value at the previous iteration.
 * See the man pages for "set_kmeans_max_iterations" and "set_kmeans_epsilon"
 * for more info.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_kmeans_options, set_kmeans_max_iterations, get_kmeans_max_iterations,
 *     set_kmeans_epsilon, get_kmeans_epsilon, free_kmeans_allocated_static_data.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int get_kmeans_clusters
(
    const Matrix* data_mp,
    int           num_clusters,
    int           (*distance_fn) (Vector *, Vector *, double *),
    Matrix**      output_cluster_mpp,
    Vector**      output_weights_vpp,
    Vector**      output_classes_vpp
)
{
    extern int     kmeans_max_iterations;
    extern int     kmeans_clusters_changed;
    extern Matrix* kmeans_cluster_mp;
    extern Vector* kmeans_classes_vp;
    extern Vector* kmeans_weights_vp;

    int i, result = NO_ERROR;

    if (   (data_mp == NULL)
        || (data_mp->num_rows < 1))
    {
        set_error("get_kmeans_clusters(): Input data matrix is empty");
        return ERROR;
    }

    if (data_mp->num_rows <= num_clusters)
    {
        set_error("get_kmeans_clusters(): Bad number of clusters");
        add_error(" (data_mp->num_rows=%d)", data_mp->num_rows);
        add_error(" (num_clusters     =%d)", num_clusters);
        return ERROR;
    }

    if ( init_kmeans_static_data(data_mp, num_clusters) == ERROR )
    {
        add_error("get_kmeans_clusters(): Failed to initialize cluster data");
        return ERROR;
    }

    i = 1;
    kmeans_clusters_changed = TRUE;

    /* Loop until cluster coordinate values stablize, or the iteration limit. */
    while (   kmeans_clusters_changed
           && (i < kmeans_max_iterations) )
    {
        if (distance_fn != NULL)
        {
            /* Use the specified distance function argument */
            if ( classify_data_by_kmeans_cluster(data_mp, distance_fn) == ERROR )
            {
                set_error("get_kmeans_clusters(): Failed to classify input data points");
                result = ERROR;
                break;
            }
        }
        else
        {
            /* Use the default Euclidean distance function */
            if ( classify_data_by_kmeans_cluster(data_mp, euclidean_distance) == ERROR )
            {
                set_error("get_kmeans_clusters(): Failed to classify input data points");
                result = ERROR;
                break;
            }
        }

#ifdef PROGRAMMER_IS_colour
#ifdef TEST
        if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
        {
            verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                        "-------------------------------------------------------------\n");
            verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                        "Classified data points (i=%d):\n", i);
            {
                int p, q;

                q = 0;
                for (p = 0; p < (data_mp->num_rows - 1); p++)
                {
                    verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL, "%3d,",
                                (int)kmeans_classes_vp->elements[p]);
                    q++;
                    if ((q % 10) == 0)
                        verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL, "\n");
                }
                verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL, "%3d.\n",
                            (int)kmeans_classes_vp->elements[data_mp->num_rows - 1]);
            }
        }
#endif
#endif

        if ( update_kmeans_clusters(data_mp) == ERROR )
        {
            set_error("get_kmeans_clusters: Failed to update cluster centres");
            result = ERROR;
            break;
        }

        ++i;

#ifdef PROGRAMMER_IS_colour
#ifdef TEST
        if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
        {
            verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                        "Updated clusters (i=%d):\n", i);
            db_mat(kmeans_cluster_mp);
        }
#endif
#endif
    }

#ifdef PROGRAMMER_IS_colour
#ifdef TEST
    if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
    {
        verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL, "Final cluster centres:\n");
        db_mat(kmeans_cluster_mp);
    }
#endif
#endif

    if (i == kmeans_max_iterations)
    {
        set_error("get_kmeans_clusters(): No clusters found within iteration limit");
        add_error("  (Max iterations=%d)", kmeans_max_iterations);
        add_error("  (Iteration i=%d)", i);
        result = ERROR;
    }
    else if (   (kmeans_cluster_mp == NULL)
             || (kmeans_weights_vp == NULL)
             || (kmeans_classes_vp == NULL))
    {
        result = ERROR;

        if (kmeans_cluster_mp == NULL)
            set_error("get_kmeans_clusters(): Computed cluster matrix is empty");

        if (kmeans_weights_vp == NULL)
            set_error("get_kmeans_clusters(): Computed weights vector is empty");

        if (kmeans_classes_vp == NULL)
            set_error("get_kmeans_clusters(): Computed cluster class vector is empty");
    }

    if (result == NO_ERROR)
    {
        result = copy_matrix(output_cluster_mpp, kmeans_cluster_mp);

        if (   (result == NO_ERROR)
            && (output_weights_vpp != NULL))
            result = copy_vector(output_weights_vpp, kmeans_weights_vp);

        if (   (result == NO_ERROR)
            && (output_classes_vpp != NULL))
            result = copy_vector(output_classes_vpp, kmeans_classes_vp);
    }

    return result;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static int init_kmeans_static_data
(
    const Matrix* data_mp,
    int           num_clusters
)
{
    extern int     kmeans_num_clusters;
    extern Matrix* kmeans_cluster_mp;
    extern Vector* kmeans_weights_vp;
    extern Vector* kmeans_classes_vp;
    extern Vector* kmeans_distance_vp;
    extern Matrix* kmeans_cluster_sum_mp;
    extern Int_vector* kmeans_cluster_count_vp;

    int num_dims;
    int num_points;

#ifdef PROGRAMMER_IS_colour
    TEST_PSE(("cluster_lib.c::init_kmeans_static_data() called.\n"));
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (   (data_mp == NULL)
        || (data_mp->elements == NULL)
        || (data_mp->num_rows <= 0)    )
    {
        set_error("Input data matrix is empty");

        if (data_mp == NULL)
            add_error("  (data_mp = NULL)");
        if  (data_mp->elements == NULL)
            add_error("  (data_mp->elements = NULL)");
        if (data_mp->num_rows <= 0)
            add_error("  (data_mp->num_rows = %d)", data_mp->num_rows);

        return ERROR;
    }

    if (data_mp->num_rows <= num_clusters)
    {
        set_error("Invalid number of clusters for input data");
        add_error("  (data_mp->num_rows = %d\n)", data_mp->num_rows);
        add_error("  (num_clusters      = %d\n)", num_clusters);
        return ERROR;
    }

    num_points = data_mp->num_rows;
    num_dims   = data_mp->num_cols;

    /* Allocate and initialize  static member variables */
    kmeans_num_clusters = num_clusters;

    if ( get_zero_matrix(&kmeans_cluster_mp, num_clusters, num_dims) == ERROR )
    {
        set_error("Failed to allocate cluster centre matrix");
        return ERROR;
    }

    if ( get_zero_vector(&kmeans_weights_vp, num_clusters) == ERROR )
    {
        set_error("Failed to allocate cluster weights vector");
        return ERROR;
    }

    if ( get_zero_vector(&kmeans_distance_vp, num_clusters) == ERROR )
    {
        set_error("Failed to allocate distance vector");
        return ERROR;
    }

    if ( get_zero_vector(&kmeans_classes_vp, num_points) == ERROR )
    {
        set_error("Failed to allocate cluster class vector");
        return ERROR;
    }

    if ( get_zero_matrix(&kmeans_cluster_sum_mp, num_clusters, num_dims) == ERROR )
    {
        set_error("Failed to allocate cluster coordinate sum matrix");
        return ERROR;
    }

    if ( get_zero_int_vector(&kmeans_cluster_count_vp, num_clusters) == ERROR )
    {
        set_error("Failed to allocated data point count in cluster vector");
        return ERROR;
    }

    if ( get_initial_kmeans_clusters(data_mp) == ERROR )
    {
        set_error("Failed to get initial random cluster centres");
        return ERROR;
    }

#ifdef PROGRAMMER_IS_colour
#ifdef TEST
    if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
    {
        verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL, "Initial clusters:\n");
        db_mat(kmeans_cluster_mp);
        verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                    "-------------------------------------------------------------\n");
    }
#endif
#endif

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/
static int get_initial_kmeans_clusters(const Matrix* data_mp)
{
    /*
    // Get the initial cluster centres by randomly choosing unique input
    // data values.
    */
    extern int     kmeans_num_clusters; /* Number of clusters              */
    extern Matrix* kmeans_cluster_mp;   /* Cluster centre matrix to update */

#ifndef USE_HARDCODED_INITIAL_CENTRES
    int  i, k;
    double index;
    int  index_is_duplicate;


    Vector* cluster_vp = NULL;
    Vector* indices_vp = NULL;

    if (kmeans_cluster_mp == NULL)
    {
        set_error("get_initial_kmeans_clusters(): Cluster matrix is empty");
        return ERROR;
    }

    ERE(get_initialized_vector(&indices_vp, kmeans_num_clusters, DBL_NOT_SET));

    for (k = 0; k < kmeans_num_clusters; k++)
    {
        do
        {

            /* Randomly select one of the input data points as an initial cluster */
            index = (double)floor( (double)(data_mp->num_rows - 1) * kjb_rand() );
            index_is_duplicate = FALSE;

            /* Make sure that the random point has not already been chosen */
            for (i = 0; i < kmeans_num_clusters; i++)
            {
                if ( index == indices_vp->elements[i] )
                {
                    index_is_duplicate = TRUE;
                    break;
                }
            }
        } while (   (index_is_duplicate)
                 || (index < 0.0)
                 || (index >= (double)(data_mp->num_rows)) );

        indices_vp->elements[k] = (double)index;

        /* Get the randomly selected input point */
        ERE(get_matrix_row(&cluster_vp, data_mp, (int)index));

        /* Perturb the coordinates by a random amount */

        /* Put the perturbed point into the cluster centre matrix */
        ERE(put_matrix_row(kmeans_cluster_mp, cluster_vp, k));

    }

    free_vector(indices_vp);
    free_vector(cluster_vp);
#endif

#ifdef USE_HARDCODED_INITIAL_CENTRES
    kmeans_cluster_mp->elements[0][0] =  50.0;
    kmeans_cluster_mp->elements[0][1] =  50.0;

    kmeans_cluster_mp->elements[1][0] = 200.0;
    kmeans_cluster_mp->elements[1][1] =  50.0;

    kmeans_cluster_mp->elements[2][0] = 150.0;
    kmeans_cluster_mp->elements[2][1] = 100.0;

    kmeans_cluster_mp->elements[3][0] =  50.0;
    kmeans_cluster_mp->elements[3][1] = 150.0;

    kmeans_cluster_mp->elements[4][0] = 225.0;
    kmeans_cluster_mp->elements[4][1] = 200.0;

    kmeans_cluster_mp->elements[5][0] =  25.0;
    kmeans_cluster_mp->elements[5][1] = 200.0;

    kmeans_cluster_mp->elements[6][0] =  75.0;
    kmeans_cluster_mp->elements[6][1] =  25.0;

    kmeans_cluster_mp->elements[7][0] = 120.0;
    kmeans_cluster_mp->elements[7][1] = 185.0;

    kmeans_cluster_mp->elements[8][0] = 185.0;
    kmeans_cluster_mp->elements[8][1] = 120.0;

    kmeans_cluster_mp->elements[9][0] = 250.0;
    kmeans_cluster_mp->elements[9][1] = 220.0;
#endif

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/
static int get_distance_from_cluster
(
    const Matrix* data_mp,
    const int     data_index,
    const int     cluster_index,
    int           (*distance_fn) (Vector *, Vector *, double *),
    double*       dist_ptr
)
{
    extern Matrix* kmeans_cluster_mp;

    int  c, result = NO_ERROR;
    double sosd; /* sum of squared difference */

    if (   (data_mp == NULL)
        || (data_mp->elements == NULL) )
    {
        set_error("get_distance_from_cluster(): Input data matrix is empty:");
        return ERROR;
    }

    if (   (data_index < 0)
        || (data_index >= data_mp->num_rows) )
    {
        set_error("get_distance_from_cluster(): Data matrix index is out of range:");
        add_error(" (data_index       =%d)", data_index);
        add_error(" (data_mp->num_rows=%d)", data_mp->num_rows);
        return ERROR;
    }

    if (   (kmeans_cluster_mp == NULL)
        || (kmeans_cluster_mp->elements == NULL)    )
    {
        set_error("get_distance_from_cluster(): Cluster matrix is empty:");
        return ERROR;
    }

    if (   (cluster_index < 0)
        || (cluster_index >= kmeans_cluster_mp->num_rows) )
    {
        set_error("get_distance_from_cluster(): Cluster matrix index is out of range:");
        add_error(" (cluster_index              =%d)", cluster_index);
        add_error(" (kmeans_cluster_mp->num_rows=%d)", kmeans_cluster_mp->num_rows);
        return ERROR;
    }

    if (data_mp->num_cols != kmeans_cluster_mp->num_cols)
    {
        set_error("get_distance_from_cluster(): Input and Cluster matrices have different dimensions:");
        add_error(" (data_mp   ->num_cols       =%d)", data_mp->num_cols);
        add_error(" (kmeans_cluster_mp->num_cols=%d)", kmeans_cluster_mp->num_cols);
        return ERROR;
    }

    if (distance_fn == NULL)
    {
        /* Default to Euclidean distance. */
        sosd = (double)0.0;

        for (c = 0; c < data_mp->num_cols; c++)
        {
            sosd +=   (   data_mp->elements[data_index][c]
                        - kmeans_cluster_mp->elements[cluster_index][c] )
                    * (   data_mp->elements[data_index][c]
                        - kmeans_cluster_mp->elements[cluster_index][c] );
        }

        *dist_ptr = sqrt(sosd);
    }
    else
    {
        Vector* data_pt_vp = NULL;
        Vector* cluster_vp = NULL;

        ERE(get_matrix_row(&data_pt_vp, data_mp, data_index));
        ERE(get_matrix_row(&cluster_vp, kmeans_cluster_mp, cluster_index));

        result = distance_fn(data_pt_vp, cluster_vp, dist_ptr);

        free_vector(data_pt_vp);
        free_vector(cluster_vp);
    }

    return result;
}

/*--------------------------------------------------------------------------*/
static int classify_data_by_kmeans_cluster
(
    const Matrix* data_mp,
    int           (*distance_fn) (Vector *, Vector *, double *)
)
{
    extern Vector* kmeans_classes_vp;
    extern Vector* kmeans_distance_vp;
    extern int     kmeans_num_clusters;

    int i, j, k, ties, closest_cluster;
    double distance, min_distance;

    ERE(get_zero_vector(&kmeans_distance_vp, kmeans_num_clusters));
    for (i = 0; i < data_mp->num_rows; i++)
        kmeans_classes_vp->elements[i] = DBL_NOT_SET;

    /* Examine each data point */
    for (i = 0; i < data_mp->num_rows; i++)
    {
        /* Calculate the distance between the current point and all clusters. */
        for (k = 0; k < kmeans_num_clusters; k++)
        {
            if (get_distance_from_cluster(data_mp, i, k, distance_fn,
                                          &distance) == ERROR)
            {
                set_error("Failed to compute distance between data point and cluster:");
                add_error(" (point  : i=%d)", i);
                add_error(" (cluster: k=%d)", k);
                return ERROR;
            }

            kmeans_distance_vp->elements[k] = distance;

#ifdef PROGRAMMER_IS_colour
#ifdef DONT_TEST
            if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
            {

                verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                            "------------------------------------------------------------------\n");
                verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                            "classify_data_by_kmeans_cluster(): point i=%d, cluster k=%d distance = %f\n",
                            i, k, distance);
            }
#endif
#endif

        }

        /* The current point belongs to the closest cluster */
        closest_cluster = get_min_vector_element(kmeans_distance_vp, &min_distance);

        ties = 0;
        for (j = 0; j < kmeans_distance_vp->length; j++)
        {
            if (kmeans_distance_vp->elements[j] == min_distance)
                ties++;
        }
        if (ties > 1)
            pso(" Point[%d] is equidistant to %d clusters.\n", i, ties);



#ifdef PROGRAMMER_IS_colour
#ifdef DONT_TEST
        if (KMEANS_DEBUG_VERBOSE_LEVEL > kjb_get_verbose_level())
        {

            verbose_pso(KMEANS_DEBUG_VERBOSE_LEVEL,
                        "------------------------------------------------------------------\n"); */
            db_rv(kmeans_distance_vp);
            dbi(closest_cluster);
        }
#endif
#endif
        if ((closest_cluster < 0) || (closest_cluster >= data_mp->num_rows))
        {
            set_error("Failed to find the closest cluster to data point - bad index:");
            add_error(" (point  : i=%d)", i);
            add_error(" (cluster: k=%d)", k);
            add_error(" (closest_cluster = %d)", closest_cluster);
            return ERROR;
        }
        else if (closest_cluster == ERROR)
        {
            set_error("Failed to find the closest cluster to data point - minimum returned error:");
            add_error(" (point  : i=%d)", i);
            add_error(" (cluster: k=%d)", k);
            return ERROR;
        }
        else
            kmeans_classes_vp->elements[i] = (double)closest_cluster;
    }

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/
static int update_kmeans_clusters(const Matrix* data_mp)
{
    extern Matrix* kmeans_cluster_mp;       /* Coords of the k clusters          */
    extern Vector* kmeans_weights_vp;       /* Relative point count in cluster k */
    extern Vector* kmeans_classes_vp;       /* Cluster index of i-th data point  */
    extern Matrix* kmeans_cluster_sum_mp;   /* Sum of point coords in cluster k  */
    extern Int_vector* kmeans_cluster_count_vp; /* Num data points in cluster k      */
    extern int     kmeans_num_clusters;     /* Number of clusters                */
    extern int     kmeans_clusters_changed; /* Flag to stop iterations           */


    int i;  /* Data point loop counter           */
    int j;  /* Data point dimension loop counter */
    int k;  /* Cluster loop counter              */
    int num_total_pts;

    /* New cluster coordinate based on mean of member point coords      */
    double new_mean;
    double diff;

    ERE(ow_zero_vector(kmeans_weights_vp));
    ERE(ow_zero_int_vector(kmeans_cluster_count_vp));
    ERE(ow_zero_matrix(kmeans_cluster_sum_mp));

    for (i = 0; i < data_mp->num_rows; i++)
    {
        /* Count the number of points in the cluster that the current
        // point belongs to                                             */
        k = (int)kmeans_classes_vp->elements[i];

        (kmeans_cluster_count_vp->elements[k])++;

        /* Compute the sum per dimension of all data points assigned to
        // the current cluster                                          */
        for (j = 0; j < data_mp->num_cols; j++)
            kmeans_cluster_sum_mp->elements[k][j] += data_mp->elements[i][j];
    }

    num_total_pts = sum_int_vector_elements(kmeans_cluster_count_vp);

    if (num_total_pts != data_mp->num_rows)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    kmeans_clusters_changed = FALSE;

    for (k = 0; k < kmeans_num_clusters; k++)
    {
        for (j = 0; j < data_mp->num_cols; j++)
        {
            new_mean =   kmeans_cluster_sum_mp->elements[k][j]
                            / (double)kmeans_cluster_count_vp->elements[k];

            diff = ABS_OF(new_mean - kmeans_cluster_mp->elements[k][j]);

            /* Update the cluster coordinate if the value has changed. */
            if ( IS_GREATER_DBL(diff, kmeans_epsilon) )
            {
                kmeans_clusters_changed = TRUE;
                kmeans_cluster_mp->elements[k][j] = new_mean;
            }
        }
    }

    /* Update the normailized cluster weights vector */
    for (k = 0; k < kmeans_num_clusters; k++)
    {
        kmeans_weights_vp->elements[k]
            =   kmeans_cluster_count_vp->elements[k] / num_total_pts;
    }

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/


/* ===========================================================================
 *                       free_kmeans_allocated_static_data
 *
 * Frees all allocated data associated with k-means algorithm.
 *
 * Frees all dynamically allocated static data variables used by the k-means
 * clustering algorithm. This function is only declared and defined if the
 * symbol TRACK_MEMORY_ALLOCATION is #defined.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_kmeans_max_iterations, get_kmeans_max_iterations,
 *     set_kmeans_epsilon, get_kmeans_epsilon,
 *     get_kmeans_clusters.
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
#ifdef TRACK_MEMORY_ALLOCATION
static void free_kmeans_allocated_static_data(void)
{
    extern Matrix* kmeans_cluster_mp;
    extern Vector* kmeans_weights_vp;
    extern Vector* kmeans_classes_vp;
    extern Vector* kmeans_distance_vp;
    extern Matrix* kmeans_cluster_sum_mp;
    extern Int_vector* kmeans_cluster_count_vp;

#ifdef PROGRAMMER_IS_colour
    TEST_PSE(("cluster_lib.c::free_kmeans_allocated_static_data() called.\n"));
#endif

    free_matrix(kmeans_cluster_mp);
    free_vector(kmeans_weights_vp);
    free_vector(kmeans_distance_vp);
    free_vector(kmeans_classes_vp);

    free_matrix(kmeans_cluster_sum_mp);
    free_int_vector(kmeans_cluster_count_vp);

    kmeans_cluster_mp       = NULL;
    kmeans_weights_vp       = NULL;
    kmeans_classes_vp       = NULL;
    kmeans_distance_vp      = NULL;
    kmeans_cluster_sum_mp   = NULL;
    kmeans_cluster_count_vp = NULL;
}
#endif

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef TRACK_MEMORY_ALLOCATION
static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;

    if (first_time)
    {

#ifdef PROGRAMMER_IS_colour
        TEST_PSE(("cluster_lib.c::prepare_memory_cleanup()\n"));
#endif

        add_cleanup_function(free_kmeans_allocated_static_data);
        first_time = FALSE;
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ===========================================================================
 *                       set_3D_histogram_num_bins
 *
 * Changes the number of bins per 3D histogram axis.
 *
 * Sets the number of histogram bins per axis used by the 3D histogram
 * clustering algorithm. All axes share the same number of bins. The
 * value of "new_num_bins_per_axis" must be > 0.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     get_clustering_options, get_3D_histogram_num_bins,
 *     get_3D_histogram_clusters
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int set_3D_histogram_num_bins(const int new_num_bins_per_axis)
{
    extern int cluster_3D_histogram_num_bins;

    if (new_num_bins_per_axis < 0)
    {
        set_error("set_3D_histogram_num_bins: number of bins < 0");
        add_error("  (new_num_bins_per_axis = %d)", new_num_bins_per_axis);
        return ERROR;
    }
    else
        cluster_3D_histogram_num_bins = new_num_bins_per_axis;

    return NO_ERROR;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                       get_3D_histogram_num_bins
 *
 * Returns the current setting of the number 3D histogram bins.
 *
 * The 3D histogram clustering algorithm subdivides the input data into
 * fixed bins. This parameter refers to the number of bins per histogram
 * axis.
 *
 * Returns:
 *     The current number of bins per axis used for 3D hsitogram clustering.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     set_clustering_options, set_3D_histogram_num_bins,
 *     get_3D_histogram_clusters
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int get_3D_histogram_num_bins(void)
{
    extern int cluster_3D_histogram_num_bins;

    return cluster_3D_histogram_num_bins;
}

/*--------------------------------------------------------------------------*/

/* ===========================================================================
 *                      get_3D_histogram_clusters
 *
 * Computes clusters by binning data in a 3D histogram.
 *
 * Computes clusters from RGB data by binning the inputs into a 3D histogram
 * with "resolution" bins per axis.
 * clustering algorithm. This function is only declared and defined if the
 * symbol TRACK_MEMORY_ALLOCATION is #defined.
 *
 * Author:
 *     Lindsay Martin
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     get_fixed_clustering_of_3D_data
 *
 *
 *
 * Index: clustering
 *
 * -----------------------------------------------------------------------------
*/
int get_3D_histogram_clusters
(
    const Matrix* input_data_mp,
    Matrix**      output_cluster_mpp,
    Vector**      output_weights_vpp
)
{
    /*
    // The code for this function was shamelessly stolen from the KJB library
    // file "lib/KJB/m/m_mat_stat.c" "get_fixed_clustering_of_3D_data()" contained in
    // .
    // The code was modified to return the weight associated with each resulting
    // cluster (totaling to 1.0), and the
    */
    extern int cluster_3D_histogram_num_bins;

    int        num_rows, num_cols;
    int***     counts;
    double***  x_scores;
    double***  y_scores;
    double***  z_scores;
    Vector*    offset_vp    = NULL;
    Vector*    range_vp     = NULL;
    Vector*    step_vp      = NULL;
    int        i, j, k;
    int        cluster, num_clusters, total_counts, resolution;

    /* Number of bins per histogram axis */
    resolution = cluster_3D_histogram_num_bins;

    num_rows = input_data_mp->num_rows;
    num_cols = input_data_mp->num_cols;

    if ((num_cols != 3) || (num_rows < 1) || (resolution < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(counts = allocate_3D_int_array(resolution, resolution, resolution));
    NRE(x_scores = allocate_3D_double_array(resolution,resolution,resolution));
    NRE(y_scores = allocate_3D_double_array(resolution,resolution,resolution));
    NRE(z_scores = allocate_3D_double_array(resolution,resolution,resolution));

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                counts[ i ][ j ][ k ] = 0;

                x_scores[ i ][ j ][ k ] = 0.0;
                y_scores[ i ][ j ][ k ] = 0.0;
                z_scores[ i ][ j ][ k ] = 0.0;
            }
        }
    }

    ERE(get_max_matrix_col_elements(&range_vp, input_data_mp));

    /* Just in case one of the elements is zero (which is a problem if the min
    // is zero also).
    */
    ERE(ow_add_scalar_to_vector(range_vp, DBL_EPSILON));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(get_min_matrix_col_elements(&offset_vp, input_data_mp));

    /* Just in case one of the elements is zero (which is a problem if the max
    // is zero also).
    */
    ERE(ow_subtract_scalar_from_vector(offset_vp, DBL_EPSILON));
    ERE(ow_subtract_vectors(range_vp, offset_vp));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(divide_vector_by_scalar(&step_vp, range_vp, (double)resolution));

    total_counts = 0;
    for (cluster = 0; cluster < num_rows; cluster++)
    {
        i = kjb_rint((input_data_mp->elements[ cluster ][ 0 ] - offset_vp->elements[ 0 ]) /
                                                        step_vp->elements[ 0 ]);
        j = kjb_rint((input_data_mp->elements[ cluster ][ 1 ] - offset_vp->elements[ 1 ]) /
                                                        step_vp->elements[ 1 ]);
        k = kjb_rint((input_data_mp->elements[ cluster ][ 2 ] - offset_vp->elements[ 2 ]) /
                                                        step_vp->elements[ 2 ]);

        x_scores[ i ][ j ][ k ] += input_data_mp->elements[ cluster ][ 0 ];
        y_scores[ i ][ j ][ k ] += input_data_mp->elements[ cluster ][ 1 ];
        z_scores[ i ][ j ][ k ] += input_data_mp->elements[ cluster ][ 2 ];

        counts[ i ][ j ][ k ]++;
        total_counts++;
    }

    num_clusters = 0;

    /* Dry run */

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                if (counts[ i ][ j ][ k ] > 0)
                {
                    num_clusters++;
                }
            }
        }
    }

    if (num_clusters <= 0)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(output_cluster_mpp, num_clusters, 3));
    ERE(get_target_vector(output_weights_vpp, num_clusters));
    cluster = 0;

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                if (counts[ i ][ j ][ k ] > 0)
                {
                    (*output_cluster_mpp)->elements[ cluster ][ 0 ] =
                            x_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*output_cluster_mpp)->elements[ cluster ][ 1 ] =
                            y_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*output_cluster_mpp)->elements[ cluster ][ 2 ] =
                            z_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];
                    (*output_weights_vpp)->elements[ cluster ] =
                           (double)counts[ i ][ j ][ k ] / (double)total_counts;
                    cluster++;
                }
            }
        }
    }

    free_vector(offset_vp);
    free_vector(step_vp);
    free_vector(range_vp);
    free_3D_int_array(counts);
    free_3D_double_array(x_scores);
    free_3D_double_array(y_scores);
    free_3D_double_array(z_scores);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int euclidean_distance
(
    Vector* v1_vp,
    Vector* v2_vp,
    double* distance_ptr
)
{
    int     i;
    double    diff, sum;

    if ( (v1_vp == NULL) || (v2_vp == NULL) )
    {
        set_error("cluster_lib.c::euclidean_distance() - One of the input vectors is NULL");
        return ERROR;
    }

    if (v1_vp->length != v2_vp->length)
    {
        set_error("cluster_lib.c::euclidean_distance() - Input vectors have different dimensions:");
        add_error(" (v1_vp->length=%d)", v1_vp->length);
        add_error(" (v2_vp->length=%d)", v2_vp->length);
        return ERROR;
    }

    sum = (double)0.0;
    for (i = 0; i < v1_vp->length; i++)
    {
        diff = v1_vp->elements[i] - v2_vp->elements[i];
        sum += (diff * diff);
    }

    *distance_ptr = (double)sqrt((double)sum);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef __cplusplus
}
#endif

#endif   /* #ifndef DONT_LINT_SHARED */


