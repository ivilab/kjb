
/* $Id: mem.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "l/l_incl.h"
/* END kobus */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qhull/qhull.h"
#include "qhull/mem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef qhDEFqhull
void    qh_errexit(int exitcode, void *, void *);
#endif

/* ============ -global data structure ==============
    see mem.h for definition
*/

#ifdef __cplusplus
namespace kjb_c {
#endif

qhmemT qhmem= {0};     /* remove "= {0}" if this causes a compiler error */

#ifdef __cplusplus
}
#endif


#ifndef qh_NOmem

/* internal functions */

static int qh_intcompare(const void *i, const void *j);

/*========== functions in alphabetical order ======== */

/*-------------------------------------------------
-intcompare- used by qsort and bsearch to compare two integers
*/
static int qh_intcompare(const void *i, const void *j) {
  return(*((int *)i) - *((int *)j));
} /* intcompare */


/*-------------------------------------------------
-memalloc- allocates memory for object from qhmem
returns:
 pointer to allocated memory (errors if insufficient memory)
notes:
  use qh_memalloc_() for inline code for quick allocations
  use explicit type conversion to avoid type warnings on some compilers
*/
void *qh_memalloc(int insize) {
  void **freelistp, *newbuffer;
  int index, size;
  int outsize, bufsize;
  void *object;

  if ((unsigned) insize <= (unsigned) qhmem.LASTsize) {
    index= qhmem.indextable[insize];
    freelistp= qhmem.freelists+index;
    if ((object= *freelistp)) {
      qhmem.cntquick++;
      *freelistp= *((void **)*freelistp);  /* replace freelist with next object */
      return (object);
    }else {
      outsize= qhmem.sizetable[index];
      qhmem.cntshort++;
      if (outsize > qhmem .freesize) {
	if (!qhmem.curbuffer)
	  bufsize= qhmem.BUFinit;
        else
	  bufsize= qhmem.BUFsize;
        qhmem.totshort += bufsize;

/* Kobus */
#ifdef USE_KJB_MALLOC
	if (!(newbuffer= KJB_MALLOC(bufsize))) {
#else
	if (!(newbuffer= malloc(bufsize))) {
#endif
/* END kobus */

	  kjb_fprintf(qhmem.ferr, "qhull error (qh_memalloc): insufficient memory\n");
	  qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
	}
	*((void **)newbuffer)= qhmem.curbuffer;  /* prepend newbuffer to curbuffer
						    list */
	qhmem.curbuffer= newbuffer;
        size= (sizeof(void **) + qhmem.ALIGNmask) & ~qhmem.ALIGNmask;
	qhmem.freemem= (void *)((char *)newbuffer+size);
	qhmem.freesize= bufsize - size;
      }
      object= qhmem.freemem;
      qhmem.freemem= (void *)((char *)qhmem.freemem + outsize);
      qhmem.freesize -= outsize;
      return object;
    }
  }else {                     /* long allocation */
    if (!qhmem.indextable) {
      kjb_fprintf (qhmem.ferr, "qhull internal error (qh_memalloc): qhmem has not been initialized.\n");
      qh_errexit (qhmem_ERRqhull, (facetT*)NULL, (ridgeT*)NULL);
    }
    outsize= insize;
    qhmem .cntlong++;
    qhmem .curlong++;
    qhmem .totlong += outsize;
    if (qhmem.maxlong < qhmem.totlong)
      qhmem.maxlong= qhmem.totlong;

/* Kobus */
#ifdef USE_KJB_MALLOC
    if (!(object= KJB_MALLOC(outsize))) {
#else
    if (!(object= malloc(outsize))) {
#endif
/* END kobus */

      kjb_fprintf(qhmem.ferr, "qhull error (qh_memalloc): insufficient memory\n");
      qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
    }
    if (qhmem.IStracing >= 5)
      kjb_fprintf (qhmem.ferr, "qh_memalloc long: %d bytes at %p\n", outsize, object);
  }
  return (object);
} /* memalloc */


/*-------------------------------------------------
-memfree- frees memory object (may be NULL)
  size is insize from qh_memalloc
  type checking warns if using (void **)object
  qh_memfree_()- in-line code for quick free's
*/
void qh_memfree(void *object, int size) {
  void **freelistp;

  if (!object)
    return;
  if (size <= qhmem.LASTsize) {
    qhmem .freeshort++;
    freelistp= qhmem.freelists + qhmem.indextable[size];
    *((void **)object)= *freelistp;
    *freelistp= object;
  }else {
    qhmem .freelong++;
    qhmem .totlong -= size;

/* Kobus */
#ifdef USE_KJB_MALLOC
    kjb_free (object);
#else
    free (object);
#endif
/* END kobus */

    if (qhmem.IStracing >= 5)
      kjb_fprintf (qhmem.ferr, "qh_memfree long: %d bytes at %p\n", size, object);
  }
} /* memfree */


/*-------------------------------------------------
-memfreeshort- frees up all short and qhmem memory allocations
returns: number and size of current long allocations
*/
void qh_memfreeshort (int *curlong, int *totlong) {
  void *buffer, *nextbuffer;

  *curlong= qhmem .cntlong - qhmem .freelong;
  *totlong= qhmem .totlong;
  for(buffer= qhmem.curbuffer; buffer; buffer= nextbuffer) {
    nextbuffer= *((void **) buffer);

/* Kobus */
#ifdef USE_KJB_MALLOC
    kjb_free(buffer);
#else
    free(buffer);
#endif
/* END kobus */

  }
  qhmem.curbuffer= NULL;
  if (qhmem .LASTsize) {

/* Kobus */
#ifdef USE_KJB_MALLOC
    kjb_free (qhmem .indextable);
    kjb_free (qhmem .freelists);
    kjb_free (qhmem .sizetable);
#else
    free (qhmem .indextable);
    free (qhmem .freelists);
    free (qhmem .sizetable);
#endif
/* END kobus */

  }
  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
} /* memfreeshort */


/*-------------------------------------------------
-meminit- initialize memory (memalloc errors until memsetup)
*/
void qh_meminit (FILE *ferr) {

  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
  qhmem.ferr= ferr;
  if (sizeof(void*) < sizeof(int)) {
    kjb_fprintf (ferr, "qhull internal error (qh_meminit): sizeof(void*) < sizeof(int).  set.c will not work\n");
    exit (1);  /* can not use qh_errexit() */
  }
} /* meminit */

/*-------------------------------------------------
-meminitbuffers- initialize memory buffers
*/
void qh_meminitbuffers (int tracelevel, int alignment, int numsizes, int bufsize, int bufinit) {

  qhmem.IStracing= tracelevel;
  qhmem.NUMsizes= numsizes;
  qhmem.BUFsize= bufsize;
  qhmem.BUFinit= bufinit;
  qhmem.ALIGNmask= alignment-1;
  if (qhmem.ALIGNmask & ~qhmem.ALIGNmask) {
    kjb_fprintf (qhmem.ferr, "qhull internal error (qh_meminit): memory alignment %d is not a power of 2\n", alignment);
    qh_errexit (qhmem_ERRqhull, (facetT*)NULL, (ridgeT*)NULL);
  }

/* Kobus */
#ifdef USE_KJB_MALLOC
  qhmem.sizetable= (int *) KJB_CALLOC (numsizes, sizeof(int));
  qhmem.freelists= (void **) KJB_CALLOC (numsizes, sizeof(void *));
#else
  qhmem.sizetable= (int *) calloc (numsizes, sizeof(int));
  qhmem.freelists= (void **) calloc (numsizes, sizeof(void *));
#endif
/* END kobus */

  if (!qhmem.sizetable || !qhmem.freelists) {
    kjb_fprintf(qhmem.ferr, "qhull error (qh_meminit): insufficient memory\n");
    qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
  }
  if (qhmem.IStracing >= 1)
    kjb_fprintf (qhmem.ferr, "qh_meminitbuffers: memory initialized with alignment %d\n", alignment);
} /* meminitbuffers */

/*-------------------------------------------------
-memsetup- set up memory after running memsize()
*/
void qh_memsetup (void) {
  int k,i;

  qsort(qhmem.sizetable, qhmem.TABLEsize, sizeof(int), qh_intcompare);
  qhmem.LASTsize= qhmem.sizetable[qhmem.TABLEsize-1];
  if (qhmem .LASTsize >= qhmem .BUFsize || qhmem.LASTsize >= qhmem .BUFinit) {
    kjb_fprintf (qhmem.ferr, "qhull error (qh_memsetup): largest mem size %d is >= buffer size %d or initial buffer size %d\n",
            qhmem .LASTsize, qhmem .BUFsize, qhmem .BUFinit);
    qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
  }

/* Kobus */
#ifdef USE_KJB_MALLOC
  if (!(qhmem.indextable= (int *)KJB_MALLOC((qhmem.LASTsize+1) * sizeof(int)))) {
#else
  if (!(qhmem.indextable= (int *)malloc((qhmem.LASTsize+1) * sizeof(int)))) {
#endif
/* END kobus */

    kjb_fprintf(qhmem.ferr, "qhull error (qh_memsetup): insufficient memory\n");
    qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
  }
  for(k=qhmem.LASTsize+1; k--; )
    qhmem.indextable[k]= k;
  i= 0;
  for(k= 0; k <= qhmem.LASTsize; k++) {
    if (qhmem.indextable[k] <= qhmem.sizetable[i])
      qhmem.indextable[k]= i;
    else
      qhmem.indextable[k]= ++i;
  }
} /* memsetup */

/*-------------------------------------------------
-memsize- define a free list for this size
*/
void qh_memsize(int size) {
  int k;

  if (qhmem .LASTsize) {
    kjb_fprintf (qhmem .ferr, "qhull error (qh_memsize): called after qhmem_setup\n");
    qh_errexit (qhmem_ERRqhull, (facetT*)NULL, (ridgeT*)NULL);
  }
  size= (size + qhmem.ALIGNmask) & ~qhmem.ALIGNmask;
  for(k= qhmem.TABLEsize; k--; ) {
    if (qhmem.sizetable[k] == size)
      return;
  }
  if (qhmem.TABLEsize < qhmem.NUMsizes)
    qhmem.sizetable[qhmem.TABLEsize++]= size;
  else
    kjb_fprintf(qhmem.ferr, "qhull warning (memsize): free list table has room for only %d sizes\n", qhmem.NUMsizes);
} /* memsize */


/*-------------------------------------------------
-memstatistics-  print out memory statistics
  does not account for wasted memory at the end of each block
*/
void qh_memstatistics (FILE *fp) {
  int i, count, totfree= 0;
  void *object;

  for (i=0; i<qhmem.TABLEsize; i++) {
    count=0;
    for (object= qhmem .freelists[i]; object; object= *((void **)object))
      count++;
    totfree += qhmem.sizetable[i] * count;
  }
  kjb_fprintf (fp, "\nmemory statistics:\n\
%7d quick allocations\n\
%7d short allocations\n\
%7d long allocations\n\
%7d short frees\n\
%7d long frees\n\
%7d bytes of short memory in use\n\
%7d bytes of short memory in freelists\n\
%7d bytes of long memory allocated (except for input)\n\
%7d bytes of long memory in use (in %d pieces)\n\
%7d bytes per memory buffer (initially %d bytes)\n",
	   qhmem .cntquick, qhmem.cntshort, qhmem.cntlong,
	   qhmem .freeshort, qhmem.freelong,
	   qhmem .totshort - qhmem .freesize - totfree,
	   totfree,
	   qhmem .maxlong, qhmem .totlong, qhmem .cntlong - qhmem .freelong,
	   qhmem .BUFsize, qhmem .BUFinit);
  if (qhmem.cntlarger) {
    kjb_fprintf (fp, "%7d calls to qh_setlarger\n%7.2g     average copy size\n",
	   qhmem.cntlarger, ((float) qhmem.totlarger)/ qhmem.cntlarger);
    kjb_fprintf (fp, "  freelists (bytes->count):");
  }
  for (i=0; i<qhmem.TABLEsize; i++) {
    count=0;
    for (object= qhmem .freelists[i]; object; object= *((void **)object))
      count++;
    kjb_fprintf (fp, " %d->%d", qhmem.sizetable[i], count);
  }
  kjb_fprintf (fp, "\n\n");
} /* memstatistics */

#else /* qh_NOmem, use malloc/free instead */

void *qh_memalloc(int insize) {
  void *object;


/* Kobus */
#ifdef USE_KJB_MALLOC
     if (!(object= KJB_MALLOC(insize))) {
#else
     if (!(object= malloc(insize))) {
#endif
/* END kobus */

    kjb_fprintf(qhmem.ferr, "qhull error (qh_memalloc): insufficient memory\n");
    qh_errexit (qhmem_ERRmem, (facetT*)NULL, (ridgeT*)NULL);
  }
  if (qhmem.IStracing >= 5)
    kjb_fprintf (qhmem.ferr, "qh_memalloc long: %d bytes at %p\n", insize, object);
  return object;
}

void qh_memfree(void *object, int size) {

  if (!object)
    return;

/* Kobus */
#ifdef USE_KJB_MALLOC
   kjb_free(object);
#else
   free (object);
#endif
/* END kobus */

  if (qhmem.IStracing >= 5)
    kjb_fprintf (qhmem.ferr, "qh_memfree long: %d bytes at %p\n", size, object);
}

void qh_memfreeshort (int *curlong, int *totlong) {

  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
  *curlong= 0;
  *totlong= 0;
}

void qh_meminit (FILE *ferr) {

  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
  qhmem.ferr= ferr;
  if (sizeof(void*) < sizeof(int)) {
    kjb_fprintf (ferr, "qhull internal error (qh_meminit): sizeof(void*) < sizeof(int).  set.c will not work\n");
    qh_errexit (qhmem_ERRqhull, (facetT*)NULL, (ridgeT*)NULL);
  }
}

void qh_meminitbuffers (int tracelevel, int alignment, int numsizes, int bufsize, int bufinit) {

  qhmem.IStracing= tracelevel;

}

void qh_memsetup (void) {

}

void qh_memsize(int size) {

}

void qh_memstatistics (FILE *fp) {

}

#endif /* qh_NOmem */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif   /* #ifndef __C2MAN__  */


