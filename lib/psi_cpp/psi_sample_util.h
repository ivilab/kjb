/* $Id: psi_sample_util.h 10707 2011-09-29 20:05:56Z predoehl $ */
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

#ifndef PSI_SAMPLE_UTIL_V1_H
#define PSI_SAMPLE_UTIL_V1_H

#include <sample_cpp/sample_recorder.h>
#include <limits>

namespace kjb
{
namespace psi
{

/**
 * A "pseudo" recorder; it doesn't record anything, but it outputs
 * the sampler progress after each iteration.
 */

struct Progress_recorder : Null_recorder<double>
{
    Progress_recorder() :
//        progress(num_iterations),
        i(0)
    {}

    template <class Model> // don't care what model type
    void operator()(const Model&, const Step_log& log)
    {
        ++i;
//        ++progress;
        std::cout << "it " << i << std::endl;
        std::cout << "lp " << log[0].lt << std::endl;
        std::cout << "accept " << log[0].accept << std::endl;
    }
//    boost::progress_display progress;
    int i;
};

// the next three functions two functions set up a generic interface
// for the sampler to manipulate it.
inline void update_model_parameter(Model& m, int dimension, double delta)
{
    double value = m.get(dimension);
    value += delta;
    m.set(dimension, value);
}

inline size_t get_model_dimension(const Model& m)
{
    return m.size();
}

inline double get_model_parameter(const Model& m, size_t i)
{
    return m.get(i);
}

inline kjb::Vector get_model_upper_bounds(const Model& m)
{
    const size_t size = m.size();
    const double inf = std::numeric_limits<double>::infinity();

    return kjb::Vector((int) size, inf);
}

inline kjb::Vector get_model_lower_bounds(const Model& m)
{
    KJB(UNTESTED_CODE());
    static const double inf = std::numeric_limits<double>::infinity();
    const size_t size = m.size();

    return kjb::Vector((int) size, -inf);

#if 0
    for(int i = 0; i < size(); i++)
    {
        switch(m.get_units[i])
        {
            case DURATION_UNIT:
                return 0;
            default:
                return -inf;
        }
    }
#endif
}



// This tells us how much to step in each dimension while sampling.
// These will be multiplied by the gradient to get the step size.
class Psi_step_size
{
public:
    Psi_step_size() :
        spacial_step_size_(),
        angle_step_size_(),
        velocity_step_size_(),
        vangle_step_size_(),
        duration_step_size_(),
        length_step_size_(),
        mass_step_size_()
    {}
            
    Psi_step_size(
            double spacial_step,
            double angle_step,
            double velocity_step,
            double angular_velocity_step,
            double duration_step,
            double mass_step,
            double length_step,
            double scale = 1) :
        spacial_step_size_(spacial_step * scale),
        angle_step_size_(angle_step * scale),
        velocity_step_size_(velocity_step * scale),
        vangle_step_size_(angular_velocity_step * scale),
        duration_step_size_(duration_step * scale),
        length_step_size_(length_step * scale),
        mass_step_size_(mass_step * scale)
    {}

    kjb::Vector operator()(const Model& m) const
    {
        kjb::Vector result(m.size());
        for(size_t i = 0; i < m.size(); i++)
        {
            Unit_type unit = m.get_units(i);
            switch(unit)
            {
                case SPACIAL_UNIT:
                    result[i] = spacial_step_size_;
                    break;
                case ANGLE_UNIT:
                    result[i] = angle_step_size_;
                    break;
                case VSPACIAL_UNIT:
                    result[i] = velocity_step_size_;
                    break;
                case VANGLE_UNIT: // angular_velocity
                    result[i] = vangle_step_size_;
                    break;
                case TIME_UNIT: 
                    result[i] = duration_step_size_;
                    break;
                case MASS_UNIT: 
                    result[i] = mass_step_size_;
                    break;
                case LENGTH_UNIT: 
                    result[i] = length_step_size_;
                case DISCRETE_UNIT:  // ???? maybe not a good idea to put it here 
                case UNKNOWN_UNIT: 
                case OTHER_UNIT: 
                case ASPACIAL_UNIT: 
                case AANGLE_UNIT: 
                default:
                    KJB_THROW_2(kjb::Runtime_error, "Unknown parameter type");
            }
            
        }

        return result;
    }

private:
    double spacial_step_size_;
    double angle_step_size_;
    double velocity_step_size_;
    double vangle_step_size_;
    double duration_step_size_;
    double length_step_size_;
    double mass_step_size_;

        
};


} // namespace psi
} // namespace kjb
#endif
