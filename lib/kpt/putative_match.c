/*
 * $Id: putative_match.c 20654 2016-05-05 23:13:43Z kobus $
 */

#include "kpt/putative_match.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 *                          get_putative_matches
 *
 * Extracts putatively matched keypoints for each image.
 *
 * This routine finds a nearest neighbor for each keypoint of target_kvp based on the
 * strength_threshold. If the redundance_threshold is not = 0, then the routine
 * removes the redundant keypoint matches.
 *
 * Input:
    const Keypoint_vector *target_kvp - target keypoints for which to find matches
    const Keypoint_vector *candidate_kvp - candidate keypoints
    const double strength_threshold - the distance ratio for the NN keypoint
                                      descriptor match 
    const double redundance_threshold - the similarity threshold
    int *num_putative_matches - the total count of the matches.
    Int_matrix  **match_idx_impp - a 2-column matrix to store the indices of the matches:
                                   i.e. element[0][0] is target_kvp index, which matched
                                   element[0][1] index from candidate_kvp vector;
                                   can be set to NULL if not interested in the values.
 * 
 * Returns: an error code or a count of putative matches.
 *
 * Related: get_keypoint_match, remove_redundant_matches
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
int get_putative_matches
(
    const Keypoint_vector *target_kvp,
    const Keypoint_vector *candidate_kvp,
    const double          strength_threshold,
    const double          redundance_threshold,
    int                   *num_putative_matches,
    Int_matrix            **match_idx_impp
)
{
    int result          = NO_ERROR;
    int num_kpts        = 0; 
    int num_candidates  = 0; 
    int i;
    int count = 0;

    verbose_pso(7, " | Getting putative matches...\n");

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if (num_putative_matches == NULL)
    {
        warn_pso("Arg-bug (%s +%d): The return parameter (num_putative_matches) is set to NULL,." __FILE__, __LINE__);
        SET_ARGUMENT_BUG();
    }
    else
    {
        verbose_pso(8, " | Setting num_putative_matches to 0.\n");
        *num_putative_matches = 0;
    }

    num_kpts        = target_kvp->length; 
    num_candidates  = candidate_kvp->length; 
    verbose_pso(7, " | Matching %d keypoints to %d candidates...\n", num_kpts, num_candidates);

    if (match_idx_impp == NULL) 
    {
        /*  Indices are not needed. */
        verbose_pso(11, "match_idx_impp was set to NULL.\n");
    }
    else /*(if (match_idx_impp != NULL) */
    {
        /* Create a matrix to hold the indices of putative matches */
        result = get_initialized_int_matrix( match_idx_impp, num_kpts, 2, NOT_SET );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): Unable to create a matrix of keypoint match indices.", __FILE__, __LINE__);
            return result;
        }
    }
  

    verbose_pso(7, " | Begin keypoint match (dist_ratio = %f)...\n", strength_threshold);
    for (i = 0; i < num_kpts; i++)
    {
        verbose_pso(9, "Matching target keypoint %d...\n", i);
        result = get_keypoint_match( target_kvp->elements[i],
                                     candidate_kvp,
                                     strength_threshold );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): An error has occurred during keypoint matching.", __FILE__, __LINE__);
            return result;
        }
        else if (result == NOT_FOUND)
        {
            result = NO_ERROR; /* Keypoint isn't matched - keep going. */
        }
        else if (result >= 0)
        {
            verbose_pso(11, " %d <-> %d", i, result);
            if (match_idx_impp != NULL) 
            {
                (*match_idx_impp)->elements[i][0] = i;
                (*match_idx_impp)->elements[i][1] = result;
                verbose_pso(9, " %d <-> %d\n", (*match_idx_impp)->elements[i][0], (*match_idx_impp)->elements[i][1]);
            }
            count++;
        }
        else
        {
            warn_pso("ERROR (%s +%d): %s() returned an unexpected result value (%d).",
                     __FILE__, __LINE__, __FUNCTION__, result);
            SET_ARGUMENT_BUG();
        }
    }

    verbose_pso(7, " | Finished keypoint match...\n");



    if ((match_idx_impp != NULL) && ( kjb_get_verbose_level() > 7))
    {
        /* DEBUGGING */    
        dbi_mat(*match_idx_impp);
    }

    if (redundance_threshold != 0)
    {
        warn_pso("The functionality to remove the redundant matches is not implemented yet.\n");
    }

    /* Return the number of matches. */
    if (result != ERROR)
    {
        if (count > 0)
        {
            *num_putative_matches = count;
            verbose_pso(9, " | Found %d matches...\n", *num_putative_matches);
        }
        else
        {
            verbose_pso(9, " | Found NO matches...\n");
            result = NOT_FOUND;
        }
    }
   
    return result;
}

