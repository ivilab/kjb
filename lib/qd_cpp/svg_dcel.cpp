/**
 * @file
 * @brief implementation for SVG rendering DCEL
 * @author Andrew Predoehl
 */
/*
 * $Id: svg_dcel.cpp 21596 2017-07-30 23:33:36Z kobus $
 */


#include "l/l_sys_debug.h"  /* For ASSERT, includes l_sys_sys.h */
#include "c_cpp/c_palette.h"
#include "qd_cpp/svg_dcel.h"
#include "qd_cpp/svgwrap.h"
#include "qd_cpp/doublecircle.h"
#include <set>


namespace
{
using kjb::qd::SVG_UNCRAMP;

// ===== Size of SVG output box, in pixels =======
const size_t svg_width_px = 500, svg_height_px = svg_width_px;



/* ===== Line width for cycles; set to zero to omit them. =====
 *
 * Units are as a fraction of the radius of the circumscribing circle.
 */
//const float FINELINE = 0.015;
const float FINELINE = 0.005;
//const float FINELINE = 0;



// ===== Optional XML-comment text description of DCEL ======
const bool add_huge_text_dump = true;



inline kjb::Vector2 dbl_pt(const kjb::qd::RatPoint& p)
{
    return kjb::Vector2(kjb::qd::dbl_ratio(p.x), kjb::qd::dbl_ratio(p.y));
}



using kjb::qd::Doubly_connected_edge_list;


/**
 * @brief return list of edge indices of outer components just inside an i.c.
 * @param dcel Planar subdivision under consideration
 * @param ei Edge index of an edge in the DCEL -- should be an inner component
 * @return edge indices (representing cycles) of outer components immediately
 *         inside cycle(ei) -- exactly one edge per cycle.  Order irrelevant.
 *
 * "Immediately inside" means "incident to a vertex of cycle(ei)."
 *
 * If ei is the edge index of an inner component of a face within DCEL dcel,
 * i.e., it surrounds a hole in face(ei), then this routine finds and returns
 * a list (possibly empty) of edges on distinct outer components of faces
 * within the hole.  That means each edge in the output is incident to a
 * vertex on cycle(ei), and is part of an outer-border cycle.  Each such cycle
 * has exactly one edge representative in the output list.  See the
 * documentation of Doubly_connected_edge_list for an explanation of the
 * preceding terminology and context, if necessary.
 *
 * If cycle(ei) is not an inner component, then this silently returns an empty
 * vector.
 */
std::vector<size_t> get_ocs_of_ic(
    const kjb::qd::Doubly_connected_edge_list& dcel,
    size_t ei
)
{
    // output vector, contains edge indices in arbitrary order
    std::vector<size_t> vocs;

    // cycle #s of all outer cycles in DCEL
    std::set<int> socs_all;
    for (size_t i = 1; i < dcel.get_face_table().size(); ++i)
    {
        const size_t e = dcel.get_face_table().at(i).outer_component;
        if (e == ei) return vocs; // input is an outer component, oops.
        socs_all.insert(dcel.get_cycle(e));
    }

    // traverse cycle of ei, look at cycles at each vertex, keep outer comps
    std::set<int> cover;
    const size_t estart = ei;
    do
    {
        const std::vector<size_t> oe(out_edges(dcel,
                                    dcel.get_edge_table().at(ei).origin));
        for (size_t k = 0; k < oe.size(); ++k)
        {
            const int c = dcel.get_cycle(oe.at(k));
            // If it is an outer component touching vertex origin(ei),
            // store the cycle number it in our cover set.
            if (socs_all.end() != socs_all.find(c)) cover.insert(c);
        }
        ei = dcel.get_edge_table().at(ei).next;
    }
    while (ei != estart);

    // Push representative edges into the output vector, one per output cycle.
#if 1
    for (std::set<int>::const_iterator i = cover.begin(); i != cover.end();)
    {
        vocs.push_back(dcel.get_edge_of_cycle(*i++));
    }
#else
    std::transform(cover.begin(), cover.end(), std::back_inserter(vocs),
            boost::bind(&dcel.get_edge_of_cycle)); // not valid code
#endif
    return vocs;
}



// Notice this is recursive; it is a small tweak on depth-first search.
void topo_sort(
    const Doubly_connected_edge_list& dcel,
    size_t face_ix,
    std::vector< bool >* facemark,
    std::vector< size_t >* toposort
)
{
    NTX(facemark);
    NTX(toposort);
    facemark -> at(face_ix) = true;

    const size_t nic
        = dcel.get_face_table().at(face_ix).inner_components.size();
    for (size_t i = 0; i < nic; ++i)
    {
        const size_t eic
            = dcel.get_face_table().at(face_ix).inner_components.at(i);
        const std::vector<size_t> ocs = get_ocs_of_ic(dcel, eic);
        for (size_t j = 0; j < ocs.size(); ++j)
        {
            const size_t fdep = dcel.get_edge_table().at(ocs.at(j)).face;
            if (!facemark -> at(fdep))
            {
                topo_sort(dcel, fdep, facemark, toposort);
            }
        }
    }
    toposort -> push_back(face_ix);
}


/*
 * Return an SVG path element describing a DCEL cycle, specified by a
 * representative edge.  We guaranteed that the first 5 chars are <path
 * and the last two chars are /> and the length is more than 7.
 * So if you want to peel off the last 2 chars and add a stroke or fill
 * or other attribute, that is safe to do.
 *
 * If you really want to fill this, like for a face, then you probably
 * also wish not to fill the holes of the face.  In that case, you might
 * appreciate the helpful optional parameter for_filling_this_face,
 * which returns a slightly different result.  If you set this parameter,
 * you must set it to the desired face index, and 'estart' must be the
 * edge listed in the face's outer_component field.  Then the returned
 * string is slightly different.  First, it will use the "z" directive
 * to connect the first and last vertex, making the string incrementally
 * a bit shorter.
 * Second, and more importantly, if the face has any holes with positive
 * area, the path will be augmented with extra M .. L .... sections that
 * delineate the holes.  If you employ a fill-rule of "evenodd" that
 * means the hole regions will NOT be filled, i.e., the faces in the
 * holes will not be obscured.  That's what I normally want, and I bet
 * it is what you want too.  Consequently you do not need to worry about
 * the order you draw faces:  no two DCEL faces will occlude.
 */
std::string get_svg_path_of_cycle(
    const Doubly_connected_edge_list& dcel,
    size_t estart,
    int for_filling_this_face = -1
)
{
    using kjb::qd::RatPoint;

    std::ostringstream svg;
    const size_t vstart = dcel.get_edge_table().at(estart).origin;
    const std::string path = "<path d='", margin(path.size(), ' ');
    const kjb::Vector2 p0(dbl_pt(dcel.get_vertex_table().at(vstart).location));
    svg << path << "M " << SVG_UNCRAMP*p0.x() << ',' << SVG_UNCRAMP*p0.y();

    /*
     * If you want a path that is nice to fill and has holes, then the
     * estart edge must be the outer_component field of the face.
     * Or use the sentinel value -1 to indicate the path should just
     * trace the border as if for line drawing.
     */
    KJB(ASSERT(
           -1 == for_filling_this_face
        || dcel.get_face_table().at(for_filling_this_face).outer_component
               == estart));
    // Face 0 does not have a valid outer component, though.
    KJB(ASSERT(for_filling_this_face != 0));

    // cycle-scan idiom
    for (size_t ei = dcel.get_edge_table().at(estart).next; ei != estart;
                ei = dcel.get_edge_table().at(ei).next)
    {
        const size_t vi = dcel.get_edge_table().at(ei).origin;
        const kjb::Vector2 pi(dbl_pt(dcel.get_vertex_table().at(vi).location));
        svg << '\n' << margin
            << "L " << SVG_UNCRAMP*pi.x() <<',' << SVG_UNCRAMP*pi.y();
    }
    // Close the cycle using the "z" directive.
    svg << " z";

    if (for_filling_this_face > 0)
    {
        // Install holes, if any.
        for (std::vector<size_t> ics = dcel.get_face_table().at(
                                       for_filling_this_face).inner_components;
             ! ics.empty();
             ics.pop_back())
        {
            // Any inner component that covers no area is not really a hole.
            if (kjb::qd::is_edge_of_stick_figure(dcel, ics.back())) continue;
            // Get hole perimeter, and trim off path-d prefix.
            std::string
                h=get_svg_path_of_cycle(dcel,ics.back()).substr(margin.size());
            h.erase(h.rfind("'/>")); // remove closure (could be more holes)
            svg << '\n' << margin << h; // add hole to face
        }
    }
    svg << "'/>";

    return svg.str();
}


std::string get_svg_text_labeling_vertex_nums(
    const Doubly_connected_edge_list& dcel
)
{
    std::ostringstream svg;
    const size_t nv = dcel.get_vertex_table().size();
    for (size_t vi = 0; vi < nv; ++vi)
    {
        const kjb::Vector2 pi(dbl_pt(dcel.get_vertex_table().at(vi).location));
        svg << "<text x='" << SVG_UNCRAMP*pi.x()
            << "' y='" << SVG_UNCRAMP*pi.y()
            << "'>" << vi << "</text>\n";
    }
    return svg.str();
}


std::string huge_text_dump(
    const Doubly_connected_edge_list& dcel,
    const std::vector<size_t>& toposort
)
{
    const int nc = dcel.get_number_of_cycles();   // number of cycles
    std::ostringstream ooo;

    ooo << "DCEL status:\n" << dcel
        << "\nNumber of (real) cycles: " << nc << "\nCycle table:\n";

    for (int c = 0; c < nc; ++c)
    {
        ooo << "Cycle_" << c << " represented by edge_"
            << dcel.get_edge_of_cycle(c) << '\n';
    }
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        ooo << "Edge_" << i << " is member of cycle_" << dcel.get_cycle(i)
            << '\n';
    }

