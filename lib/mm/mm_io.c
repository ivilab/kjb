
/* $Id: mm_io.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and  UA. Currently
|  the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#include "mm/mm_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "mm/mm_cluster_lib.h"
#include "mm/mm_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define CON_FEATURE_WEIGHT_FILE         "con_feature_weights"
#define CON_FEATURE_GROUP_FILE          "con_feature_group"
#define CON_FEATURE_ENABLE_FILE         "con_feature_enable"

#define NUM_LEVELS_PER_CATEGORY               "num_levels_per_category"
#define NUM_LEVELS_TO_SKIP_FILE               "num_levels_to_skip"
#define LAST_LEVEL_FILE                       "last_level"
#define FAN_OUT_FILE                          "fan_out_vp"
#define CLUSTER_PRIORS_FILE             "a_vp"
#define CLUSTER_AVERAGE_VERTICAL_WEIGHT_FILE    "V_mp"
#define AVERAGE_VERTICAL_WEIGHT_FILE    "V_vp"
#define AVERAGE_DIS_VERTICAL_WEIGHT_FILE    "con_V_mp"
#define AVERAGE_CON_VERTICAL_WEIGHT_FILE    "dis_V_mp"
#define DOCUMENT_CLUSTER_FILE           "P_mp"
#define DOCUMENT_FIT_FILE               "fit_vp"
#define CLUSTER_DEPENDENT_VERTICAL_WEIGHT_FILE "P_l_p_c_mvp"
#define VERTICAL_WEIGHT_FILE            "P_l_p_mp"
#define WORD_PROBABILITIES_FILE         "W_mvp"

#define CLUSTER_MEAN_FILE               "mean_mvp"
#define CLUSTER_VARIANCE_FILE           "var_mvp"
#define CLUSTER_LOG_SQRT_DET_FILE       "log_sqrt_det_vvp"

#define MEAN_FILE                       "mean_mp"
#define VARIANCE_FILE                   "var_mp"
#define LOG_SQRT_DET_FILE               "log_sqrt_det_vp"

#define MAX_CON_NORM_VAR_FILE           "max_con_norm_var"
#define CON_LOG_NORM_FILE               "con_log_norm"
#define CON_SCORE_CUTOFF_FILE           "con_score_cutoff"
#define MIN_CON_LOG_PROB_FILE           "min_con_log_prob"
#define MODEL_OPTION_FILE               "model_options"
#define RAW_LOG_LIKELIHOOD_FILE         "raw_ll"
#define MDL_LOG_LIKELIHOOD_FILE         "mdl_ll"
#define NORM_RAW_LOG_LIKELIHOOD_FILE    "norm_raw_ll"
#define NORM_MDL_LOG_LIKELIHOOD_FILE    "norm_mdl_ll"
#define AIC_FILE                        "aic"
#define NUM_PARAMETERS_FILE             "num_parameters"
#define EXTRA_DIS_PROB_FILE             "extra_dis_prob"
#define EXTRA_CON_PROB_FILE             "extra_con_prob"

#define IGNORE_DIS_PROB_FRACTION_OPTION "ignore_dis_prob_fraction"
#define IGNORE_CON_PROB_FRACTION_OPTION "ignore_con_prob_fraction"

#define DIS_PROB_BOOST_FACTOR_OPTION    "dis_prob_boost_factor"
#define CON_PROB_BOOST_FACTOR_OPTION    "con_prob_boost_factor"

#define DISCRETE_TIE_COUNT              "discrete_tie_count"
#define WEIGHT_CON_ITEMS_OPTION         "weight_con_items"
#define NORM_DEPEND_OPTION              "norm_depend"
#define NORM_ITEMS_OPTION               "norm_items"
#define NORM_CONTINUOUS_FEATURES_OPTION "norm_continuous_features"
#define NORM_ITEMS_OPTION               "norm_items"
#define NORM_CONTINUOUS_FEATURES_OPTION "norm_continuous_features"
#define VAR_OFFSET_OPTION               "var_offset"
#define PHASE_ONE_NUM_ITERATIONS_OPTION "phase_one_num_iterations"
#define PHASE_TWO_NUM_ITERATIONS_OPTION "phase_two_num_iterations"
#define PHASE_ONE_UNIFORM_VERTICAL_DIST_OPTION           "phase_one_uniform_vertical_dist"
#define PHASE_TWO_UNIFORM_VERTICAL_DIST_OPTION           "phase_two_uniform_vertical_dist"
#define PHASE_ONE_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION "phase_one_cluster_dependent_vertical_dist"
#define PHASE_TWO_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION "phase_two_cluster_dependent_vertical_dist"
#define PHASE_ONE_SAMPLE_MATCHES_OPTION         "phase_one_sample_matches"
#define PHASE_ONE_MODEL_CORRESPONDENCE_OPTION   "phase_one_model_correspondence"
#define PHASE_ONE_SHARE_MATCHES_OPTION          "phase_one_share_matches"
#define PHASE_ONE_LAST_SAMPLE_OPTION            "phase_one_last_sample"
#define PHASE_TWO_SAMPLE_MATCHES_OPTION         "phase_two_sample_matches"
#define PHASE_TWO_MODEL_CORRESPONDENCE_OPTION   "phase_two_model_correspondence"
#define PHASE_TWO_SHARE_MATCHES_OPTION          "phase_two_share_matches"
#define PHASE_TWO_LAST_SAMPLE_OPTION            "phase_two_last_sample"

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

static int fs_output_full_model = FALSE;
static int fs_output_model_raw = TRUE;

/* -------------------------------------------------------------------------- */

