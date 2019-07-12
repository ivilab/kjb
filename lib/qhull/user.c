
/* $Id: user.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "qhull/qhull_a.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*-------------------------------------------------
-call_qhull- template for calling qhull from inside your program
   remove #if 0, #endif to compile
   define: char qh_version[]= "...";
returns:
   exit code (see qh_ERR... in qhull.h)

  This can be called any number of times.

  To allow multiple, concurrent calls to qhull()
    - use qh_QHpointer, qh_save_qhull, and qh_restore_qhull to swap
      the global data structure between calls.
    - initialize the first call with qh_init_A
    - initialize other calls with qh_initqhull_start
    - use qh_freeqhull(qh_ALL) instead of qh_memfreeshort
    - warning: this ought to work but it has not been fully tested.
      Let us know if you are successful.
*/
#if 0
int call_qhull (void) {
  int curlong, totlong, exitcode, numpoints, dim;
  coordT *points;
  boolT ismalloc;
  char *flags;

  qh_init_A (stdin, stdout, stderr, 0, NULL);
  exitcode= setjmp (qh errexit);
  if (!exitcode) {
    strcat (qh rbox_command, "user");            /* for documentation only */
    qh_initflags ("qhull Tc (your flags go here)"); /* 1st word ignored */

    /* define the input data */
    points= array;    		/* an array of point coordinates */
    ismalloc= False; 		/* True if qhull should 'free(points)' at end*/
    dim= 3;         		/* dimension of points */
    numpoints= 100; 		/* number of points */;
    /* For Delaunay triangulation ('d' or 'v'), set "qh PROJECTdelaunay= True"
       to project points to a paraboloid.  You may project the points yourself.

       For halfspace intersection ('H'), compute the dual point array
       by "points= qh_sethalfspace_all (dim, numpoints, array, feasible);"
    */
    qh_init_B (points, numpoints, dim, ismalloc);

    qh_qhull();       		/* construct the hull */
    /*  For Voronoi centers ('v'), call qh_setvoronoi_all() */
    qh_check_output();     	/* optional */
    qh_produce_output();   	/* optional */
    if (qh VERIFYoutput && !qh STOPpoint && !qh STOPcone)
      qh_check_points();     	/* optional */
    exitcode= qh_ERRnone;
  }
  qh NOerrexit= True;
  qh_freeqhull(!qh_ALL);
  qh_memfreeshort (&curlong, &totlong);
  if (curlong || totlong)  	/* optional */
    kjb_fprintf(stderr, "qhull internal warning (main): did not free %d bytes of long memory (%d pieces)\n",
       totlong, curlong);
  return exitcode;
} /* call_qhull */
#endif

/*-------------------------------------------
-errexit- return exitcode to system after an error
  assumes exitcode non-zero
  prints useful information
  see qh_errexit2() in qhull.c for printing 2 facets
*/
void qh_errexit(int exitcode, facetT *facet, ridgeT *ridge) {

  if (qh ERREXITcalled) {
    kjb_fprintf(qh ferr, "\nqhull error while processing previous error.  Exit program\n");
    exit(1);
  }
  qh ERREXITcalled= True;
  if (!qh QHULLfinished)
    qh hulltime= (unsigned)clock() - qh hulltime;
  qh_errprint("ERRONEOUS", facet, NULL, ridge, NULL);
  kjb_fprintf(qh ferr, "\nWhile executing: %s | %s\n", qh rbox_command, qh qhull_command);
  kjb_fprintf(qh ferr, "Options selected for %s:\n%s\n", qh_version, qh qhull_options);
  if (qh furthest_id >= 0) {
    kjb_fprintf(qh ferr, "Last point added to hull was p%d.", qh furthest_id);
    if (zzval_(Ztotmerge))
      kjb_fprintf(qh ferr, "  Last merge was #%d.", zzval_(Ztotmerge));
    if (qh QHULLfinished)
      kjb_fprintf(qh ferr, "\nQhull has finished constructing the hull.");
    else if (qh POSTmerging)
      kjb_fprintf(qh ferr, "\nQhull has started post-merging.");
    kjb_fprintf(qh ferr, "\n");
  }
  if (qh FORCEoutput && (qh QHULLfinished || (!facet && !ridge)))
    qh_produce_output();
  else {
    if (exitcode != qh_ERRsingular && zzval_(Zsetplane) > qh hull_dim+1) {
      kjb_fprintf(qh ferr, "\nAt error exit:\n");
      qh_printsummary (qh ferr);
      if (qh PRINTstatistics) {
	qh_collectstatistics();
	qh_printstatistics(qh ferr, "at error exit");
	qh_memstatistics (qh ferr);
      }
    }
    if (qh PRINTprecision)
      qh_printstats (qh ferr, qhstat precision, NULL);
  }
  if (!exitcode)
    exitcode= qh_ERRqhull;
  else if (exitcode == qh_ERRsingular)
    qh_printhelp_singular(qh ferr);
  else if (exitcode == qh_ERRprec && !qh PREmerge)
    qh_printhelp_degenerate (qh ferr);
  if (qh NOerrexit) {
    kjb_fprintf(qh ferr, "qhull error while ending program.  Exit program\n");
    exit(1);
  }
  qh NOerrexit= True;
  longjmp(qh errexit, exitcode);
} /* errexit */


