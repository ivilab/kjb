/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test for line-segment-intersection code
 *
 * This is a test of the implementation of the Bentley-Ottmann algorithm for
 * enumerating all the intersections within a set of line segments.
 * Note this is a relatively high-level algorithm, at least when compared to
 * the low-level predicates for testing whether just a pair of segments
 * intersect or not.  That test is not here, it is in
 * qd_cpp/test/test_intersection.cpp,
 */
/*
 * $Id: test_intersect.cpp 18035 2014-11-03 05:58:09Z predoehl $
 */

#include <l_cpp/l_test.h>
#include <qd_cpp/intersection.h>
#include <qd_cpp/svgwrap.h>

#include <vector>

namespace
{

using kjb::qd::PixPath;
using kjb::qd::PixPoint;

std::vector< std::pair<size_t, size_t> > naive_intersections(const PixPath& p)
{
    std::vector< std::vector<bool> > grid(p.size(),
                                            std::vector<bool>(p.size(), 0));

    for (size_t i=2; i < p.size(); ++i)
    {
        // consecutive segments can intersect under certain conditions
        const kjb::qd::PixPoint_line_segment a(p[i-2],p[i-1]), b(p[i-1],p[i]);
        if  ( true // just mark all intersections, including shared endpoints
            /*   is_degenerate(a)
            ||  is_degenerate(b)
            ||  is_on(a, p[i])
            ||  is_on(b, p[i-2]) */
            )
        {
            grid.at(i-2).at(i-1) = true;
        }
        for (size_t j=0; j < i-2; ++j)
        {
            if (p.intersect_at_with(j, i-1))
            {
                grid.at(j).at(i-1) = true;
            }
        }
    }

    std::vector< std::pair<size_t, size_t> > output;
    for (size_t j = 0; j < p.size(); ++j)
    {
        for (size_t k = 1; k < p.size(); ++k)
        {
            if (grid.at(j).at(k))
            {
                output.push_back( std::make_pair(j, k) );
            }
        }
    }
    return output;
}

int test_a_path(const PixPath& p)
{
    kjb::qd::SvgWrap sempty(PixPath::reserve());
    sempty.set_svg(1,0);
    kjb::qd::SvgWrap s(p);
    s.set_xml(0).set_svg(0,1);

    std::cout << sempty()
            << "\n<rect x=\"0\" y=\"0\" width=\"2000\" "
               "height=\"2000\" fill=\"white\"/>\n"
           << s() << "\n<!--\n";

    const std::vector< std::pair<size_t, size_t> >
        naive(naive_intersections(p)),
        smart(kjb::qd::get_intersections(p, false));

    if (kjb_c::is_interactive())
    {
        std::cout << "\n=============\nNaive:\n";
        for (size_t i=0; i < naive.size(); ++i)
        {
            std::cout << naive.at(i).first <<", "<< naive.at(i).second <<'\n';
        }

        std::cout << "\nSmart:\n";
        for (size_t i=0; i < smart.size(); ++i)
        {
            std::cout << smart.at(i).first <<", "<< smart.at(i).second <<'\n';
        }

        std::cout << "\n-->\n";
    }
    std::cout << std::endl;

    TEST_TRUE(naive.size() == smart.size());
    for (size_t i = 0; i < naive.size(); ++i)
    {
        TEST_TRUE(naive.at(i).first == smart.at(i).first);
        TEST_TRUE(naive.at(i).second == smart.at(i).second);
    }

    return kjb_c::NO_ERROR;
}


PixPath mirror_lr(const PixPath& p)
{
    if (0 == p.size()) return p;

    PixPath q = PixPath::reserve(p.size());
    PixPoint left = p.front(), right = left;
    for (PixPath::const_iterator i = p.begin(); i != p.end(); ++i)
    {
        if (i -> x < left.x) left = *i;
        if (i -> x > right.x) right = *i;
    }

    for (PixPath::const_iterator i = p.begin(); i != p.end(); ++i)
    {
        q.push_back(PixPoint(left.x + right.x - i -> x, i -> y));
    }
    return q;
}


PixPath mirror_ud(const PixPath& p)
{
    if (0 == p.size()) return p;

    PixPath q = PixPath::reserve(p.size());
    PixPoint top = p.front(), bottom = top;
    for (PixPath::const_iterator i = p.begin(); i != p.end(); ++i)
    {
        if (i -> y < bottom.y) bottom = *i;
        if (i -> y > top.y) top = *i;
    }

    for (PixPath::const_iterator i = p.begin(); i != p.end(); ++i)
    {
        q.push_back(PixPoint(i -> x, bottom.y + top.y - i -> y));
    }
    return q;
}


int test_framework(const PixPath& p)
{
    KJB(ERE(test_a_path(p)));
    KJB(ERE(test_a_path(mirror_lr(p))));
    KJB(ERE(test_a_path(mirror_ud(p))));
    KJB(ERE(test_a_path(mirror_ud(mirror_lr(p)))));
    return kjb_c::NO_ERROR;
}


int test1()
{
    PixPath p = PixPath::reserve(9);

    p.push_back(PixPoint(500, 500));
    p.push_back(PixPoint(200, 600));
    p.push_back(PixPoint(700, 600));
    p.push_back(PixPoint(300, 1100));
    p.push_back(PixPoint(700, 700));
    p.push_back(PixPoint(500, 1100));
    p.push_back(PixPoint(100, 300));
    p.push_back(PixPoint(700, 100));
    p.push_back(PixPoint(100, 500));

    return test_framework(p);
}


int test2()
{
    PixPath p = PixPath::reserve();

    p.push_back(PixPoint(500, 500));
    p.push_back(PixPoint(100, 100));
    p.push_back(PixPoint(100, 500));
    p.push_back(PixPoint(500, 100));
    p.push_back(PixPoint(500, 300));
    p.push_back(PixPoint(300, 300));
    p.push_back(PixPoint(300, 500));
    p.push_back(PixPoint(200, 300));
    p.push_back(PixPoint(600, 300));
    p.push_back(PixPoint(300, 100));
    p.push_back(PixPoint(300, 500));
    p.push_back(PixPoint(100, 300));
    p.push_back(PixPoint(300, 300));

    return test_framework(p);
}



int test3()
{
    PixPath p = PixPath::reserve(5);

    p.push_back(PixPoint(500, 500));
    p.push_back(PixPoint(300, 100));
    p.push_back(PixPoint(100, 500));
    p.push_back(PixPoint(300, 300));
    p.push_back(PixPoint(500, 500));

    return test_framework(p);
}


int test4()
{
    PixPath p = PixPath::reserve(5);

    p.push_back(PixPoint(500, 400));
    p.push_back(PixPoint(100, 100));
    p.push_back(PixPoint(100, 500));
    p.push_back(PixPoint(100, 300));
    p.push_back(PixPoint(500, 500));

    return test_framework(p);
}

int test5()
{
    PixPath p = PixPath::reserve(6);

    p.push_back(PixPoint(100, 100));
    p.push_back(PixPoint(500, 500));
    p.push_back(PixPoint(250, 300));
    p.push_back(PixPoint(200, 300));
    p.push_back(PixPoint(510, 500));
    p.push_back(PixPoint(150, 300));

    return test_framework(p);
}


}

int main(int argc, char** argv)
{
    try
    {
        KJB(EPETE(test1()));
        KJB(EPETE(test2()));
        KJB(EPETE(test3()));
        KJB(EPETE(test4()));
        KJB(EPETE(test5()));
    }
    catch (kjb::Exception& e)
    {
        e.print_details_exit();
    }

    return EXIT_SUCCESS;
}
