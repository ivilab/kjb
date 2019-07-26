/**
 * @file
 * @brief Render DCEL as a SVG
 * @author Andrew Predoehl
 */
/*
 * $Id: svg_dcel.h 20074 2015-11-13 06:39:29Z predoehl $
 */

#ifndef QD_CPP_SVG_DCEL_H_INCLUDED_IVILAB
#define QD_CPP_SVG_DCEL_H_INCLUDED_IVILAB 1

#include <qd_cpp/pixpath.h>
#include <qd_cpp/dcel.h>

namespace kjb
{
namespace qd
{

/*
 * Not all SVG interpreters (such as ImageMagick, Firefox, Eye-of-Gnome)
 * cope linearly with all scaling factors.  Specifically, the fonts can be bad
 * if the font size is fractional.  If we scale up all coordinates by 1000,
 * the net result appears the same except that the text is also legible.
 * If you don't believe me, reduce this factor to 1 and see the effect.
 */
const double SVG_UNCRAMP = 1000;


std::string draw_dcel_as_svg(const Doubly_connected_edge_list&);

std::string text_description(const Doubly_connected_edge_list&);

}
}

#endif
