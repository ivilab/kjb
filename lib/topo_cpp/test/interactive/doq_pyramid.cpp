/**
 * @file
 * @author Andrew Predoehl
 * @brief show a bunch of DOQ images of various size, with common center
 */
/*
 * $Id: doq_pyramid.cpp 14929 2013-07-17 19:10:37Z predoehl $
 */
#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>
#include <topo_cpp/dorthoquad.h>

namespace {

void show(const kjb::Int_matrix& m)
{
    if (0 == kjb_c::kjb_fork())
    {
        std::ostringstream o;
        o << "Speedway-Campbell, size "
                        << m.get_num_cols() << 'x' << m.get_num_rows();
        kjb::Image i(m);
        i.at(m.get_num_rows()/2, m.get_num_cols()/2)=kjb::PixelRGBA(200,0,0);
        i.display(o.str());
        while (true) kjb_c::nap(1000);
    }
}


}

int main()
{
    using namespace kjb::TopoFusion;

    int sizes[] = {200, 400, 500, 800,
        /*1100, 1400, 1700, 2000,*/
        -1};

    Tile_manager t;

    /*
     * Intersection of Campbell Ave. and Speedway Blvd. in Tucson, Arizona.
     * The coordinates below land right in the middle, in the DOQ imagery.
     */
    const pt p = make_pt(505281, 3566405, 12);

    /*
     * I obtained the above point from the USGS Tucson 7.5" quadrangle,
     * with NW corner at W 111 deg, 0', 0" and N 32 deg, 15', 0".
     * By my reading of that map, the intersection is near UTM
     * easting 505333, northing 3566567, zone 12, referenced to NAD-83.
     *
     * But that is not where it is in MSR maps.  This is how I discovered the
     * displacement between MSR Maps and USGS data.
     */

    for (int i = 0; sizes[i] > 0; ++i)
    {
        const int s = sizes[i];
        show(get_aerial_image(make_pt(p.x - s/2, p.y + s/2, p.zone), s, s));

        if (i > 0)
        {
            const int r = sizes[i-1];
            show(get_aerial_image(make_pt(p.x-s/2, p.y+r/2, p.zone), s, r));
        }
    }
    return EXIT_SUCCESS;
}

