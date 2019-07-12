
/* $Id: wrap_svm_svmlight.c 4727 2009-11-16 20:53:54Z kobus $ */

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



#include "l/l_gen.h"
#include "wrap_svm/wrap_svm.h"
#include "wrap_svm/wrap_svm_svmlight.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * Kobus, 06-03-11:
 *     Before we simply returned 0, which does not make any sense?
 *
 *     I also removed the comment block. If it is not implemented, we don't tell
 *     the users!
*/

int do_svmlight_svm_computation
(
    const char* dummy_training_file_name,
    const char* dummy_model_file_name,
    const char* dummy_scale_path
)
{
    set_error("SVM light is not currently supported.");
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Kobus, 06-03-11:
 *     Before we simply returned 0, which does not make any sense?
*/

int do_svmlight_svm_prediction
(
    const char* dummy_input_file_name,
    const char* dummy_model_file_name,
    const char* dummy_prediction_file_name,
    const char* dummy_scale_path
)
{
    set_error("SVM light is not currently supported.");
    return ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

