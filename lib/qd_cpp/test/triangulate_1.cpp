/**
 * @file
 * @brief unit test for triangulation
 * @author Andy Predoehl
 *
 * The full version of this test requires time factor 2 or more.
 * The full version is fairly slow.  In development mode it will take
 * about half an hour.  In production mode it is about five minutes.
 * Those numbers are roughly accurate as of November 2015, but they
 * are liable to change.
 */
/*
 * $Id: triangulate_1.cpp 20176 2015-12-12 20:31:18Z predoehl $
 */

#include <l_cpp/l_test.h>
#include <qd_cpp/triangulate.h>
#include <qd_cpp/svg_dcel.h>
#include <gsl_cpp/gsl_qrng.h>

#include <string>
#include <fstream>
#include <iostream>

namespace
{

using kjb::qd::PixPoint;
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::Doubly_connected_edge_list;
using kjb::qd::is_face_triangle;

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

    for (int i = 0; i < 20; ++i)
    {
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
        vertices.push_back(quasirandom_pt(qq));
    }

    std::random_shuffle(vertices.begin(), vertices.end(), shuffler);

    int i = 0;
    while (vertices.size() >= 3)
    {
        // draw zig-zags in the square
        std::vector< RatPoint > zig;
        zig.push_back(vertices.back());
        vertices.pop_back();
        zig.push_back(vertices.back());
        vertices.pop_back();
        zig.push_back(vertices.back());
        vertices.pop_back();
        d = d.merge(kjb::qd::Doubly_connected_edge_list::ctor_open_path(zig));

        if (VISUALIZATION > 1)
        {
            const std::string sd = draw_dcel_as_svg(d);
            char fn[32];
            snprintf(fn, sizeof(fn), "ztri1_%03d.svg", i++);
            std::ofstream ff(fn);
            ff << sd;
        }
    }
    if (VISUALIZATION > 0)
    {
        std::ofstream ff("ztriangulate1.svg");
        ff << draw_dcel_as_svg(d);
    }
    return d;
}

int test3()
{
    const Doubly_connected_edge_list d = complicated();

    // Slice all faces into triangles.  Skip face zero, of course.
    std::vector< RatPoint_line_segment > e = edges_to_triangulate(d, 1);
    for (size_t i = 2; i < d.get_face_table().size(); ++i)
    {
        const std::vector< RatPoint_line_segment > f=edges_to_triangulate(d,i);
        std::copy(f.begin(), f.end(), std::back_inserter(e));
    }

    // s should be a fully triangulated version of d
    const Doubly_connected_edge_list s = ctor_from_edge_list(e).merge(d);

    if (VISUALIZATION > 0)
    {
        std::ofstream fd("triangulate_131.xml");
        fd << xml_output(d);
        std::ofstream ff("triangulate_131.svg");
        ff << kjb::qd::draw_dcel_as_svg(d);

        std::ofstream fe("triangulate_132.xml");
        fe << xml_output(s);
        std::ofstream fg("triangulate_132.svg");
        fg << kjb::qd::draw_dcel_as_svg(s);
    }

    // Verify all faces are now triangles.  Skip face zero, of course.
    for (size_t i = 1; i < s.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_triangle(s, i));
    }

    // Basically do the same again, behind the scenes, to obtain unity area.
    RatPoint::Rat area = 0;
    for (size_t i = 1; i < d.get_face_table().size(); ++i)
    {
        area += kjb::qd::area_of_face(d, i);
    }
    TEST_TRUE(RatPoint::Rat(1) == area);

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
        std::ofstream f("test2t5.svg");
        f << draw_dcel_as_svg(t5);
    }

    const std::vector< RatPoint_line_segment >
        e = edges_to_triangulate(t5, 1);

    Doubly_connected_edge_list t(t5);
    for (size_t i = 0; i < e.size(); ++i)
    {
        const Doubly_connected_edge_list l(e[i]);
        t = l.merge(t);
    }

    if (VISUALIZATION > 0)
    {
        std::ofstream fd("triangulate_121.xml");
        fd << xml_output(t5);
        std::ofstream ff("triangulate_121.svg");
        ff << kjb::qd::draw_dcel_as_svg(t5);

        std::ofstream fe("triangulate_122.xml");
        fe << xml_output(t);
        std::ofstream fg("triangulate_122.svg");
        fg << kjb::qd::draw_dcel_as_svg(t);
    }

    // Skip face zero, of course.
    for (size_t i = 1; i < t.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_triangle(t, i));
    }

    // Area of triangle is 40 with or without wire intrusion t4
    TEST_TRUE(RatPoint::Rat(40) == kjb::qd::area_of_face(t3, 1));
    TEST_TRUE(RatPoint::Rat(40) == kjb::qd::area_of_face(t5, 1));

    return kjb_c::NO_ERROR;
}



