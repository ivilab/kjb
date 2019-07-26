
/* $Id: m2_spline.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l_cpp/l_exception.h"
#include "m2_cpp/m2_spline.h"
#include "g_cpp/g_quaternion.h"

#include <vector>
#include <new>
#include <iostream>



using std::bad_alloc;
using std::vector;

namespace kjb
{

Nurbs_curve::Nurbs_curve() :
    _order(0),
    _num_knots(0),
    _knots(0),
    _ctl_points()
{
}

Nurbs_curve::Nurbs_curve(uint num_knots, const float* knots,
        uint degree, const std::vector<Vector>& ctl_points) :
    Spline_curve(),
    _order(degree + 1),
    _num_knots(num_knots),
    _knots(0),
    _ctl_points()
{
    if(_num_knots <= _order)
    {
        KJB_THROW_2(Illegal_argument, "Invalid knot count; must be at least (order + 1).");
    }

    unsigned int num_ctl_points = _num_knots - _order;

    if(num_ctl_points != ctl_points.size())
    {
        KJB_THROW_2(Illegal_argument, 
            // Kyle, I just didn't understand this msg:
                // "Invalid control point count; Must be |knots| - order."
            // Rephrasing: 
                "Vector of control points is the wrong length:  "
                "(length + degree + 1) must equal the number of knots."
            // -- Andrew (pls. delete the above interstitial comments someday)
            );
    }

    _knots = new float[_num_knots];
    memcpy(_knots, knots, sizeof(float) * _num_knots);

    // normalize knots
    for(unsigned int i = 0; i < _num_knots; i++)
    {
        _knots[i] /= knots[_num_knots-1];
    }

    // this should do a deep copy (check this)
    _ctl_points = ctl_points;
}

Nurbs_curve::Nurbs_curve(const Nurbs_curve& src) :
    Spline_curve(src),
    _order(src._order),
    _num_knots(src._num_knots),
    _knots(0),
    _ctl_points(src._ctl_points)
{
    _knots = new float[_num_knots];
    memcpy(_knots, src._knots, sizeof(float) * _num_knots);
}

Nurbs_curve::~Nurbs_curve()
{
    delete[] _knots;
}

void Nurbs_curve::insert_at(int /*index*/, const Vector& /*pt*/)
{
    // TODO implement
    KJB_THROW(Not_implemented);
}

Nurbs_curve& Nurbs_curve::operator=(const Nurbs_curve& src)
{
    if (this == &src) {
        return *this;
    }

    delete[] _knots;

    _order = src._order;
    _num_knots = src._num_knots;
    _knots = new float[_num_knots];
    memcpy(_knots, src._knots, sizeof(float) * _num_knots);

    _ctl_points = src._ctl_points;

    return *this;
}

Nurbs_curve* Nurbs_curve::clone() const
{
    return new Nurbs_curve(*this);
}

void Polybezier_curve::decode_control_point(size_t index, size_t& curve_i, Point_type& point_type)
{
    curve_i = (index + 1)/3;
    point_type = static_cast<Point_type>(index % 3);
}

size_t Polybezier_curve::encode_control_point(size_t curve_i, Point_type point_type)
{
    // first handle point isn't a control point
    if(curve_i == 0 && point_type == HANDLE_1_POINT)
        KJB_THROW(kjb::Index_out_of_bounds);

    size_t index = curve_i * 3 + point_type;
    if(point_type == HANDLE_1_POINT)
        index -= 3;
    return index;
}

/**
 * Evaluate the curve at some point along its length.
 *
 * @param u The position along the curve the evaluate.  Valid values are in [0.1].
 *
 * TODO: test non-uniform weights case
 */
Vector Nurbs_curve::evaluate(double u) const
{
    const int DIMENSION = _ctl_points[0].get_length();

    if(this->_ctl_points.size() == 0)
        KJB_THROW_2(kjb::Runtime_error, "Cannot evaluate empty curve.");

    Vector result(DIMENSION, 0.0);

    if(u < 0 || u > 1) KJB_THROW_2(Illegal_argument, "Nurbs_curve::evaluate(): u must be in [0, 1]");

    for(unsigned int i = 0; i < _ctl_points.size(); i++)
    {
        int DEGREE = _order - 1;

        result += _ctl_points[i] * _N(i, DEGREE, u);
    }

    return result;
}

