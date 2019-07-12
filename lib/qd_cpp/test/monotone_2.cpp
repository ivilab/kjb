/**
 * @file
 * @brief unit test for monotone regularization
 * @author Andy Predoehl
 *
 * This program is not very fast.  It takes 26 seconds on v11 if you compile
 * in production mode.  In development mode it is more like twenty minutes.
 */
/*
 * $Id: monotone_2.cpp 20160 2015-12-08 23:36:20Z predoehl $
 */

#include <l_cpp/l_test.h>
#include <qd_cpp/triangulate.h>
#include <qd_cpp/svg_dcel.h>
#include <gsl_cpp/gsl_qrng.h>

#include <string>
#include <fstream>

#define TRIANGLE_1_ZIG_0 1


namespace
{

using kjb::qd::PixPoint;
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::Doubly_connected_edge_list;

const int VISUALIZATION = 0; // meaningful values: 0=none, 1=some, 2=more.

const long GRANULARITY = 128;


RatPoint quasirandom_pt(kjb::Gsl_Qrng_Sobol& qrng)
{
    kjb::Vector v( 2 );
    v = qrng.read();

    return RatPoint(RatPoint::Rat(long(v[0] * GRANULARITY), GRANULARITY),
                    RatPoint::Rat(long(v[1] * GRANULARITY), GRANULARITY));
}


// kjb_rand is repeatable, which is what I want.
int shuffler(int i)
{
    const int BIG = 65536, dbli = 2*i;
    KJB(ASSERT(i <= BIG));
    int u = kjb_c::kjb_rand() * BIG;
    // shift right u until m is within an octave of the size of i
    for (int m = BIG; m >= dbli; m >>= 1)
    {
        u >>= 1;
    }
    return u % i;
}

Doubly_connected_edge_list complicated()
{
    Doubly_connected_edge_list d;
    kjb::Gsl_Qrng_Sobol qq( 2 );

    d = d.merge(
        kjb::qd::get_axis_aligned_rectangle(PixPoint(0,0), PixPoint(1,1)));
    std::vector< RatPoint > vertices;

    for (int i = 0; i < 30; ++i)
    {
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
    }

    std::random_shuffle(vertices.begin(), vertices.end(), shuffler);

    std::vector< RatPoint_line_segment > obstacles;
    while (vertices.size() >= 3)
    {
        // draw triangles-shapes in the square
        const RatPoint a = vertices.back();
        vertices.pop_back();
        const RatPoint b = vertices.back();
        vertices.pop_back();
        const RatPoint c = vertices.back();
        vertices.pop_back();

        obstacles.push_back(RatPoint_line_segment(a, b));
        obstacles.push_back(RatPoint_line_segment(b, c));
#if TRIANGLE_1_ZIG_0
        obstacles.push_back(RatPoint_line_segment(c, a));
#endif
    }
    d = d.merge(kjb::qd::ctor_from_edge_list(obstacles));

    if (VISUALIZATION > 0)
    {
        std::ofstream ff("m2_complicated.svg");
        ff << draw_dcel_as_svg(d);

        // It is possible to enlarge the DCEL until all coordinates are
        // integers.  This will do it.  Unfortunately the resulting
        // integers will be like 500 digits long, and unrenderable.
        // This code demonstrated that it worked.
        if (0)
        {
            RatPoint::Rat lcd = kjb::qd::common_denominator(d),
                          t[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
            t[0] *= lcd;
            t[4] *= lcd;
            std::ofstream fcd1("m2i.svg"), fcd2("m2i.xml");
            d.transform(t);
            fcd1 << draw_dcel_as_svg(d);
            fcd2 << xml_output(d);
        }
    }
    return d;
}

int test1()
{
    const Doubly_connected_edge_list d = complicated();
    static int count = 0;
    char buf[64];

    if (VISUALIZATION > 0)
    {
        snprintf(buf, sizeof(buf), "m2t1s%02d.", ++count);
        std::ofstream fx((std::string(buf)+"xml").c_str());
        fx << xml_output(d);
        std::ofstream fg((std::string(buf)+"svg").c_str());
        fg << kjb::qd::draw_dcel_as_svg(d);

        for (size_t i = 1; VISUALIZATION > 1 && i < d.get_face_table().size(); ++i)
        {
            const Doubly_connected_edge_list exp = d.face_export(i);
            snprintf(buf, sizeof(buf), "m2t1f%02d.", ++count);
            std::ofstream ff((std::string(buf)+"xml").c_str());
            ff << xml_output(exp);
            std::ofstream fg((std::string(buf)+"svg").c_str());
            fg << kjb::qd::draw_dcel_as_svg(exp);
        }
    }

    const Doubly_connected_edge_list e = kjb::qd::make_faces_ymonotone(d);

    if (VISUALIZATION > 0)
    {
        snprintf(buf, sizeof(buf), "m2t1s%02d.", ++count);
        std::ofstream fx((std::string(buf)+"xml").c_str());
        fx << xml_output(e);
        std::ofstream fg((std::string(buf)+"svg").c_str());
        fg << kjb::qd::draw_dcel_as_svg(e);
    }

    // Skip face zero, of course.
    for (size_t i = 1; i < e.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_ymonotone(e, i));
    }

    return kjb_c::NO_ERROR;
}



int test2()
{
    kjb::qd::PixPath t1 = kjb::qd::PixPath::reserve(3), t2(t1);
    t1.push_back(PixPoint(0,0));
    t1.push_back(PixPoint(10,0));
    t1.push_back(PixPoint(5,8));
    t2.push_back(PixPoint(5,5));
    t2.push_back(PixPoint(6,3));
    t2.push_back(PixPoint(9,5));
    const Doubly_connected_edge_list
        t3 = Doubly_connected_edge_list::ctor_closed_path(t1),
        t4 = Doubly_connected_edge_list::ctor_open_path(t2),
        t5 = t3.merge(t4);

    if (VISUALIZATION > 0)
    {
        std::ofstream f("m2t2s1.svg");
        f << draw_dcel_as_svg(t5);
    }

    const Doubly_connected_edge_list
        e = kjb::qd::make_faces_ymonotone(t5);

    if (VISUALIZATION > 0)
    {
        std::ofstream f("m2t2s2.svg");
        f << draw_dcel_as_svg(e);
    }

    // Skip face zero, of course.
    for (size_t i = 1; i < e.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_ymonotone(e, i));
    }

    return kjb_c::NO_ERROR;
}


}

int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));

    int time_factor = 1;
    KJB(EPETE(scan_time_factor(argv[1], &time_factor)));

    try
    {
        if (time_factor > 1) KJB(EPETE(test1()));
        KJB(EPETE(test2()));
    }
    catch(const kjb::Exception& e)
    {
        e.print_details_exit();
    }

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

