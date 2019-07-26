
/* $Id: g_quaternion.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include <cmath>
#include <assert.h>
#include <ostream>
#include <istream>
#include "l_cpp/l_exception.h"
#include "g_cpp/g_quaternion.h"
#include "l/l_debug.h"
#include "l_cpp/l_util.h"
#include "m_cpp/m_vector_d.h"

using std::ostream;

namespace kjb
{

const Quaternion::Euler_axis Quaternion::EulSafe[4] = {EulX, EulY, EulZ, EulX};
const Quaternion::Euler_axis Quaternion::EulNext[4] = {EulY, EulZ, EulX, EulY};

const float Quaternion::NORMALIZED_TOLERANCE = 0.0001; // currently arbitrary

float Quaternion::EPSILON = FLT_EPSILON; // arbitrary
float Quaternion::GIMBAL_LOCK_EPSILON = 4.1e-2; // derived from tests, given above epsilon
Quaternion::Euler_mode Quaternion::DEFAULT_EULER_MODE = ZXZR;

Quaternion::Quaternion() :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    // identity quaternion
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    _update_euler_order();
}

/**
 * Construct a quaternion directly from its components.
 * @note Does not normalize norm to 1.  Use one of the other 
 *       constructors to create a unit quaternion.
 */
Quaternion::Quaternion(double x, double y, double z, double w) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
//    KJB(UNTESTED_CODE());
    _q.x = x; _q.y = y; _q.z = z; _q.w = w;

    _update_euler_order();
}

Quaternion::Quaternion(double phi, double theta, double psi, Euler_mode mode) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    // identity quaternion
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    set_euler_mode(mode);
    _update_euler_order();

    set_euler_angles(phi, theta, psi);
}

Quaternion::Quaternion(const Matrix& rm) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    // identity quaternion
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    _update_euler_order();

    set_rotation_matrix(rm);
}

/**
 * Set rotation matrix from new x, y, z vectors
 */
Quaternion::Quaternion(
        const Vector& x,
        const Vector& y,
        const Vector& z) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    assert((x.magnitude()) - 1.0 < 3 * FLT_EPSILON);
    assert((y.magnitude()) - 1.0 < 3 * FLT_EPSILON);
    assert(fabs(z.magnitude()) - 1.0 < 3 * FLT_EPSILON);
    assert((cross(x,y) - z).magnitude() < 3 * FLT_EPSILON);
    // identity quaternion
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    _update_euler_order();

    // rotation matrix
    Matrix rm(3,3);

    rm.set_col(0, x);
    rm.set_col(1, y);
    rm.set_col(2, z);

    set_rotation_matrix(rm);
}


Quaternion::Quaternion(const Vector & axis, double angle) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    // identity quaternion
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    _update_euler_order();

    set_axis_angle(axis, angle);
}

/**
 * Quaternion representation of a point vector.
 * Private constructor; used by the rotate() method
 */
Quaternion::Quaternion(const Vector& v) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(DEFAULT_EULER_MODE),
    _q(),
    _euler_order(-1)
{
    if(v.get_length() != 3) {
        KJB_THROW_2(Illegal_argument, "Quaternion(const Vector&): Vector must be of dimension = 3");
    }

    _q.x = v(0);
    _q.y = v(1);
    _q.z = v(2);
    _q.w = 0; // (shouldn't this be 1?); no.

    // this constructor is special in that the quaternion shouldn't be normalized
    assert(fabs(magnitude_squared() - 1.0) < FLT_EPSILON); 

    _update_euler_order();
}

// quick copy (don't copy derived quantities)
Quaternion::Quaternion(const Quaternion& src) :
    _gimbal_lock(false),
    _rotation_matrix(4,4),
    _rotation_matrix_dirty(true),
    _axis(3, 0.0),
    _angle(0),
    _axis_angle_dirty(true),
    _eulers(3, 0.0),
    _eulers_dirty(true),
    _euler_mode(src._euler_mode),
    _q(src._q),
    _euler_order(src._euler_order)
{ }

Quaternion Quaternion::clone() const
{
    Quaternion copy;
    copy = *this;
    return copy;
}

void Quaternion::swap(Self& other)
{
    using std::swap;

    swap(_gimbal_lock, other._gimbal_lock);

    _rotation_matrix.swap(other._rotation_matrix);
    swap(_rotation_matrix_dirty, other._rotation_matrix_dirty);

    _axis.swap(other._axis);
    swap(_angle, other._angle);
    swap(_axis_angle_dirty, other._axis_angle_dirty);

    swap(_euler_mode, other._euler_mode); 
    _eulers.swap(other._eulers);
    swap(_eulers_dirty, other._eulers_dirty);
    swap(_q, other._q);
    swap(_euler_order, other._euler_order);
}

Quaternion& Quaternion::init_identity()
{
    _q.x = _q.y = _q.z = 0;
    _q.w = 1;

    _dirty_all();
    _gimbal_lock = false;

    return *this;
}


const Matrix& Quaternion::get_rotation_matrix() const 
{
    if(_rotation_matrix_dirty) _update_rotation_matrix();
    return _rotation_matrix;
}

const Vector& Quaternion::get_axis() const
{
    if(_axis_angle_dirty) _update_axis_angle();
    return _axis;
}

