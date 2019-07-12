/**
 * @file
 * @author Andy Predoehl
 * @brief unit test for Temporary_Directory class
 */
/*
 * $Id: test_tempdir.cpp 21356 2017-03-30 05:34:45Z kobus $
 */

#include <l/l_sys_lib.h>
#include <l/l_debug.h>
#include <l/l_init.h>
#include <l_cpp/l_stdio_wrap.h>
#include <l_cpp/l_util.h>

namespace
{

int main2()
{
    // test that it creates and destroys an empty directory (and, get_pathname)
    std::string name, filenam = "funnybunny";
    if (1)
    {
        kjb::Temporary_Directory td;
        name = td.get_pathname();
        KJB( ASSERT( is_directory( name.c_str() ) ) );
    }
    KJB( ASSERT( ! is_directory( name.c_str() ) ) );

    // test the remove method
    if (1)
    {
        kjb::Temporary_Directory td;
        name = td.get_pathname();
        KJB( ASSERT( is_directory( name.c_str() ) ) );
        KJB( EPETE( td.remove() ) );
        KJB( ASSERT( ! is_directory( name.c_str() ) ) );
    }
    KJB( ASSERT( ! is_directory( name.c_str() ) ) );

    // test that it cannot destroy a nonempty directory
    if (1)
    {
        kjb::Temporary_Directory td;
        name = td.get_pathname();
        KJB( ASSERT( is_directory( name.c_str() ) ) );
        kjb::File_Ptr_Write fp( name + DIR_STR + filenam );
        kjb_c::kjb_fputs( fp, "hello dolly" );
        if (kjb_c::is_interactive())
        {
            TEST_PSE(("PLEASE DISREGARD THE FOLLOWING ERROR MESSAGE!\n"));
        }
        else // automated test gets confused by the always-changing random
        {    // letters in the tempdir name, so throw away the message.
            td.set_silence_flag();
        }
    }
    KJB( ASSERT( is_directory( name.c_str() ) ) );
    // ok we've proved our point, so clean it up manually
    KJB( EPETE( kjb_unlink( (name + DIR_STR + filenam).c_str() ) ) );
    KJB( EPETE( kjb_rmdir( name.c_str() ) ) );

    // test that the nuke option destroys even a nonempty directory
    if (1)
    {
        kjb::Temporary_Directory td;
        name = td.get_pathname();
        KJB( ASSERT( is_directory( name.c_str() ) ) );
        kjb::File_Ptr_Write fp( name + DIR_STR + filenam );
        kjb_c::kjb_fputs( fp, "hello dolly" );
        KJB( ASSERT( is_directory( name.c_str() ) ) );
        KJB( EPETE( td.recursively_remove() ) );
        KJB( ASSERT( ! is_directory( name.c_str() ) ) );
    }
    KJB( ASSERT( ! is_directory( name.c_str() ) ) );

    // test self-nuking
    if (1)
    {
        kjb::Temporary_Recursively_Removing_Directory td;
        name = td.get_pathname();
        KJB( ASSERT( is_directory( name.c_str() ) ) );
        kjb::File_Ptr_Write fp( name + DIR_STR + filenam );
        kjb_c::kjb_fputs( fp, "hello dolly" );
    }
    KJB( ASSERT( ! is_directory( name.c_str() ) ) );

    return kjb_c::NO_ERROR;
}

}

int main ( int argc, char** argv )
{
    kjb_c::kjb_init();

    try {
        KJB( EPETE( main2() ) );
    }
    catch( kjb::Exception& e )
    {
        e.print_details_exit();
    }

    kjb_c::kjb_cleanup();
    return EXIT_SUCCESS;
}

