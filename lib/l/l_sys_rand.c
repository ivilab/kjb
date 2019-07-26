
/* $Id: l_sys_rand.c 15408 2013-09-25 05:38:18Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */

#if 0 /* was #ifdef TRY_WITHOUT */
#ifdef UNIX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/wait.h>
#    include <pwd.h>
#endif
#endif
#endif

/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    ifdef  LINUX
#        include <time.h>
#    endif
#    ifdef  MAC_OSX
#        include <time.h>
#    endif
#    ifdef  sgi
#        include <time.h>
#    endif
#endif

#ifdef MS_OS
#    include "time.h"
#endif 

#include "l/l_sys_scan.h"
#include "l/l_string.h"
#include "l/l_parse.h"
#include "l/l_sys_rand.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Default function pointer for kjb_rand */
static double (*kjb_rand_function)(void) = &kjb_rand_st;

/* Default function pointer for kjb_rand_2 */
static double (*kjb_rand_2_function)(void) = &kjb_rand_2_st;


/* -------------------------------------------------------------------------- */

typedef union Seed_union
{
    kjb_int32 i32;
    kjb_uint16 u16[ 2 ];
}
Seed_union;

typedef enum Seed_status
{
    SEEDED_BY_DEFAULT,
    EXTERNALLY_SEEDED,
    MULTIPLY_SEEDED,
    NUM_SEED_STATUS_ITEMS  /* Ignore lint */
}
Seed_status;

static int    fs_first_call_to_kjb_rand   = TRUE;
static int    fs_first_call_to_kjb_rand_2 = TRUE;
static kjb_uint16 fs_rand_buff[ 3 ] = { 0, 0, 0 };
static kjb_uint16 fs_initial_rand_buff[ 3 ] = { 0, 0, 0 };
static Seed_status fs_initial_seed_status = SEEDED_BY_DEFAULT;
static Seed_status fs_initial_seed_status_2 = SEEDED_BY_DEFAULT;
static long fs_initial_seed_2;

/* ------------------------------------------------------------------------- */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* ============================================================================
 *                                set_random_options
 *
 * Change the library options pertaining to pseudorandom number generation
 *
 * The "seed" option can be queried or set to change the state value in the
 * first PRNG.   The first PRNG has 48 bits of state, which for convenience we
 * subdivide into a 32-bit chunk and a 16-bit chunk.  The value string can have
 * a form like "123:456" which will assign 123 to the first chunk of PRNG
 * state, and 456 to the second chunk.  Since the second chunk is only 16
 * bits, the value after the colon must not have a
 * magnitude that exceeds 16 bits, or else ERROR is returned.
 *
 * Furthermore, if either or both values are stars, as in "123:*" or "*:*" then
 * the corresponding chunk(s) will be set, immediately, from the real-time
 * clock.
 *
 * Option "seed-2" sets the state value in the second PRNG.  The value string
 * may be a single ascii integer decimal value, or star. It works likewise.
 *
 * If the 'value' string begins with '?' then the PRNG state will be printed to
 * standard output.  If the 'value' string is null then this function returns
 * silently.  If the 'value' string is empty then the PRNG state is printed in
 * a more verbose form.
 *
 * Returns:
 *     NO_ERROR, if successful.  ERROR is returned when the second chunk of the
 *     first PRNG cannot store the value indicated by the 'value' string.
 *     NOT_FOUND is returned when the 'option' string contains neither "seed"
 *     nor "seed-2" exactly,
 *
 * Documentor: Andrew Predoehl
 *
 * Author:
 *     Kobus Barnard
 *
 * Index: random, options
 *
 * -----------------------------------------------------------------------------