/* =============================================================================
 *                          get_putatively_matched_keypoints_2
 *
 * Extracts symmetric putatively matched keypoints for each image.
 *
 * This routine finds a nearest neighbor for each keypoint of target_kvp based on the
 * strength_threshold. It then does the same for the candidate_kvp looking for its
 * matches in target_kvp.
 * If the redundance_threshold is not = 0, then the routine
 * removes the redundant keypoint matches.
 *
 * Input:
    const Keypoint_vector *target_kvp - target keypoints for which to find matches
    const Keypoint_vector *candidate_kvp - candidate keypoints
    const double strength_threshold - the distance ratio for the NN keypoint
                                      descriptor match 
    const double redundance_threshold - the similarity threshold
    int *num_putative_matches - the total count of the matches.
    Int_matrix  **match_idx_impp - a 2-column matrix to store the indices of the matches;
                                   can be set to NULL if not interested in the values.
 * 
 * Returns: an error code or a count of putative matches.
 *
 * Related: get_keypoint_match, remove_redundant_matches
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
int get_putative_matches_2
(
    const Keypoint_vector *target_kvp,
    const Keypoint_vector *candidate_kvp,
    const double          strength_threshold,
    const double          redundance_threshold,
    int                   *num_putative_matches,
    Int_matrix            **match_idx_impp
)
{
    int result          = NO_ERROR;
    int num_kpts        = 0; 
    int num_candidates  = 0; 
    int i;
    int count = 0;

    verbose_pso(7, " | Getting putative matches...\n");

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if (num_putative_matches == NULL)
    {
        warn_pso("%s +%d (%s())", __FILE__, __LINE__, __FUNCTION__);
        warn_pso("The return parameter (num_putative_matches) is set to NULL.\n");
        SET_ARGUMENT_BUG();
    }
    else
    {
        verbose_pso(8, " | Setting num_putative_matches to 0.\n");
        *num_putative_matches = 0;
    }

    if (match_idx_impp == NULL) 
    {
        /*  Indices are not needed. */
        verbose_pso(11, "match_idx_impp was set to NULL.\n");
    }

    num_kpts        = target_kvp->length; 
    num_candidates  = candidate_kvp->length; 
    verbose_pso(7, " | Matching %d keypoints to %d candidates & vice versa...\n", num_kpts, num_candidates);

    if (match_idx_impp != NULL) 
    {
        /* Create a matrix to hold the putative matches */
        result = get_initialized_int_matrix( match_idx_impp, (num_kpts+num_candidates), 2, NOT_SET );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): Unable to create a matrix of keypoint match indices.", __FILE__, __LINE__);
            return result;
        }
    }
  

    verbose_pso(7, " | Begin keypoint match (dist_ratio = %f)...\n", strength_threshold);
    for (i = 0; i < num_kpts; i++)
    {
        verbose_pso(9, "Matching target keypoint %d...\n", i);
        result = get_keypoint_match( target_kvp->elements[i],
                                     candidate_kvp,
                                     strength_threshold );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): An error has occurred during keypoint matching.", __FILE__, __LINE__);
            return result;
        }
        else if (result == NOT_FOUND)
        {
            result = NO_ERROR; /* Keypoint isn't matched - keep going. */
        }
        else if (result >= 0)
        {
            verbose_pso(11, " %d <-> %d", i, result);
            if (match_idx_impp != NULL) 
            {
                (*match_idx_impp)->elements[i][0] = i;
                (*match_idx_impp)->elements[i][1] = result;
                verbose_pso(9, " %d <-> %d\n", (*match_idx_impp)->elements[i][0], (*match_idx_impp)->elements[i][1]);
            }
            count++;
        }
        else
        {
            warn_pso("ERROR (%s +%d): %s() returned an unexpected result value (%d).",
                     __FILE__, __LINE__, __FUNCTION__, result);
            SET_ARGUMENT_BUG();
        }
    }

    verbose_pso(7, " | Begin the reverse keypoint match (dist_ratio = %f)...\n", strength_threshold);
    for (i = 0; i < num_candidates; i++)
    {
        verbose_pso(9, "Matching candidate keypoint %d...\n", i);
        result = get_keypoint_match( candidate_kvp->elements[i],
                                     target_kvp,
                                     strength_threshold );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): An error has occurred during keypoint matching.", __FILE__, __LINE__);
            return result;
        }
        else if (result == NOT_FOUND)
        {
            result = NO_ERROR; /* Keypoint isn't matched - keep going. */
        }
        else if (result >= 0)
        {
            verbose_pso(11, " %d <-> %d", i, result);
            if (match_idx_impp != NULL) 
            {
                (*match_idx_impp)->elements[ num_kpts+i ][0] = result; /* target kpt */
                (*match_idx_impp)->elements[ num_kpts+i ][1] = i;
                verbose_pso(9, " %d <-> %d\n", (*match_idx_impp)->elements[ num_kpts + i ][0], (*match_idx_impp)->elements[ num_kpts + i ][1]);
            }
            count++;
        }
        else
        {
            warn_pso("ERROR (%s +%d): %s() returned an unexpected result value (%d).",
                     __FILE__, __LINE__, __FUNCTION__, result);
            SET_ARGUMENT_BUG();
        }
    }
    
    verbose_pso(7, " | Finished putative keypoint match...\n");



    if ((match_idx_impp != NULL) && ( kjb_get_verbose_level() > 7))
    {
        /* DEBUGGING */    
        dbi_mat(*match_idx_impp);
    }

    if (redundance_threshold != 0)
    {
        warn_pso("The functionality to remove the redundant matches is not implemented yet.\n");
    }

    /* Return the number of matches. */
    if (result != ERROR)
    {
        if (count > 0)
        {
            *num_putative_matches = count;
            verbose_pso(9, " | Found %d matches...\n", *num_putative_matches);
        }
        else
        {
            verbose_pso(9, " | Found NO matches...\n");
            result = NOT_FOUND;
        }
    }
   
    return result;
}

