
/* $Id: qhull_a.h 20918 2016-10-31 22:08:27Z kobus $ */

#ifndef __C2MAN__     

/*
// Qhull files are distributed under the terms set out in the latest COPYING.txt
// files which is provided below. All changes are flagged with "Kobus"; none of
// the changes are substantive. The reason for all changes is simply for better
// integratation with the programs which use it.
//
// IMPORTANT NOTE: The version of qhull here is very old. If you need qhull for
// anything other than compiling the code with which I distributed it, you are
// STRONGLY ADVISED to get the latest copy. Ideally, I should upgrade the code
// here to a later version. This is on the TODO list, but unfortunately, it has
// only modest priority at this time.
//
//                                                      Kobus Barnard.
//
// ---------------------------------------------------------------------------
//
//                     Qhull, Copyright (c) 1993-1995
//
//         The National Science and Technology Research Center for
//          Computation and Visualization of Geometric Structures
//                          (The Geometry Center)
//                         University of Minnesota
//                              400 Lind Hall
//                          207 Church Street S.E.
//                        Minneapolis, MN 55455  USA
//
//                         email: qhull@geom.umn.edu
//
//  This software includes Qhull from The Geometry Center.  Qhull is
//  copyrighted as noted above.  Qhull is free software and may be obtained
//  via http from www.geom.umn.edu.  It may be freely copied, modified,
//  and redistributed under the following conditions:
//
//  1. All copyright notices must remain intact in all files.
//
//  2. A copy of this text file must be distributed along with any copies
//     of Qhull that you redistribute; this includes copies that you have
//     modified, or copies of programs or other software products that
//     include Qhull.
//
//  3. If you modify Qhull, you must include a notice giving the
//     name of the person performing the modification, the date of
//     modification, and the reason for such modification.
//
//  4. When distributing modified versions of Qhull, or other software
//     products that include Qhull, you must provide notice that the original
//     source code may be obtained as noted above.
//
//  5. There is no warranty or other guarantee of fitness for Qhull, it is
//     provided solely "as is".  Bug reports or fixes may be sent to
//     qhull_bug@geom.umn.edu; the authors may or may not act on them as
//     they desire.
//
//
*/

#ifndef qhDEFqhulla
#define qhDEFqhulla


/* Kobus */
#include "l/l_sys_def.h" 
#include "l/l_sys_sys.h"
#include "l/l_sys_std.h" 

/* END kobus */


#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <float.h>    /* some compilers will not need float.h */
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <memory.h>

#include "qhull/qhull.h"


#include "qhull/mem.h"
#include "qhull/set.h"
#include "qhull/geom.h"
#include "qhull/merge.h"
#include "qhull/poly.h"
#include "qhull/io.h"
#include "qhull/stat.h"

/* Kobus */
#include "l/l_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* End Kobus */

/* ======= -macros- =========== */

/*-----------------------------------------------
-traceN((fp.ferr, "format\n", vars));  calls fprintf if IStracing >= N
  removing tracing reduces code size but doesn't change execution speed
*/
#ifndef qh_NOtrace
#define trace0(args) {if (qh IStracing) fprintf args;}
#define trace1(args) {if (qh IStracing >= 1) fprintf args;}
#define trace2(args) {if (qh IStracing >= 2) fprintf args;}
#define trace3(args) {if (qh IStracing >= 3) fprintf args;}
#define trace4(args) {if (qh IStracing >= 4) fprintf args;}
#define trace5(args) {if (qh IStracing >= 5) fprintf args;}
#else /* qh_NOtrace */
#define trace0(args) {}
#define trace1(args) {}
#define trace2(args) {}
#define trace3(args) {}
#define trace4(args) {}
#define trace5(args) {}

#endif /* qh_NOtrace */

/* ======= -functions ===========

see corresponding .c file for definitions

	Qhull functions (qhull.c)
-qhull		construct the convex hull of a set of points
-qhull_postmerging  set up for post merging of qhull
-addpoint       add point to hull above a facet
-buildhull	constructs a hull by adding points one at a time
-buildtracing   for tracing execution of buildhull
-errexit2	return exitcode to system after an error for two facets
-findhorizon	find the horizon and visible facets for a point
-nextfurthest   returns next furthest point for processing
-partitionall	partitions all points into the outsidesets of facets
-partitioncoplanar partition coplanar point to a facet
-partitionpoint partitions a point as inside, coplanar or outside a facet
-partitionvisible partitions points in visible_list to newfacet_list

	Global.c internal functions (others in qhull.h)
-freebuffers	free up global memory buffers
-initbuffers	initialize global memory buffers
-option         append option description to qh qhull_options
-strtod/tol     duplicates strtod/tol
*/

/***** -qhull.c prototypes (alphabetical after qhull) ********************/

void 	qh_qhull (void);
void    qh_qhull_postmerging (void);
boolT   qh_addpoint (pointT *furthest, facetT *facet, boolT checkdist);
void 	qh_buildhull(void);
void    qh_buildtracing (pointT *furthest, facetT *facet);
void 	qh_errexit2(int exitcode, facetT *facet, facetT *otherfacet);
void    qh_findhorizon(pointT *point, facetT *facet, int *goodvisible,int *goodhorizon);
pointT *qh_nextfurthest (facetT **visible);
void 	qh_partitionall(setT *vertices, pointT *points,int npoints);
void    qh_partitioncoplanar (pointT *point, facetT *facet, realT *dist);
void    qh_partitionpoint (pointT *point, facetT *facet);
void 	qh_partitionvisible(boolT allpoints, int *numpoints);
void	qh_printsummary(FILE *fp);

/***** -global.c internal prototypes (alphabetical) ***********************/

void    qh_appendprint (qh_PRINT format);
void 	qh_freebuffers (void);
void    qh_initbuffers (coordT *points, int numpoints, int dim, boolT ismalloc);
void    qh_option (char *option, int *i, realT *r);
int     qh_strtol (const char *s, char **endp);
double  qh_strtod (const char *s, char **endp);

/***** -stat.c internal prototypes (alphabetical) ***********************/

void	qh_allstatA (void);
void	qh_allstatB (void);
void	qh_allstatC (void);
void	qh_allstatD (void);
void	qh_allstatE (void);
void	qh_allstatF (void);
void 	qh_freebuffers (void);
void    qh_initbuffers (coordT *points, int numpoints, int dim, boolT ismalloc);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* qhDEFqhulla */

/* -------------------------------------------------------------------------- */

#endif   /* #ifndef __C2MAN__  */


