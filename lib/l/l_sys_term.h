
/* $Id: l_sys_term.h 21593 2017-07-30 16:48:05Z kobus $ */

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

#ifndef L_SYS_TERM_INCLUDED
#define L_SYS_TERM_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define BUFF_TERM_GET_LINE(x, y)   term_get_line(x, y, sizeof(y ))

#define UP_ARROW                       (-500)
#define DOWN_ARROW                     (-501)
#define LEFT_ARROW                     (-502)
#define RIGHT_ARROW                    (-503)
#define ERASE_CHAR                     (-504)
#define INVALID_CONTROL_SEQUENCE       (-599)


int set_term_io_options   (const char* option, const char* value);
int kjb_use_default_paging(void);
int kjb_enable_paging     (void);
int kjb_disable_paging    (void);
int kjb_restore_paging    (void);
int is_in_background      (void);
int toggle_high_light     (FILE*);
int set_high_light        (FILE*);
int unset_high_light      (FILE*);

#ifdef TEST
    int record_term_input(void);
#endif

void        setup_terminal_size            (void);
int term_puts                      (const char*);
int put_prompt                     (const char*);
int term_put_n_chars               (const char*, size_t);
int term_put_n_raw_chars           (const char*, size_t);

int term_get_n_chars
(
    const char* ,
    char*       ,
    size_t
);

int overwrite_term_get_n_chars
(
    const char* ,
    char*       ,
    size_t
);

int         overwrite_term_get_char        (void);
int         move_cursor_up                 (int);
int         move_cursor_down               (int);
void        term_beep                      (void);

void        term_beep_beep
(
    int num_beeps,
    int pause_in_ms
);

int term_get_line
(
    const char* ,
    char*       ,
    size_t
);

int         term_getc                      (void);
int         enhanced_term_getc             (void);
void        term_blank_out_line            (void);
void        reset_initial_terminal_settings(void);
int         term_set_cooked_mode           (void);
int         term_set_raw_mode_with_echo    (void);
int         term_set_raw_mode_with_no_echo (void);
void        term_reset                     (void);
void        term_rewind_setting_stack      (void);
void        term_set_blocking              (void);
void        term_set_no_blocking           (void);
void        print_term_input_queue         (FILE* fp);
void        read_line_reentry_history      (const char* program_name);

#ifdef TRACK_MEMORY_ALLOCATION
    void free_line_reentry_queue(void);
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

