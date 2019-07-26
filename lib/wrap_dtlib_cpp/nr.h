/////////////////////////////////////////////////////////////////////////////
// nr.h - stuff from Numerical Recipes in C and custom vector/matrix utilities
// Authors: some from "Numerical Recipes in C", some by Doron Tal
// Date created: January, 1994

#ifndef _NR_H
#define _NR_H

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // linear algebra (mostly from numerical recipes):
    ///////////////////////////////////////////////////////////////////////////

    void nrerror(char error_text[]);
    float* fvector(long nl, long nh);
    int* ivector(long nl, long nh);
    unsigned char *cvector(long nl, long nh);
    kjb_uint32 *lvector(long nl, long nh);
    double *dvector(long nl, long nh);
    float** matrix(long nrl, long nrh, long ncl, long nch);
    double **dmatrix(long nrl, long nrh, long ncl, long nch);
    int** imatrix(long nrl, long nrh, long ncl, long nch);
    float** submatrix(float** a, long oldrl, long oldrh, long oldcl,long oldch,
                      long newrl, long newcl);
    float** convert_matrix(float* a, long nrl, long nrh, long ncl, long nch);
    float** *f3tensor(long nrl, long nrh, long ncl,long nch,long ndl,long ndh);
    void free_fvector(float* v, long nl, long nh);
    void free_ivector(int* v, long nl, long nh);
    void free_cvector(unsigned char *v, long nl, long nh);
    void free_lvector(kjb_uint32 *v, long nl, long nh);
    void free_dvector(double *v, long nl, long nh);
    void free_matrix(float** m, long nrl, long nrh, long ncl, long nch);
    void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
    void free_imatrix(int** m, long nrl, long nrh, long ncl, long nch);
    void free_submatrix(float** b, long nrl, long nrh, long ncl, long nch);
    void free_convert_matrix(float** b, long nrl, long nrh, long ncl,long nch);
    void free_f3tensor(float** *t, long nrl, long nrh, long ncl, long nch,
                       long ndl, long ndh);
    void ludcmp(float** a, int n, int* indx, float* d);
    void lubksb(float** a, int n, int* indx, float* b);

    ///////////////////////////////////////////////////////////////////////////
    // functions by Doron Tal:
    ///////////////////////////////////////////////////////////////////////////

    float* zero_fvector(long nl, long nh);
    void mat_transpose(float** m1, float** mres, long nrl, long nrh, long ncl,
                       long nch);
    float** diag_matrix(float diagVal, long nrl, long nrh, long ncl, long nch);
    float** zero_matrix(long nrl, long nrh, long ncl, long nch);
    float* mat_diag_fvector(float** mat, long nrl, long nrh,long ncl,long nch);
    float inner(float* x, float* y, long nl, long nh);
    void mat_vec_prod(float** A, float* x, float* y, long nrl, long nrh,
                      long ncl, long nch);
    void mat_mat_prod(float** A, float** B, float** res, long nrl, long nrh,
                      long ncl, long nch, long ncl2, long nch2);
    void mat_mat_sum(float** A, float** B, float** res, long nrl, long nrh,
                     long ncl, long nch);
    void mat_mat_diff(float** A, float** B, float** res, long nrl, long nrh,
                      long ncl, long nch);
    float ran1(long *idum);
    float gasdev(long *idum);
    float** rand_matrix(long nrl, long nrh, long ncl, long nch);
    void scal_vec_prod(float s, float* v, float* res, long nl, long nh);
    void vec_vec_prod(float* a, float* b, float* res, long nl, long nh);
    void vec_vec_diff(float* a, float* b, float* res, long nl, long nh);
    void vec_vec_sum(float* a, float* b, float* res, long nl, long nh);
    void vec_vec_assign(float* a, float* b, long nl, long nh);
    void scal_mat_prod(float s, float** m, float** res,
                       long nrl, long nrh, long ncl, long nch);
    float** copy_mat(float** m, long nrl, long nrh, long ncl, long nch);
    void mat_inverse(float** m, float** res, long n);
    void trace_mat(float** m, long nrl, long nrh, long ncl, long nch);
    void trace_vec(float* v, long nl, long nh);
    void diag_matrix_from_vec(float* v, float** m, long nl, long nh);

} // namespace DTLib {

#endif /* #ifndef _NR_H */
