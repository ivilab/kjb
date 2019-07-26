
/* $Id: get_temp_file_name.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h"

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char temp_file_name[ MAX_FILE_NAME_SIZE ]; 


    EPETE(get_temp_file_name(temp_file_name, sizeof(temp_file_name)));
    dbs(temp_file_name); 
    NPETE(kjb_fopen(temp_file_name, "w")); 

    EPETE(get_temp_file_name(temp_file_name, sizeof(temp_file_name)));
    dbs(temp_file_name); 
    NPETE(kjb_fopen(temp_file_name, "w")); 

    EPETE(get_temp_file_name(temp_file_name, sizeof(temp_file_name)));
    dbs(temp_file_name); 
    NPETE(kjb_fopen(temp_file_name, "w")); 

    return EXIT_SUCCESS; 
}

