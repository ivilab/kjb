
/* $Id: g_quaternion.h 20911 2016-10-30 17:50:20Z ernesto $ */

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

#ifndef KJB_QUATERNION_H
#define KJB_QUATERNION_H

#include <iosfwd>

#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

namespace kjb {

//forward declarations
template <size_t D>
class Vector_d;
typedef Vector_d<3> Vector3;

class Quaternion
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif
    friend Quaternion slerp(const Quaternion& q1, const Quaternion& q2, double t);
    friend Quaternion slerp2(const Quaternion& q1, const Quaternion& q2, double t);
    friend Quaternion nlerp(const Quaternion& q1, const Quaternion& q2, double t);

    friend std::ostream & operator<<(std::ostream & out, const Quaternion & q);

    typedef Quaternion Self;

public:
/********** BEGIN SHOEMAKE ADAPTATION *********/
    enum Euler_mode {
        XESS = 0,  XYXS, // both are equivalent
        XEDS,  XYZS,
        XOSS,  XZXS,
        XODS,  XZYS,
        YESS,  YZYS,
        YEDS,  YZXS,
        YOSS,  YXYS,
        YODS,  YXZS,
        ZESS,  ZXZS,
        ZEDS,  ZXYS,
        ZOSS,  ZYZS,
        ZODS,  ZYXS,
        XESR,  XYXR,
        XEDR,  ZYXR,
        XOSR,  XZXR,
        XODR,  YZXR,
        YESR,  YZYR,
        YEDR,  XZYR,
        YOSR,  YXYR,
        YODR,  ZXYR,
        ZESR,  ZXZR,
        ZEDR,  YXZR,
        ZOSR,  ZYZR,
        ZODR,  XYZR
    };

    enum Euler_axis {
        EulX = 0, 
        EulY = 1, 
        EulZ = 2
    };

    enum Euler_parity {
        EulParEven = 0,
        EulParOdd = 1
    };

    enum Euler_repeat {
        EulRepNo = 0,
        EulRepYes = 1
    };

    enum Euler_frame {
        EulFrmS = 0,
        EulFrmR = 1
    };

/********** END SHOEMAKE ADAPTATION *********/

    Quaternion();
    Quaternion(double x, double y, double z, double w);
    Quaternion(double phi, double theta, double psi, Euler_mode mode = DEFAULT_EULER_MODE);
    Quaternion(const Matrix& rotation_matrix);
    Quaternion(const Vector& x, const Vector& y, const Vector& z);
    /* initialize from axis/angle */
    Quaternion(const Vector& axis, double angle);

    /* @brief quick copy.  Use clone or assignment operator to
     * copy all derived representations (rotation matrix, euler
     * angles, etc) */
    Quaternion(const Quaternion &);

    Quaternion clone() const;

    void swap(Self& other);

    Quaternion & init_identity();

    bool is_identity() const
    {
        return _q.x == 0 && _q.y == 0 && _q.z == 0 && _q.w == 1.0;
    }

    /* @brief returns 4x4 homogeneous rotation matrix */
    const Matrix& get_rotation_matrix() const;
    const Vector& get_axis() const ;

    Vector get_axis_angle() const ;

    double get_angle() const ;
    /** @brief returns vector [phi, theta, psi] */
    const Vector& get_euler_angles() const;

    /** @brief returns vector [x, y, z] */
    Vector get_imag() const;
    /** @brief returns w */
    double get_real() const;

    Quaternion & set_rotation_matrix(const Matrix& );
    Quaternion & set_axis_angle(const Vector&, double angle);
    Quaternion & set_axis_angle(const Vector& axis_angle);

    /** @brief Find quaternion that rotates vector d1 to vector d2 in the smallest angle. */
    Quaternion & set_from_directions(const Vector& d1, const Vector& d2);

    Quaternion & set_from_directions(const Vector3& d1, const Vector3& d2);

