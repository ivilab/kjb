/* $Id: g_camera.cpp 21776 2017-09-17 16:44:49Z clayton $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#include <g_cpp/g_camera.h>
#include <g_cpp/g_quaternion.h>
#include <l_cpp/l_exception.h>

namespace kjb {

template <class VectorType, class MatrixType, class M_MatrixType>
VectorType backproject_dispatch_
(
    const VectorType& homo_screen_coord,
    const MatrixType& camera_matrix
)
{
    M_MatrixType m_inv;
    m_inv.resize(3,3);

    m_inv(0,0) = camera_matrix(0,0);
    m_inv(0,1) = camera_matrix(0,1);
    m_inv(0,2) = camera_matrix(0,2);

    m_inv(1,0) = camera_matrix(1,0);
    m_inv(1,1) = camera_matrix(1,1);
    m_inv(1,2) = camera_matrix(1,2);

    m_inv(2,0) = camera_matrix(2,0);
    m_inv(2,1) = camera_matrix(2,1);
    m_inv(2,2) = camera_matrix(2,2);

    m_inv = matrix_inverse(m_inv);
    return backproject_with_m_inv(homo_screen_coord, m_inv);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector3 backproject
(
    const Vector3& homo_screen_coord,
    const Matrix_d<3,4>& camera_matrix
)
{
    return backproject_dispatch_<Vector3, Matrix_d<3,4>, Matrix_d<3,3> >(
                                                            homo_screen_coord,
                                                            camera_matrix);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector backproject
(
    const Vector& homo_screen_coord,
    const Matrix& camera_matrix)
{
    return backproject_dispatch_<Vector, Matrix, Matrix >(
                                                    homo_screen_coord,
                                                    camera_matrix);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class VectorType, class MatrixType>
VectorType backproject_with_m_inv_dispatch_
(
    const VectorType& homo_screen_coord,
    const MatrixType& M_inv
)
{
    // find x's backprojection onto the plane at infinity
    // (This is the projection line's direction vector)
    VectorType d = M_inv * homo_screen_coord;
    assert(d.size() == 3);
    d.normalize();

    return d;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector backproject_with_m_inv
(
    const Vector& homo_screen_coord,
    const Matrix& M_inv
)
{
    if(homo_screen_coord.size() != 3)
    {
        KJB_THROW_2(Illegal_argument,
                    "Point must be in 2D homogenuous coordinates "
                    "(pt.size() == 3)");
    }

    return backproject_with_m_inv_dispatch_(homo_screen_coord, M_inv);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector3 backproject_with_m_inv
(
    const Vector3& homo_screen_coord,
    const Matrix_d<3,3>& M_inv
)
{
    return backproject_with_m_inv_dispatch_(homo_screen_coord, M_inv);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix lerp_extrinsic_camera_matrix(
        const Matrix& m1,
        const Matrix& m2,
        double t,
        bool use_slerp)
{
    assert(t >= 0.0);
    assert(t <= 1.0);

    boost::array<Vector, 2> T;
    T[0] = m1.get_col(3);
    T[1] = m2.get_col(3);

    // make sure the translation vectors are in standard homogeneous form
    assert(fabs(T[0][3]-1.0) < FLT_EPSILON);
    assert(fabs(T[0][3]-1.0) < FLT_EPSILON);

    Matrix tmp = m1;
    tmp.resize(3,3);
    Quaternion q1(tmp);

    tmp = m2;
    tmp.resize(3,3);
    Quaternion q2(tmp);

    if(use_slerp)
        q1 = slerp(q1, q2, t);
    else
        q1 = nlerp(q1, q2, t);
//    q1 = slerp2(q1, q2, t);

    tmp = q1.get_rotation_matrix();

    assert(tmp.get_num_rows() == 4);
    assert(tmp.get_num_cols() == 4);

    T[0] = lerp(T.begin(), T.end(), t);
    tmp.set_col(3, T[0]);

    return tmp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix lerp_extrinsic_camera_matrix(
        const std::vector<Matrix>& extrinsic,
        const std::vector<double>& timestamps,
        double t,
        bool use_slerp)
{
    assert(extrinsic.size() == timestamps.size());

    if(t > timestamps.back() || t < timestamps.front())
        KJB_THROW(Index_out_of_bounds);

    typedef std::vector<double>::const_iterator Time_iterator;
    // NOT_USED typedef std::vector<Matrix>::const_iterator Mat_iterator;

    Time_iterator successor = std::upper_bound(
            timestamps.begin(),
            timestamps.end(),
            t);

#ifdef TEST
    // this case is handled naturally later, assuming the
    // assert below holds
    if(successor == timestamps.end())
        assert(t == timestamps.back());
#endif

    size_t i = std::distance(timestamps.begin(), successor) - 1;

    if(t == timestamps[i]) 
    {
        return extrinsic[i];
    }

    // if this was false, the previous if statement (t == timestamps[i]
    // should have been true by the nature of the upper_bound function
    assert(i+1  < timestamps.size());

    double t1 = timestamps[i];
    double t2 = timestamps[i+1];

    // convert to relative
    t = (t-t1) / (t2 - t1);

    return lerp_extrinsic_camera_matrix(
        extrinsic[i],
        extrinsic[i+1],
        t,
        use_slerp
    );
}

} // namespace kjb;

