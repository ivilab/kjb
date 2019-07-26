/**
 * @file
 * @author Kyle Simek
 * @author Andrew Predoehl
 */

/* $Id$ */

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
 * =========================================================================== */

#include <l/l_incl.h>
#include <l_cpp/l_exception.h>

#include <iostream>

int main ()
{
    kjb_c::kjb_init();

    const char* str1 = "truth";
    const char* str2 = "fact";

    kjb_c::set_error( "Apples are full of quercetin." );
    kjb_c::add_error( "Bakers make delicious bread." );
    kjb_c::add_error( "Charlie digs extreme flight gear, how I judge." );

    try
    {
        ETX( kjb_c::ERROR );
    }
    catch ( kjb::KJB_error& e )
    {
        e.print( std::cout );
    }
        
    try
    {
        KJB_THROW_3(kjb::Illegal_argument,
            "In the land of %s, the man with one %s is king", (str1)(str2));
    }
    catch (kjb::Illegal_argument& ex)
    {
        ex.print( std::cout );
    }

    kjb_c::kjb_cleanup();
    return EXIT_SUCCESS;
}
