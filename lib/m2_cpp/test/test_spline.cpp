/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2009-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek.                                                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_spline.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#define DEBUG

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_init.h"
#include "m_cpp/m_spline.h"
#include "l/l_sys_term.h"
#include "l/l_sys_rand.h"
#include "m_cpp/m_vector.h"
#include "gr_cpp/gr_opengl.h"
#include <vector>

#include "test.h"

#if 0 /* ------------------------------------------------------------*/
#define TEST_SUCCESS(line) \
    try{line;} \
    catch(...) \
    { \
        printf("Test-Success failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    }
#define TEST_FAIL(line) \
    { \
        bool assert_fail_failed = false; \
        try{line;} \
        catch(...) \
        { \
            assert_fail_failed = true; \
        } \
        if(!assert_fail_failed) \
        { \
            printf("Test-fail failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    }
#endif /* ----------------------------------------------------------- */

namespace {

double nurbs_curve_1_out[101][3] =
{
 0, 0, 0,
 0.0297, 0.000298, 0,
 0.05879999999999999, 0.001184, 0,
 0.08729999999999999, 0.002646, 0,
 0.1152, 0.004671999999999999, 0,
 0.1425, 0.007250000000000001, 0,
 0.1692, 0.010368, 0,
 0.1953, 0.014014, 0,
 0.2208000000000001, 0.018176, 0,
 0.2457, 0.022842, 0,
 0.2700000000000001, 0.028, 0,
 0.2937, 0.033638, 0,
 0.3168, 0.03974399999999999, 0,
 0.3393, 0.046306, 0,
 0.3612, 0.05331200000000001, 0,
 0.3825, 0.06075, 0,
 0.4031999999999999, 0.068608, 0,
 0.4233, 0.07687400000000001, 0,
 0.4428, 0.085536, 0,
 0.4617000000000001, 0.09458200000000001, 0,
 0.4800000000000001, 0.104, 0,
 0.4977, 0.113778, 0,
 0.5148, 0.123904, 0,
 0.5313000000000001, 0.134366, 0,
 0.5472, 0.145152, 0,
 0.5625, 0.15625, 0,
 0.5772, 0.167648, 0,
 0.5912999999999999, 0.179334, 0,
 0.6048, 0.191296, 0,
 0.6176999999999999, 0.203522, 0,
 0.6299999999999999, 0.216, 0,
 0.6416999999999999, 0.228718, 0,
 0.6527999999999999, 0.241664, 0,
 0.6633, 0.254826, 0,
 0.6731999999999999, 0.268192, 0,
 0.6824999999999999, 0.28175, 0,
 0.6912, 0.295488, 0,
 0.6993, 0.309394, 0,
 0.7068000000000001, 0.323456, 0,
 0.7137, 0.337662, 0,
 0.72, 0.352, 0,
 0.7257, 0.3664580000000001, 0,
 0.7308000000000001, 0.381024, 0,
 0.7353000000000001, 0.395686, 0,
 0.7392000000000001, 0.410432, 0,
 0.7425000000000002, 0.4252500000000001, 0,
 0.7452000000000001, 0.4401280000000001, 0,
 0.7473000000000001, 0.4550540000000001, 0,
 0.7487999999999999, 0.470016, 0,
 0.7497, 0.485002, 0,
 0.75, 0.5, 0,
 0.7497, 0.514998, 0,
 0.7487999999999999, 0.529984, 0,
 0.7473, 0.544946, 0,
 0.7452, 0.559872, 0,
 0.7424999999999999, 0.5747500000000001, 0,
 0.7392, 0.5895680000000001, 0,
 0.7353, 0.604314, 0,
 0.7308000000000001, 0.618976, 0,
 0.7257, 0.6335419999999999, 0,
 0.72, 0.6479999999999999, 0,
 0.7137, 0.662338, 0,
 0.7068000000000001, 0.676544, 0,
 0.6993, 0.6906060000000001, 0,
 0.6912, 0.704512, 0,
 0.6824999999999999, 0.7182500000000001, 0,
 0.6732, 0.731808, 0,
 0.6633, 0.745174, 0,
 0.6528, 0.7583360000000001, 0,
 0.6416999999999999, 0.771282, 0,
 0.6299999999999999, 0.784, 0,
 0.6177000000000001, 0.796478, 0,
 0.6048, 0.808704, 0,
 0.5912999999999999, 0.8206659999999999, 0,
 0.5772, 0.832352, 0,
 0.5625, 0.84375, 0,
 0.5472, 0.8548480000000001, 0,
 0.5312999999999999, 0.865634, 0,
 0.5147999999999999, 0.876096, 0,
 0.4977, 0.8862220000000002, 0,
 0.4799999999999999, 0.8960000000000001, 0,
 0.4616999999999998, 0.9054180000000001, 0,
 0.4427999999999999, 0.9144639999999999, 0,
 0.4232999999999998, 0.9231260000000001, 0,
 0.4032000000000001, 0.931392, 0,
 0.3825, 0.9392499999999999, 0,
 0.3612, 0.946688, 0,
 0.3393, 0.953694, 0,
 0.3168, 0.960256, 0,
 0.2937, 0.9663620000000001, 0,
 0.27, 0.9720000000000001, 0,
 0.2456999999999999, 0.9771580000000001, 0,
 0.2207999999999999, 0.981824, 0,
 0.1952999999999999, 0.985986, 0,
 0.1691999999999999, 0.9896320000000001, 0,
 0.1424999999999998, 0.9927499999999999, 0,
 0.1152000000000001, 0.995328, 0,
 0.08730000000000007, 0.9973540000000001, 0,
 0.05880000000000005, 0.9988159999999999, 0,
 0.02970000000000003, 0.999702, 0,
 0, 1, 0
};

// reference output for nurbs curve 2, sampled between 0 and 1 at 0.01 intervals
// Generated in octave using Nurbs package's bspeval() function
double nurbs_curve_2_out[101][3] =
{
 0.5, 0, 0,
 0.5576319999999999, 3.200000000000001e-05, 0,
 0.6106560000000001, 0.000256, 0,
 0.6592639999999999, 0.000864, 0,
 0.7036479999999998, 0.002048, 0,
 0.7440000000000002, 0.004000000000000001, 0,
 0.780512, 0.006912, 0,
 0.813376, 0.010976, 0,
 0.8427839999999999, 0.016384, 0,
 0.8689279999999999, 0.023328, 0,
 0.892, 0.03200000000000001, 0,
 0.9121920000000002, 0.042592, 0,
 0.929696, 0.055296, 0,
 0.944704, 0.07030400000000001, 0,
 0.957408, 0.08780800000000002, 0,
 0.968, 0.108, 0,
 0.9766720000000001, 0.131072, 0,
 0.983616, 0.1572160000000001, 0,
 0.9890240000000001, 0.186624, 0,
 0.9930880000000001, 0.219488, 0,
 0.996, 0.2560000000000001, 0,
 0.997952, 0.2963519999999999, 0,
 0.9991359999999999, 0.340736, 0,
 0.999744, 0.389344, 0,
 0.9999680000000001, 0.442368, 0,
 1, 0.5, 0,
 0.9999680000000001, 0.5576320000000001, 0,
 0.9997440000000001, 0.6106560000000001, 0,
 0.999136, 0.6592640000000001, 0,
 0.997952, 0.7036479999999998, 0,
 0.9960000000000001, 0.744, 0,
 0.9930880000000001, 0.780512, 0,
 0.989024, 0.813376, 0,
 0.983616, 0.842784, 0,
 0.976672, 0.8689280000000001, 0,
 0.968, 0.892, 0,
 0.957408, 0.9121919999999999, 0,
 0.944704, 0.929696, 0,
 0.929696, 0.944704, 0,
 0.912192, 0.957408, 0,
 0.8919999999999999, 0.9680000000000001, 0,
 0.8689279999999999, 0.976672, 0,
 0.842784, 0.9836159999999999, 0,
 0.813376, 0.9890240000000001, 0,
 0.780512, 0.9930880000000001, 0,
 0.744, 0.996, 0,
 0.7036479999999998, 0.997952, 0,
 0.6592639999999999, 0.999136, 0,
 0.6106560000000001, 0.999744, 0,
 0.5576320000000001, 0.9999680000000001, 0,
 0.5, 1, 0,
 0.442368, 0.9999680000000001, 0,
 0.3893439999999999, 0.9997440000000001, 0,
 0.3407359999999999, 0.999136, 0,
 0.2963519999999998, 0.9979520000000001, 0,
 0.2559999999999998, 0.996, 0,
 0.2194879999999998, 0.9930880000000001, 0,
 0.1866239999999998, 0.9890239999999999, 0,
 0.1572160000000001, 0.9836159999999999, 0,
 0.1310720000000001, 0.976672, 0,
 0.108, 0.9680000000000001, 0,
 0.08780800000000002, 0.957408, 0,
 0.07030400000000001, 0.944704, 0,
 0.055296, 0.929696, 0,
 0.04259199999999998, 0.912192, 0,
 0.03199999999999998, 0.8919999999999999, 0,
 0.02332799999999998, 0.8689279999999999, 0,
 0.01638399999999997, 0.8427839999999999, 0,
 0.01097599999999998, 0.8133759999999999, 0,
 0.00691199999999998, 0.7805119999999997, 0,
 0.003999999999999984, 0.7439999999999998, 0,
 0.002048000000000006, 0.7036480000000002, 0,
 0.0008640000000000024, 0.6592640000000001, 0,
 0.0002560000000000007, 0.6106560000000001, 0,
 3.200000000000009e-05, 0.5576320000000001, 0,
 0, 0.5, 0,
 3.200000000000009e-05, 0.442368, 0,
 0.0002560000000000007, 0.3893439999999999, 0,
 0.0008640000000000024, 0.3407359999999999, 0,
 0.002048000000000006, 0.2963519999999998, 0,
 0.00400000000000001, 0.2559999999999998, 0,
 0.006912000000000019, 0.2194879999999998, 0,
 0.01097600000000003, 0.1866239999999998, 0,
 0.01638400000000004, 0.1572159999999998, 0,
 0.02332799999999998, 0.1310720000000001, 0,
 0.03199999999999998, 0.108, 0,
 0.04259199999999998, 0.08780800000000002, 0,
 0.055296, 0.07030400000000001, 0,
 0.07030400000000001, 0.055296, 0,
 0.08780800000000002, 0.04259199999999998, 0,
 0.108, 0.03199999999999998, 0,
 0.1310720000000001, 0.02332799999999998, 0,
 0.1572160000000001, 0.01638399999999997, 0,
 0.1866240000000002, 0.01097599999999998, 0,
 0.2194880000000002, 0.00691199999999998, 0,
 0.2560000000000003, 0.003999999999999984, 0,
 0.2963519999999998, 0.002048000000000006, 0,
 0.3407359999999999, 0.0008640000000000024, 0,
 0.3893439999999999, 0.0002560000000000007, 0,
 0.442368, 3.200000000000009e-05, 0,
 0.5, 0, 0
};

using kjb::Vector;
using kjb::Nurbs_curve;
using kjb::Spline_curve;
using kjb::Bezier_curve;
using kjb::Polybezier_curve;

void init( std::vector<Vector>&, std::vector<Vector>& );

void test_nurbs(
    const std::vector<Vector>&,
    const std::vector<Vector>&,
    Nurbs_curve&,
    Nurbs_curve&
);

void test_bezier(
    const std::vector<Vector>&,
    const std::vector<Vector>&,
    const Nurbs_curve&
);

void test_polybezier(
    const std::vector<Vector>&,
    const std::vector<Vector>&,
    const Nurbs_curve&
);

int test_main();

bool spline_equal(const Spline_curve& c1, const Spline_curve& c2);
bool float_equal(double d1, double d2);

const int KNOTS_A_SIZE = 8;
const int KNOTS_B_SIZE = 17;

float knots_A[ KNOTS_A_SIZE ];
float knots_B[ KNOTS_B_SIZE ];

} // anonymous namespace

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int main()
{
    std::vector<Vector> ctl_points;
    std::vector<Vector> ctl_points_2;
    Nurbs_curve ref_1, ref_2;

    kjb_c::kjb_init();
    kjb_c::kjb_disable_paging();
    init( ctl_points, ctl_points_2 );

    kjb_c::kjb_seed_rand_with_tod();

    try {
        test_nurbs( ctl_points, ctl_points_2, ref_1, ref_2 );
    }
    catch( kjb::Exception& e ) {
        std::cout << "caught Exception!\n";
        e.print_details();
        return EXIT_FAILURE;
    }
    catch(...) {
        std::cout << "caught something!\n";
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

namespace {

void init(
    std::vector<Vector>& ctl_points,
    std::vector<Vector>& ctl_points_2
)
{
    Vector v(3);
    ctl_points.reserve(4);
    v[0] = 0; v[1] = 0; v[2] = 0;
    ctl_points.push_back(v);
    v[0] = 1; v[1] = 0; v[2] = 0;
    ctl_points.push_back(v);
    v[0] = 1; v[1] = 1; v[2] = 0;
    ctl_points.push_back(v);
    v[0] = 0; v[1] = 1; v[2] = 0;
    ctl_points.push_back(v);


    knots_A[0] = knots_A[1] = knots_A[2] = knots_A[3] = 0;
    knots_A[4] = knots_A[5] = knots_A[6] = knots_A[7] = 1;


    ctl_points_2.reserve(13);
    knots_B[0] = knots_B[1] = knots_B[2] = knots_B[3] = 0;

    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);


    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 1.0; v[1] = 0.5; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    ctl_points_2.push_back(v);

    knots_B[4] = knots_B[5] = knots_B[6] = 1;

    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 0.5; v[1] = 1.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    ctl_points_2.push_back(v);

    knots_B[7] = knots_B[8] = knots_B[9] = 2;


    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 0.0; v[1] = 0.5; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);

    knots_B[10] = knots_B[11] = knots_B[12] = 3;

    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);
    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    ctl_points_2.push_back(v);

    knots_B[13] = knots_B[14] = knots_B[15] = knots_B[16] = 4;
}

void test_nurbs(
    const std::vector<Vector>& ctl_points,
    const std::vector<Vector>& ctl_points_2,
    Nurbs_curve& ref_1,
    Nurbs_curve& ref_2
)
{
    // test: constructors
    Nurbs_curve nurbs_1;
    Nurbs_curve nurbs_2( KNOTS_A_SIZE, knots_A, 3, ctl_points );
    TEST_FAIL( Nurbs_curve( KNOTS_A_SIZE+1, knots_A, 3, ctl_points ) );
    TEST_FAIL( Nurbs_curve( KNOTS_A_SIZE-1, knots_A, 3, ctl_points ) );
    Nurbs_curve nurbs_3(nurbs_2);
    TEST_TRUE( spline_equal(nurbs_2, nurbs_3) );

    // test: destructor
    Nurbs_curve* tmp = nurbs_2.clone();
    delete tmp;

    // test: assignment
    nurbs_1 = nurbs_2;
    ASSERT(spline_equal(nurbs_1, nurbs_2));

    // test: mutators

    // (not implemented)
//    nurbs_2.insert_at(4, Vector(3, 0.));

    // test: accessors
    TEST_SUCCESS(nurbs_2.evaluate(0));
    TEST_SUCCESS(nurbs_2.evaluate(.1));
    TEST_SUCCESS(nurbs_2.evaluate(.5));
    TEST_SUCCESS(nurbs_2.evaluate(.9));
    TEST_FAIL(nurbs_2.evaluate(-.1));
    TEST_FAIL(nurbs_2.evaluate(1));
    TEST_FAIL(nurbs_2.evaluate(10));

    // test for accuracy

    // Curve 1
    nurbs_1 = Nurbs_curve(KNOTS_A_SIZE, knots_A, 3, ctl_points);
    for(int i = 0; i < 100; i++)
    {
        Vector v = nurbs_1.evaluate((float) i / 100);

        TEST_TRUE( float_equal(v[0], nurbs_curve_1_out[i][0]) );
        TEST_TRUE( float_equal(v[1], nurbs_curve_1_out[i][1]) );
        TEST_TRUE( float_equal(v[2], nurbs_curve_1_out[i][2]) );
    }

    // save this curve as global for later testing of bezier and polybezier
    ref_1 = nurbs_1;

    // curve 2
    nurbs_2 = Nurbs_curve(KNOTS_B_SIZE, knots_B, 3, ctl_points_2);
    for(int i = 0; i < 100; i++)
    {
        Vector v = nurbs_2.evaluate((float) i / 100);

        ASSERT(float_equal(v[0], nurbs_curve_2_out[i][0]));
        ASSERT(float_equal(v[1], nurbs_curve_2_out[i][1]));
        ASSERT(float_equal(v[2], nurbs_curve_2_out[i][2]));
    }

    // save this curve as global for later testing of bezier and polybezier
    ref_2 = nurbs_2;
}

void test_bezier(
    const std::vector<Vector>& ctl_points,
    const std::vector<Vector>& ctl_points_2,
    const Nurbs_curve& ref_1
)
{
    // test: constructors
    Bezier_curve bezier_1;
    Bezier_curve bezier_2(4, 4);
    Bezier_curve bezier_3(bezier_2);
    ASSERT(spline_equal(bezier_2, bezier_3));

    // test: destructor
    Bezier_curve* tmp = bezier_2.clone();
    delete tmp;

    // test: mutators
    bezier_2 = Bezier_curve(3,3);
    bezier_2.set_control_point(0, ctl_points[0]);
    bezier_2.set_control_point(1, ctl_points[1]);
    bezier_2.set_control_point(2, ctl_points[2]);
    bezier_2.set_control_point(3, ctl_points[3]);

    // test: assignment
    bezier_1 = bezier_2;
    ASSERT(spline_equal(bezier_1, bezier_2));

    // test for accuracy
    for(int i = 0; i < 100; i++)
    {
        Vector v = bezier_2.evaluate((float) i / 100);

        ASSERT(float_equal(v[0], nurbs_curve_1_out[i][0]));
        ASSERT(float_equal(v[1], nurbs_curve_1_out[i][1]));
        ASSERT(float_equal(v[2], nurbs_curve_1_out[i][2]));
    }

    // test to_nurbs()
    Nurbs_curve tmp_nurbs = bezier_2.to_nurbs();
    ASSERT(spline_equal(tmp_nurbs, ref_1));
}

void test_polybezier(
    const std::vector<Vector>& ctl_points,
    const std::vector<Vector>& ctl_points_2,
    const Nurbs_curve& ref_2
)
{
    // test: constructors
    Polybezier_curve polybezier_1;
    Polybezier_curve polybezier_2(4);
    Polybezier_curve polybezier_3(polybezier_2);
    ASSERT(spline_equal(polybezier_2, polybezier_3));

    // test: destructor
    Polybezier_curve* tmp = polybezier_2.clone();
    delete tmp;

    // test: mutators
    polybezier_2 = Polybezier_curve(3);
    int i;
    for(i = 0; i < 4; i++)
    {
        polybezier_2.insert_curve_point(i, ctl_points_2[3 * i]);
        polybezier_2.set_handle_point_2(i, ctl_points_2[3*i+1]);
        polybezier_2.set_handle_point_1(i, ctl_points_2[3*i+2]);
    }
    polybezier_2.insert_curve_point(i, ctl_points_2[3 * i]);

    // test: assignment
    polybezier_1 = polybezier_2;
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    polybezier_1 = Polybezier_curve(3);

    // test alternate insert() method
    for(i = 0; i < 5; i++)
    {
        polybezier_1.insert(i,
                ctl_points[3* i],
                ctl_points[std::max(0, 3*i - 1)],
                ctl_points[std::min(12,3*i + 1)]);
    }
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    // test get()/set() method
    Vector h1 = polybezier_1.get_handle_point_1(3);
    Vector h2 = polybezier_1.get_handle_point_2(3);
    Vector c  = polybezier_1.get_curve_point(3);

    polybezier_1.set(3, Vector(3, 0.), Vector(3, 0.), Vector(3, 0.));
    ASSERT(!spline_equal(polybezier_1, polybezier_2));

    polybezier_1.set(3, c, h1, h2);
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    // test (x,y,z) variant of set method
    polybezier_1.set_handle_point_1(3, 0,0,0);
    ASSERT(!spline_equal(polybezier_1, polybezier_2));

    polybezier_1.set_handle_point_1(3, h1[0], h1[1], h1[2]);
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    // test symmetrize
    polybezier_1.set_handle_point_1(3, 0,0,0);
    polybezier_1.symmetrize_handle_1(3);
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    polybezier_1.set_handle_point_2(3, 0,0,0);
    polybezier_1.symmetrize_handle_2(3);
    ASSERT(spline_equal(polybezier_1, polybezier_2));

    // test to_nurbs()
    Nurbs_curve tmp_nurbs = polybezier_1.to_nurbs();
    ASSERT(spline_equal(tmp_nurbs, ref_2));

}

bool spline_equal(const Spline_curve& c1, const Spline_curve& c2)
{
    for(int i = 0; i < 1000; i++)
    {
        double u = kjb_c::kjb_rand();
        if( !float_equal(c1.evaluate(u)[0], c2.evaluate(u)[0]) ||
            !float_equal(c1.evaluate(u)[1], c2.evaluate(u)[1]) ||
            !float_equal(c1.evaluate(u)[2], c2.evaluate(u)[2]))
            return false;
    }
    return true;
}

bool float_equal(double d1, double d2)
{
    const double FLOAT_EQ_THRESHOLD = 1e-6;
    return (fabs(d1 - d2) < FLOAT_EQ_THRESHOLD);
}

} // anonymous namespace
