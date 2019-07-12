
/* $Id: word_list.c 6964 2010-10-06 18:58:41Z kobus $ */


#include "l/l_incl.h" 

#define BASE_NUM_TRIES  1

int main(int argc, char **argv)
{
    Int_vector* a_vp = NULL; 
    Int_vector* b_vp = NULL; 

    kjb_init();

    if (is_interactive())
    {
        kjb_set_debug_level(2); 
    }
    else 
    {
        kjb_set_debug_level(0); 
    }

    EPETE(get_initialized_int_vector(&a_vp, 2000, 0));

    START_HEAP_CHECK_2();
    EPETE(get_initialized_int_vector(&b_vp, 5000, 0));
    FINISH_HEAP_CHECK_2();

    free_int_vector(b_vp);

    kjb_cleanup();
    
    return EXIT_SUCCESS;
}

