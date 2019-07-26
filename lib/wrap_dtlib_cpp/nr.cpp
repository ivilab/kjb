/////////////////////////////////////////////////////////////////////////////
// nr.cpp - stuff from Numerical Recipes in C, plus other utilities
// Authors: some from "Numerical Recipes in C", some by Doron Tal
// Date created: January, 1994

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "wrap_dtlib_cpp/nr.h"
#include "l_cpp/l_exception.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

using namespace DTLib;
using namespace kjb_c;


/////////////////////////////////////////////////////////////////////////////
// STUFF FROM NUMERICAL RECIPES (mostly)
/////////////////////////////////////////////////////////////////////////////

#define NR_END 1

// Numerical Recipes standard error handler
void DTLib::nrerror(char error_text[])
{
    KJB_THROW_2(kjb::KJB_error, "nr.cpp:nrerror()");
}

// allocate a float vector with subscript range v[nl..nh]
float *DTLib::fvector(long nl, long nh)
{
    float *v = new float [nh-nl+1+NR_END];
    if (!v) nrerror("allocation failure in vector()");
    return v-nl+NR_END;
}

// allocate an int vector with subscript range v[nl..nh]
int *DTLib::ivector(long nl, long nh)
{
    int *v = new int[nh-nl+1+NR_END];
    if (!v) nrerror("allocation failure in ivector()");
    return v-nl+NR_END;
}

// allocate an unsigned char vector with subscript range v[nl..nh]
unsigned char *DTLib::cvector(long nl, long nh)
{
    unsigned char *v = new unsigned char[nh-nl+1+NR_END];
    if (!v) nrerror("allocation failure in cvector()");
    return v-nl+NR_END;
}

// allocate an unsigned long vector with subscript range v[nl..nh]
kjb_uint32 *DTLib::lvector(long nl, long nh)
{
    kjb_uint32 *v = new kjb_uint32[nh-nl+1+NR_END];
    if (!v) nrerror("allocation failure in lvector()");
    return v-nl+NR_END;
}

// allocate a double vector with subscript range v[nl..nh]
double *DTLib::dvector(long nl, long nh)
{
    double *v = new double[nh-nl+1+NR_END];
    if (!v) nrerror("allocation failure in dvector()");
    return v-nl+NR_END;
}

// allocate a float matrix with subscript range m[nrl..nrh][ncl..nch]
float **DTLib::matrix(long nrl, long nrh, long ncl, long nch)
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
    float **m = new float*[nrow+NR_END];
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    // allocate rows and set pointers to them
    m[nrl]= new float[nrow*ncol+NR_END];
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    // return pointer to array of pointers to rows
    return m;
}

// allocate a double matrix with subscript range m[nrl..nrh][ncl..nch]
double **DTLib::dmatrix(long nrl, long nrh, long ncl, long nch)
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
    double **m = new double*[nrow+NR_END];
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    // allocate rows and set pointers to them
    m[nrl]= new double[nrow*ncol+NR_END];
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    // return pointer to array of pointers to rows
    return m;
}

// allocate a int matrix with subscript range m[nrl..nrh][ncl..nch]
int **DTLib::imatrix(long nrl, long nrh, long ncl, long nch)
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
    int **m = new int*[nrow+NR_END];
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    // allocate rows and set pointers to them
    m[nrl]= new int[nrow*ncol+NR_END];
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    // return pointer to array of pointers to rows
    return m;
}

// point a submatrix [newrl..][newcl..] to a[oldrl..oldrh][oldcl..oldch]
float **DTLib::submatrix(float **a, long oldrl, long oldrh, long oldcl, long oldch,
                  long newrl, long newcl)
{
    long i,j,nrow=oldrh-oldrl+1,ncol=oldcl-newcl;
    float **m = new float*[nrow+NR_END];
    if (!m) nrerror("allocation failure in submatrix()");
    m += NR_END;
    m -= newrl;

    // set pointers to rows
    for(i=oldrl,j=newrl;i<=oldrh;i++,j++) m[j]=a[i]+ncol;

    // return pointer to array of pointers to rows
    return m;
}

// allocate a float matrix m[nrl..nrh][ncl..nch] that points to the matrix
// declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
// and ncol=nch-ncl+1. The routine should be called with the address
// &a[0][0] as the first argument.
float **DTLib::convert_matrix(float *a, long nrl, long nrh, long ncl, long nch)
{
    long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
    float **m = new float*[nrow+NR_END];
    if (!m) nrerror("allocation failure in convert_matrix()");
    m += NR_END;
    m -= nrl;

    // set pointers to rows
    m[nrl]=a-ncl;
    for(i=1,j=nrl+1;i<nrow;i++,j++) m[j]=m[j-1]+ncol;
    // return pointer to array of pointers to rows
    return m;
}