double Quaternion::get_angle() const
{
    if(_axis_angle_dirty) _update_axis_angle();
    return _angle;
}

Vector Quaternion::get_axis_angle() const 
{
    if(_axis_angle_dirty) _update_axis_angle();

    assert(fabs(_axis.magnitude_squared() - 1.0) < FLT_EPSILON);
    return _axis * _angle;
}

const Vector& Quaternion::get_euler_angles() const
{
    if(_eulers_dirty) _update_euler_angles();
    return _eulers;
}

Vector Quaternion::get_imag() const
{
    Vector v(3);
    v(0) = _q.x;
    v(1) = _q.y;
    v(2) = _q.z;

    return v;
}

double Quaternion::get_real() const { return _q.w; }

Quaternion& Quaternion::set_rotation_matrix(const Matrix& rotation_matrix)
{

    if(!_rotation_matrix_dirty && rotation_matrix == _rotation_matrix) return *this;

    if(rotation_matrix.get_num_rows() == 3 && 
       rotation_matrix.get_num_cols() == 3)
    {
        // TODO: add this check to the 4x4 case, too.  currently, det is only implemented up to 3x3
        if(det(rotation_matrix) < 0.0)
        {
            KJB_THROW_2(Illegal_argument, "rotation matrix may not have negative determinant.");
        }

        _rotation_matrix = rotation_matrix;
        _rotation_matrix.resize(4,4);
        for(int i = 0; i < 4; i++)
        {
            _rotation_matrix(3,i) = 0;
            _rotation_matrix(i,3) = 0;
        }

        _rotation_matrix(3,3) = 1.0;
        
    } 
    else if(rotation_matrix.get_num_rows() != 4 || 
            rotation_matrix.get_num_cols() != 4)
    {
        KJB_THROW_2(Illegal_argument, "Rotation matrix must be 4x4");
    }
    else
    {
        _rotation_matrix = rotation_matrix;
    }


    _dirty_all();
    _rotation_matrix_dirty = false;

    _update_from_rotation_matrix();

    return *this;
}

Quaternion& Quaternion::set_axis_angle(const Vector& axis, double angle)
{
    if(axis == _axis && std::fabs(angle - _angle) < FLT_EPSILON) return *this;

    if(axis.get_length() != 3) {
        KJB_THROW_2(Illegal_argument, "Axis vector must be of dimension = 3.");
    }

    _axis = axis;
    _angle = angle;

    _dirty_all();
    _axis_angle_dirty = false;

    // ensure angle between 0 and PI
    _fix_axis_angle();

    _update_from_axis_angle();


    return *this;
}

/**
 * Set axis/angle from 3-vector, where magnitude is angle, and direction
 * is axis.
 */
Quaternion& Quaternion::set_axis_angle(const Vector& axis_angle)
{
    if(axis_angle.size() != 3)
        KJB_THROW_2(Illegal_argument, "axis_angle vector must have size = 3.");

    double angle = axis_angle.magnitude();
    Vector axis = axis_angle / angle;

    set_axis_angle(axis, angle);

    return *this;
}


/** Find quaternion that rotates vector d1 to vector d2 in the smallest angle. */
Quaternion& Quaternion::set_from_directions(const Vector& d1, const Vector& d2)
{
    //// replaced this with method below, which is more stable
    //// (plus this one had a small bug when d1 == -d2)
//    Vector axis = cross(d1, d2);
//    double angle = acos(dot(d1, d2));
//
//    if(axis.magnitude() < FLT_EPSILON)
//    {
//        using std::swap;
//
//        Quaternion id;
//        swap(*this, id);
//        return *this;
//    }
//
//    axis.normalize();
//
//    return set_axis_angle(axis, angle);

    double dt = dot(d1, d2);
    if(dt < FLT_EPSILON - 1)
    {
        // d1 == -d2
        Vector axis = cross(Vector(1.0, 0.0, 0.0), d1);
        if(axis.magnitude() < FLT_EPSILON)
        {
            axis = cross(Vector(0.0, 1.0, 0.0), d1);
        }

        return set_axis_angle(axis, M_PI);
    }
    else if(dt > 1 - FLT_EPSILON)
    {
        // d1 == d2
        using std::swap;
        Quaternion id;
        swap(*this, id);
        return *this;
    }
    else
    {
        using std::swap;

        Vector d3 = cross(d1, d2);
        Quaternion q(d3[0], d3[1], d3[2], 1 + dt);
        q._normalize();
        swap(*this, q);
        return *this;
    }
}

Quaternion & Quaternion::set_from_directions(const Vector3& d1, const Vector3& d2)
{
    Vector d1_tmp(d1.begin(), d1.end());
    Vector d2_tmp(d2.begin(), d2.end());
    return set_from_directions(d1_tmp, d2_tmp);
}

Quaternion& Quaternion::set_euler_angles(double phi, double theta, double psi)
{
    if(std::fabs(_eulers(0) - phi) < FLT_EPSILON && std::fabs(_eulers(1) - theta) < FLT_EPSILON && std::fabs(_eulers(2) - psi) < FLT_EPSILON) return *this;

    _eulers(0) = phi;
    _eulers(1) = theta;
    _eulers(2) = psi;

    _dirty_all();
    _eulers_dirty = false;

    _fix_angles();

    _update_from_euler_angles();
    return *this;
}

