
/* $Id: im_score.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef IM_SCORE_INCLUDED
#define IM_SCORE_INCLUDED

#ifndef __C2MAN__ 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int im_score_main(int argc, char** argv);

#ifdef KJB_HAVE_X11
int im_score(Image*, ImageInfo*);
#endif 

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif   /* #ifndef __C2MAN__ */

#endif   /* Include this file. */