// allocate a float 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh]
float ***DTLib::f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
{
    long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
    float ***t = new float**[nrow+NR_END];
    if (!t) nrerror("allocation failure 1 in f3tensor()");
    t += NR_END;
    t -= nrl;

    // allocate pointers to rows and set pointers to them
    t[nrl]= new float*[nrow*ncol+NR_END];
    if (!t[nrl]) nrerror("allocation failure 2 in f3tensor()");
    t[nrl] += NR_END;
    t[nrl] -= ncl;

    // allocate rows and set pointers to them
    t[nrl][ncl]= new float[nrow*ncol*ndep+NR_END];
    if (!t[nrl][ncl]) nrerror("allocation failure 3 in f3tensor()");
    t[nrl][ncl] += NR_END;
    t[nrl][ncl] -= ndl;

    for(j=ncl+1;j<=nch;j++) t[nrl][j]=t[nrl][j-1]+ndep;
    for(i=nrl+1;i<=nrh;i++) {
        t[i]=t[i-1]+ncol;
        t[i][ncl]=t[i-1][ncl]+ncol*ndep;
        for(j=ncl+1;j<=nch;j++) t[i][j]=t[i][j-1]+ndep;
    }

    // return pointer to array of pointers to rows
    return t;
}

// free a float vector allocated with vector()
void DTLib::free_fvector(float *v, long nl, long nh)
{
    delete [] (v+nl-NR_END);
}

// free an int vector allocated with ivector()
void DTLib::free_ivector(int *v, long nl, long nh)
{
    delete [](v+nl-NR_END);
}

// free an unsigned char vector allocated with cvector()
void DTLib::free_cvector(unsigned char *v, long nl, long nh)
{
    delete [](v+nl-NR_END);
}

// free an unsigned long vector allocated with lvector()
void DTLib::free_lvector(kjb_uint32 *v, long nl, long nh)
{
    delete [](v+nl-NR_END);
}

// free a double vector allocated with dvector()
void DTLib::free_dvector(double *v, long nl, long nh)
{
    delete [](v+nl-NR_END);
}

// free a float matrix allocated by matrix()
void DTLib::free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
{
    delete [](m[nrl]+ncl-NR_END);
    delete [](m+nrl-NR_END);
}

// free a double matrix allocated by dmatrix()
void DTLib::free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
{
    delete [](m[nrl]+ncl-NR_END);
    delete [](m+nrl-NR_END);
}

// free an int matrix allocated by imatrix()
void DTLib::free_imatrix(int **m, long nrl, long nrh, long ncl, long nch)
{
    delete [](m[nrl]+ncl-NR_END);
    delete [](m+nrl-NR_END);
}

// free a submatrix allocated by submatrix()
void DTLib::free_submatrix(float **b, long nrl, long nrh, long ncl, long nch)
{
    delete [](b+nrl-NR_END);
}

// free a matrix allocated by convert_matrix()
void DTLib::free_convert_matrix(float **b, long nrl, long nrh, long ncl, long nch)
{
    delete [](b+nrl-NR_END);
}

// free a float f3tensor allocated by f3tensor()
void DTLib::free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch,
                   long ndl, long ndh)
{
    delete [](t[nrl][ncl]+ndl-NR_END);
    delete [](t[nrl]+ncl-NR_END);
    delete [](t+nrl-NR_END);
}

/////////////////////////////////////////////////////////////////////////////

#define IA 16807
#define IM 2147483647
#define AM (1.0f/(float)IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS (float)1.2e-7
#define RNMX (1.0f-EPS)

float DTLib::ran1(long *idum)
{
    int j;
    long k;
    static long iy=0;
    static long iv[NTAB];
    float temp;

    if (*idum <= 0 || !iy) {
        if (-(*idum) < 1) *idum=1;
        else *idum = -(*idum);
        for (j=NTAB+7;j>=0;j--) {
            k=(*idum)/IQ;
            *idum=IA*(*idum-k*IQ)-IR*k;
            if (*idum < 0) *idum += IM;
            if (j < NTAB) iv[j] = *idum;
        }
        iy=iv[0];
    }
    k=(*idum)/IQ;
    *idum=IA*(*idum-k*IQ)-IR*k;
    if (*idum < 0) *idum += IM;
    j=iy/NDIV;
    iy=iv[j];
    iv[j] = *idum;
    if ((temp=AM*iy) > RNMX) return RNMX;
    else return temp;
}

#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX

/////////////////////////////////////////////////////////////////////////////

