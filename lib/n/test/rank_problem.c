#include <n/n_svd.h>

int main(void) {
    Matrix* mat = NULL;
    Matrix* U = NULL;
    Vector* D = NULL;
    Matrix* VT = NULL;
    int rank = 0;
    int i,j;

    read_matrix(&mat,"problemMatrix");
    do_svd(mat,&U,&D,&VT,&rank);

    printf("Original Matrix A:\n");
    for (i=0; i<mat->num_rows; i++) {
        for (j=0; j<mat->num_cols; j++) {
            printf("\t%f",mat->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Derived U:\n");
    for (i=0; i<U->num_rows; i++) {
        for (j=0; j<U->num_cols; j++) {
            printf("\t%f",U->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Derived D:\n");
    for (i=0; i<D->length; i++) {
        printf("\t%f",D->elements[i]);
    }
    printf("\n");

    printf("Derived VT:\n");
    for (i=0; i<VT->num_rows; i++) {
        for (j=0; j<VT->num_cols; j++) {
            printf("\t%f",VT->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Rank: %d\n",rank);

    free_matrix(mat);
    free_matrix(U);
    free_matrix(VT);
    free_vector(D);
}
