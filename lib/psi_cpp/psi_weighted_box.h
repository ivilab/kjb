/* $Id: psi_weighted_box.h 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#ifndef  KJB_PSI_WEIGHTED_BOX_H
#define KJB_PSI_WEIGHTED_BOX_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "g_cpp/g_quaternion.h"
#include "psi_cpp/psi_util.h"
#include "psi_cpp/psi_units.h"

#include <boost/io/ios_state.hpp>

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <Control/BoxState.h>
#endif

namespace kjb
{
namespace psi
{

/**
 * Simple mathematical representation of a 3D cuboid.
 */
class Cuboid
{
public: 
    Cuboid() :
        center_(3, 0.0),
        size_(3, 0.0),
        orientation_()
    {}

    Cuboid(
            const Vector& center,
            const Vector& size,
            const Quaternion& orientation) :
        center_(center),
        size_(size),
        orientation_(orientation)
    {}

    const Vector& get_center() const { return center_; }
    const Vector& get_size() const { return size_; }
    const Quaternion& get_orientation() const { return orientation_; }

    void set_center(const Vector& center) { center_ = center; }
    void set_size(const Vector& size) { size_ = size; }
    void set_orientation(const Quaternion& orientation) { orientation_ = orientation; }
protected:
    Vector center_;
    Vector size_;
    Quaternion orientation_;
};

std::vector<Vector> get_corners(const Cuboid& c);
void render(const Cuboid& c);

#ifdef KJB_HAVE_UA_CARTWHEEL
inline Cuboid to_cuboid(const CartWheel::BoxState& cw_box)
{
    return Cuboid(
            to_kjb(cw_box.getPosition()),
            to_kjb(cw_box.getSize()),
            to_kjb(cw_box.getQuaternion()));
}

#endif

/**
 * Representation of a physical box, for simulation
 */
struct Weighted_box : public Cuboid
{
typedef Cuboid Base;
public:
    Weighted_box() :
        Base(Vector(0.0,0.0,0.0), Vector(0.0,0.0,0.0), Quaternion()),
        mass_(0.0)
    {}

    Weighted_box(const kjb::Vector& position, double rotation, const kjb::Vector& size, double mass) :
        Base(position, size, Quaternion(Vector(0.0, 1.0, 0.0), rotation)),
        mass_(mass)
    {}

    double get_mass() const { return mass_; }
    void set_mass(double mass) { mass_ = mass; }

    // return number of parameters
    static size_t size() { return 7; }
    static Unit_type get_units(size_t i)
    {
        switch(i)
        {
            case 0:
            case 1:
                return SPACIAL_UNIT;
            case 2:
                return ANGLE_UNIT;
            case 3:
            case 4:
            case 5:
                return LENGTH_UNIT;
            case 6:
                return MASS_UNIT;
            default:
                KJB_THROW(Index_out_of_bounds);
        }
    }

    /// getter method for sampler
    /// @note length and mass parameters are returned in log-space
    double get(size_t i) const
    {
        switch(i)
        {
            case 0:
                return center_[0];
            case 1:
                return center_[2];
            case 2:
                return orientation_.get_angle();
            case 3:
                return log(size_[0]);
            case 4:
                return log(size_[1]);
            case 5:
                return log(size_[2]);
            case 6:
                return log(mass_);
            default:
                KJB_THROW(Index_out_of_bounds);
            
        }
    }

    /// setter interface for sampler
    /// @note length and mass inputs are assumed to be in log-space
    void set(size_t i, double value)
    {
        switch(i)
        {
            case 0:
                center_[0] = value;
                return;
            case 1:
                center_[2] = value;
                return;
            case 2:
                orientation_.set_axis_angle(Vector(0.0, 1.0, 0.0), value);
                return;
            case 3:
                size_[0] = exp(value);
                return;
            case 4:
                size_[1] = exp(value);
                center_[1] = size_[1]/2.0 + 0.01;
                return;
            case 5:
                size_[2] = exp(value);
                return;
            case 6:
                mass_ = exp(value);
                return;
            default:
                KJB_THROW(Index_out_of_bounds);
            
        }
    }

    friend std::ostream& operator<<(std::ostream& ost, const Weighted_box& box);
    friend std::istream& operator>>(std::istream& ist, Weighted_box& box);
private:
    double mass_;
};

inline std::ostream& operator<<(std::ostream& ost, const Weighted_box& box)
{
    ASSERT(box.center_.size() == 3);
    ost << box.center_[0] << " ";
    ost << box.center_[1] << " ";
    ost << box.center_[2] << " ";

    ost << box.orientation_.get_angle() << " ";

    ASSERT(box.size_.size() == 3);
    ost << box.size_[0] << " ";
    ost << box.size_[1] << " ";
    ost << box.size_[2] << " ";

    ost << box.mass_;

    return ost;
}

inline std::istream& operator>>(std::istream& ist, Weighted_box& box)
{
    // this will restore the "skipws" state after leaving scope
    boost::io::ios_flags_saver  ifs( ist );
    ist >> std::skipws;

    box.center_.resize(3);
    ist >> box.center_[0];
    ist >> box.center_[1];
    ist >> box.center_[2];

    double angle;

    ist >> angle;

    box.orientation_.set_axis_angle(Vector(0.0, 1.0, 0.0), angle);

    box.size_.resize(3);
    ist >> box.size_[0];
    ist >> box.size_[1];
    ist >> box.size_[2];

    ist >> box.mass_;

    return ist;
}

inline Weighted_box parse_cli_weighted_box(const std::string& line)
{
    Weighted_box box;

    std::istringstream ist(line);
    ist >> box;

    return box;
}

} // namespace psi
} // namespace kjb
#endif