/* =============================================================================
 *                          get_putatively_matched_keypoints_3
 *
 * Extracts putatively matched keypoints using the location threshold.
 *
 * This routine looks for a nearest neighbor for each keypoint of target_kvp first 
 * based on the location_threshold, which limits how far away in space kpts can be.
 * It then uses the strength_threshold to compare the feature vectors. 
 * If the redundance_threshold is not = 0, then the routine
 * removes the redundant keypoint matches.
 *
 * Input:
    const Keypoint_vector *target_kvp - target keypoints for which to find matches
    const Keypoint_vector *candidate_kvp - candidate keypoints
    const double strength_threshold - the distance ratio for the NN keypoint
                                      descriptor match 
    const double location_threshold - the radius within which to look for matches
    int *num_putative_matches - the total count of matches. can be NULL, if not needed.
    Int_matrix  **match_idx_impp - a 2-column matrix to store the indices of the matches;
                                   can be set to NULL if not interested in the values.
                                   The number of rows is equal to the number of keypoints
                                   in the target_kvp. 
                                   Unmatched keypoints are marked as NOT_SET.
 * 
 * Returns: an error code.
 *
 * Related: get_keypoint_match, remove_redundant_matches
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
int get_putative_matches_3
(
    const Keypoint_vector *target_kvp,
    const Keypoint_vector *candidate_kvp,
    const double          strength_threshold,
    const double          location_threshold,
    int                   *num_putative_matches,
    Int_matrix            **match_idx_impp
)
{
    int result          = NO_ERROR;
    int num_kpts        = 0; 
    int num_candidates  = 0; 
    int i;
    int count = 0;

    verbose_pso(7, " | Getting putative matches based on the location threshold...\n");

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        /*add_error("ERROR (%s +%d) Either target_kvp or candidate_kvp was set to NULL.\n",
                    __FILE__, __LINE__ );*/
        warn_pso("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.\n",
                    __FILE__, __LINE__ );

        return NO_ERROR;
    }

    /*
    if (num_putative_matches == NULL)
    {
        warn_pso("%s +%d (%s())", __FILE__, __LINE__, __FUNCTION__);
        warn_pso("The return parameter (num_putative_matches) for %s is set to NULL.\n",
                    __FUNCTION__);
        SET_ARGUMENT_BUG();
    }
    else
    */
    if (num_putative_matches != NULL)
    {
        verbose_pso(8, " | Setting num_putative_matches to 0.\n");
        *num_putative_matches = 0;
    }

    num_kpts        = target_kvp->length; 
    num_candidates  = candidate_kvp->length; 
    verbose_pso(7, " | Matching %d keypoints to %d candidates ...\n", num_kpts, num_candidates);

    if (match_idx_impp == NULL) 
    {
        /*  Indices are not needed. */
        /*verbose_pso(11, "match_idx_impp was set to NULL.\n");*/
        warn_pso("match_idx_impp was set to NULL.\n");
    }
    else
    {
        /* Create a matrix to hold the putative matches */
        /*if (symmetric matching) then
         * result = get_initialized_int_matrix( match_idx_impp, (num_kpts+num_candidates), 2, NOT_SET );*/
        result = get_initialized_int_matrix( match_idx_impp, num_kpts, 2, NOT_SET );
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): Failed to create a matrix of keypoint match indices.", __FILE__, __LINE__);
            return result;
        }
    }

    verbose_pso(7, " | Begin keypoint match (location_threshold = %f, strength_threshold = %f)...\n", location_threshold, strength_threshold);
    for (i = 0; i < num_kpts; i++)
    {
        verbose_pso(9, "Matching target keypoint %d...\n", i);
        /*result = get_keypoint_match( target_kvp->elements[i],
                                     candidate_kvp,
                                     strength_threshold );
                                     */
        result = get_constrained_keypoint_match( target_kvp->elements[i],
                                     candidate_kvp,
                                     strength_threshold, 
                                     location_threshold);
        if (result == ERROR)
        {
            add_error("ERROR (%s +%d): An error has occurred during keypoint matching.", __FILE__, __LINE__);
            return result;
        }
        else if (result == NOT_FOUND)
        {
            result = NO_ERROR; /* Keypoint isn't matched - keep going. */
        }
        else if (result >= 0)
        {
            verbose_pso(11, " %d <-> %d", i, result);
            if (match_idx_impp != NULL) 
            {
                (*match_idx_impp)->elements[i][0] = i;
                (*match_idx_impp)->elements[i][1] = result;
                verbose_pso(11, " %d <-> %d\n", (*match_idx_impp)->elements[i][0], (*match_idx_impp)->elements[i][1]);
            }
            count++;
        }
        else
        {
            warn_pso("ERROR (%s +%d): %s() returned an unexpected result value (%d).", __FILE__, __LINE__, __FUNCTION__, result);
            SET_ARGUMENT_BUG();
        }
    }

    
    verbose_pso(7, " | Finished putative keypoint match...\n");

    if ((match_idx_impp != NULL) && ( kjb_get_verbose_level() > 7))
    {
        /* DEBUGGING */    
        dbi_mat(*match_idx_impp);
    }

    /* Return the number of matches. */
    if (result != ERROR)
    {
        if (count > 0)
        {
            if (num_putative_matches != NULL)
            {
                *num_putative_matches = count;
            }
                
            verbose_pso(9, " | Found %d matches...\n", count);
            /*result = count;*/
        }
        else
        {
            verbose_pso(9, " | Found NO matches...\n");
            result = NOT_FOUND;
        }
    }
   
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *               extract_match_positions
 *
 * The function extracts the (x,y) coordinates of the VLFeat keypoints
 * based on the values stored in the mask_mp. If mask_mp[i] is < 0, then the
 * value in the keypoint vector at position i will not be stored into the 
 * pos_mpp matrix. In the resulting matrix, the keypoint location of each element
 * of the target_pos_mp matches the corresponding element's location of the 
 * candidate_pos_mp, i.e. target_pos_mp[0] is matched to candidate_pos_mp[0], etc.
 *
 * Input parameters: 
 *  Matrix**   target_pos_mpp - stores the position of the target image keypoints 
 *  Matrix**   candidate_pos_mpp - stores the position of the candidate matches' keypoints 
 *  Keypoint_vector *target_kvp - contains target image keypoints and descriptors
 *  Keypoint_vector *candidate_kvp - contains matched keypoints and descriptors
 *  Int_matrix   *mask_imp - a 2-column matrix that stores the indices of the matched 
 *                           Keypoint_vector elements (first column - target indices,
 *                           second column - corresponding candidate indices) that need 
 *                           to be copied. If the index value == NOT_SET, then the 
 *                           keypoint is ignored. 
 *  int num_matches - the number of elements in the mask_imp matrix whose value is >=0.
 *                    If num_matches is set to something that's less than 0, then mask_imp 
 *                    is used to calculate the value internally.
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * ---------------------------------------------------------------------------*/
int extract_match_positions
(
    Matrix                    **target_pos_mpp,
    Matrix                    **candidate_pos_mpp,
    const Keypoint_vector     *target_kvp,
    const Keypoint_vector     *candidate_kvp,
    const Int_matrix          *mask_imp,
    const int                 num_matches
)
{
    int result = NO_ERROR;
    int i;
    int num_elements = 0 ;
    int num_cols = 2; /* how many columns to create in the resulting matrix */
    int target_idx = 0; /* the index value extracted from the mask */
    int candidate_idx = 0;
    int count = 0; /* an index that's used to fill in the *_pos_mpp */
    int n_matches = 0;

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.",
                    __FILE__, __LINE__ );

        return ERROR;
    }

    if (target_pos_mpp == NULL || candidate_pos_mpp == NULL) 
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.",
                    __FILE__, __LINE__ );

        return ERROR;
    }

    if (mask_imp == NULL)
    {
        /* TODO: Call extract_keypoints_positions()? */
        return result;
    }

  
    num_elements = mask_imp->num_rows ; 
    verbose_pso(8, " Number of elements in the mask matrix = %d.\n", num_elements);
    verbose_pso(8, " target_kvp->length = %d, ", target_kvp->length);
    verbose_pso(8, " candidate_kvp->length = %d\n", candidate_kvp->length);

    if (target_kvp->length != num_elements)
    {
        add_error("ERROR (%s +%d): The number of elements in the mask do not match \
                    the elements in the target keypoint vector.", __FILE__, __LINE__ );
    }

    if (num_matches < 0)
    {
        verbose_pso(8, " Computing the number of matches for %d elements.\n", num_elements);

        for (i = 0; i < num_elements; i++)
        {
            if (mask_imp->elements[i][0] >= 0)
            {
                n_matches++;
            }
        }
        verbose_pso(8, " Number of matches whose positions to copy = %d.\n", n_matches);
        /* 2 columns: for x-coordinates and for y-coordinates */
        ERE( get_target_matrix( target_pos_mpp, n_matches, num_cols ));
        ERE( get_target_matrix( candidate_pos_mpp, n_matches, num_cols ));
    }  
    else
    {
        verbose_pso(8, " Number of matches whose positions to copy = %d.\n", num_matches);
        /* 2 columns: for x-coordinates and for y-coordinates */
        ERE( get_target_matrix( target_pos_mpp, num_matches, num_cols ));
        ERE( get_target_matrix( candidate_pos_mpp, num_matches, num_cols ));

        n_matches = num_matches;
    }


    for (i=0; i < num_elements; i++)
    {

        target_idx = mask_imp->elements[i][0];
        candidate_idx = mask_imp->elements[i][1];
        verbose_pso(9, " target_idx = %d, candidate_idx = %d\n", target_idx, candidate_idx);

        if ((target_idx >= 0) && (candidate_idx >= 0))
        {
            if ((target_idx <= target_kvp->length) && (candidate_idx <= candidate_kvp->length))
            {
                if (count >= n_matches)
                {
                    warn_pso(" ERROR (%s +%d): Exceeded the number of matches in the mask matrix.",__FILE__, __LINE__);
                    warn_pso(" count = %d, n_matches = %d, num_elements = %d \n", count, n_matches, num_elements );
                    SET_ARGUMENT_BUG();
                    return ERROR;
                }  
                
                (*target_pos_mpp)->elements[ count ][0] = target_kvp->elements[target_idx]->row;
                (*target_pos_mpp)->elements[ count ][1] = target_kvp->elements[target_idx]->col;

                (*candidate_pos_mpp)->elements[ count ][0] = candidate_kvp->elements[candidate_idx]->row;
                (*candidate_pos_mpp)->elements[ count ][1] = candidate_kvp->elements[candidate_idx]->col;
                count++;
            }
            else
            {
                warn_pso(" ERROR (%s +%d): Invalid index in the mask matrix.",__FILE__, __LINE__);
                warn_pso(" target_idx = %d, target_kvp->length = %d", target_idx, target_kvp->length);
                warn_pso(" candidate_idx = %d, candidate_kvp->length = %d", candidate_idx, candidate_kvp->length);
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    verbose_pso(8, " Finished copying the matrices.\n");

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               extract_match_positions_1
 *
 * The function extracts the (x,y) coordinates of the VLFeat keypoints
 * based on the values stored in the mask_mp. If mask_mp[i] is < 0, then the
 * value in the keypoint vector at position i will not be stored into the 
 * pos_mpp matrix. In the resulting matrix, the keypoint location of each element
 * of the target_pos_mp matches the corresponding element's location of the 
 * candidate_pos_mp, i.e. target_pos_mp[0] is matched to candidate_pos_mp[0], etc.
 *
 * Input parameters: 
 *  Matrix**   target_pos_mpp - stores the position of the target image keypoints 
 *  Matrix**   candidate_pos_mpp - stores the position of the candidate matches' keypoints 
 *  Keypoint_vector *target_kvp - contains target image keypoints and descriptors
 *  Keypoint_vector *candidate_kvp - contains matched keypoints and descriptors
 *  Int_matrix   *mask_imp - a 2-column matrix that stores the indices of the matched 
 *                           Keypoint_vector elements (first column - target indices,
 *                           second column - corresponding candidate indices) that need 
 *                           to be copied. If the index value == NOT_SET, then the 
 *                           keypoint is ignored. 
 *  int num_matches - the number of elements in the mask_imp matrix whose value is >=0.
 *                    If num_matches is set to something that's less than 0, then mask_imp 
 *                    is used to calculate the value internally.
 *  Int_matrix  *kpt_idx_imp - keeps track of the indices to which keypoints in _pos_mp belong.
                            Very similar to mask_imp, except there are no NOT_SET values.
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * ---------------------------------------------------------------------------*/
int extract_match_positions_1
(
    Matrix                    **target_pos_mpp,
    Matrix                    **candidate_pos_mpp,
    const Keypoint_vector     *target_kvp,
    const Keypoint_vector     *candidate_kvp,
    const Int_matrix          *mask_imp,
    const int                 num_matches,
    Int_matrix                **kpt_idx_impp
)
{
    int result = NO_ERROR;
    int i;
    int num_elements = 0 ;
    int num_cols = 2; /* how many columns to create in the resulting matrix */
    int target_idx = 0; /* the index value extracted from the mask */
    int candidate_idx = 0;

    ERE( extract_match_positions( target_pos_mpp, candidate_pos_mpp, target_kvp, candidate_kvp,
                                      mask_imp, num_matches )) ;
       
    verbose_pso(8, " Begin creating keypoint correspondence.\n");
    num_elements = mask_imp->num_rows ; 
    ERE( get_target_int_matrix( kpt_idx_impp, num_elements, num_cols ));

    /* Create the keypoint match correspondence matrix */
    for (i=0; i < num_elements; i++)
    {
        target_idx = mask_imp->elements[i][0];
        candidate_idx = mask_imp->elements[i][1];
        verbose_pso(9, "i = %d, target_idx = %d, candidate_idx = %d\n", i, target_idx, candidate_idx);

        if (target_idx >= 0) 
        {
            (*kpt_idx_impp)->elements[i][0] = target_idx;
            (*kpt_idx_impp)->elements[i][1] = candidate_idx;
        }
    }

    verbose_pso(8, " Finished creating keypoint correspondence.\n");

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               extract_match_positions_2
 *
 * The function extracts the (x,y) coordinates of the VLFeat keypoints
 * based on the values stored in the mask_ivp. If mask_ivp[i] is < 0, then the
 * value in the keypoint vector at position i will not be stored into the 
 * pos_mpp matrix. In the resulting matrices, the keypoint location of each element
 * of the target_pos_mp matches the corresponding element's location of the 
 * candidate_pos_mp, i.e. target_pos_mp[0] is matched to candidate_pos_mp[0], etc.
 *
 * Input parameters: 
 *  Matrix**   target_pos_mpp - stores the position of the target image keypoints 
 *  Matrix**   candidate_pos_mpp - stores the position of the candidate matches' keypoints 
 *  Keypoint_vector *target_kvp - contains target image keypoints and descriptors
 *  Keypoint_vector *candidate_kvp - contains matched keypoints and descriptors
 *  Int_vector *mask_ivp - a vector that stores the indices of the matched 
 *                           Keypoint_vector elements (the indices of elements - target indices,
 *                           elements themselves - corresponding candidate indices) that need 
 *                           to be copied. If the value == NOT_SET, then the 
 *                           keypoint is ignored. 
 *  int num_matches - the number of elements in the mask_ivp matrix whose value is >=0.
 *                    If num_matches is set to something that's less than 0, then mask_ivp 
 *                    is used to calculate the value internally.
    Int_vector **kpt_to_inlier_ivpp - keeps track of the mapping from target_kvp keypoints to 
                                     the indices of the candidate_kvp's resulting inliers
    Int_vector **inlier_to_kpt_ivpp - keeps track of the mapping from inliers to the
                                     original keypoints of target_kvp
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * ---------------------------------------------------------------------------*/
int extract_match_positions_2
(
    Matrix                    **target_pos_mpp,
    Matrix                    **candidate_pos_mpp,
    const Keypoint_vector     *target_kvp,
    const Keypoint_vector     *candidate_kvp,
    const Int_vector          *mask_ivp,
    const int                 num_matches,
    Int_vector                **kpt_to_inlier_ivpp,
    Int_vector                **inlier_to_kpt_ivpp
)
{
    int result = NO_ERROR;
    int i;
    int num_elements = 0 ;
    int num_cols = 2; /* how many columns to create in the resulting matrix */
    int target_idx = 0; /* the index value extracted from the mask */
    int candidate_idx = 0;
    int count = 0; /* an index that's used to fill in the *_pos_mpp */
    int n_matches = 0;
    Int_vector *kpt_to_inlier_ivp = NULL;
    Int_vector *inlier_to_kpt_ivp = NULL;

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.",
                    __FILE__, __LINE__ );

        return ERROR;
    }

    if (target_pos_mpp == NULL || candidate_pos_mpp == NULL) 
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.",
                    __FILE__, __LINE__ );

        return ERROR;
    }

    if (mask_ivp == NULL)
    {
        /* TODO: Call extract_keypoints_positions()? */
        return result;
    }

  
    num_elements = mask_ivp->length ; 
    verbose_pso(8, " Number of elements in the mask = %d.\n", num_elements);
    verbose_pso(8, " target_kvp->length = %d, ", target_kvp->length);
    verbose_pso(8, " candidate_kvp->length = %d\n", candidate_kvp->length);

    if (target_kvp->length != num_elements)
    {
        add_error("ERROR (%s +%d): The number of elements in the mask do not match \
                    the elements in the target keypoint vector.", __FILE__, __LINE__ );
    }

    if (num_matches < 0)
    {
        verbose_pso(8, " Computing the number of matches for %d elements.\n", num_elements);

        for (i = 0; i < num_elements; i++)
        {
            if (mask_ivp->elements[i] >= 0)
            {
                n_matches++;
            }
        }
        verbose_pso(8, " Number of matches whose positions to copy = %d.\n", n_matches);
        /* 2 columns: for x-coordinates and for y-coordinates */
        ERE( get_target_matrix( target_pos_mpp, n_matches, num_cols ));
        ERE( get_target_matrix( candidate_pos_mpp, n_matches, num_cols ));
    }  
    else
    {
        verbose_pso(8, " Number of matches whose positions to copy = %d.\n", num_matches);
        /* 2 columns: for x-coordinates and for y-coordinates */
        ERE( get_target_matrix( target_pos_mpp, num_matches, num_cols ));
        ERE( get_target_matrix( candidate_pos_mpp, num_matches, num_cols ));

        n_matches = num_matches;
    }

    if (kpt_to_inlier_ivpp != NULL)
    {
        result = get_initialized_int_vector( kpt_to_inlier_ivpp, num_elements, NOT_SET );
        if (result == ERROR) { EGC(result); }        
        kpt_to_inlier_ivp = *kpt_to_inlier_ivpp;
    }
    result = get_initialized_int_vector( &kpt_to_inlier_ivp, num_elements, NOT_SET );
    if (result == ERROR) { EGC(result); }        

    if (inlier_to_kpt_ivpp != NULL)
    {
        result = get_initialized_int_vector( inlier_to_kpt_ivpp, n_matches, NOT_SET );
        if (result == ERROR) { EGC(result); }        
        inlier_to_kpt_ivp = *inlier_to_kpt_ivpp;
    }
    result = get_initialized_int_vector( &inlier_to_kpt_ivp, n_matches, NOT_SET );
    if (result == ERROR) { EGC(result); }

    for (i=0; i < num_elements; i++)
    {
        target_idx = i;
        candidate_idx = mask_ivp->elements[i];
        verbose_pso(9, " target_idx = %d, candidate_idx = %d\n", target_idx, candidate_idx);

        if (candidate_idx >= 0)
        {
            if ((target_idx <= target_kvp->length) && (candidate_idx <= candidate_kvp->length))
            {
                if (count >= n_matches)
                {
                    warn_pso(" ERROR (%s +%d): Exceeded the number of matches in the mask matrix.",__FILE__, __LINE__);
                    warn_pso(" count = %d, n_matches = %d, num_elements = %d \n", count, n_matches, num_elements );
                    SET_ARGUMENT_BUG();
                    return ERROR;
                }  
                
                (*target_pos_mpp)->elements[ count ][0] = target_kvp->elements[target_idx]->row;
                (*target_pos_mpp)->elements[ count ][1] = target_kvp->elements[target_idx]->col;

                (*candidate_pos_mpp)->elements[ count ][0] = candidate_kvp->elements[candidate_idx]->row;
                (*candidate_pos_mpp)->elements[ count ][1] = candidate_kvp->elements[candidate_idx]->col;

                /* Keep track of index mapping */
                kpt_to_inlier_ivp->elements[i] = count;
                inlier_to_kpt_ivp->elements[count] = i;

                count++;
            }
            else
            {
                warn_pso(" ERROR (%s +%d): Invalid index in the mask vector.\n",__FILE__, __LINE__);
                warn_pso(" target_idx = %d, target_kvp->length = %d\n", target_idx, target_kvp->length);
                warn_pso(" candidate_idx = %d, candidate_kvp->length = %d", candidate_idx, candidate_kvp->length);
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    verbose_pso(8, " Finished copying the matrices.\n");
cleanup:
    EPE(result);
    if (result == ERROR || kpt_to_inlier_ivpp == NULL)
    {
        verbose_pso(8, " Freeing kpt_to_inlier_ivp\n");
        free_int_vector(kpt_to_inlier_ivp);
    }
    if (result == ERROR || inlier_to_kpt_ivpp == NULL)
    {
        verbose_pso(8, " Freeing inlier_to_kpt_ivp\n");
        free_int_vector(inlier_to_kpt_ivp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                       get_constrained_keypoint_match
 *
 * Finds the closest match (in the descriptor space as well as based on the Euclidean 
 * distance of the keypoint locations) to the given keypoint in a vector of candidate 
 * keypoints.
 *
 * This routine finds the two nearest neighbors of the given keypoint. 
 * The similarity is measured by the Euclidean distance of the keypoint vectors
 * as well as their location.
 * Based on D. Lowe's "Distinctive image features from scale-invariant keypoints"
 * paper, the closest match is selected based on the ratio of the distances to the
 * nearest and the second nearest neighbor.
 *
 * Input:
 *    const Keypoint        *target_kpt - the keypoint for which to find a match
 *    const Keypoint_vector *candidate_kvp - a vector of possible matches
 *    const int             dist_ratio - a ratio of the descriptor similarity between 
 *                                       2 nearest keypoint matches
 *    const int             dist_radius - a radius around the keypoint location within
 *                                        which to look for matches.
 *
 * Returns:
 *    On error, this routine returns ERROR.
 *    If the keypoint pointers are NULL, the routine returns NOT_SET.
 *    If a match was not found then the routine returns NOT_FOUND, otherwise,
 *    On success it returns the index of the keypoint matched to the target.
 *
 * Index: match, keypoints
 *
 * -----------------------------------------------------------------------------
*/ 
int get_constrained_keypoint_match 
(
    const Keypoint        *target_kpt,
    const Keypoint_vector *candidate_kvp,
    const double          dist_ratio,
    const double          dist_radius
)
{
    int result       = NOT_FOUND;
    double dist_sq_1 = DBL_MAX;
    double dist_sq_2 = DBL_MAX;
    double dist_sq   = 0;
    int num_candidates     = 0;
    int i;
    Vector *descriptor = NULL; /* temporary holder for the match descriptors */
    int match_index  = NOT_FOUND;

    float target_x = 0;
    float target_y = 0;
    float candidate_x = 0;
    float candidate_y = 0;

    if ((target_kpt == NULL) || (candidate_kvp == NULL))
    {
        free_vector( descriptor );
        return NOT_SET;
    }

    num_candidates = candidate_kvp->length;

    target_x = target_kpt->row;
    target_y = target_kpt->col;

    for (i = 0; i < num_candidates; i++)
    {
        candidate_x = candidate_kvp->elements[i]->row;
        candidate_y = candidate_kvp->elements[i]->col;

        if ( (fabs(target_x - candidate_x) <= dist_radius) 
             && (fabs(target_y - candidate_y) <= dist_radius) )
        {
            copy_vector( &descriptor, candidate_kvp->elements[i]->descrip);

            result = keypoint_euclidean_distance( target_kpt->descrip,
                                                  descriptor, 
                                                  &dist_sq );
            if (result != ERROR)
            {
                if (dist_sq < dist_sq_1) /* && (keypoint_dot_product() >= 0 */
                {
                    dist_sq_2 = dist_sq_1;
                    dist_sq_1 = dist_sq;
                    match_index = i; 
                }
                else if (dist_sq < dist_sq_2)
                {
                    dist_sq_2 = dist_sq;
                }
            }
            else
            {
                /* TODO: Change this to return an error? */
                warn_pso("%s +%d (%s())", __FILE__, __LINE__, __FUNCTION__);
                warn_pso(" An error has occurred while computing keypoint Euclidean distance.\n");
                break;
            }
        } /* end if <= dist_radius */
                                              
    } /* end for i < num_candidates */


    if (result != ERROR)
    {
        if ( dist_sq_1 < (dist_ratio * dist_sq_2))
        {
            result = match_index;
        }
        else
        {
            result = NOT_FOUND;
        }
    }

    free_vector( descriptor );
    return result;
}


/* =============================================================================
 *               extract_match_positions
 *
 * The function extracts the (x,y) coordinates of the VLFeat keypoints
 * based on the values stored in the mask_mp. If mask_mp[i] is < 0, then the
 * value in the keypoint vector at position i will not be stored into the 
 * pos_mpp matrix. In the resulting matrix, the keypoint location of each element
 * of the target_pos_mp matches the corresponding element's location of the 
 * candidate_pos_mp, i.e. target_pos_mp[0] is matched to candidate_pos_mp[0], etc.
 *
 * Input parameters: 
 *  Matrix**   target_pos_mpp - stores the position of the target image keypoints 
 *  Matrix**   candidate_pos_mpp - stores the position of the candidate matches' keypoints 
 *  Keypoint_vector *target_kvp - contains target image keypoints and descriptors
 *  Keypoint_vector *candidate_kvp - contains matched keypoints and descriptors
 *  Int_matrix   *mask_imp - a 2-column matrix that stores the indices of the matched 
 *                           Keypoint_vector elements (first column - target indices,
 *                           second column - corresponding candidate indices) that need 
 *                           to be copied. If the index value == NOT_SET, then the 
 *                           keypoint is ignored. 
 *  int num_matches - the number of elements in the mask_imp matrix whose value is >=0.
 *                    If num_matches is set to something that's less than 0, then mask_imp 
 *                    is used to calculate the value internally.
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * ---------------------------------------------------------------------------*/
int print_keypoint_parameters
(
    const Keypoint_vector     *target_kvp,
    const Keypoint_vector     *candidate_kvp,
    const Int_matrix          *mask_imp,
    const int                 num_matches
)
{
    int result = NO_ERROR;
    int i;
    int num_elements = 0 ;
    int num_cols = 2; /* how many columns to create in the resulting matrix */
    int target_idx = 0; /* the index value extracted from the mask */
    int candidate_idx = 0;
    int count = 0; /* an index that's used to fill in the *_pos_mpp */
    int n_matches = 0;
    float diff = 0;
    float abs_diff = 0;

    if (target_kvp == NULL || candidate_kvp == NULL)
    {
        add_error("ERROR (%s +%d): Either target_kvp or candidate_kvp was set to NULL.",
                    __FILE__, __LINE__ );

        return ERROR;
    }

 
    if (mask_imp == NULL)
    {
        /* TODO: Call extract_keypoints_positions()? */
        return result;
    }

    pso(" PRINT PARAMETERS \n");
  
    num_elements = mask_imp->num_rows ; 
    verbose_pso(8, " Number of elements in the mask matrix = %d.\n", num_elements);
    verbose_pso(8, " target_kvp->length = %d, ", target_kvp->length);
    verbose_pso(8, " candidate_kvp->length = %d\n", candidate_kvp->length);

    if (target_kvp->length != num_elements)
    {
        add_error("ERROR (%s +%d): The number of elements in the mask do not match \
                    the elements in the target keypoint vector.", __FILE__, __LINE__ );
    }


    for (i=0; i < num_elements; i++)
    {
        target_idx = mask_imp->elements[i][0];
        candidate_idx = mask_imp->elements[i][1];
        verbose_pso(9, " target_idx = %d, candidate_idx = %d\n", target_idx, candidate_idx);

        if ((target_idx >= 0) && (candidate_idx >= 0))
        {
            if ((target_idx <= target_kvp->length) && (candidate_idx <= candidate_kvp->length))
            {
                       
                diff = target_kvp->elements[target_idx]->ori - candidate_kvp->elements[candidate_idx]->ori;
                abs_diff = fabs(diff);
                pso("O: %f %f (%f)\n", target_kvp->elements[target_idx]->ori, candidate_kvp->elements[candidate_idx]->ori, abs_diff);
                diff = target_kvp->elements[target_idx]->scale - candidate_kvp->elements[candidate_idx]->scale;
                abs_diff = fabs(diff);
                pso("S: %f %f (%f)\n", target_kvp->elements[target_idx]->scale, candidate_kvp->elements[candidate_idx]->scale, abs_diff);
            }
            else
            {
                warn_pso(" ERROR (%s +%d): Invalid index in the mask matrix.",__FILE__, __LINE__);
                warn_pso(" target_idx = %d, target_kvp->length = %d", target_idx, target_kvp->length);
                warn_pso(" candidate_idx = %d, candidate_kvp->length = %d", candidate_idx, candidate_kvp->length);
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    verbose_pso(8, " Finished printing the parameteres.\n");

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
