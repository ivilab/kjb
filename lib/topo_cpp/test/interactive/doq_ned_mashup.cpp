/**
 * @file
 * @brief Interactive tests for the class DOrthoQuad
 * @author Andrew Predoehl
 */

/*
 * $Id: doq_ned_mashup.cpp 17601 2014-09-25 22:40:22Z predoehl $
 */

#include <l/l_sys_term.h>
#include <l_cpp/l_ew.h>
#include <l_cpp/l_stdio_wrap.h>
#include <l_cpp/l_heartbeat.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_hsv.h>

#include <topo_cpp/LatLong-UTMconversion.h>
#include <topo_cpp/nedgrid.h>
#include <topo_cpp/nedviz.h>
#include <topo_cpp/dorthoquad.h>


namespace {

/*
 * If zero, we just repeatedly sample elevation points from an elevation
 * "reader" object, which produces a single elevation estimate at a given
 * query point.  Different readers use different interpolation methods.
 * If this macro is positive, we instead use the ned13_grid() function, which
 * uses a Gaussian process to interpolate (aka kriging) and which exploits the
 * computational savings possible by interpolating in bulk.
 */
#define USE_GRID 1

// If 1, also show incline (local elevation gradient magitude). Grid required.
#define ALSO_INCLINE 1

#if ALSO_INCLINE && !USE_GRID
#error "To visualize the incline, you must use the grid.  Set USE_GRID to 1."
#endif

const bool VERBOSE = true;

// if true, draw white pixels where the training data lie
const bool DEM_STARS = true;


void ow_normalize_0_to_1(kjb::Matrix* m)
{
    if (00 == m) return;

    const int N = m -> get_length();
    if (0 == N) return;

    const double    e_max = kjb::max(*m),
                    e_min = kjb::min(*m);
    if (e_max == e_min) return;

    const double D = 1.0 / (e_max - e_min);

    for (int i = 0; i < N; ++i)
    {
        m -> at(i) = D * (m -> at(i) - e_min);
    }
}



void sprinkle_stars_in(
    kjb::Image* e,
    const kjb::TopoFusion::pt& nw
)
{
    using namespace kjb;

    const PixelRGBA WHT(250, 250, 250);

    kjb::TopoFusion::pt se(nw);
    se.y -= e -> get_num_rows();
    se.x += e -> get_num_cols();

    Ned13_one_degree_grid::IntegralLL   beg(utm_to_se_ned13_ill(nw)),
                                        end(utm_to_se_ned13_ill(se));
    KJB(ASSERT(beg.ilat > end.ilat));
    KJB(ASSERT(beg.ilon < end.ilon));
    end.ilat += 1;
    end.ilon -= 1;

    for (int lat = end.ilat; lat <= beg.ilat; ++lat)
    {
        for (int lon = beg.ilon; lon <= end.ilon; ++lon)
        {
            Ned13_one_degree_grid::IntegralLL i(beg);
            i.ilat = lat;
            i.ilon = lon;

            kjb::TopoFusion::pt j(ned13_ill_to_utm(i));
            if (j.zone != nw.zone)
            {
                j.x = kjb::TopoFusion::getNewEasting(j, nw.zone);
                j.zone = nw.zone;
            }

            const int col = j.x - nw.x, row = nw.y - j.y;

            // clipping
            if (    0 <= row && row < e -> get_num_rows()
                &&  0 <= col && col < e -> get_num_cols()
               )
            {
                e -> at(row, col) = WHT;
            }
        }
    }
}


void display(
    const kjb::Int_matrix &doq,
    const kjb::Matrix dem,
    const std::string& name,
    const kjb::TopoFusion::pt* nw
)
{
    const int EDGE = doq.get_num_rows();
    ETX(doq.get_num_cols() != EDGE);
    ETX(dem.get_num_cols() != EDGE);
    ETX(dem.get_num_rows() != EDGE);

    kjb::Image el_overlay(EDGE, EDGE);
    for (int rrr = 0; rrr < EDGE; ++rrr)
    {
        for (int ccc = 0; ccc < EDGE; ++ccc)
        {
            const float h = dem(rrr, ccc),              // hue set by DEM
                        v = doq.at(rrr, ccc) / 255.0;   // val set by DOQ
            el_overlay.at(rrr, ccc) = kjb::PixelHSVA(h, .7, v);
        }
    }

    if (DEM_STARS && nw) sprinkle_stars_in(&el_overlay, *nw);

    if (0 == kjb_c::kjb_fork())
    {
        el_overlay.display( name );
        while(true) kjb_c::nap(1000);
    }
}



void chatter(
    const kjb::TopoFusion::pt& p, 
    const kjb::Ned13_one_degree_grid::IntegralLL& ic
)
{
    const std::pair<double, double> dne = kjb::delta_n_e_meters(ic);
    kjb_c::pso(
            "Center has UTM easting %.1f, northing %.1f, and zone %d.\n"
            "Center has latitude %f, longitude %f.\n"
            "At center, NED grid has east-west granularity of %f meters.\n"
            "At center, NED grid has north-south granularity of %f meters.\n",
            p.x, p.y, int(p.zone),
            ic.dd_latitude(), ic.dd_longitude(),
            dne.second, dne.first
        );
}


void direction_disp(
    const kjb::Int_matrix& doq,
    const kjb::Matrix& gee,
    const kjb::Matrix& gen,
    const kjb::Matrix& gmag // already normalized 0 to .8
)
{
    const int EDGE = doq.get_num_rows();
    ETX(doq.get_num_cols() != EDGE);
    ETX(gee.get_num_cols() != EDGE);
    ETX(gee.get_num_rows() != EDGE);
    ETX(gen.get_num_cols() != EDGE);
    ETX(gen.get_num_rows() != EDGE);
    ETX(gmag.get_num_cols() != EDGE);
    ETX(gmag.get_num_rows() != EDGE);

    kjb::Image el_overlay(EDGE, EDGE);
    for (int rrr = 0; rrr < EDGE; ++rrr)
    {
        for (int ccc = 0; ccc < EDGE; ++ccc)
        {
            const float angle = atan2(gen.at(rrr, ccc), gee.at(rrr, ccc)),
                        h = 0.5 + angle / 2.0 / M_PI,
                        s = 0.15 + gmag.at(rrr, ccc),
                        v = doq.at(rrr, ccc) / 255.0;   // val set by DOQ
            el_overlay.at(rrr, ccc) = kjb::PixelHSVA(h, s, v);
        }
    }

    if (0 == kjb_c::kjb_fork())
    {
        el_overlay.display( "color-coding shows incline direction, grade" );
        while(true) kjb_c::nap(1000);
    }
}


void show_me(const std::string& name, const kjb::TopoFusion::pt& utm_dem)
{
    using namespace kjb::TopoFusion;

    const int EDGE = 2000;
    const kjb::Ned13_one_degree_grid::IntegralLL
                                        ic(kjb::utm_to_se_ned13_ill(utm_dem));
    chatter(utm_dem, ic);
    // Path to DEM data.  You can add your own path here if you like.
    std::vector< std::string > path(2);
    path[0] = "/home/predoehl/visionroot/data/trails/llgrid";
    path[1] = "/net/v04/data_3/trails/elevation/ned_13as/llgrid";
    //kjb::Ned13_nearest_se_neighbor_reader g(path);
    kjb::Ned13_bilinear_reader g(path);
    //kjb::Ned13_gp_reader g(path);

    // find corners of image, and load up elevation grid covering it
    pt nw(utm_dem);
    nw.x -= EDGE/2;
    nw.y += EDGE/2;

#if USE_GRID
    #if ALSO_INCLINE
    kjb::Matrix el, gee, gen;
    ETX(kjb::ned13_grid(utm_dem, &el, &gee, &gen, EDGE, EDGE, 1, &g, path));
    kjb::Matrix gm(kjb::ew_sqrt(kjb::ew_square(gee) + kjb::ew_square(gen)));
    ow_normalize_0_to_1(&gm);
    gm *= .8;
    #else
    kjb::Matrix el = kjb::ned13_grid(utm_dem, EDGE, EDGE, 1, &g, path);
    #endif

#else
    //kjb::set_spy_filename("/tmp/predoehl/spy_on_ned.txt");

    kjb::Matrix el(EDGE, EDGE);
    pt cursor(nw);
    kjb::Heartbeat heart("filling elevation value", EDGE, 3);
    for (int rrr = 0; rrr < EDGE; ++rrr, cursor.y -= 1)
    {
        heart.beat();
        for (int ccc = 0; ccc < EDGE; ++ccc, cursor.x += 1)
        {
            el(rrr, ccc) = g.elevation_meters(cursor);
        }
        cursor.x = nw.x;
    }
    heart.stop();
#endif

#if 0 /* SPECIAL DEBUG CRAP */
    std::ofstream spy("/tmp/predoehl/elevation-spy.txt");
    spy << el << '\n';
#endif

    ow_normalize_0_to_1(&el);
    el *= .8;

    // fill image using DOQ intensity, and elevation-based hue
    const kjb::Int_matrix mdoq(
            get_aerial_image(kjb::dem_to_doq_displacement(nw), EDGE, EDGE));
    display(mdoq, el, name, &nw);

#if ALSO_INCLINE
    display(mdoq, gm, "incline magnitude", 0);
    direction_disp(mdoq, gee, gen, gm);

    const kjb::Image y(
            kjb::ned_visualize_grid(el, gee, gen, (200+EDGE)/500.0, 10));
    if (0==kjb_c::kjb_fork()) {y.display("yet another"); while(1) sleep(10);}
#endif
}


} // anonymous namespace

