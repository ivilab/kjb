/* $Id: test_matrix_d.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#include <m_cpp/m_matrix_d.h>
#include <m_cpp/m_matrix_d.impl.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <functional>
#include <algorithm>

#include <boost/foreach.hpp>

#include "l_cpp/l_exception.h"
#include "l_cpp/l_test.h"

#include <l/l_sys_rand.h>

using namespace kjb;

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSE>
void fill_with_random(Matrix_d<NROWS, NCOLS, TRANSPOSE>& mat)
{
    for(size_t row = 0; row < NROWS; row++)
    {
        mat[row] = create_random_vector<NCOLS>();
    }
}

void fill_with_random(kjb::Matrix& m)
{
    m = create_random_matrix(m.get_num_rows(), m.get_num_cols());
}

void fill_with_random(std::vector<std::vector<double> >& m)
{
    for(size_t row = 0; row < m.size(); row++)
    for(size_t col = 0; col < m[0].size(); col++)
    {
        m[row][col] = kjb_c::kjb_rand();
    }
}

int main()
{
    // CONSTRUCTORS
    {
    // default
        Matrix_d<1,1> m;
    } {
    // init
        Matrix_d<2,2> a(1.0);
        TEST_TRUE(a[0][0] == 1.0);
        TEST_TRUE(a[0][1] == 1.0);
        TEST_TRUE(a[1][0] == 1.0);
        TEST_TRUE(a[1][1] == 1.0);
    } {
    // equality comparison
        Matrix_d<2,3> a;
        Matrix_d<2,3> b;
        for(size_t row = 0; row < 2; row++)
        for(size_t col = 0; col < 3; col++)
            a(row, col) = b(row, col) = row*row + col * col;

        TEST_TRUE(a == b);
        TEST_FALSE(a != b);

        a(0, 1) = 1234567890;
        TEST_FALSE(a == b);
        TEST_TRUE(a != b);
    } {
    // copy
        Matrix_d<3,2> a;
        fill_with_random(a);
        Matrix_d<3,2> b( a );

        TEST_TRUE(a == b);

        // this shouldn't compile:
//        Matrix_d<1,2> c( (Matrix_d<2,1>()) );
//        Matrix_d<2,1> d( (Matrix_d<1,2>()) );

    // transposed self?
        Matrix_d<2,3> e( a.transpose() );
        TEST_TRUE(e == a.transpose());
        TEST_TRUE(e.transpose() == a);
        TEST_TRUE(a.transpose() == e);

        // shouldn't compile
//        Matrix_d<3,2> f( a.transpose() );
    } {
    // fill
        Matrix_d<3,2> a;
        fill_with_random(a);

        std::vector<std::vector<double> > b(3, std::vector<double>(2));
        fill_with_random(b);

        std::vector<std::vector<double> > trash1(4, std::vector<double>(2));
        std::vector<std::vector<double> > trash2(3, std::vector<double>(3));

        Matrix_d<3,2> a_copy(a.begin());
        Matrix_d<3,2> b_copy(b.begin());

        // dimension msimatch
//        TEST_FAIL((new Matrix_d<3,2>(trash1.begin()))); // this won't fail because first dimension can't be bounds-checked, since only begin is sent, not end
        TEST_FAIL((new Matrix_d<3,2>(trash2.begin())));

        TEST_TRUE(a == a_copy);
        TEST_TRUE(b[0][0] == b_copy(0,0));
        TEST_TRUE(b[0][1] == b_copy(0,1));
        TEST_TRUE(b[1][0] == b_copy(1,0));
        TEST_TRUE(b[1][1] == b_copy(1,1));
        TEST_TRUE(b[2][0] == b_copy(2,0));
        TEST_TRUE(b[2][1] == b_copy(2,1));
    } {
    // convert-from-dynamic-matrix
        Matrix a = create_random_matrix(3, 2);
        Matrix_d<3,2> b(a);
        TEST_TRUE(b == a);
        TEST_TRUE(a == b);

        // dimension_mismatch
        Matrix_d<2,2>* c;
        TEST_FAIL((c = new Matrix_d<2, 2>(a)));
    } {
    // initialize from value
        Matrix_d<3,2> a(1.2);

        // also test iterators a bit...
        BOOST_FOREACH(Vector2& row, a)
        {
            BOOST_FOREACH(double element, row)
            {
                TEST_TRUE(element == 1.2);
            }
        }
    } {
//    // copy-from-single-row
//        Vector3 row(1.0, 2.0, 3.0);
//        Matrix_d<1,3> a(row);
//        TEST_TRUE(a.front() == row);
//    } {
//    // copy-from-two-rows
//        Vector3 row(1.0, 2.0, 3.0);
//        Vector3 row2(4.0, 5.0, 6.0);
//        Matrix_d<2,3> a(row, row2);
//
//        // shouldn't compile
////        Matrix_d<4,3> b(row, row2, row2);
////        Matrix_d<4,3> c(row);
//
//        TEST_TRUE(a[0] == row);
//        TEST_TRUE(a[1] == row2);
//    } {
//    // copy-from-three-rows
//        Vector3 row(1.0, 2.0, 3.0);
//        Vector3 row2(4.0, 5.0, 6.0);
//        Vector3 row3(7.0, 8.0, 9.0);
//        Matrix_d<3,3> a(row, row2, row3);
//        // shouldn't compile:
//        Matrix_d<4,3> b(row, row2, row3, row3);
////        Matrix_d<4,3> c(row, row2);
//        TEST_TRUE(a[0] == row);
//        TEST_TRUE(a[1] == row2);
//        TEST_TRUE(a[2] == row3);
//    } {
//    // copy-from-four-rows
//        Vector3 row(1.0, 2.0, 3.0);
//        Vector3 row2(4.0, 5.0, 6.0);
//        Vector3 row3(7.0, 8.0, 9.0);
//        Vector3 row4(10.0, 21.0, 32.0);
//        Vector4 bad_row();
//        Matrix_d<4,3> a(row, row2, row3, row4);
//        // shouldn't compile:
////        Matrix_d<4,3> b(row, row2, row3);
////        Matrix_d<4,3> c(row, row2, row3, bad_row);
//        TEST_TRUE(a[0] == row);
//        TEST_TRUE(a[1] == row2);
//        TEST_TRUE(a[2] == row3);
//        TEST_TRUE(a[3] == row4);
    }
// ASSIGNMENT 
    {
    // from self
        Matrix_d<3,2> a;
        Matrix_d<3,2> a_copy;
        fill_with_random(a);
        a_copy = a;
        TEST_TRUE(a_copy == a);
    } {
    // from transposed_self?
        Matrix_d<3,2> a;
        Matrix_d<2,3> a_copy;
        Matrix_d<2,3> a_copy_2;
        fill_with_random(a);
        a_copy = a.transpose();
        TEST_TRUE(a_copy.transpose() == a);
        TEST_TRUE(a_copy == a.transpose());

        a_copy_2.transpose() = a;
        TEST_TRUE(a_copy_2.transpose() == a);
        TEST_TRUE(a_copy_2 == a.transpose());
    } {
    // from dynamic matrix
        Matrix_d<3,2> a;
        Matrix_d<2,3> a_t;

        Matrix b(3, 2);
        fill_with_random(b);
        a = b;
        a_t.transpose() = b;
        TEST_TRUE(a == b);
        TEST_TRUE(b == a);
        TEST_TRUE(a_t.transpose() == b);
        TEST_TRUE(b == a_t.transpose());
//    } {
//    // vector1 from vector1
//        Matrix_d<5, 1> a;
//        Matrix_d<1, 5> a_t;
//        Vector_d<5> b = create_random_vector<5>();
//        a = Matrix_d<5, 1>(b);
//        assert(a == b);
//        b = create_random_vector<5>();
//        b = a;
//        assert(a == b);
//
//        a_t.transpose() = b;
//        assert(a_t.front().transpose() == b);
//        assert(a_t.transpose() == b);
//        b = create_random_vector<5>();
//        b = a_t.transpose();
//        assert(a_t.transpose() == b);
    }
    
    // SUBTRACTION, NEGATION
    {
    // from self
        Matrix_d<3,2> m1;
        fill_with_random(m1);
        Matrix_d<3,2> m2(m1);
        m2(2,1) += 1;
        Matrix_d<2,3> m2_t(m1.transpose());
        m2_t(1,2) += 1;

        Matrix_d<3,2> out(0.0);
        out(2,1) = -1.0;

        Matrix_d<2,3> out_t(0.0);
        out_t(1,2) = -1.0;

        assert(out == (m1 - m2));
        assert(-out == (m2 - m1));
        assert(out == (m1 - m2_t.transpose()));
        assert(-out == (m2_t.transpose() - m1));
        assert(-out_t == (m2_t - m1.transpose()));
        assert(out_t == ( m1.transpose() - m2_t));
    } {
    // from dynamic
        Matrix m1(3, 2, 0.0);
        fill_with_random(m1);
        Matrix_d<3,2> m2(m1);
        m2(2,1) += 1;
        Matrix_d<2,3> m2_t(m1.transpose());
        m2_t(1,2) += 1;

        Matrix_d<3,2> out(0.0);
        out(2,1) = -1.0;

        Matrix_d<2,3> out_t(0.0);
        out_t(1,2) = -1.0;

        assert(out == (m1 - m2));
        assert(-out == (m2 - m1));
        assert(out == (m1 - m2_t.transpose()));
        assert(-out == (m2_t.transpose() - m1));
        assert(-out_t == (m2_t - m1.transpose()));
        assert(out_t == ( m1.transpose() - m2_t));
    } {
    // ADDITION
    // with self
        Matrix_d<3,2> m1;
        fill_with_random(m1);
        Matrix_d<3,2> m2(-m1);
        m2(2,1) += 1;
        Matrix_d<2,3> m2_t(-m1.transpose());
        m2_t(1,2) += 1;

        Matrix_d<3,2> out(0.0);
        out(2,1) = 1.0;

        Matrix_d<2,3> out_t(0.0);
        out_t(1,2) = 1.0;

        assert(out == (m1 + m2));
        assert(out == (m2 + m1));
        assert(out == (m1 + m2_t.transpose()));
        assert(out == (m2_t.transpose() + m1));
        assert(out_t == (m2_t + m1.transpose()));
        assert(out_t == ( m1.transpose() + m2_t));
    } {
    // with dynamic
        Matrix m1(3, 2);
        fill_with_random(m1);
        Matrix_d<3,2> m2(-m1);
        m2(2,1) += 1;
        Matrix_d<2,3> m2_t(-m1.transpose());
        m2_t(1,2) += 1;

        Matrix_d<3,2> out(0.0);
        out(2,1) = 1.0;

        Matrix_d<2,3> out_t(0.0);
        out_t(1,2) = 1.0;

        assert(out == (m1 + m2));
        assert(out == (m2 + m1));
        assert(out == (m1 + m2_t.transpose()));
        assert(out == (m2_t.transpose() + m1));
        assert(out_t == (m2_t + m1.transpose()));
        assert(out_t == ( m1.transpose() + m2_t));
    } {
    // MULTIPLICATION
    // to normal
        Matrix m1_dyn(2, 3);
        fill_with_random(m1_dyn);
        Matrix m2_dyn(3, 5);
        fill_with_random(m2_dyn);
        Matrix TRASH_1(3, 100);

        Matrix out_dyn = m1_dyn * m2_dyn;

        Matrix_d<2, 3> m1(m1_dyn);
        Matrix_d<3, 2> m1_t(m1_dyn.transpose());
        Matrix_d<3, 5> m2(m2_dyn);
        Matrix_d<5, 3> m2_t(m2_dyn.transpose());
        Matrix_d<2, 5> out(out_dyn);

        TEST_TRUE(out == (m1 * m2));
        TEST_TRUE(out == (m1 * m2_t.transpose()));
        TEST_TRUE(out == (m1_t.transpose() * m2_t.transpose()));
        TEST_TRUE(out == (m1_t.transpose() * m2));
        TEST_TRUE(out == (m2_t * m1_t).transpose());
        TEST_TRUE(out == (m2_t * m1.transpose()).transpose());
        TEST_TRUE(out == (m2.transpose() * m1_t).transpose());
        TEST_TRUE(out == (m2.transpose() * m1.transpose()).transpose());

        TEST_TRUE(out == (m1 * m2_dyn));
        TEST_TRUE(out == (m1_t.transpose() * m2_dyn));
        TEST_TRUE(out == (m1_dyn * m2));
        TEST_TRUE(out == (m1_dyn * m2_t.transpose()));

        // dimension mismatch
        TEST_FAIL(out == (TRASH_1 * m2));
        TEST_FAIL(out == (m2 * TRASH_1));

    } {
    // matrix-vector multiplication
        Matrix m34_dyn(3, 4);
        fill_with_random(m34_dyn);
        Matrix_d<3, 4> m34(m34_dyn);

        Vector v4_dyn = create_random_vector(4);
        Vector4 v4(v4_dyn.begin());

        Vector out_dyn = m34_dyn * v4_dyn;
        Vector3 out(out_dyn);

        TEST_TRUE(out == m34 * v4);
    }

    std::cout << "All tests passed." << std::endl;

    return 0;
}
