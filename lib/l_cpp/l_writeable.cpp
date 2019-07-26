/* $Id: l_writeable.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Kyle Simek, Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>

#include <l_cpp/l_exception.h>
#include <l_cpp/l_writeable.h>


using namespace kjb;


/**
 * @param  fname  Output file to write to.
 *
 * @throw  kjb::IO_error  Could not write to @em fname.
 */
void Writeable::write(const char* fname) const 
{
    std::ofstream out;
    std::ostringstream ost;

    out.open(fname);
    if (out.fail())
    {
        ost << fname << ": Could not open file";
        KJB_THROW_2(IO_error,ost.str());
    }

    try
    {
        write(out);
    }
    catch (IO_error e)
    {
        ost << fname << ": " << e.get_msg();
        KJB_THROW_2(IO_error,ost.str());
    }

    out.close();
    if (out.fail())
    {
        ost << fname << ": Could not close file";
        KJB_THROW_2(IO_error,ost.str());
    }
}