/**
 * Evaluate the curve at some point along its length.
 *
 * @param u The position along the curve the evaluate.  Valid values are in [0.1].
 */
Vector Nurbs_curve::gradient(double u) const
{
    const int DIMENSION = _ctl_points[0].get_length();

    Vector grad(DIMENSION, 0.0);

    if(u < 0 || u > 1) KJB_THROW_2(Illegal_argument, "Nurbs_curve::evaluate(): u must be in [0, 1]");

    // TODO: handle end-points for real.  
    // this is a HACK
    u = CLAMP(u, 0.0, 0.999);

    for(unsigned int i = 0; i < _ctl_points.size(); i++)
    {
        int DEGREE = _order - 1;

        double dN = _dN(i, DEGREE, u);
        grad += _ctl_points[i] * dN;
    }

    return grad;
}

/**
 * Returns true if the curve has zero length, i.e. all control points are identical.  Utility method, used in gradient_*() method of Nurbs_surface
 */
bool Nurbs_curve::has_zero_length() const
{
    if(_ctl_points.size() <= 1) return true;

    const Vector& p = _ctl_points[0];

    for(size_t i = 1; i < _ctl_points.size(); i++)
    {
        const Vector& p2 = _ctl_points[i];
        if(p != p2) return false;
    }

    return true;
}
/**
 * derivative of B-spline basis function.
 *
 * Running time is linear in d
 *
 * @param k knot index
 * @param d Polynomial degree of basis function
 * @param u Position along curve to evaluate (in [0,1))
 */
double Nurbs_curve::_dN(int k, int d, double u) const
{
    if(d == 0)
    {
        return 0;
    }

    float n1 = _N(k, d - 1, u);
    float n2 = _N(k+1, d - 1, u);

    float dn1 = _dN(k, d - 1, u);
    float dn2 = _dN(k+1, d - 1, u);

    ASSERT(k + d + 1 < (int) _num_knots);

    float term1_1;
    float term1_2;
    if(n1 == 0)
    {
        term1_1 = 0;
    }
    else
    {
        term1_1 = (1)/(_knots[k+d] - _knots[k]) * n1;
    }

    if(dn1 == 0)
    {
        term1_2 = 0;
    }
    else
    {
        term1_2 =  (u - _knots[k])/(_knots[k+d] - _knots[k]) * dn1;
    }

    float term2_1;
    float term2_2;
    if(n2 == 0)
    {
        term2_1 = 0;
    }
    else
    {
        term2_1 = (- 1)/(_knots[k + d + 1] - _knots[k + 1]) * n2;
    }

    if(dn2 == 0)
    {
        term2_2 = 0;
    }
    else
    {
        term2_2 = (_knots[k + d + 1]- u)/(_knots[k + d + 1] - _knots[k + 1]) * dn2;
    }

    return term1_1 + term1_2 + term2_1 + term2_2;

}

/**
 * B-spline basis function.
 *
 * Running time is linear in d
 *
 * @param k knot index
 * @param d Polynomial degree of basis function
 * @param u Position along curve to evaluate (in [0,1))
 */
