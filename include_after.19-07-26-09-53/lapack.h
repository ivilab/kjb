
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


#ifndef LAPACK_INCLUDED
#define LAPACK_INCLUDED

#ifdef LAPPACK_IS_MKL
#    include <mkl_lapack.h>
#else
#ifdef MAC_OSX_VECLIB_LAPACK
#    include  <vecLib/clapack.h>
#else

#include "m/m_gen.h"


#ifdef KJB_CPLUSPLUS
/*
 * Actually, these are fortran routines, but this seems to work.
*/
extern "C"
{
#endif

#ifdef HPUX
#    define dgeev_ dgeev
#    define dsyev_ dsyev
#    define ilaenv_ ilaenv
#    define dgetri_ dgetri
#endif

/* -------------------------------------------------------------------------- */



/*

      INTEGER          FUNCTION ILAENV( ISPEC, NAME, OPTS, N1, N2, N3,
     $                 N4 )
*
*  -- LAPACK auxiliary routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     June 30, 1999
*
*     .. Scalar Arguments ..
      CHARACTER*( * )    NAME, OPTS
      INTEGER            ISPEC, N1, N2, N3, N4
*     ..
*
*  Purpose
*  =======
*
*  ILAENV is called from the LAPACK routines to choose problem-dependent
*  parameters for the local environment.  See ISPEC for a description of
*  the parameters.
*
*  This version provides a set of parameters which should give good,
*  but not optimal, performance on many of the currently available
*  computers.  Users are encouraged to modify this subroutine to set
*  the tuning parameters for their particular machine using the option
*  and problem size information in the arguments.
*
*  This routine will not function correctly if it is converted to all
*  lower case.  Converting it to all upper case is allowed.
**  Arguments
*  =========
*
*  ISPEC   (input) INTEGER
*          Specifies the parameter to be returned as the value of
*          ILAENV.
*          = 1: the optimal blocksize; if this value is 1, an unblocked
*               algorithm will give the best performance.
*          = 2: the minimum block size for which the block routine
*               should be used; if the usable block size is less than
*               this value, an unblocked routine should be used.
*          = 3: the crossover point (in a block routine, for N less
*               than this value, an unblocked routine should be used)
*          = 4: the number of shifts, used in the nonsymmetric
*               eigenvalue routines
*          = 5: the minimum column dimension for blocking to be used;
*               rectangular blocks must have dimension at least k by m,
*               where k is given by ILAENV(2,...) and m by ILAENV(5,...)
*          = 6: the crossover point for the SVD (when reducing an m by n
*               matrix to bidiagonal form, if max(m,n)/min(m,n) exceeds
*               this value, a QR factorization is used first to reduce
*               the matrix to a triangular form.)
*          = 7: the number of processors
*          = 8: the crossover point for the multishift QR and QZ methods
*               for nonsymmetric eigenvalue problems.
*          = 9: maximum size of the subproblems at the bottom of the
*               computation tree in the divide-and-conquer algorithm
*               (used by xGELSD and xGESDD)
*          =10: ieee NaN arithmetic can be trusted not to trap
*          =11: infinity arithmetic can be trusted not to trap
*
*  NAME    (input) CHARACTER*(*)
*          The name of the calling subroutine, in either upper case or
*          lower case.
*
*  OPTS    (input) CHARACTER*(*)
*          The character options to the subroutine NAME, concatenated
*          into a single character string.  For example, UPLO = 'U',
*          TRANS = 'T', and DIAG = 'N' for a triangular routine would
*          be specified as OPTS = 'UTN'.
*
*  N1      (input) INTEGER
*  N2      (input) INTEGER
*  N3      (input) INTEGER
*  N4      (input) INTEGER
*          Problem dimensions for the subroutine NAME; these may not all
*          be required.
*
* (ILAENV) (output) INTEGER
*          >= 0: the value of the parameter specified by ISPEC
*          < 0:  if ILAENV = -k, the k-th argument had an illegal value.
*
*  Further Details
*  ===============
*
*  The following conventions have been used when calling ILAENV from the
*  LAPACK routines:
*  1)  OPTS is a concatenation of all of the character options to
*      subroutine NAME, in the same order that they appear in the
*      argument list for NAME, even if they are not used in determining
*      the value of the parameter specified by ISPEC.
*  2)  The problem dimensions N1, N2, N3, N4 are specified in the order
*      that they appear in the argument list for NAME.  N1 is used
*      first, N2 second, and so on, and unused problem dimensions are
*      passed a value of -1.
*  3)  The parameter value returned by ILAENV is checked for validity in
*      the calling subroutine.  For example, ILAENV is used to retrieve
*      the optimal blocksize for STRTRI as follows:
*
*      NB = ILAENV( 1, 'STRTRI', UPLO // DIAG, N, -1, -1, -1 )
*      IF( NB.LE.1 ) NB = MAX( 1, N )
*
*  =====================================================================
*/

extern int ilaenv_
(
    const int*  ISPEC,
    const char* DSYTRD,
    const char* UPLO,
    const int*  N1,
    const int*  N2,
    const int*  N3,
    const int*  N4,
    const int   len_DSYTRD,
    const int   len_UPLO
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 *
 * DSYEV(l)             LAPACK driver routine (version 1.1)             DSYEV(l)
 *
 * NAME
 *   DSYEV - compute all eigenvalues and, optionally, eigenvectors of a real
 *   symmetric matrix A
 *
 * SYNOPSIS
 *
 *   SUBROUTINE DSYEV( JOBZ, UPLO, N, A, LDA, W, WORK, LWORK, INFO )
 *
 *       CHARACTER     JOBZ, UPLO
 *
 *       INTEGER       INFO, LDA, LWORK, N
 *
 *       DOUBLE        PRECISION A( LDA, * ), W( * ), WORK( * )
 *
 * PURPOSE
 *   DSYEV computes all eigenvalues and, optionally, eigenvectors of a real sym-
 *   metric matrix A.
 *
 * ARGUMENTS
 *
 *   JOBZ    (input) CHARACTER*1
 *           = 'N':  Compute eigenvalues only;
 *           = 'V':  Compute eigenvalues and eigenvectors.
 *
 *   UPLO    (input) CHARACTER*1
 *           = 'U':  Upper triangle of A is stored;
 *           = 'L':  Lower triangle of A is stored.
 *
 *   N       (input) INTEGER
 *           The order of the matrix A.  N >= 0.
 *
 *   A       (input/output) DOUBLE PRECISION array, dimension (LDA, N)
 *           On entry, the symmetric matrix A.  If UPLO = 'U', the leading N-
 *           by-N upper triangular part of A contains the upper triangular part
 *           of the matrix A.  If UPLO = 'L', the leading N-by-N lower triangu-
 *           lar part of A contains the lower triangular part of the matrix A.
 *           On exit, if JOBZ = 'V', then if INFO = 0, A contains the orthonor-
 *           mal eigenvectors of the matrix A.  If JOBZ = 'N', then on exit the
 *           lower triangle (if UPLO='L') or the upper triangle (if UPLO='U') of
 *           A, including the diagonal, is destroyed.
 *
 *   LDA     (input) INTEGER
 *           The leading dimension of the array A.  LDA >= max(1,N).
 *
 *   W       (output) DOUBLE PRECISION array, dimension (N)
 *           If INFO = 0, the eigenvalues in ascending order.
 *
 *   WORK    (workspace) DOUBLE PRECISION array, dimension (LWORK)
 *           On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
 *
 *   LWORK   (input) INTEGER
 *           The length of the array WORK.  LWORK >= max(1,3*N-1).  For optimal
 *           efficiency, LWORK >= (NB+2)*N, where NB is the blocksize for DSYTRD
 *           returned by ILAENV.
 *
 *   INFO    (output) INTEGER
 *           = 0:  successful exit
 *           < 0:  if INFO = -i, the i-th argument had an illegal value
 *           > 0:  if INFO = i, the algorithm failed to converge; i off-diagonal
 *           elements of an intermediate tridiagonal form did not converge to
 *           zero.
 * #endif
 *
*/

extern void dsyev_
(
    const char* JOBZ,
    const char* UPLO,
    int*  N,
    double*     A,
    int*  LDA,
    double*     W,
    double*     WORK,
    int*  LWORK,
    int*  INFO,
    int  len_jobz,
    int  len_uplo
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * DGEEV(l)	     LAPACK driver routine (version 1.1)	     DGEEV(l)
 *
 * NAME
 *   DGEEV	- compute for an N-by-N	real nonsymmetric matrix A, the	eigenvalues
 *   and, optionally, the left and/or right eigenvectors
 *
 * SYNOPSIS
 *
 *   SUBROUTINE DGEEV( JOBVL, JOBVR, N, A,	LDA, WR, WI, VL, LDVL, VR, LDVR,
 * 		    WORK, LWORK, INFO )
 *
 *       CHARACTER	    JOBVL, JOBVR
 *
 *       INTEGER	    INFO, LDA, LDVL, LDVR, LWORK, N
 *
 *       DOUBLE	    PRECISION A( LDA, *	), VL( LDVL, * ), VR( LDVR, * ), WI(
 * 		    * ), WORK( * ), WR(	* )
 *
 * PURPOSE
 *   DGEEV	computes for an	N-by-N real nonsymmetric matrix	A, the eigenvalues
 *   and, optionally, the left and/or right eigenvectors.
 *
 *   The left eigenvectors	of A are the same as the right eigenvectors of A**T.
 *   If u(j) and v(j) are the left	and right eigenvectors,	respectively,
 *   corresponding	to the eigenvalue lambda(j), then (u(j)**T)*A =
 *   lambda(j)*(u(j)**T) and A*v(j) = lambda(j) * v(j).
 *
 *   The computed eigenvectors are	normalized to have Euclidean norm equal	to 1
 *   and largest component	real.
 *
 * ARGUMENTS
 *
 *   JOBVL	  (input) CHARACTER*1
 * 	  = 'N': left eigenvectors of A	are not	computed;
 * 	  = 'V': left eigenvectors of A	are computed.
 *
 *   JOBVR	  (input) CHARACTER*1
 * 	  = 'N': right eigenvectors of A are not computed;
 * 	  = 'V': right eigenvectors of A are computed.
 *
 *   N	  (input) INTEGER
 * 	  The order of the matrix A. N >= 0.
 *
 *   A	  (input/output) DOUBLE	PRECISION array, dimension (LDA,N)
 * 	  On entry, the	N-by-N matrix A.  On exit, A has been overwritten.
 *
 *   LDA	  (input) INTEGER
 * 	  The leading dimension	of the array A.	 LDA >=	max(1,N).
 *
 *   WR	  (output) DOUBLE PRECISION array, dimension (N)
 * 	  WI	  (output) DOUBLE PRECISION array, dimension (N) WR and	WI
 * 	  contain the real and imaginary parts,	respectively, of the computed
 * 	  eigenvalues.	Complex	conjugate pairs	of eigenvalues appear con-
 * 	  secutively with the eigenvalue having	the positive imaginary part
 * 	  first.
 *
 *   VL	  (output) DOUBLE PRECISION array, dimension (LDVL,N)
 * 	  If JOBVL = 'V', the left eigenvectors	u(j) are stored	one after
 * 	  another in the columns of VL,	in the same order as their eigen-
 * 	  values.  If JOBVL = 'N', VL is not referenced.  If the j-th
 * 	  eigenvalue is	real, then u(j)	= VL(:,j), the j-th column of VL.  If
 * 	  the j-th and (j+1)-st	eigenvalues form a complex conjugate pair,
 * 	  then u(j) = VL(:,j) +	i*VL(:,j+1) and
 * 	  u(j+1) = VL(:,j) = i*VL(:,j+1).
 *
 *   LDVL	  (input) INTEGER
 * 	  The leading dimension	of the array VL.  LDVL >= 1; if	JOBVL =	'V',
 * 	  LDVL >= N.
 *
 *   VR	  (output) DOUBLE PRECISION array, dimension (LDVR,N)
 * 	  If JOBVR = 'V', the right eigenvectors v(j) are stored one after
 * 	  another in the columns of VR,	in the same order as their eigen-
 * 	  values.  If JOBVR = 'N', VR is not referenced.  If the j-th eigen-
 * 	  value	is real, then v(j) = VR(:,j), the j-th column of VR.  If the
 * 	  j-th and (j+1)-st eigenvalues	form a complex conjugate pair, then
 * 	  v(j) = VR(:,j) + i*VR(:,j+1) and
 * 	  v(j+1) = VR(:,j) = i*VR(:,j+1).
 *
 *   LDVR	  (input) INTEGER
 * 	  The leading dimension	of the array VR.  LDVR >= 1; if	JOBVR =	'V',
 * 	  LDVR >= N.
 *
 *   WORK	  (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
 * 	  On exit, if INFO = 0,	WORK(1)	returns	the optimal LWORK.
 *
 *   LWORK	  (input) INTEGER
 * 	  The dimension	of the array WORK.  LWORK >= max(1,3*N), and if	JOBVL
 * 	  = 'V'	or JOBVR = 'V',	LWORK >= 4*N.  For good	performance, LWORK
 * 	  must generally be larger.
 *
 *   INFO	  (output) INTEGER
 * 	  = 0:	successful exit
 * 	  < 0:	if INFO	= -i, the i-th argument	had an illegal value.
 * 	  > 0:	if INFO	= i, the QR algorithm failed to	compute	all the
 * 	  eigenvalues, and no eigenvectors have	been computed; elements	i+1:N
 * 	  of WR	and WI contain eigenvalues which have converged.
 *
*/

void dgeev_
(
    const char* JOBVL,
    const char* JOBVR,
    int*  N_ptr,
    double*     A,
    int*  LDA_ptr,
    double*     WR,
    double*     WI,
    double*     VL,
    int*  LDVL_ptr,
    double*     VR,
    int*  LDVR_ptr,
    double*     WORK,
    int*  LWORK_ptr,
    int*  INFO_ptr,
    int  len_jobvl,
    int  len_jobvr

);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*    SUBROUTINE DGESV( N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
*  -- LAPACK driver routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     March 31, 1993
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LDB, N, NRHS
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      DOUBLE PRECISION   A( LDA, * ), B( LDB, * )
*     ..
*
*  Purpose
*  =======
*
*  DGESV computes the solution to a real system of linear equations
*     A * X = B,
*  where A is an N-by-N matrix and X and B are N-by-NRHS matrices.
*
*  The LU decomposition with partial pivoting and row interchanges is
*  used to factor A as
*     A = P * L * U,
*  where P is a permutation matrix, L is unit lower triangular, and U is
*  upper triangular.  The factored form of A is then used to solve the
*  system of equations A * X = B.
*
*  Arguments
*  =========
*
*  N       (input) INTEGER
*          The number of linear equations, i.e., the order of the
*          matrix A.  N >= 0.
*
*  NRHS    (input) INTEGER
*          The number of right hand sides, i.e., the number of columns
*          of the matrix B.  NRHS >= 0.
*
*  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
*          On entry, the N-by-N coefficient matrix A.
*          On exit, the factors L and U from the factorization
*          A = P*L*U; the unit diagonal elements of L are not stored.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N).
*
*  IPIV    (output) INTEGER array, dimension (N)
*          The pivot indices that define the permutation matrix P;
*          row i of the matrix was interchanged with row IPIV(i).
*
*  B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
*          On entry, the N-by-NRHS matrix of right hand side matrix B.
*          On exit, if INFO = 0, the N-by-NRHS solution matrix X.
*
*  LDB     (input) INTEGER
*          The leading dimension of the array B.  LDB >= max(1,N).
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, U(i,i) is exactly zero.  The factorization
*                has been completed, but the factor U is exactly
*                singular, so the solution could not be computed.
*/

void dgesv_ 
(
    int* n_ptr, 
    int* nrhs_ptr, 
    double *a, 
    int* lda_ptr, 
    int* ipiv_ptr, 
    double *b, 
    int *ldb_ptr, 
    int *info_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
//       SUBROUTINE DGESVD( JOBU, JOBVT, M, N, A, LDA, S, U, LDU, VT, LDVT,
//      $                   WORK, LWORK, INFO )
// *
// *  -- LAPACK driver routine (version 2.0) --
// *     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
// *     Courant Institute, Argonne National Lab, and Rice University
// *     September 30, 1994
// *
// *     .. Scalar Arguments ..
//       CHARACTER          JOBU, JOBVT
//       INTEGER            INFO, LDA, LDU, LDVT, LWORK, M, N
// *     ..
// *     .. Array Arguments ..
//       DOUBLE PRECISION   A( LDA, * ), S( * ), U( LDU, * ),
//      $                   VT( LDVT, * ), WORK( * )
// *     ..
// *
// *  Purpose
// *  =======
// *
// *  DGESVD computes the singular value decomposition (SVD) of a double
// *  M-by-N matrix A, optionally computing the left and/or right singular
// *  vectors. The SVD is written
// *
// *       A = U * SIGMA * transpose(V)
// *
// *  where SIGMA is an M-by-N matrix which is zero except for its
// *  min(m,n) diagonal elements, U is an M-by-M orthogonal matrix, and
// *  V is an N-by-N orthogonal matrix.  The diagonal elements of SIGMA
// *  are the singular values of A; they are double and non-negative, and
// *  are returned in descending order.  The first min(m,n) columns of
// *  U and V are the left and right singular vectors of A.
// *
// *  Note that the routine returns V**T, not V.
// *
// *  Arguments
// *  =========
// *
// *  JOBU    (input) CHARACTER*1
// *          Specifies options for computing all or part of the matrix U:
// *          = 'A':  all M columns of U are returned in array U:
// *          = 'S':  the first min(m,n) columns of U (the left singular
// *                  vectors) are returned in the array U;
// *          = 'O':  the first min(m,n) columns of U (the left singular
// *                  vectors) are overwritten on the array A;
// *          = 'N':  no columns of U (no left singular vectors) are
// *                  computed.
// *
// *  JOBVT   (input) CHARACTER*1
// *          Specifies options for computing all or part of the matrix
// *          V**T:
// *          = 'A':  all N rows of V**T are returned in the array VT;
// *          = 'S':  the first min(m,n) rows of V**T (the right singular
// *                  vectors) are returned in the array VT;
// *          = 'O':  the first min(m,n) rows of V**T (the right singular
// *                  vectors) are overwritten on the array A;
// *          = 'N':  no rows of V**T (no right singular vectors) are
// *                  computed.
// *
// *          JOBVT and JOBU cannot both be 'O'.
// *
// *  M       (input) INTEGER
// *          The number of rows of the input matrix A.  M >= 0.
// *
// *  N       (input) INTEGER
// *          The number of columns of the input matrix A.  N >= 0.
// *
// *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
// *          On entry, the M-by-N matrix A.
// *          On exit,
// *          if JOBU = 'O',  A is overwritten with the first min(m,n)
// *                          columns of U (the left singular vectors,
// *                          stored columnwise);
// *          if JOBVT = 'O', A is overwritten with the first min(m,n)
// *                          rows of V**T (the right singular vectors,
// *                          stored rowwise);
// *          if JOBU .ne. 'O' and JOBVT .ne. 'O', the contents of A
// *                          are destroyed.
// *
// *  LDA     (input) INTEGER
// *          The leading dimension of the array A.  LDA >= max(1,M).
// *
// *  S       (output) DOUBLE PRECISION array, dimension (min(M,N))
// *          The singular values of A, sorted so that S(i) >= S(i+1).
// *
// *  U       (output) DOUBLE PRECISION array, dimension (LDU,UCOL)
// *          (LDU,M) if JOBU = 'A' or (LDU,min(M,N)) if JOBU = 'S'.
// *          If JOBU = 'A', U contains the M-by-M orthogonal matrix U;
// *          if JOBU = 'S', U contains the first min(m,n) columns of U
// *          (the left singular vectors, stored columnwise);
// *          if JOBU = 'N' or 'O', U is not referenced.
// *
// *  LDU     (input) INTEGER
// *          The leading dimension of the array U.  LDU >= 1; if
// *          JOBU = 'S' or 'A', LDU >= M.
// *
// *  VT      (output) DOUBLE PRECISION array, dimension (LDVT,N)
// *          If JOBVT = 'A', VT contains the N-by-N orthogonal matrix
// *          V**T;
// *          if JOBVT = 'S', VT contains the first min(m,n) rows of
// *          V**T (the right singular vectors, stored rowwise);
// *          if JOBVT = 'N' or 'O', VT is not referenced.
// *
// *  LDVT    (input) INTEGER
// *          The leading dimension of the array VT.  LDVT >= 1; if
// *          JOBVT = 'A', LDVT >= N; if JOBVT = 'S', LDVT >= min(M,N).
// *
// *  WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
// *          On exit, if INFO = 0, WORK(1) returns the optimal LWORK;
// *          if INFO > 0, WORK(2:MIN(M,N)) contains the unconverged
// *          superdiagonal elements of an upper bidiagonal matrix B
// *          whose diagonal is in S (not necessarily sorted). B
// *          satisfies A = U * B * VT, so it has the same singular values
// *          as A, and singular vectors related by U and VT.
// *
// *  LWORK   (input) INTEGER
// *          The dimension of the array WORK. LWORK >= 1.
// *          LWORK >= MAX(3*MIN(M,N)+MAX(M,N),5*MIN(M,N)-4).
// *          For good performance, LWORK should generally be larger.
// *
// *  INFO    (output) INTEGER
// *          = 0:  successful exit.
// *          < 0:  if INFO = -i, the i-th argument had an illegal value.
// *          > 0:  if DBDSQR did not converge, INFO specifies how many
// *                superdiagonals of an intermediate bidiagonal form B
// *                did not converge to zero. See the description of WORK
// *                above for details.
// *
// *  =====================================================================
*/

extern void dgesvd_
(
    const char* JOBU_ptr,
    const char* JOBVT_ptr,
    int*  M_ptr,
    int*  N_ptr,
    double*     A_ptr,
    int*  LDA_ptr,
    double*     S_ptr,
    double*     U_ptr,
    int*  LDU_ptr,
    double*     VT_ptr,
    int*  LDVT_ptr,
    double*     WORK_ptr,
    int*  LWORK_ptr,
    int*  INFO_ptr,
    int   len_jobu,
    int   len_jobvt
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
*      SUBROUTINE DGETRI( N, A, LDA, IPIV, WORK, LWORK, INFO )
*
*  -- LAPACK routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     June 30, 1999
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LWORK, N
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      DOUBLE PRECISION   A( LDA, * ), WORK( * )
*     ..
*
*  Purpose
*  =======
*
*  DGETRI computes the inverse of a matrix using the LU factorization
*  computed by DGETRF.
*
*  This method inverts U and then computes inv(A) by solving the system
*  inv(A)*L = inv(U) for inv(A).
*
*  Arguments
*  =========
*
*  N       (input) INTEGER
*          The order of the matrix A.  N >= 0.
*
*  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
*          On entry, the factors L and U from the factorization
*          A = P*L*U as computed by DGETRF.
*          On exit, if INFO = 0, the inverse of the original matrix A.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N).
*
*  IPIV    (input) INTEGER array, dimension (N)
*          The pivot indices from DGETRF; for 1<=i<=N, row i of the
*          matrix was interchanged with row IPIV(i).
*
*  WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
*          On exit, if INFO=0, then WORK(1) returns the optimal LWORK.
*
*  LWORK   (input) INTEGER
*          The dimension of the array WORK.  LWORK >= max(1,N).
*          For optimal performance LWORK >= N*NB, where NB is
*          the optimal blocksize returned by ILAENV.
*
*          If LWORK = -1, then a workspace query is assumed; the routine
*          only calculates the optimal size of the WORK array, returns
*          this value as the first entry of the WORK array, and no error
*          message related to LWORK is issued by XERBLA.
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, U(i,i) is exactly zero; the matrix is
*                singular and its inverse could not be computed.
*
*  =====================================================================
*
*/

extern void dgetri_
(
    int*  N_ptr,
    double*     A_ptr,
    int*  LDA_ptr,
    int*  IPIV_ptr,
    double*     WORK_ptr,
    int*  LWORK_ptr,
    int*  INFO_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 *       SUBROUTINE DGETRF( M, N, A, LDA, IPIV, INFO )
*
*  -- LAPACK routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     March 31, 1993
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, M, N
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      DOUBLE PRECISION   A( LDA, * )
*     ..
*
*  Purpose
*  =======
*
*  DGETRF computes an LU factorization of a general M-by-N matrix A
*  using partial pivoting with row interchanges.
*
*  The factorization has the form
*     A = P * L * U
*  where P is a permutation matrix, L is lower triangular with unit
*  diagonal elements (lower trapezoidal if m > n), and U is upper
*  triangular (upper trapezoidal if m < n).
*
*  This is the right-looking Level 3 BLAS version of the algorithm.
*
*  Arguments
*  =========
*
*  M       (input) INTEGER
*          The number of rows of the matrix A.  M >= 0.
*
*  N       (input) INTEGER
*          The number of columns of the matrix A.  N >= 0.
*
*  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
*          On entry, the M-by-N matrix to be factored.
*          On exit, the factors L and U from the factorization
*          A = P*L*U; the unit diagonal elements of L are not stored.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,M).
*
*  IPIV    (output) INTEGER array, dimension (min(M,N))
*          The pivot indices; for 1 <= i <= min(M,N), row i of the
*          matrix was interchanged with row IPIV(i).
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, U(i,i) is exactly zero. The factorization
*                has been completed, but the factor U is exactly
*                singular, and division by zero will occur if it is used
*                to solve a system of equations.
*
*  =====================================================================
*/

extern void dgetrf_
(
    int*  M_ptr,
    int*  N_ptr,
    double*     A_ptr,
    int*  LDA_ptr,
    int*  IPIV_ptr,
    int*  INFO_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 *       SUBROUTINE DGEQRF( M, N, A, LDA, TAU, WORK, LWORK, INFO )
 *
 *  -- LAPACK routine (version 3.2) --
 *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
 *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
 *     November 2006
 *
 *     .. Scalar Arguments ..
       INTEGER            INFO, LDA, LWORK, M, N
 *     ..
 *     .. Array Arguments ..
       DOUBLE PRECISION   A( LDA, * ), TAU( * ), WORK( * )
 *     ..
 *
 *  Purpose
 *  =======
 *
 *  DGEQRF computes a QR factorization of a real M-by-N matrix A:
 *  A = Q * R.
 *
 *  Arguments
 *  =========
 *
 *  M       (input) INTEGER
 *          The number of rows of the matrix A.  M >= 0.
 *
 *  N       (input) INTEGER
 *          The number of columns of the matrix A.  N >= 0.
 *
 *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
 *          On entry, the M-by-N matrix A.
 *          On exit, the elements on and above the diagonal of the array
 *          contain the min(M,N)-by-N upper trapezoidal matrix R (R is
 *          upper triangular if m >= n); the elements below the diagonal,
 *          with the array TAU, represent the orthogonal matrix Q as a
 *          product of min(m,n) elementary reflectors (see Further
 *          Details).
 *
 *  LDA     (input) INTEGER
 *          The leading dimension of the array A.  LDA >= max(1,M).
 *
 *  TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))
 *          The scalar factors of the elementary reflectors (see Further
 *          Details).
 *
 *  WORK    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK))
 *          On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
 *
 *  LWORK   (input) INTEGER
 *          The dimension of the array WORK.  LWORK >= max(1,N).
 *          For optimum performance LWORK >= N*NB, where NB is
 *          the optimal blocksize.
 *
 *          If LWORK = -1, then a workspace query is assumed; the routine
 *          only calculates the optimal size of the WORK array, returns
 *          this value as the first entry of the WORK array, and no error
 *          message related to LWORK is issued by XERBLA.
 *
 *  INFO    (output) INTEGER
 *          = 0:  successful exit
 *          < 0:  if INFO = -i, the i-th argument had an illegal value
 *
 *  Further Details
 *  ===============
 *
 *  The matrix Q is represented as a product of elementary reflectors
 *
 *     Q = H(1) H(2) . . . H(k), where k = min(m,n).
 *
 *  Each H(i) has the form
 *
 *     H(i) = I - tau * v * v'
 *
 *  where tau is a real scalar, and v is a real vector with
 *  v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in A(i+1:m,i),
 *  and tau in TAU(i).
 *
 *  =====================================================================
 */
extern void dgeqrf_
(
    int*    M_ptr,
    int*    N_ptr,
    double* A_ptr,
    int*    LDA_ptr,
    double* TAU_ptr,
    double* WORK_ptr,
    int*    LWORK_ptr,
    int*    INFO_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
*        SUBROUTINE DGERQF( M, N, A, LDA, TAU, WORK, LWORK, INFO )
*
*  -- LAPACK routine (version 3.2) --
*  -- LAPACK is a software package provided by Univ. of Tennessee,    --
*  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
*     November 2006
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LWORK, M, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION   A( LDA, * ), TAU( * ), WORK( * )
*     ..
*
*  Purpose
*  =======
*
*  DGERQF computes an RQ factorization of a real M-by-N matrix A:
*  A = R * Q.
*
*  Arguments
*  =========
*
*  M       (input) INTEGER
*          The number of rows of the matrix A.  M >= 0.
*
*  N       (input) INTEGER
*          The number of columns of the matrix A.  N >= 0.
*
*  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
*          On entry, the M-by-N matrix A.
*          On exit,
*          if m <= n, the upper triangle of the subarray
*          A(1:m,n-m+1:n) contains the M-by-M upper triangular matrix R;
*          if m >= n, the elements on and above the (m-n)-th subdiagonal
*          contain the M-by-N upper trapezoidal matrix R;
*          the remaining elements, with the array TAU, represent the
*          orthogonal matrix Q as a product of min(m,n) elementary
*          reflectors (see Further Details).
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,M).
*
*  TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))
*          The scalar factors of the elementary reflectors (see Further
*          Details).
*
*  WORK    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK))
*          On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
*
*  LWORK   (input) INTEGER
*          The dimension of the array WORK.  LWORK >= max(1,M).
*          For optimum performance LWORK >= M*NB, where NB is
*          the optimal blocksize.
*
*          If LWORK = -1, then a workspace query is assumed; the routine
*          only calculates the optimal size of the WORK array, returns
*          this value as the first entry of the WORK array, and no error
*          message related to LWORK is issued by XERBLA.
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*
*  Further Details
*  ===============
*
*  The matrix Q is represented as a product of elementary reflectors
*
*     Q = H(1) H(2) . . . H(k), where k = min(m,n).
*
*  Each H(i) has the form
*
*     H(i) = I - tau * v * v'
*
*  where tau is a real scalar, and v is a real vector with
*  v(n-k+i+1:n) = 0 and v(n-k+i) = 1; v(1:n-k+i-1) is stored on exit in
*  A(m-k+i,1:n-k+i-1), and tau in TAU(i).
*
*  =====================================================================
*/
extern void dgerqf_
(
    int*    M_ptr,
    int*    N_ptr,
    double* A_ptr,
    int*    LDA_ptr,
    double* TAU_ptr,
    double* WORK_ptr,
    int*    LWORK_ptr,
    int*    INFO_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 *        SUBROUTINE DTRTRS( UPLO, TRANS, DIAG, N, NRHS, A, LDA, B, LDB, INFO )
 *
 *  -- LAPACK routine (version 3.2) --
 *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
 *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
 *     November 2006
 *
 *     .. Scalar Arguments ..
       CHARACTER          DIAG, TRANS, UPLO
       INTEGER            INFO, LDA, LDB, N, NRHS
 *     ..
 *     .. Array Arguments ..
       DOUBLE PRECISION   A( LDA, * ), B( LDB, * )
 *     ..
 *
 *  Purpose
 *  =======
 *
 *  DTRTRS solves a triangular system of the form
 *
 *     A * X = B  or  A**T * X = B,
 *
 *  where A is a triangular matrix of order N, and B is an N-by-NRHS
 *  matrix.  A check is made to verify that A is nonsingular.
 *
 *  Arguments
 *  =========
 *
 *  UPLO    (input) CHARACTER*1
 *          = 'U':  A is upper triangular;
 *          = 'L':  A is lower triangular.
 *
 *  TRANS   (input) CHARACTER*1
 *          Specifies the form of the system of equations:
 *          = 'N':  A * X = B  (No transpose)
 *          = 'T':  A**T * X = B  (Transpose)
 *          = 'C':  A**H * X = B  (Conjugate transpose = Transpose)
 *
 *  DIAG    (input) CHARACTER*1
 *          = 'N':  A is non-unit triangular;
 *          = 'U':  A is unit triangular.
 *
 *  N       (input) INTEGER
 *          The order of the matrix A.  N >= 0.
 *
 *  NRHS    (input) INTEGER
 *          The number of right hand sides, i.e., the number of columns
 *          of the matrix B.  NRHS >= 0.
 *
 *  A       (input) DOUBLE PRECISION array, dimension (LDA,N)
 *          The triangular matrix A.  If UPLO = 'U', the leading N-by-N
 *          upper triangular part of the array A contains the upper
 *          triangular matrix, and the strictly lower triangular part of
 *          A is not referenced.  If UPLO = 'L', the leading N-by-N lower
 *          triangular part of the array A contains the lower triangular
 *          matrix, and the strictly upper triangular part of A is not
 *          referenced.  If DIAG = 'U', the diagonal elements of A are
 *          also not referenced and are assumed to be 1.
 *
 *  LDA     (input) INTEGER
 *          The leading dimension of the array A.  LDA >= max(1,N).
 *
 *  B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
 *          On entry, the right hand side matrix B.
 *          On exit, if INFO = 0, the solution matrix X.
 *
 *  LDB     (input) INTEGER
 *          The leading dimension of the array B.  LDB >= max(1,N).
 *
 *  INFO    (output) INTEGER
 *          = 0:  successful exit
 *          < 0: if INFO = -i, the i-th argument had an illegal value
 *          > 0: if INFO = i, the i-th diagonal element of A is zero,
 *               indicating that the matrix is singular and the solutions
 *               X have not been computed.
 *
 *  =====================================================================
 */
extern void dtrtrs_
(
    char *UPLO_ptr,
    char *TRANS_ptr,
    char *DIAG_ptr,
    int *N_ptr,
    int *NRHS_ptr,
    double *A_ptr,
    int *LDA_ptr,
    double *B_ptr,
    int *LDB_ptr,
    int *INFO_ptr,
    int len_uplo,
    int len_trans,
    int len_diag
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 *      SUBROUTINE DPOTRF( UPLO, N, A, LDA, INFO )
 *
 *  -- LAPACK routine (version 3.3.1) --
 *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
 *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
 *  -- April 2011                                                      --
 *
 *     .. Scalar Arguments ..
       CHARACTER          UPLO
       INTEGER            INFO, LDA, N
 *     ..
 *     .. Array Arguments ..
       DOUBLE PRECISION   A( LDA, * )
 *     ..
 *
 *  Purpose
 *  =======
 *
 *  DPOTRF computes the Cholesky factorization of a real symmetric
 *  positive definite matrix A.
 *
 *  The factorization has the form
 *     A = U**T * U,  if UPLO = 'U', or
 *     A = L  * L**T,  if UPLO = 'L',
 *  where U is an upper triangular matrix and L is lower triangular.
 *
 *  This is the block version of the algorithm, calling Level 3 BLAS.
 *
 *  Arguments
 *  =========
 *
 *  UPLO    (input) CHARACTER*1
 *          = 'U':  Upper triangle of A is stored;
 *          = 'L':  Lower triangle of A is stored.
 *
 *  N       (input) INTEGER
 *          The order of the matrix A.  N >= 0.
 *
 *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
 *          On entry, the symmetric matrix A.  If UPLO = 'U', the leading
 *          N-by-N upper triangular part of A contains the upper
 *          triangular part of the matrix A, and the strictly lower
 *          triangular part of A is not referenced.  If UPLO = 'L', the
 *          leading N-by-N lower triangular part of A contains the lower
 *          triangular part of the matrix A, and the strictly upper
 *          triangular part of A is not referenced.
 *
 *          On exit, if INFO = 0, the factor U or L from the Cholesky
 *          factorization A = U**T*U or A = L*L**T.
 *
 *  LDA     (input) INTEGER
 *          The leading dimension of the array A.  LDA >= max(1,N).
 *
 *  INFO    (output) INTEGER
 *          = 0:  successful exit
 *          < 0:  if INFO = -i, the i-th argument had an illegal value
 *          > 0:  if INFO = i, the leading minor of order i is not
 *                positive definite, and the factorization could not be
 *                completed.
 *
 *  =====================================================================
 */
extern void dpotrf_
(
    char *UPLO_ptr,
    int *N_ptr,
    double *A_ptr,
    int *LDA_ptr,
    int *INFO_ptr
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * symmetric indefinite linear system solver
 */
extern void dsysv_
(
    char *UPLO_ptr,
    int *N_ptr,
    int *NRHS_ptr,
    double *A_ptr,
    int *LDA_ptr,
    int *IPIV_ptr,
    double *B_ptr,
    int *LDB_ptr,
    double *WORK_ptr,
    int *LWORK_ptr,
    int *INFO_ptr,
    int len_uplo
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * symmetric positive definite lienar system solver
 */
extern void dposv_
(
    char *UPLO_ptr,
    int *N_ptr,
    int *NRHS_ptr,
    double *A_ptr,
    int *LDA_ptr,
    double *B_ptr,
    int *LDB_ptr,
    int *INFO_ptr,
    int len_uplo
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* 
 * Linear system solver
 */
extern void dsegv_ 
(
    int *N_ptr,
    int *NRHS_ptr,
    double *A_ptr,
    int *LDA_ptr,
    double *B_ptr,
    int *LDB_ptr,
    int *INFO_ptr
);

#ifdef KJB_CPLUSPLUS
}
#endif


#endif  /* Endif of case NOT MAC_OSX */


#endif


#endif


