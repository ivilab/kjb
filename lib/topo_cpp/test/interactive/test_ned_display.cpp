/**
 * @file
 * @brief try the display method of the class Ned13_one_degree_grid.
 * @author Andrew Predoehl
 *
 * The input file is over 100 megapixels, so bear in mind that on an average
 * computer, this will be slow, especially if your system starts to swap.
 *
 * The display method forks a child process to display the image.  The child
 * goes into an infinite loop, so the user must manually kill it, such as with
 * the command "killall test_ned_display" when you've seen enough.
 *
 * The display function does not respect the "missing values" sentinel value
 * provided by the USGS, and consequently, any tiles that ARE missing some
 * values (such as in northern Montana) look, due to scaling, extra whitish.
 */
/*
 * $Id: test_ned_display.cpp 17601 2014-09-25 22:40:22Z predoehl $
 */

#include <topo_cpp/nedgrid.h>

int main()
{
    std::vector< std::string > nopath;
    try
    {
        //kjb::Ned13_one_degree_grid n(49, -115); // northern Montana
        kjb::Ned13_one_degree_grid n(33, -111, nopath); // Tucson area
#if 0
        n.display("N 32-33 deg, W 110-111 deg (Tucson, AZ)", 400);
#endif
        return EXIT_SUCCESS;
    }
    catch(kjb::Exception &e)
    {
        e.print_details_exit();
    }
}

