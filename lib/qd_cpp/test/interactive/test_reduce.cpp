/**
 * @file
 * @brief demo the reduce_pixels_pv function
 * @author Andrew Predoehl
 */
/*
 * $Id: test_reduce.cpp 20129 2015-11-24 23:57:03Z predoehl $
 */

#include <l/l_sys_rand.h>
#include <qd_cpp/pathresize.h>
#include <qd_cpp/svgwrap.h>

#include <iostream>

namespace {

int main2()
{
    using kjb_c::kjb_rand;
    using kjb::qd::PixPoint;

    int SIZE = 100, MAG = 3;
    kjb::qd::PixPath p(kjb::qd::PixPath::reserve(1000));
    p.push_back(PixPoint(5 + SIZE*kjb_rand(), 5 + SIZE*kjb_rand()));

    for (int i=0; i<30; ++i)
    {
        p.append_no_overlap(
            kjb::qd::bresenham_line(
                p.back(), 
                PixPoint(5 + SIZE*kjb_rand(), 5 + SIZE*kjb_rand())
            ));
    }

    const kjb::qd::PixPath  r(kjb::qd::reduce_pixels_pv(p)),
                            t(p.cull_redundant_points()),
                            u(kjb::qd::reduce_pixels_bfs(p));

    kjb::qd::SvgWrap sp(p), sr(r), st(t), su(u);
    sp.set_color("blue").set_svg(1,0).set_text(false);
    st.set_xml(false).set_svg(false).set_color("green");
    sr.set_xml(false).set_svg(false);
    su.set_xml(false).set_svg(0,1).set_color("orange");

    // only needed by the one generating the opening SVG tag.
    sp.m_width = sp.m_height = 5 + SIZE + 5;
    sp.m_magnify = MAG;

    std::cout << sp() << '\n' << st() << '\n' << sr() << '\n' << st() << '\n';
    std::cerr <<"<-- reduced size: " << r.size() << " -->\n"
                "<-- culled size: " << t.size() << " -->\n"
                "<-- tree-search size: " << u.size() << " -->\n";

    return 0;
}

}

int main()
{
    try {
        return main2();
    }
    catch (kjb::Exception& e) {
        e.print_details_exit();
    }
}