    Quaternion & set_euler_angles(double phi, double theta, double psi);
    Quaternion & set_euler_phi(double phi);
    Quaternion & set_euler_theta(double theta);
    Quaternion & set_euler_psi(double psi);


    Quaternion & set_euler_mode(Euler_mode m);
    Euler_mode get_euler_mode() const;

    static void set_default_euler_mode(Euler_mode m);
    static Euler_mode get_default_euler_mode();

    static void set_gimbal_lock_epsilon(float e);
    static float get_gimbal_lock_epsilon();

    static void set_epsilon(float e);
    static float get_epsilon();

    Vector rotate(const Vector& v) const;

    Vector3 rotate(const Vector3& v) const;

    Quaternion conj() const;


    /// compute the L-* norm
    double norm(unsigned int l = 2) const;

    double magnitude() const { return norm(); }

    double magnitude_squared() const 
    {
        return _q.w*_q.w + _q.x*_q.x + _q.y*_q.y + _q.z*_q.z;
    }

    /**************************************
     * Assignment Operator
     * ************************************/
    Quaternion & operator=(const Quaternion &);

    /***********************************
     * Mutating Arithmetic Operators 
     * *********************************/

    Quaternion& operator*=(const Quaternion &);
    Quaternion& operator/=(const Quaternion &);
    Quaternion& operator+=(const Quaternion &);
    Quaternion& operator-=(const Quaternion &);

    // these operators don't make sense from a user's perspective, but they're usefull in class implementation (e.g. normalize(), mean())
    Quaternion & operator*=(double);
    Quaternion & operator/=(double);


//    Quaternion & operator+=(double); //<-- no good
//    Quaternion & operator+=(float);
//    Quaternion & operator-=(float);

    /***********************************
     * Binary Arithmetic Operators 
     * *********************************/
    Quaternion operator*(const Quaternion &) const;
    Quaternion operator/(const Quaternion &) const;


    Quaternion operator+(const Quaternion& q) const
    {
        Quaternion result(*this);
        result += q;
        return result;
    }

    Quaternion operator-(const Quaternion& q) const
    {
        Quaternion result(*this);
        result -= q;
        return result;
    }


//    Quaternion operator*(float);
//    Quaternion operator/(float);
//    Quaternion operator+(float);
//    Quaternion operator-(float);

    /***********************************
     * Logical Operators 
     * *********************************/
    bool operator==(const Quaternion &) const;
    bool operator!=(const Quaternion &) const;

    mutable bool _gimbal_lock;

private:

    const static float NORMALIZED_TOLERANCE;
    static float EPSILON; // currently arbitrary
    static float GIMBAL_LOCK_EPSILON; // derived from tests
    static Euler_mode DEFAULT_EULER_MODE;

    mutable Matrix _rotation_matrix;
    mutable bool _rotation_matrix_dirty;

    mutable Vector _axis;
    mutable double _angle;
    mutable bool _axis_angle_dirty;

    mutable Vector _eulers;
    mutable bool _eulers_dirty;

    /** @brief describes the order of rotations, and 
     * stationary/moving frame */
    Euler_mode _euler_mode; // note to self: this should NOT be mutable  -KLS, Aug 2010

    struct Quat{ 
        double x;
        double y;
        double z;
        double w;
    } _q;

private:
    /* Create a quaternion representing a vector */
    explicit Quaternion(const Vector& v);

    /*********************************
     * State-syncronization functions
     *********************************/
    void _update_rotation_matrix() const;
    void _update_axis_angle() const;
    void _update_euler_angles() const;

    void _update_from_rotation_matrix();
    void _update_from_axis_angle();
    void _update_from_euler_angles();


    /**********************
     * Utility Functions
     **********************/
    void _dirty_all();
    void _fix_angles() const;
    void _fix_axis_angle() const;
    Quaternion & _normalize();



