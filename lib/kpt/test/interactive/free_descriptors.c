
/* $Id: copy_vector.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 
#include "kpt/kpt_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Keypoint_vector* kpt1_kvp = NULL;
    Keypoint_vector* kpt2_kvp = NULL;
    Matrix *tmp_mp = NULL;
    int result = EXIT_SUCCESS;
    int i = 0;

    kjb_init(); 

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    result = read_matrix( &tmp_mp, "data/frame_vl_key/000001.vl");
    if (result == ERROR) { EGC(result); }

    result = get_keypoint_vector_from_matrix( &kpt1_kvp, tmp_mp);
    if (result == ERROR) { EGC(result); }

    verbose_pso(5, "Total %d keypoints.\n", kpt1_kvp->length);
    /*
    for (i = 0; i < kpt1_kvp->length; i++)
    {
        if (kpt1_kvp->elements[i]->descrip != NULL)
        {
            pso("%d ", i);
        }
    }
    */
    result = copy_keypoint_vector( &kpt2_kvp, kpt1_kvp );
    if (result == ERROR) { EGC(result); }

    free_keypoint_vector_descriptors( kpt1_kvp );

    for (i = 0; i < kpt1_kvp->length; i++)
    {
        if ( (kpt1_kvp->elements[i]->row != kpt2_kvp->elements[i]->row)
             || (kpt1_kvp->elements[i]->col != kpt2_kvp->elements[i]->col))

        {
            add_error("row or col values fro keypoint %d don't match!", i);
            result = ERROR;
            EGC(result);
        }
    }


cleanup:
    EPE( result );
    free_matrix(tmp_mp);
    free_keypoint_vector(kpt1_kvp); 
    free_keypoint_vector(kpt2_kvp); 

    return result; 
}

