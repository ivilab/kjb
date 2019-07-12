
/* $Id: l_error.h 22177 2018-07-14 18:38:32Z kobus $ */

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

#ifndef L_ERROR_INCLUDED
#define L_ERROR_INCLUDED


#include "l/l_def.h"
#include "l/l_global.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* =============================================================================
 *                             NOTE_ERROR
 *
 * (MACRO) Adds a line to the error message store
 *
 * If the debug level is greater than 0, this macro addds an error to the list
 * of lines stored in the current error list with add_error(); The line is
 * simply:
 * |    Error noted on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NOTE_ERROR(void);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line (only).
                      I have no idea what the problem is. */

#define NOTE_ERROR() \
             {                                                         \
                  /* extern int kjb_debug_level; */                    \
                                                                       \
                  if (kjb_debug_level > 0)                             \
                  {                                                    \
                      add_error("Error noted on line %d of %s.",       \
                                __LINE__, __FILE__);                   \
                  }                                                    \
              }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             EGC
 *
 * (MACRO) Goto to "cleanup" label if argument is ERROR
 *
 * This macro jumps to a label names "cleanup" if its argument is ERROR.  It is
 * convenient when ERE would cause memory leaks by not freeing memory allocated
 * earlier in the function.  Below is a usage example.
 *
 * |    int my_func()
 * |    {
 * |         Error err = NO_ERROR;
 * |         Matrix* m = NULL
 * |         Matrix* m2 = NULL
 * |         ERE(get_target_matrix(&m, 5, 5));
 * |         EGC(err = get_target_matrix(&m2, 3, 3));
 * |         ...
 * |     cleanup:
 * |         free_matrix(m);
 * |         free_matrix(m2);
 * |         return err;
 * |    }
 *
 * Notice that the first call uses ERE, since there's nothing to free.  The 
 * second call needs to use EGC, since m needs to be freed before 
 * returning.  Also notice that you need to set err inside EGC, so the error
 * is returned at the end of the function.
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (EGC on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    int EGC(int);
#else

#define EGC(x)    do                                                          \
                  {                                                           \
                      if ((x) != 0)                                            \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EGC on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          goto cleanup;                                        \
                      }                                                        \
                  }                                                           \
                  while(0)

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ERE
 *
 * (MACRO) Returns ERROR if its argument is ERROR.
 *
 * This macro becomes an ERROR return if its argument is ERROR. It is a
 * convenient quick return that makes sense in the KJB library, but be careful
 * about cleanup! Often code with ERE()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (ERE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    int ERE(int);
#else

#define ERE(x)        if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(ERE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return ERROR;                                        \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ERN
 *
 * (MACRO) Returns NULL if its argument is ERROR.
 *
 * This macro becomes a NULL return if its argument is ERROR. It is a convenient
 * quick return that makes sense in the KJB library, but be careful about
 * cleanup! Often code with ERN()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (ERN on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void* ERN(int);
#else

#define ERN(x)        if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(ERN on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return NULL;                                         \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ER
 *
 * (MACRO) Returns (void) if its argument is ERROR.
 *
 * This macro becomes a simple return if its argument is ERROR. It is a
 * convenient quick return that makes sense in the KJB library, but be careful
 * about cleanup! Often code with ERE()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (ER on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void ER(int);
#else

#define ER(x)         if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(ER on line %d of %s.)",              \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return;                                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             EPE
 *
 * (MACRO) Prints ERROR if its argument is ERROR.
 *
 * This macro prints the if its argument for ERROR.  
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (EPE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void EPE(int);
#else

#define EPE(x)        if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EPE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ETE
 *
 * (MACRO) Terminates execution if its argument is ERROR.
 *
 * This macro terminates execution with EXIT_FAILURE if its argument is ERROR.
 * More often, one wants to also print the error message which is doen with
 * EPETE();
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (ETE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void ETE(int);
#else

#define ETE(x)        if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(ETE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_exit(EXIT_FAILURE);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             EPETE
 *
 * (MACRO) Prints error and terminates execution if its argument is ERROR.
 *
 * This macro prints the error message and terminates execution with
 * EXIT_FAILURE if its argument is ERROR. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (EPETE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void EPETE(int);
#else

#define EPETE(x)      if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EPETE on line %d of %s.)",           \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                          kjb_exit(EXIT_FAILURE);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             EPETB
 *
 * (MACRO) Prints error and terminates execution if its argument is ERROR.
 *
 * This macro prints the error message and terminates execution with return code
 * EXIT_BUG if its argument is ERROR.
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (EPETB on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void EPETE(int);
#else

#define EPETB(x)      if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EPETB on line %d of %s.)",           \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                          kjb_exit(EXIT_BUG);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ESBRE
 *
 * (MACRO) Set bug and return ERROR, if its argument is ERROR.
 *
 * This macro sets the bug flag (see set_bug(3)) and returns ERROR if the
 * macro's argument has value ERROR.  In addition, it prints the current error
 * message.
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    int ESBRE(int);
#else

#define ESBRE(x)      if ((x) == ERROR)                                        \
                      {                                                        \
                          kjb_print_error();                                   \
                          set_bug("Treating error return on line %d "          \
                                  "of %s as a bug.)", __LINE__, __FILE__);     \
                          return ERROR;                                        \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BATCH_EPETE
 *
 * (MACRO) Prints error and maybe terminates execution if argument is ERROR.
 *
 * When running interactively, this macro behaves identically to EPETE.
 * Otherwise, this macro behaves like EPE.
 *
 * We might wrap a function call with BATCH_EPETE so that the program will
 * terminate right away if someone is likely to be watching the console for
 * error messages; the program says, "Since you're right here, could you help me
 * resolve this?" But if no one is interacting, then the program does not give
 * up; rather it prints the error and tries to carry on.  This might be
 * appropriate, for example, when processing a string of commands such that even
 * if one command fails, still we want the remaining commands to be processed as
 * given.
 *
 * Related:
 *      EPETE, EPE, is_interactive
 *
 * Documenter: Andrew Predoehl
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    int BATCH_EPETE(int);
#else

#define BATCH_EPETE(x) \
                      if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(BATCH_EPETE on line %d of %s.)",     \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                                                                               \
                          if ( ! is_interactive())                             \
                          {                                                    \
                              kjb_exit(EXIT_FAILURE);                          \
                          }                                                    \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NRE
 *
 * (MACRO) Returns ERROR if its argument is NULL.
 *
 * This macro becomes an ERROR return if its argument is NULL. It is a
 * convenient quick return that makes sense in the KJB library, but be careful
 * about cleanup! Often code with NRE()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NRE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    int NRE(int);
#else

#define NRE(x)        if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NRE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return ERROR;                                        \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             NGC
 *
 * (MACRO) Jump to "cleanup" label if argument equals NULL
 *
 * This macro tests whether its argument equals NULL, and if so it performs a
 * goto to the label "cleanup," similarly to macro EGC.
 * See the docs of EGC for more discussion on how to use a cleanup clause.
 *
 * Example usage:
 * |int some_function()
 * |{
 * |     int result = ERROR;
 * |     struct Widget *p, *q, *r;
 * |     p = q = r = NULL;
 * |     NGC(p = N_TYPE_MALLOC(struct Widget, 10));
 * |     NGC(q = N_TYPE_MALLOC(struct Widget, 10));
 * |     NGC(r = N_TYPE_MALLOC(struct Widget, 10));
 * |     (* . . . work with those arrays . . . *)
 * |     result = NO_ERROR;
 * |cleanup:
 * |     kjb_free(r);
 * |     kjb_free(q);
 * |     kjb_free(p);
 * |     return result;
 * |}
 *
 * Further, if the debug level is greater than 0, then a message is added to
 * the list of messages using add_errror().
 *
 * Author: Andrew Predoehl
 *
 * Documenter: Andrew Predoehl
 *
 * Index: debugging, error handling
 *
 * ----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NGC(void*);
#else

#define NGC(x)   do                                                           \
                 {                                                            \
                     if (NULL == (x))                                         \
                     {                                                        \
                         if (kjb_debug_level > 0)                             \
                         {                                                    \
                             add_error("(NGC on line %d of %s.)",             \
                                        __LINE__, __FILE__);                  \
                         }                                                    \
                         goto cleanup;                                        \
                     }                                                        \
                 }                                                            \
                 while(0)

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NRN
 *
 * (MACRO) Returns NULL if its argument is NULL.
 *
 * This macro becomes a NULL return if its argument is ERROR. It is a convenient
 * quick return that makes sense in the KJB library, but be careful about
 * cleanup! Often code with NRN()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NRN on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void* NRN(int);
#else

#define NRN(x)        if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NRN on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return NULL;                                         \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NR
 *
 * (MACRO) Returns (void) if its argument is NULL.
 *
 * This macro becomes a simple return if its argument is NULL. It is a
 * convenient quick return that makes sense in the KJB library, but be careful
 * about cleanup! Often code with NR()'s will leak memory on failure which is
 * acceptable if the failure is rare, but this should be documented. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NR on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NR(int);
#else

#define NR(x)         if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NR on line %d of %s.)",              \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          return;                                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NPE
 *
 * (MACRO) Prints ERROR if its argument is NULL.
 *
 * This macro prints the if its argument for NULL.  
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NPE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NPE(void*);
#else

#define NPE(x)        if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NPE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NTE
 *
 * (MACRO) Terminates execution if its argument is NULL.
 *
 * This macro terminates execution if its argument for ERROR. More often, one
 * wants to also print the error message which is doen with NPETE();
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NTE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NTE(int);
#else

#define NTE(x)        if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NTE on line %d of %s.)",             \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_exit(EXIT_FAILURE);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NPETE
 *
 * (MACRO) Prints error and terminates execution if its argument is NULL.
 *
 * This macro prints the error message and terminates execution with return code
 * EXIT_FAILURE if its argument is NULL. 
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NPETE on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NPETE(void*);
#else

#define NPETE(x)      if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NPETE on line %d of %s.)",           \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                          kjb_exit(EXIT_FAILURE);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             NPETB
 *
 * (MACRO) Prints error and terminates execution if its argument is NULL.
 *
 * This macro prints the error message and terminates execution with return code
 * EXIT_BUG if its argument is NULL.
 *
 * Further, if the debug level is greater than 0, then a message is added to the
 * list of messages using add_errror(); That line is:
 * |    (NPETB on line [line] of [file].
 *
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void NPETE(void*);
#else

#define NPETB(x)      if ((x) == NULL)                                         \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(NPETB on line %d of %s.)",           \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                          kjb_exit(EXIT_BUG);                              \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


void check_num_args(int, int, int, const char*);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

