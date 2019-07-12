
/* $Id: mm_cluster_lib.c 21545 2017-07-23 21:57:31Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and  UA. Currently
|  the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include <time.h>

#include "mm/mm_cluster_lib.h"
#include "r/r_cluster_lib.h"
#include "graph/jv_lap.h"
#include "graph/hungarian.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
#define TEST_AGAINST_JV_LAP
*/

/*
#define DEBUG_NULLS
*/

#ifdef TEST_AGAINST_JV_LAP
#    ifndef TEST
#        define TEST
#    endif
#endif

/* -------------------------------------------------------------------------- */

#define MIN_LEVEL_WEIGHT (0.1)

/* -------------------------------------------------------------------------- */

#define VERTICAL_DOCUMENT_WEIGHTS_COUNT

/* -------------------------------------------------------------------------- */

typedef union Seed_union
{
    kjb_int32  i32;
    kjb_uint16 u16[ 2 ];
}
Seed_union;

/* -------------------------------------------------------------------------- */

static kjb_uint16 fs_horizontal_seed[ 3 ] = { 0, 0, 0 };
static kjb_uint16 fs_vertical_seed[ 3 ]   = { 0, 0, 0 };

static int get_hungarian_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           num_nulls,
    int           duplicate_words_for_matching
);

static int get_sampled_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           duplicate_words_for_matching
);

static int sample_log_matrix
(
    const Matrix* sample_mp,
    double*       prob_ptr,
    int*          row_ptr,
    int*          col_ptr
);

static int sample_log_vector(const Vector* sample_vp, double* prob_ptr);


/* -------------------------------------------------------------------------- */

