/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test for Bentley-Ottman algorithm
 */
/*
 * $Id: test_intersection.cpp 20160 2015-12-08 23:36:20Z predoehl $
 */

#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/ratpoint.h>
#include <qd_cpp/intersection.h>
#include <gsl_cpp/gsl_qrng.h>

#define VISUALIZATION 0

#if VISUALIZATION
#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>
#endif


#include <set>
#include <iostream>
#include <iterator>
#include <sstream>

namespace
{
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::i_numerator;
using kjb::qd::i_denominator;
using kjb::qd::dbl_ratio;

const bool VERBOSE = 0;


RatPoint quasirandom_pt(kjb::Gsl_Qrng_Sobol& qrng)
{
    kjb::Vector v( 2 );
    v = qrng.read();
    const long GRANULARITY = 100;
    RatPoint a(RatPoint::Rat(long(v[0] * GRANULARITY), GRANULARITY),
               RatPoint::Rat(long(v[1] * GRANULARITY), GRANULARITY));
    return a;
}

std::ostream& operator<<(std::ostream& os, const RatPoint_line_segment& s)
{
    os << '(' << s.a.x << ',' << s.a.y
       << ") to (" << s.b.x << ',' << s.b.y << ")";
    return os;
}


#if VISUALIZATION
const int IMG_MAG=900;

void draw_thin_white_line(kjb::Image* i, const RatPoint_line_segment& s,int j)
{
    const kjb::qd::PixPoint
        p1(s.a.x.numerator()*IMG_MAG/double(s.a.x.denominator()),
            s.a.y.numerator()*IMG_MAG/double(s.a.y.denominator())),
        p2(s.b.x.numerator()*IMG_MAG/double(s.b.x.denominator()),
            s.b.y.numerator()*IMG_MAG/double(s.b.y.denominator())),
        pmin = std::min(p1, p2);

    i-> draw_line_segment(p1.y,p1.x, p2.y,p2.x, 1, kjb::PixelRGBA(250,250,50));

    std::ostringstream label;
    label << j;
    // next line blows up if the font files are missing
    KJB(EPETE(i -> draw_text_top_left(pmin.y, pmin.x, label.str())));
}


void draw_setup(
    const std::vector< RatPoint_line_segment >& sl,
    const std::string& name
)
{
    kjb::Image iii(IMG_MAG, IMG_MAG, 0,0,0);

    for (size_t i = 0; i < sl.size(); ++i)
    {
        draw_thin_white_line(&iii, sl[i], i);
    }

    iii.write(std::string(__FILE__) + "." + name + ".tif");
}
#endif


int test_this_set(const std::vector< RatPoint_line_segment >& sl)
{
    typedef std::set< std::pair<size_t, size_t> > STSET;
    STSET r1;

    // Brute-force check for edge intersections.
    for (size_t i = 0; i < sl.size()-1; ++i)
    {
        for (size_t j = 1+i; j < sl.size(); ++j)
        {
            if (is_intersecting(sl[i], sl[j]))
            {
                r1.insert(std::make_pair(i, j));
            }
        }
    }

    const std::vector< std::pair< size_t, size_t > >
        r2 = kjb::qd::get_intersections(sl),
        r4 = kjb::qd::get_interior_intersections(sl);
    const STSET r3(r2.begin(), r2.end()), r5(r4.begin(), r4.end());

    for (STSET::const_iterator i = r5.begin(); i != r5.end(); ++i)
    {
        if (r3.end() == r3.find(*i))
        {
            kjb_c::set_error("r5 is not a subset of r3.");
            return kjb_c::ERROR;
        }
    }

    for (STSET::const_iterator i = r1.begin(); i != r1.end(); ++i)
    {
        if (r3.end() == r3.find(*i))
        {
            RatPoint_line_segment overlap(sl[0]);
            bool hit = kjb::qd::segment_intersection(sl[i->first],
                                                     sl[i->second], &overlap);
            std::cerr << "Unique to r1: " << i->first << ',' << i->second
                      << "\n\t" << sl[i->first] << "\n\t" << sl[i->second]
                      << "\n\tintersecting? " << hit;
            if (hit)
            {
                std::cerr << " overlap: ";
                if (kjb::qd::is_degenerate(overlap))
                {
                    std::cerr << overlap.a;
                }
                else
                {
                    std::cerr << overlap;
                }
            }
            std::cerr << '\n';
            kjb_c::set_error("r1 contains spurious result: bug in unit test");
            return kjb_c::ERROR;
        }
    }
    for (STSET::const_iterator i = r3.begin(); i != r3.end(); ++i)
    {
        if (r1.end() == r1.find(*i))
        {
            std::cerr <<"Unique to r3: "<< i->first <<',' << i->second
                << "\n\t" << sl[i->first] << "\n\t" << sl[i->second]
                << "\nintersecting? "
                << is_intersecting(sl[i->first], sl[i->second])
                << "\tparallel? " << are_parallel(sl[i->first], sl[i->second])
                << '\n';
        }
        if (r5.end() == r5.find(*i))
        {
            // this is ok if either segment's interior is touched.
            RatPoint_line_segment overlap(sl[0]);
            bool hit = kjb::qd::segment_intersection(sl[i->first],
                                                     sl[i->second], &overlap);
            TEST_TRUE(hit); // otherwise *i is a spurious result

            if (! kjb::qd::is_degenerate(overlap))
            {
                kjb_c::set_error("parallel overlap missing from r5");
                return kjb_c::ERROR;
            }
            // hitting interior of first would mean a failure
            if (overlap.a != sl[i->first].a && overlap.a != sl[i->first].b)
            {
                kjb_c::set_error("first interior intersection %u, %u "
                                "missing from r5", i -> first, i -> second);
                return kjb_c::ERROR;
            }
            // hitting interior of second would mean a failure
            if (overlap.a != sl[i->second].a && overlap.a != sl[i->second].b)
            {
                kjb_c::set_error("second interior intersection %u, %u "
                                "missing from r5", i -> first, i -> second);
                return kjb_c::ERROR;
            }
        }
    }

    TEST_TRUE(r1 == r3);

    return kjb_c::NO_ERROR;
}



std::vector< RatPoint_line_segment > setup13(
    const std::vector<bool>& take_me
)
{
    std::vector< RatPoint_line_segment > sl1, sl2;
    kjb::Gsl_Qrng_Sobol qq( 2 );

    for (size_t i = 0; i < take_me.size(); ++i)
    {
        const RatPoint_line_segment s(quasirandom_pt(qq), quasirandom_pt(qq));
        sl1.push_back(s);
        RatPoint_line_segment t(s.a, s.a), u(s);
        u.a.y = s.b.y;
        sl1.push_back(t);
        sl1.push_back(u);
    }

    if (VERBOSE) std::cout << "Test set segment catalog\n";
    for (size_t i = 0; i < sl1.size() && i < take_me.size(); ++i)
    {
        if (take_me[i]) sl2.push_back(sl1[i]);
        if (VERBOSE && take_me[i])
        {
            std::cout <<"segment "<< i <<": "<< sl2.back() <<'\n';
        }
    }

    return sl2;
}


int test13(const std::vector<bool> &take_me)
{
    const std::vector< RatPoint_line_segment > sl(setup13(take_me));

#if VISUALIZATION
    draw_setup(sl, __func__);
#endif

    return test_this_set(sl);
}



// Trying to simulate the flaw of test13 without all the chaos.
int test14()
{
    using kjb::qd::PixPoint;
    std::vector< RatPoint_line_segment > sl;
    sl.push_back(RatPoint_line_segment(PixPoint(0,0), PixPoint(4,4)));
    sl.push_back(RatPoint_line_segment(PixPoint(1,1), PixPoint(5,5)));
    sl.push_back(RatPoint_line_segment(PixPoint(2,2), PixPoint(6,6)));
    sl.push_back(RatPoint_line_segment(PixPoint(2,4), PixPoint(4,2)));
    sl.push_back(RatPoint_line_segment(PixPoint(1,5), PixPoint(5,1)));

    const std::vector< std::pair< size_t, size_t > >
        r = kjb::qd::get_intersections(sl),
        t = kjb::qd::get_interior_intersections(sl);
    TEST_TRUE(10 == r.size());
    TEST_TRUE(0 == r[0].first && 1 == r[0].second);
    TEST_TRUE(0 == r[1].first && 2 == r[1].second);
    TEST_TRUE(0 == r[2].first && 3 == r[2].second);
    TEST_TRUE(0 == r[3].first && 4 == r[3].second);
    TEST_TRUE(1 == r[4].first && 2 == r[4].second);
    TEST_TRUE(1 == r[5].first && 3 == r[5].second);
    TEST_TRUE(1 == r[6].first && 4 == r[6].second);
    TEST_TRUE(2 == r[7].first && 3 == r[7].second);
    TEST_TRUE(2 == r[8].first && 4 == r[8].second);
    TEST_TRUE(3 == r[9].first && 4 == r[9].second);
    TEST_TRUE(r == t);

    return test_this_set(sl);
}


// easy peasy variation of test14
int test15()
{
    using kjb::qd::PixPoint;
    std::vector< RatPoint_line_segment > sl;
    sl.push_back(RatPoint_line_segment(PixPoint(0,0), PixPoint(4,4)));
    sl.push_back(RatPoint_line_segment(PixPoint(0,1), PixPoint(4,5)));
    sl.push_back(RatPoint_line_segment(PixPoint(1,0), PixPoint(5,4)));
    sl.push_back(RatPoint_line_segment(PixPoint(2,4), PixPoint(4,2)));
    sl.push_back(RatPoint_line_segment(PixPoint(2,5), PixPoint(6,1)));

    const std::vector< std::pair< size_t, size_t > >
        r = kjb::qd::get_intersections(sl);
    TEST_TRUE(6 == r.size());
    TEST_TRUE(0 == r[0].first && 3 == r[0].second);
    TEST_TRUE(0 == r[1].first && 4 == r[1].second);
    TEST_TRUE(1 == r[2].first && 3 == r[2].second);
    TEST_TRUE(1 == r[3].first && 4 == r[3].second);
    TEST_TRUE(2 == r[4].first && 3 == r[4].second);
    TEST_TRUE(2 == r[5].first && 4 == r[5].second);

    return test_this_set(sl);
}


// rotate test13 by 90 degrees
int test16(int alot)
{
    std::vector< RatPoint_line_segment >
        sl1 = setup13(std::vector<bool>(alot, true)), sl2;

    for (size_t i = 0; i < sl1.size(); ++i)
    {
        sl2.push_back(
            RatPoint_line_segment(
                RatPoint(sl1[i].a.y, sl1[i].a.x),
                RatPoint(sl1[i].b.y, sl1[i].b.x)));
    }

    return test_this_set(sl2);
}


int test17(int alot)
{
    std::vector< RatPoint_line_segment >
        sl1 = setup13(std::vector<bool>(alot, true));
    std::vector< RatPoint > scramble;

    for ( ; ! sl1.empty(); sl1.pop_back())
    {
        scramble.push_back(sl1.back().a);
        scramble.push_back(sl1.back().b);
    }
    std::random_shuffle(scramble.begin(), scramble.end());

    while (scramble.size() > 1)
    {
        RatPoint a(scramble.back());
        scramble.pop_back();
        RatPoint b(scramble.back());
        scramble.pop_back();
        sl1.push_back(RatPoint_line_segment(a,b));
    }

    return test_this_set(sl1);
}


}

int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));

    // update the number of test iterations based on the optional argument
    int test_factor = 1, ALOT = 10;
    KJB(EPETE(scan_time_factor(argv[1], &test_factor)));
    while (test_factor--> 0)
    {
        ALOT *= 10;
    }

    /* Tests for get_intersection() -- you have to be harsh with this one. */
    KJB(EPETE(test13(std::vector<bool>(ALOT, true))));
    KJB(EPETE(test14()));
    KJB(EPETE(test15()));
    KJB(EPETE(test16(ALOT)));
    KJB(EPETE(test17(ALOT)));

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}