/**
 * Set first euler rotation
 */
Quaternion& Quaternion::set_euler_phi(double phi)
{
    _update_euler_angles(); // have to make sure that _eulers({1,2}) are current
    return set_euler_angles(phi, _eulers(1), _eulers(2));
}

/**
 * Set second euler rotation
 */
Quaternion& Quaternion::set_euler_theta(double theta)
{
    _update_euler_angles(); // have to make sure that _eulers({0,2}) are current
    return set_euler_angles(_eulers(0), theta, _eulers(2));
}

/**
 * Set first euler rotation
 */
Quaternion& Quaternion::set_euler_psi(double psi)
{
    _update_euler_angles(); // have to make sure that _eulers({0,1}) are current
    return set_euler_angles(_eulers(0), _eulers(1), psi);
}


Quaternion& Quaternion::set_euler_mode(Euler_mode m)
{
    // mostly unnecessary optimization, and it makes the euler
    // angle constructor cumbersome
//    if(m == _euler_mode) return *this;

    _euler_mode = m;
    _update_euler_order();
    _eulers_dirty = true;

    return *this;
}

Quaternion::Euler_mode Quaternion::get_euler_mode() const
{
    return _euler_mode;
}

void Quaternion::set_default_euler_mode(Euler_mode m)
{
    DEFAULT_EULER_MODE = m;
}

Quaternion::Euler_mode Quaternion::get_default_euler_mode()
{
    return DEFAULT_EULER_MODE;
}

void Quaternion::set_gimbal_lock_epsilon(float e)
{
    GIMBAL_LOCK_EPSILON = e;
}

float Quaternion::get_gimbal_lock_epsilon()
{
    return GIMBAL_LOCK_EPSILON;
}

void Quaternion::set_epsilon(float e)
{
    EPSILON = e;
}

float Quaternion::get_epsilon()
{
    return EPSILON;
}


Vector Quaternion::rotate(const Vector& v) const
{
//    KJB(UNTESTED_CODE());

    bool homogeneous = false;

    Vector v_tmp = v;

    if(v_tmp.size() == 4)
    {
        v_tmp /= v_tmp[3];
        v_tmp.resize(3);
        homogeneous = true;
    }
    else if(v_tmp.get_length() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Rotate(v): v must be of dimension = 3");
    }

    double mag = v_tmp.magnitude();

    // close to origin; rotation has no effect;
    if(mag < FLT_EPSILON) return v;
    
    // special constructor 
    Quaternion v_quat( v_tmp / mag );

    // TODO: try deleting this, as it should already be normalized
//    v_quat._normalize();

    Quaternion tmp(*this);
    tmp*= v_quat;
    tmp*= Quaternion(*this).conj();
    
    Vector result = tmp.get_imag() * mag;
    if(homogeneous)
        result.resize(4,1.0);

    return result;
}

Vector3 Quaternion::rotate(const Vector3& v) const
{
    Vector v_tmp(v.begin(), v.end());

    return Vector3(rotate(v_tmp));
}


/***********************************
 * Assignment Operator
 * *********************************/
Quaternion& Quaternion::operator=(const Quaternion& src)
{
    _q = src._q;
    _rotation_matrix = src._rotation_matrix;
    _rotation_matrix_dirty = src._rotation_matrix_dirty;
    _axis = src._axis;
    _angle = src._angle;
    _axis_angle_dirty = src._axis_angle_dirty;
    _eulers = src._eulers;
    _eulers_dirty = src._eulers_dirty;
    _euler_mode = src._euler_mode;
    _euler_order = src._euler_order;
    _gimbal_lock = src._gimbal_lock;

    return *this;
}

/***********************************
 * Mutating Arithmetic Operators 
 * *********************************/
Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    assert(std::fabs(1 - magnitude_squared()) < EPSILON);

    // identity quaternion
    if(q.get_imag().magnitude() <  EPSILON) return *this;

    double W = _q.w, X = _q.x, Y = _q.y;

    Quaternion old(*this);

    _q.w = W*q._q.w - X*q._q.x - Y*q._q.y - _q.z*q._q.z;
    _q.x = W*q._q.x + X*q._q.w + Y*q._q.z - _q.z*q._q.y;
    _q.y = W*q._q.y - X*q._q.z + Y*q._q.w + _q.z*q._q.x;
    _q.z = W*q._q.z + X*q._q.y - Y*q._q.x + _q.z*q._q.w;

    if(*this != old) {
        _dirty_all();
    }

    _normalize();

    return * this;
}

Quaternion& Quaternion::operator/=(const Quaternion& q)
{
    KJB(UNTESTED_CODE());
    assert(std::fabs(1 - magnitude_squared()) < EPSILON);

    // identity quaternion
    if(q.get_imag().magnitude() <  EPSILON) return *this;

    double denom = q.magnitude_squared();
    double RE = _q.w, I = _q.x, J = _q.y;

    Quaternion old(*this);

    _q.w = ( RE*q._q.w + I*q._q.x + J*q._q.y + _q.y*q._q.z)/denom;
    _q.x = (-RE*q._q.x + I*q._q.w - J*q._q.z + _q.y*q._q.y)/denom;
    _q.y = (-RE*q._q.y + I*q._q.z + J*q._q.w - _q.y*q._q.x)/denom;
    _q.z = (-RE*q._q.z - I*q._q.y + J*q._q.x + _q.y*q._q.w)/denom;

    if(*this != old) {
        _dirty_all();
    }

    _normalize();

    return * this;
}

