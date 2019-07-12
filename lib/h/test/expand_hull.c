
/* $Id: expand_hull.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

/*ARGSUSED*/
int main(void)
{
    Hull* hp;
    Hull* new_hp; 
    Matrix* mp = NULL;
    int i, j, k; 

    /*
    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    */

    kjb_seed_rand(0, 0);

    for (k=0; k<2; k++)
    {
        EPETE(get_target_matrix(&mp, 5 + k, 2 + k));

        for (i = 0; i<mp->num_rows; i++)
        {
            for (j=0; j<mp->num_cols; j++)
            {
                mp->elements[ i ][ j ] = (int)(100.0 * kjb_rand());
            }
        }

        db_mat(mp); 

        NPETE(hp = find_convex_hull(mp, DEFAULT_HULL_OPTIONS)); 

        dbm("Setting expand-hulls-exactly to TRUE"); 
        set_hull_options("expand-hulls-exactly","t"); 
        NPETE(new_hp = expand_hull(hp, .1, DEFAULT_HULL_OPTIONS));
        db_mat(new_hp->vertex_mp); 

        free_hull(new_hp); 
        dbm("Setting expand-hulls-exactly to FALSE"); 
        set_hull_options("expand-hulls-exactly","f"); 
        NPETE(new_hp = expand_hull(hp, .1, DEFAULT_HULL_OPTIONS));
        db_mat(new_hp->vertex_mp);  

        free_hull(new_hp); 
        free_hull(hp);
    }

    free_matrix(mp); 

    return EXIT_SUCCESS; 
}

