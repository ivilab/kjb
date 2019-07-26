/////////////////////////////////////////////////////////////////////////////
// line.h - utils for drawing or traversing lines
// Author: Franklin Antonio -- the integer routine was adapted from
// Graphics Gems, by Doron Tal
// Date last modified: April, 2000

#ifndef _LINE_H
#define _LINE_H

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // INTERFACE:
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // From Graphics Gems.  Modifications by Doron Tal: (a) removed
    // code to compute the intersection itself, just computing the
    // boolean of whether the intersection occurred or not, for speed
    // (b) float line coordinates are handled, no shorts or longs.
    // POSTCONDITION: returns true if the line passing through (x1,
    // y1) and (x2, y2) and the line passing through (x3, y3) and (x4,
    // y4) intersect each other.

    bool LinesIntersect(const int& x1, const int& y1,
                        const int& x2, const int& y2,
                        const int& x3, const int& y3,
                        const int& x4, const int& y4);

    // same as above, but in float
    bool LinesIntersect(const float& x1, const float& y1,
                        const float& x2, const float& y2,
                        const float& x3, const float& y3,
                        const float& x4, const float& y4);

} // namespace DTLib {

#endif /* #ifndef _LINE_H */

// www.homefindersbulletin.com