int test1()
{
    // shape looks like north carolina
    kjb::qd::PixPath t1 = kjb::qd::PixPath::reserve(6);
    t1.push_back(PixPoint(2,0));
    t1.push_back(PixPoint(10,0));
    t1.push_back(PixPoint(11,3));
    t1.push_back(PixPoint(9,5));
    t1.push_back(PixPoint(8,7));
    t1.push_back(PixPoint(0,2));

    const Doubly_connected_edge_list
        d = Doubly_connected_edge_list::ctor_closed_path(t1);

    const std::vector< RatPoint_line_segment >
        e = edges_to_triangulate(d, 1);

    Doubly_connected_edge_list t(d);
    for (size_t i = 0; i < e.size(); ++i)
    {
        const Doubly_connected_edge_list l(e[i]);
        t = l.merge(t);
    }

    if (VISUALIZATION > 0)
    {
        std::ofstream fd("triangulate_111.xml");
        fd << xml_output(d);
        std::ofstream ff("triangulate_111.svg");
        ff << kjb::qd::draw_dcel_as_svg(d);

        std::ofstream fe("triangulate_112.xml");
        fe << xml_output(t);
        std::ofstream fg("triangulate_112.svg");
        fg << kjb::qd::draw_dcel_as_svg(t);
    }

    return kjb_c::NO_ERROR;
}


