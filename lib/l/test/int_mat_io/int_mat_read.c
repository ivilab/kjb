
/* $Id: int_mat_read.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "i/i_incl.h" 

int main(int argc, char *argv[])
{
    Int_matrix* mp       = NULL;
    int         read_res;


    create_image_display(); 

    EPETE(read_int_matrix(&mp, argv[ 1 ]));

    if ((mp->num_rows < 100) && (mp->num_cols < 10))
    {
        EPETE(write_int_matrix(mp, (char*)NULL));
    }
    else 
    {
        KJB_image* ip = NULL;
        int i, j; 

        EPETE(get_zero_image(&ip, mp->num_rows, mp->num_cols));

        for (i = 0; i < mp->num_rows; i++)
        {
            for (j = 0; j < mp->num_cols; j++)
            {
                if (mp->elements[ i ][ j ] == 0)
                {
                    ip->pixels[ i ][ j ].r = 200.0;
                    ip->pixels[ i ][ j ].g = 0.0;
                    ip->pixels[ i ][ j ].b = 0.0;
                }
                else if (mp->elements[ i ][ j ] == 1)
                {
                    ip->pixels[ i ][ j ].r = 0.0;
                    ip->pixels[ i ][ j ].g = 200.0;
                    ip->pixels[ i ][ j ].b = 0.0;
                }
                else if (mp->elements[ i ][ j ] == 2)
                {
                    ip->pixels[ i ][ j ].r = 0.0;
                    ip->pixels[ i ][ j ].g = 0.0;
                    ip->pixels[ i ][ j ].b = 200.0;
                }
                else if (mp->elements[ i ][ j ] == 3)
                {
                    ip->pixels[ i ][ j ].r = 200.0;
                    ip->pixels[ i ][ j ].g = 0.0;
                    ip->pixels[ i ][ j ].b = 200.0;
                }
                else if (mp->elements[ i ][ j ] == 4)
                {
                    ip->pixels[ i ][ j ].r = 0.0;
                    ip->pixels[ i ][ j ].g = 200.0;
                    ip->pixels[ i ][ j ].b = 200.0;
                }
                else if (mp->elements[ i ][ j ] == 5)
                {
                    ip->pixels[ i ][ j ].r = 200.0;
                    ip->pixels[ i ][ j ].g = 200.0;
                    ip->pixels[ i ][ j ].b = 0.0;
                }
                else if (mp->elements[ i ][ j ] == 6)
                {
                    ip->pixels[ i ][ j ].r = 200.0;
                    ip->pixels[ i ][ j ].g = 200.0;
                    ip->pixels[ i ][ j ].b = 200.0;
                }
            }
        }

        EPETE(kjb_display_image(ip, NULL));

        prompt_to_continue();

        kjb_free_image(ip); 
    }

    free_int_matrix(mp); 

    return EXIT_SUCCESS; 
}

