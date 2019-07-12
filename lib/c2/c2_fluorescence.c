
/* $Id: c2_fluorescence.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "c/c_gen.h"     /* Only safe as first include in a ".c" file. */
#include "c/c_sensor.h"
#include "s2/s2_fluorescence.h"
#include "c2/c2_fluorescence.h"

/*
// The routine here were moved from c_sensor.c in order to avoid linking in the
// SLATEC library (used to determine the flourescent response).
*/

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                         generate_fluorescent_RGB_data
 *
 * Generates fluorescent RGB data.
 *
 * This routine generates RGB data from a fluorescent database and illuminant
 * spectra using the sensors in sensor_sp, or, if sensor_sp is NULL, the ones
 * managed by the KJB library. If KJB library options are made available to the
 * user, then the sensors used can be changed using the "sensor-file" or "sf"
 * option. If the user has not specified sensors, then default ones are used.
 *
 * The resulting RGB are put into the matrix *target_mpp which is created if
 * it is NULL, and resized if it is the wrong size.
 *
 * The "index" argument is used to specify a specific illuminat index in
 * illum_sp. If index is set to USE_ALL_SPECTRA then all combinations of
 * illuminants and fluorescent reflectances are used.
 *
 * Returns :
 *    If the sensors are supplied, then this routine returns NO_ERROR on
 *    success, and ERROR on failure.  If the KJB library sensors are used, then
 *    this routine returns the sensor version number on success, and ERROR on
 *    failure, with an appropriate error message being set. The sensor version
 *    number is incremented every time the use user resets the sensor data
 *    files, and thus can be used to verify that cached data is still OK with
 *    sensor_data_is_up_to_date().
 *
 * Index:
 *    spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int generate_fluorescent_RGB_data
(
    Matrix**              target_mpp,
    Spectra*              illum_sp,
    int                   index,
    Fluorescent_database* fl_db_ptr,
    Spectra*              sensor_sp
)
{
    int      result;
    Spectra* response_sp = NULL;


    if ((illum_sp == NULL) || (fl_db_ptr == NULL))
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_fluorescent_response_spectra(&response_sp, fl_db_ptr, illum_sp,
                                         index));

    result = get_RGB_data_from_spectra(target_mpp, response_sp, sensor_sp);

    free_spectra(response_sp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