// This is mostly used for intermediate calculations (e.g. finding a mean quaternion); it has nothing to do with 3D rotations.  If you want to compose two rotations, use the "*" opreator.
Quaternion& Quaternion::operator+=(const Quaternion & q)
{
    Quaternion old(*this);

    // make sure angles are in the same direction
    int sign = 1;
    if((q._q.w > 0)  != (_q.w > 0))
    {
        sign = -1;
    }

    _q.w += q._q.w * sign;
    _q.x += q._q.x * sign;
    _q.y += q._q.y * sign;
    _q.z += q._q.z * sign;

    if(*this != old) {
        _dirty_all();
    }

    return *this;
}
/// This is simple 4-vector subtraction.  If you want to invert a rotation, don't use this--multiply by the conugate instead (or use the division operator (untested?))
Quaternion& Quaternion::operator-=(const Quaternion & q)
{
    Quaternion old(*this);

    int sign = 1;
    if((q._q.w > 0)  != (_q.w > 0))
    {
        sign = -1;
    }

    _q.w -= q._q.w * sign;
    _q.x -= q._q.x * sign;
    _q.y -= q._q.y * sign;
    _q.z -= q._q.z * sign;

    if(*this != old) {
        _dirty_all();
    }

    return *this;
}

/// This is used in computing the mean, and in normalization
Quaternion& Quaternion::operator*=(double x)
{
    if(x < EPSILON) return *this;

    Quaternion old(*this);

    _q.w *= x;
    _q.x *= x;
    _q.y *= x;
    _q.z *= x;

    if(*this != old) {
        _dirty_all();
    }

    return *this;
}

Quaternion& Quaternion::operator/=(double x)
{
    if(x < EPSILON) return *this;

    Quaternion old(*this);

    _q.w /= x;
    _q.x /= x;
    _q.y /= x;
    _q.z /= x;

    if(*this != old) {
        _dirty_all();
    }

    return *this;
}
//
//Quaternion & operator+=(double x);
//{
//    if(std::fabs(x) < EPSILON) return *this;
//
//    _q.w += x;
//
//    _dirty_all();
//
//    return *this;
//}
//
//Quaternion & operator-=(double x);
//{
//    if(std::fabs(x) < EPSILON) return *this;
//
//    _q.w -= x;
//    _dirty_all();
//
//    return *this;
//}

bool Quaternion::operator==(const Quaternion& op2) const
{
    return 
        std::fabs(_q.x - op2._q.x) < EPSILON && 
        std::fabs(_q.y - op2._q.y) < EPSILON && 
        std::fabs(_q.z - op2._q.z) < EPSILON && 
        std::fabs(_q.w - op2._q.w) < EPSILON;
}

bool Quaternion::operator!=(const Quaternion& op2) const
{
    return !(*this == op2);

}

Quaternion Quaternion::conj() const
{
    Quaternion result(*this);

    assert(std::fabs(1 - magnitude_squared()) < EPSILON);

    // real part is basically zero
    if(1 - result._q.w < EPSILON) return result;

    result._q.x = -result._q.x;
    result._q.y = -result._q.y;
    result._q.z = -result._q.z;

    result._dirty_all();

    return result;
}

double Quaternion::norm(unsigned int l) const
{
    double w = 1;
    double x = 1;
    double y = 1;
    double z = 1;

    // this runs over 500% faster than the general case!
//    if(l == 2)
//        return sqrt(
//                _q.w * _q.w +
//                _q.x * _q.x + 
//                _q.y * _q.y + 
//                _q.z * _q.z );

    for(unsigned int i = 0; i < l; i++)
        w*= _q.w;

    for(unsigned int i = 0; i < l; i++)
        x*= _q.x;

    for(unsigned int i = 0; i < l; i++)
        y*= _q.y;

    for(unsigned int i = 0; i < l; i++)
        z*= _q.z;

    // sqrt is over 500% faster than pow(x, 0.5)!!
    if(l == 2) return sqrt(w + x + y + z);

    return pow(w + x + y + z, 1.0/l);
}

/***********************************
 * Binary Arithmetic Operators 
 * *********************************/
Quaternion Quaternion::operator*(const Quaternion& op_2) const
{
//    KJB(UNTESTED_CODE());

    Quaternion q(*this);

    return q *= op_2;
}

Quaternion Quaternion::operator/(const Quaternion& op_2) const
{
    KJB(UNTESTED_CODE());

    Quaternion q(*this);

    return q /= op_2;
}


/*********************************
 * State-syncronization functions
 *********************************/
