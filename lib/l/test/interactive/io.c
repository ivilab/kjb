
/* $Id: io.c 4723 2009-11-16 18:57:09Z kobus $ */

#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    FILE* fp1;
    FILE* fp2;
    int   ints[10];
    int   i;
    int   x;
    char  temp_file_name[ 100 ]; 


    BUFF_GET_TEMP_FILE_NAME(temp_file_name); 

    NPETE(fp1=kjb_fopen(temp_file_name,"w")); 

    for (i=0;i<10;i++) 
    {
        FIELD_WRITE(fp1,i); 
    }
    kjb_fclose(fp1);  

    NPETE(fp2=kjb_fopen(temp_file_name,"r")); 
    ARRAY_READ(fp2,ints); 
    kjb_fclose(fp2);  

    for (i=0;i<10;i++)
    {
        pso("Should count from one to ten.\n"); 
        pso("%d\n", ints[i]); 
    }

    NPETE(fp2=kjb_fopen(temp_file_name,"r")); 

    for (i=0;i<10;i++)
    {
        FIELD_READ(fp2,x); 
        ints[i]=x; 
    }
    kjb_fclose(fp2);  

    for (i=0;i<10;i++)
    {
        pso("Should count from one to ten.\n"); 
        pso("%d\n", ints[i]); 
    }

    kjb_unlink(temp_file_name); 
}

