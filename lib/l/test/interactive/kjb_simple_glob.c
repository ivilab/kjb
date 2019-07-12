
/* $Id: kjb_simple_glob.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 

int main(void)
{
    char beg[ MAX_FILE_NAME_SIZE ];
    char end[ MAX_FILE_NAME_SIZE ];
    Word_list* paths = NULL; 
    Word_list* star_wl = NULL; 
    unsigned int i;
    char** words;
    char** stars;
    unsigned int num_words; 
    


    while (BUFF_STDIN_GET_LINE("beg> ", beg) != EOF)
    {
        if (BUFF_STDIN_GET_LINE("end> ", end) == EOF) break;

        dbs(beg);
        dbs(end);

        dbp("--------- PATHS  -------------");

        EPE(kjb_simple_glob(&paths, &star_wl, beg, end, NULL));

        num_words = paths->num_words;
        words = paths->words;
        stars = star_wl->words; 

        for (i = 0; i < num_words; i++)
        {
            dbs(words[ i ]); 
            dbs(stars[ i ]); 
        }

        dbp("--------- FILES  -------------");

        EPE(kjb_simple_glob(&paths, &star_wl, beg, end, is_file));

        num_words = paths->num_words;
        words = paths->words;
        stars = star_wl->words; 

        for (i = 0; i < num_words; i++)
        {
            dbs(words[ i ]); 
            dbs(stars[ i ]); 
        }

    }

    free_word_list(paths); 
    free_word_list(star_wl); 

    return EXIT_SUCCESS; 
} 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