float DTLib::gasdev(long *idum)
{
    static int iset=0;
    static float gset;
    float fac,rsq,v1,v2;

    if  (iset == 0) {
        do {
            v1=2.0f*ran1(idum)-1.0f;
            v2=2.0f*ran1(idum)-1.0f;
            rsq=v1*v1+v2*v2;
        } while (rsq >= 1.0 || rsq == 0.0);
        fac=(float)sqrt(-2.0*log(rsq)/rsq);
        gset=v1*fac;
        iset=1;
        return v2*fac;
    } else {
        iset=0;
        return gset;
    }
}

/////////////////////////////////////////////////////////////////////////////

// LU decomposition

#define TINY (float)1.0e-20;

void DTLib::ludcmp(float **a, int n, int *indx, float *d)
{
    int i,imax = -1,j,k; // prog. should change 'imax' bec. it's an index!
    float big,dum,sum,temp;
    float *vv;

    vv=fvector(1,n);
    *d=1.0;
    for (i=1;i<=n;i++) {
        big=0.0f;
        for (j=1;j<=n;j++)
            if ((temp=(float)fabs(a[i][j])) > big) big=temp;
        if (big == 0.0f) nrerror("Singular matrix in routine ludcmp");
        vv[i]=1.0f/big;
    }
    for (j=1;j<=n;j++) {
        for (i=1;i<j;i++) {
            sum=a[i][j];
            for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
            a[i][j]=sum;
        }
        big=0.0f;
        for (i=j;i<=n;i++) {
            sum=a[i][j];
            for (k=1;k<j;k++)
                sum -= a[i][k]*a[k][j];
            a[i][j]=sum;
            if ( (dum=vv[i]*(float)fabs(sum)) >= big) {
                big=dum;
                imax=i;
            }
        }
        if (j != imax) {
            for (k=1;k<=n;k++) {
                dum=a[imax][k];
                a[imax][k]=a[j][k];
                a[j][k]=dum;
            }
            *d = -(*d);
            vv[imax]=vv[j];
        }
        indx[j]=imax;
        if (a[j][j] == 0.0) a[j][j]=TINY;
        if (j != n) {
            dum=1.0f/(a[j][j]);
            for (i=j+1;i<=n;i++) a[i][j] *= dum;
        }
    }
    free_fvector(vv,1,n);
}
#undef TINY

/////////////////////////////////////////////////////////////////////////////

void DTLib::lubksb(float **a, int n,int *indx, float *b)
{
    int i,ii=0,ip,j;
    float sum;

    for (i=1;i<=n;i++) {
        ip=indx[i];
        sum=b[ip];
        b[ip]=b[i];
        if (ii)
            for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
        else if (sum) ii=i;
        b[i]=sum;
    }
    for (i=n;i>=1;i--) {
        sum=b[i];
        for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
        b[i]=sum/a[i][i];
    }
}

/////////////////////////////////////////////////////////////////////////////