*/
int set_random_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    long  seed;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "seed")
       )
    {
        kjb_int32 first_value;
        kjb_int32 second_value;
        Seed_union first_seed;


        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if ((value[ 0 ] == '?') || (value[ 0 ] == '\0'))
        {
#ifdef MSB_FIRST
            first_seed.u16[ 0 ] = fs_rand_buff[ 0 ];
            first_seed.u16[ 1 ]  = fs_rand_buff[ 1 ];
            first_value = first_seed.i32;
#else
            first_seed.u16[ 0 ]  = fs_rand_buff[ 1 ];
            first_seed.u16[ 1 ] = fs_rand_buff[ 0 ];
            first_value = first_seed.i32;
#endif

            second_value = (kjb_int32)((kjb_int16)fs_rand_buff[ 2 ]);

            if (value[ 0 ] == '?')
            {
                ERE(pso("seed = %ld:%ld\n", (long)first_value,
                        (long)second_value));

            }
            else
            {
                ERE(pso("Current random number generator seed is %ld:%ld\n",
                        (long)first_value, (long)second_value));

            }

            if (value[ 0 ] == '\0')
            {
                if (fs_first_call_to_kjb_rand)
                {
                    ERE(pso("Random number generator has not been used.\n"));
                }
                else if (fs_initial_seed_status == MULTIPLY_SEEDED)
                {
                    ERE(pso("Random number generator was seeded "));
                    ERE(pso("multiple times.\n"));
                }
                else
                {
#ifdef MSB_FIRST
                    first_seed.u16[ 0 ] = fs_initial_rand_buff[ 0 ];
                    first_seed.u16[ 1 ]  = fs_initial_rand_buff[ 1 ];
                    first_value = first_seed.i32;
#else
                    first_seed.u16[ 0 ]  = fs_initial_rand_buff[ 1 ];
                    first_seed.u16[ 1 ] = fs_initial_rand_buff[ 0 ];
                    first_value = first_seed.i32;
#endif
                    second_value = (kjb_int32)((kjb_int16)fs_initial_rand_buff[ 2 ]);

                    ERE(pso("Initial random number generator seed was %ld:%ld\n",
                            (long)first_value, (long)second_value));
                }
            }
        }
        else
        {
            char first_value_buff[ 100 ];
            char second_value_buff[ 100 ];
            char value_buff[ 100 ];
            char* value_pos;

            BUFF_CPY(value_buff, value);
            value_pos = value_buff;

            BUFF_GEN_GET_TOKEN(&value_pos, first_value_buff, ":");

            if (STRCMP_EQ(first_value_buff, "*"))
            {
                first_value = time((time_t *)NULL);
            }
            else
            {
                ERE(ss1i32(first_value_buff, &first_value));
            }

            BUFF_GEN_GET_TOKEN(&value_pos, second_value_buff, ":");

            if (STRCMP_EQ(second_value_buff, "*"))
            {
                second_value = time((time_t *)NULL);
            }
            else if (second_value_buff[ 0 ] != '\0')
            {
                ERE(ss1i32(second_value_buff, &second_value));

                if (ABS_OF(second_value) > INT16_MAX)
                {
                    set_error("Magnitude of second seed commponant must be at most %d.",
                              INT16_MAX);
                    return ERROR;
                }
            }
            else
            {
                second_value = 0;
            }

            kjb_seed_rand(first_value, second_value);
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "seed-2")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_first_call_to_kjb_rand_2)
            {
                ERE(pso("seed-2 = <not set>\n"));
            }
            else if (fs_initial_seed_status_2 == MULTIPLY_SEEDED)
            {
                ERE(pso("seed-2 = <multiply seeded>\n"));
            }
            else
            {
                ERE(pso("seed-2 = %ld\n", fs_initial_seed_2));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_first_call_to_kjb_rand_2)
            {
                ERE(pso("Random number generator 2 has not been used.\n"));
            }
            else if (fs_initial_seed_status_2 == MULTIPLY_SEEDED)
            {
                ERE(pso("Random number generator 2 was seeded "));
                ERE(pso("multiple times.\n"));
            }
            else
            {
                ERE(pso("Initial random number generator 2 seed was %ld\n",
                        fs_initial_seed_2));
            }
        }
        else
        {
            if (STRCMP_EQ(value, "*"))
            {
                seed = time((time_t *)NULL);
            }
            else
            {
                ERE(ss1l(value, &seed));
            }
            kjb_seed_rand_2(seed);
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_rand
 *
 * Returns a random double between 0 and 1.
 *
 * This routine returns a random double between 0 and 1 using erand48(). The
 * seeding of the random number generator is normally exposed to the user
 * through the option "seed". It may also be seeded using the routine
 * kjb_seed_rand(). If the generator is not seeded, then a default seed is used
 * (zeros).
 *
 * The form of the seed option value is <value_1>:<value_2>, where value_1 and
 * value_2 are either integers, or the special value "*", which gets converted
 * to the time of day as returned by the routine time(NULL).
 *
 * This routine sets up a different stream of random numbers from kjb_rand_2. In
 * general, the library code uses kjb_rand_2, with kjb_rand being the normal
 * choice for higher level code. This random number generator is restartable
 * from the seed value obtained by get_rand_seed().
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

double kjb_rand(void)
{
    ASSERT(kjb_rand_function);
    return (*kjb_rand_function)();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_rand_st
 *
 * Implementation of kjb_rand for single-threaded runs.
 *
 * Do not call this function unless you really know what you are doing.
 * Normally you should call kjb_rand() instead.
 * This function implements a random number generator that relies on erand48
 * and static seeding.  It can have undefined behavior in multi-threaded
 * programs, unless calls are serialized.
 * This function IS called by the library's wrapper on pthreads, thus it is
 * not static.
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifndef NeXT

/* NOT next */ double kjb_rand_st(void)
/* NOT next */ {
/* NOT next */     double erand_res;

#ifdef SUN4

/* SUN4 */         double erand48(kjb_uint16[ 3 ]);

#endif

/* NOT next */
/* NOT next */     if (fs_first_call_to_kjb_rand)
/* NOT next */     {
/* NOT next */          fs_initial_rand_buff[ 0 ] = fs_rand_buff[ 0 ];
/* NOT next */          fs_initial_rand_buff[ 1 ] = fs_rand_buff[ 1 ];
/* NOT next */          fs_initial_rand_buff[ 2 ] = fs_rand_buff[ 2 ];
/* NOT next */
/* NOT next */          fs_first_call_to_kjb_rand = FALSE;
/* NOT next */     }
/* NOT next */
/* NOT next */     erand_res = erand48(fs_rand_buff);
/* NOT next */
/* NOT next */     return erand_res;
/* NOT next */ }

#else

/* next */ double kjb_rand_st(void)
/* next */ {
/* next */
/* next */
/* next */
/* next */     set_bug("kjb_rand needs to be updated.\n");
/* next */     return ERROR;
/* next */ }

#endif
#else    /*  Case not UNIX follows */
#ifdef MS_16_BIT_OS

/* MS_16_BIT_OS */ double kjb_rand_st(long seed)      /* CHECK */
/* MS_16_BIT_OS */ {
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     set_bug("kjb_rand needs to be updated.\n");
/* MS_16_BIT_OS */     return ERROR;
/* MS_16_BIT_OS */ }

#endif
#endif      /*   #ifdef UNIX ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_seed_rand_with_tod
 *
 * Sets seed for kjb_rand using the current time
 *
 * This routine sets the seed for kjb_rand with the current time.  Normally one
 * wants to either control the random number seed, or use use the default value
 * of zero. If a semi-random seed is used, then reproducing the results will be
 * very hard! However, if the program has to do something different on each
 * invocation, then seeding with the time of day is one way to acomplish this.
 *
 * Note:
 *     Since the user is normally exposed to the seeding options, this routine
 *     is rarely used by external modules.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_seed_rand_with_tod(void)
{
    kjb_int32 first_value;
    kjb_int32 second_value;

    first_value = time((time_t *)NULL);
    second_value = time((time_t *)NULL);

    kjb_seed_rand(first_value, second_value);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_seed_rand
 *
 * Sets seed for kjb_rand.
 *
 * This routine sets the seed for kjb_rand. See kjb_seed_rand_with_3_short for
 * an alternate interface. If neither of these functions are not used, then
 * kjb_rand is seeded with 0 at the first invocation, and it is not seeded
 * thereafter. However, an interface to this routine is exposed to the user
 * through the option "seed". Thus this routine is not normally used.
 *
 * The first seed value is used in its entirety as the first part
 * of a 48 bit seed. The two LSB of the second seed value are used for the
 * remaining 16 bits.
 *
 * Note:
 *     Since the user is normally exposed to the seeding options, this routine
 *     is rarely used by external modules.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_seed_rand(kjb_int32 first_seed_value, kjb_int32 second_seed_value)
{
    Seed_union first_seed, second_seed;

    first_seed.i32 = first_seed_value;
    second_seed.i32 = second_seed_value;


#ifdef MSB_FIRST
    fs_rand_buff[ 0 ] = first_seed.u16[ 0 ];
    fs_rand_buff[ 1 ] = first_seed.u16[ 1 ];
    fs_rand_buff[ 2 ] = second_seed.u16[ 1 ];
#else
    fs_rand_buff[ 0 ] = first_seed.u16[ 1 ];
    fs_rand_buff[ 1 ] = first_seed.u16[ 0 ];
    fs_rand_buff[ 2 ] = second_seed.u16[ 0 ];
#endif

    if (fs_first_call_to_kjb_rand)
    {
        fs_initial_seed_status = EXTERNALLY_SEEDED;
    }
    else
    {
        fs_initial_seed_status = MULTIPLY_SEEDED;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        kjb_seed_rand_with_3_short
 *
 * Sets seed for kjb_rand.
 *
 * This routine sets the seed for kjb_rand. It is an alternated inteface to the
 * seeding process than kjb_seed_rand. If neither of these two function are not
 * used, then kjb_rand is seeded with 0 at the first invocation, and it is not
 * seeded thereafter. However, an interface to this routine is exposed to the
 * user through the option "seed". Thus this routine is not normally used.
 *
 * Note:
 *     Since the user is normally exposed to the seeding options, this routine
 *     is rarely used by external modules.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_seed_rand_with_3_short(kjb_uint16 *buff)
{
    fs_rand_buff[ 0 ] = buff[ 0 ];
    fs_rand_buff[ 1 ] = buff[ 1 ];
    fs_rand_buff[ 2 ] = buff[ 2 ];

    if (fs_first_call_to_kjb_rand)
    {
        fs_initial_seed_status = EXTERNALLY_SEEDED;
    }
    else
    {
        fs_initial_seed_status = MULTIPLY_SEEDED;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_rand_seed
 *
 * Provides the current seed used by kjb_rand
 *
 * This routine provides the current seed used by kjb_rand, normally for the
 * purposes of restarting the sequence at some point.
 *
 * A user level interface to kjb_seed_rand and get_rand_seed is provided
 * throught the suite of set routines.
 *
 * Returns:
 *     If the random number generator has not been seeded, then this routine
 *     returns ERROR. Otherwise, the seed is copied into the 48 bit buffer.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

int get_rand_seed(kjb_uint16 *buff)
{


#if 0 /* ifdef HOW_IT_WAS */
    if (fs_first_call_to_kjb_rand)
    {
        set_error("Random generator has not been seeded.");
        return ERROR;
    }
    else
    {
        buff[ 0 ] = fs_rand_buff[ 0 ];
        buff[ 1 ] = fs_rand_buff[ 1 ];
        buff[ 2 ] = fs_rand_buff[ 2 ];

        return NO_ERROR;
    }
#else
    buff[ 0 ] = fs_rand_buff[ 0 ];
    buff[ 1 ] = fs_rand_buff[ 1 ];
    buff[ 2 ] = fs_rand_buff[ 2 ];

    return NO_ERROR;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_rand_2
 *
 * Returns a random double between 0 and 1.
 *
 * This routine returns a random double between 0 and 1 using drand48(). It
 * provides a second stream of random numbers, separate from kjb_rand. If no
 * other arrangements have been made, then the random number generator is
 * seeded with zero. The generator may be seeded at any point by the user
 * throught the option "seed_2". The seed so specified may be a number, or the
 * special value "*" which requests that the generator be seed by the time of
 * day.
 *
 * Note:
 *     Normally, the library routines use kjb_rand_2, and the higher level
 *     routines use kjb_rand. Both random number streams can be seeded, but
 *     only kjb_rand can be restarted.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

double kjb_rand_2()
{
    return (*kjb_rand_2_function)();
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_rand_2_st
 *
 * Implementation of kjb_rand_2 for single-threaded runs.
 *
 * Do not call this function unless you really know what you are doing.
 * Internal library functions should call kjb_rand_2() instead.
 * This function IS called by the library's wrapper on pthreads, thus it is
 * not static.
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/
#ifdef UNIX
#ifndef NeXT

/* NOT next */ double kjb_rand_2_st(void)
/* NOT next */ {
/* NOT next */     double drand_res;
#ifdef SUN4
/* SUN4 */         double drand48(void);
/* SUN4 */         void   srand48(long);
#endif
/* NOT next */
/* NOT next */     if (fs_first_call_to_kjb_rand_2)
/* NOT next */     {
/* NOT next */         if (fs_initial_seed_status_2 == SEEDED_BY_DEFAULT)
/* NOT next */         {
/* NOT next */             srand48(0);
/* NOT next */             fs_initial_seed_2 = 0;
/* NOT next */         }
/* NOT next */
/* NOT next */         fs_first_call_to_kjb_rand_2 = FALSE;
/* NOT next */     }
/* NOT next */
/* NOT next */     drand_res = drand48();
/* NOT next */
/* NOT next */     return drand_res;
/* NOT next */ }

#else

/* next */ double kjb_rand_2_st(void)
/* next */ {
/* next */     long   rand_res;
/* next */     double d_rand_res;
/* next */
/* next */
/* next */     UNTESTED_CODE();
/* next */
/* next */     if (fs_first_call_to_kjb_rand_2)
/* next */     {
/* next */         if (fs_initial_seed_status_2 == SEEDED_BY_DEFAULT)
/* next */         {
/* next */             srandom(0);
/* next */             fs_initial_seed_2 = 0;
/* next */         }
/* next */
/* next */         fs_first_call_to_kjb_rand_2 = FALSE;
/* next */     }
/* next */
/* next */     rand_res = random();
/* next */
/* next */     d_rand_res = rand_res;
/* next */
/* next */     return d_rand_res / ((double) 0x80000000);
/* next */ }

#endif
#else    /*  Case not UNIX follows */
#ifdef MS_16_BIT_OS

/* MS_16_BIT_OS */ double kjb_rand_2_st(long seed)      /* CHECK */
/* MS_16_BIT_OS */ {
/* MS_16_BIT_OS */     double drand_res;
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     UNTESTED_CODE();
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     if (fs_first_call_to_kjb_rand_2)
/* MS_16_BIT_OS */     {
/* MS_16_BIT_OS */         if (fs_initial_seed_status_2 == SEEDED_BY_DEFAULT)
/* MS_16_BIT_OS */         {
/* MS_16_BIT_OS */             srand(0);
/* MS_16_BIT_OS */             fs_initial_seed_2 = 0;
/* MS_16_BIT_OS */         }
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */         fs_first_call_to_kjb_rand_2 = FALSE;
/* MS_16_BIT_OS */     }
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     drand_res = (double)rand();
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     return (drand_res / 0x7fff);
/* MS_16_BIT_OS */ }

#endif
#endif      /*   #ifdef UNIX ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_seed_rand_2_with_tod
 *
 * Sets seed for kjb_rand_2 using the current time
 *
 * This routine sets the seed for kjb_rand_2 with the current time. Normally one
 * wants to either control the random number seed, or use use the default value
 * of zero. If a semi-random seed is used, then reproducing the results will be
 * very hard! However, if the program has to do something different on each
 * invocation, then seeding with the time of day is one way to acomplish this.
 *
 * Note:
 *     Since the user is normally exposed to the seeding options, this routine
 *     is rarely used by external modules.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_seed_rand_2_with_tod(void)
{
    long seed_value;

    seed_value = time((time_t *)NULL);

    kjb_seed_rand_2(seed_value);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_seed_rand_2
 *
 * Sets seed for kjb_rand_2.
 *
 * This routine sets the seed for kjb_rand_2. If this function is not used,
 * then kjb_rand_2 is seeded with the default value of zero at the first
 * invocation, and it is not seeded thereafter. An interface to this routine is
 * exposed to the user through the option "seed-2".
 *
 * Note:
 *     Since the user is normally exposed to the seeding options, this routine
 *     is rarely used by external modules.
 *
 * Index: random, standard library
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifndef NeXT

/* NOT next */ void kjb_seed_rand_2(long seed)
/* NOT next */
/* NOT next */ {
#ifdef SUN4
/* SUN4 */         void srand48(long);
#endif
/* NOT next */
/* NOT next */     if (fs_first_call_to_kjb_rand)
/* NOT next */     {
/* NOT next */         fs_initial_seed_2 = seed;
/* NOT next */         fs_initial_seed_status_2 = EXTERNALLY_SEEDED;
/* NOT next */     }
/* NOT next */     else
/* NOT next */     {
/* NOT next */         fs_initial_seed_status_2 = MULTIPLY_SEEDED;
/* NOT next */     }
/* NOT next */
/* NOT next */     srand48(seed);
/* NOT next */ }

#else

/* next */ void kjb_seed_rand_2(long seed)
/* next */ {
/* next */     UNTESTED_CODE();
/* next */
/* next */
/* next */     if (fs_first_call_to_kjb_rand)
/* next */     {
/* next */         fs_initial_seed_2 = seed;
/* next */         fs_initial_seed_status_2 = EXTERNALLY_SEEDED;
/* next */     }
/* next */     else
/* next */     {
/* next */         fs_initial_seed_status_2 = MULTIPLY_SEEDED;
/* next */     }
/* next */
/* next */     srandom(seed);
/* next */ }

#endif
#else    /*  Case not UNIX follows */
#ifdef MS_16_BIT_OS

/* MS_16_BIT_OS */ void kjb_seed_rand_2(long seed)      /* CHECK */
/* MS_16_BIT_OS */ {
/* MS_16_BIT_OS */     unsigned int actual_seed;
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     UNTESTED_CODE();
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     /* Loose bits here!!! */
/* MS_16_BIT_OS */     actual_seed = *((unsigned int*)&seed);
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     if (fs_first_call_to_kjb_rand)
/* MS_16_BIT_OS */     {
/* MS_16_BIT_OS */         fs_initial_seed_2 = actual_seed;
/* MS_16_BIT_OS */         fs_initial_seed_status_2 = EXTERNALLY_SEEDED;
/* MS_16_BIT_OS */     }
/* MS_16_BIT_OS */     else
/* MS_16_BIT_OS */     {
/* MS_16_BIT_OS */         fs_initial_seed_status_2 = MULTIPLY_SEEDED;
/* MS_16_BIT_OS */     }
/* MS_16_BIT_OS */
/* MS_16_BIT_OS */     srand((unsigned int)seed);
/* MS_16_BIT_OS */ }

#endif
#endif      /*   #ifdef UNIX ... #else ...   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_set_rand_function(double (*f)(void)) 
{
    NRE(f);
    kjb_rand_function = f;
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_set_rand_2_function(double (*f)(void)) 
{
    NRE(f);
    kjb_rand_2_function = f;
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

