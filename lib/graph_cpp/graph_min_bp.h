#ifndef GRAPH_MIN_BP_H_INCLUDED_UOFARIZONA_VISION
#define GRAPH_MIN_BP_H_INCLUDED_UOFARIZONA_VISION

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_int_matrix.h>

namespace kjb {

#if 0
template< typename MAT >
int min_bipartite_match( const MAT&, Int_vector*, MAT::Value_type* );

template<>
int min_bipartite_match<Matrix>(
    const Matrix&,
    Int_vector*,
    Matrix::Value_type*
);

template<>
int min_bipartite_match<Int_matrix>(
    const Int_matrix&,
    Int_vector*,
    Int_matrix::Value_type*
);

#else

int min_bipartite_match( const Matrix&, Int_vector*, Matrix::Value_type* );

#endif

}
#endif /* GRAPH_MIN_BP_H_INCLUDED_UOFARIZONA_VISION */
