
/* $Id: io.h 20918 2016-10-31 22:08:27Z kobus $ */

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

#ifndef qhDEFio
#define qhDEFio 1

#include "l/l_sys_def.h" 
#include "l/l_sys_sys.h"
#include "l/l_sys_std.h" 

#include "qhull/qhull.h" 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* ----------------------------------------------
-constants and flags
*/
#define qh_MAXfirst  200   /* maximum length of first two lines of stdin */
#define qh_MINradius 0.02  /* min radius for Gp and Gv, fraction of maxcoord */
#define qh_GEOMepsilon 2e-3 /* adjust outer planes for 'lines closer' and
                            geomview roundoff.  This prevents bleed through */

#define qh_WHITESPACE " \n\t\v\r\f"

/* ======= -functions ===========

see io.c for definitions

	User level functions
-printhelp_degenerate	print help message for degenerate data
-printhelp_singular	print help message for singular data
-produce_output 	prints out the result of qhull in desired format
-readpoints		read input points
-readfeasible           read feasible point from remainder and qh fin
-setfeasible            set feasible point from qh feasible_string

	Print functions for all output formats
-countfacets            count good facets for printing and set visitid
-printafacet		print facet in an output format
-printbegin		print header for an output format
-printend		print trailer for an output format
-printfacets		print facetlist and/or facet set in an output format
-printneighborhood	print neighborhood of one or two facets
-skipfacet		True if not printing this facet
-facet2point            return two temporary projected points for a 2-d facet
-facetvertices          returns temporary set of vertices in a set of facets
-markkeep               mark facets for qh KEEParea, KEEPmerge and KEEPminArea

	Text output functions
-printfacetlist         print facets in a facetlist (see user.c)
-printfacet		print all fields of a facet
-printfacetheader       prints header fields of a facet to fp
-printfacetridges       prints ridges of a facet to fp
-printpoint		print coordinates of a point
-printpoints_out        prints vertices for facets by their point coordinates
-printridge		print all fields for a ridge
-printvertex		print all fields for a vertex
-printvertexlist	print vertices used by a list or set of facets
-printvertices		print a set of vertices
-printvneighbors        print vertex neighbors of vertices in facetlist and facets
-dfacet			print facet by id
-dvertex		print vertex by id

	Mathematica output functions
-printfacet2math	print 2-d Mathematica facet
-printfacet3math	print 3-d Mathematica facet

	Geomview output functions
-printfacet2geom		print facet as a 2-d VECT object
-printfacet2geom_points		print points as a 2-d VECT object with offset
-printfacet3geom_nonsimplicial	print nonsimplicial facet as a 3-d geom object
-printfacet3geom_points		print a set of points as a 3-d OFF object
-printfacet3geom_simplicial	print simplicial facet as a 3-d OFF object
-printfacet4geom_nonsimplicial	print nonsimplicial facet as a 4-d 4OFF object
-printfacet4geom_simplicial	print simplicial facet as a 4-d 4OFF object
-printhyperplaneintersection	print hyperplane intersection as OFF or 4OFF
-printline3geom			prints a line as a VECT
-printpoint3			prints 2-d, 3-d, or 4-d point as 3-d coordinates
-printpointvect			prints a 2-d or 3-d point as 3-d VECT's
-printpointvect2		prints a 2-d or 3-d point as 2 3-d VECT's
-printspheres3			print 3-d vertices as spheres (OFF)
-projectdim3 			project 2-d 3-d or 4-d point to a 3-d point

	Vertex incidence output functions
-order_vertexneighbors		order neighbors for a 3-d vertex by adjacency
-printfacet3vertex		print vertices for a 3-d facet
-printfacetNvertex_nonsimplicial print vertices for an n-d non-simplicial facet
-printfacetNvertex_simplicial	print vertices for an n-d simplicial facet
-printvoronoi                   prints facet centers and for each vertex, print ids
*/

/*---------- -prototypes in alphabetical order -----------*/

void    qh_countfacets (facetT *facetlist, setT *facets, boolT printall,
              int *numfacetsp, int *numsimplicialp, int *totneighborsp,
              int *numridgesp, int *numcoplanarsp);
