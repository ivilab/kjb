/**
 * @file
 * @brief implementation of class PolyPath
 * @author Andrew Predoehl
 */
/*
 * $Id: polypath.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h" /* For ASSERT */
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "qd_cpp/polypath.h"

#include <algorithm>
#include <iterator>
#include <ostream>


namespace
{

using kjb::qd::PixPath;
using kjb::qd::PixPoint;
using kjb::Vector2;

const char* E_WHISKER = "Input has a whisker bend";
const char* E_DUPLICATE = "Input path has a duplicate point";

Vector2 unit_vector(const PixPoint& a, const PixPoint& b)
{
    if (a != b && a.is_not_unused() && b.is_not_unused())
    {
        return Vector2(b.x - a.x, b.y - a.y) * (1.0 / b.dist(a));
    }
    KJB_THROW_2(kjb::Illegal_argument, "endpoints are not distinct");
}


// Test whether the two points are identical.
// Implemented as a macro b/c KJB_THROW is a macro.
#define NO_DUPLICATES(a, b)                                    \
    do                                                         \
    {                                                          \
        if ((a) == (b))                                        \
        {                                                      \
            KJB_THROW_2(kjb::Illegal_argument, E_DUPLICATE);   \
        }                                                      \
    }                                                          \
    while(0)


PixPath interpolate_and_build_map(
    const PixPath& sp,
    std::vector<size_t>* sdmap
)
{
    NTX(sdmap);
    sdmap -> clear();

    if (0 == sp.size()) return PixPath::reserve();

    PixPath dense_path(PixPath::reserve(sp.size()));
    dense_path.push_back(sp.front());
    sdmap -> push_back(0);

    if (1 == sp.size())
    {
        return dense_path;
    }

    for (PixPath::const_iterator j=sp.begin(), i=j++; j != sp.end(); ++i, ++j)
    {
        NO_DUPLICATES(*i, *j);
        PixPath line_seg(kjb::qd::bresenham_line(*i, *j));
        KJB(ASSERT(line_seg.size() > 0));
        sdmap -> push_back(sdmap -> back() + line_seg.size() - 1);
        dense_path.append_no_overlap(line_seg);
    }

    return dense_path;
}


// precondition:  the sparse path must contain unique elements
void get_tangents(
    const PixPath& sparse,
    const PixPath& dense,
    const std::vector<size_t> sdmap,
    std::vector<Vector2>* out)
{
    NTX(out);
    out -> assign(dense.size(), Vector2(0, 0));

    if (sparse.size() < 2) return;

    size_t  dx = 0; // dense index
    PixPath::const_iterator j = sparse.begin(), i=j++;
    out -> at(dx++) = unit_vector(*i, *j);

    // cursor sx is a sparse index, the index of where point *j is
    for (size_t sx = 1; j != sparse.end(); ++i, ++j, ++sx)
    {
        NO_DUPLICATES(*i, *j);
        const Vector2 u = unit_vector(*i, *j);
        while (dx <= sdmap[sx])
        {
            out -> at(dx++) = u;
        }
    }

    // compute the "tangents" (as defined in the trails paper) at the corners
    for (size_t sx = 1; 1+sx < sdmap.size(); ++sx)
    {
        out -> at (sdmap[sx]) += out -> at(sdmap[1+sx]); // prev + next dir
        const double mag = out -> at(sdmap[sx]).magnitude();
        if (mag <= 0)
        {
            KJB_THROW_2(kjb::Illegal_argument, E_WHISKER);
        }
        out -> at(sdmap[sx]) *= 1.0 / mag;      // normalize to unit length
    }
}

} // anonymous ns



