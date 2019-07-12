/* $Id: test_matrix_indexing.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_mat_view.h"
#include "m_cpp/m_vec_view.h"
#include "m_cpp/m_int_matrix.h"
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include "l_cpp/l_test.h"
#include "l_cpp/l_index.h"

#include <boost/assign/std/vector.hpp>

using namespace kjb;
using namespace std;
using namespace boost::assign;

int main(int /* argc */, char** /* argv */)
{
    try{
        const int MAT_SIZE = 100;

        Matrix mat(MAT_SIZE, MAT_SIZE);

        for(int i = 0; i < MAT_SIZE; i++)
        {
            mat.set_col(i, create_random_vector(MAT_SIZE));
        }

        const Matrix ref_mat(mat);

        Vector sequence(MAT_SIZE);
        for(int i = 0; i < MAT_SIZE; i++)
        {
            sequence(i) = i;
        }

        // TEST PHASE 1:  Using Index_range::ALL
        const Index_range& _ = Index_range::ALL;

        // SIMPLE SET
        int rand_row = (int) sample(Uniform_distribution(0, MAT_SIZE));
        int rand_col = (int) sample(Uniform_distribution(0, MAT_SIZE));
        // set row
        mat(rand_row, _) = sequence;
        // set col
        mat(_, rand_col) = sequence;

        Vector test;
        test = mat(_, rand_col);
        TEST_TRUE(sequence == test);

        test.resize(0);
        test = mat(":", rand_col);
        TEST_TRUE(sequence == test);

        // CONFIRM AGAINST PARENT
        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            double EXPECTED;
            if (col == rand_col)
            {
                EXPECTED = row;
            }
            else if(row == rand_row)
            {
                EXPECTED = col;
            }
            else
            {
                EXPECTED = ref_mat(row,col);
            }

            TEST_TRUE(mat(row, col) == EXPECTED);
        }


        // SIMPLE GET
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        TEST_TRUE(mat(_, rand_col) == sequence);

        mat = ref_mat;
        mat(rand_row, _) = sequence;
        TEST_TRUE(mat(rand_row, _) == Matrix(sequence).transpose());

        // MULTIPLICATION
        // mult by scalar
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) *= -1;
        TEST_TRUE(mat(_, rand_col) == -sequence);
        // div by scalar
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) /= 2;
        TEST_TRUE(mat(_, rand_col) == sequence/2);


        // ADDITION
        // add to vector 
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) += sequence;
        TEST_TRUE(mat(_, rand_col) == 2 * sequence);

        // add to other view
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) += mat(_,rand_col);
        TEST_TRUE(mat(_, rand_col) == 2 * sequence);


        // SUBTRACTION
        // subtract from vector
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) -= sequence;
        TEST_TRUE(mat(_, rand_col) == 0 * sequence);

        // subtract from other view
        mat = ref_mat;
        mat(_, rand_col) = sequence;
        mat(_, rand_col) -= mat(_,rand_col);
        TEST_TRUE(mat(_, rand_col) == 0 * sequence);
        
        // PHASE 2: index by contiguous lists of indices
        Int_vector first_three(3);
        first_three(0) = 0;
        first_three(1) = 1;
        first_three(2) = 2;

        vector<size_t> last_three(3);
        last_three[0] = MAT_SIZE - 3;
        last_three[1] = MAT_SIZE - 2;
        last_three[2] = MAT_SIZE - 1;

        Matrix zeros = create_zero_matrix(3,3);

        mat = ref_mat;
        mat(first_three, last_three) = zeros;

        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            double EXPECTED = ref_mat(row, col);

            if(row < 3 && col >= MAT_SIZE - 3)
            {
                EXPECTED = 0.0;
            }

            TEST_TRUE(EXPECTED == mat(row, col));
        }


        // PHASE 3: index by non-contiguous lists of indices
        // set to matrix values
        int rand1 = (int) sample(Uniform_distribution(0, MAT_SIZE));
        int rand2 = (int) sample(Uniform_distribution(0, MAT_SIZE));
        int rand3 = (int) sample(Uniform_distribution(0, MAT_SIZE));
        Int_vector row_indices(3, 0);
        row_indices[0] = rand1;
        row_indices[1] = rand2;
        row_indices[2] = rand3;

        rand1 = (int) sample(Uniform_distribution(0, MAT_SIZE));
        rand2 = (int) sample(Uniform_distribution(0, MAT_SIZE));
        rand3 = (int) sample(Uniform_distribution(0, MAT_SIZE));

        vector<size_t> col_indices(3, 0);
        col_indices[0] = rand1;
        col_indices[1] = rand2;
        col_indices[2] = rand3;

        const double c_seq_matrix[][3] = {
            {1.0, 2.0, 3.0}, 
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}
        };

        Matrix seq_matrix(3,3, (double*) c_seq_matrix);

        mat = ref_mat;
        mat(row_indices, col_indices) = seq_matrix;

        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            Int_vector::reverse_iterator row_it = std::find(row_indices.rbegin(), row_indices.rend(), row);
            vector<size_t>::reverse_iterator col_it = std::find(col_indices.rbegin(), col_indices.rend(), col);

            double EXPECTED = ref_mat(row, col);
            if(row_it != row_indices.rend() && col_it != col_indices.rend())
            {
                // get the corresponding index of the seqence matrix
                int seq_row = row_it - row_indices.rbegin();
                int seq_col = col_it - col_indices.rbegin();

                // convert from "distance to end" to "distance from beginning
                seq_row = 2 - seq_row;
                seq_col = 2 - seq_col;
                EXPECTED = seq_matrix(seq_row, seq_col);
            }

            TEST_TRUE(EXPECTED == mat(row,col));
        }

        // add to matrix values
        mat(row_indices, col_indices) += seq_matrix;

        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            Int_vector::reverse_iterator row_it = std::find(row_indices.rbegin(), row_indices.rend(), row);
            vector<size_t>::reverse_iterator col_it = std::find(col_indices.rbegin(), col_indices.rend(), col);

            double EXPECTED = ref_mat(row, col);
            if(row_it != row_indices.rend() && col_it != col_indices.rend())
            {
                // get the corresponding index of the seqence matrix
                int seq_row = row_it - row_indices.rbegin();
                int seq_col = col_it - col_indices.rbegin();

                // convert from "distance to end" to "distance from beginning
                seq_row = 2 - seq_row;
                seq_col = 2 - seq_col;
                EXPECTED = 2 * seq_matrix(seq_row, seq_col);
            }

            TEST_TRUE(EXPECTED == mat(row,col));
        }
        // subtract matrix values
        mat(row_indices, col_indices) -= seq_matrix;

        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            Int_vector::reverse_iterator row_it = std::find(row_indices.rbegin(), row_indices.rend(), row);
            vector<size_t>::reverse_iterator col_it = std::find(col_indices.rbegin(), col_indices.rend(), col);

            double EXPECTED = ref_mat(row, col);
            if(row_it != row_indices.rend() && col_it != col_indices.rend())
            {
                // get the corresponding index of the seqence matrix
                int seq_row = row_it - row_indices.rbegin();
                int seq_col = col_it - col_indices.rbegin();

                // convert from "distance to end" to "distance from beginning
                seq_row = 2 - seq_row;
                seq_col = 2 - seq_col;
                EXPECTED = seq_matrix(seq_row, seq_col);
            }

            TEST_TRUE(EXPECTED == mat(row,col));
        }

        vector<size_t> first_and_last;
        first_and_last += 0, 49;

        Matrix identity = create_identity_matrix(50);
        identity(0, _) *= -1;
        identity(49, _) *= -1;
        // test inplicit cast in a function receiving a matrix
        Matrix inverse = matrix_inverse(identity(first_and_last, first_and_last));
        TEST_TRUE(inverse == -create_identity_matrix(2));


        // test failure cases
        //  index out of bounds -- trivial
        TEST_FAIL( mat(row_indices, MAT_SIZE) = seq_matrix);
        vector<size_t> junk_col_indices = col_indices;
        // index out of bounds -- part of collection
        junk_col_indices[0] = MAT_SIZE;
        TEST_FAIL( mat(row_indices, junk_col_indices) = seq_matrix);
        // dimension mismatch
        Matrix junk_seq_matrix = seq_matrix;
        junk_seq_matrix.resize(4,3);
        TEST_FAIL( mat(row_indices, col_indices) = junk_seq_matrix);
        TEST_FAIL( mat(row_indices, col_indices) += junk_seq_matrix);
        TEST_FAIL( mat(row_indices, col_indices) -= junk_seq_matrix);

        
        // PHASE 4: index by matlab-style indices
        mat = ref_mat;
        mat("0:9" , "0:9") = create_identity_matrix(10);

        for(int row = 0; row < MAT_SIZE; row++)
        for(int col = 0; col < MAT_SIZE; col++)
        {
            double EXPECTED = ref_mat(row, col);
            if(col < 10 && row < 10)
            {
                if(col != row)
                {
                    EXPECTED = 0.0;
                }
                else
                {
                    EXPECTED = 1.0;
                }
            }

            TEST_TRUE(EXPECTED == mat(row, col));
        }

        // flip rows and columns of submatrix
        // THIS ALSO TESTS OVERLAPPING ASSIGNMENT
        assert(MAT_SIZE >= 50); 
        mat = ref_mat;
        mat("0:49","0:49") = mat("49:-1:0", "49:-1:0");
        for(int row = 0; row < 50; row++)
        for(int col = 0; col < 50; col++)
        {
            double EXPECTED = ref_mat(49 - row, 49 - col);

            TEST_TRUE(EXPECTED == mat(row, col));
        }

        // set every even element to 0.0
        mat = ref_mat;
        assert(MAT_SIZE >= 100);
        mat("0:2:49,50:99","0:2:49,50:99") = 0.0;
        for(int row = 0; row < 100; row++)
        for(int col = 0; col < 100; col++)
        {
            double EXPECTED = ref_mat(row, col);

            if((row >= 50 || row % 2 == 0) && 
               (col >= 50 || col % 2 == 0))
            {
                EXPECTED = 0.0;
            }
            else if(row % 2 == 0 && col % 2 == 0)
            {
                EXPECTED = 0.0;
            }

            TEST_TRUE(EXPECTED == mat(row, col));
        }
    }
//    catch(Exception& ex)
//    {
//        cerr << ex.get_msg() << endl;
//        throw ex;
//    }
    catch(std::exception& ex)
    {
        cerr << ex.what() << endl;
        throw ex;
    }

    return 0;
}


