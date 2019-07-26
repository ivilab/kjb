
/* $Id: set.h 20918 2016-10-31 22:08:27Z kobus $ */

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

#ifndef qhDEFset
#define qhDEFset 1

#include "l/l_sys_def.h" 
#include "l/l_sys_sys.h"
#include "l/l_sys_std.h" 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* ================= -structures- ===============
*/
#ifndef DEFsetT
#define DEFsetT 1
typedef struct setT setT;   /* a set is a sorted or unsorted array of pointers */
#endif

/* ----------------------------------------------
-setT- a set or list of pointers with maximum size and actual size.
 The set/list is terminated with NULL
 The maxsize element (e[set->maxsize]) is:
   NULL                if actual size equals maximum size
   actual_size + 1     if actual size is less than maximum size
 styles of setT:
   unique              a set of unique elements, usually ordered
   unordered           a collection of elements with duplicates allowed
   table               a collection of elements with 'NULL' allowed
                       can not use FOREACH... because of internal NULLs
*/
typedef union setelemT setelemT;
union setelemT {
  void    *p;
  int      i;         /* integer used for e[maxSize] */
};

struct setT {
  unsigned int maxsize; /* maximum number of elements (except NULL) */
  setelemT e[1];        /* array of pointers, tail is NULL */
                        /* last slot (unless NULL) is actual size+1
                           e[maxsize]==NULL or e[e[maxsize]-1]==NULL */
};

/* ----------------------------------------------
-constants and flags
*/
#define SETelemsize sizeof(setelemT) /* size of set element in bytes */


/* =========== -macros- ========================= */