namespace kjb
{
namespace qd
{


/**
 * @brief private ctor takes a sequency of polygonal path vertices
 * @param corners a sequence of pairwise-unique polygonal path vertices
 */
PolyPath::PolyPath(const PixPath& corners)
:   m_sparsepath(corners)
{
    PixPath densepath(interpolate_and_build_map(corners, &sdmap));
    PixPath::swap(densepath);
    get_tangents(m_sparsepath, *this, sdmap, &m_tangent);
}



/**
 * @brief print the inner state of a PolyPath object to a stream
 * @return ERROR iff the object has broken invariants, otherwise NO_ERROR
 */
int PolyPath::debug_print(std::ostream& os) const
{
    const size_t SZ = size();

    if (SZ != m_tangent.size())
    {
        os << "size of dense path does not match size of tangent array.\n";
        return kjb_c::ERROR;
    }

    for (size_t iii = 0, sdix = 0; iii < SZ; ++iii)
    {
        os << '#' << iii << "\t ";

        if (sdix < sdmap.size() && sdmap[sdix] == iii)
        {
            os << "VERTEX";
            ++sdix;
            if (sdix < sdmap.size() && sdmap.at(sdix) <= sdmap.at(sdix-1))
            {
                os << "\nsparse-to-dense map is not strictly increasing.\n";
                std::copy(sdmap.begin(), sdmap.end(),
                                    std::ostream_iterator<size_t>(os, ", "));
                os << '\n';
                return kjb_c::ERROR;
            }
        }
        else
        {
            os << "filler";
        }

        os  << "\t loc=" << operator[](iii).str()
            << "\t tan=(" << m_tangent[iii].x() << ", " << m_tangent[iii].y()
            << ")\n";
    }
    return kjb_c::NO_ERROR;
}


Vector2 get_unit_vector_2x_angle(const Vector2& v)
{
    const double m = v.magnitude();
    if (0 == m)
    {
        KJB_THROW_2(Illegal_argument, "input vector has zero length");
    }
    return get_unit_vector_2x_angle_of_unit_vector(v * 1.0/m);
}


Vector2 get_unit_vector_2x_angle_of_unit_vector(const Vector2& u)
{
    const double    cos_A = u.x(),
                    cos_2A = 2.0 * cos_A * cos_A - 1.0,
                    sin_2A = 2.0 * cos_A * u.y();
    return Vector2(cos_2A, sin_2A);
}


bool is_valid_as_polypath(const PixPath& path, bool throw_failure)
{
    // Search for repeated points.
    for (PixPath::const_iterator i = path.begin(); i != path.end(); )
    {
        const unsigned hit_count = path.hits(*i++);
        KJB(ASSERT(hit_count >= 1)); // this line eventually should replace next
        if (hit_count < 1) KJB_THROW(Cant_happen); // eventually remove this
        if (hit_count != 1)
        {
            if (throw_failure)
            {
                std::ostringstream err;
                err << E_DUPLICATE << " at (" << (i-1)->str()
                    << ") which is hit " << hit_count << " times.";
                KJB_THROW_2(Illegal_argument, err.str());
            }
            return false;
        }
    }

    /* Search for "whisker bends," i.e., three points path[i], path[i+1], and
     * path[i+2] in the sequence, where the points are collinear, but if you
     * draw a line segment from path[i] to path[i+2], the segment does not
     * include point path[i+1].  I call it a whisker bend because it looks like
     * a cat's whisker sticking out from the path.  We do not tolerate these
     * bends in a PolyPath, because there is no nice way to define a tangent at
     * path[i+1].
     */
    for (size_t j = 1; j+1 < path.size(); ++j)
    {
        if (0 == path.bracket_cross_at(j))
        {
            const PixPoint  d1 = path[j] - path[j-1],
                            d2 = path[j] - path[j+1];
            if (d1.x * d2.x >= 0 && d1.y * d2.y >= 0)
            {
                if (throw_failure)
                {
                    std::ostringstream err;
                    err << E_WHISKER << " around index " << j
                        << " comprising points (" << path[j-1].str() << "); ("
                        << path[j].str() << "); (" << path[j+1].str() << ").";
                    KJB_THROW_2(Illegal_argument, err.str());
                }
                return false;
            }
        }
    }

    return true;
}


} // namespace qd
} // namespace kjb