float *DTLib::zero_fvector(long nl, long nh)
{
    float *res = fvector(nl, nh);
    for(int i = nl; i <= nh; i++)
        res[i] = 0.0f;
    return res;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::mat_transpose(float **m1, float **mres, long nrl, long nrh, long ncl,
                   long nch)
{
    for(int i = nrl; i <= nrh; i++)
        for(int j = ncl; j <= nch; j++)
            mres[i][j] = m1[j][i];
}

/////////////////////////////////////////////////////////////////////////////

float **DTLib::diag_matrix(float diagVal, long nrl, long nrh, long ncl, long nch)
{
    float **res = matrix(nrl, nrh, ncl, nch);
    for(int i = nrl; i <= nrh; i++)
        for(int j = ncl; j <= nch; j++)
            if (i == j)
                res[i][j] = diagVal;
            else
                res[i][j] = 0.0f;
    return res;
}

/////////////////////////////////////////////////////////////////////////////

float **DTLib::zero_matrix(long nrl, long nrh, long ncl, long nch)
{
    float **res = matrix(nrl, nrh, ncl, nch);
    for(int i = nrl; i <= nrh; i++)
        for(int j = ncl; j <= nch; j++)
            res[i][j] = 0.0f;
    return res;
}

/////////////////////////////////////////////////////////////////////////////

float *DTLib::mat_diag_fvector(float **mat, long nrl, long nrh, long ncl, long nch)
{
    int nh;
    int rows = nrh-nrl+1, cols = nch-ncl+1;
    if (rows < cols)
        nh = rows;
    else nh = cols;
    float *v = fvector(1, nh);
    for (int i = 1; i <= nh; i++)
        for (int j = 1; j <= nh; j++)
            if (i == j) v[i] = mat[i][i];
    return v;
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::inner(float *x, float* y, long nl, long nh)
{
    float res = 0.0f;
    for (int i = nl; i <= nh; i++)
        res += x[i]*y[i];
    return res;
}

/////////////////////////////////////////////////////////////////////////////

// compute matrix*vector Ax = y
// PRECOND: matrix A must be [ncl, nch]x[nrl, nrh]
//          vector x must be [ncl, nch]
//          vector y must be [nrl, nrh]

void DTLib::mat_vec_prod(float **A, float *x, float *y,
                  long nrl, long nrh, long ncl, long nch)
{
    for (int i = nrl; i <= nrh; i++)
        y[i] = inner(A[i], x, ncl, nch);
}

/////////////////////////////////////////////////////////////////////////////

// PRECOND: matrix A is [ncl, nch] x [nrl, nrh]
//          matrix B is [ncl2, nch2] x [ncl, nch]
//          matrix res is [ncl2, nch2] x [ncl, nch]

void DTLib::mat_mat_prod(float **A, float **B, float **res, long nrl, long nrh,
                  long ncl, long nch, long ncl2, long nch2)
{
    for (int i = ncl; i <= nch; i++) // rows of result
        for (int j = ncl2; j <= nch2; j++) { // cols of result
            res[i][j] = 0.0f;
            for (int s = nrl; s <= nrh; s++)
                res[i][j] += A[i][s]*B[s][j];
        } // for j
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::mat_mat_sum(float **A, float **B, float **res,
                 long nrl, long nrh, long ncl, long nch)
{
    for (int i = nrl; i <= nrh; i++)
        for (int j = ncl; j <= nch; j++)
            res[i][j] = A[i][j]+B[i][j];
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::mat_mat_diff(float **A, float **B, float **res,
                  long nrl, long nrh, long ncl, long nch)
{
    for (int i = nrl; i <= nrh; i++)
        for (int j = ncl; j <= nch; j++)
            res[i][j] = A[i][j]-B[i][j];
}

/////////////////////////////////////////////////////////////////////////////

float **DTLib::rand_matrix(long nrl, long nrh, long ncl, long nch)
{
    long seed = (long)-1871;
    gasdev(&seed);
    long idnum = (long)13;
    float **res = matrix(nrl, nrh, ncl, nch);
    for(int i = nrl; i <= nrh; i++)
        for(int j = ncl; j <= nch; j++)
            res[i][j] = gasdev(&idnum);
    return res;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::scal_vec_prod(float s, float *v, float *res, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        res[i] = v[i]*s;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::vec_vec_prod(float *a, float *b, float *res, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        res[i] = a[i]*b[i];
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::vec_vec_diff(float *a, float *b, float *res, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        res[i] = a[i]-b[i];
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::vec_vec_sum(float *a, float *b, float *res, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        res[i] = a[i]+b[i];
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::vec_vec_assign(float *a, float *b, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        a[i] = b[i];
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::scal_mat_prod(float s, float **m, float **res,
                   long nrl, long nrh, long ncl, long nch)
{
    for (int i = nrl; i <= nrh; i++)
        for (int j = ncl; j <= nch; j++)
            res[i][j] = m[i][j]*s;
}

/////////////////////////////////////////////////////////////////////////////

float** DTLib::copy_mat(float **m, long nrl, long nrh, long ncl, long nch)
{
    float **res = matrix(nrl, nrh, ncl, nch);
    for (int i = nrl; i <= nrh; i++)
        for (int j = ncl; j <= nch; j++)
            res[i][j] = m[i][j];
    return res;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::mat_inverse(float **m, float **res, long n)
{
    float **a = copy_mat(m, 1, n, 1, n);
    float d, *col = fvector(1, n);
    int *indx = ivector(1, n), i, j;
    ludcmp(a, n, indx, &d);
    for (j = 1; j <= n; j++) {
        for (i = 1; i <= n; i++) col[i] = 0.0f;
        col[j] = 1.0f;
        lubksb(a, n, indx, col);
        for (i = 1; i <= n; i++) res[i][j] = col[i];
    } // for j
    free_matrix(a, 1, n, 1, n);
    free_fvector(col, 1, n);
    free_ivector(indx, 1, n);
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::trace_mat(float **m, long nrl, long nrh, long ncl, long nch)
{
    for (int i = nrl; i <= nrh; i++) {
        for (int j = ncl; j <= nch; j++)
            fprintf(stderr, "%12.4f", m[i][j]);
        fprintf(stderr, "\n");
    } // for i
    fprintf(stderr, "\n");
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::diag_matrix_from_vec(float *v, float **m, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        for (int j = nl; j <= nh; j++)
            if (i == j) m[i][i] = v[i];
            else m[i][j] = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::trace_vec(float* v, long nl, long nh)
{
    for (int i = nl; i <= nh; i++)
        fprintf(stderr, "%12.4f", v[i]);
    fprintf(stderr, "\n");
}
