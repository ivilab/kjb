
/* $Id: l_sys_net.h 5389 2010-02-14 20:24:43Z kobus $ */

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

#ifndef L_SYS_NET_INCLUDED
#define L_SYS_NET_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define DEFAULT_REGISTERED_VERSION_NUM       1


#define BUFF_GET_HOST_NAME(x)    kjb_get_host_name(x, sizeof(x))


int kjb_get_host_name(char*, size_t);
int get_host_suffix(char*, char*, size_t);

int get_inet_socket(const char* ip_str, const char* port_str);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

