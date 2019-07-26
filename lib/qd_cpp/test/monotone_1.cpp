/**
 * @file
 * @brief unit test for monotone regularization
 * @author Andy Predoehl
 *
 * Basic test for efficacy of monotone.cpp functions.
 * If you turn on VISUALIZATION it will draw you a picture.
 */
/*
 * $Id: monotone_1.cpp 20139 2015-11-30 05:50:36Z predoehl $
 */

#include <l/l_init.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/triangulate.h>
#include <qd_cpp/svg_dcel.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace
{

const bool VISUALIZATION = false;

kjb::qd::Doubly_connected_edge_list test_polygon()
{
    kjb::qd::PixPath p = kjb::qd::PixPath::reserve(25);
    p.push_back(kjb::qd::PixPoint(19,  9));
    p.push_back(kjb::qd::PixPoint(21, 15));
    p.push_back(kjb::qd::PixPoint(16, 13));
    p.push_back(kjb::qd::PixPoint(15, 23));
    p.push_back(kjb::qd::PixPoint(13, 20));
    p.push_back(kjb::qd::PixPoint(12, 20));
    p.push_back(kjb::qd::PixPoint(11, 21));
    p.push_back(kjb::qd::PixPoint(10, 20));
    p.push_back(kjb::qd::PixPoint( 9, 20));
    p.push_back(kjb::qd::PixPoint( 8, 23));
    p.push_back(kjb::qd::PixPoint( 6, 23));
    p.push_back(kjb::qd::PixPoint( 4, 23));
    p.push_back(kjb::qd::PixPoint( 3, 19));
    p.push_back(kjb::qd::PixPoint( 6, 16));
    p.push_back(kjb::qd::PixPoint( 5, 12));
    p.push_back(kjb::qd::PixPoint( 2, 14));
    p.push_back(kjb::qd::PixPoint( 1, 10));
    p.push_back(kjb::qd::PixPoint( 4,  5));
    p.push_back(kjb::qd::PixPoint( 7,  7));
    p.push_back(kjb::qd::PixPoint( 8,  7));
    p.push_back(kjb::qd::PixPoint( 9,  7));
    p.push_back(kjb::qd::PixPoint(10,  1));
    p.push_back(kjb::qd::PixPoint(12,  1));
    p.push_back(kjb::qd::PixPoint(14,  1));
    p.push_back(kjb::qd::PixPoint(12, 11));
    return kjb::qd::Doubly_connected_edge_list::ctor_closed_path(p);
}



int test1()
{
    kjb::qd::Doubly_connected_edge_list d = test_polygon();

    TEST_FALSE(is_face_ymonotone(d, 1));

    if (VISUALIZATION)
    {
        std::string sd = draw_dcel_as_svg(d);
        const size_t ct = sd.find("<!-- Text ");
        KJB(ASSERT( ct < sd.size() ));
#if MONOTONE_CPP_DEBUG
        sd.insert(ct, db_fence_labels(d, 1, 25));
#endif
        std::ofstream ff("monotone_1a.svg");
        ff << sd;
    }

    return kjb_c::NO_ERROR;
}


int test2()
{
    using kjb::qd::Doubly_connected_edge_list;

    const Doubly_connected_edge_list d = test_polygon(),
                               mon = kjb::qd::make_a_face_ymonotone(d, 1);

    for (size_t i = 1; i < mon.get_face_table().size(); ++i)
    {
        if (kjb_c::is_interactive()) std::cout << "trying face " << i << '\n';
        TEST_TRUE(is_face_ymonotone(mon, i));
    }

    const std::string s = xml_output(d);
    std::istringstream t(s);
    TEST_TRUE(d == Doubly_connected_edge_list::ctor_xml_stream(t));

    if (VISUALIZATION)
    {
        std::string sd = draw_dcel_as_svg(mon);
        std::ofstream ff("monotone_1b.svg");
        ff << sd;
    }
    return kjb_c::NO_ERROR;
}


}

int main(int /*argc*/, char** /*argv*/)
{
    KJB(EPETE(kjb_init()));
    KJB(EPETE(test1()));
    KJB(EPETE(test2()));

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

