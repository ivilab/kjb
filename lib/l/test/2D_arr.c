
/* $Id: 2D_arr.c 21491 2017-07-20 13:19:02Z kobus $ */


 
#include "l/l_incl.h" 

#define MAX_NUM_ROWS 200
#define MAX_NUM_COLS 200

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



/* #define VERBOSE 1 */


static void report_failure(const char*);


/* -------------------------------------------------------------------------- */

int main(int  argc, char *argv[] )
{
    unsigned char*    byte_ptr;
    unsigned char**   byte_ptr_ptr;
    short*   short_ptr;
    short**  short_ptr_ptr;
    int*     int_ptr;
    int**    int_ptr_ptr;
    long*    long_ptr;
    long**   long_ptr_ptr;
    kjb_int16*   int16_ptr;
    kjb_int16**  int16_ptr_ptr;
    kjb_int32*   int32_ptr;
    kjb_int32**  int32_ptr_ptr;
    float*   float_ptr;
    float**  float_ptr_ptr;
    double*  double_ptr;
    double** double_ptr_ptr;
    int      i, j;
    int      num_rows, num_cols; 
    int      max_num_rows = MAX_NUM_ROWS;
    int      max_num_cols = MAX_NUM_COLS; 
    int      test_factor = 1; 


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (is_interactive())
    {
        kjb_set_verbose_level(2);
    }
    else 
    {
        kjb_set_verbose_level(0); 
    }

    if (test_factor == 0)
    {
        max_num_rows = 10;
        max_num_cols = 10;
    }
    else if (test_factor > 1)
    {
        double d_max_num_rows = (double)max_num_rows * sqrt((double)test_factor);
        double d_max_num_cols = (double)max_num_cols * sqrt((double)test_factor);

        max_num_rows = kjb_rint(d_max_num_rows); 
        max_num_cols = kjb_rint(d_max_num_cols); 
    }

    num_rows = 0;

    while (num_rows < max_num_rows)
    {
        num_cols = 0;

        while (num_cols < max_num_cols)
        {
            verbose_pso(1, "%d X %d\n", num_rows, num_cols); 

            byte_ptr_ptr = allocate_2D_byte_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                byte_ptr = *byte_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *byte_ptr = TEST_BYTE;
                byte_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( byte_ptr_ptr[ i ][ j] != TEST_BYTE )
                    {
                        report_failure( "2D unsigned char array failure.\n" );
                    }
                }
            }

            free_2D_byte_array(byte_ptr_ptr);

            byte_ptr_ptr = allocate_2D_byte_array(num_rows, num_cols);

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    byte_ptr_ptr[ i ][ j] = TEST_BYTE;
                }
            }

            if (num_rows > 0)
            {
                byte_ptr = *byte_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *byte_ptr != TEST_BYTE )
                {
                    report_failure( "2D unsigned char array failure.\n" );
                }
                byte_ptr++;
            }
           
            short_ptr_ptr = allocate_2D_short_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                short_ptr = *short_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *short_ptr = TEST_SHORT;
                short_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( short_ptr_ptr[ i ][ j] != TEST_SHORT )
                    {
                        report_failure( "2D short array failure.\n" );
                    }
                }
            }

            free_2D_short_array( short_ptr_ptr );

            short_ptr_ptr = allocate_2D_short_array(num_rows, num_cols);

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    short_ptr_ptr[ i ][ j] = TEST_SHORT;
                }
            }

            if (num_rows > 0)
            {
                short_ptr = *short_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *short_ptr != TEST_SHORT )
                {
                    report_failure( "2D short array failure.\n" );
                }
                short_ptr++;
            }

           
            int_ptr_ptr = allocate_2D_int_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                int_ptr = *int_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *int_ptr = TEST_INT;
                int_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( int_ptr_ptr[ i ][ j] != TEST_INT )
                    {
                        report_failure( "2D int array failure.\n" );
                    }
                }
            }

            free_2D_int_array( int_ptr_ptr );

            int_ptr_ptr = allocate_2D_int_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    int_ptr_ptr[ i ][ j] = TEST_INT;
                }
            }

            if (num_rows > 0)
            {
                int_ptr = *int_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *int_ptr != TEST_INT )
                {
                    report_failure( "2D int array failure.\n" );
                }
                int_ptr++;
            }


            long_ptr_ptr = allocate_2D_long_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                long_ptr = *long_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *long_ptr = TEST_LONG;
                long_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( long_ptr_ptr[ i ][ j] != TEST_LONG )
                    {
                        report_failure( "2D long array failure.\n" );
                    }
                }
            }

            free_2D_long_array( long_ptr_ptr );

            long_ptr_ptr = allocate_2D_long_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    long_ptr_ptr[ i ][ j] = TEST_LONG;
                }
            }

            if (num_rows > 0)
            {
                long_ptr = *long_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *long_ptr != TEST_LONG )
                {
                    report_failure( "2D long array failure.\n" );
                }
                long_ptr++;
            }

            int16_ptr_ptr = allocate_2D_int16_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                int16_ptr = *int16_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *int16_ptr = TEST_INT16;
                int16_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( int16_ptr_ptr[ i ][ j] != TEST_INT16 )
                    {
                        report_failure( "2D kjb_int16 array failure.\n" );
                    }
                }
            }

            free_2D_int16_array( int16_ptr_ptr );

            int16_ptr_ptr = allocate_2D_int16_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    int16_ptr_ptr[ i ][ j] = TEST_INT16;
                }
            }

            if (num_rows > 0)
            {
                int16_ptr = *int16_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *int16_ptr != TEST_INT16 )
                {
                    report_failure( "2D kjb_int16 array failure.\n" );
                }
                int16_ptr++;
            }


            int32_ptr_ptr = allocate_2D_int32_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                int32_ptr = *int32_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *int32_ptr = TEST_INT32;
                int32_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    if ( int32_ptr_ptr[ i ][ j] != TEST_INT32 )
                    {
                        report_failure( "2D kjb_int32 array failure.\n" );
                    }
                }
            }

            free_2D_int32_array( int32_ptr_ptr );

            int32_ptr_ptr = allocate_2D_int32_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    int32_ptr_ptr[ i ][ j] = TEST_INT32;
                }
            }

            if (num_rows > 0)
            {
                int32_ptr = *int32_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                if ( *int32_ptr != TEST_INT32 )
                {
                    report_failure( "2D kjb_int32 array failure.\n" );
                }
                int32_ptr++;
            }

            float_ptr_ptr = allocate_2D_float_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                float_ptr = *float_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *float_ptr = TEST_FLOAT;  
                float_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    /*
                    // Dangerous "exact" test, but OK for this purpose.
                    */
                    if ( float_ptr_ptr[ i ][ j] != TEST_FLOAT )
                    {
                        report_failure( "2D float array failure.\n" );
                    }
                }
            }

            free_2D_float_array( float_ptr_ptr );

            float_ptr_ptr = allocate_2D_float_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    float_ptr_ptr[ i ][ j] = TEST_FLOAT;
                }
            }

            if (num_rows > 0)
            {
                float_ptr = *float_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                /*
                // Dangerous "exact" test, but OK for this purpose.
                */
                if ( *float_ptr != TEST_FLOAT )
                {
                    report_failure( "2D float array failure.\n" );
                }
                float_ptr++;
            }

            double_ptr_ptr = allocate_2D_double_array( num_rows, num_cols );

            if (num_rows > 0)
            {
                double_ptr = *double_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                *double_ptr = TEST_DOUBLE;
                double_ptr++;
            }

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    /*
                    // Dangerous "exact" test, but OK for this purpose.
                    */
                    if ( double_ptr_ptr[ i ][ j] != TEST_DOUBLE )
                    {
                        report_failure( "2D double array failure.\n" );
                    }
                }
            }

            free_2D_double_array( double_ptr_ptr );

            double_ptr_ptr = allocate_2D_double_array( num_rows, num_cols );

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    double_ptr_ptr[ i ][ j] = TEST_DOUBLE;
                }
            }

            if (num_rows > 0)
            {
                double_ptr = *double_ptr_ptr;
            }

            for (i=0; i<num_rows * num_cols; i++)
            {
                /*
                // Dangerous "exact" test, but OK for this purpose.
                */
                if ( *double_ptr != TEST_DOUBLE )
                {
                    report_failure( "2D double array failure.\n" );
                }
                double_ptr++;
            }

            free_2D_double_array( double_ptr_ptr);
            free_2D_float_array(float_ptr_ptr);
            free_2D_int32_array(int32_ptr_ptr);
            free_2D_int16_array(int16_ptr_ptr);
            free_2D_long_array(long_ptr_ptr);
            free_2D_int_array(int_ptr_ptr);
            free_2D_short_array(short_ptr_ptr );
            free_2D_byte_array(byte_ptr_ptr);

            num_cols += kjb_rint(5.0 * kjb_rand()); 
        }
        num_rows += kjb_rint(5.0 * kjb_rand()); 
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