double Nurbs_curve::_N(int k, int d, double u) const
{
#ifndef NDEBUG
    if(k < 0 || k >= (int) _num_knots)
    {
        KJB_THROW_2(Illegal_argument, "_N: knot index out of bounds.");
    }

    if(d < 0)
    {
        KJB_THROW_2(Illegal_argument, "_N: degree must be >= 0.");
    }

    if(u < 0 || u > 1)
    {
        KJB_THROW_2(Illegal_argument, "_N: t must be in [0,1].");
    }
#endif

    if(d == 0)
    {
        if(_knots[k] <= u && u < _knots[k+1])
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    float n1 = _N(k, d - 1, u);
    float n2 = _N(k+1, d - 1, u);

    ASSERT(k + d + 1 < (int) _num_knots);
    float term1 = (n1 == 0 ? n1 : (u - _knots[k])/(_knots[k+d] - _knots[k]) * n1);
    float term2 = (n2 == 0 ? n2 : (_knots[k + d + 1] - u)/(_knots[k + d + 1] - _knots[k + 1]) * n2);

    return term1 + term2;
}


Nurbs_surface::Nurbs_surface() :
    _order_s(0),
    _order_t(0),
    _num_knots_s(0),
    _num_knots_t(0),
    _knots_s(0),
    _knots_t(0),
    _ctl_points()
{
}

Nurbs_surface::Nurbs_surface(
        uint num_knots_s,
        const float* knots_s,
        uint num_knots_t,
        const float* knots_t,
        uint degree_s,
        uint degree_t,
        const std::vector<std::vector<Vector> >& ctl_points) : 
    Spline_surface(),
    _order_s(degree_s + 1),
    _order_t(degree_t + 1),
    _num_knots_s(num_knots_s),
    _num_knots_t(num_knots_t),
    _knots_s(0),
    _knots_t(0),
    _ctl_points()
{
    if(_num_knots_s <= _order_s || _num_knots_t <= _order_t)
    {
        KJB_THROW_2(Illegal_argument, "Invalid knot count; must be at least (order + 1).");
    }

    unsigned int num_ctl_points_s = _num_knots_s - _order_s;
    unsigned int num_ctl_points_t = _num_knots_t - _order_t;

    if(num_ctl_points_s != ctl_points.size() ||
        num_ctl_points_t != ctl_points[0].size() )
    {
        KJB_THROW_2(Illegal_argument, 
                "Vector of control points is the wrong length:  "
                "(length + degree + 1) must equal the number of knots.");
    }

    _knots_s = new float[_num_knots_s];
    memcpy(_knots_s, knots_s, sizeof(float) * _num_knots_s);

    // normalize knots
    for(uint i = 0; i < _num_knots_s; i++)
    {
        _knots_s[i] /= knots_s[_num_knots_s-1];
    }

    _knots_t = new float[_num_knots_t];
    memcpy(_knots_t, knots_t, sizeof(float) * _num_knots_t);

    // normalize knots
    for(uint i = 0; i < _num_knots_t; i++)
    {
        _knots_t[i] /= knots_t[_num_knots_t-1];
    }
    
    _ctl_points = ctl_points;
}

Nurbs_surface::Nurbs_surface(const Nurbs_surface& src) :
    Spline_surface(src),
    _order_s(src._order_s),
    _order_t(src._order_t),
    _num_knots_s(src._num_knots_s),
    _num_knots_t(src._num_knots_t),
    _knots_s(0),
    _knots_t(0),
    _ctl_points(src._ctl_points)
{
    _knots_s = new float[_num_knots_s];
    memcpy(_knots_s, src._knots_s, sizeof(float) * _num_knots_s);

    _knots_t = new float[_num_knots_t];
    memcpy(_knots_t, src._knots_t, sizeof(float) * _num_knots_t);
}

Nurbs_surface::~Nurbs_surface()
{
    delete[] _knots_s;
    delete[] _knots_t;
}

Nurbs_surface& Nurbs_surface::operator=(const Nurbs_surface& src)
{
    if (this == &src) {
        return *this;
    }

    delete[] _knots_s;
    delete[] _knots_t;

    _order_s = src._order_s;
    _order_t = src._order_t;
    _num_knots_s = src._num_knots_s;
    _num_knots_t = src._num_knots_t;
    _knots_s = new float[_num_knots_s];
    memcpy(_knots_s, src._knots_s, sizeof(float) * _num_knots_s);
    _knots_t = new float[_num_knots_t];
    memcpy(_knots_t, src._knots_t, sizeof(float) * _num_knots_t);

    _ctl_points = src._ctl_points;

    return *this;
}

Nurbs_surface* Nurbs_surface::clone() const
{
    return new Nurbs_surface(*this);
}

/**
 * Returns the gradient in the s direction at a point on the surface
 */
Vector Nurbs_surface::gradient_s(double s, double t) const
{
    // TODO:  Merge shared code with gradient_t
    if(s < 0.0 || t < 0.0 || s > 1.0 || t > 1.0)
    {
        KJB_THROW_2(Illegal_argument, "Surface parameters must be in [0,1].");
    }


    // evaluate each of the curves in the t direction.  use result points to
    // create a new nurbs curve and evaluate gradient of that in the s direction

    // TODO caching of intermediate values

    // build nurbs curve from control points 
    int num_s = _ctl_points.size();
    // int num_t = _ctl_points[0].size();
    int dimension = _ctl_points[0][0].get_length();

    vector<Vector> curve_ctl_pts(num_s, Vector(dimension));

    for(int i = 0; i < num_s; i++)
    {
        Nurbs_curve curve(_num_knots_t, _knots_t, _order_t-1, _ctl_points[i]);
        curve_ctl_pts[i] = curve.evaluate(t);
    }

    Nurbs_curve curve(_num_knots_s, _knots_s, _order_s-1, curve_ctl_pts);

    // in situations where all control points converge to a singularity, 
    // just perturb slightly and re-compute.
    // HACK! 
    // TODO: Think about throwing an exception in this case.
    if(curve.has_zero_length())
    {
        std::cout << "ZERO-LENGTH HACK" << std::endl;
        const double DELTA = 0.01;
        if(s == 0)
            return gradient_s(s + DELTA, t);
        else
            return gradient_s(s - DELTA, t);
    }

    return curve.gradient(s);
}

/**
 * Returns the gradient in the t direction at a point on the surface
 */
Vector Nurbs_surface::gradient_t(double s, double t) const
{
    // TODO:  Merge shared code with gradient_s
    if(s < 0.0 || t < 0.0 || s > 1.0 || t > 1.0)
    {
        KJB_THROW_2(Illegal_argument, "Surface parameters must be in [0,1].");
    }


    // evaluate each of the curves in the t direction.  use result points to
    // create a new nurbs curve and evaluate gradient of that in the s direction

    // TODO caching of intermediate values

    // build nurbs curve from control points 
    int num_t = _ctl_points[0].size();
    // int num_t = _ctl_points[0].size();
    int dimension = _ctl_points[0][0].get_length();

    vector<Vector> curve_ctl_pts(num_t, Vector(dimension));

    for(int i = 0; i < num_t; i++)
    {
        // build vector of control points for an "s" curve
        int num_s = _ctl_points.size();
        vector<kjb::Vector> s_ctl_points(num_s);
        for(int j = 0; j < num_s; j++)
        {
            s_ctl_points[j] = _ctl_points[j][i];
        }

        Nurbs_curve curve(_num_knots_s, _knots_s, _order_s-1, s_ctl_points);
        curve_ctl_pts[i] = curve.evaluate(s);
    }


    Nurbs_curve curve(_num_knots_t, _knots_t, _order_t-1, curve_ctl_pts);

    // in situations where all control points converge to a singularity, 
    // just perturb slightly and re-compute.
    // HACK!  
    // TODO: Think about throwing an exception in this case. or make delta adjustable
    if(curve.has_zero_length())
    {
        const double DELTA = 0.01;
        if(s == 0)
            return gradient_t(s + DELTA, t);
        else
            return gradient_t(s - DELTA, t);
    }

    return curve.gradient(t);
}

/**
 * @return the surface normal at a point on the surface*/
Vector Nurbs_surface::normal(double s, double t) const
{
    Vector ds = gradient_s(s, t).normalize();
    ASSERT(fabs(ds.magnitude() - 1.0) < FLT_EPSILON);

    Vector dt = gradient_t(s, t).normalize();
    ASSERT(fabs(dt.magnitude() - 1.0) < FLT_EPSILON);

    return cross(ds, dt).normalize();
}

Vector Nurbs_surface::evaluate(double s, double t) const
{
    // evaluate each of the curves in the s direction.  use result points to
    // create a new nurbs curve and evaluate that in the t direction

    // TODO caching of intermediate values

    // build nurbs curve from control points 
    int num_s = _ctl_points.size();
    // int num_t = _ctl_points[0].size();
    int dimension = _ctl_points[0][0].get_length();

    vector<Vector> curve_ctl_pts(num_s, Vector(dimension));

    for(int i = 0; i < num_s; i++)
    {
        Nurbs_curve curve(_num_knots_t, _knots_t, _order_t-1, _ctl_points[i]);
        curve_ctl_pts[i] = curve.evaluate(t);
    }

    Nurbs_curve curve(_num_knots_s, _knots_s, _order_s-1, curve_ctl_pts);

    return curve.evaluate(s);
    
}

void Nurbs_surface::_print() const
{
        {
            for(uint row = 0; row < _ctl_points.size(); row++)
            {
                for(uint col = 0; col < _ctl_points[0].size(); col++)
                {
                    std::cout << _ctl_points[row][col] << "\t";
                }
                std::cout << std::endl;
            }
        }
}



Bezier_curve::Bezier_curve(int degree, int dimension) :
    Spline_curve(),
    _ctl_points(degree + 1)
{
    for(int i = 0; i < degree + 1; i++)
    {
        _ctl_points[i] = Vector(dimension, 0.0);
    }
}

Bezier_curve::Bezier_curve(const Bezier_curve& src) :
    Spline_curve(src),
    _ctl_points(src._ctl_points)
{}

Bezier_curve& Bezier_curve::operator=(const Bezier_curve& src)
{
    _ctl_points = src._ctl_points;
    return *this;
}

Bezier_curve* Bezier_curve::clone() const
{
    return new Bezier_curve(*this);
}

Vector Bezier_curve::evaluate(double u) const
{
    // TODO directly implement Bezier evaluator
    //
    // The nurbs evalutor is a tested reference implementation, but later we should re-implement the bezier evaluator directly.
    return to_nurbs().evaluate(u);
}

Vector Bezier_curve::gradient(double u) const
{
    // TODO directly implement Bezier evaluator
    //
    // The nurbs evalutor is a tested reference implementation, but later we should re-implement the bezier evaluator directly.
    return to_nurbs().gradient(u);
}


void Bezier_curve::set_control_point(int index, const Vector& pt)
{
    if(index < 0 || index >= 4)
    {
        KJB_THROW_2(Illegal_argument, "Cubic_bezier_curve::set_control_point(): index must be in [0,3]");
    }

    if(pt.get_length() != _ctl_points[0].get_length())
    {
        KJB_THROW_2(Illegal_argument, "Cubic_bezier_curve::set_control_point(): control point dimension doesn't match spline's dimension.");
    }

    _ctl_points[index] = pt;
}

const Vector& Bezier_curve::get_control_point(int index) const
{
    return _ctl_points.at(index);
}

Nurbs_curve Bezier_curve::to_nurbs() const
{
    const int ORDER = _ctl_points.size();
    const int NUM_KNOTS = ORDER * 2;
    float* knots = new float[NUM_KNOTS];

    for(int i = 0; i < ORDER; i++)
    {
        knots[i] = 0;
    }

    for(int i = ORDER; i < 2 * ORDER; i++)
    {
        knots[i] = 1;
    }

    return Nurbs_curve(NUM_KNOTS, knots, ORDER-1, _ctl_points);
}


Polybezier_curve::Polybezier_curve(int dimension) : 
    Spline_curve(),
    _curve_points(),
    _handle_1_points(),
    _handle_2_points(),
    _dimension(dimension)
{ }

Polybezier_curve::Polybezier_curve(const std::vector<kjb::Vector>& control_points) :
    Spline_curve(),
    _curve_points(),
    _handle_1_points(),
    _handle_2_points(),
    _dimension()
{
    if(control_points.size() == 0)
        return;
    if(control_points.size() == 1)
    {
        const kjb::Vector& p = control_points[0];
        _dimension = p.size();
        _curve_points.resize(1, p);
        _handle_1_points.resize(1, p);
        _handle_2_points.resize(1, p);
        return;
    }

    if((control_points.size()+2) % 3 != 0)
        KJB_THROW_2(kjb::Illegal_argument, "Polybezier_curve: invalid number of control points");

    size_t num_curve_pts = (control_points.size() + 2)/3;
    _dimension = control_points[0].size();
    _curve_points.resize(num_curve_pts);
    _handle_1_points.resize(num_curve_pts);
    _handle_2_points.resize(num_curve_pts);

    size_t i = 0;
    for(size_t j = 0; j < num_curve_pts-1; ++j)
    {
        _curve_points[j] = control_points[i++];
        _handle_2_points[j] = control_points[i++];
        _handle_1_points[j+1] = control_points[i++];
    }
    _curve_points.back() = control_points[i++];

    ASSERT(i == control_points.size());

    _handle_2_points.back() = 2*_curve_points.back() - _handle_1_points.back();

    _handle_1_points.front() = 2*_curve_points.front() - _handle_2_points.front();
}

Polybezier_curve* Polybezier_curve::clone() const
{
    return new Polybezier_curve(*this);
}

size_t Polybezier_curve::size() const
{
    return _curve_points.size();
}

size_t Polybezier_curve::dimension() const
{
    return _dimension;
}

Vector Polybezier_curve::evaluate(double u) const
{
    const Polybezier_curve& c = *this;

    static const size_t DEGREE = 3;
    if(u < 0.0 || u > 1.0)
        KJB_THROW(kjb::Index_out_of_bounds);

    size_t d = c.dimension();
    std::vector<kjb::Vector> pts(DEGREE+1, kjb::Vector(d));

    // which segment are we in?
    if(u == 1.0)
        return _curve_points.back();
    if(u == 0.0)
        return _curve_points.front();

    size_t i = u*(c.size()-1);

    // convert to index-within-segment
    u = u*(c.size()-1) - i;

    // BEGIN assume DEGREE = 3
    pts[0] = c.get_curve_point(i);
    pts[1] = c.get_handle_point_2(i);
    pts[2] = c.get_handle_point_1(i+1);
    pts[3] = c.get_curve_point(i+1);
    // END assume DEGREE = 3

    std::vector<kjb::Vector> tmp_pts(DEGREE, kjb::Vector(d));

    // use de Casteljau's Algorithm; take the first and last point of each sub-curve
    // (this is a general implementation, for any DEGREE,
    //  and assumes new_pts already has first, last ,and middle point
    //  assigned)
    for(size_t d = 1; d <= DEGREE; ++d)
    {
        size_t cur_degree = 3 - d;
        size_t cur_num_pts = cur_degree + 1;
        for(size_t j = 0; j < cur_num_pts; ++j)
        {
            tmp_pts[j] = pts[j] + u*(pts[j+1] - pts[j]);
        }

        tmp_pts.swap(pts);
    }
    return pts[0];
}

Vector Polybezier_curve::gradient(double u) const
{
    return to_nurbs().gradient(u);
}

void Polybezier_curve::insert_curve_point(int index, const Vector& pt)
{
    if(index < 0 || index > (int) _curve_points.size())
    {
        KJB_THROW(Index_out_of_bounds);
    }
    
    _curve_points.insert(_curve_points.begin() + index, pt);
    _handle_1_points.insert(_handle_1_points.begin() + index, pt);
    _handle_2_points.insert(_handle_2_points.begin() + index, pt);
}

void Polybezier_curve::insert(int index, const Vector& curve_pt, const Vector& handle_1_pt, const Vector& handle_2_pt)
{
    if(index < 0 || index > (int) _curve_points.size())
    {
        KJB_THROW(Index_out_of_bounds);
    }
    
    _curve_points.insert(_curve_points.begin() + index, curve_pt);
    _handle_1_points.insert(_handle_1_points.begin() + index, handle_1_pt);
    _handle_2_points.insert(_handle_2_points.begin() + index, handle_2_pt);
}


Vector& Polybezier_curve::get_curve_point(int index)
{
    return _curve_points.at(index);
}

const Vector& Polybezier_curve::get_curve_point(int index) const
{
    return _curve_points.at(index);
}

Vector& Polybezier_curve::get_handle_point_1(int index)
{
    return _handle_1_points.at(index);
}

const Vector& Polybezier_curve::get_handle_point_1(int index) const
{
    return _handle_1_points.at(index);
}

Vector& Polybezier_curve::get_handle_point_2(int index)
{
    return _handle_2_points.at(index);
}

const Vector& Polybezier_curve::get_handle_point_2(int index) const
{
    return _handle_2_points.at(index);
}

Vector& Polybezier_curve::get_control_point(int index)
{
    size_t i;
    Point_type type;
    decode_control_point(index, i, type);

    switch(type)
    {
        case CURVE_POINT:
            return get_curve_point(i);
        case HANDLE_2_POINT:
            return get_handle_point_2(i);
        case HANDLE_1_POINT:
            return get_handle_point_1(i);
        default:
            abort(); // error
    }
}

const Vector& Polybezier_curve::get_control_point(int index) const
{
    size_t i = (index + 1)/3;
    size_t type = index % 3;

    switch(type)
    {
        case 0:
            return get_curve_point(i);
        case 1:
            return get_handle_point_2(i);
        case 2:
            return get_handle_point_1(i);
        default:
            abort(); // error
    }
}

void Polybezier_curve::set_curve_point(int index, const Vector& pt)
{
    _curve_points.at(index) = pt;
}

void Polybezier_curve::set_handle_point_1(int index, const Vector& pt)
{
    if((int) pt.get_length() != _dimension)
    {
        KJB_THROW_2(Illegal_argument, "Wrong dimension in new spline point.");
    }

    _handle_1_points.at(index) = pt;
}

void Polybezier_curve::set_handle_point_2(int index, const Vector& pt)
{
    if((int) pt.get_length() != _dimension)
    {
        KJB_THROW_2(Illegal_argument, "Wrong dimension in new spline point.");
    }

    _handle_2_points.at(index) = pt;
}

void Polybezier_curve::delete_curve_point(int index)
{
    ASSERT(index < this->size());
    ASSERT(index >= 0);

    _curve_points.erase(_curve_points.begin() + index);
    _handle_2_points.erase(_handle_2_points.begin() + index);
    _handle_1_points.erase(_handle_1_points.begin() + index);
}

void Polybezier_curve::reverse()
{
    std::reverse(_curve_points.begin(), _curve_points.end());
    std::reverse(_handle_2_points.begin(), _handle_2_points.end());
    std::reverse(_handle_1_points.begin(), _handle_1_points.end());
}

/**
 * Set handle 1 to make symmetric to handle 2
 *
 * @param index the index of the curve point corresponding to the handle to be set
 */
void Polybezier_curve::symmetrize_handle_1(int index)
{
    if(index < 0 || index > (int) _curve_points.size() )
    {
        KJB_THROW(Index_out_of_bounds);
    }

    Vector& pt = _curve_points[index];
    Vector& h2 = _handle_2_points[index];

    Vector h1 = pt - (h2 - pt);

    set_handle_point_1(index, h1);
}

/**
 * Set handle 2 to make symmetric to handle 1
 *
 * @param index the index of the curve point corresponding to the handle to be set
 */
void Polybezier_curve::symmetrize_handle_2(int index)
{
    if(index < 0 || index > (int) _curve_points.size() )
    {
        KJB_THROW(Index_out_of_bounds);
    }

    Vector& pt = _curve_points[index];
    Vector& h1 = _handle_1_points[index];

    Vector h2 = pt - (h1 - pt);

    set_handle_point_2(index, h2);
}

Nurbs_curve Polybezier_curve::to_nurbs() const
{
    int total_ctl_pts = 3 *  (_curve_points.size()) - 2;
//    float* ctl_points = new float[3 * total_ctl_pts];
    float* knots = new float[total_ctl_pts + 4];

//    int index = 0;
    int knots_index = 0;

    vector<Vector> ctl_points;
    ctl_points.reserve(_curve_points.size() * 3);

    // 2 extra knots at beginning
    knots[knots_index++] = 0;
    knots[knots_index++] = 0;
    knots[knots_index++] = 0;
    knots[knots_index++] = 0;

    ctl_points.push_back(_curve_points[0]);
    ctl_points.push_back(_handle_2_points[0]);

    int i;
    for(i = 1; i < (int) _curve_points.size() - 1; i++)
    {
        knots[knots_index++] = i;
        knots[knots_index++] = i;
        knots[knots_index++] = i; 

        ctl_points.push_back(_handle_1_points[i]);
        ctl_points.push_back(_curve_points[i]);
        ctl_points.push_back(_handle_2_points[i]);
    }

    ctl_points.push_back(_handle_1_points[i]);
    ctl_points.push_back(_curve_points[i]);

    // 2 extra knots at end
    knots[knots_index++] = i;
    knots[knots_index++] = i;
    knots[knots_index++] = i;
    knots[knots_index++] = i;

//    for(int i = 0; i < knots_index; i++)
//    {
//        cout << "knots: " << knots[i] << endl;
//    }
//
//
//    for(int i = 0; i < _curve_points.size(); i++)
//    {
//        cout << "pts: " << _curve_points[i][0] << " " << _curve_points[i][1] << " " << _curve_points[i][2] << " " << endl;
//    }
//
//    for(int i = 0; i < 3*total_ctl_pts; i++)
//    {
//        if(i % 3 == 0) cout << "ctl: ";
//
//        cout << ctl_points[i] << " ";
//
//        if(i % 3 == 2) cout << endl;
//    }
 
    ASSERT(total_ctl_pts == (int) ctl_points.size());
    return Nurbs_curve(total_ctl_pts + 4, knots, 3, ctl_points);
}

#ifdef DEBUG
void Polybezier_curve::print_svg() const
{
    cout << "m ";
    cout << _curve_points[0][0] << " ";
    cout << _curve_points[0][1] << " ";
    cout << "C ";

    cout << _handle_2_points[0][0] << " ";
    cout << _handle_2_points[0][1] << " ";

    int i;
    for(i = 1; i < _curve_points.size() - 1; i++)
    {
        cout << _handle_1_points[i][0] << " ";
        cout << _handle_1_points[i][1] << " ";

        cout << _curve_points[i][0] << " ";
        cout << _curve_points[i][1] << " ";

        cout << _handle_2_points[i][0] << " ";
        cout << _handle_2_points[i][1] << " ";
    }

    cout << _handle_1_points[i][0] << " ";
    cout << _handle_1_points[i][1] << " ";

    cout << _curve_points[i][0] << " ";
    cout << _curve_points[i][1] << " ";

}
#endif

bool operator==(const Polybezier_curve& op1, const Polybezier_curve& op2)
{
    if(op1.size() != op2.size()) return false;

    if(op1.size() == 0) return true;

    if(!std::equal(
            op1._curve_points.begin(),
            op1._curve_points.end(),
            op2._curve_points.begin()))
        return false;

    if(!std::equal(
            op1._handle_1_points.begin()+1,
            op1._handle_1_points.end(),
            op2._handle_1_points.begin()+1))
        return false;

    if(!std::equal(
            op1._handle_2_points.begin(),
            op1._handle_2_points.end()-1,
            op2._handle_2_points.begin()))
        return false;

    return true;
}

void Polybezier_curve::swap(Polybezier_curve& other)
{
    _curve_points.swap(other._curve_points);
    _handle_1_points.swap(other._handle_1_points);
    _handle_2_points.swap(other._handle_2_points);
    std::swap(_dimension, other._dimension);
}

Polybezier_curve subdivide(const Polybezier_curve& c, double u)
{
    static const size_t DEGREE = 3;
    if(u < 0.0 || u > 1.0)
        KJB_THROW(kjb::Index_out_of_bounds);

    size_t d = c.dimension();
    std::vector<kjb::Vector> pts(DEGREE+1, kjb::Vector(d));
    std::vector<kjb::Vector> new_pts(2*DEGREE+1, kjb::Vector(d));

    // BEGIN assume DEGREE = 3
    new_pts[3] = c.evaluate(u);
    // END

    // which segment are we in?
    size_t i;
    if(u == 1.0)
        i = c.size()-2; // edge-case: force back into final segment
    else
        i = u*(c.size()-1);

    // convert to index-within-segment
    u = u*(c.size()-1) - i;

    // BEGIN assume DEGREE = 3
    pts[0] = c.get_curve_point(i);
    pts[1] = c.get_handle_point_2(i);
    pts[2] = c.get_handle_point_1(i+1);
    pts[3] = c.get_curve_point(i+1);

    new_pts[0] = pts[0];
    new_pts[6] = pts[3];
    // END assume DEGREE = 3

    std::vector<kjb::Vector> tmp_pts(DEGREE, kjb::Vector(d));

    // use de Casteljau's Algorithm; take the first and last point of each sub-curve
    // (this is a general implementation, for any DEGREE,
    //  and assumes new_pts already has first, last ,and middle point
    //  assigned)
    for(size_t d = 1; d < DEGREE; ++d)
    {
        size_t cur_degree = 3 - d;
        size_t cur_num_pts = cur_degree + 1;
        for(size_t j = 0; j < cur_degree+1; ++j)
        {
            tmp_pts[j] = pts[j] + u*(pts[j+1] - pts[j]);
        }

        *(new_pts.begin()+d)  = tmp_pts[0];
        *(new_pts.rbegin()+d) = tmp_pts[cur_num_pts-1];

        tmp_pts.swap(pts);
    }

    
    // BEGIN assume DEGREE = 3
    Polybezier_curve new_curve = c;
    new_curve.set_handle_point_2(i, new_pts[1]);
    new_curve.set_handle_point_1(i+1, new_pts[5]);
    new_curve.insert(i+1, new_pts[3], new_pts[2], new_pts[4]);
    // END assume DEGREE = 3
    
    return new_curve;
}

} // namespace kjb