    // write adjacency list of cycles (based on incident faces)
    ooo << "\n DCEL adjacency:\n";
    for (int i = 0; i < int(dcel.get_number_of_cycles()); ++i)
    {
        const size_t e = dcel.get_edge_of_cycle(i),
                     f = dcel.get_edge_table().at(e).face;
        ooo << "Cycle " << i << " incident to face " << f << '\n';
    }

    // Topo sort of faces -- not sure it is useful for anything.
    // It does not solve the painter's algorithm problem, although at first I
    // thought it would.  The painter's algorithm problem is solved by
    // articulating SVG that leaves unfilled holes in filled faces.
    if (!toposort.empty())
    {
        ooo << "Topological sort of faces:  ";
        std::copy(toposort.begin(), toposort.end(),
                  std::ostream_iterator<size_t>(ooo, " "));
        ooo << '\n';
    }

    return ooo.str();
}



std::string draw_cycles(
    const Doubly_connected_edge_list& dcel,
    const std::string& outer_border_color, ///< html style color string
    const std::string& inner_component_color,
    float line_width
)
{
    const size_t NF = dcel.get_face_table().size();
    std::ostringstream ooo;

    // enumerate outer boundary cycles
    std::set<int> ocs;
    for (size_t f = 1; f < NF; ++f)
    {
        ocs.insert(dcel.get_cycle(
                    dcel.get_face_table().at(f).outer_component));
    }

    // draw cycles:  first inner components, then outer boundaries.
    const int nc = dcel.get_number_of_cycles();   // number of cycles


    // inner components
    ooo << "<!-- BEGIN CYCLES -->\n"
           "<g opacity='0.6' "
           "stroke-width='" << SVG_UNCRAMP*line_width <<"'>\n"
           "<!-- Inner component cycles -->\n"
           "<g fill='none' stroke='" << inner_component_color
        << "' stroke-dasharray='"
        << SVG_UNCRAMP*line_width*2
        << ' ' << SVG_UNCRAMP*line_width*4
        << "' >\n";

    for (int k = 0; k < nc; ++k)
    {
        if (ocs.end() == ocs.find(k))
        {
            ooo << get_svg_path_of_cycle(dcel, dcel.get_edge_of_cycle(k))
                << "\n";
        }
    }

    // outer borders
    ooo << "</g>\n<!-- Outer border cycles -->\n"
           "<g fill='none' stroke='" << outer_border_color
        << "' stroke-dasharray='"
        << SVG_UNCRAMP*line_width*2 << ' ' << SVG_UNCRAMP*line_width*5
        << "' >\n";
    for (int k = 0; k < nc; ++k)
    {
        if (ocs.end() != ocs.find(k))
        {
            ooo << get_svg_path_of_cycle(dcel, dcel.get_edge_of_cycle(k))
                << "\n";
        }
    }

    ooo << "</g>\n</g> <!-- END CYCLES -->\n";

    // Label vertices with numeric index
    ooo << "<!-- BEGIN TEXT -->\n"
           "<g fill='" << outer_border_color << "' "
           "font-family='arial' "
           "font-size='" << SVG_UNCRAMP*line_width*10 << "'"
           " >\n"
        << get_svg_text_labeling_vertex_nums(dcel)
        << "</g> <!-- END TEXT -->\n";

    return ooo.str();
}


