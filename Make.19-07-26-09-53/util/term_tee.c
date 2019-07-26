
#include "l/l_incl.h"

/* Experimental attempt to support line editing in "test_program -i".  */

#define LINE_VERSION

#ifdef LINE_VERSION

int main(int argc, char** argv)
{
    char file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp; 
    char line[ GET_LINE_BUFF_SIZE ]; 
    char prompt[ 64 ] = "";


    kjb_set_verbose_level(0);
    /*
    kjb_init();
    */

    check_num_args(argc, 1, 2, (char*)NULL);

    BUFF_CPY(file_name, argv[ 1 ]);

    NPETE(fp = kjb_fopen(file_name, "w"));

    if (argc == 3) 
    {
        BUFF_CPY(prompt, argv[ 2 ]);
    }

    while (BUFF_TERM_GET_LINE(prompt, line) != EOF)
    {
        EPETE(put_line(line));
        EPETE(fput_line(fp, line));
        EPETE(kjb_fflush(NULL));
        nap(100);
    }

    kjb_fclose(fp); 

    return EXIT_SUCCESS; 
}


#else 

/* NOT FINISHED! */

int main(int argc, char** argv)
{
    char file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp; 
    char c; 


    kjb_init();

    check_num_args(argc, 1, 1, (char*)NULL);

    BUFF_CPY(file_name, argv[ 1 ]);

    NPETE(fp = kjb_fopen(file_name, "w"));

    if (is_interactive())
    {
        term_set_raw_mode_with_echo();
        add_cleanup_function(term_reset);
    }

    while (kjb_read(fileno(stdin), (void*)&c, 1) != EOF)
    {
        if (c == '') break;

        EPETE(kjb_write(fileno(stdout), (void*)&c, 1));
        EPETE(kjb_write(fileno(fp), (void*)&c, 1));
    }

    kjb_fclose(fp); 

    return EXIT_SUCCESS; 
}

#endif

