
/* $Id: copy_vector.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 
#include "kpt/kpt_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Keypoint_vector* kpt1_kvp = NULL;
    Keypoint_vector* kpt2_kvp = NULL;
    Matrix *temp = NULL;
    Int_vector* mask_ivp = NULL;
    int result = EXIT_SUCCESS;


    kjb_init(); 

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    result = read_matrix( &temp, "data/000001.vl");
    if (result == ERROR) { EGC(result); }

    /*result = read_keypoint_from_vector( &kpt1_kvp, "000001.vl");*/
    result = get_keypoint_vector_from_matrix( &kpt1_kvp, temp);
    if (result == ERROR) { EGC(result); }

    result = read_int_vector( &mask_ivp, "data/mask_ivp");
    if (result == ERROR) { EGC(result); }

    db_irv(mask_ivp);

    /*result = copy_keypoint_vector( &kpt2_kvp, kpt1_kvp );*/
    result = copy_keypoint_vector_selected_rows( &kpt2_kvp, kpt1_kvp, mask_ivp );
    if (result == ERROR) { EGC(result); }

    /*
    result = save_keypoint_vector( kpt2_kvp, "000001-new.vl");
    if (result == ERROR) { EGC(result); }
*/


cleanup:
    EPE( result );
    free_keypoint_vector(kpt1_kvp); 
    free_keypoint_vector(kpt2_kvp); 
    free_matrix(temp);
    free_int_vector(mask_ivp);

    return result; 
}