int test4()
{
    const char *in =
        "<?xml version='1.0' ?>"
        "<dcel>"
        "<vertices>"
        "<vertex index=\"0\">"
        "<outedge>0</outedge>"
        "<location>"
        "<x><rational><numerator>697</numerator>"
            "<denominator>2688</denominator></rational></x>"
        "<y><rational><numerator>2239</numerator>"
            "<denominator>2688</denominator></rational></y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"1\">"
        "<outedge>1</outedge>"
        "<location>"
        "<x><rational><numerator>21</numerator>"
            "<denominator>64</denominator></rational></x>"
        "<y><rational><numerator>63</numerator>"
            "<denominator>64</denominator></rational></y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"2\">"
        "<outedge>2</outedge>"
        "<location>"
        "<x><rational><numerator>11</numerator>"
            "<denominator>64</denominator></rational></x>"
        "<y><rational><numerator>57</numerator>"
            "<denominator>64</denominator></rational></y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"3\">"
        "<outedge>4</outedge>"
        "<location>"
        "<x><rational><numerator>1</numerator>"
            "<denominator>16</denominator></rational></x>"
        "<y><rational><numerator>15</numerator>"
            "<denominator>16</denominator></rational></y>"
        "</location>"
        "</vertex>"
        "</vertices>"
        "<edges>"
        "<edge index=\"0\"><origin>0</origin><twin>1</twin>"
            "<incface>1</incface><next>6</next><prev>2</prev></edge>"
        "<edge index=\"1\"><origin>1</origin><twin>0</twin>"
            "<incface>0</incface><next>3</next><prev>7</prev></edge>"
        "<edge index=\"2\"><origin>2</origin><twin>3</twin>"
            "<incface>1</incface><next>0</next><prev>4</prev></edge>"
        "<edge index=\"3\"><origin>0</origin><twin>2</twin>"
            "<incface>0</incface><next>5</next><prev>1</prev></edge>"
        "<edge index=\"4\"><origin>3</origin><twin>5</twin>"
            "<incface>1</incface><next>2</next><prev>6</prev></edge>"
        "<edge index=\"5\"><origin>2</origin><twin>4</twin>"
            "<incface>0</incface><next>7</next><prev>3</prev></edge>"
        "<edge index=\"6\"><origin>1</origin><twin>7</twin>"
            "<incface>1</incface><next>4</next><prev>0</prev></edge>"
        "<edge index=\"7\"><origin>3</origin><twin>6</twin>"
            "<incface>0</incface><next>1</next><prev>5</prev></edge>"
        "</edges>"
        "<faces>"
        "<face index=\"0\">"
            "<icomponents><edgeindex>7</edgeindex></icomponents></face>"
        "<face index=\"1\"><ocomponent><edgeindex>6</edgeindex></ocomponent>"
            "<icomponents></icomponents></face>"
        "</faces>"
        "</dcel>",
        *out =
        "<?xml version='1.0' ?>"
        "<dcel>"
        "<vertices>"
        "<vertex index=\"0\">"
        "<outedge>9</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>21</numerator>"
        "<denominator>64</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>63</numerator>"
        "<denominator>64</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"1\">"
        "<outedge>6</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>11</numerator>"
        "<denominator>64</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>57</numerator>"
        "<denominator>64</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"2\">"
        "<outedge>4</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>697</numerator>"
        "<denominator>2688</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>2239</numerator>"
        "<denominator>2688</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index=\"3\">"
        "<outedge>8</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>16</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>15</numerator>"
        "<denominator>16</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "</vertices>"
        "<edges>"
        "<edge index=\"0\">"
        "<origin>0</origin>"
        "<twin>1</twin>"
        "<incface>1</incface>"
        "<next>5</next>"
        "<prev>3</prev>"
        "</edge>"
        "<edge index=\"1\">"
        "<origin>1</origin>"
        "<twin>0</twin>"
        "<incface>2</incface>"
        "<next>9</next>"
        "<prev>7</prev>"
        "</edge>"
        "<edge index=\"2\">"
        "<origin>0</origin>"
        "<twin>3</twin>"
        "<incface>0</incface>"
        "<next>4</next>"
        "<prev>8</prev>"
        "</edge>"
        "<edge index=\"3\">"
        "<origin>2</origin>"
        "<twin>2</twin>"
        "<incface>1</incface>"
        "<next>0</next>"
        "<prev>5</prev>"
        "</edge>"
        "<edge index=\"4\">"
        "<origin>2</origin>"
        "<twin>5</twin>"
        "<incface>0</incface>"
        "<next>6</next>"
        "<prev>2</prev>"
        "</edge>"
        "<edge index=\"5\">"
        "<origin>1</origin>"
        "<twin>4</twin>"
        "<incface>1</incface>"
        "<next>3</next>"
        "<prev>0</prev>"
        "</edge>"
        "<edge index=\"6\">"
        "<origin>1</origin>"
        "<twin>7</twin>"
        "<incface>0</incface>"
        "<next>8</next>"
        "<prev>4</prev>"
        "</edge>"
        "<edge index=\"7\">"
        "<origin>3</origin>"
        "<twin>6</twin>"
        "<incface>2</incface>"
        "<next>1</next>"
        "<prev>9</prev>"
        "</edge>"
        "<edge index=\"8\">"
        "<origin>3</origin>"
        "<twin>9</twin>"
        "<incface>0</incface>"
        "<next>2</next>"
        "<prev>6</prev>"
        "</edge>"
        "<edge index=\"9\">"
        "<origin>0</origin>"
        "<twin>8</twin>"
        "<incface>2</incface>"
        "<next>7</next>"
        "<prev>1</prev>"
        "</edge>"
        "</edges>"
        "<faces>"
        "<face index=\"0\">"
        "<icomponents>"
        "<edgeindex>8</edgeindex>"
        "</icomponents>"
        "</face>"
        "<face index=\"1\">"
        "<ocomponent><edgeindex>5</edgeindex>"
        "</ocomponent>"
        "<icomponents>"
        "</icomponents>"
        "</face>"
        "<face index=\"2\">"
        "<ocomponent><edgeindex>9</edgeindex>"
        "</ocomponent>"
        "<icomponents>"
        "</icomponents>"
        "</face>"
        "</faces>"
        "</dcel>";

    std::istringstream stin(in), sto(out);
    const Doubly_connected_edge_list
        din(Doubly_connected_edge_list::ctor_xml_stream(stin)),
        doref(Doubly_connected_edge_list::ctor_xml_stream(sto));

    const std::vector< RatPoint_line_segment > e=edges_to_triangulate(din, 1);
    TEST_TRUE(1 == e.size());
    const Doubly_connected_edge_list dout = din.merge(e.front());

    for (size_t j = 1; j < dout.get_face_table().size(); ++j)
    {
        TEST_TRUE(is_face_triangle(dout, j));
    }

    TEST_TRUE(is_isomorphic(dout, doref, 00));

    return kjb_c::NO_ERROR;
}



