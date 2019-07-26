
/* $Id: append_word_list.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "l/l_incl.h" 

#define BASE_NUM_TRIES  1

int main(void)
{
    Word_list* word_list_ptr = NULL; 
    char line[ 1000 ];

    kjb_init();

    if (is_interactive())
    {
        kjb_set_debug_level(2); 
    }
    else 
    {
        kjb_set_debug_level(0); 
    }

    while (BUFF_GET_REAL_LINE(stdin, line) != EOF)
    {
        EPETE(append_word_list(&word_list_ptr, line));
    }

    EPETE(write_word_list(word_list_ptr, NULL));

    free_word_list(word_list_ptr); 

    kjb_cleanup();
    
    return EXIT_SUCCESS;
}

