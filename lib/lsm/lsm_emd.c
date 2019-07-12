
/* $Id: lsm_emd.c 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef DONT_LINT_SHARED

/*
    emd.c

    Copyright (C) 1998 Yossi Rubner
    Computer Science Department, Stanford University
    E-Mail: rubner@cs.stanford.edu
    URL: http://vision.stanford.edu/~rubner
*/

/* #define DEBUG_DB_IO 1 */

/* Only safe as the first include of a .c file. */
#include "s/s_incl.h"  /* Includes l_incl.h */

#include "lsm/lsm_cluster.h"
#include "lsm/lsm_emd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* SLLNode is used for singly-linked lists */
typedef struct SLLNode {
    int i;
    double   val;
    struct SLLNode *next;
} SLLNode;

/* DLLNode is used for doubly-linked lists */
typedef struct DLLNode {
    int    i, j;
    double   val;
    struct DLLNode *next_col;               /* next column */
    struct DLLNode *next_row;               /* next row    */
} DLLNode;


/* Global variable declarations */
static int       size_sig1;
static int       size_sig2;
static int       max_sig_size;
static int       prev_max_sig_size = NOT_SET;

static Matrix*   costs_mp      = NULL;
static double      max_cost;
static DLLNode*  basic_vars_np = NULL;
static DLLNode*  end_X         = NULL;
static DLLNode*  enter_X       = NULL;
static unsigned char**   is_X          = NULL;
static DLLNode** RowsX_np_ptr  = NULL;
static DLLNode** ColsX_np_ptr  = NULL;
static double      max_W;


static int allocate_static_data(Signature* sig1, Signature* sig2);

#ifdef TRACK_MEMORY_ALLOCATION
static void free_allocated_static_emd_data(void);
static void prepare_memory_cleanup(void);
#endif

/*
// The following functions are Yossi Rubner's EMD functions modified to use
// the KJB library data types, and extended to dynamically allocate memory
// for arbitrary sized data structures.
*/
static int find_initial_solution
(
    Signature* sig1,
    Signature* sig2,
    int        (*distance_fn)(Vector* v1_vp, Vector* v2_vp, double* dist_ptr),
    double*    initial_cost_ptr
);

static int find_basic_variables(SLLNode* row_list, SLLNode* col_list);

static int is_solution_optimal(SLLNode* row_list, SLLNode* col_list);

static int find_loop(DLLNode** Loop, int* num_steps_ptr);

static int improve_solution(void);

static int russel(Vector* supply_vp, Vector* demand_vp);

static int add_basic_variable
(
    int      i_min,
    int      j_min,
    Vector*  supply_vp,
    Vector*  demand_vp,
    SLLNode* prev_row_i_min,
    SLLNode* prev_col_j_min,
    SLLNode* row_head
);

/*
// The following functions are helpers for the signature database ADT.
*/
static int get_unclustered_data
(
    Matrix*  input_data_mp,
    Matrix** clusters_mpp,
    Vector** weights_vpp
);

static int fp_read_signature_db_header
(
    FILE*               fp,
    int*                num_signatures_ptr,
    Emd_cluster_method* cluster_method_ptr,
    Emd_data_origin*    data_origin_ptr
);

static int fp_write_signature_db_header
(
    FILE*               fp,
    const Signature_db* src_db_ptr
);

/*-------------------------------------------------------------------------*/