int set_multi_modal_cluster_lib_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result         = NOT_FOUND;
    int  __attribute__((unused)) temp_boolean_value;



    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "hc-em-horizontal-seed")
       )
    {
        Seed_union first_value, second_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if ((value[ 0 ] == '?') || (value[ 0 ] == '\0'))
        {
#ifdef MSB_FIRST
            first_value.u16[ 0 ] = fs_horizontal_seed[ 0 ];
            first_value.u16[ 1 ] = fs_horizontal_seed[ 1 ];
            second_value.u16[ 0 ] = 0;
            second_value.u16[ 1 ] = fs_horizontal_seed[ 2 ];
#else
            first_value.u16[ 1 ] = fs_horizontal_seed[ 0 ];
            first_value.u16[ 0 ] = fs_horizontal_seed[ 1 ];
            second_value.u16[ 1 ] = 0;
            second_value.u16[ 0 ] = fs_horizontal_seed[ 2 ];
#endif 

            if (value[ 0 ] == '?')
            {
                ERE(pso("hc-em-horizontal-seed = %ld:%ld\n",
                        first_value.i32, 
                        (kjb_int32)((kjb_int16)fs_horizontal_seed[ 2 ])));
            }
            else
            {
                ERE(pso("Current horizontal seed is %ld:%ld\n",
                        first_value.i32, 
                        (kjb_int32)((kjb_int16)fs_horizontal_seed[ 2 ])));
            }
        }
        else
        {
            char first_value_buff[ 100 ];
            char second_value_buff[ 100 ];
            char value_buff[ 100 ];
            char* value_pos;

            BUFF_CPY(value_buff, value);
            value_pos = value_buff;

            BUFF_GEN_GET_TOKEN(&value_pos, first_value_buff, ":");

            if (STRCMP_EQ(first_value_buff, "*"))
            {
                first_value.i32 = time((time_t *)NULL);
            }
            else
            {
                ERE(ss1i32(first_value_buff, &(first_value.i32)));
            }
            BUFF_GEN_GET_TOKEN(&value_pos, second_value_buff, ":");

            if (STRCMP_EQ(second_value_buff, "*"))
            {
                second_value.i32 = time((time_t *)NULL);
            }
            else if (second_value_buff[ 0 ] != '\0')
            {
                ERE(ss1i32(second_value_buff, &(second_value.i32)));

                if (ABS_OF(second_value.i32) > INT16_MAX)
                {
                    set_error("Magnitude of second seed commponant must be at most %d.",
                              INT16_MAX);
                    return ERROR;
                }
            }
            else
            {
                second_value.i32 = 0;
            }


#ifdef MSB_FIRST
            fs_horizontal_seed[ 0 ] = first_value.u16[ 0 ];
            fs_horizontal_seed[ 1 ] = first_value.u16[ 1 ];
            fs_horizontal_seed[ 2 ] = second_value.u16[ 1 ];
#else
            fs_horizontal_seed[ 0 ] = first_value.u16[ 1 ];
            fs_horizontal_seed[ 1 ] = first_value.u16[ 0 ];
            fs_horizontal_seed[ 2 ] = second_value.u16[ 0 ];
#endif
        }
        result = NO_ERROR;
    }


    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "hc-em-vertical-seed")
       )
    {
        Seed_union first_value, second_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if ((value[ 0 ] == '?') || (value[ 0 ] == '\0'))
        {
#ifdef MSB_FIRST
            first_value.u16[ 0 ] = fs_vertical_seed[ 0 ];
            first_value.u16[ 1 ] = fs_vertical_seed[ 1 ];
            second_value.u16[ 0 ] = 0;
            second_value.u16[ 1 ] = fs_vertical_seed[ 2 ];
#else
            first_value.u16[ 1 ] = fs_vertical_seed[ 0 ];
            first_value.u16[ 0 ] = fs_vertical_seed[ 1 ];
            second_value.u16[ 1 ] = 0;
            second_value.u16[ 0 ] = fs_vertical_seed[ 2 ];
#endif 

            if (value[ 0 ] == '?')
            {
                ERE(pso("hc-em-vertical-seed = %ld:%ld\n",
                        first_value.i32, 
                        (kjb_int32)((kjb_int16)fs_vertical_seed[ 2 ])));
            }
            else
            {
                ERE(pso("Current vertical seed is %ld:%ld\n",
                        first_value.i32, 
                        (kjb_int32)((kjb_int16)fs_vertical_seed[ 2 ])));
            }
        }
        else
        {
            char first_value_buff[ 100 ];
            char second_value_buff[ 100 ];
            char value_buff[ 100 ];
            char* value_pos;

            BUFF_CPY(value_buff, value);
            value_pos = value_buff;

            BUFF_GEN_GET_TOKEN(&value_pos, first_value_buff, ":");

            if (STRCMP_EQ(first_value_buff, "*"))
            {
                first_value.i32 = time((time_t *)NULL);
            }
            else
            {
                ERE(ss1i32(first_value_buff, &(first_value.i32)));
            }
            BUFF_GEN_GET_TOKEN(&value_pos, second_value_buff, ":");

            if (STRCMP_EQ(second_value_buff, "*"))
            {
                second_value.i32 = time((time_t *)NULL);
            }
            else if (second_value_buff[ 0 ] != '\0')
            {
                ERE(ss1i32(second_value_buff, &(second_value.i32)));

                if (ABS_OF(second_value.i32) > INT16_MAX)
                {
                    set_error("Magnitude of second seed commponant must be at most %d.",
                              INT16_MAX);
                    return ERROR;
                }
            }
            else
            {
                second_value.i32 = 0;
            }


#ifdef MSB_FIRST
            fs_vertical_seed[ 0 ] = first_value.u16[ 0 ];
            fs_vertical_seed[ 1 ] = first_value.u16[ 1 ];
            fs_vertical_seed[ 2 ] = second_value.u16[ 1 ];
#else
            fs_vertical_seed[ 0 ] = first_value.u16[ 1 ];
            fs_vertical_seed[ 1 ] = first_value.u16[ 0 ];
            fs_vertical_seed[ 2 ] = second_value.u16[ 0 ];
#endif
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_initial_vertical_indicators
(
    Matrix** V_l_c_mpp,
    int      first_level,
    int      last_level_num,
    int      num_levels,
    int      num_clusters
)
{
    int        result;
    kjb_uint16 save_seed[ 3 ];
    int        level;
    int        cluster;
    Matrix*    V_l_c_mp;


    /* Untested since addition of last level parameter in topology. */
    UNTESTED_CODE();

    ERE(get_rand_seed(save_seed));

    kjb_seed_rand_with_3_short(fs_vertical_seed);

    result = get_random_matrix(V_l_c_mpp, num_levels, num_clusters);
    V_l_c_mp = *V_l_c_mpp;

    if (result != ERROR)
    {
        /*
        // FIX
        //
        // The 0.1 should be an option, shared with the horizontal inits (the
        // inti logic should be moved out of m_cluster_lib.c.
        */
        result = ow_add_scalar_to_matrix(V_l_c_mp, 0.1);
    }

    if (result != ERROR)
    {
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double sum = 0.0;

            for (level = first_level; level < last_level_num; level++)
            {
                sum += V_l_c_mp->elements[ level ][ cluster ];
            }

            for (level = first_level; level < last_level_num; level++)
            {
                V_l_c_mp->elements[ level ][ cluster ] /= sum;
            }
        }
    }

    ERE(get_rand_seed(fs_vertical_seed));

    kjb_seed_rand_with_3_short(save_seed);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_initial_horizontal_indicators
(
    Matrix** P_c_p_mpp,
    int      num_points,
    int      num_clusters
)
{
    int result;
    kjb_uint16 save_seed[ 3 ];

    ERE(get_rand_seed(save_seed));

    kjb_seed_rand_with_3_short(fs_horizontal_seed);

    result = get_cluster_random_matrix_1(P_c_p_mpp,
                                         num_points, num_clusters);

    ERE(get_rand_seed(fs_horizontal_seed));

    kjb_seed_rand_with_3_short(save_seed);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_multi_modal_model
(
    Multi_modal_model**      target_model_ptr_ptr,
    const Multi_modal_model* source_model_ptr
)
{
    Multi_modal_model*  target_model_ptr;


    if (source_model_ptr == NULL)
    {
        free_multi_modal_model(*target_model_ptr_ptr);
        *target_model_ptr_ptr = NULL;
        return NO_ERROR;
    }

    if (*target_model_ptr_ptr == NULL)
    {
        NRE(target_model_ptr = allocate_multi_modal_model());
        *target_model_ptr_ptr = target_model_ptr;
    }
    else
    {
        target_model_ptr = *target_model_ptr_ptr;
    }

    ERE(copy_int_vector(&(target_model_ptr->con_feature_enable_vp), 
                        source_model_ptr->con_feature_enable_vp));

    ERE(copy_int_vector(&(target_model_ptr->con_feature_group_vp), 
                        source_model_ptr->con_feature_group_vp));

    ERE(copy_vector(&(target_model_ptr->con_feature_weight_vp), 
                    source_model_ptr->con_feature_weight_vp));

    target_model_ptr->weight_con_items = source_model_ptr->weight_con_items;

    target_model_ptr->discrete_tie_count    = source_model_ptr->discrete_tie_count;

    target_model_ptr->phase_one_num_iterations    = source_model_ptr->phase_one_num_iterations;
    target_model_ptr->phase_two_num_iterations    = source_model_ptr->phase_two_num_iterations;
    target_model_ptr->phase_one_uniform_vertical_dist    = source_model_ptr->phase_one_uniform_vertical_dist;
    target_model_ptr->phase_two_uniform_vertical_dist    = source_model_ptr->phase_two_uniform_vertical_dist;
    target_model_ptr->phase_one_cluster_dependent_vertical_dist    = source_model_ptr->phase_one_cluster_dependent_vertical_dist;
    target_model_ptr->phase_two_cluster_dependent_vertical_dist    = source_model_ptr->phase_two_cluster_dependent_vertical_dist;

    target_model_ptr->num_clusters = source_model_ptr->num_clusters;
    target_model_ptr->num_categories = source_model_ptr->num_categories;
    target_model_ptr->num_con_features = source_model_ptr->num_con_features;
    target_model_ptr->var_offset    = source_model_ptr->var_offset;
    target_model_ptr->raw_log_likelihood = source_model_ptr->raw_log_likelihood;
    target_model_ptr->mdl_log_likelihood = source_model_ptr->mdl_log_likelihood;
    target_model_ptr->aic = source_model_ptr->aic;
    target_model_ptr->num_parameters = source_model_ptr->num_parameters;

    target_model_ptr->extra_con_prob = source_model_ptr->extra_con_prob;
    target_model_ptr->extra_dis_prob = source_model_ptr->extra_dis_prob;

    target_model_ptr->con_log_norm = source_model_ptr->con_log_norm;
    target_model_ptr->con_score_cutoff = source_model_ptr->con_score_cutoff;
    target_model_ptr->min_con_log_prob = source_model_ptr->min_con_log_prob;
    target_model_ptr->max_con_norm_var = source_model_ptr->max_con_norm_var;

    target_model_ptr->dis_item_prob_threshold  = source_model_ptr->dis_item_prob_threshold;
    target_model_ptr->norm_items               = source_model_ptr->norm_items;
    target_model_ptr->norm_continuous_features = source_model_ptr->norm_continuous_features;
    target_model_ptr->dis_prob_boost_factor    = source_model_ptr->dis_prob_boost_factor;
    target_model_ptr->con_prob_boost_factor    = source_model_ptr->con_prob_boost_factor;

    target_model_ptr->ignore_dis_prob_fraction    = source_model_ptr->ignore_dis_prob_fraction;
    target_model_ptr->ignore_con_prob_fraction    = source_model_ptr->ignore_con_prob_fraction;

    target_model_ptr->phase_one_sample_matches    = source_model_ptr->phase_one_sample_matches;
    target_model_ptr->phase_one_last_sample    = source_model_ptr->phase_one_last_sample;
    target_model_ptr->phase_one_model_correspondence    = source_model_ptr->phase_one_model_correspondence;
    target_model_ptr->phase_one_share_matches    = source_model_ptr->phase_one_share_matches;
    target_model_ptr->phase_two_sample_matches    = source_model_ptr->phase_two_sample_matches;
    target_model_ptr->phase_two_last_sample    = source_model_ptr->phase_two_last_sample;
    target_model_ptr->phase_two_model_correspondence    = source_model_ptr->phase_two_model_correspondence;
    target_model_ptr->phase_two_share_matches    = source_model_ptr->phase_two_share_matches;

    ERE(copy_topology(&(target_model_ptr->topology_ptr),
                        source_model_ptr->topology_ptr));

    ERE(copy_vector(&(target_model_ptr->a_vp), source_model_ptr->a_vp));
    ERE(copy_matrix(&(target_model_ptr->V_mp), source_model_ptr->V_mp));
    ERE(copy_vector(&(target_model_ptr->V_vp), source_model_ptr->V_vp));
    ERE(copy_matrix(&(target_model_ptr->con_V_mp), source_model_ptr->con_V_mp));
    ERE(copy_matrix(&(target_model_ptr->dis_V_mp), source_model_ptr->dis_V_mp));
    ERE(copy_matrix(&(target_model_ptr->P_c_p_mp), source_model_ptr->P_c_p_mp));
    ERE(copy_vector(&(target_model_ptr->fit_vp), source_model_ptr->fit_vp));

    ERE(copy_matrix_vector(&(target_model_ptr->P_l_p_c_mvp),
                           source_model_ptr->P_l_p_c_mvp));

    ERE(copy_matrix(&(target_model_ptr->P_l_p_mp),
                    source_model_ptr->P_l_p_mp));

    ERE(copy_matrix_vector(&(target_model_ptr->P_i_n_mvp),
                           source_model_ptr->P_i_n_mvp));


    ERE(copy_matrix_vector(&(target_model_ptr->mean_mvp),
                           source_model_ptr->mean_mvp));

    ERE(copy_matrix_vector(&(target_model_ptr->var_mvp),
                           source_model_ptr->var_mvp));

    ERE(copy_vector_vector(&(target_model_ptr->con_log_sqrt_det_vvp),
                           source_model_ptr->con_log_sqrt_det_vvp));


    ERE(copy_matrix(&(target_model_ptr->mean_mp),
                    source_model_ptr->mean_mp));

    ERE(copy_matrix(&(target_model_ptr->var_mp),
                           source_model_ptr->var_mp));

    ERE(copy_vector(&(target_model_ptr->con_log_sqrt_det_vp),
                    source_model_ptr->con_log_sqrt_det_vp));


    ERE(copy_cluster_data(&(target_model_ptr->data_ptr),
                          source_model_ptr->data_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int compute_mdl_log_likelihood
(
    int                 num_points,
    Multi_modal_model* model_ptr
)
{


    return compute_mdl_log_likelihood_2(num_points, model_ptr,
                                        model_ptr->raw_log_likelihood,
                                        &(model_ptr->mdl_log_likelihood),
                                        &(model_ptr->aic));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int compute_mdl_log_likelihood_2
(
    int                 num_points,
    Multi_modal_model* model_ptr,
    double              raw_log_likelihood,
    double*             mdl_ll_ptr,
    double*             aic_ll_ptr
)
{
    const int first_level        = model_ptr->topology_ptr->first_level;
    const int last_level_num         = model_ptr->topology_ptr->last_level_num;
    const int num_clusters       = model_ptr->a_vp->length;
    int       num_parameters     = 0;
    int       level;
    double    encoding_cost;
    double      dis_item_prob_threshold       = model_ptr->dis_item_prob_threshold;


    /* Untested since addition of last level parameter in topology. */
    UNTESTED_CODE();

    TEST_PSO(("MDL still needs hist and vec.\n"));

    for (level = first_level; level < last_level_num; level++)
    {
        if (model_ptr->var_mvp != NULL)
        {
            Matrix* var_mp = model_ptr->var_mvp->elements[ level ];
            int ii, jj;

            /* Now add two for each good con cluster */
            for (ii = 0; ii < var_mp->num_rows; ii++)
            {
                for (jj = 0; jj < var_mp->num_cols; jj++)
                {
                    if (var_mp->elements[ ii ][ jj ] > -0.5)
                    {
                        num_parameters += 2;
                    }
                }
            }
        }

        if (model_ptr->P_i_n_mvp != NULL)
        {
            Matrix* P_i_n_mp = model_ptr->P_i_n_mvp->elements[ level ];
            int ii, jj;

            /* Now add one for each non-negligible item prob. */

            for (ii = 0; ii < P_i_n_mp->num_rows; ii++)
            {
                for (jj = 0; jj < P_i_n_mp->num_cols; jj++)
                {
                    if (    (P_i_n_mp->elements[ ii ][ jj ] > 0.0)
                         && (P_i_n_mp->elements[ ii ][ jj ] > dis_item_prob_threshold / 2.0)
                       )
                    {
                        num_parameters++;
                    }
                }
            }
        }
    }

    /* Cluster mixing weights */
    /* I use one less, because the elements of a_vp sum to one. */
    num_parameters += model_ptr->a_vp->length - 1;

#ifdef VERTICAL_DOCUMENT_WEIGHTS_COUNT
    if (model_ptr->P_l_p_c_mvp != NULL)
    {
        /* Vertical cluster mixing weights */
        /* I use one less, because each vertical slice sums to one. */
        num_parameters += (last_level_num - first_level - 1) * num_clusters * num_points;
    }
    else if (model_ptr->P_l_p_mp != NULL)
    {
        /* Vertical cluster mixing weights */
        /* I use one less, because each vertical slice sums to one. */
        num_parameters += (last_level_num - first_level - 1) * num_points;
    }
    else
    {
        /* Vertical cluster mixing weights */
        /* I use one less, because each vertical slice sums to one. */
        num_parameters += (last_level_num - first_level - 1) * num_clusters;
    }
#else
    /* Vertical cluster mixing weights */
    /* I use one less, because each vertical slice sums to one. */
    num_parameters += (last_level_num - first_level - 1) * num_clusters;
#endif

    model_ptr->num_parameters = num_parameters;

    if (raw_log_likelihood < DBL_HALF_MOST_NEGATIVE / 2.0)
    {
        if (mdl_ll_ptr != NULL)
        {
            *mdl_ll_ptr = DBL_HALF_MOST_NEGATIVE;
        }

        if (aic_ll_ptr != NULL)
        {
            *aic_ll_ptr = DBL_HALF_MOST_NEGATIVE;
        }

        return NO_ERROR;
    }

    if (mdl_ll_ptr != NULL)
    {
        /* MDL log-likelihood penalty */
        encoding_cost = 0.5 * num_parameters * log((double)num_points);
        *mdl_ll_ptr = raw_log_likelihood - encoding_cost;
    }

    if (aic_ll_ptr != NULL)
    {
        /* AIC log-likelihood penalty */
        encoding_cost = 2.0 * num_parameters;
        encoding_cost *= (1.0 + (num_parameters + 1.0)/(num_points - num_parameters - 1));

        *aic_ll_ptr = -2.0 * raw_log_likelihood + encoding_cost;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_target_topology_vector
(
    Topology_vector** topology_vpp,
    int               num_topologies
)
{
    Topology_vector* topology_vp;
    int i;


    free_topology_vector(*topology_vpp);

    NRE(*topology_vpp = TYPE_MALLOC(Topology_vector));
    topology_vp = *topology_vpp;

    NRE(topology_vp->topologies = N_TYPE_MALLOC(Topology*, num_topologies));

    topology_vp->num_topologies = num_topologies;

    for (i = 0; i < num_topologies; i++)
    {
        topology_vp->topologies[ i ] = allocate_topology();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_topology_vector(Topology_vector* topology_vp)
{

    if (topology_vp != NULL)
    {
        int i;

        for (i = 0; i < topology_vp->num_topologies; i++)
        {
            free_topology(topology_vp->topologies[ i ]);
        }

        kjb_free(topology_vp->topologies);
        kjb_free(topology_vp);

    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_topology
(
    Topology** topology_ptr_ptr,
    int        num_levels,
    int        first_level,
    int        last_level_num,
    int        fan_out,
    int        first_fan_out,
    int        last_fan_out
)
{
    Topology*   topology_ptr = *topology_ptr_ptr;


    ASSERT(num_levels > 1);
    ASSERT(fan_out > 0);

    ERE(get_initialized_int_vector(&(topology_ptr->fan_out_vp),
                                   num_levels - 1, fan_out));

    if (first_fan_out > 0)
    {
       topology_ptr->fan_out_vp->elements[ 0 ] = first_fan_out;
    }

    if (last_fan_out > 0)
    {
       topology_ptr->fan_out_vp->elements[  num_levels - 2 ] = last_fan_out;
    }

    ERE(get_topology_2(topology_ptr_ptr, topology_ptr->fan_out_vp,
                       first_level, last_level_num));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_topology_2
(
    Topology** topology_ptr_ptr,
    const Int_vector* fan_out_vp,
    int        first_level,
    int        last_level_num
)
{
    Topology*   topology_ptr = *topology_ptr_ptr;
    int         num_levels = fan_out_vp->length + 1;
    int         level;
    int         level_count  = 1;
    int         num_clusters;
    int         cluster;
    Int_matrix* node_mp;
    int fan_out;


    if (topology_ptr == NULL)
    {
        NRE(*topology_ptr_ptr = allocate_topology());
        topology_ptr = *topology_ptr_ptr;
    }

#ifdef OBSOLETE /* Superseeded by separate word model. */
    topology_ptr->hack_nando_gaussian = FALSE;
#endif
    topology_ptr->num_levels_per_category = 0;

    topology_ptr->first_level = first_level;
    topology_ptr->last_level_num = last_level_num;
    topology_ptr->num_levels = num_levels;

    ASSERT(num_levels > 1);

    ERE(get_target_int_vector(&(topology_ptr->level_counts_vp),
                              num_levels));

    for (level = 0; level < num_levels; level++)
    {
        if ((level < first_level) || (level >= last_level_num))
        {
            topology_ptr->level_counts_vp->elements[ level ] = 0;
        }
        else
        {
            topology_ptr->level_counts_vp->elements[ level ] = level_count;
        }

        if (level < (num_levels - 1))
        {
            fan_out = fan_out_vp->elements[ level ];

            ASSERT(fan_out > 0);

            level_count *= fan_out;
        }
    }

    num_clusters = level_count;

    ASSERT(num_clusters > 0);

    topology_ptr->num_clusters = num_clusters;

    ERE(get_initialized_int_matrix(&(topology_ptr->node_mp),
                                   num_clusters, num_levels,
                                   NOT_SET))

    node_mp = topology_ptr->node_mp;

    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        int node = cluster;

        for (level = num_levels - 1; level >= first_level; level--)
        {
            if (level < last_level_num)
            {
                node_mp->elements[ cluster ][ level ] = node;
            }

            if (level > 0)
            {
                ASSERT(fan_out_vp->elements[ level - 1 ] > 0);

                node /= fan_out_vp->elements[ level - 1 ];
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Topology* allocate_topology(void)
{
    Topology* topology_ptr;

    NRN(topology_ptr = TYPE_MALLOC(Topology));
    topology_ptr->fan_out_vp = NULL;
    topology_ptr->level_counts_vp = NULL;
    topology_ptr->node_mp = NULL;

#ifdef OBSOLETE /* Superseeded by separate word model. */
    topology_ptr->hack_nando_gaussian = FALSE;
#endif
    topology_ptr->num_levels_per_category = 0;

    topology_ptr->first_level = 0;
    topology_ptr->last_level_num = NOT_SET;
    topology_ptr->num_levels = NOT_SET;
    topology_ptr->num_clusters = NOT_SET;

    return topology_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_topology(Topology* topology_ptr)
{
    if (topology_ptr != NULL)
    {
        free_int_vector(topology_ptr->fan_out_vp);
        free_int_vector(topology_ptr->level_counts_vp);
        free_int_matrix(topology_ptr->node_mp);
        kjb_free(topology_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_topology
(
    Topology**      target_topology_ptr_ptr,
    const Topology* source_topology_ptr
)
{
    Topology* target_topology_ptr = *target_topology_ptr_ptr;

    if (source_topology_ptr == NULL)
    {
        free_topology(*target_topology_ptr_ptr);
        *target_topology_ptr_ptr = NULL;
        return NO_ERROR;
    }

    if (target_topology_ptr == NULL)
    {
        NRE(target_topology_ptr = allocate_topology());
        *target_topology_ptr_ptr = target_topology_ptr;
    }

    target_topology_ptr->num_clusters = source_topology_ptr->num_clusters;
    target_topology_ptr->num_levels = source_topology_ptr->num_levels;
    target_topology_ptr->first_level = source_topology_ptr->first_level;
    target_topology_ptr->last_level_num = source_topology_ptr->last_level_num;
#ifdef OBSOLETE /* Superseeded by separate word model. */
    target_topology_ptr->hack_nando_gaussian = source_topology_ptr->hack_nando_gaussian;
#endif
    target_topology_ptr->num_levels_per_category = source_topology_ptr->num_levels_per_category;

    ERE(copy_int_vector(&(target_topology_ptr->fan_out_vp),
                        source_topology_ptr->fan_out_vp));

    ERE(copy_int_vector(&(target_topology_ptr->level_counts_vp),
                        source_topology_ptr->level_counts_vp));

    ERE(copy_int_matrix(&(target_topology_ptr->node_mp),
                        source_topology_ptr->node_mp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Cluster_data* allocate_cluster_data(void)
{
    Cluster_data* data_ptr;

    NRN(data_ptr = TYPE_MALLOC(Cluster_data));

    data_ptr->num_categories = 0;
    data_ptr->num_con_features = 0;
    data_ptr->num_vec_features = 0;
    data_ptr->num_his_features = 0;
    data_ptr->num_tokens = 0;

    data_ptr->num_points = 0;
    data_ptr->max_num_con_items = 0;
    data_ptr->max_num_dis_items = 0;
    data_ptr->max_num_dis_items_with_multiplicity = 0.0;
    data_ptr->max_num_vec_items = 0;
    data_ptr->max_num_his_items = 0;
    data_ptr->weight_vp = NULL;
    data_ptr->group_vp = NULL;
    data_ptr->con_item_weight_vvp = NULL;
    data_ptr->dis_predict_flag_vp = NULL;  /* Not used, yet? */
    data_ptr->dis_item_mp = NULL;
    data_ptr->dis_item_multiplicity_mp = NULL;
    data_ptr->dis_item_aux_mp = NULL;
    data_ptr->dis_item_aux_index_vvp = NULL;
    data_ptr->dis_item_aux_value_vvp = NULL;
    data_ptr->dis_item_observed_counts_mp = NULL;
    data_ptr->dis_item_derived_counts_mp = NULL;
#ifdef NEVER_USED
    data_ptr->dis_category_level_prior_mp = NULL;
#endif
    data_ptr->con_predict_flag_vp = NULL;
    data_ptr->con_item_mvp = NULL;
    data_ptr->con_item_token_mp = NULL;
    data_ptr->con_item_info_mvp = NULL;
    data_ptr->con_item_index_vvp = NULL;
    data_ptr->con_item_merge_prior_mvp = NULL;
    data_ptr->vec_item_vvvvp = NULL;
    data_ptr->vec_num_dim_vp = NULL;
    data_ptr->his_item_vvvvp = NULL;
    data_ptr->his_num_dim_vp = NULL;
    data_ptr->label_vp = NULL;
    data_ptr->id_vp = NULL;
    data_ptr->blob_word_posterior_mvp = NULL;
    data_ptr->blob_word_prior_mvp = NULL;

    return data_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_cluster_data
(
    Cluster_data**      target_data_ptr_ptr,
    const Cluster_data* source_data_ptr
)
{
    Cluster_data* target_data_ptr = *target_data_ptr_ptr;

    if (source_data_ptr == NULL)
    {
        free_cluster_data(*target_data_ptr_ptr);
        *target_data_ptr_ptr = NULL;
        return NO_ERROR;
    }

    if (target_data_ptr == NULL)
    {
        NRE(target_data_ptr = allocate_cluster_data());
        *target_data_ptr_ptr = target_data_ptr;
    }

    target_data_ptr->num_categories = source_data_ptr->num_categories;
    target_data_ptr->num_con_features = source_data_ptr->num_con_features;
    target_data_ptr->num_vec_features = source_data_ptr->num_vec_features;
    target_data_ptr->num_his_features = source_data_ptr->num_his_features;
    target_data_ptr->num_tokens = source_data_ptr->num_tokens;

    target_data_ptr->num_points = source_data_ptr->num_points;
    target_data_ptr->max_num_dis_items = source_data_ptr->max_num_dis_items;
    target_data_ptr->max_num_dis_items_with_multiplicity = source_data_ptr->max_num_dis_items_with_multiplicity;
    target_data_ptr->max_num_con_items = source_data_ptr->max_num_con_items;

    ERE(copy_int_vector(&(target_data_ptr->group_vp),
                        source_data_ptr->group_vp));

    ERE(copy_vector(&(target_data_ptr->weight_vp),
                    source_data_ptr->weight_vp));

    ERE(copy_vector_vector(&(target_data_ptr->con_item_weight_vvp),
                           source_data_ptr->con_item_weight_vvp));

    ERE(copy_int_vector(&(target_data_ptr->dis_predict_flag_vp),
                        source_data_ptr->dis_predict_flag_vp));

    ERE(copy_int_matrix(&(target_data_ptr->dis_item_mp),
                        source_data_ptr->dis_item_mp));

    ERE(copy_matrix(&(target_data_ptr->dis_item_multiplicity_mp),
                    source_data_ptr->dis_item_multiplicity_mp));

    ERE(copy_int_vector_vector(&(target_data_ptr->dis_item_aux_index_vvp),
                               source_data_ptr->dis_item_aux_index_vvp));

    ERE(copy_vector_vector(&(target_data_ptr->dis_item_aux_value_vvp),
                           source_data_ptr->dis_item_aux_value_vvp));

    ERE(copy_matrix(&(target_data_ptr->dis_item_aux_mp),
                    source_data_ptr->dis_item_aux_mp));

    ERE(copy_int_matrix(&(target_data_ptr->dis_item_observed_counts_mp),
                        source_data_ptr->dis_item_observed_counts_mp));

    ERE(copy_int_matrix(&(target_data_ptr->dis_item_derived_counts_mp),
                        source_data_ptr->dis_item_derived_counts_mp));

#ifdef NEVER_USED
    ERE(copy_matrix(&(target_data_ptr->dis_category_level_prior_mp),
                    source_data_ptr->dis_category_level_prior_mp));
#endif

    ERE(copy_int_vector(&(target_data_ptr->con_predict_flag_vp),
                        source_data_ptr->con_predict_flag_vp));

    ERE(copy_matrix_vector(&(target_data_ptr->con_item_mvp),
                           source_data_ptr->con_item_mvp));

    ERE(copy_int_matrix(&(target_data_ptr->con_item_token_mp),
                        source_data_ptr->con_item_token_mp));

    ERE(copy_matrix_vector(&(target_data_ptr->con_item_info_mvp),
                           source_data_ptr->con_item_info_mvp));

    ERE(copy_int_vector_vector(&(target_data_ptr->con_item_index_vvp),
                               source_data_ptr->con_item_index_vvp));

    ERE(copy_matrix_vector(&(target_data_ptr->con_item_merge_prior_mvp),
                           source_data_ptr->con_item_merge_prior_mvp));

    ERE(copy_v4(&(target_data_ptr->vec_item_vvvvp),
                source_data_ptr->vec_item_vvvvp));

    ERE(copy_int_vector(&(target_data_ptr->vec_num_dim_vp),
                        source_data_ptr->vec_num_dim_vp));

    ERE(copy_v4(&(target_data_ptr->his_item_vvvvp),
                source_data_ptr->his_item_vvvvp));

    ERE(copy_int_vector(&(target_data_ptr->his_num_dim_vp),
                        source_data_ptr->his_num_dim_vp));

    ERE(copy_int_vector(&(target_data_ptr->id_vp), source_data_ptr->id_vp));

    ERE(copy_int_vector(&(target_data_ptr->label_vp),
                        source_data_ptr->label_vp));

    ERE(copy_matrix_vector(&(target_data_ptr->blob_word_posterior_mvp),
                           source_data_ptr->blob_word_posterior_mvp));

    ERE(copy_matrix_vector(&(target_data_ptr->blob_word_prior_mvp),
                           source_data_ptr->blob_word_prior_mvp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int split_cluster_data
(
    Cluster_data**      train_data_ptr_ptr,
    Cluster_data**      held_out_data_ptr_ptr,
    const Cluster_data* source_data_ptr,
    const Int_vector*   training_image_nums_vp,
    const Int_vector*   held_out_image_nums_vp
)
{
    int           num_images        = source_data_ptr->num_points;
    Cluster_data* train_data_ptr    = *train_data_ptr_ptr;
    Cluster_data* held_out_data_ptr = * held_out_data_ptr_ptr;

    UNTESTED_CODE();

    if (source_data_ptr == NULL)
    {
        free_cluster_data(*train_data_ptr_ptr);
        *train_data_ptr_ptr = NULL;

        free_cluster_data(*held_out_data_ptr_ptr);
        *held_out_data_ptr_ptr = NULL;
        return NO_ERROR;
    }

    if (train_data_ptr == NULL)
    {
        NRE(train_data_ptr = allocate_cluster_data());
        *train_data_ptr_ptr = train_data_ptr;
    }
    if (held_out_data_ptr == NULL)
    {
        NRE(held_out_data_ptr = allocate_cluster_data());
        *held_out_data_ptr_ptr = held_out_data_ptr;
    }


    train_data_ptr->num_points = training_image_nums_vp->length;
    held_out_data_ptr->num_points = held_out_image_nums_vp->length;

    train_data_ptr->num_categories = source_data_ptr->num_categories;
    train_data_ptr->num_tokens = source_data_ptr->num_tokens;
    train_data_ptr->num_con_features = source_data_ptr->num_con_features;
    train_data_ptr->num_vec_features = source_data_ptr->num_vec_features;
    train_data_ptr->num_his_features = source_data_ptr->num_his_features;

    train_data_ptr->max_num_dis_items = source_data_ptr->max_num_dis_items;
    train_data_ptr->max_num_dis_items_with_multiplicity = source_data_ptr->max_num_dis_items_with_multiplicity;
    train_data_ptr->max_num_con_items = source_data_ptr->max_num_con_items;

    held_out_data_ptr->num_categories = source_data_ptr->num_categories;
    held_out_data_ptr->num_tokens = source_data_ptr->num_tokens;

    held_out_data_ptr->num_con_features = source_data_ptr->num_con_features;
    held_out_data_ptr->num_vec_features = source_data_ptr->num_vec_features;
    held_out_data_ptr->num_his_features = source_data_ptr->num_his_features;

    held_out_data_ptr->max_num_dis_items = source_data_ptr->max_num_dis_items;
    held_out_data_ptr->max_num_dis_items_with_multiplicity = source_data_ptr->max_num_dis_items_with_multiplicity;
    held_out_data_ptr->max_num_con_items = source_data_ptr->max_num_con_items;

    /*
    // Split cluster data into training and held out cluster data
    // If any vector/vv etc is not as long as the number of training images
    // simply copy the source vector/vv etc into training and heldout vectors/vv etc.
     */

    if((source_data_ptr->weight_vp != NULL) && (source_data_ptr->weight_vp->length == num_images))
    {

        ERE(split_vector(&(train_data_ptr->weight_vp),&(held_out_data_ptr->weight_vp),
                         source_data_ptr->weight_vp,training_image_nums_vp));
    }
    else
    {
        EPETE(copy_vector(&(train_data_ptr->weight_vp),source_data_ptr->weight_vp));
        EPETE(copy_vector(&(held_out_data_ptr->weight_vp),source_data_ptr->weight_vp));

    }

    if((source_data_ptr->con_item_weight_vvp != NULL) &&
       (source_data_ptr->con_item_weight_vvp->length == num_images))
    {
        ERE(split_vector_vector(&(train_data_ptr->con_item_weight_vvp),
                                &(held_out_data_ptr->con_item_weight_vvp),
                                source_data_ptr->con_item_weight_vvp,
                                training_image_nums_vp));
    }
    else
    {
        EPETE(copy_vector_vector(&(train_data_ptr->con_item_weight_vvp),
                                 source_data_ptr->con_item_weight_vvp));
        EPETE(copy_vector_vector(&(held_out_data_ptr->con_item_weight_vvp),
                                 source_data_ptr->con_item_weight_vvp));
    }

    if((source_data_ptr->dis_predict_flag_vp != NULL) &&
       (source_data_ptr->dis_predict_flag_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->dis_predict_flag_vp),
                             &(held_out_data_ptr->dis_predict_flag_vp),
                             source_data_ptr->dis_predict_flag_vp,
                             training_image_nums_vp));
    }
    else
    {

        EPETE(copy_int_vector(&(train_data_ptr->dis_predict_flag_vp),
                              source_data_ptr->dis_predict_flag_vp));
        EPETE(copy_int_vector(&(held_out_data_ptr->dis_predict_flag_vp),
                              source_data_ptr->dis_predict_flag_vp));

    }

    if(( source_data_ptr->dis_item_mp != NULL) && (source_data_ptr->dis_item_mp->num_rows == num_images))
    {
        ERE(split_int_matrix_by_rows(&(train_data_ptr->dis_item_mp),&(held_out_data_ptr->dis_item_mp),
                                     source_data_ptr->dis_item_mp,training_image_nums_vp));
    }
    else
    {
        EPETE(copy_int_matrix(&(train_data_ptr->dis_item_mp), source_data_ptr->dis_item_mp));
        EPETE(copy_int_matrix(&(held_out_data_ptr->dis_item_mp), source_data_ptr->dis_item_mp));

    }

    if((source_data_ptr->dis_item_multiplicity_mp != NULL) &&
       (source_data_ptr->dis_item_multiplicity_mp->num_rows == num_images))
    {
        ERE(split_matrix_by_rows(&(train_data_ptr->dis_item_multiplicity_mp),
                                 &(held_out_data_ptr->dis_item_multiplicity_mp),
                                 source_data_ptr->dis_item_multiplicity_mp,
                                 training_image_nums_vp));
    }
    else
    {
        EPETE(copy_matrix(&(train_data_ptr->dis_item_multiplicity_mp),
                          source_data_ptr->dis_item_multiplicity_mp));
        EPETE(copy_matrix(&(held_out_data_ptr->dis_item_multiplicity_mp),
                          source_data_ptr->dis_item_multiplicity_mp));
    }
    if((source_data_ptr->dis_item_aux_index_vvp != NULL) &&
       (source_data_ptr->dis_item_aux_index_vvp->length == num_images))
    {
        ERE(split_int_vector_vector(&(train_data_ptr->dis_item_aux_index_vvp),
                                    &(held_out_data_ptr->dis_item_aux_index_vvp),
                                    source_data_ptr->dis_item_aux_index_vvp,
                                    training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector_vector(&(train_data_ptr->dis_item_aux_index_vvp),
                                   source_data_ptr->dis_item_aux_index_vvp));
        ERE(copy_int_vector_vector(&(held_out_data_ptr->dis_item_aux_index_vvp),
                                   source_data_ptr->dis_item_aux_index_vvp));

    }

    if((source_data_ptr->dis_item_aux_value_vvp != NULL) &&
       (source_data_ptr->dis_item_aux_value_vvp->length == num_images))
    {
        ERE(split_vector_vector(&(train_data_ptr->dis_item_aux_value_vvp),
                                &(held_out_data_ptr->dis_item_aux_value_vvp),
                                source_data_ptr->dis_item_aux_value_vvp,
                                training_image_nums_vp));
    }
    else
    {
        ERE(copy_vector_vector(&(train_data_ptr->dis_item_aux_value_vvp),
                               source_data_ptr->dis_item_aux_value_vvp));

        ERE(copy_vector_vector(&(held_out_data_ptr->dis_item_aux_value_vvp),
                               source_data_ptr->dis_item_aux_value_vvp));
    }

    if((source_data_ptr->dis_item_aux_mp != NULL) &&
       (source_data_ptr->dis_item_aux_mp->num_rows == num_images))
    {
        ERE(split_matrix_by_rows(&(train_data_ptr->dis_item_aux_mp),
                                 &(held_out_data_ptr->dis_item_aux_mp),
                                 source_data_ptr->dis_item_aux_mp,
                                 training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix(&(train_data_ptr->dis_item_aux_mp), source_data_ptr->dis_item_aux_mp));
        ERE(copy_matrix(&(held_out_data_ptr->dis_item_aux_mp), source_data_ptr->dis_item_aux_mp));

    }

    if((source_data_ptr->dis_item_observed_counts_mp != NULL) &&
       (source_data_ptr->dis_item_observed_counts_mp->num_rows == num_images))
    {
        ERE(split_int_matrix_by_rows(&(train_data_ptr->dis_item_observed_counts_mp),
                                     &(held_out_data_ptr->dis_item_observed_counts_mp),
                                     source_data_ptr->dis_item_observed_counts_mp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_matrix(&(train_data_ptr->dis_item_observed_counts_mp),
                            source_data_ptr->dis_item_observed_counts_mp));
        ERE(copy_int_matrix(&(held_out_data_ptr->dis_item_observed_counts_mp),
                            source_data_ptr->dis_item_observed_counts_mp));

    }

    if(( source_data_ptr->dis_item_derived_counts_mp != NULL) &&
       ( source_data_ptr->dis_item_derived_counts_mp->num_rows == num_images))
    {
        ERE(split_int_matrix_by_rows(&(train_data_ptr->dis_item_derived_counts_mp),
                                     &(held_out_data_ptr->dis_item_derived_counts_mp),
                                     source_data_ptr->dis_item_derived_counts_mp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_matrix(&(train_data_ptr->dis_item_derived_counts_mp),
                            source_data_ptr->dis_item_derived_counts_mp));
        ERE(copy_int_matrix(&(held_out_data_ptr->dis_item_derived_counts_mp),
                            source_data_ptr->dis_item_derived_counts_mp));
    }

    if((source_data_ptr->con_predict_flag_vp != NULL) &&
       (source_data_ptr->con_predict_flag_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->con_predict_flag_vp),
                             &(held_out_data_ptr->con_predict_flag_vp),
                             source_data_ptr->con_predict_flag_vp,
                             training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector(&(train_data_ptr->con_predict_flag_vp),
                            source_data_ptr->con_predict_flag_vp));

        ERE(copy_int_vector(&(held_out_data_ptr->con_predict_flag_vp),
                            source_data_ptr->con_predict_flag_vp));
    }

    if((source_data_ptr->con_item_mvp != NULL) &&
       (source_data_ptr->con_item_mvp->length == num_images))
    {
        ERE(split_matrix_vector(&(train_data_ptr->con_item_mvp),
                                &(held_out_data_ptr->con_item_mvp),
                                source_data_ptr->con_item_mvp,
                                training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix_vector(&(train_data_ptr->con_item_mvp),source_data_ptr->con_item_mvp));
        ERE(copy_matrix_vector(&(held_out_data_ptr->con_item_mvp),source_data_ptr->con_item_mvp));

    }

    if((source_data_ptr->con_item_token_mp != NULL)&&
       (source_data_ptr->con_item_token_mp->num_rows == num_images))
    {
        ERE(split_int_matrix_by_rows(&(train_data_ptr->con_item_token_mp),
                                     &(held_out_data_ptr->con_item_token_mp),
                                     source_data_ptr->con_item_token_mp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_matrix(&(train_data_ptr->con_item_token_mp),source_data_ptr->con_item_token_mp));
        ERE(copy_int_matrix(&(held_out_data_ptr->con_item_token_mp),source_data_ptr->con_item_token_mp));

    }

    if((source_data_ptr->con_item_info_mvp != NULL) &&
       (source_data_ptr->con_item_info_mvp->length == num_images))
    {
        ERE(split_matrix_vector(&(train_data_ptr->con_item_info_mvp),
                                &(held_out_data_ptr->con_item_info_mvp),
                                source_data_ptr->con_item_info_mvp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix_vector(&(train_data_ptr->con_item_info_mvp),source_data_ptr->con_item_info_mvp));
        ERE(copy_matrix_vector(&(held_out_data_ptr->con_item_info_mvp),source_data_ptr->con_item_info_mvp));

    }

    if((source_data_ptr->con_item_index_vvp != NULL) &&
       (source_data_ptr->con_item_index_vvp->length == num_images))
    {
        ERE(split_int_vector_vector(&(train_data_ptr->con_item_index_vvp),
                                    &(held_out_data_ptr->con_item_index_vvp),
                                    source_data_ptr->con_item_index_vvp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector_vector(&(train_data_ptr->con_item_index_vvp),
                                   source_data_ptr->con_item_index_vvp));
        ERE(copy_int_vector_vector(&(held_out_data_ptr->con_item_index_vvp),
                                   source_data_ptr->con_item_index_vvp));
    }

    if((source_data_ptr->con_item_merge_prior_mvp != NULL) &&
       (source_data_ptr->con_item_merge_prior_mvp->length == num_images))
    {
        ERE(split_matrix_vector(&(train_data_ptr->con_item_merge_prior_mvp),
                                &(held_out_data_ptr->con_item_merge_prior_mvp),
                                source_data_ptr->con_item_merge_prior_mvp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix_vector(&(train_data_ptr->con_item_merge_prior_mvp),
                               source_data_ptr->con_item_merge_prior_mvp));
        ERE(copy_matrix_vector(&(held_out_data_ptr->con_item_merge_prior_mvp),
                               source_data_ptr->con_item_merge_prior_mvp));

    }

    if((source_data_ptr->vec_item_vvvvp != NULL) &&
       (source_data_ptr->vec_item_vvvvp->length == num_images))
    {
        ERE(split_v4(&(train_data_ptr->vec_item_vvvvp),
                     &(held_out_data_ptr->vec_item_vvvvp),
                     source_data_ptr->vec_item_vvvvp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_v4(&(train_data_ptr->vec_item_vvvvp),source_data_ptr->vec_item_vvvvp));
        ERE(copy_v4(&(held_out_data_ptr->vec_item_vvvvp),source_data_ptr->vec_item_vvvvp));
    }

    if((source_data_ptr->vec_num_dim_vp != NULL) &&
       (source_data_ptr->vec_num_dim_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->vec_num_dim_vp),
                             &(held_out_data_ptr->vec_num_dim_vp),
                             source_data_ptr->vec_num_dim_vp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector(&(train_data_ptr->vec_num_dim_vp),
                            source_data_ptr->vec_num_dim_vp));
        ERE(copy_int_vector(&(held_out_data_ptr->vec_num_dim_vp),
                            source_data_ptr->vec_num_dim_vp));
    }

    if((source_data_ptr->his_item_vvvvp != NULL) &&
       (source_data_ptr->his_item_vvvvp->length == num_images))
    {
        ERE(split_v4(&(train_data_ptr->his_item_vvvvp),
                     &(held_out_data_ptr->his_item_vvvvp),
                     source_data_ptr->his_item_vvvvp,training_image_nums_vp));
    }
    else
    {
        ERE(copy_v4(&(train_data_ptr->his_item_vvvvp),
                    source_data_ptr->his_item_vvvvp));
        ERE(copy_v4(&(held_out_data_ptr->his_item_vvvvp),
                    source_data_ptr->his_item_vvvvp));
    }

    if((source_data_ptr->his_num_dim_vp != NULL) &&
       (source_data_ptr->his_num_dim_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->his_num_dim_vp),
                             &(held_out_data_ptr->his_num_dim_vp),
                             source_data_ptr->his_num_dim_vp, training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector(&(train_data_ptr->his_num_dim_vp),
                            source_data_ptr->his_num_dim_vp));
        ERE(copy_int_vector(&(held_out_data_ptr->his_num_dim_vp),
                            source_data_ptr->his_num_dim_vp));

    }

    if((source_data_ptr->id_vp != NULL) &&
       (source_data_ptr->id_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->id_vp),
                             &(held_out_data_ptr->id_vp),
                             source_data_ptr->id_vp, training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector(&(train_data_ptr->id_vp),
                            source_data_ptr->id_vp));
        ERE(copy_int_vector(&(held_out_data_ptr->id_vp),
                            source_data_ptr->id_vp));
    }

    if((source_data_ptr->label_vp != NULL) &&
       (source_data_ptr->label_vp->length == num_images))
    {
        ERE(split_int_vector(&(train_data_ptr->label_vp),
                             &(held_out_data_ptr->label_vp),
                             source_data_ptr->label_vp, training_image_nums_vp));
    }
    else
    {
        ERE(copy_int_vector(&(train_data_ptr->label_vp),
                            source_data_ptr->label_vp));
        ERE(copy_int_vector(&(held_out_data_ptr->label_vp),
                            source_data_ptr->label_vp));
    }


    /* Kobus: Can we kill the following? */

    /*
#ifdef NEVER_USED
ERE(copy_matrix(&(target_data_ptr->dis_category_level_prior_mp),
source_data_ptr->dis_category_level_prior_mp));
#endif
    */

    /* Kobus. */

    if((source_data_ptr->blob_word_posterior_mvp != NULL) &&
       (source_data_ptr->blob_word_posterior_mvp->length == num_images))
    {
        ERE(split_matrix_vector(&(train_data_ptr->blob_word_posterior_mvp),
                                &(held_out_data_ptr->blob_word_posterior_mvp),
                                source_data_ptr->blob_word_posterior_mvp,
                                training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix_vector(&(train_data_ptr->blob_word_posterior_mvp),source_data_ptr->blob_word_posterior_mvp));
        ERE(copy_matrix_vector(&(held_out_data_ptr->blob_word_posterior_mvp),source_data_ptr->blob_word_posterior_mvp));

    }

    if((source_data_ptr->blob_word_prior_mvp != NULL) &&
       (source_data_ptr->blob_word_prior_mvp->length == num_images))
    {
        ERE(split_matrix_vector(&(train_data_ptr->blob_word_prior_mvp),
                                &(held_out_data_ptr->blob_word_prior_mvp),
                                source_data_ptr->blob_word_prior_mvp,
                                training_image_nums_vp));
    }
    else
    {
        ERE(copy_matrix_vector(&(train_data_ptr->blob_word_prior_mvp),source_data_ptr->blob_word_prior_mvp));
        ERE(copy_matrix_vector(&(held_out_data_ptr->blob_word_prior_mvp),source_data_ptr->blob_word_prior_mvp));

    }

    /* End Kobus. */

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_target_cluster_data_like_source
(
    Cluster_data**      target_data_ptr_ptr,
    const Cluster_data* source_data_ptr,
    int                 num_points
)
{
    Cluster_data* target_data_ptr = *target_data_ptr_ptr;


    /*
     * In support of one version of do_hierarchical_em_2() only. We DO NOT deal
     * with axu data at this point because this module does not care about it.
    */

    if (source_data_ptr == NULL)
    {
        free_cluster_data(*target_data_ptr_ptr);
        *target_data_ptr_ptr = NULL;
        return NO_ERROR;
    }

    if (target_data_ptr == NULL)
    {
        NRE(target_data_ptr = allocate_cluster_data());
        *target_data_ptr_ptr = target_data_ptr;
    }

    if (num_points < 0)
    {
        num_points = source_data_ptr->num_points;
    }

    target_data_ptr->num_points = num_points;
    target_data_ptr->num_categories = source_data_ptr->num_categories;
    target_data_ptr->num_con_features = source_data_ptr->num_con_features;
    target_data_ptr->num_vec_features = source_data_ptr->num_vec_features;
    target_data_ptr->num_his_features = source_data_ptr->num_his_features;
    target_data_ptr->num_tokens = source_data_ptr->num_tokens;
    target_data_ptr->max_num_dis_items = source_data_ptr->max_num_dis_items;
    target_data_ptr->max_num_dis_items_with_multiplicity = source_data_ptr->max_num_dis_items_with_multiplicity;
    target_data_ptr->max_num_con_items = source_data_ptr->max_num_con_items;
    target_data_ptr->max_num_vec_items = source_data_ptr->max_num_vec_items;
    target_data_ptr->max_num_his_items = source_data_ptr->max_num_his_items;

    if (source_data_ptr->group_vp != NULL)
    {
        ERE(get_target_int_vector(&(target_data_ptr->group_vp), num_points));
    }

    if (source_data_ptr->weight_vp != NULL)
    {
        ERE(get_target_vector(&(target_data_ptr->weight_vp), num_points));
    }

    if (source_data_ptr->con_item_weight_vvp != NULL)
    {
        ERE(get_target_vector_vector(&(target_data_ptr->con_item_weight_vvp),
                                     source_data_ptr->con_item_weight_vvp->length));
    }

    if (source_data_ptr->dis_predict_flag_vp != NULL)
    {
        ERE(get_target_int_vector(&(target_data_ptr->dis_predict_flag_vp),
                                  num_points));
    }

    if (source_data_ptr->dis_item_mp != NULL)
    {
        int num_cols = source_data_ptr->dis_item_mp->num_cols;

        ERE(get_target_int_matrix(&(target_data_ptr->dis_item_mp),
                                  num_points, num_cols));

        ERE(get_target_matrix(&(target_data_ptr->dis_item_multiplicity_mp),
                              num_points, num_cols));

        ERE(get_target_int_matrix(
                              &(target_data_ptr->dis_item_observed_counts_mp),
                              num_points, num_cols));

        ERE(get_target_int_matrix(
                              &(target_data_ptr->dis_item_derived_counts_mp),
                              num_points, num_cols));

#ifdef NEVER_USED
        ERE(get_target_matrix(&(target_data_ptr->dis_category_level_prior_mp),
                              num_points, num_cols));
#endif

    }

    if (source_data_ptr->con_predict_flag_vp != NULL)
    {
        ERE(get_target_int_vector(&(target_data_ptr->con_predict_flag_vp), num_points));
    }

    if (source_data_ptr->con_item_token_mp != NULL)
    {
        int num_cols = source_data_ptr->con_item_token_mp->num_cols;

        ERE(get_target_int_matrix(&(target_data_ptr->con_item_token_mp),
                                  num_points, num_cols));
    }

    if (source_data_ptr->con_item_mvp != NULL)
    {
        ERE(get_target_matrix_vector(&(target_data_ptr->con_item_mvp),
                                     num_points));
    }

    if (source_data_ptr->con_item_info_mvp != NULL)
    {
        ERE(get_target_matrix_vector(&(target_data_ptr->con_item_info_mvp),
                                     num_points));
    }

    if (source_data_ptr->con_item_index_vvp != NULL)
    {
        ERE(get_target_int_vector_vector(&(target_data_ptr->con_item_index_vvp),
                                         num_points));
    }

    if (source_data_ptr->con_item_merge_prior_mvp != NULL)
    {
        ERE(get_target_matrix_vector(&(target_data_ptr->con_item_merge_prior_mvp),
                                     num_points));
    }

    if (source_data_ptr->vec_item_vvvvp != NULL)
    {
        ERE(get_target_v4(&(target_data_ptr->vec_item_vvvvp), num_points));
        ERE(get_target_int_vector(&(target_data_ptr->vec_num_dim_vp),
                                  source_data_ptr->vec_num_dim_vp->length));
    }

    if (source_data_ptr->his_item_vvvvp != NULL)
    {
        ERE(get_target_v4(&(target_data_ptr->his_item_vvvvp), num_points));
        ERE(get_target_int_vector(&(target_data_ptr->his_num_dim_vp),
                                  source_data_ptr->his_num_dim_vp->length));
    }

    if (source_data_ptr->id_vp != NULL)
    {
        ERE(get_target_int_vector(&(target_data_ptr->id_vp), num_points));
    }

    if (source_data_ptr->label_vp != NULL)
    {
        ERE(get_target_int_vector(&(target_data_ptr->label_vp), num_points));
    }

    if (source_data_ptr->blob_word_posterior_mvp != NULL)
    {
        ERE(get_target_matrix_vector(&(target_data_ptr->blob_word_posterior_mvp),
                                     num_points));
    }

    if (source_data_ptr->blob_word_prior_mvp != NULL)
    {
        ERE(get_target_matrix_vector(&(target_data_ptr->blob_word_prior_mvp),
                                     num_points));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_max_num_cluster_data_items(Cluster_data* data_ptr)
{
    int            max_num_dis_items = 0;
    double         max_num_dis_items_with_multiplicity = 0.0;
    int            max_num_con_items = 0;
    int            max_num_vec_items = 0;
    int            max_num_his_items = 0;
    Matrix*        dis_item_multiplicity_mp = data_ptr->dis_item_multiplicity_mp;
    Int_matrix*    dis_item_mp       = data_ptr->dis_item_mp;
    Matrix_vector* con_item_mvp      = data_ptr->con_item_mvp;
    V_v_v_v*       vec_item_vvvvp    = data_ptr->vec_item_vvvvp;
    V_v_v_v*       his_item_vvvvp    = data_ptr->his_item_vvvvp;


    if (dis_item_mp != NULL)
    {
        int num_rows = dis_item_mp->num_rows;
        int num_cols = dis_item_mp->num_cols;
        int i, j;

        if (dis_item_multiplicity_mp == NULL)
        {
            warn_pso("dis_item_multiplicity_mp is NULL in set_max_num_cluster_data_items.\n");
        }

        for (i = 0; i < num_rows; i++)
        {
            int num_dis_items = 0;
            double num_dis_items_with_multiplicity = 0.0;

            for (j = 0; j < num_cols; j++)
            {
                if (dis_item_mp->elements[ i ][ j ] >= 0)
                {
                    num_dis_items++;

                    if (dis_item_multiplicity_mp != NULL)
                    {
                        num_dis_items_with_multiplicity += dis_item_multiplicity_mp->elements[ i ][ j ];
                    }
                }
            }

            max_num_dis_items = MAX_OF(max_num_dis_items, num_dis_items);

            max_num_dis_items_with_multiplicity =
                MAX_OF(max_num_dis_items_with_multiplicity,
                       num_dis_items_with_multiplicity);
        }
    }

    data_ptr->max_num_dis_items = max_num_dis_items;
    data_ptr->max_num_dis_items_with_multiplicity = max_num_dis_items_with_multiplicity;

    if (con_item_mvp != NULL)
    {
        int i;

        for (i = 0; i < con_item_mvp->length; i++)
        {
            int num_con_items = (con_item_mvp->elements[ i ] == NULL) ? 0 : con_item_mvp->elements[ i ]->num_rows;

            max_num_con_items = MAX_OF(max_num_con_items, num_con_items);
        }
    }

    data_ptr->max_num_con_items = max_num_con_items;

    if (vec_item_vvvvp != NULL)
    {
        int i;

        for (i = 0; i < vec_item_vvvvp->length; i++)
        {
            int num_vec_items = (vec_item_vvvvp->elements[ i ] == NULL) ? 0 : vec_item_vvvvp->elements[ i ]->length;

            max_num_vec_items = MAX_OF(max_num_vec_items, num_vec_items);
        }
    }

    data_ptr->max_num_vec_items = max_num_vec_items;


    if (his_item_vvvvp != NULL)
    {
        int i;

        for (i = 0; i < his_item_vvvvp->length; i++)
        {
            int num_his_items = (his_item_vvvvp->elements[ i ] == NULL) ? 0 : his_item_vvvvp->elements[ i ]->length;

            max_num_his_items = MAX_OF(max_num_his_items, num_his_items);
        }
    }

    data_ptr->max_num_his_items = max_num_his_items;


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_cluster_data(Cluster_data* data_ptr)
{

    if (data_ptr != NULL)
    {
        free_vector(data_ptr->weight_vp);
        free_int_vector(data_ptr->group_vp);
        free_vector_vector(data_ptr->con_item_weight_vvp);
        free_int_matrix(data_ptr->dis_item_mp);
        free_int_vector(data_ptr->dis_predict_flag_vp);
        free_matrix(data_ptr->dis_item_multiplicity_mp);
        free_int_vector_vector(data_ptr->dis_item_aux_index_vvp);
        free_vector_vector(data_ptr->dis_item_aux_value_vvp);
        free_matrix(data_ptr->dis_item_aux_mp);
        free_int_matrix(data_ptr->dis_item_derived_counts_mp);
        free_int_matrix(data_ptr->dis_item_observed_counts_mp);
#ifdef NEVER_USED
        free_matrix(data_ptr->dis_category_level_prior_mp);
#endif
        free_int_vector(data_ptr->con_predict_flag_vp);
        free_int_matrix(data_ptr->con_item_token_mp);
        free_matrix_vector(data_ptr->con_item_mvp);
        free_matrix_vector(data_ptr->con_item_info_mvp);
        free_int_vector_vector(data_ptr->con_item_index_vvp);
        free_matrix_vector(data_ptr->con_item_merge_prior_mvp);
        free_v4(data_ptr->vec_item_vvvvp);
        free_v4(data_ptr->his_item_vvvvp);
        free_int_vector(data_ptr->vec_num_dim_vp);
        free_int_vector(data_ptr->his_num_dim_vp);
        free_int_vector(data_ptr->id_vp);
        free_int_vector(data_ptr->label_vp);
        free_matrix_vector(data_ptr->blob_word_posterior_mvp);
        free_matrix_vector(data_ptr->blob_word_prior_mvp);
        kjb_free(data_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int add_discrete_nulls_to_data
(
    Cluster_data**      data_with_nulls_pp,
    const Cluster_data* data_ptr
)
{
    Cluster_data* data_with_nulls_ptr;
    Int_matrix*   dis_item_with_nulls_mp = NULL;
    Matrix*       dis_item_with_nulls_multiplicity_mp = NULL;
    int           i;


    ERE(copy_cluster_data(data_with_nulls_pp, data_ptr));
    data_with_nulls_ptr = *data_with_nulls_pp;

    (data_with_nulls_ptr->max_num_dis_items_with_multiplicity) += 1.0;
    (data_with_nulls_ptr->max_num_dis_items)++;
    (data_with_nulls_ptr->num_categories)++;

    ERE(get_initialized_int_matrix(&dis_item_with_nulls_mp,
                                   data_ptr->dis_item_mp->num_rows,
                                   1 + data_ptr->dis_item_mp->num_cols,
                                   NOT_SET));
    ERE(ow_copy_int_matrix(dis_item_with_nulls_mp, 0, 0, data_ptr->dis_item_mp));

    ERE(get_zero_matrix(&dis_item_with_nulls_multiplicity_mp,
                        data_ptr->dis_item_multiplicity_mp->num_rows,
                        1 + data_ptr->dis_item_multiplicity_mp->num_cols));
    ERE(ow_copy_matrix(dis_item_with_nulls_multiplicity_mp, 0, 0,
                       data_ptr->dis_item_multiplicity_mp));

    for (i = 0; i < data_ptr->dis_item_mp->num_rows; i++)
    {
        int j = 0;

        while (dis_item_with_nulls_mp->elements[ i ][ j ] >= 0)
        {
            j++;
        }

        ASSERT(j < dis_item_with_nulls_mp->num_cols);

        dis_item_with_nulls_mp->elements[ i ][ j ] = data_ptr->num_categories;
        dis_item_with_nulls_multiplicity_mp->elements[ i ][ j ] = 1.0;
    }

    ERE(copy_int_matrix(&(data_with_nulls_ptr->dis_item_mp),
                        dis_item_with_nulls_mp));

    ERE(copy_matrix(&(data_with_nulls_ptr->dis_item_multiplicity_mp),
                    dis_item_with_nulls_multiplicity_mp));

    free_int_matrix(dis_item_with_nulls_mp);
    free_matrix(dis_item_with_nulls_multiplicity_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int subtract_discrete_nulls_from_model(Multi_modal_model* model_ptr)
{
    Matrix_vector* P_i_n_mvp = model_ptr->P_i_n_mvp;
    int i;

    if (P_i_n_mvp == NULL) return NO_ERROR;

    for (i = 0; i < P_i_n_mvp->length; i++)
    {
        /* We don't normalize so we know how much the null was worth. */
        (P_i_n_mvp->elements[ i ]->num_cols)--;
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Multi_modal_model* allocate_multi_modal_model(void)
{
    Multi_modal_model* model_ptr;

    NRN(model_ptr = TYPE_MALLOC(Multi_modal_model));

    NRN(model_ptr->topology_ptr = allocate_topology());

    model_ptr->con_feature_enable_vp = NULL;
    model_ptr->con_feature_group_vp = NULL;
    model_ptr->con_feature_weight_vp = NULL;


    model_ptr->extra_dis_prob        = 0.0;
    model_ptr->extra_con_prob        = 0.0;

    model_ptr->dis_item_prob_threshold        = DBL_NOT_SET;

    model_ptr->ignore_dis_prob_fraction       = 0.0;
    model_ptr->ignore_con_prob_fraction       = 0.0;

    model_ptr->dis_prob_boost_factor          = DBL_NOT_SET;
    model_ptr->con_prob_boost_factor          = DBL_NOT_SET;
    model_ptr->norm_items                     = NOT_SET;
    model_ptr->norm_continuous_features       = NOT_SET;

    model_ptr->max_con_norm_var = DBL_HALF_MOST_NEGATIVE;
    model_ptr->min_con_log_prob = DBL_HALF_MOST_NEGATIVE;
    model_ptr->con_log_norm = DBL_HALF_MOST_NEGATIVE;
    model_ptr->con_score_cutoff = DBL_HALF_MOST_NEGATIVE;

    model_ptr->var_offset                                   = NOT_SET;

    model_ptr->weight_con_items                             = FALSE;

    model_ptr->discrete_tie_count                           = 1;

    model_ptr->phase_one_model_correspondence               = FALSE;
    model_ptr->phase_two_model_correspondence               = FALSE;
    model_ptr->phase_one_share_matches                      = TRUE;
    model_ptr->phase_two_share_matches                      = TRUE;
    model_ptr->phase_one_sample_matches                     = FALSE;
    model_ptr->phase_two_sample_matches                     = FALSE;
    model_ptr->phase_one_last_sample                        = NOT_SET;
    model_ptr->phase_two_last_sample                        = NOT_SET;
    model_ptr->phase_one_num_iterations                     = NOT_SET;
    model_ptr->phase_two_num_iterations                     = NOT_SET;
    model_ptr->phase_one_uniform_vertical_dist              = NOT_SET;
    model_ptr->phase_two_uniform_vertical_dist              = NOT_SET;
    model_ptr->phase_one_cluster_dependent_vertical_dist    = NOT_SET;
    model_ptr->phase_two_cluster_dependent_vertical_dist    = NOT_SET;

    model_ptr->num_clusters = 0;
    model_ptr->num_categories = 0;
    model_ptr->num_con_features = 0;
    model_ptr->raw_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    model_ptr->mdl_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    model_ptr->aic = DBL_HALF_MOST_NEGATIVE;
    model_ptr->num_parameters = NOT_SET;
    model_ptr->a_vp = NULL;
    model_ptr->V_mp = NULL;
    model_ptr->V_vp = NULL;
    model_ptr->con_V_mp = NULL;
    model_ptr->dis_V_mp = NULL;
    model_ptr->fit_vp = NULL;
    model_ptr->P_c_p_mp = NULL;
    model_ptr->P_l_p_c_mvp = NULL;
    model_ptr->P_l_p_mp = NULL;
    model_ptr->P_i_n_mvp = NULL;
    model_ptr->mean_mvp = NULL;
    model_ptr->var_mvp  = NULL;
    model_ptr->con_log_sqrt_det_vvp  = NULL;
    model_ptr->mean_mp = NULL;
    model_ptr->var_mp  = NULL;
    model_ptr->con_log_sqrt_det_vp  = NULL;
    model_ptr->vec_mean_vvvvp  = NULL;
    model_ptr->vec_var_vvvp  = NULL;
    model_ptr->his_mean_vvvvp  = NULL;
    model_ptr->his_var_vvvp  = NULL;
    model_ptr->data_ptr = NULL;

    return model_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_un_normalize_multi_modal_model
(
    Multi_modal_model* model_ptr,
    const Vector*       mean_vp,
    const Vector*       var_vp,
    double              norm_stdev
)
{
    Vector* stdev_vp     = NULL;
    Vector* log_stdev_vp = NULL;


    if ((var_vp == NULL) || (mean_vp == NULL) || (norm_stdev < 0.0))
    {
        /* Presumably the model is not respect to a normalized data set. */
        return NO_ERROR;
    }

    if (model_ptr != NULL)
    {
        Matrix_vector* model_mean_mvp  = model_ptr->mean_mvp;
        Matrix_vector* model_var_mvp = model_ptr->var_mvp;
        Matrix* model_mean_mp  = model_ptr->mean_mp;
        Matrix* model_var_mp = model_ptr->var_mp;
#ifdef LOG_SQRT_DET_ALREADY_CORRECT
        Vector_vector* model_con_log_sqrt_det_vvp = model_ptr->con_log_sqrt_det_vvp;
        Vector_vector* model_con_log_sqrt_det_vp = model_ptr->con_log_sqrt_det_vp;
#endif
        int            i;

        if (model_mean_mvp != NULL)
        {
#ifdef LOG_SQRT_DET_ALREADY_CORRECT
            double log_sqrt_det;
            double log_sqrt_det_extra;
#endif

            ERE(sqrt_vector(&stdev_vp, var_vp));

#ifdef LOG_SQRT_DET_ALREADY_CORRECT
            ERE(log_vector(&log_stdev_vp, stdev_vp, LOG_ZERO));
            log_sqrt_det = sum_vector_elements(log_stdev_vp);
#endif

            for (i = 0; i < model_mean_mvp->length; i++)
            {
                if (model_mean_mvp->elements[ i ] != NULL)
                {
                    Matrix* model_mean_i_mp = model_mean_mvp->elements[ i ];
                    Matrix* model_var_i_mp = model_var_mvp->elements[ i ];
#ifdef LOG_SQRT_DET_ALREADY_CORRECT
                    Vector* model_log_sqrt_det_vp =
                                    model_con_log_sqrt_det_vvp->elements[ i ];
                    int     num_dim = model_var_i_mp->num_cols;

                    log_sqrt_det_extra = num_dim * log(norm_stdev) - log_sqrt_det;

                    /*
                    // Un-normalize log sqrt determinant.
                    */
                    ERE(ow_add_scalar_to_vector(model_log_sqrt_det_vp,
                                                log_sqrt_det_extra));
#endif
                    /*
                    // Un-normalize variances
                    */
                    ERE(ow_multiply_matrix_by_row_vector_ew(model_var_i_mp,
                                                            var_vp));
                    ERE(ow_divide_matrix_by_scalar(model_var_i_mp,
                                                   norm_stdev * norm_stdev));

                    /*
                    // Un-normalize means
                    */
                    ERE(ow_multiply_matrix_by_row_vector_ew(model_mean_i_mp,
                                                            stdev_vp));
                    ERE(ow_divide_matrix_by_scalar(model_mean_i_mp, norm_stdev));
                    ERE(ow_add_row_vector_to_matrix(model_mean_i_mp, mean_vp));
                }
            }
        }

        if (model_mean_mp != NULL)
        {
#ifdef LOG_SQRT_DET_ALREADY_CORRECT
            double log_sqrt_det;
            double log_sqrt_det_extra;
            int     num_dim = model_var_mp->num_cols;
#endif
            UNTESTED_CODE();

            ERE(sqrt_vector(&stdev_vp, var_vp));

#ifdef LOG_SQRT_DET_ALREADY_CORRECT
            ERE(log_vector(&log_stdev_vp, stdev_vp, LOG_ZERO));
            log_sqrt_det = sum_vector_elements(log_stdev_vp);
#endif

#ifdef LOG_SQRT_DET_ALREADY_CORRECT
            log_sqrt_det_extra = num_dim * log(norm_stdev) - log_sqrt_det;

            /*
            // Un-normalize log sqrt determinant.
            */
            ERE(ow_add_scalar_to_vector(model_log_sqrt_det_vp,
                                        log_sqrt_det_extra));
#endif
            /*
            // Un-normalize variances
            */
            ERE(ow_multiply_matrix_by_row_vector_ew(model_var_mp,
                                                    var_vp));
            ERE(ow_divide_matrix_by_scalar(model_var_mp,
                                           norm_stdev * norm_stdev));

            /*
            // Un-normalize means
            */
            ERE(ow_multiply_matrix_by_row_vector_ew(model_mean_mp,
                                                    stdev_vp));
            ERE(ow_divide_matrix_by_scalar(model_mean_mp, norm_stdev));
            ERE(ow_add_row_vector_to_matrix(model_mean_mp, mean_vp));
        }
    }

    free_vector(stdev_vp);
    free_vector(log_stdev_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_multi_modal_model(Multi_modal_model* model_ptr)
{

    if (model_ptr == NULL) return;

    free_topology(model_ptr->topology_ptr);

    free_int_vector(model_ptr->con_feature_enable_vp);
    free_int_vector(model_ptr->con_feature_group_vp);
    free_vector(model_ptr->con_feature_weight_vp);

    free_vector(model_ptr->a_vp);

    free_vector(model_ptr->V_vp);
    free_matrix(model_ptr->V_mp);
    free_matrix(model_ptr->con_V_mp);
    free_matrix(model_ptr->dis_V_mp);
    free_vector(model_ptr->fit_vp);
    free_matrix(model_ptr->P_c_p_mp);
    free_matrix_vector(model_ptr->P_l_p_c_mvp);
    free_matrix(model_ptr->P_l_p_mp);
    free_matrix_vector(model_ptr->P_i_n_mvp);
    free_matrix_vector(model_ptr->mean_mvp);
    free_matrix_vector(model_ptr->var_mvp );
    free_vector_vector(model_ptr->con_log_sqrt_det_vvp);
    free_matrix(model_ptr->mean_mp);
    free_matrix(model_ptr->var_mp );
    free_vector(model_ptr->con_log_sqrt_det_vp);
    free_v4(model_ptr->vec_mean_vvvvp);
    free_v3(model_ptr->vec_var_vvvp);
    free_v4(model_ptr->his_mean_vvvvp);
    free_v3(model_ptr->his_var_vvvp);
    free_cluster_data(model_ptr->data_ptr);

    kjb_free(model_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void verify_cluster_data(const Cluster_data* data_ptr)
{

    if (data_ptr == NULL) return;

    if (data_ptr->con_item_mvp != NULL)
    {
        verify_matrix_vector(data_ptr->con_item_mvp, NULL);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           use_sampling,
    int           num_nulls,
    int           duplicate_words_for_matching
)
{
    int result = ERROR;


    if (use_sampling)
    {
        static int first_time = TRUE;

        if (first_time)
        {
            pso("Sampling matches.\n");
        }

        result = get_sampled_matches(match_mpp, num_matches_ptr,
                                     log_pair_probs_mp,
                                     duplicate_words_for_matching);
        first_time = FALSE;
    }
    else
    {
        static int first_time = TRUE;

        if (first_time)
        {
            pso("Using graph matching\n");
        }

        result = get_hungarian_matches(match_mpp, num_matches_ptr,
                                       log_pair_probs_mp, num_nulls,
                                       duplicate_words_for_matching);

        first_time = FALSE;
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_sampled_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           duplicate_words_for_matching
)
{
    int         num_con_items      = log_pair_probs_mp->num_rows;
    int         num_dis_items      = log_pair_probs_mp->num_cols;
    int         con_item;
    int         dis_item;
    int         done_all_dis_items = FALSE;
    int         done_all_con_items = FALSE;
    Int_vector* con_item_found_vp  = NULL;
    Int_vector* dis_item_found_vp  = NULL;
    int         max_num_matches = num_con_items;
    Matrix*     sample_mp          = NULL;
    int         match_count              = 0;
    int         i;
    int         result = NO_ERROR;
    double      sampled_prob;


    ERE(get_target_int_matrix(match_mpp, max_num_matches, 2));

#ifdef TEST
    ERE(ow_set_int_matrix((*match_mpp), NOT_SET));
#endif

    if (result != ERROR)
    {
        result = copy_matrix(&sample_mp, log_pair_probs_mp);
    }

    if (result != ERROR)
    {
        result = get_initialized_int_vector(&con_item_found_vp, num_con_items,
                                            FALSE);
    }

    if (result != ERROR)
    {
        result = get_initialized_int_vector(&dis_item_found_vp, num_dis_items,
                                            FALSE);
    }

     /* db_mat(sample_mp);  */

    while (TRUE)
    {
        if (result == ERROR) break;

        done_all_con_items = TRUE;

        for (con_item = 0; con_item < num_con_items; con_item++)
        {
            if (! con_item_found_vp->elements[ con_item ])
            {
                done_all_con_items = FALSE;
            }
        }

        done_all_dis_items = TRUE;

        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
        {
            if (! dis_item_found_vp->elements[ dis_item ])
            {
                done_all_dis_items = FALSE;
            }
        }

        if (done_all_con_items)
        {
#ifdef HOW_IT_WAS_AND_PERHAPS_SHOULD_BE
            /*
            // This makes is so that all data is accounted for. However, to be
            // consistent with the graph matching method, once we have matched
            // all the con items, we will stop. It is not clear what should be
            // done, but likely we want to be consistent over the two cases.
            // The same applies to additional playing with NULLs.
            */

            if (done_all_dis_items) break;

             /* dbp("++++++++++++++++++++++++++++++++++++++++");  */

            if (match_count >= max_num_matches)
            {
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }

             /* dbw(); */

            done_all_con_items = FALSE;

            for (con_item = 0; con_item < num_con_items; con_item++)
            {
                con_item_found_vp->elements[ con_item ] = FALSE;

                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                {
                    if (! dis_item_found_vp->elements[ dis_item ])
                    {
                        sample_mp->elements[ con_item ][ dis_item ] =
                            log_pair_probs_mp->elements[ con_item ][ dis_item ];
                    }
                }
            }
#else
            /* The new way--once we have matched all the con items, we are
            // done. The leftover dis items are not matched.
            */
            break;
#endif
        }

        if (done_all_dis_items)
        {
            if (! duplicate_words_for_matching) break;

            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
            {
                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    if (! con_item_found_vp->elements[ con_item ])
                    {
                        sample_mp->elements[ con_item ][ dis_item ] =
                            log_pair_probs_mp->elements[ con_item ][ dis_item ];
                    }
                }
            }
        }

        result = sample_log_matrix(sample_mp, &sampled_prob, &con_item, &dis_item);

        if (result == ERROR) break;

         /* dbe(sampled_prob);  */

          /* dbi(con_item);  */
          /* dbi(dis_item);   */

        ASSERT(con_item < num_con_items);
        ASSERT(dis_item < num_dis_items);

        ASSERT(match_count < max_num_matches);

        (*match_mpp)->elements[ match_count ][ 0 ] = con_item;
        (*match_mpp)->elements[ match_count ][ 1 ] = dis_item;

        match_count++;

        dis_item_found_vp->elements[ dis_item ] = TRUE;
        con_item_found_vp->elements[ con_item ] = TRUE;

        for (i = 0; i < num_dis_items; i++)
        {
            sample_mp->elements[ con_item ][ i ] = DBL_MOST_NEGATIVE;
        }

        for (i = 0; i < num_con_items; i++)
        {
            sample_mp->elements[ i ][ dis_item ] = DBL_MOST_NEGATIVE;
        }

         /* dbi(match_count);  */
         /* db_mat(sample_mp);  */
         /* dbp("===========================================");  */

    }

    *num_matches_ptr = match_count;

    free_int_vector(con_item_found_vp);
    free_int_vector(dis_item_found_vp);

    free_matrix(sample_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int sample_log_matrix
(
    const Matrix* sample_mp,
    double*       prob_ptr,
    int*          row_ptr,
    int*          col_ptr
)
{
    int result = NO_ERROR;
    Vector* sample_vp = NULL;
    int num_rows = sample_mp->num_rows;
    int num_cols = sample_mp->num_cols;
    int i, j;
    int count = 0;
    int len = num_cols * num_rows;


    ERE(get_target_vector(&sample_vp, len));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            sample_vp->elements[ count ] = sample_mp->elements[ i ][ j ];
            count++;
        }
    }

    if ((count = sample_log_vector(sample_vp, prob_ptr)) == ERROR)
    {
        result = ERROR;
    }
    else
    {
        *row_ptr = count / num_cols ;
        *col_ptr = count % num_cols;
    }

    free_vector(sample_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int sample_log_vector
(
    const Vector* log_sample_vp,
    double*       prob_ptr
)
{
    Vector* sample_vp = NULL;
    Vector* cum_vp = NULL;
    int     len    = log_sample_vp->length;
    double  sum    = 0.0;
    int     elem   = NOT_SET;
    double  prob   = kjb_rand();
    int i;
    double max_log_prob = DBL_MOST_NEGATIVE;


    ERE(get_target_vector(&sample_vp, len));

    ASSERT_IS_NUMBER_DBL(max_log_prob);

    for (i = 0; i < len; i++)
    {
        max_log_prob = MAX_OF(max_log_prob,
                              log_sample_vp->elements[ i ]);
        ASSERT_IS_NUMBER_DBL(max_log_prob);

    }

    for (i = 0; i < len; i++)
    {
        if (log_sample_vp->elements[ i ] <= DBL_HALF_MOST_NEGATIVE / 2.0)
        {
            sample_vp->elements[ i ] = 0.0;
        }
        else
        {
            sample_vp->elements[ i ] = exp(log_sample_vp->elements[ i ] - max_log_prob);
        }
        ASSERT_IS_NUMBER_DBL(sample_vp->elements[ i ]);
    }

    verify_vector(sample_vp, NULL);

    ERE(scale_vector_by_sum_2(&cum_vp, sample_vp, NULL));

    for (i = 0; i < len; i++)
    {
        sum += cum_vp->elements[ i ];
        cum_vp->elements[ i ] = sum;
    }

    ASSERT(cum_vp->elements[ len - 1 ] > .9999);
    ASSERT(cum_vp->elements[ len - 1 ] < 1.0001);

    /* Deal with round off. */

    cum_vp->elements[ len - 1 ] = 1.0;

    for (i = 0; i < len; i++)
    {
        if (prob <= cum_vp->elements[ i ])
        {
            elem = i;
            break;
        }
    }

    if (elem == NOT_SET)
    {
        SET_CANT_HAPPEN_BUG();
        elem = ERROR;
    }
    else
    {
        *prob_ptr = sample_vp->elements[ elem ];
    }

    free_vector(cum_vp);
    free_vector(sample_vp);

    return elem;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_hungarian_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           num_nulls,
    int           duplicate_words_for_matching
)
{
    int         num_con_items      = log_pair_probs_mp->num_rows;
    int         num_dis_items      = log_pair_probs_mp->num_cols;
    int         i,j;
    int         result = NO_ERROR;
    double max_log_prob;
    Matrix* cost_mp = NULL;
    Int_vector* row_vp = NULL;
    double hungarian_cost;
    int max_i, max_j;
    int dis_dup_count = duplicate_words_for_matching ? MAX_OF(1, num_con_items / num_dis_items) : 1;
#ifdef TEST
    int dim = MAX_OF(num_con_items, num_dis_items);
    double hungarian_sum = 0.0;
    double greedy_sum = 0.0;
    Matrix* copy_log_pair_probs_mp = NULL;
    int count;
#endif
    int match_count = 0;
    int match;
    int base_m = num_con_items;
    int base_n = num_dis_items * dis_dup_count;
    int m = base_m;
    int n = base_n;


    if (num_nulls > 0)
    {
#ifdef HOW_IT_WAS_BEFORE_FALL_2003
        int num_implicit_nulls = MAX_OF(m, n) - MIN_OF(m, n);

#ifdef DEBUG_NULLS
        dbi(num_nulls);
        dbi(num_implicit_nulls);
#endif

        num_nulls = MAX_OF(0, num_nulls - num_implicit_nulls);
#endif

        m += num_nulls;
        n += num_nulls;
    }

    ERE(get_target_int_matrix(match_mpp, base_m, 2));


#ifdef TEST
    if (num_nulls <= 0)
    {
        ASSERT(num_dis_items * dis_dup_count <= dim);

        ERE(get_target_matrix(&copy_log_pair_probs_mp, num_con_items,
                              dis_dup_count * num_dis_items));

        for (count = 0; count < dis_dup_count; count++)
        {
            ERE(ow_copy_matrix_block(copy_log_pair_probs_mp, 0, count * num_dis_items,
                                     log_pair_probs_mp, 0, 0,
                                     num_con_items, num_dis_items));
        }

        for (count = 0; count < MIN_OF(num_con_items, dis_dup_count * num_dis_items); count++)
        {
            get_max_matrix_element(copy_log_pair_probs_mp,
                                   &max_log_prob, &max_i, &max_j);

            greedy_sum += max_log_prob;

            for (j = 0; j < dis_dup_count * num_dis_items; j++)
            {
                copy_log_pair_probs_mp->elements[ max_i ][ j ] = DBL_MOST_NEGATIVE;
            }

            for (i = 0; i < num_con_items; i++)
            {
                copy_log_pair_probs_mp->elements[ i ][ max_j ] = DBL_MOST_NEGATIVE;
            }
        }
    }
#endif

    if (result != ERROR)
    {
        get_max_matrix_element(log_pair_probs_mp,
                               &max_log_prob, &max_i, &max_j);
    }

    if (result != ERROR)
    {
        result = get_zero_matrix(&cost_mp, m, n);
    }

    if (result != ERROR)
    {
        double max_cost = DBL_MIN;

        for (i = 0; i < base_m; i++)
        {
            for (j = 0; j < base_n; j++)
            {
                int jj = j % num_dis_items;

                cost_mp->elements[ i ][ j ] = max_log_prob - log_pair_probs_mp->elements[ i ][ jj ];
                ASSERT(cost_mp->elements[ i ][ j ] >= 0.0);

                if (num_nulls > 0)
                {
                    max_cost = MAX_OF(max_cost, cost_mp->elements[ i ][ j ]);
                }
            }
        }

        if (num_nulls > 0)
        {
            for (i = 0; i < base_m; i++)
            {
                for (j = 0; j < base_n; j++)
                {
                    cost_mp->elements[ i ][ j ] += max_cost;
                }
            }
        }
    }


    if (result != ERROR)
    {
        if (hungarian(cost_mp, &row_vp, &hungarian_cost) == ERROR)
        {
#ifdef TEST
            dbp("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
            dbi(m);
            dbi(n);
            db_irv(row_vp);
            db_mat(cost_mp);
#endif
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
#ifdef DEBUG_NULLS
        dbi(num_con_items);
        dbi(num_dis_items);
        dbi(num_nulls);
        dbi(m);
        dbi(n);
        dbi(base_m);
        dbi(base_n);
        db_irv(row_vp);
#endif


        for (i = 0; i < base_m; i++)
        {
            match = row_vp->elements[ i ];

            if ((match >= 0) && (match < base_n))
            {
                match %= num_dis_items;

                (*match_mpp)->elements[ match_count ][ 0 ] = i;
                (*match_mpp)->elements[ match_count ][ 1 ] = match;
                match_count++;

#ifdef DEBUG_NULLS
                dbi(i);
                dbi(match);
#endif

#ifdef TEST
                /* This is the maximum log likelihood (not cost!) */
                hungarian_sum += log_pair_probs_mp->elements[ i ][ match];
#endif
            }
        }

#ifdef TEST
        if ((match_count > num_dis_items) || (match_count > num_con_items))
        {
            dbi(m);
            dbi(n);
            dbi(num_dis_items);
            dbi(num_con_items);
            dbi(cost_mp->num_rows);
            dbi(cost_mp->num_cols);
            dbi(base_m);
            dbi(base_n);
            dbi(match_count);
            db_irv(row_vp);
        }
#endif

        ASSERT_IS_NOT_GREATER_INT(match_count, base_m);
        ASSERT_IS_NOT_GREATER_INT(match_count, base_n);


#ifdef DEBUG_NULLS
        dbi(match_count);
        dbp("-----------------");
#endif
        ASSERT(match_count > 0);
    }

    if (result != ERROR)
    {
#ifdef TEST
        if (    (num_nulls <= 0)
             && ((hungarian_sum - greedy_sum) / (ABS_OF(hungarian_sum) + ABS_OF(greedy_sum)) < -0.0001)
           )
        {
            TEST_PSO(("Hungarian graph match cost is less than greedy method.\n"));
            TEST_PSO(("Relative difference is %.4e.\n",
                      (hungarian_sum - greedy_sum) / (ABS_OF(hungarian_sum) + ABS_OF(greedy_sum))));
        }
#endif
    }

    free_matrix(cost_mp);
    free_int_vector(row_vp);
#ifdef TEST
    free_matrix(copy_log_pair_probs_mp);
#endif

    if (result != ERROR)
    {
        *num_matches_ptr = match_count;
        SET_MAT_ROWS((*match_mpp), match_count);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

