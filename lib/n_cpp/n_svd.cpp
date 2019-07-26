/* $Id: n_svd.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <n_cpp/n_svd.h>
#include <n/n_svd.h>
#include <l_cpp/l_util.h>

namespace kjb
{

void Svd::compute_svd( const kjb_c::Matrix* pa )
{
    KJB( ETX( do_svd(
            pa,
            & mat_u.get_underlying_representation_with_guilt(),
            & vec_d.get_underlying_representation_with_guilt(),
            & mat_vt.get_underlying_representation_with_guilt(),
            & rank
        ) ) );
}

void Svd::compute_svd( const Matrix& mat_a )
{
    compute_svd( mat_a.get_c_matrix() );
}

Svd::Svd( const Matrix& mat_a )
{
    compute_svd( mat_a );
}

Svd::Svd( const kjb_c::Matrix* pa )
{
    compute_svd( pa );
}

void Svd::swap( Svd& other )
{
    mat_u.swap( other.mat_u );
    vec_d.swap( other.vec_d );
    mat_vt.swap( other.mat_vt );
    std::swap( rank, other.rank );
}

const Matrix& Svd::u() const
{
    return mat_u;
}

const Vector& Svd::d() const
{
    return vec_d;
}

const Matrix& Svd::vt() const
{
    return mat_vt;
}

Matrix Svd::reconstruction() const
{
    return mat_u * create_diagonal_matrix( vec_d ) * mat_vt;
}

} // namespace kjb
