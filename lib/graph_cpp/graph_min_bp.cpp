/**
 * @file
 * @author Andrew Predoehl
 * @brief Implementation of dynamic programming solution to Matching
 */
/*
 * $Id: graph_min_bp.cpp 13649 2013-01-24 17:40:40Z predoehl $
 */

#include <l/l_def.h>
#include <l_cpp/l_util.h>
#include <graph_cpp/graph_min_bp.h>
#include <m_cpp/m_int_vector.h>

#include <vector>
#include <map>
#include <algorithm>

//define HERE printf("Now at %s:%d in %s().\n", __FILE__, __LINE__, __func__ )

namespace {

using kjb_c::NO_ERROR;
using kjb_c::ERROR;

using kjb::Matrix;
using kjb::Int_vector;

const int LONELY = -1;

struct Solution
{
    Int_vector association;
    Matrix::Value_type cost;
};

typedef std::map< Int_vector, Solution > Memo;

Solution solve_subsquare(
    const Matrix&,
    std::vector< Memo >&,
    Int_vector&,
    Int_vector& 
);

/// return a vector of values 0, 1, . . . , n-1 in that order.
Int_vector ramp( int n )
{
    Int_vector result( n );
    while( --n >= 0 )
    {
        result.at( n ) = n;
    }
    return result;
}


bool valid( int m, const Int_vector& sorted_row, const Int_vector& sorted_col )
{
    const int nnn = sorted_row.get_length();
    if ( m < nnn || nnn != sorted_col.get_length() )
    {
        return false;
    }

    for( int iii = 1; iii < nnn; ++iii )
    {
        if  (       sorted_row.at( iii - 1 ) == sorted_row.at( iii )
                ||  sorted_col.at( iii - 1 ) == sorted_col.at( iii )
                ||  sorted_row.at( iii ) < 0 || m <= sorted_row.at( iii )
                ||  sorted_col.at( iii ) < 0 || m <= sorted_col.at( iii )
            )
        {
            return false;
        }
    }
    return true;
}


Int_vector concat( const Int_vector& a, const Int_vector& b )
{
    Int_vector result( a.get_length() + b.get_length() );
    for ( int iii = 0; iii < a.get_length(); ++iii )
    {
        result.at( iii ) = a.at( iii );
    }
    for ( int iii = 0; iii < b.get_length(); ++iii )
    {
        result.at( iii + a.get_length() ) = b.at( iii );
    }
    return result;
}

Int_vector expel_one( const Int_vector& v, int i )
{
    Int_vector result( v );
    result.at( i ) = result.at( v.get_length() - 1 );
    result.resize( v.get_length() - 1 );
    return result;
}

Solution grind_subsquare(
    const Matrix& weights,
    std::vector< Memo >& memos,
    Int_vector& row,
    Int_vector& col,
    const Int_vector& key
)
{
    const int mmm = row.get_length();
    KJB( ASSERT( mmm == col.get_length() ) );
    KJB( ASSERT( 2 * mmm == key.get_length() ) );

    Solution s_best;
    for( int k = 0; k < mmm; ++k )
    {
        Int_vector r_nm( row );
        r_nm.resize( mmm - 1 );
        const int& r_m = row.at( mmm-1 );
        Int_vector c_nk( expel_one( col, k ) );
        const int& c_k = col.at( k );
        // assume r_m, c_k are paired, and solve rest of association:
        Solution s = solve_subsquare( weights, memos, r_nm, c_nk );
        KJB( ASSERT( 2 * mmm - 2 == s.association.size() ) );
        // compute cost of that solution (it might be terrible).
        Matrix::Value_type cost = s.cost + weights.at( r_m, c_k );

        // compare this solution with the best (if any) so far; keep best.
        if ( 0 == s_best.association.size() || cost < s_best.cost )
        {
            s_best.cost = cost;
            // merge the solutions
            s_best.association.resize( 2 * mmm );
            int jjj = 0;
            for( ; jjj < mmm - 1; ++jjj )
            {
                s_best.association.at( jjj ) = s.association.at( jjj );
            }
            s_best.association.at( jjj++ ) = r_m;
            for( ; jjj < 2*mmm - 1; ++jjj )
            {
                s_best.association.at( jjj ) = s.association.at( jjj-1 );
            }
            s_best.association.at( jjj ) = c_k;
        }
    }

    // memoize and return the answer
    memos[mmm][key] = s_best;
    return s_best;
}


Solution solve_subsquare(
    const Matrix& weights,
    std::vector< Memo >& memos,
    Int_vector& row,
    Int_vector& col
)
{
    KJB( ASSERT( row.get_length() && row.get_length() == col.get_length() ) );

    const int mmm = col.get_length();

    // recursive base case
    if ( 1 == mmm )
    {
        Solution s;
        s.cost = weights.at( row.at( 0 ), col.at( 0 ) );
        s.association.resize( 2 * mmm, 0 );
        s.association.at( 0 ) = row.at( 0 );
        s.association.at( 1 ) = col.at( 0 );
        return s;
    }

    std::sort( row.begin(), row.end() );
    std::sort( col.begin(), col.end() );
    KJB( ASSERT( valid( weights.get_num_rows(), row, col ) ) );

    // Try for memoized solution
    Solution s;
    Int_vector key( concat( row, col ) );
    if ( mmm < int( memos.size() ) && (s=memos[mmm][key]).association.size() )
    {
        return s;
    }

    // Solve the hard way, then memoize
    while( int( memos.size() ) <= mmm )
    {
        memos.push_back( Memo() );
    }
    return grind_subsquare( weights, memos, row, col, key );
}


int solve_square(
    const Matrix& weights,
    Int_vector* assignment,
    Matrix::Value_type* cost
)
{
    const int nnn = weights.get_num_rows();
    KJB( ASSERT( nnn == weights.get_num_cols() ) );

    std::vector< Memo > memos;
    Int_vector rl( ramp( nnn ) ), cl( ramp( nnn ) );
    Solution s = solve_subsquare( weights, memos, rl, cl );

    KJB( ASSERT( 2 * nnn == s.association.get_length() ) );
    for( int iii = 0; iii < nnn; ++iii )
    {
        assignment -> at( s.association.at( iii ) )
                                            = s.association.at( nnn+iii );
    }
    if ( cost ) *cost = s.cost;

    return NO_ERROR;
}


int solve_nonsquare(
    const Matrix& weights,
    Int_vector* assignment,
    Matrix::Value_type* cost
)
{
    const int SKEW = weights.get_num_cols() - weights.get_num_rows();

    KJB( ASSERT( SKEW ) );

    if ( SKEW < 0 )
    {
        Int_vector atrans( weights.get_num_cols() );
        KJB( ERE( solve_nonsquare( weights.transpose(), &atrans, cost ) ) );
        for( int iii = 0; iii < weights.get_num_rows(); ++iii )
        {
            assignment -> at( iii ) = LONELY;
        }
        for( int iii = 0; iii < atrans.get_length(); ++iii )
        {
            assignment -> at( atrans.at( iii ) ) = iii;
        }
        return NO_ERROR;
    }

    KJB( ASSERT( 0 < SKEW ) );

    const Matrix::Value_type BIG = 1 + 2 * max( weights );
    Matrix square_weights( weights );
    square_weights.resize(weights.get_num_cols(), weights.get_num_cols(), BIG);

    Int_vector a_long( weights.get_num_cols() );
    Matrix::Value_type c_long;
    KJB( ERE( solve_square( square_weights, &a_long, &c_long ) ) );
    for( int iii = 0; iii < weights.get_num_cols(); ++iii )
    {
        assignment -> at( iii ) = a_long.at( iii );
    }
    if ( cost ) *cost = c_long - BIG * SKEW;

    return NO_ERROR;
}

}

