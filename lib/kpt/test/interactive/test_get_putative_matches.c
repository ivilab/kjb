#include "kpt/kpt_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int result = EXIT_SUCCESS;
    Keypoint_vector* kpt1_kvp = NULL;
    Keypoint_vector* kpt2_kvp = NULL;
    int num_putative_matches = NOT_SET;
    Int_matrix *match_idx_imp = NULL;
    const char *kpt1_filename = "data/frame_vl_key/000001.vl";
    const char *kpt2_filename = "data/frame_vl_key/000003.vl";


    kjb_init(); 

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    pso("Reading keypoints from '%s'\n", kpt1_filename);
    result = read_vl_keypoint_vector_from_file(kpt1_filename , &kpt1_kvp);
    if (result == ERROR) { EGC(result); }
    pso("Read %d keypoints\n", kpt1_kvp->length);

    pso("\nReading keypoints from '%s'\n", kpt2_filename);
    result = read_vl_keypoint_vector_from_file(kpt2_filename , &kpt2_kvp);
    if (result == ERROR) { EGC(result); }
    pso("Read %d keypoints\n", kpt2_kvp->length);

    result = get_putative_matches_3(kpt1_kvp,
                                  kpt2_kvp,
                                  0.6,
                                  10,
                                  &num_putative_matches,
                                  &(match_idx_imp));    
    if (result == ERROR) { EGC(result); }
    pso("%d putative matches.\n", num_putative_matches);

    pso("\nRunning the function without getting num_putative_matches back.\n");
    result = get_putative_matches_3(kpt1_kvp,
                                  kpt2_kvp,
                                  0.6,
                                  10,
                                  NULL,
                                  &(match_idx_imp));    
    if (result == ERROR) { EGC(result); }

    pso("\nRunning the function without getting num_putative_matches & match_idx_imp back.\n");
    result = get_putative_matches_3(kpt1_kvp,
                                  kpt2_kvp,
                                  0.6,
                                  10,
                                  NULL,
                                  NULL);    
    if (result == ERROR) { EGC(result); }

    pso("\nRunning the function without getting match_idx_imp back.\n");
    result = get_putative_matches_3(kpt1_kvp,
                                  kpt2_kvp,
                                  0.6,
                                  10,
                                  &num_putative_matches,
                                  NULL); 
    if (result == ERROR) { EGC(result); }
    pso("%d putative matches.\n", num_putative_matches);

cleanup:
    EPE( result );
    pso("\nCleaning up.\n");
    free_keypoint_vector(kpt1_kvp); 
    free_keypoint_vector(kpt2_kvp); 
    free_int_matrix(match_idx_imp);
    pso("Finished.\n");

    return result; 
}