void    dfacet( int id);
void    dvertex( int id);
void	qh_facet2point(facetT *facet, pointT **point0, pointT **point1, realT *mindist);
setT   *qh_facetvertices (facetT *facetlist, setT *facets, boolT allfacets);
void    qh_markkeep (facetT *facetlist);
void    qh_order_vertexneighbors(vertexT *vertex);
void	qh_printafacet(FILE *fp, int format, facetT *facet, boolT printall);
void    qh_printbegin (FILE *fp, int format, facetT *facetlist, setT *facets, boolT printall);
void 	qh_printcenter (FILE *fp, int format, char *string, facetT *facet);
void    qh_printcentrum (FILE *fp, facetT *facet, realT radius);
void    qh_printend (FILE *fp, int format, facetT *facetlist, setT *facets, boolT printall);
void    qh_printend4geom (FILE *fp, facetT *facet, int *num, boolT printall);
void	qh_printfacet(FILE *fp, facetT *facet);
void	qh_printfacet2math(FILE *fp, facetT *facet, int notfirst);
void	qh_printfacet2geom(FILE *fp, facetT *facet, realT color[3]);
void    qh_printfacet2geom_points(FILE *fp, pointT *point1, pointT *point2,
			       facetT *facet, realT offset, realT color[3]);
void	qh_printfacet3math (FILE *fp, facetT *facet, int notfirst);
void	qh_printfacet3geom_nonsimplicial(FILE *fp, facetT *facet, realT color[3]);
void	qh_printfacet3geom_points(FILE *fp, setT *points, facetT *facet, realT offset, realT color[3]);
void	qh_printfacet3geom_simplicial(FILE *fp, facetT *facet, realT color[3]);
void	qh_printfacet3vertex(FILE *fp, facetT *facet, int format);
void	qh_printfacet4geom_nonsimplicial(FILE *fp, facetT *facet, realT color[3]);
void	qh_printfacet4geom_simplicial(FILE *fp, facetT *facet, realT color[3]);
void	qh_printfacetNvertex_nonsimplicial(FILE *fp, facetT *facet, int id);
void	qh_printfacetNvertex_simplicial(FILE *fp, facetT *facet, int format);
void    qh_printfacetheader(FILE *fp, facetT *facet);
void    qh_printfacetridges(FILE *fp, facetT *facet);
void	qh_printfacets(FILE *fp, int format, facetT *facetlist, setT *facets, boolT printall);
void	qh_printhelp_degenerate(FILE *fp);
void	qh_printhelp_singular(FILE *fp);
void	qh_printhyperplaneintersection(FILE *fp, facetT *facet1, facetT *facet2,
  		   setT *vertices, realT color[3]);
void	qh_printneighborhood (FILE *fp, int format, facetT *facetA, facetT *facetB, boolT printall);
void    qh_printline3geom (FILE *fp, pointT *pointA, pointT *pointB, realT color[3]);
void	qh_printpoint(FILE *fp, char *string, pointT *point);
void    qh_printpoint3 (FILE *fp, pointT *point);
void    qh_printpoints_out (FILE *fp, facetT *facetlist, setT *facets, int printall);
void    qh_printpointvect (FILE *fp, pointT *point, coordT *normal, pointT *center, realT radius, realT color[3]);
void    qh_printpointvect2 (FILE *fp, pointT *point, coordT *normal, pointT *center, realT radius);
void	qh_printridge(FILE *fp, ridgeT *ridge);
void    qh_printspheres(FILE *fp, setT *vertices, realT radius);
void	qh_printvertex(FILE *fp, vertexT *vertex);
void	qh_printvertexlist (FILE *fp, char* string, facetT *facetlist,
                         setT *facets, boolT printall);
void	qh_printvertices (FILE *fp, char* string, setT *vertices);
void    qh_printvneighbors (FILE *fp, facetT* facetlist, setT *facets, boolT printall);
void    qh_printvoronoi (FILE *fp, int format, facetT *facetlist, setT *facets, boolT printall);
void	qh_produce_output(void);
void    qh_projectdim3 (pointT *source, pointT *destination);
int     qh_readfeasible (int dim, char *remainder);
coordT *qh_readpoints(int *numpoints, int *dimension, boolT *ismalloc);
void    qh_setfeasible (int dim);
boolT	qh_skipfacet(facetT *facet);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* qhDEFio */

/* -------------------------------------------------------------------------- */

#endif   /* #ifndef __C2MAN__  */


