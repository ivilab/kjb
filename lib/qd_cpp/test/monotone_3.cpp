/**
 * @file
 * @brief unit test for monotone regularization
 * @author Andy Predoehl
 *
 * This program is not very fast.  It takes 26 seconds on v11 if you compile
 * in production mode.  In development mode it is much slower.
 */
/*
 * $Id: monotone_3.cpp 20166 2015-12-09 21:50:27Z predoehl $
 */

#include <l_cpp/l_test.h>
#include <gsl_cpp/gsl_qrng.h>
#include <qd_cpp/pixpath.h>
#include <qd_cpp/triangulate.h>
#include <qd_cpp/svg_dcel.h>

#include <string>
#include <fstream>
#include <iostream>


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

    for (int i = 0; i < 24; ++i)
    {
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
    }

    std::random_shuffle(vertices.begin(), vertices.end(), shuffler);

    int j = 1000;
    const size_t starsize = 8;
    while (vertices.size() >= starsize)
    {
        // draw triangles-shapes in the square
        std::vector< RatPoint > octo;
        RatPoint c(0,0);
        for (size_t i = 0; i < starsize; ++i)
        {
            octo.push_back(vertices.back());
            c.x += vertices.back().x;
            c.y += vertices.back().y;
            vertices.pop_back();
        }
        c.x /= starsize;
        c.y /= starsize;

        // outer spines
        Doubly_connected_edge_list o1;
        for (size_t i = 0; i < starsize; ++i)
        {
            o1 = o1.merge(RatPoint_line_segment(c, octo[i]));
        }
        const size_t vix = o1.lookup_vertex(c);
        KJB(ASSERT(1+starsize == o1.get_vertex_table().size()));
        KJB(ASSERT(vix < 1+starsize));
        o1.translate(RatPoint(-c.x, -c.y));

        // inner core
        Doubly_connected_edge_list o2(o1);
        const RatPoint::Rat tweak[9] = {3, -4, 0, 4, 3, 0, 0, 0, 10};
        o2.transform(tweak);

        // stitch together
        Doubly_connected_edge_list o3;
        size_t ej = o1.get_vertex_table().at(vix).outedge;
        for (size_t i = 0; i < starsize; ++i)
        {
            RatPoint_line_segment s(get_edge(o1, ej));
            KJB(ASSERT(s.a == RatPoint(0,0)));
            ej = o1.get_edge_table().at(ej).next;
            const size_t vj = o1.get_edge_table().at(ej).origin;
            s.a = o2.get_vertex_table().at(vj).location;
            ej = o1.get_edge_table().at(ej).next;
            RatPoint_line_segment t(get_edge(o1, ej));
            KJB(ASSERT(t.a == RatPoint(0,0)));
            t.a = s.a;
            Doubly_connected_edge_list o4(s);
            o4 = o4.merge(t);
            o4 = o4.merge(o3);
            o3.swap(o4);
        }
        o3.translate(c);

        if (VISUALIZATION > 1)
        {
            char fn[32];
            snprintf(fn, sizeof(fn), "zmonotone3_%d.svg", j++);
            std::ofstream ff(fn);
            ff << draw_dcel_as_svg(o3);
        }
        o3 = o3.merge(d);
        d.swap(o3);
    }
    return d;
}


int test1()
{
    const Doubly_connected_edge_list d = complicated(),
                                     e = kjb::qd::make_faces_ymonotone(d);

    /*
    const size_t ct = sd.rfind("</svg>");
    KJB(ASSERT( ct < sd.size() ));
    sd.resize(ct);
    sd += db_fence_labels(d, 1, 0.05) + "</svg>\n";
    */

    if (VISUALIZATION > 0)
    {
        std::string sd = kjb::qd::draw_dcel_as_svg(d),
                    se = kjb::qd::draw_dcel_as_svg(e);

        std::ofstream ff("monotone_3_i.svg");
        ff << sd;
        std::ofstream gg("monotone_3_o.svg");
        gg << se;
    }

    // Skip face zero, of course.
    for (size_t i = 1; i < e.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_ymonotone(e, i));
    }

    return EXIT_SUCCESS;
}


