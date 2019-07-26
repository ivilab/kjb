
/* $Id: vec_metric.c 21491 2017-07-20 13:19:02Z kobus $ */


/*
 * FIXME
 *
 * For some reason, gcc cannot produce good code for the IH code, especially
 * without optimization. We set no optimization in the makefile, and set the
 * tolerence relatively large. This issue is not really resolved. 
*/

#include "m/m_incl.h"


/*
#define VERBOSE 1
*/

#define BASE_MAX_LEN    1000
#define BASE_NUM_TRIES   100


#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif


/* -------------------------------------------------------------------------- */

static double IH_DotProduct  (Vector* vect1, Vector* vect2);
static double IH_AngleVectors(Vector* vect1, Vector* vect2);
static double IH_VectorMag   (Vector* vect);

/* -------------------------------------------------------------------------- */

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int     length;
    int     count;
    Vector* first_vp  = NULL;
    Vector* second_vp = NULL;
    int  num_tries;
    int  max_len;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 10;
        max_len   = 20;
    }
    else
    {
        double factor_for_linear = pow((double)test_factor, 1.0/3.0);

        max_len = kjb_rint((double)BASE_MAX_LEN * factor_for_linear);
        num_tries = kjb_rint((double)BASE_NUM_TRIES * factor_for_linear);
    }

    if (is_interactive())
    {
        kjb_set_verbose_level(2);
        kjb_set_debug_level(2);
    }
    else
    {
        kjb_set_verbose_level(0);
        kjb_set_debug_level(0);
    }

    for (count=0; count<num_tries; count++)
    {
        for (length=1; length<max_len; length++)
        {
            int  i;
            double diff[ 100 ];  /* Room for lots! */
            double dot_product;
            double test_dot_product;
            double angle_in_degrees;
            double test_angle_in_degrees;
            double angle_in_radians;
            double test_angle_in_radians;
            int  diff_count            = 0;


            verbose_pso(1, "\n-------------------------------------------------\n\n");
            verbose_pso(1, "%d %d\n\n", count, length);
            
            EPETE(get_random_vector(&first_vp, length)); 
            EPETE(get_random_vector(&second_vp, length)); 

            EPETE(ow_subtract_scalar_from_vector(first_vp, 0.5)); 
            EPETE(ow_subtract_scalar_from_vector(second_vp, 0.5)); 

            dot_product = IH_DotProduct(first_vp, second_vp);

            EPETE(get_dot_product(first_vp, second_vp, &test_dot_product));
            diff[ diff_count++ ] = ABS_OF(dot_product - test_dot_product) / (100.0 * (double)length); 

            angle_in_radians = IH_AngleVectors(first_vp, second_vp);
            EPETE(get_vector_angle_in_radians(first_vp, second_vp, 
                                              &test_angle_in_radians));
            diff[ diff_count++ ] = ABS_OF(angle_in_radians - test_angle_in_radians) / (100.0 * (double)length); 

            angle_in_degrees = 180.0 * angle_in_radians / M_PI;
            EPETE(get_vector_angle_in_degrees(first_vp, second_vp, 
                                              &test_angle_in_degrees));
            diff[ diff_count++ ] = ABS_OF(angle_in_degrees - test_angle_in_degrees) / (10000.0 * (double)length); 

            for (i=0; i<diff_count; i++)
            {
                verbose_pso(2, "Test %d: %e\n", i, diff[ i ]);

                if (diff[ i ] > 1e-7)
                {
                    
                    p_stderr("IH angle: %.20e\n", angle_in_radians); 
                    p_stderr("KJB angle: %.20e\n", test_angle_in_radians); 
                    p_stderr("Test %d with length %d of set %d failed. (%e > %e)\n", 
                             i, length, count, 
                            (double)diff[ i ], DBL_EPSILON);
                    status = EXIT_BUG;
                }
            }
        }
    }
    
    free_vector(first_vp); 
    free_vector(second_vp); 


    return status; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static double IH_DotProduct(Vector *vect1, Vector *vect2)
{
/* Returns the dot product of the two vectors vect1 and vect2.
*/
   int    i;
   double dotproduct = 0.0;
   
   /* verify input arguments */
   if (vect1 == NULL || vect2 == NULL || vect1->length < 0) {
      /* SetVisionError(VE_FUNCARGS, "DotProduct"); */
      kjb_exit(EXIT_BUG); 
   }
   /* vectors must be the same size */
   if (vect1->length != vect2->length) {
      /* SetVisionError(VE_VECTSIZE, "in function DotProduct"); */
      kjb_exit(EXIT_BUG); 
   }   
   for (i = 0; i < vect1->length; i++)
      dotproduct += vect1->elements[i] * vect2->elements[i];
      
   return (dotproduct);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static double IH_AngleVectors(Vector *vect1, Vector *vect2)
{
/* Returns the angle in radians between the two vectors vect1 and
   vect2.  If an error occurs, -1.0 is returned.
*/
   double dotproduct, mag;

   /* verify input arguments */
   if (vect1 == NULL || vect2 == NULL
         || vect1->length < 0 || vect2->length < 0) {
      /* SetVisionError(VE_FUNCARGS, "AngleVectors"); */
      return (-1.0);
   }
   /* undefined for vectors of no magnitude */
   if ((mag = IH_VectorMag(vect1) * IH_VectorMag(vect2)) <= 0.0) {
      /* SetVisionError(VE_VECTMAG, "in function AngleVectors"); */
      return (-1.0);
   }      
   /* dot product over mags is the cosine of the angle */
   dotproduct = IH_DotProduct(vect1, vect2) / mag;

   /* could have slight roundoff that would be rejected by acos() */
   dotproduct = (dotproduct > 1.0 ? 1.0 : 
         (dotproduct < -1.0 ? -1.0 : dotproduct));
            
   return (acos(dotproduct));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static double IH_VectorMag(Vector *vect)
{
/* On success, returns the magnitude of vector vect, otherwise -1.0.
*/
   int i;
   double total = 0.0;

   /* verify arguments */
   if (vect == NULL || vect->length < 0) {
      /* SetVisionError(VE_FUNCARGS, "VectorMag"); */
      return (-1.0);
   }
   /* calculate magnitude^2 */
   for (i = 0; i < vect->length; i++)
      total += vect->elements[i] * vect->elements[i];

   return (sqrt(total));
}



