
/* $Id: hungarian.c 20654 2016-05-05 23:13:43Z kobus $ */


/*
// Implementation of the hungarian method based on code supplied by the
// Stanford Graphbase. See that resource for the official code.
//
// The modifications include the non-substantive changes to integrate it into
// my library more seemlessly, and the more substantive change to make it work
// with floating point numbers.  My preferred implementation of this is based
// on simply mapping the floating point problem to integers. This is less
// accurate than doing it properly with floats, but the error is bounded by an
// easily calculated quantity (set the variable "tol"). To the extent that the
// integer code is correct, this code is robust in that it gives an exact
// solution to an approximate problem.
//
// I initially attempted to make the code work directly on floats. This proved
// more difficult than expected as I don't understand what is going on well
// enough to specify the tolerences properly. My attempt at this is available
// in the disabled routine dbl_hungarian()).
*/


#include "m/m_incl.h"
#include "graph/hungarian.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* ============================================================================
 *                                 hungarian
 *
 * Approximates the best matching of a weighted bipartite graph.
 *
 * This approximately solves a bipartite graph matching problem for a graph
 * with real weights, by approximating the weights with large integers, and
 * using routine int_hungarian() -- please see the documentation for that
 * function.  The input and output interfaces correspond closely.
 *
 * The return value is ERROR if unsuccessful (e.g., if memory allocation
 * fails), otherwise NO_ERROR.
 *
 * A related function that requires C++ is min_bipartite_match() found in
 * lib/graph_cpp.
 *
 * Related:
 *    int_hungarian
 *
 * Documenters:
 *     Andrew Predoehl
 *
 * Index: graphs, matching, optimization
 *
 * ----------------------------------------------------------------------------
