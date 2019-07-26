/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test for intersection and other line segment utilities
 *
 * This tests the low-level predicates and other utilities that check if
 * two segments intersect, or if a segment and a line intersect, or if
 * a sequence of three points turns clockwise or counterclockwise.
 */
/*
 * $Id: test_ratpoint.cpp 20160 2015-12-08 23:36:20Z predoehl $
 */

#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/ratpoint.h>

#include <iostream>

namespace
{
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::i_numerator;
using kjb::qd::i_denominator;
using kjb::qd::dbl_ratio;


int test_intersection(const int p[])
{
    const kjb::qd::PixPoint s1a(p[0], p[1]),
                            s1b(p[2], p[3]),
                            s2a(p[4], p[5]),
                            s2b(p[6], p[7]);
    const kjb::qd::PixPoint_line_segment s1(s1a, s1b), s2(s2a, s2b);
    if (p[8])
    {
        TEST_TRUE(kjb::qd::are_parallel(s1, s2));
    }
    else
    {
        TEST_FALSE(kjb::qd::are_parallel(s1, s2));
    }
    if (p[9])
    {
        TEST_TRUE(kjb::qd::is_intersecting(s1, s2));
        if ( ! p[8] )
        {
            // not parallel
            const RatPoint isx( kjb::qd::line_intersection(s1, s2));
            const kjb::Vector2 fxy( dbl_ratio(isx.x), dbl_ratio(isx.y) );


            if (kjb_c::is_interactive())
            {
                std::cout << "intersection point expected to be at\n"
                "\tx=" << p[10] << '/' << p[11]
                <<  "\t       \t"
                "\ty=" << p[12] << '/' << p[13] << "\n"
                "\tx=" << (double(p[10])/p[11]) << " approx"
                "\ty=" << (double(p[12])/p[13]) << " approx"
                "\nbut empirically\n"
                "\tx,y = "<< isx
                << "\n"
                "\tx=" << fxy.x() << " approx"
                "\ty="<< fxy.y() << " approx"
                "\n";
            }

            TEST_APPROX_EQUALITY( fxy.x() * p[11], p[10] );
            TEST_APPROX_EQUALITY( fxy.y() * p[13], p[12] );
            TEST_TRUE(p[10] == i_numerator(isx.x));
            TEST_TRUE(p[11] == i_denominator(isx.x));
            TEST_TRUE(p[12] == i_numerator(isx.y));
            TEST_TRUE(p[13] == i_denominator(isx.y));
        }
    }
    else
    {
        TEST_FALSE(kjb::qd::is_intersecting(s1, s2));
    }
    return kjb_c::NO_ERROR;
}

int test1()
{
    const int p[10] = {5,2,  8,2,  0,1,  1,1,  1, 0};
    return test_intersection(p);
}

int test2()
{
    const int p[10] = {5,2,  8,2,  0,2,  1,2,  1, 0};
    return test_intersection(p);
}

int test3()
{
    const int p[10] = {5,2,  8,2,  0,2,  5,2,  1, 1};
    return test_intersection(p);
}

int test4()
{
    const int p[10] = {5,2,  8,2,  0,2,  9,2,  1, 1};
    return test_intersection(p);
}

int test5()
{
    const int p[10] = {0,2,  1,2,  5,2,  8,2,  1, 0};
    return test_intersection(p);
}

int test6()
{
    const int p[10] = {0,2,  5,2,  5,2,  8,2,  1, 1};
    return test_intersection(p);
}

int test7()
{
    const int p[14] = {5,3,  1,1,  1,7,  3,2,  0, 1,  3,1, 2,1};
    return test_intersection(p);
}

int test8()
{
    const int p[10] = {5,3,  1,1,  1,7,  3,3,  0, 0};
    return test_intersection(p);
}

int test9()
{
    const int p[14] = {5,3,  1,1,  1,7,  3,1,  0, 1,  19,7, 13,7};
    return test_intersection(p);
}


RatPoint_line_segment seg(
    int axnum,
    int axden,
    int aynum,
    int ayden,
    int bxnum,
    int bxden,
    int bynum,
    int byden
)
{
    typedef RatPoint::Rat Rat;
    return RatPoint_line_segment(
            RatPoint(Rat(axnum, axden), Rat(aynum, ayden)),
            RatPoint(Rat(bxnum, bxden), Rat(bynum, byden))
        );
}


int test10()
{
    typedef RatPoint::Rat Rat;
    const RatPoint_line_segment s(seg(35,39, 17,13, 127,11, 521,47));

    // test endpoint -- they should be on the segment b/c segment is "closed."
    TEST_TRUE(is_on(s, RatPoint(Rat(35,39), Rat(17,13))));
    TEST_TRUE(is_on(s, RatPoint(Rat(127,11), Rat(521,47))));

    // Test somewhere in the middle, computed by hand.
    // On some architectures, this computation overflows a boost:rational<int>.
    TEST_TRUE(is_on(s, RatPoint(Rat(5,1), Rat(544772,107348))));

    // next point is on the line but not on the segment
    TEST_FALSE(is_on(s, RatPoint(Rat(18477,32857), Rat(1,1))));

    return kjb_c::NO_ERROR;
}

// test triangle_area
int test11()
{
    typedef RatPoint::Rat Rat;
    const RatPoint_line_segment s(seg(1,1, 4,1, 5,1, 1,1));
    const Rat   a1 = triangle_area(s, RatPoint(Rat(1,1), Rat(1,1))),
                a2 = triangle_area(s, RatPoint(Rat(5,1), Rat(4,1)));
    TEST_TRUE(-6==i_numerator(a1) && 1==i_denominator(a1));
    TEST_TRUE(+6==i_numerator(a2) && 1==i_denominator(a2));

    // double-check the "counterclockwise means positive" quality:
    // t goes from origin to point (x,y)=(1,0).
    const RatPoint_line_segment t(seg(0,1, 0,1,   1,1, 0,1));
    const Rat   a3 = triangle_area(t, RatPoint(Rat(1,1), Rat(1,1))),
                a4 = triangle_area(t, RatPoint(Rat(0,1), Rat(1,1)));
    TEST_TRUE(a3==a4);
    TEST_TRUE(a3 > 0);
    TEST_TRUE(1==i_numerator(a3) && 2==i_denominator(a3));

    // double-check "clockwise means negative area":
    const Rat   a5 = triangle_area(t, RatPoint(Rat(1,1), Rat(-1,1))),
                a6 = triangle_area(t, RatPoint(Rat(0,1), Rat(-1,1)));
    TEST_TRUE(a5==a6);
    TEST_TRUE(a5 < 0);
    TEST_TRUE(-1==i_numerator(a5) && 2==i_denominator(a5));
    return kjb_c::NO_ERROR;
}


// return FALSE IF BAD
bool test_intersection(
    const RatPoint_line_segment& s,
    const RatPoint_line_segment& t,
    bool answer
)
{
    const RatPoint_line_segment sflip(s.b, s.a), tflip(t.b, t.a);

    return      is_intersecting(s,t) == answer
            &&  is_intersecting(t,s) == answer
            &&  is_intersecting(sflip,t) == answer
            &&  is_intersecting(t,sflip) == answer
            &&  is_intersecting(s, tflip) == answer
            &&  is_intersecting(tflip, s) == answer
            &&  is_intersecting(sflip, tflip) == answer
            &&  is_intersecting(tflip, sflip) == answer;
}

// test line segment intersection predicate
int test12()
{
    using kjb::qd::is_intersecting;

    const RatPoint_line_segment s1(seg(1,1, 1,1,  2,1, 2,1)),
                                t1(seg(3,1, 3,1,  4,1, 4,1)),
                                t2(seg(3,1, 3,1,  2,1, 2,1)),
                                            t3(seg(3,1, 3,1,  0,1, 0,1)),
                                            t4(seg(0,1, 0,1,  3,1, 3,1)),
                                            t5(seg(1,1, 1,1,  1,1, 1,1)),
                                            t6(seg(3,1, 3,1,  3,1, 3,1)),
                                            t7(seg(3,2, 3,2,  3,2, 3,2)),
                                            t8(seg(3,2, 3,1,  5,2, 4,1)),
                                            t9(seg(0,1, 3,2,  5,2, 4,1));

    TEST_TRUE(test_intersection(s1, t1, 0));
    TEST_TRUE(test_intersection(s1, t2, 1));
    TEST_TRUE(test_intersection(s1, t3, 1));
    TEST_TRUE(test_intersection(s1, t4, 1));
    TEST_TRUE(test_intersection(s1, t5, 1));
    TEST_TRUE(test_intersection(s1, t6, 0));
    TEST_TRUE(test_intersection(s1, t7, 1));
    TEST_TRUE(test_intersection(s1, t8, 0));
    TEST_TRUE(test_intersection(s1, t9, 0));

    const RatPoint_line_segment u1(seg(3,2, 3,2,  3,2, 5,2)),
                                u2(seg(3,2, 1,1,  3,2, 5,2)),
                                u3(seg(3,2, 13,8, 3,2, 5,2));

    TEST_TRUE(test_intersection(s1, u1, 1));
    TEST_TRUE(test_intersection(s1, u2, 1));
    TEST_TRUE(test_intersection(s1, u3, 0));

    return kjb_c::NO_ERROR;
}


int test13()
{
    using kjb::qd::closest_point_on_seg;
    typedef RatPoint::Rat Rat;

    const RatPoint_line_segment domain(RatPoint(0,3), RatPoint(1,1));

    const RatPoint x1 = closest_point_on_seg( RatPoint(1, 5),
                            domain),
                   x2 = closest_point_on_seg( RatPoint(2, 4),
                            domain),
                   x3 = closest_point_on_seg( RatPoint(Rat(9, 4), Rat(7, 2)),
                            domain),
                   x4 = closest_point_on_seg( RatPoint(Rat(5, 2), 3),
                            domain),
                   x5 = closest_point_on_seg( RatPoint(Rat(11, 4), Rat(5, 2)),
                            domain),
                   x6 = closest_point_on_seg( RatPoint(3, 2),
                            domain),
                   x7 = closest_point_on_seg( RatPoint(4, 1),
                            domain);

    TEST_TRUE( x1 == RatPoint(0, 3));
    TEST_TRUE( x2 == RatPoint(0, 3));
    TEST_TRUE( x3 == RatPoint(Rat(1, 4), Rat(5, 2)));
    TEST_TRUE( x4 == RatPoint(Rat(1, 2), 2));
    TEST_TRUE( x5 == RatPoint(Rat(3, 4), Rat(3, 2)));
    TEST_TRUE( x6 == RatPoint(1, 1));
    TEST_TRUE( x7 == RatPoint(1, 1));

    return kjb_c::NO_ERROR;
}


}



int main(int argc, char** argv)
{
    KJB(kjb_c::kjb_init());

    /* Tests for elementary intersection functions of ratpoint.{h, cpp}. */
    KJB(EPETE(test1()));
    KJB(EPETE(test2()));
    KJB(EPETE(test3()));
    KJB(EPETE(test4()));
    KJB(EPETE(test5()));
    KJB(EPETE(test6()));
    KJB(EPETE(test7()));
    KJB(EPETE(test8()));
    KJB(EPETE(test9()));
    KJB(EPETE(test10()));
    KJB(EPETE(test11()));
    KJB(EPETE(test12()));
    KJB(EPETE(test13()));

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}
