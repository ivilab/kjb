/**
 * @file
 * @author Andrew Predoehl
 * @brief demo of some SVG generation tricks
 */
/*
 * $Id: test_svg.cpp 20532 2016-03-12 00:54:48Z predoehl $
 */

#include <qd_cpp/svgwrap.h>
#include <iostream>

int main(int argc, char** argv)
{
    kjb::qd::PixPath p( kjb::qd::PixPath::reserve(4) );
    p.push_back( kjb::qd::PixPoint(50,50) );
    p.push_back( kjb::qd::PixPoint(950,50) );
    p.push_back( kjb::qd::PixPoint(950,950) );
    p.push_back( kjb::qd::PixPoint(50,950) );
    p.push_back( kjb::qd::PixPoint(50,50) );

    kjb::qd::PixPath q( kjb::qd::PixPath::reserve(4) );
    q.push_back( kjb::qd::PixPoint(25,140) );
    q.push_back( kjb::qd::PixPoint(975,140) );
    q.push_back( kjb::qd::PixPoint(975,860) );
    q.push_back( kjb::qd::PixPoint(25,860) );
    q.push_back( kjb::qd::PixPoint(25,140) );

    // crazy nonsimple polygon
    kjb::qd::PixPath oct( kjb::qd::PixPath::reserve() );
    oct.push_back( kjb::qd::PixPoint(700,700) );
    oct.push_back( kjb::qd::PixPoint(550,200) );
    oct.push_back( kjb::qd::PixPoint(360,730) );
    oct.push_back( kjb::qd::PixPoint(410,755) );
    oct.push_back( kjb::qd::PixPoint(800,250) );
    oct.push_back( kjb::qd::PixPoint(260,320) );
    oct.push_back( kjb::qd::PixPoint(220,380) );
    oct.push_back( kjb::qd::PixPoint(570,430) );
    oct.push_back( kjb::qd::PixPoint(400,250) );
    oct.push_back( kjb::qd::PixPoint(250,500) );
    oct.push_back( kjb::qd::PixPoint(750,550) );
    oct.push_back( kjb::qd::PixPoint(700,700) );
    oct.push_back( kjb::qd::PixPoint(550,200) );

    kjb::qd::SvgWrap s(p);
    s.m_width = 1000;
    s.m_height = 1000;
    s.set_text(0).set_color("#c42").set_svg(1,0);
    s.set_path_extra(
            "stroke-width=\"50\" opacity=\"0.6\" "
            "stroke-linecap=\"round\" "
            "stroke-linejoin=\"round\""
        );
    std::cout << s() << '\n';

    kjb::qd::SvgWrap r(q);
    r.set_text(0).set_svg(0,0).set_xml(0);
    r.set_color("salmon").set_fill(1).set_fill_color("tomato");
    r.set_path_extra(
            "stroke-width=\"15\" "
            "stroke-linecap=\"round\" "
            "stroke-linejoin=\"round\""
        );
    std::cout << r() << '\n';

    kjb::qd::SvgWrap z(oct);
    z.set_svg(0).set_xml(0).set_color("blue").set_text(0);
    z.set_fill(1).set_fill_color("firebrick");
    z.set_path_extra(
            "stroke-width=\"27\" opacity=\"0.6\" "
            "stroke-linecap=\"round\" "
            "stroke-linejoin=\"miter\" "
            "fill-rule=\"evenodd\""
        );
    std::cout << z() << '\n';

    // you can copy it
    kjb::qd::SvgWrap w(s);
    w.set_svg(0,1).set_xml(0).set_color("#ee0");
    w.set_path_extra( w.get_path_extra() +
            " stroke-dasharray=\"30,40,50,70,100\"");
    std::cout << w();
    /*
     * About stroke-dasharray above:
     *
     * It means 30 pix filled, 40 empty, 50 filled, 70 empty, 100 filled,
     * 30 empty, 40 filled, 50 empty, 70 filled, 100 empty, repeating that way.
     * Then the end-caps get drawn on it, making the filled pieces bigger.
     */

	std::cout << "<!--\n" << kjb::qd::svg_path(oct) << "\n-->\n";

    return EXIT_SUCCESS;
}