*/
int hungarian
(
    const Matrix* cost_mp,
    Int_vector**  row_assignment_vpp,
    double*       cost_ptr
)
{
    int m = cost_mp->num_rows;
    int n = cost_mp->num_cols;
    Int_matrix* int_cost_mp = NULL;
    Matrix* scaled_mp = NULL;
    double max_elem;
    double scale_up;
    double factor;
    int int_cost;
    int result = NO_ERROR;
    /*
    double tol;
    */

    max_elem = max_abs_matrix_element(cost_mp);

    scale_up = (double)(INT_MAX / 4) / MAX_OF(n, m);
    factor = scale_up / max_elem;
    /* tol = MAX_OF(m, n) / factor; */


    if (result != ERROR)
    {
        result = multiply_matrix_by_scalar(&scaled_mp, cost_mp, factor);
    }

    if (result != ERROR)
    {
        result = copy_matrix_to_int_matrix(&int_cost_mp, scaled_mp);
    }
    if (result != ERROR)
    {
        result = int_hungarian(int_cost_mp, row_assignment_vpp, &int_cost);
    }

    if (result != ERROR)
    {
        *cost_ptr = (double)int_cost / factor;
    }

    free_matrix(scaled_mp);
    free_int_matrix(int_cost_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int min_of_row( const int* a, int num_cols )
{
    int mor = a[ --num_cols ];
    while( --num_cols >= 0 )
    {
        mor = MIN_OF( mor, a[ num_cols ] );
    }
    return mor;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int min_of_col( const Int_matrix* mp, int col_index )
{
    int row = mp -> num_rows;
    int moc = mp -> elements[ --row ][ col_index ];
    while( --row >= 0 )
    {
        moc = MIN_OF( moc, mp -> elements[ row ][ col_index ] );
    }
    return moc;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                                 int_hungarian
 *
 * Minimally matches an integer-weighted bipartite graph.
 *
 * This solves a bipartite graph matching problem for a graph with integer
 * weights (costs), using the Hungarian algorithm, a.k.a. the Kuhn-Munkres
 * algorithm. See http://en.wikipedia.org/wiki/Hungarian_algorithm
 * or C. H. Papadimitriou and K. Steiglitz, _Combinatorial Algorithms_
 * (Prentice-Hall, 1982).
 *
 * Input should be an M x N matrix of weights W, where rows 1 - M correspond to
 * the (w.l.o.g.) left set of vertices, and columns 1 - N correspond to the
 * right set of vertices.  Hence W[i,j], the entry at location (i,j), contains
 * the cost of pairing left vertex i to right vertex j.
 *
 * If we assume M=N we can be more formal:  a matching p is a permutation of
 * the numbers {0,...,M-1}, which we interpret as pairing the i-th row with
 * column p(i).  Costs are additive, so the cost of a matching is defined as
 * | C(p) = W[1,p(1)] + W[2,p(2)] + . . . + W[M,p(M)].
 * This algorithm is optimal in the sense than it finds a matching p* such that
 * C(p*) is equal to or smaller than the cost of any other matching.
 *
 * If M < N, then the result is not a permutation but an injection
 * | p:{0,...,M-1} --> {0,...,N-1}
 * with a similar interpretation.  That means p is invertible, and (N-M)
 * columns are skipped.
 *
 * If M > N then (M-N) rows are skipped.  The result is a surjection
 * | p:{0,...,N-1} --> {-1,...,M-1}
 * and, informally, the above definition of cost
 * C(p) will still hold if we're willing to imagine that W contains a phantom
 * "column negative one," in which all entries equal a very big number, like
 * | W[-1,k] = max{ W[i,j] : -1<i<M, -1<j<N } for all -1<k<N.
 *
 * Output is returned in an Int_vector and an int.  The caller must pass in a
 * double pointer to an Int_vector (via the usual convention), called
 * row_assignment_vpp.  The caller must also pass in a pointer to an int,
 * called cost_ptr.  Neither output pointer may equal NULL.
 * Int_vector **row_assignment_vpp will have length M, and the i-th entry will
 * store the assignment p(i) as described above (a value in range -1 to N-1
 * that indicates the matched column index).
 * The total cost of the entire matching is returned in *cost_ptr.
 *
 * The return value is ERROR if unsuccessful (e.g., if memory allocation
 * fails), otherwise NO_ERROR.
 *
 * Related:
 *    hungarian
 *
 * Documenter:
 *     Andrew Predoehl
 *
 * Index: graphs, matching, optimization
 *
 * ----------------------------------------------------------------------------
*/

int int_hungarian
(
    const Int_matrix* cost_mp,
    Int_vector**      row_assignment_vpp,
    int*              cost_ptr
)
{
    Int_matrix* aa_mp = NULL;
    int**       aa;
    int*        col_mate;
    int*        row_mate;
    int*        parent_row;
    int*        unchosen_row;
    int*        row_dec;
    int*        col_inc;
    int*        slack;
    int*        slack_row;
    int*        matched_cols;
    int*        row_assignments;
    int         j, k, l, m, n, q, s, t, unmatched, transposed;

    int         cost   = 0;
    int         result = NO_ERROR;

    NRE( cost_mp );
    NRE( row_assignment_vpp );
    NRE( cost_ptr );

    m = cost_mp->num_rows;
    n = cost_mp->num_cols;

    ERE(get_target_int_vector(row_assignment_vpp, m));

    row_assignments = (*row_assignment_vpp)->elements;

    if (m > n)
    {
        ERE(get_int_transpose(&aa_mp, cost_mp));
        transposed  = 1;
    }
    else
    {
        ERE(copy_int_matrix(&aa_mp, cost_mp));
        transposed = 0;
    }

    m  = aa_mp->num_rows;
    n  = aa_mp->num_cols;
    aa  = aa_mp->elements;

    NRE(col_mate =     INT_MALLOC(m));
    NRE(row_mate =     INT_MALLOC(n));
    NRE(parent_row =   INT_MALLOC(n));
    NRE(unchosen_row = INT_MALLOC(m));
    NRE(row_dec =      INT_MALLOC(m));
    NRE(col_inc =      INT_MALLOC(n));
    NRE(slack =        INT_MALLOC(n));
    NRE(slack_row =    INT_MALLOC(n));


    if (m == n)
    {
        for (l=0; l<n; l++)
        {
#if 0
            s = aa[ 0 ][ l ];

            for(k= 1;k<n;k++)
            {
                if(aa[ k ][ l ]<s)s= aa[ k ][ l ];
            }
#else
            int s = min_of_col( aa_mp, l );
#endif

            if (s!=0)
            {
                for (k=0; k<m; k++)
                {
                    aa[ k ][ l ]-= s;
                }
            }
        }
    }

    t= 0;

    for(l= 0;l<n;l++)
    {
        row_mate[l]= -1;
        parent_row[l]= -1;
        col_inc[l]= 0;
        slack[l]= INT_MAX;
    }

    for(k= 0;k<m;k++)
    {
#if 0
        s= aa[ k ][ 0 ];

        for(l= 1;l<n;l++)
        {
            if(aa[ k ][ l ]<s)
            {
                s= aa[ k ][ l ];
            }
        }

        row_dec[k]= s;
#else
        int s = row_dec[k] = min_of_row( aa[k], n );
#endif

        for(l= 0;l<n;l++)
        {
            if((s==aa[ k ][ l ])&&(row_mate[l]<0))
            {
                col_mate[k]= l;
                row_mate[l]= k;
                goto row_done;
            }
        }
        col_mate[k]= -1;
        unchosen_row[t++]= k;

row_done:;
    }

    if(t==0)goto done;

    unmatched= t;

    while(1)
    {
        q= 0;
        while(1)
        {
            while(q<t)
            {
                k= unchosen_row[q];
                s= row_dec[k];

                for(l= 0;l<n;l++)
                {
                    if(slack[l])
                    {
                        int del= aa[ k ][ l ] - s + col_inc[l];

                        if(del<slack[l])
                        {
                            if(del==0)
                            {
                                if(row_mate[l]<0)goto breakthru;

                                slack[l]= 0;
                                parent_row[l]= k;
                                unchosen_row[t++]= row_mate[l];
                            }
                            else
                            {
                                slack[l]= del;
                                slack_row[l]= k;
                            }
                        }
                    }
                }

                q++;
            }

            s= INT_MAX;

            for(l= 0;l<n;l++)
            {
                if(slack[l]&&slack[l]<s)
                {
                    s= slack[l];
                }
            }

            for(q= 0;q<t;q++)
            {
                row_dec[unchosen_row[q]]+= s;
            }

            for(l= 0;l<n;l++)
            {
                if(slack[l])
                {
                    slack[l]-= s;

                    if(slack[l]==0)
                    {
                        k= slack_row[l];

                        if(row_mate[l]<0)
                        {
                            for(j= l+1;j<n;j++)
                                if(slack[j]==0)col_inc[j]+= s;
                            goto breakthru;
                        }
                        else
                        {
                            parent_row[l]= k;
                            unchosen_row[t++]= row_mate[l];
                        }
                    }
                }
                else
                {
                    col_inc[l]+= s;
                }
            }
        }

breakthru:

        while(1)
        {
            j= col_mate[k];
            col_mate[k]= l;
            row_mate[l]= k;

            if(j<0)break;

            k= parent_row[j];
            l= j;
        }

        if(--unmatched==0)goto done;

        t= 0;

        for(l= 0;l<n;l++)
        {
            parent_row[l]= -1;
            slack[l]= INT_MAX;
        }

        for(k= 0;k<m;k++)
        {
            if(col_mate[k]<0)
            {
                unchosen_row[t++]= k;
            }
        }

    }

done:

#ifdef TEST
    for(k= 0;k<m;k++)
    {
        if (result == ERROR) break;

        for(l= 0;l<n;l++)
        {
            ASSERT(aa[ k ][ l ] >= row_dec[k] - col_inc[l]);
        }
    }

    for(k= 0;k<m;k++)
    {
        if (result == ERROR) break;

        l= col_mate[k];

        ASSERT(l >= 0);
        ASSERT(aa[ k ][ l ] == row_dec[k]-col_inc[l]);
    }

    k= 0;

    for(l= 0;l<n;l++)if(col_inc[l])k++;

    ASSERT(k <= m);
#endif

    if (transposed)
    {
        for(l= 0;l<n;l++)
        {
            row_assignments[ l ] = row_mate[ l ];

            if (row_mate[ l ] != -1)
            {
                cost += cost_mp->elements[ l ][ row_mate[ l ] ];
            }
        }
    }
    else
    {
        for(k= 0;k<m;k++)
        {
            row_assignments[ k ] = col_mate[k];
            cost += cost_mp->elements[ k ][ col_mate[k] ];
        }
    }

    /*
    // As this continues to test well, this could be relegated to TEST code.
    */

    NRE(matched_cols = INT_MALLOC(n));

    for(l= 0;l<n;l++)
    {
        matched_cols[ l ] = FALSE;
    }

    for(k= 0;k<m;k++)
    {
        j = row_assignments[ k ];

        if (j >= 0)
        {
            if (j >= n)
            {
                if (result != ERROR)
                {
                    kjb_clear_error();
                }
                add_error("Row %d is matched to out of range col %d.",
                          k, j);

                result = ERROR;
            }
            else if (matched_cols[ j ] == TRUE)
            {
                if (result != ERROR)
                {
                    kjb_clear_error();
                }
                add_error("Column %d is doubly matched.", j);

                result = ERROR;
            }

            matched_cols[ j ] = TRUE;
        }
    }

    kjb_free(matched_cols);

    *cost_ptr = cost;

    free_int_matrix(aa_mp);
    kjb_free(col_mate);
    kjb_free(row_mate);
    kjb_free(parent_row);
    kjb_free(unchosen_row);
    kjb_free(row_dec);
    kjb_free(col_inc);
    kjb_free(slack);
    kjb_free(slack_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*
 * =============================================================================
 * STATIC                      dbl_hungarian
 *
 * Incomplete attempt to adapt int_hungarian for double weights.
 *
 * Until this is recoded, it should be considered broken!.
 *
 * This is a lame attempt to hack the integer code. It is not
 * clear if it is helpful, or if it best to start from scratch!
 * -----------------------------------------------------------------------------
*/
static int dbl_hungarian
(
    const Matrix* cost_mp,
    Int_vector**  row_assignment_vpp,
    double*       cost_ptr
)
{
#ifdef PROGRAMMER_IS_kobus
    Matrix* aa_mp        = NULL;
    double**       aa;
    double         cost         = 0.0;
    int         m            = cost_mp->num_rows;
    int         n            = cost_mp->num_cols;
    int        k;
    int        l;
    int        j;
    double        s;
    int*       col_mate;
    int*       row_mate;
    int*       parent_row;
    int*       unchosen_row;
    int        t;
    int        q;
    double*       row_dec;
    double*       col_inc;
    double*       slack;
    int*       slack_row;
    int        unmatched;
    int        transposed;
    int result = NO_ERROR;
    int* matched_cols;
    int* row_assignments;
#endif




    /*
     * Until this is recoded, it should be consdered completely broken!.
     *
     * What is here is a failed attempt to hack the integer code. It is not
     * clear if it is helpful, or if it best to start from scratch!
    */

#ifndef PROGRAMMER_IS_kobus
    set_error("The routine dbl_hungarian() is broken.");
    return ERROR;
#else

    ERE(get_target_int_vector(row_assignment_vpp, m));

    row_assignments = (*row_assignment_vpp)->elements;


    if (m > n)
    {
        ERE(get_transpose(&aa_mp, cost_mp));
        transposed  = 1;
    }
    else
    {
        ERE(copy_matrix(&aa_mp, cost_mp));
        transposed = 0;
    }

    m  = aa_mp->num_rows;
    n  = aa_mp->num_cols;

    /*
    max_elem = max_matrix_element(aa_mp);
    ERE(ow_divide_matrix_by_scalar(aa_mp, max_elem));
    */

    aa  = aa_mp->elements;

    NRE(col_mate =     INT_MALLOC(m));
    NRE(row_mate =     INT_MALLOC(n));
    NRE(parent_row =   INT_MALLOC(n));
    NRE(unchosen_row = INT_MALLOC(m));
    NRE(row_dec =      DBL_MALLOC(m));
    NRE(col_inc =      DBL_MALLOC(n));
    NRE(slack =        DBL_MALLOC(n));
    NRE(slack_row =    INT_MALLOC(n));


    /*
    if(m == n)
    {
        for(l= 0;l<n;l++)
        {
            s= aa[ 0 ][ l ];

            for(k= 1;k<n;k++)
            {
                if(aa[ k ][ l ]<s)
                {
                    s= aa[ k ][ l ];
                }
            }

            for(k= 0;k<n;k++)
            {
                aa[ k ][ l ]-= s;

            }
        }
    }
    */


    t= 0;

    for(l= 0;l<n;l++)
    {
        row_mate[l]= -1;
        parent_row[l]= -1;
        col_inc[l]= 0.0;
        slack[l]= DBL_MAX;
    }

    for(k= 0;k<m;k++)
    {
        s= aa[ k ][ 0 ];

        for(l= 1;l<n;l++)
        {
            if(aa[ k ][ l ] < s)
            {
                s = aa[ k ][ l ];
            }
        }

        row_dec[k]= s;

        for(l= 0;l<n;l++)
        {
            /* PRECISION */
            if(    (IS_EQUAL_DBL(s, aa[ k ][ l ]))
                && (row_mate[l]<0)
              )
            {
                col_mate[k]= l;
                row_mate[l]= k;
                goto row_done;
            }
        }
        col_mate[k]= -1;
        unchosen_row[t++]= k;

row_done:;
    }

    if(t==0)goto done;

    unmatched= t;


    while(1)
    {
        q= 0;
        while(1)
        {
            while(q<t)
            {
                k= unchosen_row[q];
                s= row_dec[k];

                for(l= 0;l<n;l++)
                {
                    /* PRECISION */
                    /* Integer case: if(slack[l]) */
                    if(ABS_OF(slack[l] > 100000.0 * DBL_EPSILON))
                    {
                        double del= aa[ k ][ l ] - s + col_inc[l];

                        if(del<slack[l])
                        {
                            /* PRECISION */
                            /* Integer case: if(del==0) */
                            if (    (del == 0.0)
                                 || (del / MAX_OF(aa[k][l], MAX_OF(ABS_OF(s), ABS_OF(col_inc[l]))) < 100000.0 * DBL_EPSILON)
                               )
                            {
                                /* if (del != 0.0) dbe(del); */

                                if (row_mate[l]<0) goto breakthru;

                                slack[l] = 0.0;
                                parent_row[l]= k;
                                unchosen_row[t++]= row_mate[l];
                            }
                            else
                            {
                                slack[l]= del;
                                slack_row[l]= k;
                            }
                        }
                    }
                    /*
                    else if (slack[l] != 0.0)
                    {
                        dbe(slack[l]);
                    }
                    */
                }

                q++;
            }

            s= DBL_MAX;

            for(l= 0;l<n;l++)
            {
                /* PRECISION */
                /* Integer case: if (slack[l]&&slack[l]<s) */
                if (    (ABS_OF(slack[l]) > 100000.0 * DBL_EPSILON)
                     && (slack[l] < s)
                   )
                {
                    s= slack[l];
                }
            }

            for(q= 0;q<t;q++)
            {
                row_dec[unchosen_row[q]]+= s;
            }

            for(l= 0;l<n;l++)
            {

                /* PRECISION */
                /* Integer case: if(slack[l]) */
                if(ABS_OF(slack[l] > 100000.0 * DBL_EPSILON))
                {
                    slack[l]-= s;

                    /* PRECISION */
                    /* Integer case: if(slack[l]==0) */
                    if(ABS_OF(slack[l] < 100000.0 * DBL_EPSILON))
                    {
                        /* if (slack[l] != 0.0) dbe(slack[l]); */

                        k= slack_row[l];

                        if(row_mate[l] < 0)
                        {
                            for(j= l+1;j<n;j++)
                            {
                                /* PRECISION */
                                /* Integer case: if(slack[j]==0) */
                                if(ABS_OF(slack[j] < 100000.0 * DBL_EPSILON))
                                {
                                    /*
                                    if (slack[j] != 0.0)
                                    {
                                        dbe(slack[j]);
                                    }
                                    */
                                    col_inc[j]+= s;
                                }
                            }
                            goto breakthru;
                        }
                        else
                        {
                            parent_row[l]= k;
                            unchosen_row[t++]= row_mate[l];
                        }
                    }
                }
                else
                {
                    col_inc[l]+= s;
                }
            }
        }

breakthru:

        while(1)
        {
            j= col_mate[k];
            col_mate[k]= l;
            row_mate[l]= k;

            if(j<0)break;

            k= parent_row[j];
            l= j;
        }

        if(--unmatched==0)goto done;

        t= 0;

        for(l= 0;l<n;l++)
        {
            parent_row[l]= -1;
            slack[l]= DBL_MAX;
        }

        for(k= 0;k<m;k++)
        {
            if(col_mate[k]<0)
            {
                unchosen_row[t++]= k;
            }
        }

    }

done:

#ifdef TEST
    for(k= 0;k<m;k++)
    {
        if (result == ERROR) break;

        for(l= 0;l<n;l++)
        {
            if ( !
                  (    ((ABS_OF(aa[ k ][ l ]) < 1000.0 * DBL_EPSILON) && (ABS_OF(row_dec[k] - col_inc[l]) < 1000.0 * DBL_EPSILON))
                    || ((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l])) >  -1e-8)
                    || ((aa[ k ][ l ] - row_dec[k] + col_inc[l]) >  -1000.0 * DBL_EPSILON)
                  )
                )
            {
                dbp("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                dbp(" ");
                dbp("Problem spotted in Hungarian code.");
                dbp(" ");
                dbw();
                dbp(" ");
                dbe(aa[ k ][ l ]);
                dbe(row_dec[k] - col_inc[l]);
                dbe(aa[ k ][ l ] - row_dec[k] + col_inc[l]);
                dbe((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l])));
                dbp("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

                result = ERROR;

#ifdef DEF_OUT
                ASSERT(    ((ABS_OF(aa[ k ][ l ]) < 1000.0 * DBL_EPSILON) && (ABS_OF(row_dec[k] - col_inc[l]) < 1000.0 * DBL_EPSILON))
                        || ((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l])) > -1e-7)
                        || ((aa[ k ][ l ] - row_dec[k] + col_inc[l]) >  -1000.0 * DBL_EPSILON)
                      );
#endif
            }
        }
    }

    for(k= 0;k<m;k++)
    {
        l= col_mate[k];

        ASSERT(l >= 0);

        if ( !
              (    ((ABS_OF(aa[ k ][ l ]) < 1000.0 * DBL_EPSILON) && (ABS_OF(row_dec[k] - col_inc[l]) < 1000.0 * DBL_EPSILON))
                || (ABS_OF((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l]))) < 1e-7)
                || (ABS_OF(aa[ k ][ l ] - row_dec[k] + col_inc[l]) < 1000.0 * DBL_EPSILON)
              )
            )
        {
            dbp("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            dbp(" ");
            dbp("Problem spotted in Hungarian code.");
            dbp(" ");
            dbw();
            dbp(" ");
            dbe(aa[ k ][ l ]);
            dbe(row_dec[k] - col_inc[l]);
            dbe(aa[ k ][ l ] - row_dec[k] + col_inc[l]);
            dbe((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l])));
            dbp("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

            result = ERROR;

#ifdef DEF_OUT
            ASSERT(    ((ABS_OF(aa[ k ][ l ]) < 1000.0 * DBL_EPSILON) && (ABS_OF(row_dec[k] - col_inc[l]) < 1000.0 * DBL_EPSILON))
                    || (ABS_OF((aa[ k ][ l ] - row_dec[k] + col_inc[l]) / (ABS_OF(aa[ k ][ l ]) + ABS_OF(row_dec[k] - col_inc[l]))) < 1e-7)
                    || (ABS_OF(aa[ k ][ l ] - row_dec[k] + col_inc[l]) < 1000.0 * DBL_EPSILON)
                  );
#endif
        }
    }

    k= 0;

    for(l= 0;l<n;l++)if(col_inc[l])k++;

    ASSERT(k <= m);

#endif

    if (transposed)
    {
        for(l= 0;l<n;l++)
        {
            row_assignments[ l ] = row_mate[ l ];

            if (row_mate[ l ] != -1)
            {
                cost += cost_mp->elements[ l ][ row_mate[ l ] ];
            }
        }
    }
    else
    {
        for(k= 0;k<m;k++)
        {
            row_assignments[ k ] = col_mate[k];
            cost += cost_mp->elements[ k ][ col_mate[k] ];
        }
    }

    NRE(matched_cols = INT_MALLOC(n));

    for(l= 0;l<n;l++)
    {
        matched_cols[ l ] = FALSE;
    }

    for(k= 0;k<m;k++)
    {
        j = row_assignments[ k ];

        if (j >= 0)
        {
            if (j >= n)
            {
                if (result != ERROR)
                {
                    kjb_clear_error();
                }
                add_error("Row %d is matched to out of range col %d.",
                          k, j);

                result = ERROR;
            }
            else if (matched_cols[ j ] == TRUE)
            {
                if (result != ERROR)
                {
                    kjb_clear_error();
                }
                add_error("Column %d is doubly matched.", j);

                result = ERROR;
            }

            matched_cols[ j ] = TRUE;
        }
    }

    kjb_free(matched_cols);


    *cost_ptr = cost;

    free_matrix(aa_mp);
    kjb_free(col_mate);
    kjb_free(row_mate);
    kjb_free(parent_row);
    kjb_free(unchosen_row);
    kjb_free(row_dec);
    kjb_free(col_inc);
    kjb_free(slack);
    kjb_free(slack_row);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

