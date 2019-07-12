
/* $Id: c_sensor.c 15419 2013-09-25 18:12:40Z predoehl $ */

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

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef DATA_DIR
#    define DATA_DIR        "data"
#endif

#ifndef SENSOR_DIR
#    define SENSOR_DIR      "camera"
#endif

#ifndef SENSOR_FILE
#    define SENSOR_FILE     "sensors.spect"
#endif

#ifndef XYZ_DIR
#    define XYZ_DIR         "cie"
#endif

#ifndef XYZ_FILE
#    define XYZ_FILE        "XYZ.spect"
#endif

#define MIN_PROJECTION_DENOMINATOR  (0.0001)

/* -------------------------------------------------------------------------- */

static int fs_random_sensor_mapping_flag = FALSE;
static char fs_current_sensor_file[ MAX_FILE_NAME_SIZE ] = { '\0' };
static int  fs_sensor_version = 0;

/* Determined at first use. */
static Spectra *fs_rgb_original_sp = NULL;
static Spectra *fs_xyz_original_sp = NULL;

/* Derived from fs_rgb_original_sp as offset/step/num_intervals changes. */
static Spectra *fs_rgb_derived_sp  = NULL;
static Spectra *fs_xyz_derived_sp  = NULL;


static const double fs_xyz_response_data[ 3 ][ NUM_SPECTROMETER_INTERVALS ] =
{
      /* X */
   {
    0.0014, 0.0020, 0.0033, 0.0052, 0.0087, 0.0143, 0.0208, 0.0338, 0.0551,
    0.0868, 0.1344, 0.1985, 0.2590, 0.3048, 0.3344, 0.3483, 0.3494, 0.3418,
    0.3301, 0.3140, 0.2908, 0.2604, 0.2186, 0.1728, 0.1314, 0.0956, 0.0647,
    0.0411, 0.0242, 0.0122, 0.0049, 0.0022, 0.0052, 0.0155, 0.0348, 0.0633,
    0.0994, 0.1424, 0.1891, 0.2383, 0.2904, 0.3455, 0.4033, 0.4643, 0.5284,
    0.5945, 0.6616, 0.7288, 0.7948, 0.8579, 0.9163, 0.9673, 1.0090, 1.0410,
    1.0598, 1.0622, 1.0510, 1.0227, 0.9792, 0.9232, 0.8544, 0.7730, 0.6855,
    0.6011, 0.5227, 0.4479, 0.3775, 0.3130, 0.2561, 0.2071, 0.1649, 0.1292,
    0.0997, 0.0768, 0.0598, 0.0468, 0.0354, 0.0263, 0.0196, 0.0148, 0.0114,
    0.0087, 0.0066, 0.0051, 0.0038, 0.0029, 0.0030, 0.0020, 0.0011, 0.0010,
    0.0007, 0.0005, 0.0004, 0.0002, 0.0002, 0.0002, 0.0001, 0.0001, 0.0001,
    0.0000, 0.0000
   },
       /* Y */
   {
    0.0000, 0.0001, 0.0001, 0.0001, 0.0002, 0.0004, 0.0005, 0.0009, 0.0015,
    0.0025, 0.0040, 0.0065, 0.0098, 0.0136, 0.0180, 0.0230, 0.0284, 0.0345,
    0.0418, 0.0502, 0.0600, 0.0709, 0.0837, 0.0990, 0.1175, 0.1390, 0.1627,
    0.1912, 0.2268, 0.2701, 0.3230, 0.3893, 0.4635, 0.5444, 0.6295, 0.7100,
    0.7779, 0.8362, 0.8850, 0.9238, 0.9540, 0.9760, 0.9904, 0.9981, 0.9999,
    0.9950, 0.9827, 0.9639, 0.9385, 0.9070, 0.8700, 0.8276, 0.7812, 0.7324,
    0.6822, 0.6310, 0.5796, 0.5284, 0.4780, 0.4291, 0.3810, 0.3329, 0.2866,
    0.2449, 0.2082, 0.1750, 0.1451, 0.1188, 0.0962, 0.0771, 0.0610, 0.0476,
    0.0366, 0.0281, 0.0218, 0.0170, 0.0128, 0.0095, 0.0071, 0.0053, 0.0041,
    0.0031, 0.0024, 0.0018, 0.0014, 0.0010, 0.0007, 0.0006, 0.0005, 0.0004,
    0.0002, 0.0002, 0.0001, 0.0001, 0.0001, 0.0001, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000
   },
       /* Z */
   {
    0.0065, 0.0093, 0.0157, 0.0250, 0.0416, 0.0679, 0.0986, 0.1610, 0.2633,
    0.4153, 0.6456, 0.9590, 1.2590, 1.4944, 1.6565, 1.7471, 1.7806, 1.7789,
    1.7642, 1.7335, 1.6692, 1.5648, 1.3902, 1.1871, 0.9946, 0.8130, 0.6521,
    0.5203, 0.4163, 0.3348, 0.2720, 0.2234, 0.1793, 0.1383, 0.1039, 0.0782,
    0.0608, 0.0478, 0.0369, 0.0277, 0.0203, 0.0146, 0.0103, 0.0073, 0.0053,
    0.0039, 0.0029, 0.0023, 0.0019, 0.0018, 0.0017, 0.0015, 0.0012, 0.0011,
    0.0010, 0.0008, 0.0006, 0.0004, 0.0002, 0.0002, 0.0002, 0.0001, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000
   }
};