    /*
     * A significant portion of the heavy lifting in this class
     * is done by Ken Shoemake's code in Graphics Gems IV for
     * converting between euler/matrix/quaternion.
     * It was originally written in C, and I've adapted it for
     * use in this class.  Part of adapting it is converting 
     * #define constants to enum types and #define macros to 
     * inline functions.  The following block defines these. 
     */
/********** BEGIN SHOEMAKE ADAPTATION *********/
public: 

    static const Euler_axis EulSafe[4];
    static const Euler_axis EulNext[4];

private:
    int _euler_order;
    void _update_euler_order();
    static int _eul_ord(Euler_axis i, Euler_parity p, Euler_repeat r, Euler_frame f);
    static void _eul_get_ord(int ord, Euler_axis &i, Euler_axis &j, Euler_axis &k, Euler_axis&h, Euler_parity &n, Euler_repeat &s, Euler_frame &f);
/********** END SHOEMAKE ADAPTATION *********/

#ifdef KJB_HAVE_BST_SERIAL
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /* version */)
    {

        ar & _euler_mode;
        ar & _q.x;
        ar & _q.y;
        ar & _q.z;
        ar & _q.w;

        if(Archive::is_loading::value == true)
            _update_euler_order();
    }
#endif

};

/**** INLINE DEFINITIONS ******/

inline void Quaternion::_dirty_all()
{
    _rotation_matrix_dirty = true;
    _axis_angle_dirty = true;
    _eulers_dirty = true;
}

// Bit-packs the representation for the  euler-angle convention
// Adapted from Ken Shoemake's EulOrd macro in "Graphic Gems IV"
// This will be used later, to do conversion between eulers,
// matrices, and quaternions.
inline int Quaternion::_eul_ord(Euler_axis i, Euler_parity p, Euler_repeat r, Euler_frame f)
{
    return (((((((i)<<1)+(p))<<1)+(r))<<1)+(f));
}

// Adapted from Ken Shoemake's EulGetOrd macro in "Graphic Gems IV"
inline void Quaternion::_eul_get_ord(int ord, Euler_axis &i, Euler_axis &j, Euler_axis &k, Euler_axis &h, Euler_parity &n, Euler_repeat &s, Euler_frame &f)
{
    unsigned int o=ord;
    f=Euler_frame(o&1);
    o=o>>1;
    s=Euler_repeat(o&1);
    o=o>>1;
    n=Euler_parity(o&1);
    o=o>>1;
    i=EulSafe[o&3];
    j=EulNext[i+n];
    k=EulNext[i+1-n];
    h=s?k:i;
}

inline double norm(const Quaternion& q, unsigned int l = 2)
{
    return q.norm(l);
}

//template <class Iterator>
//Quaternion mean(Iterator begin, Iterator end, const Quaternion& /* dummy */)
//{
//    Quaternion q_sum;
//
//    // first += second + ... + end;
//    q_sum = std::accumulate(
//                begin,
//                end,
//                Quaternion(0.0, 0.0, 0.0, 0.0));
//
//    q_sum /= norm(q_sum, 2);
//    return q_sum;
//}
//
/**
 * Returns the "rotational difference" between q1 and q2.  In other words, this returns the rotation through which q2 must rotate to become q1.
 *
 * diff = difference(q1, q2);
 * assert(q1 == q2 * diff);
 */
inline Quaternion difference(const Quaternion& q1, const Quaternion& q2)
{
    Quaternion result = q2.conj() * q1;
    return result /= norm(result);
}

/** Spherical linear interpolation between two quaternions */
Quaternion slerp(const Quaternion& q1, const Quaternion& q2, double t);

/** An alternative implementation of spherical linear interpolation */
Quaternion slerp2(const Quaternion& q1, const Quaternion& q2, double t);

/**
 * Normalized linear interpolation of two quaternions.  More efficient than slerp, but rotation rate is non-constant.
 */
Quaternion nlerp(const Quaternion& q1, const Quaternion& q2, double t);

/** @brief  Non-member swap function. */
inline void swap(Quaternion& q1, Quaternion& q2) { q1.swap(q2); }

} // namespace kjb
#endif
