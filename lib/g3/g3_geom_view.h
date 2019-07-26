
/* $Id: g3_geom_view.h 4727 2009-11-16 20:53:54Z kobus $ */

/*
   Copyright (c) 1994-2008 by Kobus Barnard (author).

   Personal and educational use of this code is granted, provided
   that this header is kept intact, and that the authorship is not
   misrepresented. Commercial use is not permitted.
*/

#ifndef G3_GEOM_VIEW_INCLUDED
#define G3_GEOM_VIEW_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define DEFAULT_MAX_NUM_GEOM_VIEWS     1
#define CURRENT_GEOM_VIEW              (-1000)
#define NEW_GEOM_VIEW                  (-1001)


typedef struct Geom_view
{
   int            pid;
   int            id;
   int            write_des;
   int            coloured_object_count;
   Queue_element* object_list_head;
   char*          label;
   void*          app_field;
}
Geom_view;


int set_max_num_geom_views(int);
Geom_view* geom_view_open(void);
Geom_view* get_geom_view_info_ptr(int);

int geom_view_geometry
(
    Geom_view* ,
    const char*      ,
    const char*      ,
    double     ,
    double     ,
    double
);

int geom_view_add_geometry
(
    Geom_view*  ,
    const char*       ,
    const char* ,
    double      ,
    double      ,
    double
);

int   geom_view_dummy     (const Geom_view*);
int   geom_view_add_axis  (Geom_view*, double);
int   geom_view_add_object(Geom_view*, const char*);
int   geom_view_display   (const Geom_view*);
int   geom_view_clear_data(const Geom_view*);
int   geom_view_close     (Geom_view*);
void  geom_view_close_all (void);
char* get_facet_list      (const char*, double, double, double);
int   geom_view_write     (const Geom_view*, const char*);

TRAP_FN_RETURN_TYPE geom_view_sigpipe_fn(int);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