static const double fs_rgb_response_data[ 3 ][ NUM_SPECTROMETER_INTERVALS ] =
{
 /* Red */
   {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 126.9, 576.8, 1413.4,
    2613.6, 4098.5, 5758.4, 7469.3, 9104.2, 10545.2, 11694.8, 12483.6, 12867.7,
    12823.5, 12359.4, 11525.1, 10415.8, 9168.5, 7946.2, 6861.6, 5940.5, 5167.2,
    4516.5, 3962.8, 3484.2, 3065.9, 2700.5, 2385.3, 2119.2, 1898.0, 1711.1,
    1539.5, 1361.3, 1162.1, 941.9, 715.5, 505.4, 329.2, 193.9, 97.0, 33.2, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0
   },
/* Green */
   {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 56.5, 116.9, 160.1, 195.1, 256.7, 396.8, 675.7, 1155.4,
    1894.2, 2935.8, 4286.3, 5905.5, 7716.5, 9620.0, 11508.2, 13274.4, 14821.0,
    16064.9, 16943.2, 17417.9, 17480.9, 17155.9, 16496.0, 15565.0, 14397.3,
    12999.6, 11384.1, 9583.6, 7663.1, 5721.3, 3885.1, 2293.6, 1075.0, 312.8,
    1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0
   },
/* Blue */
   {
    0.0, 2831.4, 5708.8, 8656.9, 11677.9, 14754.1, 17851.4, 20926.2, 23936.3,
    26837.3, 29577.6, 32094.3, 34312.2, 36141.7, 37484.5, 38273.3, 38508.7,
    38218.5, 37429.8, 36167.0, 34453.0, 32311.0, 29769.8, 26871.9, 23680.5,
    20283.0, 16787.8, 13319.9, 10018.9, 7031.2, 4491.8, 2503.9, 1119.1, 320.4,
    10.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
   }
};

/* -------------------------------------------------------------------------- */

static int set_sensor_file(const char* file_name);
static int set_RGB_sensors(int count, double offset, double step);
static int set_XYZ_sensors(int count, double offset, double step);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_RGB_sensors(void);
    static void free_XYZ_sensors(void);
#endif

/* -------------------------------------------------------------------------- */