int test5()
{
    kjb::qd::PixPath t1 = kjb::qd::PixPath::reserve(5), t2(t1);
    t1.push_back(PixPoint(2,-2));
    t1.push_back(PixPoint(0,3));
    t1.push_back(PixPoint(-2,-2));
    t1.push_back(PixPoint(3,1));
    t1.push_back(PixPoint(-3,1));
    t2.push_back(t1[0]);
    t2.push_back(t1[3]);
    t2.push_back(t1[1]);
    t2.push_back(t1[4]);
    t2.push_back(t1[2]);
    Doubly_connected_edge_list
        t3 = Doubly_connected_edge_list::ctor_closed_path(t1),
        t4 = Doubly_connected_edge_list::ctor_closed_path(t2);
    std::vector< RatPoint::Rat > x;
    x.push_back(RatPoint::Rat(4));
    x.push_back(RatPoint::Rat(-3));
    x.push_back(RatPoint::Rat(0));
    x.push_back(RatPoint::Rat(3));
    x.push_back(RatPoint::Rat(4));
    x.push_back(RatPoint::Rat(0));
    x.push_back(RatPoint::Rat(0));
    x.push_back(RatPoint::Rat(0));
    x.push_back(RatPoint::Rat(1));
    t4.transform(x);
    Doubly_connected_edge_list t5(t4);
    t5.transform(x);
    const Doubly_connected_edge_list
        t6(t4.merge(t5)),
        t7(t6.merge(t3));
    /*
    std::ofstream f("foo.svg");
    f << draw_dcel_as_svg(t7);
    */

    std::vector< RatPoint_line_segment > e = edges_to_triangulate(t7, 1);
    for (size_t i = 2; i < t7.get_face_table().size(); ++i)
    {
        const std::vector< RatPoint_line_segment >
            g = edges_to_triangulate(t7, i);
        std::copy(g.begin(), g.end(), std::back_inserter(e));
    }

    const Doubly_connected_edge_list t8 = ctor_from_edge_list(e).merge(t7);
    for (size_t i = 1; i < t8.get_face_table().size(); ++i)
    {
        TEST_TRUE(is_face_triangle(t8, i));
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
        KJB(EPETE(test1()));
        KJB(EPETE(test2()));
        if (time_factor > 1) KJB(EPETE(test3()));
        KJB(EPETE(test4()));
        KJB(EPETE(test5()));
    }
    catch(const kjb::Exception& e)
    {
        e.print_details_exit(std::cout, true);
    }

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

