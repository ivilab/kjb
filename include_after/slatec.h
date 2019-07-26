
/* =========================================================================== *
|
|  Copyright (c) 1994-2002 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowleged in publications, and relevent papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarentee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#ifndef SLATEC_INCLUDED
#define SLATEC_INCLUDED


#include "m/m_gen.h"


#ifdef __cplusplus
/*
 * Actually, these are fortran routines, but this seems to work.
*/
extern "C"
{
#endif


#ifdef HPUX
#    define dlsei_ dlsei
#    define dbocls_ dbocls
#    define d1mach_ d1mach
#    define i1mach_ i1mach
#endif

/* -------------------------------------------------------------------------- */

/*

* *DECK DLSEI
*       SUBROUTINE DLSEI (W, MDW, ME, MA, MG, N, PRGOPT, X, RNORME,
*      +   RNORML, MODE, WS, IP)
* C***BEGIN PROLOGUE  DLSEI
* C***PURPOSE  Solve a linearly constrained least squares problem with
* C            equality and inequality constraints, and optionally compute
* C            a covariance matrix.
* C***LIBRARY   SLATEC
* C***CATEGORY  K1A2A, D9
* C***TYPE      DOUBLE PRECISION (LSEI-S, DLSEI-D)
* C***KEYWORDS  CONSTRAINED LEAST SQUARES, CURVE FITTING, DATA FITTING,
* C             EQUALITY CONSTRAINTS, INEQUALITY CONSTRAINTS,
* C             QUADRATIC PROGRAMMING
* C***AUTHOR  Hanson, R. J., (SNLA)
* C           Haskell, K. H., (SNLA)
* C***DESCRIPTION
* C
* C     Abstract
* C
* C     This subprogram solves a linearly constrained least squares
* C     problem with both equality and inequality constraints, and, if the
* C     user requests, obtains a covariance matrix of the solution
* C     parameters.
* C
* C     Suppose there are given matrices E, A and G of respective
* C     dimensions ME by N, MA by N and MG by N, and vectors F, B and H of
* C     respective lengths ME, MA and MG.  This subroutine solves the
* C     linearly constrained least squares problem
* C
* C                   EX = F, (E ME by N) (equations to be exactly
* C                                       satisfied)
* C                   AX = B, (A MA by N) (equations to be
* C                                       approximately satisfied,
* C                                       least squares sense)
* C                   GX .GE. H,(G MG by N) (inequality constraints)
* C
* C     The inequalities GX .GE. H mean that every component of the
* C     product GX must be .GE. the corresponding component of H.
* C
* C     In case the equality constraints cannot be satisfied, a
* C     generalized inverse solution residual vector length is obtained
* C     for F-EX.  This is the minimal length possible for F-EX.
* C
* C     Any values ME .GE. 0, MA .GE. 0, or MG .GE. 0 are permitted.  The
* C     rank of the matrix E is estimated during the computation.  We call
* C     this value KRANKE.  It is an output parameter in IP(1) defined
* C     below.  Using a generalized inverse solution of EX=F, a reduced
* C     least squares problem with inequality constraints is obtained.
* C     The tolerances used in these tests for determining the rank
* C     of E and the rank of the reduced least squares problem are
* C     given in Sandia Tech. Rept. SAND-78-1290.  They can be
* C     modified by the user if new values are provided in
* C     the option list of the array PRGOPT(*).
* C
* C     The user must dimension all arrays appearing in the call list..
* C     W(MDW,N+1),PRGOPT(*),X(N),WS(2*(ME+N)+K+(MG+2)*(N+7)),IP(MG+2*N+2)
* C     where K=MAX(MA+MG,N).  This allows for a solution of a range of
* C     problems in the given working space.  The dimension of WS(*)
* C     given is a necessary overestimate.  Once a particular problem
* C     has been run, the output parameter IP(3) gives the actual
* C     dimension required for that problem.
* C
* C     The parameters for DLSEI( ) are
* C
* C     Input.. All TYPE REAL variables are DOUBLE PRECISION
* C
* C     W(*,*),MDW,   The array W(*,*) is doubly subscripted with
* C     ME,MA,MG,N    first dimensioning parameter equal to MDW.
* C                   For this discussion let us call M = ME+MA+MG.  Then
* C                   MDW must satisfy MDW .GE. M.  The condition
* C                   MDW .LT. M is an error.
* C
* C                   The array W(*,*) contains the matrices and vectors
* C
* C                                  (E  F)
* C                                  (A  B)
* C                                  (G  H)
* C
* C                   in rows and columns 1,...,M and 1,...,N+1
* C                   respectively.
* C
* C                   The integers ME, MA, and MG are the
* C                   respective matrix row dimensions
* C                   of E, A and G.  Each matrix has N columns.
* C
* C     PRGOPT(*)    This real-valued array is the option vector.
* C                  If the user is satisfied with the nominal
* C                  subprogram features set
* C
* C                  PRGOPT(1)=1 (or PRGOPT(1)=1.0)
* C
* C                  Otherwise PRGOPT(*) is a linked list consisting of
* C                  groups of data of the following form
* C
* C                  LINK
* C                  KEY
* C                  DATA SET
* C
* C                  The parameters LINK and KEY are each one word.
* C                  The DATA SET can be comprised of several words.
* C                  The number of items depends on the value of KEY.
* C                  The value of LINK points to the first
* C                  entry of the next group of data within
* C                  PRGOPT(*).  The exception is when there are
* C                  no more options to change.  In that
* C                  case, LINK=1 and the values KEY and DATA SET
* C                  are not referenced.  The general layout of
* C                  PRGOPT(*) is as follows.
* C
* C               ...PRGOPT(1) = LINK1 (link to first entry of next group)
* C               .  PRGOPT(2) = KEY1 (key to the option change)
* C               .  PRGOPT(3) = data value (data value for this change)
* C               .       .
* C               .       .
* C               .       .
* C               ...PRGOPT(LINK1)   = LINK2 (link to the first entry of
* C               .                       next group)
* C               .  PRGOPT(LINK1+1) = KEY2 (key to the option change)
* C               .  PRGOPT(LINK1+2) = data value
* C               ...     .
* C               .       .
* C               .       .
* C               ...PRGOPT(LINK) = 1 (no more options to change)
* C
* C                  Values of LINK that are nonpositive are errors.
* C                  A value of LINK .GT. NLINK=100000 is also an error.
* C                  This helps prevent using invalid but positive
* C                  values of LINK that will probably extend
* C                  beyond the program limits of PRGOPT(*).
* C                  Unrecognized values of KEY are ignored.  The
* C                  order of the options is arbitrary and any number
* C                  of options can be changed with the following
* C                  restriction.  To prevent cycling in the
* C                  processing of the option array, a count of the
* C                  number of options changed is maintained.
* C                  Whenever this count exceeds NOPT=1000, an error
* C                  message is printed and the subprogram returns.
* C
* C                  Options..
* C
* C                  KEY=1
* C                         Compute in W(*,*) the N by N
* C                  covariance matrix of the solution variables
* C                  as an output parameter.  Nominally the
* C                  covariance matrix will not be computed.
* C                  (This requires no user input.)
* C                  The data set for this option is a single value.
* C                  It must be nonzero when the covariance matrix
* C                  is desired.  If it is zero, the covariance
* C                  matrix is not computed.  When the covariance matrix
* C                  is computed, the first dimensioning parameter
* C                  of the array W(*,*) must satisfy MDW .GE. MAX(M,N).
* C
* C                  KEY=10
* C                         Suppress scaling of the inverse of the
* C                  normal matrix by the scale factor RNORM**2/
* C                  MAX(1, no. of degrees of freedom).  This option
* C                  only applies when the option for computing the
* C                  covariance matrix (KEY=1) is used.  With KEY=1 and
* C                  KEY=10 used as options the unscaled inverse of the
* C                  normal matrix is returned in W(*,*).
* C                  The data set for this option is a single value.
* C                  When it is nonzero no scaling is done.  When it is
* C                  zero scaling is done.  The nominal case is to do
* C                  scaling so if option (KEY=1) is used alone, the
* C                  matrix will be scaled on output.
* C
* C                  KEY=2
* C                         Scale the nonzero columns of the
* C                         entire data matrix.
* C                  (E)
* C                  (A)
* C                  (G)
* C
* C                  to have length one.  The data set for this
* C                  option is a single value.  It must be
* C                  nonzero if unit length column scaling
* C                  is desired.
* C
* C                  KEY=3
* C                         Scale columns of the entire data matrix
* C                  (E)
* C                  (A)
* C                  (G)
* C
* C                  with a user-provided diagonal matrix.
* C                  The data set for this option consists
* C                  of the N diagonal scaling factors, one for
* C                  each matrix column.
* C
* C                  KEY=4
* C                         Change the rank determination tolerance for
* C                  the equality constraint equations from
* C                  the nominal value of SQRT(DRELPR).  This quantity can
* C                  be no smaller than DRELPR, the arithmetic-
* C                  storage precision.  The quantity DRELPR is the
* C                  largest positive number such that T=1.+DRELPR
* C                  satisfies T .EQ. 1.  The quantity used
* C                  here is internally restricted to be at
* C                  least DRELPR.  The data set for this option
* C                  is the new tolerance.
* C
* C                  KEY=5
* C                         Change the rank determination tolerance for
* C                  the reduced least squares equations from
* C                  the nominal value of SQRT(DRELPR).  This quantity can
* C                  be no smaller than DRELPR, the arithmetic-
* C                  storage precision.  The quantity used
* C                  here is internally restricted to be at
* C                  least DRELPR.  The data set for this option
* C                  is the new tolerance.
* C
* C                  For example, suppose we want to change
* C                  the tolerance for the reduced least squares
* C                  problem, compute the covariance matrix of
* C                  the solution parameters, and provide
* C                  column scaling for the data matrix.  For
* C                  these options the dimension of PRGOPT(*)
* C                  must be at least N+9.  The Fortran statements
* C                  defining these options would be as follows:
* C
* C                  PRGOPT(1)=4 (link to entry 4 in PRGOPT(*))
* C                  PRGOPT(2)=1 (covariance matrix key)
* C                  PRGOPT(3)=1 (covariance matrix wanted)
* C
* C                  PRGOPT(4)=7 (link to entry 7 in PRGOPT(*))
* C                  PRGOPT(5)=5 (least squares equas.  tolerance key)
* C                  PRGOPT(6)=... (new value of the tolerance)
* C
* C                  PRGOPT(7)=N+9 (link to entry N+9 in PRGOPT(*))
* C                  PRGOPT(8)=3 (user-provided column scaling key)
* C
* C                  CALL DCOPY (N, D, 1, PRGOPT(9), 1)  (Copy the N
* C                    scaling factors from the user array D(*)
* C                    to PRGOPT(9)-PRGOPT(N+8))
* C
* C                  PRGOPT(N+9)=1 (no more options to change)
* C
* C                  The contents of PRGOPT(*) are not modified
* C                  by the subprogram.
* C                  The options for WNNLS( ) can also be included
* C                  in this array.  The values of KEY recognized
* C                  by WNNLS( ) are 6, 7 and 8.  Their functions
* C                  are documented in the usage instructions for
* C                  subroutine WNNLS( ).  Normally these options
* C                  do not need to be modified when using DLSEI( ).
* C
* C     IP(1),       The amounts of working storage actually
* C     IP(2)        allocated for the working arrays WS(*) and
* C                  IP(*), respectively.  These quantities are
* C                  compared with the actual amounts of storage
* C                  needed by DLSEI( ).  Insufficient storage
* C                  allocated for either WS(*) or IP(*) is an
* C                  error.  This feature was included in DLSEI( )
* C                  because miscalculating the storage formulas
* C                  for WS(*) and IP(*) might very well lead to
* C                  subtle and hard-to-find execution errors.
* C
* C                  The length of WS(*) must be at least
* C
* C                  LW = 2*(ME+N)+K+(MG+2)*(N+7)
* C
* C                  where K = max(MA+MG,N)
* C                  This test will not be made if IP(1).LE.0.
* C
* C                  The length of IP(*) must be at least
* C
* C                  LIP = MG+2*N+2
* C                  This test will not be made if IP(2).LE.0.
* C
* C     Output.. All TYPE REAL variables are DOUBLE PRECISION
* C
* C     X(*),RNORME,  The array X(*) contains the solution parameters
* C     RNORML        if the integer output flag MODE = 0 or 1.
* C                   The definition of MODE is given directly below.
* C                   When MODE = 0 or 1, RNORME and RNORML
* C                   respectively contain the residual vector
* C                   Euclidean lengths of F - EX and B - AX.  When
* C                   MODE=1 the equality constraint equations EX=F
* C                   are contradictory, so RNORME .NE. 0.  The residual
* C                   vector F-EX has minimal Euclidean length.  For
* C                   MODE .GE. 2, none of these parameters is defined.
* C
* C     MODE          Integer flag that indicates the subprogram
* C                   status after completion.  If MODE .GE. 2, no
* C                   solution has been computed.
* C
* C                   MODE =
* C
* C                   0  Both equality and inequality constraints
* C                      are compatible and have been satisfied.
* C
* C                   1  Equality constraints are contradictory.
* C                      A generalized inverse solution of EX=F was used
* C                      to minimize the residual vector length F-EX.
* C                      In this sense, the solution is still meaningful.
* C
* C                   2  Inequality constraints are contradictory.
* C
* C                   3  Both equality and inequality constraints
* C                      are contradictory.
* C
* C                   The following interpretation of
* C                   MODE=1,2 or 3 must be made.  The
* C                   sets consisting of all solutions
* C                   of the equality constraints EX=F
* C                   and all vectors satisfying GX .GE. H
* C                   have no points in common.  (In
* C                   particular this does not say that
* C                   each individual set has no points
* C                   at all, although this could be the
* C                   case.)
* C
* C                   4  Usage error occurred.  The value
* C                      of MDW is .LT. ME+MA+MG, MDW is
* C                      .LT. N and a covariance matrix is
* C                      requested, or the option vector
* C                      PRGOPT(*) is not properly defined,
* C                      or the lengths of the working arrays
* C                      WS(*) and IP(*), when specified in
* C                      IP(1) and IP(2) respectively, are not
* C                      long enough.
* C
* C     W(*,*)        The array W(*,*) contains the N by N symmetric
* C                   covariance matrix of the solution parameters,
* C                   provided this was requested on input with
* C                   the option vector PRGOPT(*) and the output
* C                   flag is returned with MODE = 0 or 1.
* C
* C     IP(*)         The integer working array has three entries
* C                   that provide rank and working array length
* C                   information after completion.
* C
* C                      IP(1) = rank of equality constraint
* C                              matrix.  Define this quantity
* C                              as KRANKE.
* C
* C                      IP(2) = rank of reduced least squares
* C                              problem.
* C
* C                      IP(3) = the amount of storage in the
* C                              working array WS(*) that was
* C                              actually used by the subprogram.
* C                              The formula given above for the length
* C                              of WS(*) is a necessary overestimate.
* C                              If exactly the same problem matrices
* C                              are used in subsequent executions,
* C                              the declared dimension of WS(*) can
* C                              be reduced to this output value.
* C     User Designated
* C     Working Arrays..
* C
* C     WS(*),IP(*)              These are respectively type real
* C                              and type integer working arrays.
* C                              Their required minimal lengths are
* C                              given above.
* C
* C***REFERENCES  K. H. Haskell and R. J. Hanson, An algorithm for
* C                 linear least squares problems with equality and
* C                 nonnegativity constraints, Report SAND77-0552, Sandia
* C                 Laboratories, June 1978.
* C               K. H. Haskell and R. J. Hanson, Selected algorithms for
* C                 the linearly constrained least squares problem - a
* C                 users guide, Report SAND78-1290, Sandia Laboratories,
* C                 August 1979.
* C               K. H. Haskell and R. J. Hanson, An algorithm for
* C                 linear least squares problems with equality and
* C                 nonnegativity constraints, Mathematical Programming
* C                 21 (1981), pp. 98-118.
* C               R. J. Hanson and K. H. Haskell, Two algorithms for the
* C                 linearly constrained least squares problem, ACM
* C                 Transactions on Mathematical Software, September 1982.
* C***ROUTINES CALLED  D1MACH, DASUM, DAXPY, DCOPY, DDOT, DH12, DLSI,
* C                    DNRM2, DSCAL, DSWAP, XERMSG
* C***REVISION HISTORY  (YYMMDD)
* C   790701  DATE WRITTEN
* C   890531  Changed all specific intrinsics to generic.  (WRB)
* C   890618  Completely restructured and extensively revised (WRB & RWC)
* C   890831  REVISION DATE from Version 3.2
* C   891214  Prologue converted to Version 4.0 format.  (BAB)
* C   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ)
* C   900510  Convert XERRWV calls to XERMSG calls.  (RWC)
* C   900604  DP version created from SP version.  (RWC)
* C   920501  Reformatted the REFERENCES section.  (WRB)
* C      END
*/


void dlsei_
(
    double*    W,
    int* MDW,
    int* ME,
    int* MA,
    int* MG,
    int* N,
    double*    PRGOPT,
    double*    X,
    double*    RNORME,
    double*    RNORML,
    int* MODE,
    double*    WS,
    int* IP
);


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*

* *DECK DBOCLS
*       SUBROUTINE DBOCLS (W, MDW, MCON, MROWS, NCOLS, BL, BU, IND, IOPT,
*      +   X, RNORMC, RNORM, MODE, RW, IW)
* C***BEGIN PROLOGUE  DBOCLS
* C***PURPOSE  Solve the bounded and constrained least squares
* C            problem consisting of solving the equation
* C                      E*X = F  (in the least squares sense)
* C             subject to the linear constraints
* C                            C*X = Y.
* C***LIBRARY   SLATEC
* C***CATEGORY  K1A2A, G2E, G2H1, G2H2
* C***TYPE      DOUBLE PRECISION (SBOCLS-S, DBOCLS-D)
* C***KEYWORDS  BOUNDS, CONSTRAINTS, INEQUALITY, LEAST SQUARES, LINEAR
* C***AUTHOR  Hanson, R. J., (SNLA)
* C***DESCRIPTION
* C
* C   **** All INPUT and OUTPUT real variables are DOUBLE PRECISION ****
* C
* C     This subprogram solves the bounded and constrained least squares
* C     problem. The problem statement is:
* C
* C     Solve E*X = F (least squares sense), subject to constraints
* C     C*X=Y.
* C
* C     In this formulation both X and Y are unknowns, and both may
* C     have bounds on any of their components.  This formulation
* C     of the problem allows the user to have equality and inequality
* C     constraints as well as simple bounds on the solution components.
* C
* C     This constrained linear least squares subprogram solves E*X=F
* C     subject to C*X=Y, where E is MROWS by NCOLS, C is MCON by NCOLS.
* C
* C      The user must have dimension statements of the form
* C
* C      DIMENSION W(MDW,NCOLS+MCON+1), BL(NCOLS+MCON), BU(NCOLS+MCON),
* C     * X(2*(NCOLS+MCON)+2+NX), RW(6*NCOLS+5*MCON)
* C       INTEGER IND(NCOLS+MCON), IOPT(17+NI), IW(2*(NCOLS+MCON))
* C
* C     (here NX=number of extra locations required for the options; NX=0
* C     if no options are in use. Also NI=number of extra locations
* C     for options 1-9.)
* C
* C    INPUT
* C    -----
* C
* C    -------------------------
* C    W(MDW,*),MCON,MROWS,NCOLS
* C    -------------------------
* C     The array W contains the (possibly null) matrix [C:*] followed by
* C     [E:F].  This must be placed in W as follows:
* C          [C  :  *]
* C     W  = [       ]
* C          [E  :  F]
* C     The (*) after C indicates that this data can be undefined. The
* C     matrix [E:F] has MROWS rows and NCOLS+1 columns. The matrix C is
* C     placed in the first MCON rows of W(*,*) while [E:F]
* C     follows in rows MCON+1 through MCON+MROWS of W(*,*). The vector F
* C     is placed in rows MCON+1 through MCON+MROWS, column NCOLS+1. The
* C     values of MDW and NCOLS must be positive; the value of MCON must
* C     be nonnegative. An exception to this occurs when using option 1
* C     for accumulation of blocks of equations. In that case MROWS is an
* C     OUTPUT variable only, and the matrix data for [E:F] is placed in
* C     W(*,*), one block of rows at a time. See IOPT(*) contents, option
* C     number 1, for further details. The row dimension, MDW, of the
* C     array W(*,*) must satisfy the inequality:
* C
* C     If using option 1,
* C                     MDW .ge. MCON + max(max. number of
* C                     rows accumulated, NCOLS) + 1.
* C     If using option 8,
* C                     MDW .ge. MCON + MROWS.
* C     Else
* C                     MDW .ge. MCON + max(MROWS, NCOLS).
* C
* C     Other values are errors, but this is checked only when using
* C     option=2.  The value of MROWS is an output parameter when
* C     using option number 1 for accumulating large blocks of least
* C     squares equations before solving the problem.
* C     See IOPT(*) contents for details about option 1.
* C
* C    ------------------
* C    BL(*),BU(*),IND(*)
* C    ------------------
* C     These arrays contain the information about the bounds that the
* C     solution values are to satisfy. The value of IND(J) tells the
* C     type of bound and BL(J) and BU(J) give the explicit values for
* C     the respective upper and lower bounds on the unknowns X and Y.
* C     The first NVARS entries of IND(*), BL(*) and BU(*) specify
* C     bounds on X; the next MCON entries specify bounds on Y.
* C
* C    1.    For IND(J)=1, require X(J) .ge. BL(J);
* C          IF J.gt.NCOLS,        Y(J-NCOLS) .ge. BL(J).
* C          (the value of BU(J) is not used.)
* C    2.    For IND(J)=2, require X(J) .le. BU(J);
* C          IF J.gt.NCOLS,        Y(J-NCOLS) .le. BU(J).
* C          (the value of BL(J) is not used.)
* C    3.    For IND(J)=3, require X(J) .ge. BL(J) and
* C                                X(J) .le. BU(J);
* C          IF J.gt.NCOLS,        Y(J-NCOLS) .ge. BL(J) and
* C                                Y(J-NCOLS) .le. BU(J).
* C          (to impose equality constraints have BL(J)=BU(J)=
* C          constraining value.)
* C    4.    For IND(J)=4, no bounds on X(J) or Y(J-NCOLS) are required.
* C          (the values of BL(J) and BU(J) are not used.)
* C
* C     Values other than 1,2,3 or 4 for IND(J) are errors. In the case
* C     IND(J)=3 (upper and lower bounds) the condition BL(J) .gt. BU(J)
* C     is  an  error.   The values BL(J), BU(J), J .gt. NCOLS, will be
* C     changed.  Significant changes mean that the constraints are
* C     infeasible.  (Users must make this decision themselves.)
* C     The new values for BL(J), BU(J), J .gt. NCOLS, define a
* C     region such that the perturbed problem is feasible.  If users
* C     know that their problem is feasible, this step can be skipped
* C     by using option number 8 described below.
* C     See IOPT(*) description.
* C
* C    -------
* C    IOPT(*)
* C    -------
* C     This is the array where the user can specify nonstandard options
* C     for DBOCLS( ). Most of the time this feature can be ignored by
* C     setting the input value IOPT(1)=99. Occasionally users may have
* C     needs that require use of the following subprogram options. For
* C     details about how to use the options see below: IOPT(*) CONTENTS.
* C
* C     Option Number   Brief Statement of Purpose
* C     ------ ------   ----- --------- -- -------
* C           1         Return to user for accumulation of blocks
* C                     of least squares equations.  The values
* C                     of IOPT(*) are changed with this option.
* C                     The changes are updates to pointers for
* C                     placing the rows of equations into position
* C                     for processing.
* C           2         Check lengths of all arrays used in the
* C                     subprogram.
* C           3         Column scaling of the data matrix, [C].
* C                                                        [E]
* C           4         User provides column scaling for matrix [C].
* C                                                             [E]
* C           5         Provide option array to the low-level
* C                     subprogram SBOLS( ).
* C           6         Provide option array to the low-level
* C                     subprogram SBOLSM( ).
* C           7         Move the IOPT(*) processing pointer.
* C           8         Do not preprocess the constraints to
* C                     resolve infeasibilities.
* C           9         Do not pretriangularize the least squares matrix.
* C          99         No more options to change.
* C
* C    ----
* C    X(*)
* C    ----
* C     This array is used to pass data associated with options 4,5 and
* C     6. Ignore this parameter (on input) if no options are used.
* C     Otherwise see below: IOPT(*) CONTENTS.
* C
* C    OUTPUT
* C    ------
* C
* C    -----------------
* C    X(*),RNORMC,RNORM
* C    -----------------
* C     The array X(*) contains a solution (if MODE .ge.0 or .eq.-22) for
* C     the constrained least squares problem. The value RNORMC is the
* C     minimum residual vector length for the constraints C*X - Y = 0.
* C     The value RNORM is the minimum residual vector length for the
* C     least squares equations. Normally RNORMC=0, but in the case of
* C     inconsistent constraints this value will be nonzero.
* C     The values of X are returned in the first NVARS entries of X(*).
* C     The values of Y are returned in the last MCON entries of X(*).
* C
* C    ----
* C    MODE
* C    ----
* C     The sign of MODE determines whether the subprogram has completed
* C     normally, or encountered an error condition or abnormal status. A
* C     value of MODE .ge. 0 signifies that the subprogram has completed
* C     normally. The value of mode (.ge. 0) is the number of variables
* C     in an active status: not at a bound nor at the value zero, for
* C     the case of free variables. A negative value of MODE will be one
* C     of the cases (-57)-(-41), (-37)-(-22), (-19)-(-2). Values .lt. -1
* C     correspond to an abnormal completion of the subprogram. These
* C     error messages are in groups for the subprograms DBOCLS(),
* C     SBOLSM(), and SBOLS().  An approximate solution will be returned
* C     to the user only when max. iterations is reached, MODE=-22.
* C
* C    -----------
* C    RW(*),IW(*)
* C    -----------
* C     These are working arrays.  (normally the user can ignore the
* C     contents of these arrays.)
* C
* C    IOPT(*) CONTENTS
* C    ------- --------
* C     The option array allows a user to modify some internal variables
* C     in the subprogram without recompiling the source code. A central
* C     goal of the initial software design was to do a good job for most
* C     people. Thus the use of options will be restricted to a select
* C     group of users. The processing of the option array proceeds as
* C     follows: a pointer, here called LP, is initially set to the value
* C     1. At the pointer position the option number is extracted and
* C     used for locating other information that allows for options to be
* C     changed. The portion of the array IOPT(*) that is used for each
* C     option is fixed; the user and the subprogram both know how many
* C     locations are needed for each option. The value of LP is updated
* C     for each option based on the amount of storage in IOPT(*) that is
* C     required. A great deal of error checking is done by the
* C     subprogram on the contents of the option array. Nevertheless it
* C     is still possible to give the subprogram optional input that is
* C     meaningless. For example option 4 uses the locations
* C     X(NCOLS+IOFF),...,X(NCOLS+IOFF+NCOLS-1) for passing scaling data.
* C     The user must manage the allocation of these locations.
* C   1
* C   -
* C     This option allows the user to solve problems with a large number
* C     of rows compared to the number of variables. The idea is that the
* C     subprogram returns to the user (perhaps many times) and receives
* C     new least squares equations from the calling program unit.
* C     Eventually the user signals "that's all" and a solution is then
* C     computed. The value of MROWS is an output variable when this
* C     option is used. Its value is always in the range 0 .le. MROWS
* C     .le. NCOLS+1. It is the number of rows after the
* C     triangularization of the entire set of equations. If LP is the
* C     processing pointer for IOPT(*), the usage for the sequential
* C     processing of blocks of equations is
* C
* C
* C        IOPT(LP)=1
* C         Move block of equations to W(*,*) starting at
* C         the first row of W(*,*).
* C        IOPT(LP+3)=# of rows in the block; user defined
* C
* C     The user now calls DBOCLS( ) in a loop. The value of IOPT(LP+1)
* C     directs the user's action. The value of IOPT(LP+2) points to
* C     where the subsequent rows are to be placed in W(*,*). Both of
* C     these values are first defined in the subprogram. The user
* C     changes the value of IOPT(LP+1) (to 2) as a signal that all of
* C     the rows have been processed.
* C
* C      .<------CYCLE LOOP
* C      .    ELSE IF (IOPT(LP+1) .EQ. 2) THEN
* C      <-------EXIT LOOP SOLUTION COMPUTED IF MODE .GE. 0
* C      . ELSE
* C      . ERROR CONDITION; SHOULD NOT HAPPEN.
* C      .D1MACH, DASUM, DBOLS, DCOPY, DDOT, DNRM2, DSCAL,
* C                    XERMSG
* C***REVISION HISTORY  (YYMMDD)
* C   821220  DATE WRITTEN
* C   891006  Cosmetic changes to prologue.  (WRB)
* C   891006  REVISION DATE from Version 3.2
* C   891214  Prologue converted to Version 4.0 format.  (BAB)
* C   900510  Convert XERRWV calls to XERMSG calls.  (RWC)
* C   910819  Added variable M for MOUT+MCON in reference to DBOLS.  (WRB)
* C   920501  Reformatted the REFERENCES section.  (WRB)
* C***END PROLOGUE  DBOCLS
* C
*/

void dbocls_
(
    double*    W,
    int* MDW,
    int* MCON,
    int* MROWS,
    int* NCOLS,
    double*    BL,
    double*    BU,
    int* IND,
    int* IOPT,
    double*    X,
    double*    RNORMC,
    double*    RNORM,
    int* MODE,
    double*    RW,
    int* IW
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 *
*
* *DECK I1MACH
*       INTEGER FUNCTION I1MACH (I)
* C***BEGIN PROLOGUE  I1MACH
* C***PURPOSE  Return integer machine dependent constants.
* C***LIBRARY   SLATEC
* C***CATEGORY  R1
* C***TYPE      INTEGER (I1MACH-I)
* C***KEYWORDS  MACHINE CONSTANTS
* C***AUTHOR  Fox, P. A., (Bell Labs)
* C           Hall, A. D., (Bell Labs)
* C           Schryer, N. L., (Bell Labs)
* C***DESCRIPTION
* C
* C   I1MACH can be used to obtain machine-dependent parameters for the
* C   local machine environment.  It is a function subprogram with one
* C   (input) argument and can be referenced as follows:
* C
* C        K = I1MACH(I)
* C
* C   where I=1,...,16.  The (output) value of K above is determined by
* C   the (input) value of I.  The results for various values of I are
* C   discussed below.
* C
* C   I/O unit numbers:
* C     I1MACH( 1) = the standard input unit.
* C     I1MACH( 2) = the standard output unit.
* C     I1MACH( 3) = the standard punch unit.
* C     I1MACH( 4) = the standard error message unit.
* C
* C   Words:
* C     I1MACH( 5) = the number of bits per integer storage unit.
* C     I1MACH( 6) = the number of characters per integer storage unit.
* C
* C   Integers:
* C     assume integers are represented in the S-digit, base-A form
* C
* C                sign ( X(S-1)*A**(S-1) + ... + X(1)*A + X(0) )
* C
* C                where 0 .LE. X(I) .LT. A for I=0,...,S-1.
* C     I1MACH( 7) = A, the base.
* C     I1MACH( 8) = S, the number of base-A digits.
* C     I1MACH( 9) = A**S - 1, the largest magnitude.
* C
* C   Floating-Point Numbers:
* C     Assume floating-point numbers are represented in the T-digit,
* C     base-B form
* C                sign (B**E)*( (X(1)/B) + ... + (X(T)/B**T) )
* C
* C                where 0 .LE. X(I) .LT. B for I=1,...,T,
* C                0 .LT. X(1), and EMIN .LE. E .LE. EMAX.
* C     I1MACH(10) = B, the base.
* C
* C   Single-Precision:
* C     I1MACH(11) = T, the number of base-B digits.
* C     I1MACH(12) = EMIN, the smallest exponent E.
* C     I1MACH(13) = EMAX, the largest exponent E.
* C
* C   Double-Precision:
* C     I1MACH(14) = T, the number of base-B digits.
* C     I1MACH(15) = EMIN, the smallest exponent E.
* C     I1MACH(16) = EMAX, the largest exponent E.
* C
* C   To alter this function for a particular environment, the desired
* C   set of DATA statements should be activated by removing the C from
* C   column 1.  Also, the values of I1MACH(1) - I1MACH(4) should be
* C   checked for consistency with the local operating system.
* C
* C***REFERENCES  P. A. Fox, A. D. Hall and N. L. Schryer, Framework for
* C                 a portable library, ACM Transactions on Mathematical
* C                 Software 4, 2 (June 1978), pp. 177-188.
* C***ROUTINES CALLED  (NONE)
* C***REVISION HISTORY  (YYMMDD)
* C   750101  DATE WRITTEN
* C   891012  Added VAX G-floating constants.  (WRB)
* C   891012  REVISION DATE from Version 3.2
* C   891214  Prologue converted to Version 4.0 format.  (BAB)
* C   900618  Added DEC RISC constants.  (WRB)
* C   900723  Added IBM RS 6000 constants.  (WRB)
* C   901009  Correct I1MACH(7) for IBM Mainframes. Should be 2 not 16.
* C           (RWC)
* C   910710  Added HP 730 constants.  (SMR)
* C   911114  Added Convex IEEE constants.  (WRB)
* C   920121  Added SUN -r8 compiler option constants.  (WRB)
* C   920229  Added Touchstone Delta i860 constants.  (WRB)
* C   920501  Reformatted the REFERENCES section.  (WRB)
* C   920625  Added Convex -p8 and -pd8 compiler option constants.
* C           (BKS, WRB)
* C   930201  Added DEC Alpha and SGI constants.  (RWC and WRB)
* C   930618  Corrected I1MACH(5) for Convex -p8 and -pd8 compiler
* C           options.  (DWL, RWC and WRB).
* C***END PROLOGUE  I1MACH
* C
*
*/

int i1mach_(int* machine_const_id_ptr);


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * *DECK D1MACH
*       DOUBLE PRECISION FUNCTION D1MACH (I)
* C***BEGIN PROLOGUE  D1MACH
* C***PURPOSE  Return floating point machine dependent constants.
* C***LIBRARY   SLATEC
* C***CATEGORY  R1
* C***TYPE      DOUBLE PRECISION (R1MACH-S, D1MACH-D)
* C***KEYWORDS  MACHINE CONSTANTS
* C***AUTHOR  Fox, P. A., (Bell Labs)
* C           Hall, A. D., (Bell Labs)
* C           Schryer, N. L., (Bell Labs)
* C***DESCRIPTION
* C
* C   D1MACH can be used to obtain machine-dependent parameters for the
* C   local machine environment.  It is a function subprogram with one
* C   (input) argument, and can be referenced as follows:
* C
* C        D = D1MACH(I)
* C
* C   where I=1,...,5.  The (output) value of D above is determined by
* C   the (input) value of I.  The results for various values of I are
* C   discussed below.
* C
* C   D1MACH( 1) = B**(EMIN-1), the smallest positive magnitude.
* C   D1MACH( 2) = B**EMAX*(1 - B**(-T)), the largest magnitude.
* C   D1MACH( 3) = B**(-T), the smallest relative spacing.
* C   D1MACH( 4) = B**(1-T), the largest relative spacing.
* C   D1MACH( 5) = LOG10(B)
* C
* C   Assume double precision numbers are represented in the T-digit,
* C   base-B form
* C
* C              sign (B**E)*( (X(1)/B) + ... + (X(T)/B**T) )
* C
* C   where 0 .LE. X(I) .LT. B for I=1,...,T, 0 .LT. X(1), and
* C   EMIN .LE. E .LE. EMAX.
* C
* C   The values of B, T, EMIN and EMAX are provided in I1MACH as
* C   follows:
* C   I1MACH(10) = B, the base.
* C   I1MACH(14) = T, the number of base-B digits.
* C   I1MACH(15) = EMIN, the smallest exponent E.
* C   I1MACH(16) = EMAX, the largest exponent E.
* C
* C   To alter this function for a particular environment, the desired
* C   set of DATA statements should be activated by removing the C from
* C   column 1.  Also, the values of D1MACH(1) - D1MACH(4) should be
* C   checked for consistency with the local operating system.
* C
* C***REFERENCES  P. A. Fox, A. D. Hall and N. L. Schryer, Framework for
* C                 a portable library, ACM Transactions on Mathematical
* C                 Software 4, 2 (June 1978), pp. 177-188.
* C***ROUTINES CALLED  XERMSG
* C***REVISION HISTORY  (YYMMDD)
* C   750101  DATE WRITTEN
* C   890213  REVISION DATE from Version 3.2
* C   891214  Prologue converted to Version 4.0 format.  (BAB)
* C   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ)
* C   900618  Added DEC RISC constants.  (WRB)
* C   900723  Added IBM RS 6000 constants.  (WRB)
* C   900911  Added SUN 386i constants.  (WRB)
* C   910710  Added HP 730 constants.  (SMR)
* C   911114  Added Convex IEEE constants.  (WRB)
* C   920121  Added SUN -r8 compiler option constants.  (WRB)
* C   920229  Added Touchstone Delta i860 constants.  (WRB)
* C   920501  Reformatted the REFERENCES section.  (WRB)
* C   920625  Added CONVEX -p8 and -pd8 compiler option constants.
* C           (BKS, WRB)
* C   930201  Added DEC Alpha and SGI constants.  (RWC and WRB)
* C***END PROLOGUE  D1MACH
*
*/

double d1mach_(int* machine_const_id_ptr);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_CPLUSPLUS
}
#endif

#endif