void Quaternion::_update_rotation_matrix() const
{
//    KJB(UNTESTED_CODE());

    assert(fabs(magnitude_squared() - 1.0) < FLT_EPSILON);

    if(!_rotation_matrix_dirty) return;
    _rotation_matrix_dirty = false;

    Matrix& m = _rotation_matrix;
    // use common quat-to-mat code
    m(3,0) = m(3,1) = m(3,2) = m(0,3) = m(1,3) = m(2,3) = 0;
    m(3,3) = 1;

    // Credit: J.M.P. van Waveren
    // From article: "From Quaternion to Matrix and Back"
    // I have modified for use in this application, most notably, transposed the rotation matrix
    // and ignored the "translation along axis" part
    double x2 = _q.x + _q.x;
    double y2 = _q.y + _q.y;
    double z2 = _q.z + _q.z;
    {
        double xx2 = _q.x * x2;
        double yy2 = _q.y * y2;
        double zz2 = _q.z * z2;
        m(0,0) = 1.0f - yy2 - zz2;
        m(1,1) = 1.0f - xx2 - zz2;
        m(2,2) = 1.0f - xx2 - yy2;
    }
    {
        double yz2 = _q.y * z2;
        double wx2 = _q.w * x2;
        m(1,2) = yz2 - wx2;
        m(2,1) = yz2 + wx2;
    }
    {
        double xy2 = _q.x * y2;
        double wz2 = _q.w * z2;
        m(0,1) = xy2 - wz2;
        m(1,0) = xy2 + wz2;
    }
    {
        double xz2 = _q.x * z2;
        double wy2 = _q.w * y2;
        m(2,0) = xz2 - wy2;
        m(0,2) = xz2 + wy2;
    }

    assert(fabs(m.get_col(0).magnitude() - 1.0) < FLT_EPSILON);
    assert(fabs(m.get_col(1).magnitude() - 1.0) < FLT_EPSILON);
    assert(fabs(m.get_col(2).magnitude() - 1.0) < FLT_EPSILON);
    assert(dot(m.get_col(0), m.get_col(1)) < FLT_EPSILON);
    assert(dot(m.get_col(1), m.get_col(2)) < FLT_EPSILON);
    assert(dot(m.get_col(2), m.get_col(3)) < FLT_EPSILON);
}

void Quaternion::_update_axis_angle() const
{
//    KJB(UNTESTED_CODE());

    if(!_axis_angle_dirty) return;
    _axis_angle_dirty = false;

    // get sin(a/2) and cos(a/2)
    _axis = get_imag();
    double sin_a_2 = _axis.magnitude();
    double cos_a_2 = get_real();

    _angle = std::atan2(sin_a_2, cos_a_2) * 2;
    _axis.normalize();

    _fix_axis_angle();
}

void Quaternion::_update_euler_angles() const
{
    using std::atan2;
    using std::sqrt;

//    KJB(UNTESTED_CODE());

    if(!_eulers_dirty) return;
    _eulers_dirty = false;

    _update_rotation_matrix();

    // EXTRACT EULERS FROM ROTATION MATRIX
    // This function is quite unreadable.  Luckilly, it is
    // reliable: it is adapted from Shoemake's C code from 
    // Graphics Gems IV, and compactly handles all 
    // 24 euler conventions.  
    //
    // #define's have been replaced by inline functions
    // and enums.  
    Euler_axis i,j,k,h;
    Euler_parity n;
    Euler_repeat s;
    Euler_frame f;

    Matrix m(_rotation_matrix);

    _eul_get_ord(_euler_order,i,j,k,h,n,s,f);

    double phi, theta, psi;

    if (s==EulRepYes) {
        double sy = sqrt(m(i,j)*m(i,j) + m(i,k)*m(i,k));
        if (sy > 16*FLT_EPSILON) {
            phi = atan2(m(i,j), m(i,k));
            theta = atan2(sy, m(i,i));
            psi = atan2(m(j,i), -m(k,i));
        } else {
            phi = atan2(-m(j,k), m(j,j));
            theta = atan2(sy, m(i,i));
            psi = 0;
        }
    } else {
        double cy = sqrt(m(i,i)*m(i,i) + m(j,i)*m(j,i));
        if (cy > 16*FLT_EPSILON) {
            phi = atan2(m(k,j), m(k,k));
            theta = atan2(-m(k,i), cy);
            psi = atan2(m(j,i), m(i,i));
        } else {
            phi = atan2(-m(j,k), m(j,j));
            theta = atan2(-m(k,i), cy);
            psi = 0;
        }
    }
    if (n==EulParOdd) {phi = -phi; theta = - theta; psi = -psi;}
    if (f==EulFrmR) {double t = phi; phi = psi; psi = t;}
    
    _eulers(0) = phi;
    _eulers(1) = theta;
    _eulers(2) = psi;

    _fix_angles();
}