static int read_multi_modal_model_2(Multi_modal_model** model_pp, const char* model_dir);

/* -------------------------------------------------------------------------- */

int set_mmm_io_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result         = NOT_FOUND;
    int  temp_boolean_value;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "mm-output-full-model")
          || match_pattern(lc_option, "output-full-model")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("All multi-modal model information %s written.\n",
                    fs_output_full_model ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("output-full-model = %s\n",
                    fs_output_full_model ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_output_full_model = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "mm-output-model-using-raw")
          || match_pattern(lc_option, "mm-output-model-raw")
          || match_pattern(lc_option, "output-model-using-raw")
          || match_pattern(lc_option, "output-model-raw")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Multi-modal models %s output in raw format.\n",
                    fs_output_model_raw ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("output-model-using-raw = %s\n",
                    fs_output_model_raw ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_output_model_raw = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_multi_modal_model(Multi_modal_model** model_pp, const char* model_dir)
{
    int result;
    const char* action = (model_pp == NULL) ? "check" : "read";
    const char* cap_action = (model_pp == NULL) ? "Check" : "Read";


    verbose_pso(2, "Attempting to %s multi-modal model from %s\n",
                action,model_dir);

    result = read_multi_modal_model_2(model_pp, model_dir);

    if (result == NO_ERROR)
    {
        verbose_pso(2, "%s of model from %s SUCEEDED\n",
                    cap_action, model_dir);
    }
    else
    {
        verbose_pso(2, "%s of model from %s FAILED\n",
                    cap_action, model_dir);
    }

    return result;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_multi_modal_model_2
(
    Multi_modal_model** model_pp,
    const char*         model_dir
)
{
    char                file_name[ MAX_FILE_NAME_SIZE ];
    Int_vector*         temp_level_parameter_vp = NULL;
    int                 first_level = 0;
    int                 last_level_num;
    int                 num_levels;
    Multi_modal_model* model_ptr;
    int                 i;
    int result = NO_ERROR;
    FILE* model_option_fp;
    Int_vector* fan_out_vp;


    /*
     * Read signficant options that were in place for training. Some of these
     * will effect the future use of the model in tasks like annotation. Some of
     * the options affect the weighting between regions and words in training,
     * and thus will have no direct effect in annotation (the model itself may
     * be different, which may indirectly affect the results).
    */
    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, MODEL_OPTION_FILE));
    NRE(model_option_fp = kjb_fopen(file_name, "r"));

    /*
     * Request for a dry run. Checks that there is a model directory.
    */
    if (model_pp == NULL)
    {
        (void)kjb_fclose(model_option_fp);
        return NO_ERROR;
    }

    free_multi_modal_model(*model_pp);
    NRE(model_ptr = allocate_multi_modal_model());
    *model_pp = model_ptr;

    /*CONSTCOND*/
    while (TRUE)
    {
        char   line[ 1000 ];
        int    num_options;
        char** option_list;
        char** value_list;
        char** option_list_pos;
        char** value_list_pos;
        int    read_res        = BUFF_GET_REAL_LINE(model_option_fp, line);
        int    temp_boolean_value;

        if (read_res == ERROR)
        {
            result = ERROR;
            break;
        }
        else if (read_res == EOF)
        {
            break;
        }

        num_options = parse_options(line, &option_list, &value_list);

        if (num_options == ERROR)
        {
            result = ERROR;
            break;
        }

        option_list_pos = option_list;
        value_list_pos  = value_list;

        for (i=0; i<num_options; i++)
        {
            if (IC_STRCMP_EQ(*option_list_pos,
                             WEIGHT_CON_ITEMS_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->weight_con_items = temp_boolean_value;
                }

                dbi(model_ptr->weight_con_items);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  DISCRETE_TIE_COUNT))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->discrete_tie_count)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->discrete_tie_count);
            }
            else if (IC_STRCMP_EQ(*option_list_pos, NORM_DEPEND_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->norm_depend = temp_boolean_value;
                }

                dbi(model_ptr->norm_depend);
            }
            else if (IC_STRCMP_EQ(*option_list_pos, IGNORE_DIS_PROB_FRACTION_OPTION))
            {
                if (ss1d(*value_list_pos,
                         &(model_ptr->ignore_dis_prob_fraction)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->ignore_dis_prob_fraction);
            }
            else if (IC_STRCMP_EQ(*option_list_pos, IGNORE_CON_PROB_FRACTION_OPTION))
            {
                if (ss1d(*value_list_pos,
                         &(model_ptr->ignore_con_prob_fraction)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->ignore_con_prob_fraction);
            }
            else if (IC_STRCMP_EQ(*option_list_pos, DIS_PROB_BOOST_FACTOR_OPTION))
            {
                if (ss1d(*value_list_pos,
                         &(model_ptr->dis_prob_boost_factor)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->dis_prob_boost_factor);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  CON_PROB_BOOST_FACTOR_OPTION))
            {
                if (ss1d(*value_list_pos,
                         &(model_ptr->con_prob_boost_factor)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->con_prob_boost_factor);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  NORM_CONTINUOUS_FEATURES_OPTION))
            {
                if (ss1pi(*value_list_pos,
                          &(model_ptr->norm_continuous_features)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->norm_continuous_features);
            }
            else if (IC_STRCMP_EQ(*option_list_pos, NORM_ITEMS_OPTION))
            {
                if (ss1pi(*value_list_pos,
                          &(model_ptr->norm_items)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->norm_items);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_SAMPLE_MATCHES_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_one_sample_matches = temp_boolean_value;
                }

                dbi(model_ptr->phase_one_sample_matches);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_SAMPLE_MATCHES_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_two_sample_matches = temp_boolean_value;
                }

                dbi(model_ptr->phase_two_sample_matches);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_SHARE_MATCHES_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_one_share_matches = temp_boolean_value;
                }

                dbi(model_ptr->phase_one_share_matches);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_SHARE_MATCHES_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_two_share_matches = temp_boolean_value;
                }

                dbi(model_ptr->phase_two_share_matches);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_LAST_SAMPLE_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_one_last_sample)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_one_last_sample);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_LAST_SAMPLE_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_two_last_sample)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_two_last_sample);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_MODEL_CORRESPONDENCE_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_one_model_correspondence)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_one_model_correspondence);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_MODEL_CORRESPONDENCE_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_two_model_correspondence)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_two_model_correspondence);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                 VAR_OFFSET_OPTION ))
            {
                if (ss1snd(*value_list_pos,
                           &(model_ptr->var_offset)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->var_offset);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_NUM_ITERATIONS_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_one_num_iterations)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_one_num_iterations);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_UNIFORM_VERTICAL_DIST_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_one_uniform_vertical_dist = temp_boolean_value;
                }

                dbi(model_ptr->phase_one_uniform_vertical_dist);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_ONE_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_one_cluster_dependent_vertical_dist = temp_boolean_value;
                }

                dbi(model_ptr->phase_one_cluster_dependent_vertical_dist);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_NUM_ITERATIONS_OPTION))
            {
                if (ss1i(*value_list_pos,
                         &(model_ptr->phase_two_num_iterations)) == ERROR)
                {
                    result = ERROR;
                    break;
                }
                dbi(model_ptr->phase_two_num_iterations);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_UNIFORM_VERTICAL_DIST_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_two_uniform_vertical_dist = temp_boolean_value;
                }

                dbi(model_ptr->phase_two_uniform_vertical_dist);
            }
            else if (IC_STRCMP_EQ(*option_list_pos,
                                  PHASE_TWO_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION))
            {
                temp_boolean_value = get_boolean_value(*value_list_pos);

                if (temp_boolean_value == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else
                {
                    model_ptr->phase_two_cluster_dependent_vertical_dist = temp_boolean_value;
                }

                dbi(model_ptr->phase_two_cluster_dependent_vertical_dist);
            }
            else
            {
#ifdef NOT_TRANSITION
                set_error("%q is an invalid option.\n", *option_list_pos);
                result = ERROR;
                break;
#else
                warn_pso("%q is an invalid option.\n", *option_list_pos);
#endif
            }

            value_list_pos++;
            option_list_pos++;
        }

        free_options(num_options, option_list, value_list);

        if (result == ERROR) break;
    }

    /* Only reading, so close errors are not interesting.*/
    push_error_action(FORCE_ADD_ERROR_ON_ERROR); 
    (void)kjb_fclose(model_option_fp);
    pop_error_action(); 

    if (result == ERROR)
    {
        return ERROR;
    }

    /*
    // In the user world we call the first_level, num_levels_to_skip, because
    // we want to count first_level from 0, and rather than keep track of a
    // user notion of level and a program notion of level (or force the user
    // notion of level to start at 0), we change the conceptial understanding
    // of what is counted.
    */
    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, NUM_LEVELS_TO_SKIP_FILE));

    if (read_int_vector(&temp_level_parameter_vp, file_name) != ERROR)
    {
        if (temp_level_parameter_vp->length != 1)
        {
            warn_pso("File %q should only contain one integer. Ignoring rest.\n",
                     file_name);
        }

        first_level = temp_level_parameter_vp->elements[ 0 ];
    }

    if (first_level < 0)
    {
        warn_pso("The first level read from %s is negative.\n",
                 file_name);
        warn_pso("Using 0 instead.\n");
    }

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, FAN_OUT_FILE));

    ERE(read_int_vector(&(model_ptr->topology_ptr->fan_out_vp), file_name));
    fan_out_vp = model_ptr->topology_ptr->fan_out_vp;

    num_levels = model_ptr->topology_ptr->fan_out_vp->length + 1;

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, LAST_LEVEL_FILE));

    if ( ! is_file(file_name))
    {
        last_level_num = num_levels;
    }
    else
    {
        ERE(read_int_vector(&temp_level_parameter_vp, file_name));

        if (temp_level_parameter_vp->length != 1)
        {
            warn_pso("File %q should only contain one integer. Ignoring rest.\n",
                     file_name);
        }
        last_level_num = temp_level_parameter_vp->elements[ 0 ];
    }

    if (last_level_num > num_levels)
    {
        warn_pso("The last level read from %s is more than the number of levels.\n",
                 file_name);
        warn_pso("Using the number of levels (%d) instead.\n",
                 num_levels);

        last_level_num = num_levels;
    }

    free_int_vector(temp_level_parameter_vp);

    if (first_level > last_level_num - 1)
    {
        warn_pso("The number of levels implied by %q is %d.\n",
                 file_name, num_levels);
        warn_pso("But the number of levels to skip is %d.\n",
                 first_level);
        warn_pso("Forcing the number of levels to skip to be %d.\n",
                 last_level_num - 1);
        first_level = last_level_num - 1;
    }

    ERE(get_topology_2(&(model_ptr->topology_ptr),
                       fan_out_vp, first_level, last_level_num));
    model_ptr->num_clusters = model_ptr->topology_ptr->num_clusters;

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, CLUSTER_PRIORS_FILE));
    (void)read_vector(&(model_ptr->a_vp), file_name);

    ASSERT_IS_EQUAL_INT(model_ptr->num_clusters,
                        model_ptr->a_vp->length);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, AVERAGE_VERTICAL_WEIGHT_FILE));
    read_vector(&(model_ptr->V_vp), file_name);

    /*
     * For now, we either read the non-cluster oriented data, or the cluster oriented
     * data, but not both.
    */
    if (model_ptr->V_vp == NULL)
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, CLUSTER_AVERAGE_VERTICAL_WEIGHT_FILE));
        ERE(read_matrix(&(model_ptr->V_mp), file_name));
    }
    else
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, AVERAGE_CON_VERTICAL_WEIGHT_FILE));
        (void)read_matrix(&(model_ptr->con_V_mp), file_name);

        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, AVERAGE_DIS_VERTICAL_WEIGHT_FILE));
        (void)read_matrix(&(model_ptr->dis_V_mp), file_name);

        ERE(get_target_matrix(&(model_ptr->V_mp), num_levels, 1));

        for (i = 0; i < num_levels; i++)
        {
            model_ptr->V_mp->elements[ i ][ 0 ] = model_ptr->V_vp->elements[ i ];
        }
    }

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, DOCUMENT_CLUSTER_FILE));
    /* OK if file is not there--computing some things do not require it. */
    (void)read_matrix(&(model_ptr->P_c_p_mp), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, DOCUMENT_FIT_FILE));
    (void)read_vector(&(model_ptr->fit_vp), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR,
                    CLUSTER_DEPENDENT_VERTICAL_WEIGHT_FILE));
    /* OK if file is not there--model_ptr->P_l_p_c_mvp can be NULL. */
    (void)read_matrix_vector(&(model_ptr->P_l_p_c_mvp), file_name);
    dbx(model_ptr->P_l_p_c_mvp);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, VERTICAL_WEIGHT_FILE));
    /* OK if file is not there--model_ptr->P_l_p_c_mvp can be NULL. */
    (void)read_matrix(&(model_ptr->P_l_p_mp), file_name);
    dbx(model_ptr->P_l_p_mp);

    dbi(model_ptr->topology_ptr->num_levels_per_category);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, NUM_LEVELS_PER_CATEGORY));
    (void)read_int_from_file(&(model_ptr->topology_ptr->num_levels_per_category), 
                             file_name);

    if (model_ptr->topology_ptr->num_levels_per_category > 0)
    {
        int level;
        int num_levels_per_category = model_ptr->topology_ptr->num_levels_per_category;
        int num_categories = num_levels / num_levels_per_category;

        model_ptr->num_categories = num_categories;

        ERE(get_target_matrix_vector(&(model_ptr->P_i_n_mvp), num_levels));

        for (level = 0; level < num_levels; level++)
        {
            ERE(get_zero_matrix(&(model_ptr->P_i_n_mvp->elements[ level ]), 1, num_categories));
            model_ptr->P_i_n_mvp->elements[ level ]->elements[ 0 ][ level / num_levels_per_category ] = 1.0;
        }

    }
    else
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, WORD_PROBABILITIES_FILE));
        /* OK if file is not there--model_ptr->P_i_n_mvp can be NULL. */
        (void)read_matrix_vector(&(model_ptr->P_i_n_mvp), file_name);

        if (model_ptr->P_i_n_mvp != NULL)
        {
            model_ptr->num_categories = model_ptr->P_i_n_mvp->elements[ first_level ]->num_cols;
            dbi(model_ptr->num_categories);
        }
    }

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, MEAN_FILE));
    /* OK if file is not there--model_ptr->mean_mvp can be NULL. */
    (void)read_matrix(&(model_ptr->mean_mp), file_name);

    /* If we have means, then we need the variances, and friends. */
    if (model_ptr->mean_mp != NULL)
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, VARIANCE_FILE));
        ERE(read_matrix(&(model_ptr->var_mp), file_name));

        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, LOG_SQRT_DET_FILE));
        ERE(read_vector(&(model_ptr->con_log_sqrt_det_vp), file_name));

        ERE(get_target_matrix_vector(&(model_ptr->mean_mvp), model_ptr->mean_mp->num_rows));
        ERE(get_target_matrix_vector(&(model_ptr->var_mvp), model_ptr->var_mp->num_rows));
        ERE(get_target_vector_vector(&(model_ptr->con_log_sqrt_det_vvp), model_ptr->var_mp->num_rows));

        for (i = 0; i < model_ptr->mean_mp->num_rows; i++)
        {
            int j;

            ERE(get_target_matrix(&(model_ptr->mean_mvp->elements[ i ]), 1, model_ptr->mean_mp->num_cols));
            ERE(get_target_matrix(&(model_ptr->var_mvp->elements[ i ]), 1, model_ptr->var_mp->num_cols));
            ERE(get_target_vector(&(model_ptr->con_log_sqrt_det_vvp->elements[ i ]), 1));

            model_ptr->con_log_sqrt_det_vvp->elements[ i ]->elements[ 0 ] = model_ptr->con_log_sqrt_det_vp->elements[ i ];

            for (j = 0; j < model_ptr->mean_mp->num_cols; j++)
            {
                model_ptr->mean_mvp->elements[ i ]->elements[ 0 ][ j ] = model_ptr->mean_mp->elements[ i ][ j ];
                model_ptr->var_mvp->elements[ i ]->elements[ 0 ][ j ] = model_ptr->var_mp->elements[ i ][ j ];
            }
        }

        model_ptr->num_con_features = model_ptr->mean_mp->num_cols; 
    }

    /*
     * For now, we either read the non-cluster oriented data, or the cluster oriented
     * data, but not both.
    */
    if (model_ptr->mean_mp == NULL)
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, CLUSTER_MEAN_FILE));
        /* OK if file is not there--model_ptr->mean_mvp can be NULL. */
        (void)read_matrix_vector(&(model_ptr->mean_mvp), file_name);

        /* If we have means, then we need the variances, and friends. */
        if (model_ptr->mean_mvp != NULL)
        {
            ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                            model_dir, DIR_STR, CLUSTER_VARIANCE_FILE));
            ERE(read_matrix_vector(&(model_ptr->var_mvp), file_name));

            ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                            model_dir, DIR_STR, CLUSTER_LOG_SQRT_DET_FILE));
            ERE(read_vector_vector(&(model_ptr->con_log_sqrt_det_vvp), file_name));

            for (i = 0; i < model_ptr->mean_mvp->length; i++)
            {
                const Matrix* sub_mean_mp = model_ptr->mean_mvp->elements[ i ];

                if (sub_mean_mp != NULL)
                {
                    model_ptr->num_con_features = sub_mean_mp->num_cols; 
                    break;
                }
            }
        }

    }

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, CON_FEATURE_ENABLE_FILE));
    (void)read_int_vector(&(model_ptr->con_feature_enable_vp), file_name);

    if (model_ptr->num_con_features > 0)
    {
        if (model_ptr->con_feature_enable_vp == NULL)
        {
            ERE(get_unity_int_vector(&(model_ptr->con_feature_enable_vp),
                                     model_ptr->num_con_features));
        }
        else if (model_ptr->con_feature_enable_vp->length != model_ptr->num_con_features)
        {
            set_error("Mismatch in the number of continous features."); 
            add_error("The model matrices suggest %d whereas %s suggests %d.",
                      model_ptr->num_con_features, file_name, 
                      model_ptr->con_feature_enable_vp->length);
            return ERROR;
        }
    }

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, CON_FEATURE_GROUP_FILE));
    (void)read_int_vector(&(model_ptr->con_feature_group_vp), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, CON_FEATURE_WEIGHT_FILE));
    (void)read_vector(&(model_ptr->con_feature_weight_vp), file_name);


    if ((model_ptr->mean_mp != NULL) || (model_ptr->mean_mvp != NULL))
    {
        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, CON_LOG_NORM_FILE));
        ERE(read_double(&(model_ptr->con_log_norm), file_name));

        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, CON_SCORE_CUTOFF_FILE));
        model_ptr->con_score_cutoff = DBL_MOST_NEGATIVE;
        EPE(read_double(&(model_ptr->con_score_cutoff), file_name));
        dbe(model_ptr->con_score_cutoff);

        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, MIN_CON_LOG_PROB_FILE));
        (void)read_double(&(model_ptr->min_con_log_prob), file_name);

        ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                        model_dir, DIR_STR, MAX_CON_NORM_VAR_FILE));
        EPE(read_double(&(model_ptr->max_con_norm_var), file_name));
        dbe(model_ptr->max_con_norm_var);
    }


    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, RAW_LOG_LIKELIHOOD_FILE));
    (void)read_double(&(model_ptr->raw_log_likelihood), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, MDL_LOG_LIKELIHOOD_FILE));
    (void)read_double(&(model_ptr->mdl_log_likelihood), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, AIC_FILE));
    (void)read_double(&(model_ptr->aic), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, EXTRA_DIS_PROB_FILE));
    (void)read_double(&(model_ptr->extra_dis_prob), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, EXTRA_CON_PROB_FILE));
    (void)read_double(&(model_ptr->extra_con_prob), file_name);

    ERE(kjb_sprintf(file_name, sizeof(file_name), "%s%s%s",
                    model_dir, DIR_STR, NUM_PARAMETERS_FILE));
    (void)read_int_from_file(&(model_ptr->num_parameters), file_name);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_multi_modal_model
