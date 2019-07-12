/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: msvc-compat.h 14469 2013-05-20 05:06:14Z jguan1 $ */

#ifndef FLANDMAR_MSVC_COMPAT
#define FLANDMAR_MSVC_COMPAT
#ifdef FLANDMAR_MSC_VER

namespace kjb {
namespace flandmark {

typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned __int16 uint16_t;
typedef __int16 int16_t;
typedef unsigned __int32 uint32_t;
typedef __int32 int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;

}} //namespace kjb::flandmark

#else
#	include <stdint.h>
#endif

#endif
