/**
 * @brief test for proper singleton behavior from class Tile_manager
 * @file
 * @author Andrew Predoehl
 *
 * Class Tile_manager is supposed to use the "singleton" pattern, i.e., only
 * one object can be instantiated from this class.  But does it really enforce
 * that stipulation?  Let's find out!
 *
 * Hurray for test programs:  I discovered a file pointer leak in topo/master.c
 * with this program.
 */
/*
 * $Id: singleton_man.cpp 14848 2013-07-04 06:38:17Z predoehl $
 */
#include <l/l_init.h>
#include <l/l_sys_io.h>
#include <l/l_sys_lib.h>
#include <l_cpp/l_test.h>
#include <topo/index.h>
#include <topo_cpp/dorthoquad.h>

int test1()
{
    // instantiate it once:  shame on you.
    kjb::TopoFusion::Tile_manager t1;

    // instantiate it twice:  shame on me.
    TEST_FAIL( kjb::TopoFusion::Tile_manager t2 );
    TEST_FAIL( kjb::TopoFusion::Tile_manager t2("foo") );

    return kjb_c::NO_ERROR;
}

int main()
{
    KJB(EPETE(kjb_init()));
    KJB(EPETE(test1()));
    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

