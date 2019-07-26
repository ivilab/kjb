
/* $Id: m_norm.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_norm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

Norm_method parse_normalization_method(const char* normalization_string)
{
    char lc_normalization_string[ 100 ];


    EXTENDED_LC_BUFF_CPY(lc_normalization_string, normalization_string);

    if (    (STRCMP_EQ(lc_normalization_string, "norm"))
         || (STRCMP_EQ(lc_normalization_string, "mag"))
         || (STRCMP_EQ(lc_normalization_string, "magnitude"))
         || (STRCMP_EQ(lc_normalization_string, "l2"))
       )
    {
        return NORMALIZE_BY_MAGNITUDE;
    }
    else if ((STRCMP_EQ(lc_normalization_string, "sum")))
    {
        return NORMALIZE_BY_SUM;
    }
    else if (    (STRCMP_EQ(lc_normalization_string, "mean"))
              || (STRCMP_EQ(lc_normalization_string, "ave"))
              || (STRCMP_EQ(lc_normalization_string, "average"))
            )
    {
        return NORMALIZE_BY_MEAN;
    }
    else if (    (STRCMP_EQ(lc_normalization_string, "max"))
              || (STRCMP_EQ(lc_normalization_string, "max-abs"))
              || (STRCMP_EQ(lc_normalization_string, "l1"))
            )
    {
        return NORMALIZE_BY_MAX_ABS_VALUE;
    }
    else if (STRCMP_EQ(lc_normalization_string, "off"))
    {
        return DONT_NORMALIZE;
    }
    else
    {
        set_error("%q is an invalid normalization method.",
                  normalization_string);
        return NORMALIZATION_METHOD_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

