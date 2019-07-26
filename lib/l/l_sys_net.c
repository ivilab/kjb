
/* $Id: l_sys_net.c 22170 2018-06-23 23:01:50Z kobus $ */

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

#include "l/l_gen.h"    /* Only safe as first include in a ".c" file. */

#ifdef SUN5
#ifndef MAKE_DEPEND
#    include <sys/systeminfo.h>
#endif
#endif


#include "l/l_io.h"
#include "l/l_string.h"
#include "l/l_parse.h"
#include "l/l_sys_net.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef UNIX
int kjb_get_host_name(char* host_buff, size_t buff_len)
{
    static int first_time = TRUE;
    static char host_name_cache[ 200 ] = { '\0' } ;

    int res, done;
    FILE* host_fp;
    char line[ 200 ];
    char* line_pos;
    char dummy[ 200 ];
    char host_string[ 200 ];
    char host_string_one[ 200 ];
    char host_string_two[ 200 ];


    /*
     * Note: The flag first_time gets set to false only if the
     *       attempt to get the host name succeeds. Otherwise we
     *       keep banging away at it.
     */
    if ( ! first_time )
    {
        kjb_strncpy(host_buff, host_name_cache, buff_len);
        return NO_ERROR;
    }

#ifdef SUN5
    res = sysinfo(SI_HOSTNAME, host_buff, (int)buff_len);
#else
    res = gethostname(host_buff, (size_t)buff_len);
#endif

    if (res == EOF)
    {
        set_error("Can't determine local host name.");
        *host_buff = '\0';
        return ERROR;
    }
    else if (FIND_CHAR_YES(host_buff, '.'))
    {
        return NO_ERROR;
    }
    else
    {
        host_fp = kjb_fopen("/etc/hosts", "r");

        if (host_fp == NULL) return NO_ERROR;

        line[ 0 ] = '\0';

        done = FALSE;

        while (! done)
        {
            res = BUFF_GET_REAL_LINE(host_fp, line);

            if ((res == EOF) || (res == ERROR))
            {
                done = TRUE;
            }
            else
            {
                line_pos = line;
                BUFF_GET_TOKEN(&line_pos, dummy);
                trim_beg(&line_pos);

                host_string_one[ 0 ] = '\0';
                BUFF_GET_TOKEN(&line_pos, host_string_one);

                trim_beg(&line_pos);
                host_string_two[ 0 ] = '\0';
                BUFF_GET_TOKEN(&line_pos, host_string_two);

                if ((host_string_one[ 0 ] != '\0') &&
                    (host_string_two[ 0 ] != '\0'))
                {
                    if (strlen(host_string_two)>strlen(host_string_one))
                    {
                        BUFF_CPY(host_string, host_string_two);
                    }
                    else
                    {
                        BUFF_CPY(host_string, host_string_one);
                    }

                    if (strlen(host_string) > strlen(host_buff))
                    {
                        if (HEAD_CMP_EQ(host_string, host_buff))
                        {
                            kjb_strncpy(host_buff, host_string, buff_len);
                            done = TRUE;
                        }
                     }
                 }
             }
         }

        kjb_fclose(host_fp);  /* Ignore return--only reading. */

        /*
         * Success, so cache it !
         */
        first_time = FALSE;
        BUFF_CPY(host_name_cache, host_buff);

        return NO_ERROR;
    }
}
#else
#ifdef NOVELL
int kjb_get_host_name(host_buff, buff_len)
    char*   host_buff;
    long buff_len;
{


    *host_buff = '\0';

    set_bug("Local host name is not supported on this system.");

    return ERROR;
}

#else
int kjb_get_host_name(host_buff, buff_len)
    char*   host_buff;
    size_t buff_len;
{


    *host_buff = '\0';

    set_bug("Local host name is not supported on this system.");

    return ERROR;
}
#endif
#endif

int get_host_suffix(char* host, char* suffix_buff, size_t buff_len)
{
    int char_count, host_len;
    char* host_pos;


    host_len = strlen(host);

    *suffix_buff = '\0';

    char_count = host_len - 1;
    host_pos = host + char_count;

    if (*host_pos == '.')
    {
        set_error("Format of host name %q is invalid.", host);
        return ERROR;
    }

    while ((char_count > 0) && (*host_pos != '.'))
    {
        host_pos--;
        char_count--;
    }

    if (char_count == 0)
    {
        return NO_ERROR;
    }

    host_pos--;
    char_count--;

    if (*host_pos == '.')
    {
        set_error("Format of host name %q is invalid.", host);
        return ERROR;
    }

    while ((char_count > 0) && (*host_pos != '.'))
    {
        host_pos--;
        char_count--;
    }

    if (char_count <= 0)
    {
        set_error("Format of host name %q is invalid.", host);
        return ERROR;
    }

    host_pos++;

    if (strlen(host_pos) + 1 > buff_len)
    {
        SET_BUFFER_OVERFLOW_BUG();
        return ERROR;
    }

    strcpy(suffix_buff, host_pos);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_inet_socket
 *
 * Simple interface to get a socket to an internet service
 *
 * This routine is a super simple interfact to get a socket to an internet
 * service, typically a dumb device. This routine is not very sophisticated and
 * it could use some improvements. But it does save looking up all the complex
 * internet stuff which is usually not needed for simple applications. 
 *
 * Returns:
 *    socket (io descriptor) on success
 *    ERROR otherwise
 *
 * Index:
 *     networking 
 *
 * -----------------------------------------------------------------------------
*/

#ifdef LINUX

#ifndef MAKE_DEPEND
#    include <sys/socket.h> 
#    include <netdb.h> 
#endif

int get_inet_socket(const char* ip_str, const char* port_str)
{
    struct addrinfo *addr = NULL;
    int fd;


    if (getaddrinfo(ip_str, port_str, NULL, &addr) != 0)
    {
        set_error("Error getting address.%S");
        return ERROR;
    }

    if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == EOF)
    {
        set_error("Error getting socket.%S");
        return ERROR;
    }

    if (connect(fd, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        set_error("Unable to connect to %s using port %s.%S", 
                  ip_str, port_str);
        return ERROR;
    }

    freeaddrinfo(addr); 

    return fd;
}

#else 

#ifdef MAC_OSX

#ifndef MAKE_DEPEND
#    include <sys/socket.h> 
#    include <netdb.h> 
#endif

int get_inet_socket(const char* ip_str, const char* port_str)
{
    struct addrinfo hints, *addr = NULL;
    int fd;

    SET_TO_ZERO(hints);
    hints.ai_family = PF_UNSPEC;

    /* Mac cares about socket type, at least for the tell_iboot application. */
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip_str, port_str, &hints, &addr) != 0)
    {
        set_error("Error getting address.%S");
        return ERROR;
    }

    if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == EOF)
    {
        set_error("Error getting socket.%S");
        return ERROR;
    }

    if (connect(fd, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        set_error("Unable to connect to %s using port %s.%S", 
                  ip_str, port_str);
        return ERROR;
    }

    freeaddrinfo(addr); 

    return fd;
}

#else  

#ifdef UNIX 

int get_inet_socket(const char* dummy_ip_str, const char* dummy_port_str)
{
    set_error("Routine get_inet_socket is only tested on LINUX systems.");
    add_error("This routine needs to be tested independently.");

    return ERROR;
}

#else 

int get_inet_socket(const char* dummy_ip_str, const char* dummy_port_str)
{
    set_error("Routine get_inet_socket is not implemented for non UNIX systems.\n");
    return ERROR;
}

#endif 

#endif 

#endif 


#ifdef __cplusplus
}
#endif

