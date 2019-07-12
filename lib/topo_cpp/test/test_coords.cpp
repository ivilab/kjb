/**
 * @file
 * @brief test coordinate conversions
 * @author Andrew Predoehl
 */
/*
 * $Id: test_coords.cpp 22174 2018-07-01 21:49:18Z kobus $
 */

#include <l/l_sys_debug.h>
#include <l/l_sys_io.h>
#include <l/l_sys_time.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l/l_init.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_test.h>
#include <i_cpp/i_image.h>
#include <topo_cpp/LatLong-UTMconversion.h>

#include <string>
#include <algorithm>
#include <functional>
#include <iostream>

#define HAS_SPY 0

namespace {

const int WGS_84 = kjb::TopoFusion::ELLIPSOID_WGS_84;

struct City {
    const char* name;
    double lat, lon;
    double easting, northing;
    int zone;

    bool missing() const { return -1 == lat || -1 == lon; }
    bool end_of_list() const { return 0 == name; }
    bool no_utm() const { return easting < 0 || northing < 0 || zone < 0; }
}
us_capitals[] = {
    { "Montgomery, AL",     32.361538, -86.279118,  567823.4, 3580739.0, 16 },
    { "Juneau, AK",         58.351422, -134.511579, 528591.3, 6467939.9,  8 },
    { "Phoenix, AZ",        33.528370, -112.076300, 400057.8, 3710383.1, 12 },
    { "Little Rock, AR",    34.736009, -92.331122,  -1, -1, -1  },
    { "Sacramento, CA",     38.555605, -121.468926, -1, -1, -1  },
    { "Denver, CO",         39.726287, -104.965486, 502957.8, 4397379.0, 13 },
    { "Hartford, CT",       41.762550, -72.688587,  -1, -1, -1  },
    { "Dover, DE",          39.161921, -75.526755,  -1, -1, -1  },
    { "Tallahassee, FL",    30.451800, -84.272770,  761883.8, 3372010.5, 16 },
    { "Atlanta, GA",        33.759506, -84.403176,  -1, -1, -1  },
    { "Honolulu, HI",       21.308950, -157.826182, 621755.5, 2356793.9,  4 },
    { "Boise, ID",          43.613739, -116.237651, -1, -1, -1  },
    { "Springfield, IL",    39.783250, -89.650373,  -1, -1, -1  },
    { "Indianapolis, IN",   -1, -1,                 -1, -1, -1  },
    { "Des Moines, IA",     -1, -1,                 -1, -1, -1  },
    { "Topeka, KS",         -1, -1,                 -1, -1, -1  },
    { "Frankfort, KY",      -1, -1,                 -1, -1, -1  },
    { "Baton Rouge, LA",    -1, -1,                 -1, -1, -1  },
    { "Augusta, ME",        44.323535, -69.765261,  438980.2, 4908092.9, 19 },
    { "Annapolis, MD",      -1, -1,                 -1, -1, -1  },
    { "Boston, MA",         -1, -1,                 -1, -1, -1  },
    { "Lansing, MI",        -1, -1,                 -1, -1, -1  },
    { "Saint Paul, MN",     -1, -1,                 -1, -1, -1  },
    { "Jackson, MS",        -1, -1,                 -1, -1, -1  },
    { "Jefferson City, MO", -1, -1,                 -1, -1, -1  },
    { "Helena, MT",         46.595805, -112.027031, -1, -1, -1  },
    { "Lincoln, NE",        40.809868, -96.675345,  -1, -1, -1  },
    { "Carson City, NV",    39.160949, -119.753877, -1, -1, -1  },
    { "Concord, NH",        -1, -1,                 -1, -1, -1  },
    { "Trenton, NJ",        -1, -1,                 -1, -1, -1  },
    { "Santa Fe, NM",       35.535926, -105.924596, -1, -1, -1  },
    { "Albany, NY",         -1, -1,                 -1, -1, -1  },
    { "Raleigh, NC",        -1, -1,                 -1, -1, -1  },
    { "Bismarck, ND",       46.813343, -100.779004, -1, -1, -1  },
    { "Columbus, OH",       -1, -1,                 -1, -1, -1  },
    { "Oklahoma City, OK",  -1, -1,                 -1, -1, -1  },
    { "Salem, OR",          -1, -1,                 -1, -1, -1  },
    { "Harrisburg, PA",     -1, -1,                 -1, -1, -1  },
    { "Providence, RI",     -1, -1,                 -1, -1, -1  },
    { "Columbia, SC",       -1, -1,                 -1, -1, -1  },
    { "Pierre, SD",         -1, -1,                 -1, -1, -1  },
    { "Nashville, TN",      -1, -1,                 -1, -1, -1  },
    { "Austin, TX",         30.300474, -97.747247,  -1, -1, -1  },
    { "Salt Lake City, UT", 40.754700, -111.892622, -1, -1, -1  },
    { "Montpelier, VT",     -1, -1,                 -1, -1, -1  },
    { "Richmond, VA",       -1, -1,                 -1, -1, -1  },
    { "Olympia, WA",        47.042418, -122.893077, 508122.4, 5209883.4, 10 },
    { "Charleston, WV",     -1, -1,                 -1, -1, -1  },
    { "Madison, WI",        -1, -1,                 -1, -1, -1  },
    { "Cheyenne, WY",       41.145548, -104.802042, -1, -1, -1  },
    { 0, 0, 0, 0, 0, 0 } /* SENTINEL END OF LIST */
};

#define SIMILAR(a, b)       similar((a), (b), 10e-9, __LINE__)
#define SIMILAR_2(a, b, t)  similar((a), (b), (t), __LINE__)

// count the length of the above list
size_t count_capitals()
{
    size_t ct = 0;
    for (const City* p = us_capitals; ! p->end_of_list(); ++p )
    {
        ++ct;
    }
    return ct;
}


// won't work well if a and b should both equal zero.
int similar(double a, double b, double TOL, unsigned line)
{
    const double d = fabs(a-b);
    const double s = fabs(a)+fabs(b);
    if (s > 0 && kjb_c::is_interactive()) kjb_c::pso("%f\t%f\t%e\t", a,b,d/s);
    bool is_similar = d < s * TOL;

    if (!is_similar)
    {
        kjb_c::add_error("Nonsimilar output from line %u:\n" 
                            "\ta = %e, b = %e, d = %e, s = %e\n"
                            "\ttolerance = %e\n",
                            line, a, b, d, s, TOL);
    }
    return is_similar ? kjb_c::NO_ERROR : kjb_c::ERROR;
}

#if HAS_SPY
int sim_spy()
{
    using kjb::TopoFusion::spy;
    KJB(ERE(SIMILAR(spy[0].eccPrimeSquared, spy[1].eccPrimeSquared)));
    KJB(ERE(SIMILAR(spy[0].e1,      spy[1].e1)));
    KJB(ERE(SIMILAR(spy[0].mu,      spy[1].mu)));
    KJB(ERE(SIMILAR(spy[0].phi1Rad, spy[1].phi1Rad)));
    KJB(ERE(SIMILAR(spy[0].N1,      spy[1].N1)));
    KJB(ERE(SIMILAR(spy[0].T1,      spy[1].T1)));
    KJB(ERE(SIMILAR(spy[0].C1,      spy[1].C1)));
    KJB(ERE(SIMILAR(spy[0].R1,      spy[1].R1)));
    KJB(ERE(SIMILAR(spy[0].D,       spy[1].D)));
    KJB(ERE(SIMILAR(spy[0].Lat,     spy[1].Lat)));
    KJB(ERE(SIMILAR(spy[0].Long,    spy[1].Long)));;

    return kjb_c::NO_ERROR;
}
#endif


// verify that LL to UTM and UTM to LL are inverses, plus numerical noise.
// "spy" code tests that my alternative version of UTM to LL is equivalent to
// scott's original code.  Scott's original code is no longer compiled now.
int test1()
{
    for (const City* p = us_capitals; ! p->end_of_list(); ++p )
    {
        if (p->missing()) continue;
        kjb::TopoFusion::pt q;
        kjb::TopoFusion::LLtoUTM(WGS_84, p->lat, p->lon, q);
#if HAS_SPY
        double la, lo;
        kjb::TopoFusion::UTMtoLL(WGS_84, q, la, lo);
        KJB(ERE(SIMILAR(la, p->lat)));
        KJB(ERE(SIMILAR(lo, p->lon)));
#endif
        double la2, lo2;
        kjb::TopoFusion::utm_to_lat_long(WGS_84, q, la2, lo2);
        if (kjb_c::is_interactive()) kjb_c::pso("%s\t", p->name);
        KJB(ERE(SIMILAR(la2, p->lat)));
        if (kjb_c::is_interactive()) kjb_c::pso("\n%s\t", p->name);
        KJB(ERE(SIMILAR(lo2, p->lon)));
        if (kjb_c::is_interactive()) kjb_c::pso("\n", p->name);

#if HAS_SPY
        for (int i=0; i<2; ++i)
        {
            using kjb::TopoFusion::spy;
            kjb_c::pso("spy: %e %e %e %e %e %e %e %e %e %e %e\n",
                spy[i].eccPrimeSquared,
                spy[i].e1,
                spy[i].mu,
                spy[i].phi1Rad,
                spy[i].N1,
                spy[i].T1,
                spy[i].C1,
                spy[i].R1,
                spy[i].D,
                spy[i].Lat,
                spy[i].Long);
        }
        KJB(ERE(sim_spy()));
#endif
    }
    return kjb_c::NO_ERROR;
}

std::vector<kjb::TopoFusion::pt> get_cities()
{
    std::vector<kjb::TopoFusion::pt> pts;

    for (const City* p = us_capitals; ! p->end_of_list(); ++p )
    {
        if (p->missing()) continue;
        kjb::TopoFusion::pt q;
        kjb::TopoFusion::LLtoUTM(WGS_84, p->lat, p->lon, q);
        pts.push_back(q);
    }
    return pts;
}

// compare singleton and vector versions of the UTM to LL conversion
// empirically they appear to produce identical results, exact float equality.
int test2()
{
    std::vector<kjb::TopoFusion::pt> pts = get_cities();
    std::vector<double> la, lo;
    kjb::TopoFusion::utm_to_lat_long(WGS_84, pts, &la, &lo);
    for (size_t iii = 0; iii < pts.size(); ++iii)
    {
        double nla, wlo;
        kjb::TopoFusion::utm_to_lat_long(WGS_84, pts[iii], nla, wlo);
        if (kjb_c::is_interactive()) kjb_c::pso("%d\t", iii);
        KJB(ERE(SIMILAR(nla, la[iii])));
        if (kjb_c::is_interactive()) kjb_c::pso("\n%d\t", iii);
        KJB(ERE(SIMILAR(wlo, lo[iii])));
        if (kjb_c::is_interactive()) kjb_c::pso("\n");
    }

    return kjb_c::NO_ERROR;
}

// speed test:  is the vector version significantly faster?  yes!
int test3(int tf)
{
    long trial1, trial2;

    if (tf <= 0 || ! kjb_c::is_interactive()) return kjb_c::NO_ERROR;
    const size_t TEST_SIZE = 1024u*1024u << tf-1;

    std::vector<kjb::TopoFusion::pt> pts = get_cities(), big;

    while(big.size() < TEST_SIZE)
    {
        big.resize(big.size() + pts.size());
        std::copy(pts.begin(), pts.end(), &big[big.size()-pts.size()]);
    }

    std::vector<double> la(big.size()), lo(big.size());

    // trial 1
    kjb_c::init_cpu_time();
    kjb::TopoFusion::utm_to_lat_long(WGS_84, pts, &la, &lo);
    trial1 = kjb_c::get_cpu_time();

    // trial 2
    kjb_c::init_cpu_time();
    for (size_t iii = 0; iii < big.size(); ++iii)
    {
        kjb::TopoFusion::utm_to_lat_long(WGS_84, big[iii], la[iii], lo[iii]);
    }
    trial2 = kjb_c::get_cpu_time();

    kjb_c::pso("   Vector version time: %f seconds\n"
               "Singleton version time: %f seconds\n",
               trial1/1000.0, trial2/1000.0);

    return kjb_c::NO_ERROR;
}


inline int fre(bool p)
{
    return p ? kjb_c::NO_ERROR : kjb_c::ERROR;
}


// tried not to write the following, and use bind2nd instead.  could not.
struct find_city : public std::unary_function< const City&, bool >
{
    std::string key;
    find_city(const std::string& city) : key(city) {}
    bool operator()(const City& c) const
    {
        return key == c.name;
    }
};


// test zone computation
int test4()
{
    const size_t CCT = count_capitals();

    // find a city in zone 10
    const City *c1 = std::find_if(us_capitals, us_capitals + CCT,
                                                    find_city("Olympia, WA"));
    if (c1 >= us_capitals + CCT) return kjb_c::ERROR;

    kjb::TopoFusion::pt olympia;
    kjb::TopoFusion::LLtoUTM(WGS_84, c1 -> lat, c1 -> lon, olympia);
    KJB(ERE(fre(10 == olympia.zone)));  // everyone knows Olypmpia is zone 10.
    KJB(ERE(fre(10 == zone_of(WGS_84, olympia))));

    // go 3 degrees east of Olympia, get UTM coordinates (b/c in the next zone)
    kjb::TopoFusion::pt not_oly;
    kjb::TopoFusion::LLtoUTM(WGS_84, c1 -> lat, c1 -> lon + 3, not_oly);
    KJB(ERE(fre(11 == not_oly.zone)));  // we have crossed a zone boundary!
    KJB(ERE(fre(11 == zone_of(WGS_84, not_oly))));
    not_oly.x = kjb::TopoFusion::getNewEasting(not_oly, 10);
    not_oly.zone = 10;                  // estimate this point from zone 10
    KJB(ERE(fre(11 == zone_of(WGS_84, not_oly))));

    // find a city in zone 11
    const City *c2 = std::find_if(us_capitals, us_capitals + CCT,
                                                find_city("Carson City, NV"));
    if (c2 >= us_capitals + CCT) return kjb_c::ERROR;

    kjb::TopoFusion::pt carsonc;
    kjb::TopoFusion::LLtoUTM(WGS_84, c2 -> lat, c2 -> lon, carsonc);
    KJB(ERE(fre(11 == carsonc.zone)));  // Carson City is in zone 11.
    KJB(ERE(fre(11 == zone_of(WGS_84, carsonc))));

    // go 0.5 degrees west of Carson City (into Cali), get UTM coordinates
    kjb::TopoFusion::pt not_cc;
    kjb::TopoFusion::LLtoUTM(WGS_84, c1 -> lat, c1 -> lon - 0.5, not_cc);
    KJB(ERE(fre(10 == not_cc.zone)));   // we have crossed a zone boundary!
    KJB(ERE(fre(10 == zone_of(WGS_84, not_cc))));
    not_cc.x = kjb::TopoFusion::getNewEasting(not_cc, 11);
    not_cc.zone = 11;                   // estimate this point from zone 11
    KJB(ERE(fre(10 == zone_of(WGS_84, not_cc))));

    return kjb_c::NO_ERROR;
}


int test5()
{
    if (! kjb_c::is_interactive()) return kjb_c::NO_ERROR;

    if (kjb_c::kjb_fork()) return kjb_c::NO_ERROR;

    double nmost = -90, smost = 90, emost = -180, wmost = 180;
    for (const City* p = us_capitals; ! p -> end_of_list(); ++p )
    {
        if (p->missing()) continue;
        nmost = std::max(nmost, p -> lat);
        smost = std::min(smost, p -> lat);
        emost = std::max(emost, p -> lon);
        wmost = std::min(wmost, p -> lon);
    }

    const int NSSIZE = 500, EWSIZE = 800;
    kjb::Image g(NSSIZE, EWSIZE, 0, 0, 0);
    const double    delta_ns = nmost - smost, delta_ew = emost - wmost,
                    MAG = .9 * std::min(NSSIZE/delta_ns, EWSIZE/delta_ew);

    for (const City* p = us_capitals; ! p->end_of_list(); ++p )
    {
        if (p->missing()) continue;

        int row = NSSIZE*.05 + MAG * (nmost - p -> lat),
            col = EWSIZE*.05 + MAG * (p -> lon - wmost);
        ETX(g.draw_text_top_left(row, col, p -> name));
    }

    g.display("us capitals");
    while (true) kjb_c::nap(1000);
    /* NOTREACHED */
    return kjb_c::NO_ERROR;
}



// compare Lat/Long-to-UTM conversion results to known-good UTM points
int test6()
{
    for (const City* p = us_capitals; ! p->end_of_list(); ++p )
    {
        // thirty centimeters easting and northing error (default setting)
        double  TOL = 0.30, NTOL = TOL, ETOL = TOL;
        if (0 == strcmp(p -> name, "Juneau, AK")) ETOL = 5.0; // bad in Alaska
        if (0 == strcmp(p -> name, "Honolulu, HI")) ETOL = 9.0; // worse in HI

        if (p->missing()) continue;
        if (p->no_utm()) continue;
        kjb::TopoFusion::pt q;
        kjb::TopoFusion::LLtoUTM(WGS_84, p->lat, p->lon, q);
        if (kjb_c::is_interactive())
        {
            KJB(TEST_PSE((  "%s\nlat %9.6f lon %11.6f "
                            "  east %8.1f north %9.1f zone %2d"
                            "\n                       "
                            "expected east %8.1f north %9.1f zone %2d"
                            "\n                       "
                            "error         %9.2f      %10.2f     "
                            "%2d\n",
                            p -> name, p -> lat, p -> lon,
                            p -> easting, p -> northing, int(p -> zone),
                            q.x, q.y, q.zone,
                            p -> easting - q.x, p -> northing - q.y,
                            int(p -> zone - q.zone)
                )));
        }
        KJB(ERE(fabs(q.x - p->easting) >= ETOL ? ERROR : NO_ERROR));
        KJB(ERE(fabs(q.y - p->northing) >= NTOL ? ERROR : NO_ERROR));
        // Actual error seems to be more like +/- 5 cm or less.
        KJB(ERE(p-> zone != q.zone ? ERROR : NO_ERROR));
    }
    return kjb_c::NO_ERROR;
}



}

int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));

    int time_factor = 1;
    KJB(EPETE(scan_time_factor(argv[1], &time_factor)));

    if (kjb_c::is_interactive())
    {
        std::cout << "Interactive run with time factor " << time_factor <<'\n';
    }

    KJB(EPETE(kjb::TopoFusion::validate_ellipsoid_table()));
    KJB(EPETE(test1()));
    KJB(EPETE(test2()));
    KJB(EPETE(test3(time_factor)));
    KJB(EPETE(test4()));
    KJB(EPETE(test5()));
    KJB(EPETE(test6()));

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}
