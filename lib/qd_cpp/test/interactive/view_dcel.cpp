/*
 * $Id: view_dcel.cpp 20129 2015-11-24 23:57:03Z predoehl $
 */

#include <l/l_sys_tsig.h>
#include <l_cpp/l_stdio_wrap.h>
#include <qd_cpp/svg_dcel.h>

#include <iostream>
#include <fstream>

int fail(const std::string& msg)
{
    std::cerr << "Error: " << msg << '\n';
    return EXIT_FAILURE;
}

int main(int argc, char** argv)
{
    if (argc < 2) return fail("need a filename argument (XML dcel input)");

    std::ifstream f(argv[1]);
    if (!f) return fail("cannot read from file");


    kjb::qd::Doubly_connected_edge_list d;
    try
    {
        kjb::qd::Doubly_connected_edge_list
            e = kjb::qd::Doubly_connected_edge_list::ctor_xml_stream(f);
        d.swap(e);
    }
    catch (const kjb::Exception& e)
    {
        e.print_details(std::cerr);
        return fail("file parsing error");
    }

    kjb::Temporary_Recursively_Removing_Directory dir;
    const std::string fnsvg(dir.get_pathname() + DIR_STR + "1.svg"),
                      fntif(dir.get_pathname() + DIR_STR + "1.tif");
    std::ofstream fosvg(fnsvg.c_str());
    fosvg << kjb::qd::draw_dcel_as_svg(d);
    fosvg.close();
    kjb_c::kjb_system(("convert " + fnsvg + " " + fntif).c_str());

    if (0 == kjb_c::kjb_fork())
    {
        kjb_c::kjb_system(("display " + fntif).c_str());
        while (true)
        {
            kjb_c::nap(1000);
        }
        /* NOTREACHED */
    }
    kjb_c::nap(1000);

    return EXIT_SUCCESS;
}