int test2()
{
    typedef RatPoint::Rat Rat;

    kjb::qd::PixPath p(kjb::qd::PixPath::reserve(4)), q(p), r(p);
    p.push_back(PixPoint(0,0));
    p.push_back(PixPoint(1,0));
    p.push_back(PixPoint(1,1));
    p.push_back(PixPoint(2,2));
    p.push_back(PixPoint(3,1));
    p.push_back(PixPoint(3,0));
    p.push_back(PixPoint(4,0));
    p.push_back(PixPoint(2,10));
    Doubly_connected_edge_list
        d = Doubly_connected_edge_list::ctor_closed_path(p);

    q.push_back(PixPoint(0, 0));
    q.push_back(PixPoint(1, -1));
    q.push_back(PixPoint(-1, -1));
    Doubly_connected_edge_list
        e = Doubly_connected_edge_list::ctor_closed_path(q);

    // shrink e to quarter size, then translate
    const Rat x1[9] = {5, 0, 20, 0, 5, 17, 0, 0, 10};
    Doubly_connected_edge_list f = e.transform(x1).merge(d);

    Doubly_connected_edge_list g = f;
    g.translate(PixPoint(5, 0));
    Doubly_connected_edge_list h = g.merge(f);
    h.translate(PixPoint(1, 0));

    r.push_back(PixPoint(-1, -1));
    r.push_back(PixPoint(12, -1));
    r.push_back(PixPoint(7, 30));
    Doubly_connected_edge_list
        j = h.merge(Doubly_connected_edge_list::ctor_closed_path(r));

    if (VISUALIZATION > 1)
    {
        std::ofstream s5("m3t2s5.svg");
        s5 << kjb::qd::draw_dcel_as_svg(j);
    }

    Doubly_connected_edge_list k = kjb::qd::make_faces_ymonotone(j);

    if (VISUALIZATION > 1)
    {
        std::ofstream s6("m3t2s6.svg");
        s6 << kjb::qd::draw_dcel_as_svg(k);
    }

    // Skip face zero, of course.
    for (size_t i = 1; i < k.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_ymonotone(k, i));
    }

    return EXIT_SUCCESS;
}

int test3()
{
    // Even a degenerate segment should construct to a valid (empty) DCEL.
    Doubly_connected_edge_list
        d = RatPoint_line_segment(RatPoint(0,0), RatPoint(0,0));

    TEST_TRUE(is_empty(d));
    TEST_TRUE(kjb_c::NO_ERROR == kjb::qd::is_valid(d));

    return EXIT_SUCCESS;
}


int test4()
{
    // select and stir quasirandom points
    kjb::Gsl_Qrng_Sobol qq( 2 );
    std::vector< RatPoint > vertices;
    for (int i = 0; i < 10; ++i)
    {
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
    }
    std::random_shuffle(vertices.begin(), vertices.end(), shuffler);

    // define bounding box also
    vertices.push_back(PixPoint(0, 0));
    vertices.push_back(PixPoint(0, 1));
    vertices.push_back(PixPoint(0, 1));
    vertices.push_back(PixPoint(1, 1));
    vertices.push_back(PixPoint(1, 1));
    vertices.push_back(PixPoint(1, 0));
    vertices.push_back(PixPoint(1, 0));
    vertices.push_back(PixPoint(0, 0));

    // turn vertex list into segments
    std::vector< RatPoint_line_segment > obstacles;
    while (vertices.size() >= 2)
    {
        RatPoint a(vertices.back());
        vertices.pop_back();
        RatPoint b(vertices.back());
        vertices.pop_back();
        obstacles.push_back(RatPoint_line_segment(a, b));
    }

    // All the above defines the test problem.
    Doubly_connected_edge_list d = ctor_from_edge_list(obstacles);

    if (VISUALIZATION > 1)
    {
        std::ofstream f("m3t4s1.svg");
        f << kjb::qd::draw_dcel_as_svg(d);
    }

    std::vector< RatPoint_line_segment > m;
    for (size_t i = 1; i < d.get_face_table().size(); ++i)
    {
        std::vector< RatPoint_line_segment > mi = edges_to_ymonotonize(d, i);
        std::copy(mi.begin(), mi.end(), std::back_inserter(m));
    }
    //std::copy(m.begin(), m.end(), std::back_inserter(d));
    d = d.merge(ctor_from_edge_list(m));

    if (VISUALIZATION > 1)
    {
        std::ofstream f("m3t4s2.svg");
        f << kjb::qd::draw_dcel_as_svg(d);
    }

    std::vector< RatPoint_line_segment > t;
    for (size_t i = 1; i < d.get_face_table().size(); ++i)
    {
        std::vector< RatPoint_line_segment > ti = edges_to_triangulate(d, i);
        std::copy(ti.begin(), ti.end(), std::back_inserter(t));
    }
    //std::copy(t.begin(), t.end(), std::back_inserter(d));
    d = d.merge(ctor_from_edge_list(t));

    if (VISUALIZATION > 1)
    {
        std::ofstream f("m3t4s3.svg");
        f << kjb::qd::draw_dcel_as_svg(d);
    }

    return EXIT_SUCCESS;
}


}


int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));

    try
    {
//        if (EXIT_SUCCESS != test3()) return EXIT_FAILURE;
//        if (EXIT_SUCCESS != test2()) return EXIT_FAILURE;
        /*if (EXIT_SUCCESS != test1()) return EXIT_FAILURE;*/
        if (EXIT_SUCCESS != test4()) return EXIT_FAILURE;
    }
    catch(const kjb::Exception& e)
    {
        e.print_details_exit(std::cout, true);
    }

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