void Quaternion::_update_from_rotation_matrix()
{
//    KJB(UNTESTED_CODE());

    // Credit: J.M.P. van Waveren
    // From article: "From Quaternion to Matrix and Back"
    // I have modified for use in this application, most notably, transposed the rotation matrix
    // and ignored the "translation along axis" part
    Matrix m( _rotation_matrix);
    if ( m(0,0) + m(1,1) + m(2,2) > 0.0f ) {
        double t = + m(0,0) + m(1,1) + m(2,2) + 1.0f;
//        double s = ReciprocalSqrt( t ) * 0.5f;
        double s = 0.5f / sqrt(t);
        _q.w = s * t;
        _q.z = ( m(1,0) - m(0,1) ) * s;
        _q.y = ( m(0,2) - m(2,0) ) * s;
        _q.x = ( m(2,1) - m(1,2) ) * s;
    } else if ( m(0,0) > m(1,1) && m(0,0) > m(2,2) ) {
        double t = + m(0,0) - m(1,1) - m(2,2) + 1.0f;
//        double s = ReciprocalSqrt( t ) * 0.5f;
        double s = 0.5f / sqrt(t);
        _q.x = s * t;
        _q.y = ( m(1,0) + m(0,1) ) * s;
        _q.z = ( m(0,2) + m(2,0) ) * s;
        _q.w = ( m(2,1) - m(1,2) ) * s;
    } else if ( m(1,1) > m(2,2) ) {
        double t = - m(0,0) + m(1,1) - m(2,2) + 1.0f;
//        double s = ReciprocalSqrt( t ) * 0.5f;
        double s = 0.5f / sqrt(t);
        _q.y = s * t;
        _q.x = ( m(1,0) + m(0,1) ) * s;
        _q.w = ( m(0,2) - m(2,0) ) * s;
        _q.z = ( m(2,1) + m(1,2) ) * s;
    } else {
        double t = - m(0,0) - m(1,1) + m(2,2) + 1.0f;
//        double s = ReciprocalSqrt( t ) * 0.5f;
        double s = 0.5f / sqrt(t);
        _q.z = s * t;
        _q.w = ( m(1,0) - m(0,1) ) * s;
        _q.x = ( m(0,2) + m(2,0) ) * s;
        _q.y = ( m(2,1) + m(1,2) ) * s;
    }

    _normalize();
}

void Quaternion::_update_from_axis_angle()
{
//    KJB(UNTESTED_CODE());

    // Taken from wikipedia "Quaternions_and_spatial_rotation".
    double sin_a = std::sin(_angle/2);

    // TODO isnt this necessary?
//    sin_a *= sin_a;

    // use common axis angle to quat code
    _q.x = _axis[0] * sin_a;
    _q.y = _axis[1] * sin_a;
    _q.z = _axis[2] * sin_a;
    _q.w = std::cos(_angle/2);

    _normalize();
}

void Quaternion::_update_from_euler_angles()
{
//    KJB(UNTESTED_CODE());

    // EXTRACT QUATERNION FROM EULER ANGLES
    // This function is quite unreadable.  Luckilly, it is
    // reliable: it is adapted from Shoemake's C code from 
    // Graphics Gems IV, p 227, and compactly handles all 
    // 24 euler conventions.  
    //
    // #define's have been replaced by inline functions
    // and enums.  
    
    double a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    // temporary angle holders
    double phi = _eulers(0);
    double theta = _eulers(1);
    double psi = _eulers(2);

    Euler_axis i,j,k,h;
    Euler_parity n;
    Euler_repeat s;
    Euler_frame f;

    _eul_get_ord(_euler_order,i,j,k,h,n,s,f);

    // swap first and last angles if rotating frame
    if (f==EulFrmR) {double t = phi; phi = psi; psi = t;}

    // invert theta if axis sequence is reversed 
    if (n==EulParOdd) theta = -theta;

    ti = phi*0.5; tj = theta*0.5; th = psi*0.5;
    ci = cos(ti);  cj = cos(tj);  ch = cos(th);
    si = sin(ti);  sj = sin(tj);  sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;

    if (s==EulRepYes) {
    a[i] = cj*(cs + sc);    /* Could speed up with */
    a[j] = sj*(cc + ss);    /* trig identities. */
    a[k] = sj*(cs - sc);
    _q.w = cj*(cc - ss);
    } else {
    a[i] = cj*sc - sj*cs;
    a[j] = cj*ss + sj*cc;
    a[k] = cj*cs - sj*sc;
    _q.w = cj*cc + sj*ss;
    }
    if (n==EulParOdd) a[j] = -a[j];
    _q.x = a[EulX]; _q.y = a[EulY]; _q.z = a[EulZ];

    _normalize();
}


