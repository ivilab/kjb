/*
 * $Id: l_mt_util.h 15510 2013-10-05 06:21:52Z predoehl $
 */

#ifndef L_MT_UTIL_H_LIBKJB_INCLUDED
#define L_MT_UTIL_H_LIBKJB_INCLUDED 1

#include <l/l_sys_def.h>
#include <l/l_sys_io.h>

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

void *kjb_mt_malloc(Malloc_size num_bytes);

void kjb_mt_free(void *ptr);

FILE *kjb_mt_fopen(
    const char *input_fd_name,
    const char *mode
);

long kjb_mt_fread(
    FILE *fp,
    void *buff,
    size_t len
);

long kjb_mt_fwrite(
    FILE *fp,
    const void *line,
    size_t len
);

long kjb_mt_fprintf(
    FILE* fp,
    const char* format_str,
    ...
);

int kjb_mt_fclose(FILE *fp);


/* below:  end-of-file boilerplate */

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif /* L_MT_UTIL_H_LIBKJB_INCLUDED */