std::string svg_setup(
    const kjb::qd::DoubleCircle& circle,
    const std::string& color
)
{
    const size_t layer_count = 5;
    const double growth = 0.02,
                 rplus = circle.radius * (1 + growth*layer_count);

    std::ostringstream ooo;
    ooo << "<?xml version='1.0' ?>\n"
            " <!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' \n"
            " 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n"
            "<svg xmlns='http://www.w3.org/2000/svg'\n"
            " xmlns:xlink='http://www.w3.org/1999/xlink'\n"
            " width='" << svg_width_px << "px'"
            " height='" << svg_height_px << "px'"
            " viewBox='"
        << SVG_UNCRAMP*(circle.center.x()-rplus) << ' '
        << SVG_UNCRAMP*(circle.center.y()-rplus)
        << ' ' << SVG_UNCRAMP*2*rplus << ' ' << SVG_UNCRAMP*2*rplus << "'>\n";

    // draw the big circle -- layers of translucent circles give a fuzzy edge.
    for (size_t k = 0; k < 5; ++k)
    {
        ooo << "<circle cx='" << SVG_UNCRAMP*circle.center.x()
            << "' cy='" << SVG_UNCRAMP*circle.center.y()
            << "' r='" << SVG_UNCRAMP* circle.radius * (1 + k*growth)
            << "' fill='" << color;

        if (k > 0) ooo << "' opacity='0.5";

        ooo << "' />\n";
    }

    return ooo.str();
}