namespace kjb {

/**
 * @brief solve a matching with minimum cost in a complete bipartite graph.
 *
 * This does NOT use the Hungarian algorithm.  It uses a dynamic programming
 * approach, and it is slower but simple to understand.  It might be too slow
 * if the graph is large.
 *
 * The input graph is implicitly a complete bipartite graph G=(L,R,E,w) where
 * L = (l0, l1, . . . , lM) and R = (r0, r1, . . . , rN) are the disjoint sets
 * of vertices, E is the edge set, and w : E --> R is the weight function.
 * Every weight must be nonnegative, because it denotes a cost.  Costs are
 * additive, so the optimal assignment is the sum of the selected edges, and
 * this routine computes an optimum in the sense that no other assignment has
 * smaller total cost.
 *
 * @param[in] weights       Matrix of weights, where the value at row i and
 *                          column j equals the weight of edge {li,rj}.
 * @param[out] assignment   Points to vector into which the output will be
 *                          written; it will be resized to length (M+1).
 *                          If N==M, assignment->at(i) = j denotes that edge
 *                          {li,rj} is in the optimal matching.
 *                          However, entries of '-1' occur when N > M, and
 *                          they indicate unassigned vertices of L.
 *                          This pointer must not equal null.
 * @param[out] cost         Optional output pointer; if not equal to null, then
 *                          *cost will contain the total cost of assignment.
 *
 * @return ERROR or NO_ERROR depending on outcome.
 */
int min_bipartite_match(
    const Matrix& weights,
    Int_vector* assignment,
    Matrix::Value_type* cost
)
{
    KJB( NRE( assignment ) );
    if ( weights.get_length() < 2 )
    {
        if ( weights.get_length() )
        {
            assignment -> resize( 1, 0 );
            if ( cost ) *cost = weights.at(0,0);
        }
        return NO_ERROR;
    }

    if ( min( weights ) < 0 )
    {
        kjb_c::set_error("Input matrix of weights contains a negative cost.");
        return ERROR;
    }

    assignment -> resize( weights.get_num_rows() );

    if ( weights.get_num_cols() != weights.get_num_rows() )
    {
        return solve_nonsquare( weights, assignment, cost );
    }
    return solve_square( weights, assignment, cost );
}

}
