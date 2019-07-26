
/* $Id: word_list.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "l/l_incl.h" 


int main(void)
{
    Word_list* word_list_ptr = NULL; 
    Word_list* sorted_word_list_ptr = NULL; 

    kjb_init();

    if (is_interactive())
    {
        kjb_set_debug_level(2); 
    }
    else 
    {
        kjb_set_debug_level(0); 
    }

    EPETE(read_word_list(&word_list_ptr, NULL));
    EPETE(sort_word_list(&sorted_word_list_ptr, word_list_ptr));
    EPETE(write_word_list(sorted_word_list_ptr, NULL));

    free_word_list(word_list_ptr); 
    free_word_list(sorted_word_list_ptr); 

    kjb_cleanup();
    
    return EXIT_SUCCESS;
}