int set_sensor_options(const char *option, const char *value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "sf"))
         || (match_pattern(lc_option, "sensor-file"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else
        {
            result = set_sensor_file(value);
        }
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "random-sensor-map"))
         || (match_pattern(lc_option, "random-sensor-mapping"))
       )
    {
        int temp_boolean_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("Under construction\n"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Random sensor mapping %s used.\n",
                    fs_random_sensor_mapping_flag ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_random_sensor_mapping_flag = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_sensor_file(const char *file_name)
{
    int result = NO_ERROR;


    if (*file_name == '\0')
    {
        if (fs_current_sensor_file[ 0 ] == '\0')
        {
            ERE(pso("No sensor file has been set.\n"));
        }
        else
        {
            ERE(pso("Sensor file is %s.\n", fs_current_sensor_file));
        }
    }
    else if (*file_name == '?')
    {
        if (fs_current_sensor_file[ 0 ] == '\0')
        {
            ERE(pso("sensor-file = off\n"));
        }
        else
        {
            ERE(pso("sensor-file = %s\n", fs_current_sensor_file));
        }
    }
    else
    {
        Spectra* new_rgb_original_sp = NULL;
        char config_file_name[ MAX_FILE_NAME_SIZE ];
        char config_dir[ MAX_FILE_NAME_SIZE ];

        BUFF_CPY(config_dir, "data");
        BUFF_CAT(config_dir, DIR_STR);
        BUFF_CAT(config_dir, "sensors");


        ERE(get_config_file("SENSOR_FILE", config_dir, file_name,
                            "sensor file",
                            config_file_name, sizeof(config_file_name)));

        ERE(read_sensor_spectra(&new_rgb_original_sp, config_file_name));

#ifdef TRACK_MEMORY_ALLOCATION
        if (fs_rgb_original_sp == NULL)
        {
            add_cleanup_function(free_RGB_sensors);
        }
#endif

        result = copy_spectra(&fs_rgb_original_sp, new_rgb_original_sp);

        if ((result != ERROR) && (fs_random_sensor_mapping_flag))
        {
            Matrix* rand_map_mp = NULL;

            ERE(get_random_matrix(&rand_map_mp, 3, 3));

            /* Using the fact that fs_rgb_original_sp==fs_rgb_original_sp */
            ERE(multiply_matrices(&(fs_rgb_original_sp->spectra_mp), rand_map_mp,
                                  new_rgb_original_sp->spectra_mp));
            free_matrix(rand_map_mp);
        }

        free_spectra(new_rgb_original_sp);

        if (result != ERROR)
        {
            fs_sensor_version++;

            free_spectra(fs_rgb_derived_sp);
            fs_rgb_derived_sp = NULL;
            BUFF_REAL_PATH(config_file_name, fs_current_sensor_file);
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               sensor_data_is_up_to_date
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
 */

int sensor_data_is_up_to_date(int proposed_version)
{
    return (proposed_version == fs_sensor_version);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_sensor_version
 *
 * Returns the version number of the sensor
 *
 * This routine returns the version number of the sensor. This is so that
 * routines relying on results from this module can determine if cached results
 * are out of date due to the user re-setting the sensor file.
 *
 * Returns:
 *     The sensor version, which is 0 when no sensors have been set, and is
 *     incremented each time the sensors get changed.
 *
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
 */

int get_sensor_version(void)
{
    return fs_sensor_version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           normalize_illum_spectra
 *
 * Normalizes illumination spectra.
 *
 * This routine normalizes illumination spectra using the sensors in sensor_sp,
 * or, if sensor_sp is NULL, the ones managed by the KJB library. If KJB library
 * options are made available to the user, then the sensors used can be changed
 * using the "sensor-file" or "sf" option. If the user has not specified
 * sensors, then default ones are used.
 *
 * The illumination spectra are normalized so that the maximal response is set
 * to the parameter max_rgb which is often 255.0. If the value of max_rgb is
 * negative then normalization is not done. The routine can be used with any
 * kind of spectra, but since it really only makes sense (under foreseable
 * conditions) to perform this operation on illuminant spectra, the routine name
 * is chosen to reflect this.
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
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int normalize_illum_spectra
(
    Spectra**      normalized_illum_sp_ptr,
    const Spectra* illum_sp,
    const Spectra* sensor_sp,
    double         max_rgb
)
{

    ERE(copy_spectra(normalized_illum_sp_ptr, illum_sp));

    return ow_normalize_illum_spectra(*normalized_illum_sp_ptr, sensor_sp,
                                      max_rgb);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           ow_normalize_illum_spectra
 *
 * Normalizes illumination spectra.
 *
 * This routine normalizes illumination spectra using the sensors in sensor_sp,
 * or, if sensor_sp is NULL, the ones managed by the KJB library. If KJB library
 * options are made available to the user, then the sensors used can be changed
 * using the "sensor-file" or "sf" option. If the user has not specified
 * sensors, then default ones are used.
 *
 * The illumination spectra are normalized so that the maximal response is set
 * to the parameter max_rgb which is often 255.0. If the value of max_rgb is
 * negative then normalization is not done. The routine can be used with any
 * kind of spectra, but since it really only makes sense (under foreseable
 * conditions) to perform this operation on illuminant spectra, the routine name
 * is chosen to reflect this.
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
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int ow_normalize_illum_spectra
(
    Spectra*       illum_sp,
    const Spectra* sensor_sp,
    double         max_rgb
)
{
    int     result     = NO_ERROR;
    Matrix* rgb_mp     = NULL;
    Vector* max_rgb_vp = NULL;
    int version;


    version = get_RGB_data_from_spectra(&rgb_mp, illum_sp, sensor_sp);

    if (max_rgb >= 0.0)
    {
        result = version;

        if (result != ERROR)
        {
            result = get_max_matrix_row_elements(&max_rgb_vp, rgb_mp);
        }

        if (result != ERROR)
        {
            result = ow_divide_vector_by_scalar(max_rgb_vp, max_rgb);
        }

        if (result != ERROR)
        {
            result = ow_divide_matrix_by_col_vector(illum_sp->spectra_mp,
                                                     max_rgb_vp);
        }
    }

    free_matrix(rgb_mp);
    free_vector(max_rgb_vp);

    if (result == ERROR)
    {
        return result;
    }
    else
    {
        return version;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_RGB_sensors
 *
 *
 *
 * Index: sensors, colour, RGB colour
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_sensors
(
    Spectra** sp_ptr,
    int       num_intervals,
    double    offset,
    double    step
)
{
    int version;


    ERE(version = set_RGB_sensors(num_intervals, offset, step));
    ERE(copy_spectra(sp_ptr, fs_rgb_derived_sp));

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_spectrum_xy_locus
 *
 *
 *
 * Index: sensors, colour, xy chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_spectrum_xy_locus(Vector **x_vpp, Vector **y_vpp)
{
    double  X, Y, Z;
    double* x_pos;
    double* y_pos;
    int     count;
    int     i;


    ERE(set_XYZ_sensors(NUM_SPECTROMETER_INTERVALS, SPECTROMETER_OFFSET,
                        SPECTROMETER_STEP));

    NRE(*x_vpp = create_vector( NUM_SPECTROMETER_INTERVALS) );
    NRE(*y_vpp = create_vector( NUM_SPECTROMETER_INTERVALS) );

    x_pos = (*x_vpp)->elements;
    y_pos = (*y_vpp)->elements;
    count = 0;

    for (i=0; i<NUM_SPECTROMETER_INTERVALS; i++)
    {
        X = (fs_xyz_derived_sp->spectra_mp->elements)[ 0 ][i];
        Y = (fs_xyz_derived_sp->spectra_mp->elements)[ 1 ][i];
        Z = (fs_xyz_derived_sp->spectra_mp->elements)[ 2 ][i];

        if (X+Y+Z > .01)
        {
            *x_pos = X/(X+Y+Z);
            *y_pos = Y/(X+Y+Z);
            x_pos++;
            y_pos++;
            count++;
        }
    }

    if (count < 3)
    {
        set_error("Not enough points for spectrum locus.");
        return ERROR;
    }

    *x_pos = (*x_vpp)->elements[ 0 ];
    *y_pos = (*y_vpp)->elements[ 0 ];
    count++;

    (*x_vpp)->length = count;
    (*y_vpp)->length = count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_spectrum_uv_locus
 *
 *
 *
 * Index: sensors, colour, uv chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_spectrum_uv_locus(Vector **u_vpp, Vector **v_vpp)
{
    double  X, Y, Z;
    double* u_pos;
    double* v_pos;
    int     count;
    int     i;


    ERE(set_XYZ_sensors(NUM_SPECTROMETER_INTERVALS, SPECTROMETER_OFFSET,
                        SPECTROMETER_STEP));

    NRE(*u_vpp= create_vector( NUM_SPECTROMETER_INTERVALS) );
    NRE(*v_vpp = create_vector( NUM_SPECTROMETER_INTERVALS) );

    u_pos = (*u_vpp)->elements;
    v_pos = (*v_vpp)->elements;
    count = 0;

    for (i=0; i<NUM_SPECTROMETER_INTERVALS; i++)
    {
        X = (fs_xyz_derived_sp->spectra_mp->elements)[ 0 ][i];
        Y = (fs_xyz_derived_sp->spectra_mp->elements)[ 1 ][i];
        Z = (fs_xyz_derived_sp->spectra_mp->elements)[ 2 ][i];

        if ((X + (15.0*Y) + (3.0*Z)) > .05)
        {
            *u_pos = (4.0 * X)/(X + (15.0*Y) + (3.0*Z));
            *v_pos = (9.0 * Y)/(X + (15.0*Y) + (3.0*Z));
            u_pos++;
            v_pos++;
            count++;
        }
    }

    if (count < 3)
    {
        set_error("Not enough points for spectrum locus.");
        return ERROR;
    }

    *u_pos = (*u_vpp)->elements[ 0 ];
    *v_pos = (*v_vpp)->elements[ 0 ];
    count++;

    (*u_vpp)->length = count;
    (*v_vpp)->length = count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_spectrum_rg_locus
 *
 *
 *
 * Index: sensors, colour, rg chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_spectrum_rg_locus(Vector **r_vpp, Vector **g_vpp)
{
    double  R, G, B;
    double* r_pos;
    double* g_pos;
    int     count;
    int     i;
    int     version;


    ERE(version = set_RGB_sensors(NUM_SPECTROMETER_INTERVALS,
                                  SPECTROMETER_OFFSET,
                                  SPECTROMETER_STEP));

    NRE(*r_vpp= create_vector( NUM_SPECTROMETER_INTERVALS) );
    NRE(*g_vpp = create_vector( NUM_SPECTROMETER_INTERVALS) );

    r_pos = (*r_vpp)->elements;
    g_pos = (*g_vpp)->elements;
    count = 0;

    for (i=0; i<NUM_SPECTROMETER_INTERVALS; i++)
    {
        R = (fs_rgb_derived_sp->spectra_mp->elements)[ 0 ][ i ];
        G = (fs_rgb_derived_sp->spectra_mp->elements)[ 1 ][ i ];
        B = (fs_rgb_derived_sp->spectra_mp->elements)[ 2 ][ i ];

        if (R+G+B > 2.0)
        {
            *r_pos = R/(R+G+B);
            *g_pos = G/(R+G+B);

            r_pos++;
            g_pos++;
            count++;
        }
    }

    if (count < 3)
    {
        set_error("Not enough points for spectrum locus.");
        return ERROR;
    }

    *r_pos = (*r_vpp)->elements[ 0 ];
    *g_pos = (*g_vpp)->elements[ 0 ];
    count++;

    (*r_vpp)->length = count;
    (*g_vpp)->length = count;

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_xy_from_spectrum
 *
 *
 *
 * Index: sensors, colour, xy chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_xy_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* x_ptr,
    double* y_ptr
)
{
    double X, Y, Z;


    ERE(get_XYZ_from_spectrum(spectrum_vp, offset, step, &X, &Y, &Z));

    ERE(get_xy_from_XYZ(X, Y, Z, x_ptr, y_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_xy_from_XYZ
 *
 * Maps XYZ to xy
 *
 * Index: sensors, colour, xy chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_xy_from_XYZ
(
    double  X,
    double  Y,
    double  Z,
    double* x_ptr,
    double* y_ptr
)
{


#ifdef PREVIOUS_WAY
    if (IS_NEGATIVE_DBL(X) || IS_NEGATIVE_DBL(Y) || IS_NEGATIVE_DBL(Z) )
    {
        *x_ptr = *y_ptr = NOT_SET;
    }
    else if ( (X+Y+Z) < .001)
    {
        *x_ptr = *y_ptr = 0.0;
    }
#else
    if ((X < 0.0) || (Y < 0.0) || (Z < 0.0))
    {
        *x_ptr = *y_ptr = DBL_NOT_SET;
    }
    else if ((X+Y+Z) < MIN_PROJECTION_DENOMINATOR)
    {
        *x_ptr = *y_ptr = DBL_NOT_SET;
    }
#endif
    else
    {
        *x_ptr = X / (X+Y+Z);
        *y_ptr = Y / (X+Y+Z);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_uv_from_spectrum
 *
 *
 *
 * Index: sensors, colour, uv chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_uv_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* u_ptr,
    double* v_ptr
)
{
    double X, Y, Z;


    ERE(get_XYZ_from_spectrum(spectrum_vp, offset, step, &X, &Y, &Z));

    ERE(get_uv_from_XYZ(X, Y, Z, u_ptr, v_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_uv_from_XYZ
 *
 *
 *
 * Index: sensors, colour, uv chromaticity, XYZ colour
 *
 * -----------------------------------------------------------------------------
*/

int get_uv_from_XYZ
(
    double  X,
    double  Y,
    double  Z,
    double* u_ptr,
    double* v_ptr
)
{


#ifdef PREVIOUS_WAY
    if (IS_NEGATIVE_DBL(X) || IS_NEGATIVE_DBL(Y) || IS_NEGATIVE_DBL(Z) )
    {
        *u_ptr = *v_ptr = NOT_SET;
    }
    else if ((X + (15.0*Y) + (3.0*Z)) < .5)
    {
        *u_ptr = *v_ptr = 0.0;
    }
#else
    if ((X < 0.0) || (Y < 0.0) || (Z < 0.0))
    {
        *u_ptr = *v_ptr = DBL_NOT_SET;
    }
    else if ((X + (15.0*Y) + (3.0*Z)) < MIN_PROJECTION_DENOMINATOR)
    {
        *u_ptr = *v_ptr = DBL_NOT_SET;
    }
#endif
    else
    {
        *u_ptr = (4.0 * X)/(X + (15.0*Y) + (3.0*Z));
        *v_ptr = (9.0 * Y)/(X + (15.0*Y) + (3.0*Z));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_rg_from_spectrum
 *
 *
 *
 * Index: sensors, colour, rg chromaticity
 *
 * -----------------------------------------------------------------------------
*/

int get_rg_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* r_ptr,
    double* g_ptr
)
{
    double R, G, B;
    int  version;


    ERE(version = get_RGB_from_spectrum(spectrum_vp, offset, step, &R, &G, &B));

    ERE(get_rg_from_RGB(R, G, B, r_ptr, g_ptr));

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_rg_from_RGB
 *
 *
 *
 * Index: sensors, colour, rg chromaticity, RGB colour
 *
 * -----------------------------------------------------------------------------
*/

int get_rg_from_RGB
(
    double  R,
    double  G,
    double  B,
    double* r_ptr,
    double* g_ptr
)
{
    double rgb_sum;


    rgb_sum = R + G + B;

#ifdef PREVIOUS_WAY
    if (IS_NEGATIVE_DBL(R) || IS_NEGATIVE_DBL(G) || IS_NEGATIVE_DBL(B) )
    {
        *r_ptr = *g_ptr = NOT_SET;
    }
    else if ( ! IS_POSITIVE_DBL(rgb_sum) )
    {
        *r_ptr = *g_ptr = 0.0;
    }
#else
    if ((R < 0.0) || (G < 0.0) || (B < 0.0))
    {
        *r_ptr = *g_ptr = DBL_NOT_SET;
    }
    else if (rgb_sum < MIN_PROJECTION_DENOMINATOR)
    {
        *r_ptr = *g_ptr = DBL_NOT_SET;
    }
#endif
    else
    {
        *r_ptr = R / rgb_sum;
        *g_ptr = G / rgb_sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_XYZ_from_spectrum
 *
 *
 *
 * Index: sensors, colour, XYZ colour
 *
 * -----------------------------------------------------------------------------
*/

int get_XYZ_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* X_ptr,
    double* Y_ptr,
    double* Z_ptr
)
{
    static double xyz_elements[ 3 ];
    static Vector xyz_vector        = { 3, 3, xyz_elements };
    Vector*       xyz_vp            = &xyz_vector;


    ERE(set_XYZ_sensors(spectrum_vp->length, offset, step));

    ERE(multiply_matrix_and_vector(&xyz_vp, fs_xyz_derived_sp->spectra_mp,
                                   spectrum_vp));

    *X_ptr = step * 683.0 * xyz_vector.elements[ 0 ];
    *Y_ptr = step * 683.0 * xyz_vector.elements[ 1 ];
    *Z_ptr = step * 683.0 * xyz_vector.elements[ 2 ];

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_RGB_from_spectrum
 *
 *
 *
 * Index: sensors, colour, RGB colour
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* R_ptr,
    double* G_ptr,
    double* B_ptr
)
{
    static double rgb_elements[ 3 ];
    static Vector rgb_vector        = { 3, 3, rgb_elements };
    Vector*       rgb_vp            = &rgb_vector;
    int           version;


    ERE(version = set_RGB_sensors(spectrum_vp->length, offset, step));

    ERE(multiply_matrix_and_vector(&rgb_vp, fs_rgb_derived_sp->spectra_mp,
                                   spectrum_vp));

    *R_ptr = (step / 4.0) * rgb_vector.elements[ 0 ];
    *G_ptr = (step / 4.0) * rgb_vector.elements[ 1 ];
    *B_ptr = (step / 4.0) * rgb_vector.elements[ 2 ];

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* FIX: This does not do anything yet. */

/*ARGSUSED*/
int get_ct_from_spectrum
(
    Vector* __attribute__((unused)) dummy_spectrum_vp,
    double  __attribute__((unused)) dummy_offset,
    double  __attribute__((unused)) dummy_step,
    double* ct_ptr
)
{



    *ct_ptr = 0.0;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_sum_from_spectrum
 *
 * Computes integral of spectrum
 *
 * This routime computes integral of spectrum, which is the sum of the
 * samplings, multiplied by the step size.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure (not likely!).
 *
 * Index: sensors, colour
 *
 * -----------------------------------------------------------------------------
*/

int get_sum_from_spectrum
(
    Vector* spectrum_vp,
    double  step,
    double* sum_ptr
)
{



    *sum_ptr = step * sum_vector_elements(spectrum_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_RGB_vector_from_spectrum
 *
 *
 *
 * Index: sensors, colour, RGB colour
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_vector_from_spectrum
(
    Vector** target_vpp,
    Vector*  input_spectrum_vp,
    double   offset,
    double   step
)
{
    double R, G, B;
    int  version;


    ERE(version = get_RGB_from_spectrum(input_spectrum_vp, offset, step,
                                        &R, &G, &B));

    ERE(get_target_vector(target_vpp, 3));

    (*target_vpp)->elements[ 0 ] = R;
    (*target_vpp)->elements[ 1 ] = G;
    (*target_vpp)->elements[ 2 ] = B;

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         generate_RGB_data
 *
 * Generates RGB data from spectra.
 *
 * This routine generates RGB data from reflectance and illuminant spectra using
 * the sensors in sensor_sp, or, if sensor_sp is NULL, the ones managed by the
 * KJB library. If KJB library options are made available to the user, then the
 * sensors used can be changed using the "sensor-file" or "sf" option. If the
 * user has not specified sensors, then default ones are used.
 *
 * The resulting RGB are put into the matrix *target_mpp which is created if
 * it is NULL, and resized if it is the wrong size.
 *
 * The "index" argument is used to specify a specific illuminat index in
 * illum_sp. If index is set to USE_ALL_SPECTRA then all combinations of
 * illuminants and reflectances are used.
 *
 * Returns:
 *    If the sensors are supplied, then this routine returns NO_ERROR on
 *    success, and ERROR on failure.  If the KJB library sensors are used, then
 *    this routine returns the sensor version number on success, and ERROR on
 *    failure, with an appropriate error message being set. The sensor version
 *    number is incremented every time the use user resets the sensor data
 *    files, and thus can be used to verify that cached data is still OK with
 *    sensor_data_is_up_to_date().
 *
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int generate_RGB_data
(
    Matrix**       target_mpp,
    const Spectra* illum_sp,
    int            index,
    const Spectra* reflect_sp,
    const Spectra* sensor_sp
)
{
    int            result;
    Spectra*       new_sp = NULL;
    const Spectra* sp;


    if ((illum_sp == NULL) && (reflect_sp == NULL))
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    if (illum_sp == NULL)
    {
        sp = reflect_sp;
    }
    else if (reflect_sp == NULL)
    {
        sp = illum_sp;
    }
    else
    {
        ERE(multiply_spectra(&new_sp, illum_sp, index, reflect_sp));
        sp = new_sp;
    }

    result = get_RGB_data_from_spectra(target_mpp, sp, sensor_sp);

    free_spectra(new_sp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_RGB_data_from_spectra
 *
 * Generates RGB data from spectra.
 *
 * This routine generates RGB data from spectra using the sensors in sensor_sp,
 * or, if sensor_sp is NULL, the ones managed by the KJB library. If KJB library
 * options are made available to the user, then the sensors used can be changed
 * using the "sensor-file" or "sf" option. If the user has not specified
 * sensors, then default ones are used.
 *
 * The resulting RGB are put into the matrix *target_mpp which is created if
 * it is NULL, and resized if it is the wrong size.
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
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_data_from_spectra
(
    Matrix**       target_mpp,
    const Spectra* input_sp,
    const Spectra* sensor_sp
)
{
    int      result             = NO_ERROR;
    int      version            = NO_ERROR;
    Spectra* standard_sensor_sp = NULL;


    if (sensor_sp == NULL)
    {
        version = get_RGB_sensors(&standard_sensor_sp, NOT_SET, DBL_NOT_SET,
                                  DBL_NOT_SET);
        if (version == ERROR)
        {
            result = ERROR;
        }
        sensor_sp = standard_sensor_sp;
    }

    if (result != ERROR)
    {
        result = check_spectra_are_comparable(sensor_sp, input_sp);

        if (result == ERROR)
        {
            insert_error("Unable to compute RGB from spectra and sensors.");
        }
    }

    if (result != ERROR)
    {
        result = multiply_by_transpose(target_mpp, input_sp->spectra_mp,
                                       sensor_sp->spectra_mp);
    }

    free_spectra(standard_sensor_sp);

    if (result == ERROR)
    {
        return ERROR;
    }
    else
    {
        return version;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_RGB_vector_from_spectra
 *
 * Generates RGB data from spectra.
 *
 * This routine generates RGB data from a single spectra using the sensors
 * managed by the KJB library. If KJB library options are made available to the
 * user, then the sensors used can be changed using the "sensor-file" or "sf"
 * option. If the user has not specified sensors, then default ones are used.
 *
 * The "index" argument is used to specify which of the spectra in input_sp us
 * used.
 *
 * The resulting RGB are put into the vector *target_vpp which is created if
 * it is NULL, and resized if it is the wrong size.
 *
 * Returns :
 *    This routine returns the sensor version number on success, and ERROR on
 *    failure, with an appropriate error message being set. The sensor version
 *    number is incremented every time the use user resets the sensor data
 *    files, and thus can be used to verify that cached data is still OK with
 *    sensor_data_is_up_to_date().
 *
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_vector_from_spectra
(
    Vector**       target_vpp,
    const Spectra* input_sp,
    int            index
)
{
    double R, G, B;
    int  version;


    ERE(version = get_RGB_from_spectra(input_sp, index, &R, &G, &B));
    ERE(get_target_vector(target_vpp, 3));

    (*target_vpp)->elements[ 0 ] = R;
    (*target_vpp)->elements[ 1 ] = G;
    (*target_vpp)->elements[ 2 ] = B;

    return version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_RGB_from_spectra
 *
 * Generates RGB data from spectra.
 *
 * This routine generates RGB data from a single spectra using the sensors
 * managed by the KJB library. If KJB library options are made available to the
 * user, then the sensors used can be changed using the "sensor-file" or "sf"
 * option. If the user has not specified sensors, then default ones are used.
 *
 * The "index" argument is used to specify which of the spectra in input_sp us
 * used.
 *
 * The resulting RGB are put into *R_ptr, *G_ptr, and *B_ptr.
 *
 * Returns :
 *    This routine returns NO_ERROR on success, and ERROR on failure, with an
 *    appropriate error message being set.
 *
 * Index: spectra, sensors, colour, RGB
 *
 * -----------------------------------------------------------------------------
*/

int get_RGB_from_spectra
(
    const Spectra* input_sp,
    int            index,
    double*        R_ptr,
    double*        G_ptr,
    double*        B_ptr
)
{
    int     result;
    Vector* spectrum_vp = NULL;
    double  offset      = input_sp->offset;
    double  step        = input_sp->step;
    Matrix* spectra_mp  = input_sp->spectra_mp;


    ERE(get_matrix_row(&spectrum_vp, spectra_mp, index));

    result = get_RGB_from_spectrum(spectrum_vp, offset, step, R_ptr, G_ptr,
                                   B_ptr);

    free_vector(spectrum_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                          set_RGB_sensors
 * -----------------------------------------------------------------------------
*/

static int set_RGB_sensors(int count, double offset, double step)
{
    int i, j;


    if (fs_rgb_original_sp == NULL)
    {
        char sensor_data_dir[ MAX_FILE_NAME_SIZE ];
        char sensor_file[ MAX_FILE_NAME_SIZE ];


#ifdef TRACK_MEMORY_ALLOCATION
        add_cleanup_function(free_RGB_sensors);
#endif

        verbose_pso(10, "Setting up base RGB sensors\n");

        BUFF_CPY(sensor_data_dir, DATA_DIR);
        BUFF_CAT(sensor_data_dir, DIR_STR);
        BUFF_CAT(sensor_data_dir, SENSOR_DIR);

        /*
        **  OK, so this is perhaps a bit too clever. However, the idea is
        **  to try a configuration file, and if it succeeds, read it. If either
        **  the getting of the configuration file, or its read fails, then
        **  print an error message and use the internal data.
        */
        if (    (get_config_file("SENSOR_FILE", sensor_data_dir,
                                 SENSOR_FILE, "sensor",
                                 sensor_file, sizeof(sensor_file))
                 == ERROR)
             ||
#ifdef TRY_DIFFERENTLY
                ((fs_rgb_original_sp = read_sensor_spectra(sensor_file)) == NULL)
#else
                (set_sensor_file(sensor_file) == ERROR)
#endif
            )
        {
            kjb_print_error();
            p_stderr("No sensor file : using internal data\n");

            NRE(fs_rgb_original_sp = create_spectra(3, NUM_SPECTROMETER_INTERVALS,
                                                SPECTROMETER_OFFSET,
                                                SPECTROMETER_STEP,
                                                SENSOR_SPECTRA));

            for (i=0; i<3; i++)
            {
                for (j=0; j<fs_rgb_original_sp->spectra_mp->num_cols; j++)
                {
                    (fs_rgb_original_sp->spectra_mp->elements)[ i ][ j ] =
                                                 fs_rgb_response_data[ i ][ j ];
                }
            }

            fs_sensor_version++;
        }
        else
        {
            verbose_pso(2, "Using sensor file %s.\n", sensor_file);
            BUFF_CPY(fs_current_sensor_file, sensor_file);
        }
    }

    if (count  < 0)   count  = fs_rgb_original_sp->spectra_mp->num_cols;
    if (offset < 0.0) offset = fs_rgb_original_sp->offset;
    if (step   < 0.0) step   = fs_rgb_original_sp->step;

    /*
    ** If we have derived sensor spectra, then check if it is OK. If it is not,
    ** then free it, and force recalculation.
    */
    if (    (fs_rgb_derived_sp != NULL)
         && (    (count != fs_rgb_derived_sp->spectra_mp->num_cols)
              || (! IS_EQUAL_DBL(offset, fs_rgb_derived_sp->offset))
              || (! IS_EQUAL_DBL(step, fs_rgb_derived_sp->step))
            )
       )
    {
        verbose_pso(10, "Forcing recalculation of derived RGB sensors\n");
        free_spectra(fs_rgb_derived_sp);
        fs_rgb_derived_sp = NULL;
    }

    /*
    ** If we don't have derived, then compute it from fs_rgb_original_sp
    */
    if (fs_rgb_derived_sp == NULL)
    {
        verbose_pso(10, "Re-calculating derived RGB sensors\n");
        ERE(convert_spectra(&fs_rgb_derived_sp, fs_rgb_original_sp, count, offset,
                            step));
    }

    return fs_sensor_version;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      free_RGB_sensors
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION
static void free_RGB_sensors(void)
{
    free_spectra(fs_rgb_original_sp);
    free_spectra(fs_rgb_derived_sp);

    fs_rgb_original_sp = NULL;
    fs_rgb_derived_sp  = NULL;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 * STATIC                        set_XYZ_sensors
 * -----------------------------------------------------------------------------
*/

static int set_XYZ_sensors(int count, double offset, double step)
{
    int i, j;


    if (fs_xyz_original_sp == NULL)
    {
        char sensor_data_dir[ MAX_FILE_NAME_SIZE ];
        char xyz_file[ MAX_FILE_NAME_SIZE ];


#ifdef TRACK_MEMORY_ALLOCATION
        add_cleanup_function(free_XYZ_sensors);
#endif

        BUFF_CPY(sensor_data_dir, DATA_DIR);
        BUFF_CAT(sensor_data_dir, DIR_STR);
        BUFF_CAT(sensor_data_dir, XYZ_DIR);

        /*
        **  OK, so this is perhaps a bit too clever. However, the idea is
        **  to try a configuration file, and if it succeeds, read it. If either
        **  the getting of the configuration file, or its read fails, then
        **  print an error message and use the internal data.
        */
        if (    (get_config_file("XYZ_FILE", sensor_data_dir, XYZ_FILE, "XYZ",
                                 xyz_file, sizeof(xyz_file))
                 == ERROR)
             ||
                ((read_sensor_spectra(&fs_xyz_original_sp, xyz_file)) == ERROR)
            )
        {

#ifdef DONT_BUG_USER_ABOUT_USUAL_CASE  /* Internal data is the usual case. */
            kjb_print_error();
            kjb_fprintf(stderr,
                   "Using internal data due to failure to find XYZ file.\n");
#endif

            NRE(fs_xyz_original_sp = create_spectra(3, NUM_SPECTROMETER_INTERVALS,
                                                SPECTROMETER_OFFSET,
                                                SPECTROMETER_STEP,
                                                SENSOR_SPECTRA));

            for (i=0; i<3; i++)
            {
                for (j=0; j<fs_xyz_original_sp->spectra_mp->num_cols; j++)
                {
                    (fs_xyz_original_sp->spectra_mp->elements)[ i ][ j ] =
                                                   fs_xyz_response_data[ i ][ j ];
                }
            }
        }
#ifdef DONT_BUG_USER_ABOUT_USUAL_CASE  /* Internal data is the usual case. */
        else
        {
            verbose_pso(2, "Using XYZ file %s.\n", xyz_file);
        }
#endif

        fs_xyz_original_sp->type = SENSOR_SPECTRA;
    }

    /*
    ** If we have derived sensor spectra, then check if it is OK. If it is not,
    ** then free it, and force recalculation.
    */
    if (    (fs_xyz_derived_sp != NULL)
         && (    (count != fs_xyz_derived_sp->spectra_mp->num_cols)
              || (! IS_EQUAL_DBL(offset, fs_xyz_derived_sp->offset))
              || (! IS_EQUAL_DBL(step, fs_xyz_derived_sp->step))
            )
       )
    {
        free_spectra(fs_xyz_derived_sp);
        fs_xyz_derived_sp = NULL;
    }

    /*
    ** If we don't have derived, then compute it from fs_xyz_original_sp
    */
    if (fs_xyz_derived_sp == NULL)
    {
        ERE(convert_spectra(&fs_xyz_derived_sp, fs_xyz_original_sp, count, offset,
                            step));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 * STATIC                          free_XYZ_sensors
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION
static void free_XYZ_sensors(void)
{
    free_spectra(fs_xyz_original_sp);
    free_spectra(fs_xyz_derived_sp);


    fs_xyz_original_sp = NULL;
    fs_xyz_derived_sp  = NULL;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