/*-------------------------------------------
-errprint- prints out the information of the erroneous object
    any parameter may be NULL, also prints neighbors and geomview output
*/
void qh_errprint(char *string, facetT *atfacet, facetT *otherfacet, ridgeT *atridge, vertexT *atvertex) {
  int i;

  if (atfacet) {
    kjb_fprintf(qh ferr, "%s FACET:\n", string);
    qh_printfacet(qh ferr, atfacet);
  }
  if (otherfacet) {
    kjb_fprintf(qh ferr, "%s OTHER FACET:\n", string);
    qh_printfacet(qh ferr, otherfacet);
  }
  if (atridge) {
    kjb_fprintf(qh ferr, "%s RIDGE:\n", string);
    qh_printridge(qh ferr, atridge);
    if (atridge->top && atridge->top != atfacet && atridge->top != otherfacet)
      qh_printfacet(qh ferr, atridge->top);
    if (atridge->bottom
	&& atridge->bottom != atfacet && atridge->bottom != otherfacet)
      qh_printfacet(qh ferr, atridge->bottom);
    if (!atfacet)
      atfacet= atridge->top;
    if (!otherfacet)
      otherfacet= otherfacet_(atridge, atfacet);
  }
  if (atvertex) {
    kjb_fprintf(qh ferr, "%s VERTEX:\n", string);
    qh_printvertex (qh ferr, atvertex);
  }
  if (qh FORCEoutput && atfacet && !qh QHULLfinished && !qh IStracing) {
    kjb_fprintf(qh ferr, "ERRONEOUS and NEIGHBORING FACETS to output\n");
    for (i= 0; i< qh_PRINTEND; i++)
      qh_printneighborhood (qh fout, qh PRINTout[i], atfacet, otherfacet,
			    !qh_ALL);
  }
} /* errprint */


/*-----------------------------------------
-printfacetlist- print all fields for a list and/or set of facets to .ferr
  includes all vertices
  if !printall, only prints good facets
*/
void qh_printfacetlist(facetT *facetlist, setT *facets, boolT printall) {
  facetT *facet, **facetp;

  qh_printbegin (qh ferr, qh_PRINTfacets, facetlist, facets, printall);
  FORALLfacet_(facetlist)
    qh_printafacet(qh ferr, qh_PRINTfacets, facet, printall);
  FOREACHfacet_(facets)
    qh_printafacet(qh ferr, qh_PRINTfacets, facet, printall);
  qh_printend (qh ferr, qh_PRINTfacets, facetlist, facets, printall);
} /* printfacetlist */


/*-----------------------------------------
-user_memsizes- allocate up to 10 additional, quick allocation sizes
*/
void qh_user_memsizes (void) {

  /* qh_memsize (size); */
} /* user_memsizes */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif   /* #ifndef __C2MAN__  */