std::vector<size_t> get_toposort_of_faces(
    const Doubly_connected_edge_list& dcel
)
{
    const size_t NF = dcel.get_face_table().size();
    std::vector< bool > facemark(NF, false);
    std::vector< size_t > toposort;
    for (size_t k = 0; k < NF; ++k)
    {
        if (facemark.at(k)) continue;
        topo_sort(dcel, k, &facemark, &toposort);
    }
    std::reverse(toposort.begin(), toposort.end());
    KJB(ASSERT(toposort.size() == NF));
    return toposort;
}


kjb::qd::DoubleCircle circumscribe_dcel(const Doubly_connected_edge_list& dcel)
{
    const size_t NV = dcel.get_vertex_table().size();
    std::vector< kjb::Vector2 > pts(NV);
    for (size_t i = 0; i < NV; ++i)
    {
        pts.at(i) = dbl_pt(dcel.get_vertex_table().at(i).location);
    }
    return kjb::qd::DoubleCircle(pts);
}



}


namespace kjb
{
namespace qd
{


std::string draw_dcel_as_svg(const Doubly_connected_edge_list& dcel)
{
    // palette of colors for faces and cycles
    const Palette pal(2 + dcel.get_face_table().size());

    // topologically sort face dag; get paintable permutation of face indices
    // (update:  no, topo sort does not solve the painter's alg problem.)
    const std::vector< size_t > toposort = get_toposort_of_faces(dcel);

    // Represent index-0 face by a big circle.
    const DoubleCircle big = circumscribe_dcel(dcel);

    // set up output with bounding box around the big, index-0 circle.
    std::ostringstream svg;
    svg << svg_setup(big, pal[0].as_hex_triplet());

    // draw filled polygons
    svg << "<g fill-rule='evenodd'> <!-- BEGIN FACES -->\n";
    for (size_t i = 0; i < toposort.size(); ++i)
    {
        const size_t f = toposort.at(i); // face index
        if (0 == f) continue; // index-0 face is already represented by circle.
        const size_t eoc = dcel.get_face_table().at(f).outer_component;

        svg << "<g fill='" << pal[f].as_hex_triplet() << "'>\n"
               "<!-- face " << f << " -->\n"
            << get_svg_path_of_cycle(dcel, eoc, f) << "</g>\n";
    }
    svg << "</g> <!-- END FACES -->\n";

    // draw cycles
    if (FINELINE > 0)
    {
        const size_t k_in = pal.size() - 1, k_out = k_in - 1;
        svg << draw_cycles(dcel, pal[k_out].as_hex_triplet(),
                           pal[k_in].as_hex_triplet(), big.radius*FINELINE);
    }

    svg << "</svg>\n";

    // Maybe add XML comment describing this DCEL in plain text.
    if (add_huge_text_dump)
    {
        svg << "<!-- " << huge_text_dump(dcel, toposort) << "\n-->\n";
    }

    return svg.str();
}


std::string text_description(const Doubly_connected_edge_list& dcel)
{
    // topologically sort face dag; get paintable permutation of face indices
    return huge_text_dump(dcel, std::vector< size_t >());
}


}
}