(
    const Multi_modal_model* model_ptr,
    int                      num_points,
    const char*              output_dir
)
{
    const Int_vector*    fan_out_vp  = model_ptr->topology_ptr->fan_out_vp;
    const int            first_level = model_ptr->topology_ptr->first_level;
    const int            last_level_num = model_ptr->topology_ptr->last_level_num;
    const int            num_levels = model_ptr->topology_ptr->num_levels;
    /* num_levels = model_ptr->topology_ptr->fan_out_vp->length + 1; */
    const Matrix*        V_mp        = model_ptr->V_mp;
    const Vector*        V_vp        = model_ptr->V_vp;
    const Matrix_vector* W_mvp       = model_ptr->P_i_n_mvp;
    int                  result      = NO_ERROR;
    const Matrix*        P_mp        = model_ptr->P_c_p_mp;
    char                 option_file_name[ MAX_FILE_NAME_SIZE ];
    FILE*                option_fp;
    int (*output_matrix_fn)(const char*,const char*, const Matrix*) =
            fs_output_model_raw ? output_raw_matrix : output_matrix;
    int (*output_matrix_vector_fn)(const char*,const char*, const Matrix_vector*) =
            fs_output_model_raw ? output_raw_matrix_vector : output_matrix_vector;


    /*
     * Record signficant options that were in place for training. Some of these
     * will effect the future use of the model in tasks like annotation. Some of
     * the options affect the weighting between regions and words in training,
     * and thus will have no direct effect in annotation (the model itself may
     * be different, which may indirectly affect the results).
    */

    BUFF_CPY(option_file_name, output_dir);
    BUFF_CAT(option_file_name, DIR_STR);
    BUFF_CAT(option_file_name, MODEL_OPTION_FILE);
    NRE(option_fp = kjb_fopen(option_file_name, "w"));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    WEIGHT_CON_ITEMS_OPTION,
                    (model_ptr->weight_con_items) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    DISCRETE_TIE_COUNT,
                    model_ptr->discrete_tie_count));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    NORM_DEPEND_OPTION,
                    (model_ptr->norm_depend) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %.5f\n",
                    IGNORE_DIS_PROB_FRACTION_OPTION,
                    model_ptr->ignore_dis_prob_fraction));

    ERE(kjb_fprintf(option_fp, "%s = %.5f\n",
                    IGNORE_CON_PROB_FRACTION_OPTION,
                    model_ptr->ignore_con_prob_fraction));

    ERE(kjb_fprintf(option_fp, "%s = %.5f\n",
                    DIS_PROB_BOOST_FACTOR_OPTION,
                    model_ptr->dis_prob_boost_factor));

    ERE(kjb_fprintf(option_fp, "%s = %.5f\n",
                    CON_PROB_BOOST_FACTOR_OPTION,
                    model_ptr->con_prob_boost_factor));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    NORM_ITEMS_OPTION,
                    model_ptr->norm_items));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    NORM_CONTINUOUS_FEATURES_OPTION,
                    model_ptr->norm_continuous_features));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_ONE_SAMPLE_MATCHES_OPTION,
                    (model_ptr->phase_one_sample_matches) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_TWO_SAMPLE_MATCHES_OPTION,
                    (model_ptr->phase_two_sample_matches) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_ONE_MODEL_CORRESPONDENCE_OPTION,
                    model_ptr->phase_one_model_correspondence));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_TWO_MODEL_CORRESPONDENCE_OPTION,
                    model_ptr->phase_two_model_correspondence));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_ONE_SHARE_MATCHES_OPTION,
                    (model_ptr->phase_one_share_matches) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_TWO_SHARE_MATCHES_OPTION,
                    (model_ptr->phase_two_share_matches) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_ONE_LAST_SAMPLE_OPTION,
                    model_ptr->phase_one_last_sample));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_TWO_LAST_SAMPLE_OPTION,
                    model_ptr->phase_two_last_sample));

    ERE(kjb_fprintf(option_fp, "%s = %.5e\n",
                    VAR_OFFSET_OPTION,
                    model_ptr->var_offset));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_ONE_NUM_ITERATIONS_OPTION,
                    model_ptr->phase_one_num_iterations));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_ONE_UNIFORM_VERTICAL_DIST_OPTION,
                    (model_ptr->phase_one_uniform_vertical_dist) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_ONE_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION,
                    (model_ptr->phase_one_cluster_dependent_vertical_dist) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %d\n",
                    PHASE_TWO_NUM_ITERATIONS_OPTION,
                    model_ptr->phase_two_num_iterations));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_TWO_UNIFORM_VERTICAL_DIST_OPTION,
                    (model_ptr->phase_two_uniform_vertical_dist) ? "t" : "f"));

    ERE(kjb_fprintf(option_fp, "%s = %s\n",
                    PHASE_TWO_CLUSTER_DEPENDENT_VERTICAL_DIST_OPTION,
                    (model_ptr->phase_two_cluster_dependent_vertical_dist) ? "t" : "f"));

    ERE(kjb_fclose(option_fp));

    if (model_ptr->con_feature_enable_vp)
    {
        ERE(output_int_vector(output_dir, CON_FEATURE_ENABLE_FILE, 
                              model_ptr->con_feature_enable_vp));
    }

    if (model_ptr->con_feature_group_vp)
    {
        ERE(output_int_vector(output_dir, CON_FEATURE_GROUP_FILE,
                              model_ptr->con_feature_group_vp));
    }

    if (model_ptr->con_feature_weight_vp)
    {
        ERE(output_vector(output_dir, CON_FEATURE_WEIGHT_FILE, 
                          model_ptr->con_feature_weight_vp));
    }

    if (first_level > 0)
    {
        ERE(output_int(output_dir, NUM_LEVELS_TO_SKIP_FILE, first_level));
    }

    if (last_level_num < num_levels)
    {
        ERE(output_int(output_dir, LAST_LEVEL_FILE, last_level_num));
    }

    ERE(output_int_vector(output_dir, FAN_OUT_FILE, fan_out_vp));

    ERE(output_vector(output_dir, CLUSTER_PRIORS_FILE, model_ptr->a_vp));

    /*
     * If V_vp is left over from another model, we might be in trouble. Get rid
     * of it.
    */
    if ((V_mp != NULL) && (V_vp == NULL))
    {
        (void)kjb_unlink_2(output_dir, AVERAGE_VERTICAL_WEIGHT_FILE);
    }

    /*
     * If V_mp is left over from another model, we might be in trouble. Get rid
     * of it.
    */
    if ((V_mp == NULL) && (V_vp != NULL))
    {
        (void)kjb_unlink_2(output_dir, CLUSTER_AVERAGE_VERTICAL_WEIGHT_FILE);
    }

    ERE(output_matrix_fn(output_dir, CLUSTER_AVERAGE_VERTICAL_WEIGHT_FILE, V_mp));

    ERE(output_vector(output_dir, AVERAGE_VERTICAL_WEIGHT_FILE, V_vp));

    ERE(output_matrix_fn(output_dir, AVERAGE_CON_VERTICAL_WEIGHT_FILE,
                      model_ptr->con_V_mp));
    ERE(output_matrix_fn(output_dir, AVERAGE_DIS_VERTICAL_WEIGHT_FILE,
                      model_ptr->dis_V_mp));

    if (fs_output_full_model)
    {
        ERE(output_matrix_fn(output_dir, DOCUMENT_CLUSTER_FILE, P_mp));
        ERE(output_vector(output_dir, DOCUMENT_FIT_FILE, model_ptr->fit_vp));

        ERE(output_matrix_vector_fn(output_dir,
                                    CLUSTER_DEPENDENT_VERTICAL_WEIGHT_FILE,
                                    model_ptr->P_l_p_c_mvp));

        ERE(output_matrix_fn(output_dir, VERTICAL_WEIGHT_FILE,
                             model_ptr->P_l_p_mp));
    }

    if (model_ptr->topology_ptr->num_levels_per_category > 0)
    {
        (void)kjb_unlink_2(output_dir, WORD_PROBABILITIES_FILE);
        (void)kjb_unlink_2(output_dir, CLUSTER_MEAN_FILE);
        (void)kjb_unlink_2(output_dir, CLUSTER_VARIANCE_FILE);
        (void)kjb_unlink_2(output_dir, CLUSTER_LOG_SQRT_DET_FILE);

        ERE(output_int(output_dir, NUM_LEVELS_PER_CATEGORY,
                       model_ptr->topology_ptr->num_levels_per_category));
        ERE(output_matrix_fn(output_dir, MEAN_FILE, model_ptr->mean_mp));
        ERE(output_matrix_fn(output_dir, VARIANCE_FILE, model_ptr->var_mp));
        ERE(output_vector(output_dir, LOG_SQRT_DET_FILE,
                          model_ptr->con_log_sqrt_det_vp));
    }
    else
    {
        (void)kjb_unlink_2(output_dir, NUM_LEVELS_PER_CATEGORY);
        (void)kjb_unlink_2(output_dir, MEAN_FILE);
        (void)kjb_unlink_2(output_dir, VARIANCE_FILE);
        (void)kjb_unlink_2(output_dir, LOG_SQRT_DET_FILE);

        ERE(output_matrix_vector_fn(output_dir, WORD_PROBABILITIES_FILE, W_mvp));
        ERE(output_matrix_vector_fn(output_dir, CLUSTER_MEAN_FILE, model_ptr->mean_mvp));
        ERE(output_matrix_vector_fn(output_dir, CLUSTER_VARIANCE_FILE, model_ptr->var_mvp));
        ERE(output_vector_vector(output_dir, CLUSTER_LOG_SQRT_DET_FILE,
                                 model_ptr->con_log_sqrt_det_vvp));
    }

    ERE(output_double(output_dir, CON_LOG_NORM_FILE,
                      model_ptr->con_log_norm));

    ERE(output_double(output_dir, MAX_CON_NORM_VAR_FILE,
                      model_ptr->max_con_norm_var));

    ERE(output_double(output_dir, CON_SCORE_CUTOFF_FILE,
                      model_ptr->con_score_cutoff));

    ERE(output_double(output_dir, MIN_CON_LOG_PROB_FILE,
                      model_ptr->min_con_log_prob));

    ERE(output_double(output_dir, RAW_LOG_LIKELIHOOD_FILE,
                      model_ptr->raw_log_likelihood));

    ERE(output_double(output_dir, MDL_LOG_LIKELIHOOD_FILE,
                      model_ptr->mdl_log_likelihood));

    ERE(output_double(output_dir, NORM_RAW_LOG_LIKELIHOOD_FILE,
                      model_ptr->raw_log_likelihood / num_points));

    ERE(output_double(output_dir, NORM_MDL_LOG_LIKELIHOOD_FILE,
                      model_ptr->mdl_log_likelihood / num_points));

    ERE(output_double(output_dir, AIC_FILE, model_ptr->aic));

    ERE(output_double(output_dir, EXTRA_DIS_PROB_FILE, model_ptr->extra_dis_prob));
    ERE(output_double(output_dir, EXTRA_CON_PROB_FILE, model_ptr->extra_con_prob));

    ERE(output_int(output_dir, NUM_PARAMETERS_FILE,
                   model_ptr->num_parameters));

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

