
/* $Id: l_help.h 8927 2011-03-14 22:35:25Z predoehl $ */

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

#ifndef L_HELP_INCLUDED
#define L_HELP_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define HELP_TOPIC_CHAR                   '/'
#define HELP_FILE_COMMENT_LINE_CHAR       '#'
#define CONTINUE_WITH_HELP_TOPIC_CHAR     '&'
#define SYSTEM_DEPENDENT_HELP_LINE_CHAR   '%'
#define DOCUMENT_TYPE_ID_CHAR             '@'

#define MAX_HELP_TOPIC_LEN               20000
#define MAX_HELP_TOPIC_LIST_LEN           5000
#define MAX_NUM_SUB_TOPICS                  35   /* 1-9 and a-z */


#ifndef MAN_OUTPUT_WIDTH
#    define MAN_OUTPUT_WIDTH 80
#endif

/* ------------------------------------------------------------------------- */

typedef enum Help_request
   {
    HELP_ON_HELP,
    CONTINUE_WITH_TOPIC,
    REDRAW_CURRENT_HELP_PAGE,
    FORWARD_HELP_PAGE,
    BACK_HELP_PAGE,
    FORWARD_HELP_SUB_TOPIC,
    BACK_HELP_SUB_TOPIC,
    REPEAT_HELP_TOPIC,
    PREVIOUS_HELP_TOPIC,
    HELP_MENU_CHOICE,
    SKIP_HELP_TOPIC,
    REWRITE_HELP_MENU,
    DONT_EXIT_HELP,
    EXIT_HELP,
    HELP_ERROR
   }
Help_request;

/* ------------------------------------------------------------------------- */

int set_help_file       (const char* file_name);
int kjb_help            (const char* program_name, const char* topic);
int local_get_help_topic(FILE*, const char*, char*, size_t);
int local_get_topic_list(FILE*, char*, size_t);
int man_print_heading   (FILE*, const char* topic);
int man_print_title     (FILE*, const char* title, const char* section);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

