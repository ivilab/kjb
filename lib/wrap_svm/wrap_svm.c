
/* $Id: wrap_svm.c 6041 2010-06-28 22:16:07Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2005-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Ranjini Swaminathan
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */


#include "m/m_incl.h"

#include "wrap_svm/wrap_svm.h"
#include "wrap_svm/wrap_svm_libsvm.h"
#include "wrap_svm/wrap_svm_svmlight.h"


/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_SVM_LEARN_OPTIONS 52 /*26 options and a value for each option*/
#define MAX_SVM_CLASSIFY_OPTIONS 4

/* -------------------------------------------------------------------------- */

/*
 * Kobus: Use this to test without KJB_HAVE_LIBSVM
#undef KJB_HAVE_LIBSVM
*/

/* This table must be updated in parallel with fs_svm_prediction_methods. */
static Method_option fs_svm_computation_methods[ ] =
{
#ifdef KJB_HAVE_LIBSVM
    { "libsvm",   "libsvm",   (int(*)())do_libsvm_svm_computation},
#endif
    { "svmlight", "svmlight", (int(*)())do_svmlight_svm_computation}

};

static const int fs_num_svm_computation_methods =
                                sizeof(fs_svm_computation_methods) /
                                          sizeof(fs_svm_computation_methods[ 0 ]);

/* First one in the above table is the default one. */
static int         fs_svm_computation_method                  = 0;
static const char* fs_svm_computation_method_option_short_str =
                                                       "svm-computation";
static const char* fs_svm_computation_method_option_long_str =
                                                      "svm-computation-method";

/* This table must be updated in parallel with fs_svm_computation_methods. */
static Method_option fs_svm_prediction_methods[ ] =
{
#ifdef KJB_HAVE_LIBSVM
    { "libsvm",   "libsvm",   (int(*)())do_libsvm_svm_prediction},
#endif
    { "svmlight", "svmlight", (int(*)())do_svmlight_svm_prediction}

};

static const int fs_num_svm_prediction_methods =
                                sizeof(fs_svm_prediction_methods) /
                                          sizeof(fs_svm_prediction_methods[ 0 ]);
/* First one in the above table is the default one. */
static int         fs_svm_prediction_method                  = 0;
static const char* fs_svm_prediction_method_option_short_str =
                                                       "svm-prediction";
static const char* fs_svm_prediction_method_option_long_str =
                                                      "svm-prediction-method";



/* -------------------------------------------------------------------------- */

int set_svm_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           fs_svm_computation_method_option_short_str)
          || match_pattern(lc_option,
                           fs_svm_computation_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_svm_computation_methods,
                                fs_num_svm_computation_methods,
                                fs_svm_computation_method_option_long_str,
                                "SVM computation method", value,
                                &fs_svm_computation_method));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           fs_svm_prediction_method_option_short_str)
          || match_pattern(lc_option,
                           fs_svm_prediction_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_svm_prediction_methods,
                                fs_num_svm_prediction_methods,
                                fs_svm_prediction_method_option_long_str,
                                "SVM prediction  method", value,
                                &fs_svm_prediction_method));
        result = NO_ERROR;
    }



    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              compute_svm
 *
 * Builds a Support Vector Machine
 *
 * This routine builds a Support Vector Machine using one of possibly several
 * wrapped methods under the control of user supplied options. The default is to
 * use libsvm if it is available. This KJB library interface to support vector
 * machine code is file based. It is up to the user to be consistent regarding
 * building and predicting as this is not currently validated.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if the routine used build the SVM fails.
 *
 * Author: Ranjini Swaminathan and Kobus Barnard
 *
 * Documentor: Ranjini Swaminathan and Kobus Barnard
 *
 * Index: SVM
 *
 * -----------------------------------------------------------------------------
 */

/*  Kobus, 06-03-11:
 *      If we tell the user that this routine "builds" and SVM, then perhaps it
 *      should be called "build_svm". Also, strings that are not changed should
 *      be declared as const.
*/

int compute_svm(const char* training_file_name, const char * model_file_name,
                const char *scale_path)
{
    int result;



    if (    (fs_svm_computation_method < 0)
         || (fs_svm_computation_method >= fs_num_svm_computation_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else
    {
        int (*svm_computation_fn) (const char *, const char *,const char *)
        /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*) (const char *, const char *, const char *)) fs_svm_computation_methods[ fs_svm_computation_method ].fn;

        result = svm_computation_fn(training_file_name, model_file_name,scale_path);

        if (result == ERROR)
        {
            add_error("Error occured while computing SVM.");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              predict_with_svm
 *
 * Predicts a Support Vector Machine
 *
 * This routine predicts with a support vector machine built with one of
 * possibly several wrapped methods under the control of user supplied options.
 * The default is to use libsvm if it is available. This KJB library interface
 * to support vector machine code is file based. It is up to the user to be
 * consistent regarding building and predicting as this is not currently
 * validated.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if the wrapped prediction function fails.
 *
 * Author: Ranjini Swaminathan and Kobus Barnard
 *
 * Documentor: Ranjini Swaminathan and Kobus Barnard
 *
 * Index: svm
 *
 * -----------------------------------------------------------------------------
 */

int predict_with_svm(const char* input_file_name, const char * model_file_name,
                     const char * prediction_file_name, const char *scale_path)
{
    int result;



    if (    (fs_svm_prediction_method < 0)
         || (fs_svm_prediction_method >= fs_num_svm_prediction_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else
    {
        int (*svm_prediction_fn) (const char *, const char *, const char *, const char *)
        /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*) (const char *, const char *, const char *, const char *))fs_svm_prediction_methods[ fs_svm_prediction_method ].fn;

        result = svm_prediction_fn(input_file_name, model_file_name, prediction_file_name,scale_path);

        if (result == ERROR)
        {
            add_error("Error occured while predicting using SVM.");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
}
#endif