static int allocate_static_data(Signature* sig1, Signature* sig2)
{
    extern int       max_sig_size;
    extern int       prev_max_sig_size;
    extern Matrix*   costs_mp;
    extern DLLNode*  basic_vars_np;
    extern DLLNode** RowsX_np_ptr;
    extern DLLNode** ColsX_np_ptr;
    extern unsigned char**    is_X;


#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::allocate_static_data() called.\n"));
# endif
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    /* Determine maximum signature size for dynamic allocation */
    if (sig1->num_features > sig2->num_features)
        max_sig_size = sig1->num_features;
    else
        max_sig_size = sig2->num_features;

    /* Allocate Cost matrix. get_zero_matrix() resizes as necessary */
    ERE(get_zero_matrix(&costs_mp, max_sig_size + 1, max_sig_size + 1));

    /* Allocate the Basic Variables Array of Doubly Linked Nodes */
    if (basic_vars_np == NULL)
    {
        NRE( basic_vars_np = N_TYPE_MALLOC(DLLNode, 2 * (max_sig_size + 1)) );
    }
    else if (max_sig_size == prev_max_sig_size)
    {
        /*EMPTY*/
        ; /* Do nothing. */
    }
    else
    {
        DLLNode* temp_vars_np;

        NRE( temp_vars_np = N_TYPE_MALLOC(DLLNode, 2 * (max_sig_size + 1)) );
        kjb_free(basic_vars_np);
        basic_vars_np = temp_vars_np;
    }

    /* Allocate Row Indeces vector */
    if (RowsX_np_ptr == NULL)
    {
        NRE( RowsX_np_ptr = N_TYPE_MALLOC(DLLNode*, (max_sig_size + 1)) );
    }
    else if (max_sig_size == prev_max_sig_size)
    {
        /*EMPTY*/
        ; /* Do nothing. */
    }
    else
    {
        DLLNode** temp_RowsX_np_ptr;

        NRE( temp_RowsX_np_ptr = N_TYPE_MALLOC(DLLNode*, (max_sig_size + 1)) );
        kjb_free(RowsX_np_ptr);
        RowsX_np_ptr = temp_RowsX_np_ptr;
    }

    /* Allocate Column Indeces vector */
    if (ColsX_np_ptr == NULL)
    {
        NRE( ColsX_np_ptr = N_TYPE_MALLOC(DLLNode*, (max_sig_size + 1)) );
    }
    else if (max_sig_size == prev_max_sig_size)
    {
        /*EMPTY*/
        ; /* Do nothing. */
    }
    else
    {
        DLLNode** temp_ColsX_np_ptr;

        NRE( temp_ColsX_np_ptr = N_TYPE_MALLOC(DLLNode*, (max_sig_size + 1)) );
        kjb_free(ColsX_np_ptr);
        ColsX_np_ptr = temp_ColsX_np_ptr;
    }

    /* Allocate "boolean" IsX array */
    if (is_X == NULL)
    {
        NRE( is_X = allocate_2D_byte_array( (max_sig_size + 1),
                                            (max_sig_size + 1) ) );
    }
    else if (max_sig_size == prev_max_sig_size)
    {
        /*EMPTY*/
        ; /* Do nothing. */
    }
    else
    {
        unsigned char** temp_is_X;

        NRE( temp_is_X = allocate_2D_byte_array( (max_sig_size + 1),
                                                 (max_sig_size + 1) ) );
        free_2D_byte_array(is_X);
        is_X = temp_is_X;
    }

    prev_max_sig_size = max_sig_size;

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef TRACK_MEMORY_ALLOCATION
static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;

    if (first_time)
    {
        add_cleanup_function(free_allocated_static_emd_data);
        first_time = FALSE;
    }
}
#endif

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef TRACK_MEMORY_ALLOCATION
static void free_allocated_static_emd_data(void)
{
    extern Matrix*   costs_mp;
    extern DLLNode*  basic_vars_np;
    extern DLLNode** RowsX_np_ptr;
    extern DLLNode** ColsX_np_ptr;
    extern unsigned char**   is_X;

    free_matrix(costs_mp);
    kjb_free(basic_vars_np);
    kjb_free(RowsX_np_ptr);
    kjb_free(ColsX_np_ptr);
    free_2D_byte_array(is_X);
}
#endif
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ===========================================================================
 *                        get_earthmover_distance
 *
 * Computes the earthmover's distance.
 *
 * This routine computes the earthmover's distance between two distributions.
 * Distributions are stored in "signatures" which consist of an array of
 * "features" and a corresponding array of feature weights.
 *
 * In the emd_lib.c module, a "feature" is simply a "Vector".
 *
 * A "signature" is defined as follows:
 *
 * |     typedef struct Signature
 * |    {
 * |        int      num_features; Number of features in the signature
 * |        Vector** feature_vec;  Pointer to array of features
 * |        double*    weights_vec;  Pointer to array of feature weights
 * |    } Signature;
 *
 * Each feature in a signature has a corresponding weight indicating its relative
 * abundance in the distribution.
 *
 * A distance function must be supplied that computes the distance between
 * two features. A function that computes the Euclidean distance between two
 * features as defined above is provided: See the "distance" function.
 *
 * This routine will optionally return the flow that was required to map one input
 * distribution to the other. If "flow_vec" is NULL, then no flow is computed. If
 * output flows are required, then "flow_vec" must be preallocated, and contain
 * n1 + n2 - 1 elements, where n1 and n2 are the numbers of elements in the two
 * input signatures.
 *
 * Note that the earthmover's distance is a true metric ONLY when the two input
 * signatures are the same size.
 *
 * Uses code written by Yosi Rubner. See http://vision.stanford.edu/~rubner
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Yossi Rubner, Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int get_earthmover_distance
(
    Signature* sig1, /* First input signature    */
    Signature* sig2, /* Second input signature   */
    int        (*distance_fn) (Vector *, Vector *, double *),
    Flow*      flow_vec, /* Output array of flows    */
    int*       num_flows_ptr, /* Number of valid flows    */
    double*    em_distance_ptr /* Earthmover's Distance    */
)
{
    extern int      size_sig1;
    extern int      size_sig2;
    extern DLLNode* basic_vars_np;
    extern DLLNode* end_X;
    extern DLLNode* enter_X;
    extern Matrix*  costs_mp;

    int      itr;
    double     total_cost;
    double     initial_cost;
    DLLNode* XP;
    Flow*    flow_ptr = NULL;
    SLLNode* row_list = NULL;
    SLLNode* col_list = NULL;

    int     result = NO_ERROR;

    if ((sig1 == NULL) || (sig2 == NULL))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    result = allocate_static_data(sig1, sig2);

    if (result == ERROR)
    {
        add_error("emd_lib.c::get_earthmover_distance() - Unable to obtain memory for data");
        return ERROR;
    }

    NRE( row_list = N_TYPE_MALLOC(SLLNode, max_sig_size + 1) );
    NRE( col_list = N_TYPE_MALLOC(SLLNode, max_sig_size + 1) );

    result = find_initial_solution(sig1, sig2, distance_fn, &initial_cost);

    if (result == ERROR)
    {
        add_error("emd_lib.c::get_earthmover_distance() - Unable to compute initial solution");
        return ERROR;
    }

    /* If size_sig1 = 1 or size_sig2 = 1 then we are done */
    if (size_sig1 > 1 && size_sig2 > 1)
    {
        for (itr = 1; itr < EMD_MAX_ITERATIONS; itr++)
        {
            /* Find basic variables */
            if ( (result = find_basic_variables(row_list, col_list)) == ERROR )
            {
                add_error("emd_lib.c::get_earthmover_distance():");
                add_error("  Unable to find basic variables at iteration %d", itr);
                break;
            }
    
            /* Check for optimality */
            result = is_solution_optimal(row_list, col_list);

            if ( result == TRUE )
            {
                result = NO_ERROR;
                break;
            }

            else if (result == ERROR)
            {
                add_error("emd_lib.c::get_earthmover_distance():");
                add_error("  Unable to determine if solution is optimal at iteration %d", itr);
                break;
            }


            /* Improve solution */
            if ( (result = improve_solution()) == ERROR )
            {
                add_error("emd_lib.c::get_earthmover_distance()");
                add_error("  Unable to improve solution at iteration %d", itr);
                break;
            }
        }

        if ( (result == NO_ERROR) && (itr == EMD_MAX_ITERATIONS) )
        {
            set_error("emd_lib.c::get_earthmover_distance()");
            add_error("  Max number of iterations=%d has been reached without finding solution\n",
                      EMD_MAX_ITERATIONS);
            result = NO_SOLUTION;
        }

    }

    if ((result == ERROR) || (result == NO_SOLUTION))
        *em_distance_ptr = DBL_NOT_SET;
    else
    {
        /* Compute the total flow */
        total_cost = 0;
        if (flow_vec != NULL)
            flow_ptr = flow_vec;

        for(XP = basic_vars_np; XP < end_X; XP++)
        {
            if ( XP == enter_X )
                /* enter_X is the empty slot */
                continue;

            if (   (XP->i == sig1->num_features)
                || (XP->j == sig2->num_features) )
                /* dummy feature */
                continue;

            if ( XP->val == 0 )
                /* zero flow */
                continue;

            total_cost += XP->val * costs_mp->elements[XP->i][XP->j];

            if (flow_vec != NULL)
            {
                flow_ptr->from_feature = XP->i;
                flow_ptr->to_feature   = XP->j;
                flow_ptr->flow_amount  = XP->val;
                flow_ptr++;
            }
        }

        if (flow_vec != NULL)
        {
#ifdef HOW_IT_WAS
            *num_flows_ptr = flow_ptr - flow_vec;
#else
            UNTESTED_CODE();
            *num_flows_ptr = (flow_ptr - flow_vec) / sizeof(Flow*);
#endif
        }


        /* Return the normalized cost == emd */
        *em_distance_ptr = (total_cost / initial_cost);
    }

    kjb_free(row_list);
    kjb_free(col_list);


    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/**********************
   find_initial_solution
**********************/
static int find_initial_solution
(
    Signature* sig1,
    Signature* sig2,
    int        (*distance_fn) (Vector *, Vector *, double *),
    double*    initial_cost_ptr
)
{
    extern int       size_sig1;
    extern int       size_sig2;
    extern int       max_sig_size;
    extern Matrix*   costs_mp;
    extern double      max_cost;
    extern double      max_W;
    extern DLLNode** RowsX_np_ptr;
    extern DLLNode** ColsX_np_ptr;
    extern DLLNode*  enter_X;
    extern DLLNode*  end_X;
    extern unsigned char**    is_X;


    int      i, j;
    double     sum_supply, sum_demand, diff, distance;
    Vector*  feature1_vp = NULL;
    Vector*  feature2_vp = NULL;
    Vector*  supply_vp   = NULL;
    Vector*  demand_vp   = NULL;


    *initial_cost_ptr = DBL_NOT_SET;

    ERE( get_zero_vector(&supply_vp, max_sig_size + 1) );
    ERE( get_zero_vector(&demand_vp, max_sig_size + 1) );

    size_sig1 = sig1->num_features; /* set global */
    size_sig2 = sig2->num_features; /* set global */

    if (size_sig1 > max_sig_size || size_sig2 > max_sig_size)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    /* Compute the distance matrix and find the maximum cost. */
    max_cost = 0;

    for(i=0; i < size_sig1; i++)
        for(j=0; j < size_sig2; j++)
        {
            feature1_vp = sig1->feature_vec[i];
            feature2_vp = sig2->feature_vec[j];

            if (distance_fn(feature1_vp, feature2_vp, &distance) == ERROR)
            {
                set_error("emd_lib.c::find_initial_solution(): distance function failed");
                return ERROR;
            }

            costs_mp->elements[i][j] = distance;

            if (costs_mp->elements[i][j] > max_cost)
                max_cost = costs_mp->elements[i][j];
        }
    
    /* Sum up the supply and demand */
    sum_supply = 0.0;

    for(i=0; i < size_sig1; i++)
    {
        supply_vp->elements[i]  = sig1->weights_vec[i];
        sum_supply += sig1->weights_vec[i];
        RowsX_np_ptr[i] = NULL;
    }
    sum_demand = 0.0;

    for(j=0; j < size_sig2; j++)
    {
        demand_vp->elements[j]  = sig2->weights_vec[j];
        sum_demand += sig2->weights_vec[j];
        ColsX_np_ptr[j] = NULL;
    }

    /* If supply different than the demand, add a zero-cost dummy cluster */
    diff = sum_supply - sum_demand;
    if (fabs(diff) >= EMD_EPSILON * sum_supply)
    {
        if (diff < 0.0)
        {
            for (j=0; j < size_sig2; j++)
                costs_mp->elements[size_sig1][j] = 0;

            supply_vp->elements[size_sig1] = -diff;
            RowsX_np_ptr[size_sig1] = NULL;
            size_sig1++;
        }
        else
        {
            for (i=0; i < size_sig1; i++)
                costs_mp->elements[i][size_sig2] = 0;

            demand_vp->elements[size_sig2] = diff;
            ColsX_np_ptr[size_sig2] = NULL;
            size_sig2++;
        }
    }

    /* Initialize the basic variable structures */
    for (i=0; i < size_sig1; i++)
        for (j=0; j < size_sig2; j++)
            is_X[i][j] = FALSE;

    end_X = basic_vars_np;
    max_W = sum_supply > sum_demand ? sum_supply : sum_demand;

    /* Find initial solution */
    ERE( russel(supply_vp, demand_vp) );

    /* An empty slot (only "size_sig1 + size_sig2 - 1" basic variables) */
    enter_X = end_X++;

    if (sum_supply > sum_demand)
        *initial_cost_ptr = sum_demand;
    else
        *initial_cost_ptr = sum_supply;

    free_vector(supply_vp);
    free_vector(demand_vp);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/**********************
    find_basic_variables
 **********************/
static int find_basic_variables(SLLNode* row_list, SLLNode* col_list)
{
    extern int    size_sig1;
    extern int    size_sig2;
    extern unsigned char** is_X;

    int      i, j, found;
    int      num_found_in_row_list, num_found_in_col_list;
    SLLNode  row0_head, row1_head;
    SLLNode* curr_row = NULL;
    SLLNode* prev_row = NULL;

    SLLNode  col0_head, col1_head;
    SLLNode* curr_col = NULL;
    SLLNode* prev_col = NULL;

    int result;

    /* Initialize the rows list and the columns list */
    row0_head.next = curr_row = row_list;

    for (i = 0; i < size_sig1; i++)
    {
        curr_row->i    = i;
        curr_row->next = curr_row + 1;
        curr_row++;
    }
    (--curr_row)->next = NULL;
    row1_head.next = NULL;

    curr_col = col_list + 1;
    col0_head.next = size_sig2 > 1 ? col_list + 1 : NULL;

    for (j = 1; j < size_sig2; j++)
    {
        curr_col->i    = j;
        curr_col->next = curr_col+1;
        curr_col++;
    }
    (--curr_col)->next = NULL;
    col1_head.next = NULL;

    /* There are size_sig1 + size_sig2 variables, but only
    // size_sig1+size_sig2-1 independent equations, so set V[0]=0
    */
    col_list[0].i   = 0;
    col_list[0].val = 0;
    col1_head.next = col_list;
    col1_head.next->next = NULL;

    /* Loop until all variables are found */
    num_found_in_row_list = num_found_in_col_list = 0;
    result = NO_ERROR;

    while (   (result == NO_ERROR)
           && (   (num_found_in_row_list < size_sig1)
               || (num_found_in_col_list < size_sig2)
              )
          )
    {
        found = FALSE;
        if (num_found_in_col_list < size_sig2)
        {
            /* Loop over all marked columns */
            prev_col = &col1_head;
            for ( curr_col = col1_head.next;
                  curr_col != NULL;
                  curr_col = curr_col->next
                )
            {
                j = curr_col->i;
                /* Find the variables in column j */
                prev_row = &row0_head;
                for ( curr_row = row0_head.next;
                      curr_row != NULL;
                      curr_row = curr_row->next
                    )
                {
                    i = curr_row->i;
                    if ( is_X[i][j] )
                    {
                        /* Compute row_list[i] */
                        curr_row->val = costs_mp->elements[i][j] - curr_col->val;
                        /* ...and add it to the marked list */
                        prev_row->next = curr_row->next;
                        curr_row->next = row1_head.next != NULL ? row1_head.next : NULL;
                        row1_head.next = curr_row;
                        curr_row = prev_row;
                    }
                    else
                        prev_row = curr_row;
                }
                prev_col->next = curr_col->next;
                num_found_in_col_list++;
                found = TRUE;
            }
        }
        if (num_found_in_row_list < size_sig1)
        {
            /* Loop over all marked rows */
            prev_row = &row1_head;
            for ( curr_row = row1_head.next;
                  curr_row != NULL;
                  curr_row = curr_row->next
                )
            {
                i = curr_row->i;
                /* Find the variables in rows i */
                prev_col = &col0_head;
                for ( curr_col = col0_head.next;
                      curr_col != NULL;
                      curr_col = curr_col->next
                    )
                {
                    j = curr_col->i;
                    if ( is_X[i][j] )
                    {
                        /* Compute col_list[j] */
                        curr_col->val  = costs_mp->elements[i][j] - curr_row->val;
                        /* ...and add it to the marked list */
                        prev_col->next = curr_col->next;
                        curr_col->next = col1_head.next != NULL ? col1_head.next: NULL;
                        col1_head.next = curr_col;
                        curr_col = prev_col;
                    }
                    else
                        prev_col = curr_col;
                }
                prev_row->next = curr_row->next;
                num_found_in_row_list++;
                found = TRUE;
            }
        }

        /* If not all basic variables are found, then the EMD_EPSILON defined
        // in emd_lib.h is not right for the scale of the problem.
        */
        if (found == FALSE)
        {
            set_error("emd_lib.c::find_basic_variable() - Unable to find all basic variables for E.M.D. optimization");
            result = ERROR;
        }
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */


/**********************
    is_solution_optimal
 **********************/
static int is_solution_optimal(SLLNode* row_list, SLLNode* col_list)
{
    extern int      size_sig1;
    extern int      size_sig2;
    extern Matrix*  costs_mp;
    extern double     max_cost;
    extern DLLNode* enter_X;
    extern unsigned char**   is_X;

    double   delta, min_delta;
    int i, j, i_min = NOT_SET, j_min = NOT_SET;

    /* Find the minimal cost[i,j] -row_list[i] - col_list[j] over all i,j */
    min_delta = EMD_INFINITY;
    for(i = 0; i < size_sig1; i++)
        for(j = 0; j < size_sig2; j++)
            if (! is_X[i][j] )
            {
                delta =    costs_mp->elements[i][j]
                         - row_list[i].val
                         - col_list[j].val;

                if (min_delta > delta)
                {
                    min_delta = delta;
                    i_min = i;
                    j_min = j;
                }
            }


    if (min_delta == EMD_INFINITY)
    {
        set_error("emd_lib.c::is_solution_optimal() - Unable to determine if earth mover's distance solution is optimal");
        return ERROR;
    }

    ASSERT(i_min != NOT_SET);
    ASSERT(j_min != NOT_SET);

    enter_X->i = i_min;
    enter_X->j = j_min;

    /* return min_delta >= -EMD_EPSILON * max_cost; */
    if (min_delta >= -EMD_EPSILON * max_cost)
        return TRUE;
    else
        return FALSE;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/**********************
    improve_solution
**********************/
static int improve_solution(void)
{
    extern int       max_sig_size;
    extern DLLNode** RowsX_np_ptr;
    extern DLLNode** ColsX_np_ptr;
    extern DLLNode*  enter_X;
    extern unsigned char**    is_X;

    int       i, j, k;
    double      min_X;
    int       num_steps;
    DLLNode** Loop   = NULL;
    DLLNode*  curr_X = NULL;
    DLLNode*  LeaveX = NULL;

    NRE( Loop = N_TYPE_MALLOC(DLLNode*, 2 * (max_sig_size + 1)) );

    /* Enter the new basic variable */
    i = enter_X->i;
    j = enter_X->j;
    is_X[i][j] = TRUE;

    enter_X->next_col = RowsX_np_ptr[i];
    enter_X->next_row = ColsX_np_ptr[j];
    enter_X->val      = 0;

    RowsX_np_ptr[i] = enter_X;
    ColsX_np_ptr[j] = enter_X;

    /* Find a chain reaction */
    ERE( find_loop(Loop, &num_steps) );

    /* Find the largest value in the loop */
    min_X = EMD_INFINITY;

    for (k = 1; k < num_steps; k += 2)
    {
        if ( Loop[k]->val < min_X )
        {
            LeaveX = Loop[k];
            min_X  = Loop[k]->val;
        }
    }

    /* Update the loop */
    for (k = 0; k < num_steps; k += 2)
    {
        Loop[k]->val   += min_X;
        Loop[k+1]->val -= min_X;
    }


    /* Remove the leaving basic variable */
    i = LeaveX->i;
    j = LeaveX->j;
    is_X[i][j] = FALSE;

    if (RowsX_np_ptr[i] == LeaveX)
        RowsX_np_ptr[i] = LeaveX->next_col;
    else
        for (curr_X = RowsX_np_ptr[i]; curr_X != NULL; curr_X = curr_X->next_col)
            if (curr_X->next_col == LeaveX)
            {
                curr_X->next_col = curr_X->next_col->next_col;
                break;
            }
    if (ColsX_np_ptr[j] == LeaveX)
        ColsX_np_ptr[j] = LeaveX->next_row;
    else
        for (curr_X = ColsX_np_ptr[j]; curr_X != NULL; curr_X = curr_X->next_row)
            if (curr_X->next_row == LeaveX)
            {
                curr_X->next_row = curr_X->next_row->next_row;
                break;
            }

    /* Set enter_X to be the new empty slot */
    enter_X = LeaveX;

    kjb_free(Loop);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */


/**********************
    find_loop
**********************/
static int find_loop(DLLNode** Loop, int* num_steps_ptr)
{
    extern int      size_sig1;
    extern int      size_sig2;
    extern int      max_sig_size;
    extern DLLNode* enter_X;
    extern DLLNode* basic_vars_np;

    int i, num_steps;
    DLLNode** curr_X;
    DLLNode*  new_X;
    unsigned char*     is_used = NULL;

    int result = NO_ERROR;

    NRE( is_used = N_TYPE_MALLOC(unsigned char, 2 * (max_sig_size +1)) );

    for (i=0; i < size_sig1 + size_sig2; i++)
        is_used[i] = FALSE;

    curr_X = Loop;
    new_X = *curr_X = enter_X;
    is_used[enter_X - basic_vars_np] = TRUE;
    num_steps = 1;

    do
    {
        if (num_steps % 2 == 1)
        {
            /* Find an unused X in the row */
            new_X = RowsX_np_ptr[new_X->i];
            while (new_X != NULL && is_used[new_X - basic_vars_np])
                new_X = new_X->next_col;
        }
        else
        {
            /* Find an unused X IN the column, or the entering X */
            new_X = ColsX_np_ptr[new_X->j];
            while (    (new_X != NULL)
                    && (is_used[new_X - basic_vars_np])
                    && (new_X != enter_X)
                  )
                new_X = new_X->next_row;

            if (new_X == enter_X)
                break;
        }

        if (new_X != NULL)  /* Found the next X */
        {
            /* Add X to the loop */
            *++curr_X = new_X;
            is_used[new_X - basic_vars_np] = TRUE;
            num_steps++;
        }
        else  /* Didn't find the next X */
        {
            /* Backtrack */
            do
            {
                new_X = *curr_X;
                do
                {
                    if (num_steps % 2 == 1)
                        new_X = new_X->next_row;
                    else
                        new_X = new_X->next_col;
                } while (new_X != NULL && is_used[new_X - basic_vars_np]);
    
                if (new_X == NULL)
                {
                    is_used[*curr_X - basic_vars_np] = FALSE;
                    curr_X--;
                    num_steps--;
                }
            } while (new_X == NULL && curr_X >= Loop);
    
            is_used[*curr_X - basic_vars_np] = FALSE;
            *curr_X = new_X;
            is_used[new_X - basic_vars_np] = TRUE;
        }
    } while(curr_X >= Loop);

    if (curr_X == Loop)
    {
        set_error("emd_lib.c::find_loop() - Unable to find loop through variables");
        *num_steps_ptr = NOT_SET;
        result = ERROR;
    }
    else
        *num_steps_ptr = num_steps;

    kjb_free(is_used);

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */


/**********************
    russel
**********************/
static int russel(Vector* supply_vp, Vector* demand_vp)
{
    extern int     size_sig1;
    extern int     size_sig2;
    extern int     max_sig_size;
    extern Matrix* costs_mp;
    extern double    max_cost;

    int      i, j, found, i_min = NOT_SET, j_min = NOT_SET;
    double     min_delta, old_val, diff;
    Matrix*  delta = NULL;

    SLLNode  row_head, col_head;
    SLLNode* row_list = NULL;
    SLLNode* col_list = NULL;
    SLLNode* curr_row = NULL;
    SLLNode* prev_row = NULL;
    SLLNode* curr_col = NULL;
    SLLNode* prev_col = NULL;
    SLLNode* prev_row_i_min = NULL;
    SLLNode* prev_col_j_min = NULL;
    SLLNode* backtrack_node = NULL;

    ERE( get_zero_matrix(&delta, max_sig_size + 1, max_sig_size + 1) );
    NRE( row_list = N_TYPE_MALLOC(SLLNode, max_sig_size + 1) );
    NRE( col_list = N_TYPE_MALLOC(SLLNode, max_sig_size + 1) );

    /* Initialize the rows list, and the columns list */
    row_head.next = curr_row = row_list;
    for (i = 0; i < size_sig1; i++)
    {
        curr_row->i = i;
        curr_row->val = -EMD_INFINITY;
        curr_row->next = curr_row + 1;
        curr_row++;
    }
    (--curr_row)->next = NULL;

    col_head.next = curr_col = col_list;
    for (j = 0; j < size_sig2; j++)
    {
        curr_col->i = j;
        curr_col->val = -EMD_INFINITY;
        curr_col->next = curr_col + 1;
        curr_col++;
    }
    (--curr_col)->next = NULL;

    /* Find the maximum row and column values (row_list[i] and col_list[j]) */
    for(i=0; i < size_sig1 ; i++)
        for(j=0; j < size_sig2 ; j++)
        {
            double v;
            v = costs_mp->elements[i][j];
            if (row_list[i].val <= v)
                row_list[i].val = v;
            if (col_list[j].val <= v)
                col_list[j].val = v;
        }

    /* Compute the delta matrix */
    for(i = 0; i < size_sig1 ; i++)
        for(j = 0; j < size_sig2 ; j++)
        {
            delta->elements[i][j] =
                  costs_mp->elements[i][j]
                - row_list[i].val
                - col_list[j].val;
        }


    /* Find the basic variables */
    do
    {

        /* Find the smallest delta[i][j] */
        found = FALSE;
        min_delta = EMD_INFINITY;
        prev_row = &row_head;
        for (curr_row=row_head.next; curr_row != NULL; curr_row=curr_row->next)
        {
            /* int i; */ /* From Yosi */
            i = curr_row->i;
            prev_col = &col_head;
            for (curr_col=col_head.next; curr_col != NULL; curr_col=curr_col->next)
            {
                /* int j; */ /* From Yosi */
                j = curr_col->i;
                if (min_delta > delta->elements[i][j])
                {
                    min_delta = delta->elements[i][j];
                    i_min = i;
                    j_min = j;
                    prev_row_i_min = prev_row;
                    prev_col_j_min = prev_col;
                    found = TRUE;
                }
                prev_col = curr_col;
            }
            prev_row = curr_row;
        }

        if (! found)
            break;

        ASSERT(i_min != NOT_SET);
        ASSERT(j_min != NOT_SET);

        /* Add X[i_min][j_min] to the basis, and adjust supplies and cost */
        backtrack_node = prev_row_i_min->next;
        ERE( add_basic_variable(i_min, j_min, supply_vp, demand_vp,
                                prev_row_i_min, prev_col_j_min, &row_head) );

        /* Update the necessary delta[][] */
        if (backtrack_node == prev_row_i_min->next)  /* Line i_min was deleted */
        {
            for (curr_col = col_head.next; curr_col != NULL; curr_col = curr_col->next)
            {
                /* int j; */ /* From Yosi */
                j = curr_col->i;

                /* Column j needs updating */
                if (curr_col->val == costs_mp->elements[i_min][j])
                {
                    /* Find the new maximum value in the column */
                    old_val = curr_col->val;
                    curr_col->val = -EMD_INFINITY;
                    for (curr_row = row_head.next; curr_row != NULL; curr_row = curr_row->next)
                    {
                        /* int i; */ /* From Yosi */
                        i = curr_row->i;
                        if (curr_col->val <= costs_mp->elements[i][j])
                            curr_col->val = costs_mp->elements[i][j];
                    }
        
                    /* If needed, adjust the relevant delta[*][j] */
                    diff = old_val - curr_col->val;
                    if (fabs(diff) < EMD_EPSILON * max_cost)
                        for (curr_row=row_head.next; curr_row != NULL; curr_row=curr_row->next)
                            delta->elements[curr_row->i][j] += diff;
                }
            }
        }
        else  /* Column j_min was deleted */
        {
            for (curr_row = row_head.next; curr_row != NULL; curr_row = curr_row->next)
            {
                /* int i; */ /* From Yosi */
                i = curr_row->i;
                if (curr_row->val == costs_mp->elements[i][j_min])  /* Row i needs updating */
                {
                    /* Find the new maximum value in the row */
                    old_val = curr_row->val;
                    curr_row->val = -EMD_INFINITY;
                    for (curr_col=col_head.next; curr_col != NULL; curr_col=curr_col->next)
                    {
                        /* int j; */ /* From Yosi */
                        j = curr_col->i;
                        if(curr_row->val <= costs_mp->elements[i][j])
                            curr_row->val = costs_mp->elements[i][j];
                    }
        
                    /* If needed, adjust the relevant delta[i][*] */
                    diff = old_val - curr_row->val;
                    if (fabs(diff) < EMD_EPSILON * max_cost)
                        for (curr_col=col_head.next; curr_col != NULL; curr_col=curr_col->next)
                            delta->elements[i][curr_col->i] += diff;
                }
            }
        }
    } while (row_head.next != NULL || col_head.next != NULL);

    free_matrix(delta);
    kjb_free(row_list);
    kjb_free(col_list);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/**********************
    add_basic_variable
**********************/
static int add_basic_variable
(
    int      i_min,
    int      j_min,
    Vector*  supply_vp,
    Vector*  demand_vp,
    SLLNode* prev_row_i_min,
    SLLNode* prev_col_j_min,
    SLLNode* row_head
)
{
    extern DLLNode*        end_X;
    extern unsigned char** is_X;
    extern DLLNode**       RowsX_np_ptr;
    extern DLLNode**       ColsX_np_ptr;
    extern double          max_W;
    double   T;

    /* Degenerate case */
    if (    fabs(supply_vp->elements[i_min] - demand_vp->elements[j_min])
         <= (EMD_EPSILON * max_W) )
    {
        T = supply_vp->elements[i_min];
        supply_vp->elements[i_min]  = 0.0;
        demand_vp->elements[j_min] -= T;
    }

    /* Supply exhausted */
    else if (supply_vp->elements[i_min] < demand_vp->elements[j_min])
    {
        T = supply_vp->elements[i_min];
        supply_vp->elements[i_min]  = 0.0;
        demand_vp->elements[j_min] -= T;
    }
    else  /* Demand exhausted */
    {
        T = demand_vp->elements[j_min];
        demand_vp->elements[j_min]  = 0.0;
        supply_vp->elements[i_min] -= T;
    }

    /* X(i_min,j_min) is a basic variable */
    is_X[i_min][j_min] = TRUE;

    end_X->val      = T;
    end_X->i        = i_min;
    end_X->j        = j_min;
    end_X->next_col = RowsX_np_ptr[i_min];
    end_X->next_row = ColsX_np_ptr[j_min];

    RowsX_np_ptr[i_min] = end_X;
    ColsX_np_ptr[j_min] = end_X;
    end_X++;

    /* Delete supply row only if the empty, and if not last row */
    if (supply_vp->elements[i_min] == 0 && row_head->next->next != NULL)
        prev_row_i_min->next = prev_row_i_min->next->next;/* Remove row from list */
    else
        prev_col_j_min->next = prev_col_j_min->next->next;/* Remove col from list */

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/*
// The following functions extend Yosi's earthmover's distance code
// for use with the GAMUT program.
*/


/* ===========================================================================
 *                           euclidean_distance
 *
 * Computes the Euclidean distance between two features.
 *
 * This routine computes the Euclidean distance between two features which are
 * implemented as KJB Vectors. Included as the default distance function used
 * by the earthmover's distance code.
 *
 * Returns:
 *     NO_ERROR on success; ERROR on failure, with a descriptive error message
 *     being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     get_eathmover_distance
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int euclidean_distance
(
    Vector* f1_vp,
    Vector* f2_vp,
    double* distance_ptr
)
{
    int     i;
    double    diff, sum;

    if ( (f1_vp == NULL) || (f2_vp == NULL) )
    {
        set_error("emd_lib.c::euclidean_distance() - One of the input vectors is NULL");
        return ERROR;
    }

    if (f1_vp->length != f2_vp->length)
    {
        set_error("emd_lib.c::euclidean_distance() - Input vectors have different dimensions:");
        add_error(" (f1_vp->length=%d)", f1_vp->length);
        add_error(" (f2_vp->length=%d)", f2_vp->length);
        return ERROR;
    }

    sum = (double)0.0;
    for (i = 0; i < f1_vp->length; i++)
    {
        diff = f1_vp->elements[i] - f2_vp->elements[i];
        sum += (diff * diff);
    }

    *distance_ptr = (double)sqrt((double)sum);

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ===========================================================================
 *                        get_target_signature
 *
 * Gets target signature.
 *
 * This routine implements the creation/over-writing semantics used in
 * the KJB library in the case of signatures. If *target_sig_ptr_ptr
 * is NULL, then this routine creates the signature. If the target signature
 * is not NULL and is the correct size, then this routine does nothing
 * (recycles the memory). If the target signature is the wrong size, then it
 * is freed abd reallocated.
 *
 * Distributions are stored in signatures which consist of an array of
 * features and a corresponding array of feature weights.
 *
 * A "signature" is defined as follows:
 *
 * |     typedef struct Signature
 * |    {
 * |        int      num_features; Number of features in the signature
 * |        Vector** feature_vec;  Pointer to array of features
 * |        double*    weights_vec;  Pointer to array of feature weights
 * |    } Signature;
 *
 * In the emd_lib.c module, a "feature" is simply a KJB Vector. All features
 * should have the same length.
 *
 * Each feature in a signature has a corresponding weight indicating its relative
 * abundance in the distribution. All weights should sum to 1.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     free_signature, copy_signature, set_signature_features,
 *     set_signature_weights, get_signature_from_RGB, get_clustered_data,
 *     convert_spectrum_to_signature
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int get_target_signature
(
    Signature** target_sig_ptr_ptr,
    const int   num_features
)
{
    Signature* out_sig_ptr;
    int        i;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_target_signature() called.\n"));
#endif
#endif

    if (num_features < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    out_sig_ptr = *target_sig_ptr_ptr;

    if (out_sig_ptr == NULL)
    {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Creating new signature\n"));
#endif
#endif

        NRE(out_sig_ptr = TYPE_MALLOC(Signature));
        out_sig_ptr->num_features = num_features;

        NRE(out_sig_ptr->feature_vec = N_TYPE_MALLOC(Vector*, num_features));
        NRE(out_sig_ptr->weights_vec = N_TYPE_MALLOC(double,    num_features));

        for (i = 0; i < num_features; i++)
        {
            out_sig_ptr->feature_vec[i] = NULL;
            out_sig_ptr->weights_vec[i] = DBL_NOT_SET;
        }

        *target_sig_ptr_ptr = out_sig_ptr;
    }
    else if (out_sig_ptr->num_features == num_features)

    {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Reusing exising signature\n"));
#endif
#endif

        if (out_sig_ptr->feature_vec == NULL)
        {
            NRE(out_sig_ptr->feature_vec = N_TYPE_MALLOC(Vector*, num_features));
            for (i = 0; i < num_features; i++)
                out_sig_ptr->feature_vec[i] = NULL;
        }

        if (out_sig_ptr->weights_vec == NULL)
        {
            NRE(out_sig_ptr->weights_vec = N_TYPE_MALLOC(double, num_features));
        }

        for (i = 0; i < num_features; i++)
            out_sig_ptr->weights_vec[i] = DBL_NOT_SET;
    }
    else
    {
        Vector**   temp_feature_vec;
        double*      temp_weights_vec;
        Vector*    old_vp;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Resizing existing signature\n"));
#endif
#endif

        NRE(temp_feature_vec = N_TYPE_MALLOC(Vector*, num_features));
        NRE(temp_weights_vec = N_TYPE_MALLOC(double, num_features));

        for (i = 0; i < num_features; i++)
        {
            temp_feature_vec[i] = NULL;
            temp_weights_vec[i] = DBL_NOT_SET;
        }

        for (i = 0; i < out_sig_ptr->num_features; i++)
        {
            old_vp = out_sig_ptr->feature_vec[i];
            free_vector(old_vp);
        }

        kjb_free(out_sig_ptr->feature_vec);
        kjb_free(out_sig_ptr->weights_vec);

        out_sig_ptr->num_features = num_features;
        out_sig_ptr->feature_vec  = temp_feature_vec;
        out_sig_ptr->weights_vec  = temp_weights_vec;
    }

    ASSERT(out_sig_ptr->num_features == num_features);
    ASSERT(out_sig_ptr->num_features >= 0);
    ASSERT(out_sig_ptr->feature_vec != NULL);
    ASSERT(out_sig_ptr->weights_vec != NULL);

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE((" out_sig_ptr               = %X\n", out_sig_ptr));
    TEST_PSE((" out_sig_ptr->feature_vec  = %X\n", out_sig_ptr->feature_vec));
    TEST_PSE((" out_sig_ptr->weights_vec  = %X\n", out_sig_ptr->weights_vec));
    TEST_PSE((" out_sig_ptr->num_features = %d\n", out_sig_ptr->num_features));
#endif
#endif

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_target_signature() returning.\n"));
#endif
#endif

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* ===========================================================================
 *                              free_signature
 *
 * Frees the memory associated with a signature.
 *
 * This routine frees the storage associated with a signature obtains from
 * get_target_signature. If the argument is NULL, then this routine returns
 * safely without doing anything.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     get_target_signature, copy_signature, set_signature_features,
 *     set_signature_weights, get_signature_from_RGB, get_clustered_data,
 *     convert_spectrum_to_signature
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/

void free_signature(Signature* signature_ptr)
{
    int i;
    Vector* ftr_vp;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    if (signature_ptr == NULL)
        TEST_PSE((" free_signature() - signature is NULL\n"));
    else
        TEST_PSE((" free_signature() - freeing %X\n", signature_ptr));
#endif
#endif

    if (signature_ptr != NULL)
    {
        if (signature_ptr->feature_vec != NULL)
        {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
            TEST_PSE(("   freeing %d features\n", signature_ptr->num_features));
#endif
#endif

            for (i = 0; i < signature_ptr->num_features; i++)
            {
                ftr_vp = signature_ptr->feature_vec[i];
                free_vector(ftr_vp);
                signature_ptr->feature_vec[i] = NULL;
            }

            kjb_free(signature_ptr->feature_vec);
            signature_ptr->feature_vec = NULL;
        }

        if (signature_ptr->weights_vec != NULL)
        {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
            TEST_PSE(("   freeing weights\n"));
#endif
#endif

            kjb_free(signature_ptr->weights_vec);
            signature_ptr->weights_vec = NULL;
        }

        kjb_free(signature_ptr);
        signature_ptr = NULL;
    }
}


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* ===========================================================================
 *                             copy_signature
 *
 * Copies a signature.
 *
 * This routine performs a deep copy of the signature into the one pointed
 * to by output_sig_ptr_ptr. If the signature pointed to by output_sig_ptr_ptr
 * is NULL, then a signature of the appropriate size is created through a call
 * to get_target_signature. If the output signature exists, but is the wrong
 * size, then the associated storage is freed and reallocated. Otherwise, the
 * storage associated with the output signature is recycled.
 *
 * This routine returns ERROR when the following conditions occur:
 * | 1) The signature pointed to by source_sig_ptr is NULL.
 * | 2) Either the source feature vector or weight vector is NULL.
 * | 3) Allocation of the output signature fails on the call to
 * |     get_target_signature.
 * | 4) Copying the feature vector fails on the call to copy_vector.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     get_target_signature, free_signature, set_signature_features,
 *     set_signature_weights, get_signature_from_RGB, get_clustered_data,
 *     convert_spectrum_to_signature
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int copy_signature
(
    Signature**      output_sig_ptr_ptr,
    const Signature* source_sig_ptr
)
{
    int     i;
    Vector* src_ftr_vp;
    Vector* out_ftr_vp;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::copy_signature() called.\n"));
    TEST_PSE((" source_sig_ptr = %X\n", source_sig_ptr));
#endif
#endif

    if (source_sig_ptr == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (   ( source_sig_ptr->feature_vec == NULL )
        || ( source_sig_ptr->weights_vec == NULL )
       )
    {
        kjb_clear_error();

        set_error("emd_lib.c::copy_signature() - Invalid source signature:");

        if ( source_sig_ptr->feature_vec == NULL )
            add_error(" Feature vector is NULL");

        if ( source_sig_ptr->weights_vec == NULL )
            add_error(" Weight vector is NULL");

        return ERROR;
    }

    ERE(get_target_signature(output_sig_ptr_ptr, source_sig_ptr->num_features));

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE((" (*output_sig_ptr_ptr) = %X\n", *output_sig_ptr_ptr));
#endif
#endif

    for (i = 0; i < source_sig_ptr->num_features; i++)
    {
        src_ftr_vp = source_sig_ptr->feature_vec[i];

        if ( src_ftr_vp == NULL)
        {
            set_error("emd_lib.c::copy_signature() - Invalid feature in source signature:");
            add_error(" (source_sig_ptr->feature_vec[%d] = NULL)", i);
            return ERROR;
        }

        out_ftr_vp = (*output_sig_ptr_ptr)->feature_vec[i];
        ERE(copy_vector(&out_ftr_vp, src_ftr_vp));
        (*output_sig_ptr_ptr)->feature_vec[i] = out_ftr_vp;
        (*output_sig_ptr_ptr)->weights_vec[i] = source_sig_ptr->weights_vec[i];
    }

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::copy_signature() returning.\n"));
#endif
#endif

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* ===========================================================================
 *                        set_signature_features
 *
 * Sets the feature vector of the target signature.
 *
 * This routine sets the feature vector of the signature pointed to by
 * target_sig_ptr to the values contained in the input Matrix feature_data_mp.
 * Each row in the input Matrix is copied to a KJB Vector and stored as an
 * element in the signature's feature vector.
 *
 * Preconditions:
 * | 1) The signature pointed to by target_sig_ptr must already be allocated.
 * | 2) The matrix pointed to by feature_data_mp must not be NULL.
 * | 3) The number of features in the target signature must be equal to the
 * |     number of rows in the input Matrix.
 * | 4) The number of columns in the input Matrix must be >= zero.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     get_target_signature, free_signature, copy_signature,
 *     set_signature_weights, get_signature_from_RGB, get_clustered_data,
 *     convert_spectrum_to_signature
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int set_signature_features
(
    Signature*    target_sig_ptr,
    const Matrix* feature_data_mp
)
{
    int i, result = NO_ERROR;

    if (target_sig_ptr == NULL)
    {
        set_error("emd_lib.c::set_signature_features() - Target signature is NULL");
        return ERROR;
    }

    if (target_sig_ptr->feature_vec == NULL)
    {
        set_error("emd_lib.c::set_signature_features() - Invalid target signature:");
        add_error(" Feature vector is NULL");
        return ERROR;
    }

    if (feature_data_mp == NULL)
    {
        set_error("emd_lib.c::set_signature_features() - Input feature data matrix is NULL");
        return ERROR;
    }

    if ( target_sig_ptr->num_features != feature_data_mp->num_rows )
    {
        set_error("emd_lib.c::set_signature_features() - Size mismatch:");
        add_error(" (target_sig_ptr->num_features=%d)", target_sig_ptr->num_features);
        add_error(" (feature_data_mp->num_rows=%d)", feature_data_mp->num_rows);
        return ERROR;
    }

    if (feature_data_mp->num_cols < 0)
    {
        set_error("emd_lib.c::set_signature_features():");
        add_error(" Invalid number of columns in input data matrix");
        add_error(" (feature_data_mp->num_cols=%d)", feature_data_mp->num_cols);
        return ERROR;
    }

    for ( i = 0; i < target_sig_ptr->num_features; i++ )
    {
        result = get_matrix_row(&(target_sig_ptr->feature_vec[i]),
                                feature_data_mp, i);

        if (result == ERROR)
        {
            add_error("emd_lib.c::set_signature_features() - Failed to get matrix data row");
            add_error(" (row index i=%d)", i);
            break;
        }
    }

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* ===========================================================================
 *                        set_signature_weights
 *
 * Sets the weights vector of the target signature.
 *
 * This routine sets the weights vector of the target signature pointed to by
 * target_sig_ptr to the values contained in the input Vector weight_data_vp.
 * The weight data in the target signature will be normalized to sum to 1.
 *
 * Preconditions:
 * | 1) The signature pointed to by target_sig_ptr must already be allocated.
 * | 2) The vector pointed to by weight_data_vp must not be NULL.
 * | 3) The number of features in the target signature must be equal to the
 * |     input weight Vector length.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     free_signature, copy_signature, set_signature_features,
 *     set_signature_features, get_signature_from_RGB, get_clustered_data,
 *     convert_spectrum_to_signature
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/

int set_signature_weights
(
    Signature*    target_sig_ptr,
    const Vector* weight_data_vp
)
{
    int   i;
    double  sum = 0.0;

    if (target_sig_ptr == NULL)
    {
        set_error("emd_lib.c::set_signature_weights() - Target signature is NULL");
        return ERROR;
    }

    if (weight_data_vp == NULL)
    {
        set_error("emd_lib.c::set_signature_weights() - Input weight data vector is NULL");
        return ERROR;
    }

    if ( target_sig_ptr->num_features != weight_data_vp->length )
    {
        set_error("emd_lib.c::set_signature_weights() - Size mismatch:");
        add_error(" (target_sig_ptr->num_features=%d)", target_sig_ptr->num_features);
        add_error(" (weight_data_vp->length=%d)", weight_data_vp->length);
        return ERROR;
    }

    if (target_sig_ptr->weights_vec == NULL)
    {
        set_error("emd_lib.c::set_signature_weights() - Invalid target signature:");
        add_error(" Weight vector is NULL");
        return ERROR;
    }

    for ( i = 0; i < target_sig_ptr->num_features; i++ )
    {
        sum += weight_data_vp->elements[i] ;
        target_sig_ptr->weights_vec[i] = weight_data_vp->elements[i];
    }

    if ( IS_ZERO_DBL(sum) )
    {
        set_error("emd_lib.c::set_signature_weights() - Invalid input weight vector:");
        add_error(" Input weight vector data sums to zero");
        return ERROR;
    }
    else
    {
        for ( i = 0; i < target_sig_ptr->num_features; i++ )
            target_sig_ptr->weights_vec[i] /= sum;
    }

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int put_matrix_data_into_signature
(
    Signature**   target_sig_ptr_ptr,
    const Matrix* source_sig_data_mp
)
{
    /*
    // This function is intended as a helper to "read_signature_db", and is NOT
    // intended as a general purpose routine.
    //
    // The source data matrix has the feature data in the first n-1 columns, and
    // the weight data in column n.
    */

    int        i, j;

    if (source_sig_data_mp == NULL)
    {
        set_error("emd_lib.c::put_matrix_data_into_signature():");
        add_error(" Matrix containing signature data is empty.");
        return ERROR;
    }

    if (source_sig_data_mp->num_rows < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ( get_target_signature(target_sig_ptr_ptr, source_sig_data_mp->num_rows)
         == ERROR )
    {
        add_error("emd_lib.c::put_matrix_data_into_signature():");
        add_error(" Failed to allocate target signature.");
        return ERROR;
    }

    if ((*target_sig_ptr_ptr)->num_features != source_sig_data_mp->num_rows)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    for (i = 0; i < source_sig_data_mp->num_rows; i++)
    {
        Vector* target_ftr_vp = (*target_sig_ptr_ptr)->feature_vec[i];

        ERE(get_zero_vector(&target_ftr_vp, source_sig_data_mp->num_cols - 1));

        for (j = 0; j < (source_sig_data_mp->num_cols - 1); j++)
            target_ftr_vp->elements[j] = source_sig_data_mp->elements[i][j];

        (*target_sig_ptr_ptr)->feature_vec[i] = target_ftr_vp;

        (*target_sig_ptr_ptr)->weights_vec[i]
            = source_sig_data_mp->elements[i][source_sig_data_mp->num_cols - 1];
    }

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int put_signature_data_into_matrix
(
    Matrix**         target_sig_data_mpp,
    const Signature* source_sig_ptr
)
{
    /*
    // This function is intended as a helper to "write_signature_db", and is NOT
    // intended as a general purpose routine.
    //
    // The output data matrix has the feature data in the first n-1 columns, and
    // the weight data in column n.
    */

    Matrix* temp_mp = NULL;
    int     i, j, num_dim, result;

    if (source_sig_ptr == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (   (source_sig_ptr->feature_vec == NULL)
        || (source_sig_ptr->feature_vec[0] == NULL) )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_dim = (source_sig_ptr->feature_vec[0])->length;

    /* Extra column in output matrix is for weight values */
    result = get_zero_matrix(&temp_mp, source_sig_ptr->num_features, num_dim + 1);
    if (result == ERROR)
    {
        add_error("Unable to allocate matrix to hold signature data.");
        return ERROR;
    }

    if (   (temp_mp->num_rows != source_sig_ptr->num_features)
        || (temp_mp->num_cols != num_dim + 1)                  )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    for (i = 0; i < source_sig_ptr->num_features; i++)
    {
        for (j = 0; j < num_dim; j++)
            temp_mp->elements[i][j]
                = (source_sig_ptr->feature_vec[i])->elements[j];

        temp_mp->elements[i][num_dim] = source_sig_ptr->weights_vec[i];
    }

    result = copy_matrix(target_sig_data_mpp, temp_mp);

    if (result == ERROR)
        add_error("Unable to copy temporary matrix to output.");

    free_matrix(temp_mp);

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int get_signature_from_RGB
(
    Signature**        output_sig_ptr_ptr,
    int                num_features,
    Matrix*            data_mp,
    int                (*distance_fn) (Vector *, Vector *, double *),
    Emd_cluster_method cluster_method
)
{
    int result         = NO_ERROR;
    Matrix* RGB_mp     = NULL;
    Vector* weights_vp = NULL;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_signature_from_RGB() called.\n"));
#endif
#endif

    if (num_features < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (data_mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ( data_mp->num_rows < num_features )
    {
        set_error("emd_lib.c::get_signature_from_RGB():");
        add_error(" Not enough rows in input matrix to generate %d features",
                   num_features);
        add_error(" (data_mp->num_rows=%d)", data_mp->num_rows);
        add_error(" (num_features     =%d)", num_features);
        return ERROR;
    }

    result = get_clustered_data(cluster_method, data_mp, num_features,
                                distance_fn,
                                &RGB_mp, &weights_vp);

    if ( result == ERROR )
    {
        add_error("Unable to obtain clustered data from input matrix");
        return ERROR;
    }

    /* Depending on the clustering algorithm, we cannot count on the returned
    // cluster centres having "num_features" clusters.
    */
    if (RGB_mp->num_rows == num_features)
        /* We got the specified number of features. */
        result = get_target_signature(output_sig_ptr_ptr, num_features);
    else
        result = get_target_signature(output_sig_ptr_ptr, RGB_mp->num_rows);

    if (result == ERROR)
    {
        add_error("Unable to obtain signature");
        return ERROR;
    }

    result = set_signature_features(*output_sig_ptr_ptr, RGB_mp);

    if ( result == ERROR )
        add_error("Unable to update signature features");

    if ( result == NO_ERROR )
    {
        result = set_signature_weights(*output_sig_ptr_ptr, weights_vp);

        if (result == ERROR)
            add_error("Unable to update signature weights");
    }

    free_matrix(RGB_mp);
    free_vector(weights_vp);

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_signature_from_RGB() returning.\n"));
#endif
#endif

    return result;
}
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

void verbose_psig(int cut_off, Signature* sig_ptr)
{
    Vector* cur_ftr_vp;
    int     i, j;

    if (cut_off > kjb_get_verbose_level ())
        return;

    if (sig_ptr == NULL)
    {
        verbose_pso(cut_off, "Signature is NULL\n");
        return;
    }

    verbose_pso(cut_off, "Signature contains %d features:\n", sig_ptr->num_features);
    verbose_pso(cut_off, " Features:\n");

    if (sig_ptr->feature_vec == NULL)
    {
        verbose_pso(cut_off, "  Feature vector is NULL\n");
    }
    else
    {
        for (i = 0; i < sig_ptr->num_features; i++)
        {
            cur_ftr_vp = sig_ptr->feature_vec[i];

            if (cur_ftr_vp == NULL)
            {
                pso("<< %2d >>   %3d:     NULL\n", cut_off, i);
                continue;
            }
            else
            {
                if (cur_ftr_vp->length < 1)
                    pso("<< %2d >>   %3d: Zero length\n", cut_off, i);
                else
                {
                    pso("<< %2d >>   %3d: %8.4f", cut_off, i, cur_ftr_vp->elements[0]);

                    for (j = 1; j < cur_ftr_vp->length; j++)
                    {
                        pso(" %8.4f", cur_ftr_vp->elements[j]);
                    }
                    pso("\n");
                }
            }
        }
    }

    verbose_pso(cut_off, " Weights:\n");
    if (sig_ptr->weights_vec == NULL)
    {
        verbose_pso(cut_off, "  Weight vector is NULL\n");
    }
    else
    {
        for (i = 0; i < sig_ptr->num_features; i++)
            pso("<< %2d >>   %3d: %8.4f\n", cut_off, i, sig_ptr->weights_vec[i]);
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int get_clustered_data
(
    Emd_cluster_method cluster_method, /* Clustering method to use        */
    Matrix*            input_data_mp,  /* Input data matrix to cluster    */
    int                num_clusters,   /* Number of clusters to generate  */
    int                (*distance_fn) (Vector *, Vector *, double *),
    Matrix**           clusters_mpp,   /* Output clustered data           */
    Vector**           weights_vpp     /* Output cluster weights          */
)
{
    int result;

    switch( cluster_method )
    {
      case EMD_NO_CLUSTERING:
          result = get_unclustered_data(input_data_mp,
                                      clusters_mpp,
                                      weights_vpp);
          break;
      case EMD_2D_HISTOGRAM:
          TEST_PSE(("Sorry, chromticity clustering method is unavailable"));
          result = ERROR;
          break;
      case EMD_3D_HISTOGRAM:
          TEST_PSE(("Sorry, 3D fixed bin clustering method is unavailable"));
          result = get_3D_histogram_clusters(input_data_mp,
                                             clusters_mpp, weights_vpp);
          break;
      case EMD_K_MEANS:
          result = get_kmeans_clusters(input_data_mp, num_clusters, distance_fn,
                                       clusters_mpp, weights_vpp, NULL);
          break;
      case EMD_CLARANS:
          TEST_PSE(("Sorry, CLARANS clustering method is unavailable"));
          result = ERROR;
          break;
      default:
          SET_ARGUMENT_BUG();
          result = ERROR;
    }

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int get_unclustered_data
(
    Matrix*  input_data_mp,
    Matrix** clusters_mpp,
    Vector** weights_vpp
)
{
    int i, num_clusters, result;

    if (input_data_mp == NULL)
    {
        set_error("emd_lib.c::get_unclustered_data():");
        add_error(" Input RGB matrix is NULL");
        return ERROR;
    }

    num_clusters = input_data_mp->num_rows;

    result = copy_matrix(clusters_mpp, input_data_mp);

    if (result == ERROR)
    {
        set_error("emd_lib.c::get_unclustered_data():");
        add_error(" Failed to copy input data matrix");
    }
    else
    {
        result = get_zero_vector(weights_vpp, num_clusters);
        if (result == ERROR)
        {
            set_error("emd_lib.c::get_unclustered_data():");
            add_error(" Failed to allocate weight vector");
        }
        else
        {
            for (i = 0; i < num_clusters; i++)
                (*weights_vpp)->elements[i] = (double)(1.0 / (double)num_clusters);
        }
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int convert_spectrum_to_signature
(
    Signature** target_sig_ptr_ptr,
    Spectra*    source_sp,
    int         spectrum_index
)
{
    int  i, num_spectra, num_measurements;
    double wavelength, power, sum_power;

    if (source_sp == NULL)
    {
        set_error("convert_spectrum_to_signature() - Input spectra is empty");
        return ERROR;
    }

    num_spectra      = source_sp->spectra_mp->num_rows;
    num_measurements = source_sp->spectra_mp->num_cols;

    if (   (spectrum_index < 0)
        || (spectrum_index >= num_spectra) )
    {
        set_error("convert_spectrum_to_signature() - Invalid spectrum index");
        add_error("  (spectrum_index=%d)", spectrum_index);
        add_error("  (num_spectra   =%d)", num_spectra);
        return ERROR;
    }

    if (get_target_signature(target_sig_ptr_ptr, num_measurements) == ERROR)
    {
        set_error("convert_spectrum_to_signature() - Failed to allocate signature");
        return ERROR;
    }

    wavelength = (double)source_sp->offset;
    sum_power  = (double)0.0;

    for ( i = 0; i < num_measurements; i++ )
    {
        Vector* target_ftr_vp = NULL;

        ERE(get_zero_vector(&target_ftr_vp, 1));
        target_ftr_vp->elements[0] = (double)wavelength;
        (*target_sig_ptr_ptr)->feature_vec[i] = target_ftr_vp;

        power = source_sp->spectra_mp->elements[spectrum_index][i];
        (*target_sig_ptr_ptr)->weights_vec[i] = power;

        wavelength += (double)source_sp->step;
        sum_power  += power;
    }

    /* Normalize each spectrum such that it's sum is 1.0 */
    for ( i = 0; i < num_measurements; i++ )
    {
        (*target_sig_ptr_ptr)->weights_vec[i]
            = (*target_sig_ptr_ptr)->weights_vec[i] / sum_power;
    }

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ===========================================================================
 *                        get_target_signature_db
 *
 * Gets target signature database.
 *
 * This routine implements the creation/over-writing semantics used in
 * the KJB library in the case of signature databases.
 * If *target_sig_db_ptr_ptr is NULL, then this routine creates the signature
 * database. If the target signature database is not NULL and is the correct
 * size, then this routine does nothing (recycles the memory). If the target
 * signature database is the wrong size, then it is resized.
 *
 * A signature database is defined as follows:
 *
 * |  typedef struct Signature_db
 * |  {
 * |    int         num_signatures;        Number of signatures in database
 * |    Signature** signature_ptr_vec;     Array of pointers to signatures
 * |    Emd_cluster_method cluster_method; Method used to make signatures
 * |    Emd_data_origin    data_origin;    Illuminant data origin (spectra/RGB)
 * |    Emd_illum_data     illum_data;     Either Spectra or RGB Matrix
 * |  } Signature_db;
 *
 * The following clustering methods are currently defined:
 * |  EMD_NO_CLUSTERING  Data is not clustered.
 * |  EMD_2D_HISTOGRAM   Data is clustered in rg-chromaticity space using
 * |                      a 2-D histogram (not currently implemented).
 * |  EMD_3D_HISTOGRAM   Data is clustered in RGB space using a 3-D histogram.
 * |  EMD_K_MEANS        Data is clustered using the k-means algorithm.
 * |  EMD_CLARANS        Data is clustered using the CLARANS algorithm
 * |                      (not currently implemented).
 *
 * Each signature database contains the illuminant data from which it was
 * constructed. The illuminant data can be in one of two forms:
 * | 1) a KJB Spectra containing the illuminant spectra, or
 * | 2) a KJB Matrix containing the RGB of each illuminant.
 *
 * The data_origin field of the Signature_db indicates the storage format of
 * the illuminant data and can have the following values:
 * |  EMD_FROM_RGB      The illum_data field contains a Matrix of RGBs.
 * |  EMD_FROM_SPECTRA  The illum_data field contains a Spectra of illuminants.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Authors:
 *     Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Related:
 *     free_signature_db, copy_signature_db, add_signature_to_db,
 *     get_signature_from_db, set_signature_db_illum_data,
 *     get_signature_db_illum_data, read_signature_db, write_signature_db
 *
 * Index: earthmover
 *
 * -----------------------------------------------------------------------------
*/
int get_target_signature_db
(
    Signature_db** target_sig_db_ptr_ptr, /* Double pointer to output database*/
    const int      num_signatures         /* Number of signatures in database */
)
{
    Signature_db* out_sig_db;
    Signature**   temp_sig_ptr_vec;
    int i;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_target_signature_db() called.\n"));
#endif
#endif

    if (num_signatures < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    out_sig_db = *target_sig_db_ptr_ptr;

    if (out_sig_db == NULL)
    {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Creating new signature database:\n"));
#endif
#endif

        /* Create a new target database from scratch */
        NRE(out_sig_db = TYPE_MALLOC(Signature_db));
        out_sig_db->num_signatures = num_signatures;

        NRE(temp_sig_ptr_vec = N_TYPE_MALLOC(Signature*, num_signatures));

        for (i = 0; i < num_signatures; i++)
            temp_sig_ptr_vec[i] = NULL;

        out_sig_db->signature_ptr_vec = temp_sig_ptr_vec;
        out_sig_db->data_origin = EMD_FROM_RGB;
        out_sig_db->illum_data.illum_mp = NULL;

        *target_sig_db_ptr_ptr = out_sig_db;
    }
    else if (out_sig_db->num_signatures == num_signatures)
    {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Reusing existing signature database:\n"));
#endif
#endif

        /* Target database is correct size. Reuse the arrays */
        if (out_sig_db->signature_ptr_vec == NULL)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        for (i = 0; i < num_signatures; i++)
        {
            if (out_sig_db->signature_ptr_vec[i] != NULL)
                free_signature(out_sig_db->signature_ptr_vec[i]);

            out_sig_db->signature_ptr_vec[i] = NULL;
        }
    }
    else
    {

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE((" Resizing existing signature database:\n"));
#endif
#endif

        /* Target database is wrong size. Free and reallocate arrays */
        for (i = 0; i < num_signatures; i++)
        {
            if (out_sig_db->signature_ptr_vec[i] != NULL)
                free_signature(out_sig_db->signature_ptr_vec[i]);

            out_sig_db->signature_ptr_vec[i] = NULL;
        }

        NRE(temp_sig_ptr_vec = N_TYPE_MALLOC(Signature*, num_signatures));

        kjb_free(out_sig_db->signature_ptr_vec);

        out_sig_db->signature_ptr_vec = temp_sig_ptr_vec;
    }

    out_sig_db->num_signatures = num_signatures;
    out_sig_db->cluster_method = EMD_NO_CLUSTERING; /* default */

    if (    (out_sig_db->data_origin == EMD_FROM_RGB)
         && (out_sig_db->illum_data.illum_mp != NULL)
       )
    {
        free_matrix(out_sig_db->illum_data.illum_mp);
    }
    else if (    (out_sig_db->data_origin == EMD_FROM_SPECTRA)
              && (out_sig_db->illum_data.illum_sp != NULL)
            )
    {
        free_spectra(out_sig_db->illum_data.illum_sp);
    }

    out_sig_db->data_origin = EMD_FROM_RGB;
    out_sig_db->illum_data.illum_mp = NULL;

    ASSERT( (*target_sig_db_ptr_ptr) != NULL );
    ASSERT( (*target_sig_db_ptr_ptr)->signature_ptr_vec != NULL );
    ASSERT( (*target_sig_db_ptr_ptr)->num_signatures == num_signatures );

    ASSERT(   (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_RGB
           || (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_SPECTRA );

#ifdef TEST
    if ( (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_RGB )
        ASSERT( (*target_sig_db_ptr_ptr)->illum_data.illum_mp == NULL );
    if ( (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_SPECTRA )
        ASSERT( (*target_sig_db_ptr_ptr)->illum_data.illum_sp == NULL );
#endif

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("  *target_sig_db_ptr_ptr                     = %X\n", *target_sig_db_ptr_ptr));
    TEST_PSE((" (*target_sig_db_ptr_ptr)->signature_ptr_vec = %X\n", (*target_sig_db_ptr_ptr)->signature_ptr_vec));
    TEST_PSE((" (*target_sig_db_ptr_ptr)->num_signatures    = %d\n", (*target_sig_db_ptr_ptr)->num_signatures));
#endif
#endif

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_target_signature_db() returning.\n"));
#endif
#endif

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

void free_signature_db(Signature_db* sig_db_ptr)
{

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    if (sig_db_ptr != NULL)
    {
        TEST_PSE(("emd_lib.c::free_signature_db() - %d features:\n", sig_db_ptr->num_signatures));
        TEST_PSE(("sig_db_ptr = %X\n", sig_db_ptr));
    }
    else
        TEST_PSE(("emd_lib.c::free_signature_db() - database is empty. Skipping free.\n"));
#endif
#endif

    if (sig_db_ptr != NULL)
    {
        int i;
        Signature* sig_in_db_ptr;

        for (i = 0; i < sig_db_ptr->num_signatures; i++)
        {
            sig_in_db_ptr = sig_db_ptr->signature_ptr_vec[i];

            if (sig_in_db_ptr != NULL)
            {
                free_signature(sig_in_db_ptr);
                sig_in_db_ptr = NULL;
            }

        }

        kjb_free(sig_db_ptr->signature_ptr_vec);
        sig_db_ptr->signature_ptr_vec = NULL;

        if (sig_db_ptr->data_origin == EMD_FROM_RGB)
        {
            free_matrix(sig_db_ptr->illum_data.illum_mp);
            sig_db_ptr->illum_data.illum_mp = NULL;
        }
        else if  (sig_db_ptr->data_origin == EMD_FROM_SPECTRA)
        {
            free_spectra(sig_db_ptr->illum_data.illum_sp);
            sig_db_ptr->illum_data.illum_sp = NULL;
        }

        kjb_free(sig_db_ptr);
        sig_db_ptr = NULL;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
        TEST_PSE(("Done freeing signature db.\n"));
#endif
#endif

    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */


int copy_signature_db
(
    Signature_db**      target_sig_db_ptr_ptr,
    const Signature_db* source_sig_db_ptr
)
{
    Signature* temp_sig_ptr = NULL;
    int i, result = NO_ERROR;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::copy_signature_db() called.\n"));
#endif
#endif

    if (source_sig_db_ptr == NULL)
    {
        free_signature_db(*target_sig_db_ptr_ptr);
        *target_sig_db_ptr_ptr = NULL;
        return NO_ERROR;
    }

    ERE(get_target_signature_db(target_sig_db_ptr_ptr,
                                source_sig_db_ptr->num_signatures));

    (*target_sig_db_ptr_ptr)->cluster_method = source_sig_db_ptr->cluster_method;
    (*target_sig_db_ptr_ptr)->data_origin    = source_sig_db_ptr->data_origin;

    for (i = 0; i < source_sig_db_ptr->num_signatures; i++)
    {
        result = get_signature_from_db(source_sig_db_ptr, i, &temp_sig_ptr);
        if (result == ERROR)
        {
            add_error("Failed to get signature %d from source database", i);
            break;
        }

        result = add_signature_to_db(*target_sig_db_ptr_ptr, i,
                                     temp_sig_ptr);
        if (result == ERROR)
        {
            add_error("Failed to add signature %d to target database", i);
            break;
        }
    }

    if (result == NO_ERROR)
    {
        Emd_data_origin source_data_origin;
        Emd_illum_data  source_illum_data;

        result = get_signature_db_illum_data(source_sig_db_ptr,
                                             &source_data_origin,
                                             &source_illum_data);
        if (result == ERROR)
        {
            add_error("Failed to get illuminant data from source signature database");
        }
        else
        {
            result = set_signature_db_illum_data(*target_sig_db_ptr_ptr,
                                                 source_data_origin,
                                                 &source_illum_data);
            if (result == ERROR)
            {
                add_error("Failed to copy illuminant data to target signature database");
            }
        }

        ASSERT( (*target_sig_db_ptr_ptr) != NULL );
        ASSERT( (*target_sig_db_ptr_ptr)->signature_ptr_vec != NULL );
        ASSERT( (*target_sig_db_ptr_ptr)->num_signatures == source_sig_db_ptr->num_signatures );
        ASSERT( (*target_sig_db_ptr_ptr)->cluster_method == source_sig_db_ptr->cluster_method );
        ASSERT( (*target_sig_db_ptr_ptr)->data_origin    == source_sig_db_ptr->data_origin );

        ASSERT(   (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_RGB
               || (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_SPECTRA );

#ifdef TEST
        if ( (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_RGB )
            ASSERT( (*target_sig_db_ptr_ptr)->illum_data.illum_mp != NULL );
        if ( (*target_sig_db_ptr_ptr)->data_origin == EMD_FROM_SPECTRA )
            ASSERT( (*target_sig_db_ptr_ptr)->illum_data.illum_sp != NULL );
#endif

    }

    free_signature(temp_sig_ptr);

#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::copy_signature_db() returning.\n"));
# endif
#endif

    return result;
}


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int add_signature_to_db
(
    Signature_db*    sig_db_to_update_ptr, /* Pointer to database to update  */
    const int        sig_index,            /* Index of signature to add      */
    const Signature* sig_to_add_ptr        /* Pointer of signature to add    */
)
{
    Signature* temp_sig_ptr;

#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::add_signature_to_db() called.\n"));
    TEST_PSE((" sig_index            = %d\n", sig_index));
    TEST_PSE((" sig_db_to_update_ptr = %X\n", sig_db_to_update_ptr));
    TEST_PSE((" sig_to_add_ptr       = %X\n", sig_to_add_ptr));
# endif
#endif

    if (sig_db_to_update_ptr == NULL)
    {
        set_error("emd_lib.c::add_signature_to_db() - Signature database is NULL");
        return ERROR;
    }

    if(sig_index >= sig_db_to_update_ptr->num_signatures)
    {
        set_error("emd_lib.c::add_signature_to_db() - Invalid database index:");
        add_error(" (sig_index=%d)", sig_index);
        add_error(" (sig_db_to_update_ptr->num_signatures=%d)", sig_db_to_update_ptr->num_signatures);
        return ERROR;
    }

    if (sig_to_add_ptr == NULL)
    {
        set_error("emd_lib.c::add_signature_to_db() - Signature to add is NULL");
        return ERROR;
    }

    if (   (sig_to_add_ptr->feature_vec == NULL)
        || (sig_to_add_ptr->weights_vec == NULL) )
    {
        set_error("emd_lib.c::add_signature_to_db() - Invalid input signature:");

        if (sig_to_add_ptr->feature_vec == NULL)
            add_error(" Feature vector is NULL");

        if (sig_to_add_ptr->weights_vec == NULL)
            add_error(" Weight vector is NULL");

        return ERROR;
    }
    /*
      temp_sig_ptr = sig_db_to_update_ptr->signature_ptr_vec[sig_index];

      ERE(copy_signature( &temp_sig_ptr, sig_to_add_ptr ));

     sig_db_to_update_ptr->signature_ptr_vec[sig_index] = temp_sig_ptr;
    */
    ERE(copy_signature( &(sig_db_to_update_ptr->signature_ptr_vec[sig_index]),
                        sig_to_add_ptr ));
    temp_sig_ptr = sig_db_to_update_ptr->signature_ptr_vec[sig_index];
    ASSERT(temp_sig_ptr != NULL);

#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::add_signature_to_db(index %d) returning.\n",
              sig_index));
# endif
#endif

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int get_signature_from_db
(
    const Signature_db* sig_db_to_query_ptr, /* Pointer to database to query */
    const int           sig_index,           /* Index of signature to get   */
    Signature**         output_sig_ptr_ptr  /* Double pointer of output signature */
)
{
    Signature*    source_sig_ptr;

#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::get_signature_from_db() called.\n"));
    TEST_PSE((" sig_index           = %d\n", sig_index));
    TEST_PSE((" sig_db_to_query_ptr = %X\n", sig_db_to_query_ptr));
    TEST_PSE((" *output_sig_ptr_ptr = %X\n", *output_sig_ptr_ptr));
# endif
#endif

    if (sig_db_to_query_ptr == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (sig_index >= sig_db_to_query_ptr->num_signatures)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_sig_ptr = sig_db_to_query_ptr->signature_ptr_vec[sig_index];

    if (source_sig_ptr == NULL)
    {
        set_error("emd_lib.c::get_signature_from_db()");
        add_error(" Database signature %d is NULL", sig_index);
        return ERROR;
    }

    if (copy_signature(output_sig_ptr_ptr, source_sig_ptr) == ERROR)
    {
        add_error("emd_lib.c::get_signature_from_db():");
        add_error(" Failed to copy database signature %d", sig_index);
        return ERROR;
    }

#ifdef PROGRAMMER_IS_colour
# ifdef EMD_CHECK_LEAKS
    TEST_PSE((" *output_sig_ptr_ptr = %X (updated)\n", *output_sig_ptr_ptr));
    TEST_PSE(("emd_lib.c::get_signature_from_db(index %d) returning.\n",
              sig_index));
# endif
#endif

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int set_signature_db_illum_data
(
    Signature_db*   target_sig_db_ptr,
    Emd_data_origin data_origin,
    Emd_illum_data* illum_data_ptr
)
{
    int result = NO_ERROR;

    if (   (target_sig_db_ptr == NULL)
        || (illum_data_ptr == NULL)   )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    target_sig_db_ptr->data_origin = data_origin;

    if (data_origin == EMD_FROM_RGB)
    {
        result = copy_matrix( &(target_sig_db_ptr->illum_data.illum_mp),
                              illum_data_ptr->illum_mp );
        if (result == ERROR)
        {
            add_error("set_signature_db_illum_data() - Failed to copy illuminant matrix");
            return ERROR;
        }
    }
    else if (data_origin == EMD_FROM_SPECTRA)
    {
        result =  copy_spectra( &(target_sig_db_ptr->illum_data.illum_sp),
                                illum_data_ptr->illum_sp );
        if (result == ERROR)
        {
            add_error("set_signature_db_illum_data() - Failed to copy illuminant spectra");
            result = ERROR;
        }
    }
    else
    {
        set_error("set_signature_db_illum_data() - Invalid signature data origin");
        result = ERROR;
    }

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int get_signature_db_illum_data
(
    const Signature_db* source_sig_db_ptr, /* Pointer to database to query */
    Emd_data_origin*    illum_data_origin_ptr, /* Output data origin type */
    Emd_illum_data*     illum_data_ptr /* Pointer to output illum data */
)
{
    if (    (source_sig_db_ptr     == NULL)
         || (illum_data_origin_ptr == NULL)
         || (illum_data_ptr        == NULL)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (source_sig_db_ptr->data_origin != EMD_FROM_RGB    )
         && (source_sig_db_ptr->data_origin != EMD_FROM_SPECTRA)
       )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    if (source_sig_db_ptr->data_origin == EMD_FROM_RGB)
        illum_data_ptr->illum_mp = source_sig_db_ptr->illum_data.illum_mp;
    else
        illum_data_ptr->illum_sp = source_sig_db_ptr->illum_data.illum_sp;

    *illum_data_origin_ptr = source_sig_db_ptr->data_origin;

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int read_signature_db
(
    Signature_db** target_sig_db_ptr_ptr, /* Dbl pointer to signature db */
    const char*    file_name, /* Name of file to read from   */
    const char*    error_msg /* Message to post on error    */
)
{
    FILE* fp;
    int                num_signatures = NOT_SET;
    Emd_cluster_method cluster_method = EMD_CLUSTER_METHOD_NOT_SET;
    Emd_data_origin    data_origin    = EMD_DATA_ORIGIN_NOT_SET;
    Matrix*            sig_data_mp    = NULL;
    Signature*         sig_ptr        = NULL;
    Matrix*            illum_mp       = NULL;
    Spectra*           illum_sp       = NULL;
    int                i;
    int                result = NO_ERROR;


    /*
    // Kobus--Untested since slight modifications for new calling conventions
    // on matrix/spectra reads.
    */
    UNTESTED_CODE();

    if ((file_name == NULL) || (file_name[ 0 ] == '\0'))
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    /*
    // Kobus: This wont work now that this is a library routine. Perhaps things
    // need to be re-thought a bit? For now, I will just use a diffenent routine
    // name.
    //
    // fp = open_gamut_config_file(NULL, file_name, error_msg);
    //
    */
    fp = open_config_file((char*)NULL, (char*)NULL, file_name, error_msg);

    if (fp == NULL)
    {
        return ERROR;
    }

    result = fp_read_signature_db_header(fp, &num_signatures,
                                         &cluster_method, &data_origin);
#ifdef PROGRAMMER_IS_colour
#ifdef DEBUG_DB_IO
    verbose_pso(500,"read_signature_db() got the following data file header info:\n");
    verbose_pso(500,"  num_signatures: %d\n", num_signatures);
    verbose_pso(500,"  cluster_method: %d\n", cluster_method);
    verbose_pso(500,"  data_origin   : %d\n", data_origin);
#endif
#endif

    if (result == ERROR)
    {
        add_error("Failed to read signature database header from file \"%s\".",
                  file_name);
        return ERROR;
    }

    if (   (num_signatures == NOT_SET)
        || (num_signatures < 1)       )
    {
        set_error("Invalid number of signatures (emd_nos=%d) in database header.",
                  num_signatures);
        return ERROR;
    }

    if ((int)cluster_method == NOT_SET)
    {
        set_error("Invalid clustering method (emd_clm=%d) in database header.",
                  (int)cluster_method);
        return ERROR;
    }

    if ((int)data_origin == NOT_SET)
    {
        set_error("Invalid illuminant data origin (emd_ido=%d) in database header.",
                  (int)data_origin);
        return ERROR;
    }

    /* Allocate and initialize the signature database */
    if (get_target_signature_db(target_sig_db_ptr_ptr, num_signatures) == ERROR)
    {
        add_error("Unable to allocate signature database.");
        return ERROR;
    }

    (*target_sig_db_ptr_ptr)->data_origin   = data_origin;
    (*target_sig_db_ptr_ptr)->cluster_method = cluster_method;

#ifdef PROGRAMMER_IS_colour
#ifdef DEBUG_DB_IO
    pso("\n============================================================\n");
    pso("read_signature_db(): Before matrix reads:\n");
    dbx( (*target_sig_db_ptr_ptr)                      );
    dbx( (*target_sig_db_ptr_ptr)->signature_ptr_vec   );
    dbx( (*target_sig_db_ptr_ptr)->illum_data.illum_mp );
    dbx( (*target_sig_db_ptr_ptr)->illum_data.illum_sp );
#endif
#endif

    /* Read in each of the signatures */
    for (i = 0; i < num_signatures; i++)
    {
        result = fp_read_matrix_with_header(&sig_data_mp, fp);

        if (result == ERROR)
        {
            add_error("Unable to read data for signature %d from file.", i);
            break;
        }

#ifdef PROGRAMMER_IS_colour
#ifdef DEBUG_DB_IO
        pso("Matrix Data for signature %d:\n", i);
        db_mat(sig_data_mp);
        pso("\n");
#endif
#endif

        if (put_matrix_data_into_signature(&sig_ptr, sig_data_mp) == ERROR)
        {
            add_error("Unable to copy matrix containing data into signature.");
            result = ERROR;
            break;
        }

        if (add_signature_to_db(*target_sig_db_ptr_ptr, i, sig_ptr) == ERROR)
        {
            add_error("Unable to update signature database entry %d.", i);
            result = ERROR;
            break;
        }

        free_matrix(sig_data_mp);
        sig_data_mp = NULL;
    }

    if (result == NO_ERROR)
    {
        Emd_illum_data illum_data;

        /*
        // Read in the illuminant database as either a matrix of RGBs,
        //  or as illuminant spectra.
        */

        if (data_origin == EMD_FROM_RGB)
        {

#ifdef PROGRAMMER_IS_colour
#ifdef DEBUG_DB_IO
            pso("\nDEBUG: read_signature_db(): I think that the illums are RGB (matrix)\n");
#endif
#endif

            result = get_zero_matrix(&illum_mp, num_signatures, 3);

            if (result == ERROR)
            {
                set_error("read_signature_db() - llluminant data error:");
                add_error("  Unable to allocate matrix for illuminant RGB data");
            }
            else
            {
                result = fp_ow_read_matrix_by_rows(illum_mp, fp);

                if (result == ERROR)
                {
                    set_error("read_signature_db() - llluminant data error:");
                    add_error("  Unable to read illuminant RGB matrix from signature database file.");
                }
                else
                {
                    illum_data.illum_mp = illum_mp;

                    result = set_signature_db_illum_data(*target_sig_db_ptr_ptr,
                                                         data_origin,
                                                         &illum_data);

                    if (result == ERROR)
                    {
                        set_error("read_signature_db() - llluminant data error:");
                        add_error("  Unable to copy illuminant RGB matrix into signature database.");
                    }
                }
            }
        }
        else if (data_origin == EMD_FROM_SPECTRA)
        {

#ifdef PROGRAMMER_IS_colour
#ifdef DEBUG_DB_IO
            pso("\nDEBUG: read_signature_db(): I think that the illums are spectra\n");
#endif
#endif

            result = fp_read_spectra(&illum_sp, fp);

            if (illum_sp == NULL)
            {
                set_error("read_signature_db() - Illuminant data error:");
                add_error("  Unable to read illuminant spectra from signature database file.");
            }
            else
            {
                illum_data.illum_sp = illum_sp;

                result = set_signature_db_illum_data(*target_sig_db_ptr_ptr,
                                                     data_origin,
                                                     &illum_data);

                if (result == ERROR)
                {
                    set_error("read_signature_db() - Illuminant data error:");
                    add_error("  Unable to copy illuminant spectra into signature database.");
                }
            }
        }
    }

    free_matrix(sig_data_mp);
    free_signature(sig_ptr);
    free_matrix(illum_mp);
    free_spectra(illum_sp);

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

int write_signature_db
(
    const char*         file_name,
    const Signature_db* src_db_ptr
)
{
    FILE*      fp;
    Signature* sig_from_db_ptr = NULL;
    Matrix*    sig_data_mp     = NULL;
    Vector*    num_features_vp = NULL;
    int        i, result = NO_ERROR;

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::write_signature_db() called.\n"));
#endif
#endif

    fp = kjb_fopen(file_name, "w");

    if (fp == NULL)
    {
        add_error("Could not open file \"%s\" for signature database output.",
                  file_name);
        return ERROR;
    }

    if (src_db_ptr == NULL)
    {
        set_error("Cannot write signature database to \"%s\".",
                  file_name);
        return ERROR;
    }

    ERE(fp_write_signature_db_header(fp, src_db_ptr));

    ERE(get_zero_vector(&num_features_vp, src_db_ptr->num_signatures));

    for (i = 0; i < src_db_ptr->num_signatures; i++)
    {
        num_features_vp->elements[i]
            = (src_db_ptr->signature_ptr_vec[i])->num_features;
    }

    for (i = 0; i < src_db_ptr->num_signatures; i++)
    {

        result = get_signature_from_db(src_db_ptr, i, &sig_from_db_ptr);
        if ( result == ERROR )
        {
            add_error("Unable to fetch signature %d from database.",  i);
            return ERROR;
        }

        result = put_signature_data_into_matrix(&sig_data_mp, sig_from_db_ptr);
        if (result == ERROR)
        {
            add_error("Unable to copy signature data into matrix for writing.");
            break;
        }

        result = fp_write_matrix_with_header(sig_data_mp, fp);

        if (result == ERROR)
        {
            add_error("Unable to write signature to file.");
            break;
        }

    }

    if (result == NO_ERROR)
    {

        /* Write the associated illuminant database to the file. */
        if (src_db_ptr->data_origin == EMD_FROM_RGB)
        {
            result = fp_write_matrix_with_header(src_db_ptr->illum_data.illum_mp, fp);
        }
        else if (src_db_ptr->data_origin == EMD_FROM_SPECTRA)
        {
            result = fp_write_spectra(src_db_ptr->illum_data.illum_sp, fp);
        }
    }

    free_vector(num_features_vp);
    free_matrix(sig_data_mp);
    free_signature(sig_from_db_ptr);

#ifdef PROGRAMMER_IS_colour
#ifdef EMD_CHECK_LEAKS
    TEST_PSE(("emd_lib.c::write_signature_db() returning.\n"));
#endif
#endif

    return kjb_fclose(fp);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int fp_read_signature_db_header
(
    FILE*               fp,                 /* Ptr to input file             */
    int*                num_signatures_ptr, /* Ptr to number of signatures   */
    Emd_cluster_method* cluster_method_ptr, /* Ptr to clustering method used */
    Emd_data_origin*    data_origin_ptr /* Ptr to origin of illum db     */
)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int    i;
    int    num_options;
    char** option_list;
    char** option_list_pos;
    char** value_list;
    char** value_list_pos;

    char   line[ 5000 ];
    char*  line_pos;
    long   top_file_pos;
    long   save_file_pos;

    int    num_signatures  = NOT_SET;
    int    cluster_method  = NOT_SET;
    int    data_origin     = NOT_SET;
    int    result          = NO_ERROR;

    int    still_looking_for_header = TRUE;

    if (fp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(top_file_pos = kjb_ftell(fp));

    while ( still_looking_for_header )
    {
        int read_result;

        read_result  = BUFF_FGET_LINE(fp, line);

        if (read_result == EOF)
        {
            add_error("No data in correlation signature database file \"%F\"", fp);
            result = ERROR;
            break;
        }

        if (read_result < 0)
        {
            add_error("Error reading data from signature database file \"%F\"", fp);
            result = ERROR;
            break;
        }

        save_file_pos = kjb_ftell(fp);

        if (save_file_pos == ERROR)
        {
            result = ERROR;
            break;
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

            if (*line_pos == kjb_header_char)
            {
                line_pos++;

                num_options = parse_options(line_pos, &option_list,
                                            &value_list);
                option_list_pos = option_list;
                value_list_pos  = value_list;

                if (num_options == ERROR)
                {
                    add_error("Unable to parse options in correlation matrix header.");
                    result = ERROR;
                    break;
                }

                for (i=0; i<num_options; i++)
                {
                    /* Number of signatures */
                    if (IC_STRCMP_EQ(*option_list_pos,"emd_nos"))
                    {
                        if (ss1pi(*value_list_pos, &num_signatures) == ERROR)
                        {
                            add_error("Unable to read \"emd_nos\" (num. signatures) option");
                            cat_error(" from signature database header.");
                            result = ERROR;
                            break;
                        }
                    }
                    /* Clustering method */
                    else if (IC_STRCMP_EQ(*option_list_pos,"emd_clm"))
                    {
                        if (ss1pi(*value_list_pos, &cluster_method) == ERROR)
                        {
                            add_error("Unable to read \"emd_clm\" (cluster method) option");
                            cat_error(" from signature database header.");
                            result = ERROR;
                            break;
                        }
                    }

                    /* Illuminant database origin */
                    else if (IC_STRCMP_EQ(*option_list_pos,"emd_ido"))
                    {
                        if (ss1pi(*value_list_pos, &data_origin) == ERROR)
                        {
                            add_error("Unable to read \"emd_ido\" (illum. data origin) option");
                            cat_error(" from signature database header.");
                            result = ERROR;
                            break;
                        }
                    }

                    value_list_pos++;
                    option_list_pos++;
                }

                free_options(num_options, option_list, value_list);

                if (result == NO_ERROR)
                {
                    still_looking_for_header = FALSE;
                }
                else
                    break;
            }
        }
        else
        {
            result =  NO_ERROR;
            break;
        }
    }

    if (result != ERROR)
    {
        if (KJB_IS_SET(num_signatures))
        {
            *num_signatures_ptr = num_signatures;
        }

        if (KJB_IS_SET(cluster_method))
        {
            *cluster_method_ptr = (Emd_cluster_method)cluster_method;
        }

        if (KJB_IS_SET(data_origin))
        {
            *data_origin_ptr = (Emd_data_origin)data_origin;
        }

        ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
    }
    else
    {
        ERE(kjb_fseek(fp, top_file_pos, SEEK_SET));
    }

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int fp_write_signature_db_header
(
    FILE*               fp,
    const Signature_db* source_sig_db_ptr
)
{
    char   comment_str[100];
    int    result;


    if (fp == NULL)
    {
        set_error("emd_lib.c::fp_write_signature_db_header(): Invalid file pointer.");
        return ERROR;
    }

    /* Write out the header. */
    ERE(kjb_fprintf(fp, "#!"));
    ERE(kjb_fprintf(fp, " emd_nos=%d",     source_sig_db_ptr->num_signatures));
    ERE(kjb_fprintf(fp, " emd_clm=%d",     source_sig_db_ptr->cluster_method));
    ERE(kjb_fprintf(fp, " emd_ido=%d\n\n", source_sig_db_ptr->data_origin));

    /* Write out human-readable comments. */
    ERE(kjb_fprintf(fp, "# (emd_nos) Number of signatures: %d\n",
                    source_sig_db_ptr->num_signatures));

    switch( source_sig_db_ptr->cluster_method ) {
      case EMD_NO_CLUSTERING:
          BUFF_CPY(comment_str, "Data is not clustered");
          result = NO_ERROR;
          break;
      case EMD_2D_HISTOGRAM:
          BUFF_CPY(comment_str, "Clusters from chromaticity histogram");
          result = NO_ERROR;
          break;
      case EMD_3D_HISTOGRAM:
          BUFF_CPY(comment_str, "Clusters from 3-D histogram");
          result = NO_ERROR;
          break;
      case EMD_K_MEANS:
          BUFF_CPY(comment_str, "Clusters from k-means");
          result = NO_ERROR;
          break;
      case EMD_CLARANS:
          BUFF_CPY(comment_str, "Clusters from CLARANS");
          result = NO_ERROR;
          break;
      default:
          SET_ARGUMENT_BUG();
          result = ERROR;
    }

    if ( result == NO_ERROR )
    {

        ERE(kjb_fprintf(fp, "# (emd_clm) %s\n", comment_str));

        if ( source_sig_db_ptr->data_origin == EMD_FROM_RGB )
        {
            ERE(kjb_fprintf(fp, "# (emd_ido) %s\n\n", "Data from RGB"));
        }

        else if ( source_sig_db_ptr->data_origin == EMD_FROM_SPECTRA )
        {
            ERE(kjb_fprintf(fp, "# (emd_ido) %s\n\n", "Data from spectra"));
        }

        else
        {
            SET_ARGUMENT_BUG();
            result = ERROR;
        }
    }

    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef DONT_LINT_SHARED */

