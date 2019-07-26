
/* $Id: kjb_err.c 9126 2011-04-01 02:39:12Z predoehl $ */



#include "l/l_gen.h" 

int main(int argc, char *argv[])
{
    char *test_str; 
    char buff[ 1000 ]; 

    int err_strlen;



    test_str = "Another very looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong message";

    set_error("%s", test_str);  

    add_error("%50.30s", 
              "A looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong message");

    add_error("Trouble with %F.", stdin);
    /* add_error("Test bad format element with: -->%Z<--.", stdin); */
    add_error("%s", test_str);  

    add_error("%50.30s", 
              "A looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong message");

    add_error("Trouble with %F.", stdin);
    add_error("Trouble with %hd.", ((kjb_int16)666));
    add_error("Trouble with %+*.*hd.", 30,5, ((kjb_int16)666));
    add_error("Trouble with %*.*Lf.", 10,5, ((long_double)1.2345678));

    kjb_fprintf(stderr, "Printing.\n");
    kjb_print_error(); 

    kjb_fprintf(stderr, "Printing nothing.\n");
    kjb_print_error(); 


    kjb_clear_error();
    insert_error("%s", test_str); 


    kjb_fprintf(stderr, "Printing.\n");
    kjb_print_error(); 

    kjb_fprintf(stderr, "Printing nothing.\n");
    kjb_print_error(); 


    set_error("%s", test_str);  
    insert_error("%50.30s", 
                 "A looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong message");

    insert_error("Trouble with %F.", stdin);
    /* insert_error("Test bad format element with: -->%Z<--.", stdin); */
    insert_error("%s", test_str);  
    insert_error("%50.30s", 
                 "A looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong message");

    insert_error("Trouble with %F.", stdin);
    insert_error("Trouble with %hd.", ((kjb_int16)666));
    insert_error("Trouble with %+*.*hd.", 30,5, ((kjb_int16)666));
    insert_error("Trouble with %*.*Lf.", 10,5, ((long_double)1.2345678));

    kjb_fprintf(stderr, "Printing.\n");
    kjb_print_error(); 

    kjb_fprintf(stderr, "Printing nothing.\n");
    kjb_print_error(); 

    set_error("Set error");
    add_error("After set error");
    insert_error("Before set error");
    cat_error(" .... AND MORE");
    add_error("After After set error");
    cat_error(" .... AND MORE");

    kjb_fprintf(stderr, "Printing.\n");

    err_strlen = kjb_get_strlen_error();

    kjb_get_error(buff, 1000);
    kjb_fprintf(stderr, "\n\nBuff is ->%s<-\n\n", buff);

    if ( err_strlen != signed_strlen( buff ) )
    {
        kjb_printf( stderr, "Failure:  strlen prediction was %d, "
                        "actual was %d\n", err_strlen, signed_strlen( buff ) );
        return EXIT_FAILURE;
    }

    set_error("Another user message");

    kjb_fprintf(stderr, "Printing.\n");
    kjb_print_error(); 

    kjb_fprintf(stderr, "Printing nothing.\n");
    kjb_print_error(); 

    return EXIT_SUCCESS;
}