void Quaternion::_update_euler_order()
{
    //KJB(UNTESTED_CODE());
    switch(_euler_mode)
    {
        // innermost, parity, repeat, frame
        case XESS:  
                    // First, second, third, frame
                    case XYXS:
            _euler_order = _eul_ord(EulX,EulParEven,EulRepYes,EulFrmS);
            break;
        case XEDS:  case XYZS:
            _euler_order = _eul_ord(EulX,EulParEven,EulRepNo,EulFrmS);
            break;
        case XOSS:  case XZXS:
            _euler_order = _eul_ord(EulX,EulParOdd,EulRepYes,EulFrmS);
            break;
        case XODS:  case XZYS:
            _euler_order = _eul_ord(EulX,EulParOdd,EulRepNo,EulFrmS);
            break;
        case YESS:  case YZYS:
            _euler_order = _eul_ord(EulY,EulParEven,EulRepYes,EulFrmS);
            break;
        case YEDS:  case YZXS:
            _euler_order = _eul_ord(EulY,EulParEven,EulRepNo,EulFrmS);
            break;
        case YOSS:  case YXYS:
            _euler_order = _eul_ord(EulY,EulParOdd,EulRepYes,EulFrmS);
            break;
        case YODS:  case YXZS:
            _euler_order = _eul_ord(EulY,EulParOdd,EulRepNo,EulFrmS);
            break;
        case ZESS:  case ZXZS:
            _euler_order = _eul_ord(EulZ,EulParEven,EulRepYes,EulFrmS);
            break;
        case ZEDS:  case ZXYS:
            _euler_order = _eul_ord(EulZ,EulParEven,EulRepNo,EulFrmS);
            break;
        case ZOSS:  case ZYZS:
            _euler_order = _eul_ord(EulZ,EulParOdd,EulRepYes,EulFrmS);
            break;
        case ZODS:  case ZYXS:
            _euler_order = _eul_ord(EulZ,EulParOdd,EulRepNo,EulFrmS);
            break;
        case XESR:  case XYXR:
            _euler_order = _eul_ord(EulX,EulParEven,EulRepYes,EulFrmR);
            break;
        case XEDR:  case ZYXR:
            _euler_order = _eul_ord(EulX,EulParEven,EulRepNo,EulFrmR);
            break;
        case XOSR:  case XZXR:
            _euler_order = _eul_ord(EulX,EulParOdd,EulRepYes,EulFrmR);
            break;
        case XODR:  case YZXR:
            _euler_order = _eul_ord(EulX,EulParOdd,EulRepNo,EulFrmR);
            break;
        case YESR:  case YZYR:
            _euler_order = _eul_ord(EulY,EulParEven,EulRepYes,EulFrmR);
            break;
        case YEDR:  case XZYR:
            _euler_order = _eul_ord(EulY,EulParEven,EulRepNo,EulFrmR);
            break;
        case YOSR:  case YXYR:
            _euler_order = _eul_ord(EulY,EulParOdd,EulRepYes,EulFrmR);
            break;
        case YODR:  case ZXYR:
            _euler_order = _eul_ord(EulY,EulParOdd,EulRepNo,EulFrmR);
            break;
        case ZESR:  case ZXZR:
            _euler_order = _eul_ord(EulZ,EulParEven,EulRepYes,EulFrmR);
            break;
        case ZEDR:  case YXZR:
            _euler_order = _eul_ord(EulZ,EulParEven,EulRepNo,EulFrmR);
            break;
        case ZOSR:  case ZYZR:
            _euler_order = _eul_ord(EulZ,EulParOdd,EulRepYes,EulFrmR);
            break;
        case ZODR:  case XYZR:
            _euler_order = _eul_ord(EulZ,EulParOdd,EulRepNo,EulFrmR);
            break;
        default:
            KJB_THROW(Runtime_error); // can't happen
    }
}

/************************
 * Utility Functions
 * *********************/
//Are these ranges valid for all conventions?  Specifically, pitch, roll, yaw?
void Quaternion::_fix_angles() const
{
    Euler_axis i,j,k,h;
    Euler_parity parity;
    Euler_repeat repeat;
    Euler_frame f;

    _eul_get_ord(_euler_order,i,j,k,h,parity,repeat,f);

    // make all angles between [0,2PI)
    while(_eulers(0) < 0) _eulers(0) += 2 * M_PI;
    while(_eulers(1) < 0) _eulers(1) += 2 * M_PI;
    while(_eulers(2) < 0) _eulers(2) += 2 * M_PI;

    while(_eulers(0) >= (2 * M_PI)) _eulers(0) -= 2 * M_PI;
    while(_eulers(1) >= (2 * M_PI)) _eulers(1) -= 2 * M_PI;
    while(_eulers(2) >= (2 * M_PI)) _eulers(2) -= 2 * M_PI;

    // SPECIAL CASES:
    // Case 1:  ZXZ rotation; eulers = (0, 0, x); convert to (x, 0, 0)
    if(repeat == EulRepYes && 
            _eulers(0) < EPSILON && 
            _eulers(1) < EPSILON && 
            _eulers(2) >= EPSILON)
    {
        double t = _eulers(0);
        _eulers(0) = _eulers(2);
        _eulers(2) = t;
    }

    // END SPECIAL CASES
    if(repeat == EulRepYes)
    {
        // True "Euler" angles (e.g. zxz)
        
        // make second angle between [0, PI]
        if(_eulers(1) > M_PI)
        {
            // flip first and last angles by 180 degrees
            _eulers(0) += M_PI;
            if(_eulers(0) >= 2.f* M_PI) _eulers(0) -= 2.f* M_PI;

            _eulers(2) += M_PI;
            if(_eulers(2) >= 2.f * M_PI) _eulers(2) -= 2.f* M_PI;

            _eulers(1) = 2.f * M_PI - _eulers(1);

        }

        assert(_eulers(1) >= 0);
        assert(_eulers(1) <= M_PI);


        if(fabs(_eulers(1)) < GIMBAL_LOCK_EPSILON ||
                M_PI - fabs(_eulers(1)) < GIMBAL_LOCK_EPSILON )
        {
            _gimbal_lock = true;
        }
        else
        {
            _gimbal_lock = false;
        }
    }
    else
    {
        // Tait–Bryan rotations (e.g. yaw, pitch, roll, or zxy)
       
        // get angle between [-pi, pi]
        if(_eulers(1) > M_PI) _eulers(1) -= 2*M_PI;

        // make second angle between [-PI/2, PI/2]
        if(fabs(_eulers(1)) > M_PI/2.f)
        {
            _eulers(0) += M_PI;
            if(_eulers(0) > 2*M_PI) _eulers(0) -= 2*M_PI;

            _eulers(2) += M_PI;
            if(_eulers(2) > 2*M_PI) _eulers(2) -= 2*M_PI;


            // get angle above/below horizon
            double delta = M_PI - fabs(_eulers(1));

            // invert if below horizon
            if(_eulers(1) < 0) _eulers(1) = -delta;
            else _eulers(1) = delta;

        }
        
        assert(fabs(_eulers(1)) <= M_PI/2.f);

        if(M_PI/2 - fabs(_eulers(1)) < GIMBAL_LOCK_EPSILON ||
                _eulers(0) < GIMBAL_LOCK_EPSILON ||
                _eulers(2) < GIMBAL_LOCK_EPSILON ||
                2*M_PI - _eulers(0) < GIMBAL_LOCK_EPSILON ||
                2*M_PI - _eulers(1) < GIMBAL_LOCK_EPSILON
                )
        {
            _gimbal_lock = true;
        }
        else
        {
            _gimbal_lock = false;
        }
    }
}