/*-----------------------------------------------
-FOREACHsetelement_(type, set, variable)- define FOREACH iterator
    variable is NULL at end of loop
    assumes *variable and **variablep are declared
    variablep is one beyond variable.
    to repeat an element,
          variablep--; / *repeat* /
    use FOREACHsetelement_i_() if need index or include NULLs
    WARNING: strange behavior if don't use braces when nested

-FOREACHsetelement_i_(type, set, variable)- define FOREACH iterator
    assumes *variable, variable_n, and variable_i are declared
    variable_i is index, variable_n is qh_setsize()
    variable may be NULL inside looop
    at end of loop, variable==NULL and variable_i==variable_n
    variable_i--; variable_n-- repeats for deleted element

-FOREACHsetelementreverse_
    same as FOREACHsetelement but returns elements in reverse order
    uses 'int variabletemp'

-FOREACHsetelementreverse12_
    same as FOREACHsetelement but returns e[1], e[0], e[2], e[3],

-FOREACHelem_(set)- for each element in a set of elements
-FOREACHset_(sets)- for each set in a set of sets
-SETindex_(set,elem)- returns index for iterated elem in set
*/
#define FOREACHsetelement_(type, set, variable) \
        if (set || (variable= NULL)) for(\
          variable##p= (type **)&((set)->e[0].p); \
	  (variable= *variable##p++);)
#define FOREACHsetelement_i_(type, set, variable) \
        if (set || (variable= NULL)) for (\
          variable##_i= 0, variable= (type *)((set)->e[0].p), \
                   variable##_n= qh_setsize(set);\
          variable##_i < variable##_n;\
          variable= (type *)((set)->e[++variable##_i].p) )
#define FOREACHsetelementreverse_(type, set, variable) \
        if (set || (variable= NULL)) for(\
	   variable##temp= qh_setsize(set)-1, variable= qh_setlast(set);\
	   variable; variable= \
	   ((--variable##temp >= 0) ? SETelem_(set, variable##temp) : NULL))
#define FOREACHsetelementreverse12_(type, set, variable) \
        if (set || (variable= NULL)) for(\
          variable##p= (type **)&((set)->e[1].p); \
	  (variable= *variable##p); \
          variable##p == ((type **)&((set)->e[0].p))?variable##p += 2: \
	      (variable##p == ((type **)&((set)->e[1].p))?variable##p--:variable##p++))
#define FOREACHelem_(set) FOREACHsetelement_(void, set, elem)
#define FOREACHset_(sets) FOREACHsetelement_(setT, sets, set)
#define SETindex_(set, elem) ((void **)elem##p - (void **)&(set)->e[1].p)
#define SETref_(elem) (elem##p[-1])

/*-----------------------------------------------
-SETelem_(set, n)- return the n'th element of set
      assumes that n is valid [0..size] and that set is defined
      may need a type cast

-SETelemaddr_(set, n, type)-return address of the n'th element of a set
      assumes that n is valid [0..size] and set is defined

-SETfirst_(set)- return first element of set
-SETsecond_(set)- return second element of set
-SETaddr_(set, type)-   return address of set's elements
*/
#define SETelem_(set, n)           ((set)->e[n].p)
#define SETelemaddr_(set, n, type) ((type **)(&((set)->e[n].p)))
#define SETfirst_(set)             ((set)->e[0].p)
#define SETsecond_(set)            ((set)->e[1].p)
#define SETaddr_(set,type)	   ((type **)(&((set)->e[0].p)))

/*-----------------------------------------------
-SETreturnsize_(set, size) - return size of a set
      set must be defined
      use qh_setsize(set) unless speed is critical

-SETempty_(set) - return true (1) if set is empty
      set may be NULL
*/
#define SETreturnsize_(set, size) (((size)= ((set)->e[(set)->maxsize].i))?(--(size)):((size)= (set)->maxsize))
#define SETempty_(set) 	          (!set || (SETfirst_(set) ? 0:1))

/* ======= -functions ===========

   see set.c for function definitions

	Add functions
-setaddsorted	    adds an element to a sorted set
-setaddnth	    adds newelem as n'th element of sorted or unsorted set
-setappend	    appends an element to a set
-setappend_set      appends a set to a set
-setappend2ndlast   makes newelem the next to the last element in set
-setlarger	    returns a larger set that contains elements of *setp
-setreplace	    replaces oldelem in set with newelem
-setunique	    add an element if not already in set

	Access and predicate functions	
-setin		    returns 1 if setelem is in a set, 0 otherwise
-setindex	    returns the index of elem in set.   If none, returns -1
-setlast	    return last element of set or NULL
-setequal	    returns 1 if two sorted sets are equal, otherwise returns 0
-setequal_except    returns 1 if two sorted sets are equal except at element
-setequal_skip	    returns 1 if two sorted sets are equal except for skips

	Delete functions
-setdel		    deletes oldelem from unsorted set.
-setdelsorted	    deletes oldelem from sorted set
-setdelnth	    delete and return nth element from unsorted set
-setdelnthsorted    delete and return nth element from sorted set
-setdellast	    delete and return last element from set or NULL
-setnew_delnthsorted create a sorted set not containing nth element

	Allocation and deallocation functions
-setnew		    create a new set
-setfree	    free the space occupied by a sorted or unsorted set
-setfreelong	    frees a set only if it's in long memory
-setfree2           free a set and its elements

	Temporary set functions
-settemp	    return a stacked, temporary set
-settempfree	    free temporary set at top of qhmem.tempstack
-settemppop	    pop qhmem.tempstack (makes temporary set permanent)
-settemppush	    push temporary set unto qhmem.tempstack (makes it temporary)
-settempfree_all    free all temporary sets in qhmem.tempstack

	Other functions
-setsize	    returns the size of a set
-setcompact	    compact NULLs in an unsorted set
-setcopy	    copies a sorted or unsorted set into another
-setcheck	    check set for validity
-setduplicate       duplicate a set of elements
-setprint	    print set elements to fp
-settruncate        truncate set to size elements
-setzero            zero remainder of set and set to maximum size
*/

/*---------- -prototypes in alphabetical order -----------*/

void  qh_setaddsorted(setT **setp, void *elem);
void  qh_setaddnth(setT **setp, int nth, void *newelem);
void  qh_setappend(setT **setp, void *elem);
void  qh_setappend_set(setT **setp, setT *setA);
void  qh_setappend2ndlast(setT **setp, void *elem);
void  qh_setcheck(setT *set, char *typename_arg, int id);
void  qh_setcompact(setT *set);
setT *qh_setcopy(setT *set, int extra);
void *qh_setdel(setT *set, void *elem);
void *qh_setdellast(setT *set);
void *qh_setdelnth(setT *set, int nth);
void *qh_setdelnthsorted(setT *set, int nth);
void *qh_setdelsorted(setT *set, void *newelem);
setT *qh_setduplicate( setT *set, int elemsize);
int   qh_setequal(setT *setA, setT *setB);
int   qh_setequal_except (setT *setA, void *skipelemA, setT *setB, void *skipelemB);
int   qh_setequal_skip (setT *setA, int skipA, setT *setB, int skipB);
void  qh_setfree(setT **set);
void  qh_setfree2( setT **setp, int elemsize);
void  qh_setfreelong(setT **set);
int   qh_setin(setT *set, void *setelem);
int   qh_setindex(setT *set, void *setelem);
void  qh_setlarger(setT **setp);
void *qh_setlast(setT *set);
setT *qh_setnew(int size);
setT *qh_setnew_delnthsorted(setT *set, int size, int nth, int prepend);
void  qh_setprint(FILE *fp, char* string, setT *set);
void  qh_setreplace(setT *set, void *oldelem, void *newelem);
int   qh_setsize(setT *set);
setT *qh_settemp(int setsize);
void  qh_settempfree(setT **set);
void  qh_settempfree_all(void);
setT *qh_settemppop(void);
void  qh_settemppush(setT *set);
void  qh_settruncate (setT *set, int size);
int   qh_setunique (setT **set, void *elem);
void  qh_setzero (setT *set, int index, int size);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* qhDEFset */

/* -------------------------------------------------------------------------- */

#endif   /* #ifndef __C2MAN__  */


