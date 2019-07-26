
/* $Id: kjb_glob.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 

int main(void)
{
    char glob[ MAX_FILE_NAME_SIZE ];
    Word_list* paths = NULL; 
    unsigned int i;
    char** words;
    unsigned int num_words; 
    


    while (BUFF_STDIN_GET_LINE("glob> ", glob) != EOF)
    {
        dbp("--------- MATCHES  -------------");

        EPE(kjb_glob(&paths, glob, NULL));

        num_words = paths->num_words;
        words = paths->words;

        for (i = 0; i < num_words; i++)
        {
            dbs(words[ i ]); 
        }

    }

    free_word_list(paths); 

    return EXIT_SUCCESS; 
} 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