// ensure angle between 0 and PI
void Quaternion::_fix_axis_angle() const
{
    if(_angle > M_PI) {
        _angle = 2 * M_PI - _angle;
        _axis *= -1;
    }

}

Quaternion& Quaternion::_normalize()
{
    double mag = norm();
    if(mag > EPSILON) 
    {
        *this /= mag;
    }
    else
    {
        //abort(); // bug
        KJB_THROW_2(KJB_error, "Quaternion magnitude is zero!!");
    }

    return *this;
}


Quaternion slerp(const Quaternion& q1, const Quaternion& q2, double t)
{
    if(t < 0 || t > 1) KJB_THROW(Index_out_of_bounds);

    if(t < FLT_EPSILON) return  q1;
    if(1-t < FLT_EPSILON) return q2;

    kjb::Vector4 v1( q1._q.x, q1._q.y, q1._q.z, q1._q.w);
    kjb::Vector4 v2( q2._q.x, q2._q.y, q2._q.z, q2._q.w);

    double cos_theta = dot(v1, v2);

    // use the smallest angle between the two vectors,
    // since the north and south hemisphere in Quaternion space
    // represent the same rotation
    if(cos_theta < 0.0)
    {
        v2 = -v2;
        cos_theta = -cos_theta;
    }

    if(cos_theta > 0.999)
    {
        v1 = v1 + t * (v2 - v1);
        v1.normalize();
        return Quaternion(v1[0], v1[1], v1[2], v1[3]);
    }

    double theta = acos(cos_theta);

    // formula from shoemake '85
    v2 = (sin((1.0 - t) * theta) * v1 + sin(t * theta) * v2)/sin(theta);

    return Quaternion(v2[0], v2[1], v2[2], v2[3]);
}

Quaternion slerp2(const Quaternion& q1, const Quaternion& q2, double t)
{
    if(t < 0 || t > 1) KJB_THROW(Index_out_of_bounds);

    if(t < FLT_EPSILON) return  q1;
    if(1-t < FLT_EPSILON) return q2;

    kjb::Vector4 v1( q1._q.x, q1._q.y, q1._q.z, q1._q.w);
    kjb::Vector4 v2( q2._q.x, q2._q.y, q2._q.z, q2._q.w);

    double cos_theta = dot(v1, v2);

    // use the smallest angle between the two vectors,
    // since the north and south hemisphere in Quaternion space
    // represent the same rotation
    if(cos_theta < 0.0)
    {
        v2 = -v2;
        cos_theta = -cos_theta;
    }

    if(cos_theta > 0.999)
    {
        v1 = v1 + t * (v2 - v1);
        v1.normalize();
        return Quaternion(v1[0], v1[1], v1[2], v1[3]);
    }

    assert(cos_theta >= -1 && cos_theta <= 1);

    double theta = acos(cos_theta);
    theta *= t;

    v2 = v2 - v1 * cos_theta;
    v2.normalize();

    v2 = v1 * cos(theta) + v2 * sin(theta);
    return Quaternion(v2[0], v2[1], v2[2], v2[3]);
}

Quaternion nlerp(const Quaternion& q1, const Quaternion& q2, double t)
{
    if(t < 0 || t > 1) KJB_THROW(Index_out_of_bounds);

    if(t < FLT_EPSILON) return  q1;
    if(1-t < FLT_EPSILON) return q2;

    kjb::Vector4 v1( q1._q.x, q1._q.y, q1._q.z, q1._q.w);
    kjb::Vector4 v2( q2._q.x, q2._q.y, q2._q.z, q2._q.w);

    // use the smallest angle between the two vectors,
    // since the north and south hemisphere in Quaternion space
    // represent the same rotation
    if(dot(v1,v2) < 0.0)
    {
        v2 = -v2;
    }

    v1 = v1 + t * (v2 - v1);
    v1.normalize();
    return Quaternion(v1[0], v1[1], v1[2], v1[3]);
}



ostream & operator<<(ostream& out, const Quaternion& q)
{
    out << "x: " << q._q.x << ", y: " << q._q.y << ", z:" << q._q.z << ", w:" << q._q.w;
    return out;
}
}