int main()
{
    kjb_c::kjb_init();
    kjb_c::kjb_disable_paging();

    try 
    {
        using kjb::TopoFusion::make_pt;

        // simply instantiate one of these, that is all.
        kjb::TopoFusion::Tile_manager t;

        show_me(
#if 1
            // something is wrong here, I am trying to figure out what.
            "Something wrong GDMBR (hdtrails5, #104)",
            //make_pt(220181, 3640007, 13)
            make_pt(219381, 3639207, 13)
#elif 1
            // this is an "infamous" (to me) spot in Scott's dataset:
            "Hilly GDMBR (hdtrails5, #116)",
            make_pt(777278, 3674857, 12)
#elif 0
            // so is this:
            "Hilly GDMBR (hdtrails5, #117)",
            make_pt(777058, 3676798, 12)
#elif 1
            "Dine Tah (Navajo nation)",
            make_pt(497233, 3991310, 12)
#elif 1
            // at Grand Canyon
            "Bright Angel Trail",
            make_pt(397230, 3991310, 12)
#elif 1
            // at Grand Canyon
            "South Kaibab Trail (Cedar Ridge)",
            make_pt(401900, 3992000, 12)
#elif 1
            // at Grand Canyon
            "South Kaibab Trail (Skeleton Point)",
            make_pt(401900, 3993500, 12)
#elif 1
            // where this code was written
            "U of Arizona campus",
            make_pt(504800, 3566200, 12)
#elif 1
            // west of Tucson, AZ; not as interesting as I expected.
            "Gate's Pass",
            make_pt(490549, 3564920, 12)
#elif 0
            // backpacking trail in Zion National Park
            "Walter's Wiggles",
            make_pt(326974, 4127260, 12)
#elif 1
            // between Tucson and Casa Grande, site of a Civil War battle
            "Picacho Peak",
            make_pt(462000, 3611000, 12)
#else
            // near Colorado Springs, CO (notice the cog railway)
            "Pike's Peak",
            make_pt(496700, 4298250, 13)
#endif
        );
    }
    catch( kjb::Exception& e )
    {
        e.print_details_exit();
    }

    kjb_c::kjb_cleanup();
    return EXIT_SUCCESS;
}
