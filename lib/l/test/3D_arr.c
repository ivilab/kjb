
/* $Id: 3D_arr.c 21491 2017-07-20 13:19:02Z kobus $ */


 
#include "l/l_incl.h" 

#define MAX_NUM_BLOCKS  50
#define MAX_NUM_ROWS    50
#define MAX_NUM_COLS    50

#define TEST_BYTE   'K'
#define TEST_INT    44
#define TEST_LONG   -95
#define TEST_FLOAT  123.5f
#define TEST_DOUBLE -789.45
#define TEST_SHORT  89
#define TEST_INT32  -821
#define TEST_INT16  4598

#ifdef SINGLE_PRECISION
#    define TEST_REAL  -78.3f
#else
#    define TEST_REAL  -78.3
#endif 


static void report_failure(const char*);

/* -------------------------------------------------------------------------- */

int main(int  argc, char *argv[] )
{
    unsigned char*   byte_ptr = NULL;
    unsigned char*** byte_ptr_ptr_ptr;
    short*           short_ptr = NULL;
    short***         short_ptr_ptr_ptr;
    int*             int_ptr = NULL;
    int***           int_ptr_ptr_ptr;
    long*            long_ptr = NULL;
    long***          long_ptr_ptr_ptr;
    kjb_int16*       int16_ptr = NULL;
    kjb_int16***     int16_ptr_ptr_ptr;
    kjb_int32*       int32_ptr = NULL;
    kjb_int32***     int32_ptr_ptr_ptr;
    float*           float_ptr = NULL;
    float***         float_ptr_ptr_ptr;
    double*          double_ptr = NULL;
    double***        double_ptr_ptr_ptr;
    int              i, j, k, num_rows, num_cols;
    int              num_blocks;
    int              max_num_rows = MAX_NUM_ROWS;
    int              max_num_cols = MAX_NUM_COLS;
    int              max_num_blocks = MAX_NUM_BLOCKS;
    int              test_factor    = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (is_interactive())
    {
        kjb_set_verbose_level(2);
        kjb_set_debug_level(1);
    }
    else 
    {
        kjb_set_verbose_level(0); 
    }

    if (test_factor == 0)
    {
        max_num_blocks = 10;
        max_num_rows = 10;
        max_num_cols = 10;
    }
    else if (test_factor > 1)
    {
        double d_max_num_rows = (double)max_num_rows * pow((double)test_factor, 1.0 / 3.0);
        double d_max_num_cols = (double)max_num_cols * pow((double)test_factor, 1.0 / 3.0);
        double d_max_num_blocks = (double)max_num_blocks * pow((double)test_factor, 1.0 / 3.0);

        max_num_rows = kjb_rint(d_max_num_rows); 
        max_num_cols = kjb_rint(d_max_num_cols); 
        max_num_blocks = kjb_rint(d_max_num_blocks); 
    }

    num_blocks = 0; 

    while (num_blocks < max_num_blocks)
    {
        num_rows = 0;

        while (num_rows < max_num_rows)
        {
            num_cols = 0;

            while (num_cols < max_num_cols)
            {
                verbose_pso(1, "%d X %d X %d\n", num_blocks, num_rows, num_cols); 

                byte_ptr_ptr_ptr = allocate_3D_byte_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    byte_ptr = **byte_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *byte_ptr = TEST_BYTE;
                    byte_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( byte_ptr_ptr_ptr[ i ][ j][ k ] != TEST_BYTE )
                            {
                                report_failure( "3D unsigned char array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_byte_array( byte_ptr_ptr_ptr );

                byte_ptr_ptr_ptr = allocate_3D_byte_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            byte_ptr_ptr_ptr[ i ][ j][ k ] = TEST_BYTE;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    byte_ptr = **byte_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *byte_ptr != TEST_BYTE )
                    {
                        report_failure( "3D unsigned char array failure.\n" );
                    }
                    byte_ptr++;
                }



               
                short_ptr_ptr_ptr = allocate_3D_short_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    short_ptr = **short_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *short_ptr = TEST_SHORT;
                    short_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( short_ptr_ptr_ptr[ i ][ j][ k ] != TEST_SHORT )
                            {
                                report_failure( "3D short array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_short_array( short_ptr_ptr_ptr );

                short_ptr_ptr_ptr = allocate_3D_short_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            short_ptr_ptr_ptr[ i ][ j][ k ] = TEST_SHORT;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    short_ptr = **short_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *short_ptr != TEST_SHORT )
                    {
                        report_failure( "3D short array failure.\n" );
                    }
                    short_ptr++;
                }

                int_ptr_ptr_ptr = allocate_3D_int_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int_ptr = **int_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *int_ptr = TEST_INT;
                    int_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( int_ptr_ptr_ptr[ i ][ j][ k ] != TEST_INT )
                            {
                                report_failure( "3D int array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_int_array( int_ptr_ptr_ptr );

                int_ptr_ptr_ptr = allocate_3D_int_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            int_ptr_ptr_ptr[ i ][ j][ k ] = TEST_INT;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int_ptr = **int_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *int_ptr != TEST_INT )
                    {
                        report_failure( "3D int array failure.\n" );
                    }
                    int_ptr++;
                }

                long_ptr_ptr_ptr = allocate_3D_long_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    long_ptr = **long_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *long_ptr = TEST_LONG;
                    long_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( long_ptr_ptr_ptr[ i ][ j][ k ] != TEST_LONG )
                            {
                                report_failure( "3D long array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_long_array( long_ptr_ptr_ptr );

                long_ptr_ptr_ptr = allocate_3D_long_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            long_ptr_ptr_ptr[ i ][ j][ k ] = TEST_LONG;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    long_ptr = **long_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *long_ptr != TEST_LONG )
                    {
                        report_failure( "3D long array failure.\n" );
                    }
                    long_ptr++;
                }

                int16_ptr_ptr_ptr = allocate_3D_int16_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int16_ptr = **int16_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *int16_ptr = TEST_INT16;
                    int16_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( int16_ptr_ptr_ptr[ i ][ j][ k ] != TEST_INT16 )
                            {
                                report_failure( "3D kjb_int16 array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_int16_array( int16_ptr_ptr_ptr );

                int16_ptr_ptr_ptr = allocate_3D_int16_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            int16_ptr_ptr_ptr[ i ][ j][ k ] = TEST_INT16;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int16_ptr = **int16_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *int16_ptr != TEST_INT16 )
                    {
                        report_failure( "3D kjb_int16 array failure.\n" );
                    }
                    int16_ptr++;
                }

                int32_ptr_ptr_ptr = allocate_3D_int32_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int32_ptr = **int32_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *int32_ptr = TEST_INT32;
                    int32_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            if ( int32_ptr_ptr_ptr[ i ][ j][ k ] != TEST_INT32 )
                            {
                                report_failure( "3D kjb_int32 array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_int32_array( int32_ptr_ptr_ptr );

                int32_ptr_ptr_ptr = allocate_3D_int32_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            int32_ptr_ptr_ptr[ i ][ j][ k ] = TEST_INT32;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    int32_ptr = **int32_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    if ( *int32_ptr != TEST_INT32 )
                    {
                        report_failure( "3D kjb_int32 array failure.\n" );
                    }
                    int32_ptr++;
                }


                float_ptr_ptr_ptr = allocate_3D_float_array( num_blocks, num_rows, num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    float_ptr = **float_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *float_ptr = TEST_FLOAT;
                    float_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            /*
                            // Dangerous "exact" test, but OK for this purpose.
                            */
                            if ( float_ptr_ptr_ptr[ i ][ j][ k ] != TEST_FLOAT )
                            {
                                report_failure( "3D float array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_float_array( float_ptr_ptr_ptr );

                float_ptr_ptr_ptr = allocate_3D_float_array( num_blocks, num_rows, num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            float_ptr_ptr_ptr[ i ][ j][ k ] = TEST_FLOAT;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    float_ptr = **float_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    /*
                    // Dangerous "exact" test, but OK for this purpose.
                    */
                    if ( *float_ptr != TEST_FLOAT )
                    {
                        report_failure( "3D float array failure.\n" );
                    }
                    float_ptr++;
                }

                double_ptr_ptr_ptr = allocate_3D_double_array( num_blocks, num_rows, 
                                                              num_cols );

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    double_ptr = **double_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    *double_ptr = TEST_DOUBLE;
                    double_ptr++;
                }

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            /*
                            // Dangerous "exact" test, but OK for this purpose.
                            */
                            if ( double_ptr_ptr_ptr[ i ][ j][ k ] != TEST_DOUBLE )
                            {
                                report_failure( "3D double array failure.\n" );
                            }
                        }
                    }
                }

                free_3D_double_array( double_ptr_ptr_ptr );

                double_ptr_ptr_ptr = allocate_3D_double_array( num_blocks, num_rows, 
                                                               num_cols );

                for (i=0; i<num_blocks; i++) 
                {
                    for (j=0; j<num_rows; j++)
                    {
                        for (k=0; k<num_cols; k++)
                        {
                            double_ptr_ptr_ptr[ i ][ j][ k ] = TEST_DOUBLE;
                        }
                    }
                } 

                if ((num_blocks > 0) && (num_rows > 0))
                {
                    double_ptr = **double_ptr_ptr_ptr;
                }

                for (i=0; i<num_blocks * num_rows * num_cols; i++)
                {
                    /*
                    // Dangerous "exact" test, but OK for this purpose.
                    */
                    if ( *double_ptr != TEST_DOUBLE )
                    {
                        report_failure( "3D double array failure.\n" );
                    }
                    double_ptr++;
                }

                free_3D_double_array(double_ptr_ptr_ptr);
                free_3D_float_array(float_ptr_ptr_ptr);
                free_3D_int32_array(int32_ptr_ptr_ptr);
                free_3D_int16_array(int16_ptr_ptr_ptr);
                free_3D_long_array(long_ptr_ptr_ptr);
                free_3D_int_array(int_ptr_ptr_ptr);
                free_3D_short_array(short_ptr_ptr_ptr);
                free_3D_byte_array(byte_ptr_ptr_ptr);

                num_cols += kjb_rint(5.0 * kjb_rand()); 
            }

            num_rows += kjb_rint(5.0 * kjb_rand()); 
        }

        num_blocks += kjb_rint(5.0 * kjb_rand()); 
    }


    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void report_failure(const char* message)
{


     p_stderr("\n");
     p_stderr("!!!!!!!!!!!!  FAILURE   !!!!!!!!!!!!\n\n");
     set_high_light(stderr);
     kjb_fputs(stderr, message);
     unset_high_light(stderr);
     p_stderr("\n");
     p_stderr("------------------------------------\n");
     p_stderr("\n");

     kjb_exit( EXIT_BUG );
}


