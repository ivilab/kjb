/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2009-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek, Andrew Predoehl                                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_matrix.cpp 5908 2010-05-21 23:31:54Z ksimek $ */

#include <l/l_sys_io.h>
#include "m_cpp/m_matrix_stream_io.h"

#include "l_cpp/l_test.h"
#include "l_cpp/l_int_matrix.h"
#include <fstream>
#include <l_cpp/l_int_matrix.h>

const char* FNAME = "temp_io_matrix";

void test_read_write_int_matrix()
{
    using namespace kjb;
    
    Int_matrix mat(2,3);
    mat(0,0) = 1;
    mat(0,1) = 2;
    mat(0,2) = 32342314;
    
    mat(1,0) = -1888282;
    mat(0,1) = 2;
    mat(1,2) = -4;
    
    std::ofstream out;
    out.open( FNAME );
    TEST_FALSE(out.fail());

    Matrix_stream_io::write_int_matrix(out, mat);
    
    out.close();
    
    Int_matrix mat2;
    std::ifstream in;
    in.open( FNAME );
    TEST_FALSE(in.fail());
    Matrix_stream_io::read_int_matrix(in, mat2);
    
    TEST_TRUE(mat == mat2);
    kjb_c::kjb_unlink( FNAME );
}

int main(int /* argc */, char ** /* argv */)
{
    TEST_FALSE( kjb_c::is_file( FNAME ) ); // that name ought to be unused now

    test_read_write_int_matrix();
    RETURN_VICTORIOUSLY(); 
}
