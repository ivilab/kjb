
/* $Id: dynamics_moves.h 18278 2014-11-25 01:42:10Z ksimek $ */

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
|  Author: Luca Del Pero
* =========================================================================== */

#ifndef DYNAMICS_MOVES_INCLUDED
#define DYNAMICS_MOVES_INCLUDED

#include <sample_cpp/likelihood_dynamics.h>
#include <st_cpp/st_parapiped.h>
#include <camera_cpp/perspective_camera.h>

namespace kjb {

enum Parapiped_camera_dynamics_params {
    PCD_PARAPIPED_X = 0,
    PCD_PARAPIPED_Y,
    PCD_PARAPIPED_Z,
    PCD_PARAPIPED_WIDTH,
    PCD_PARAPIPED_HEIGHT,
    PCD_PARAPIPED_LENGTH,
    PCD_PARAPIPED_YAW,
    PCD_CAMERA_FOCAL,
    PCD_CAMERA_PITCH,
    PCD_CAMERA_ROLL
};

class Parapiped_camera_dynamics : public Likelihood_dynamics
{
public :
    Parapiped_camera_dynamics
    (
        const kjb::Vector & ideltas,
        const kjb::Vector & ietas,
        double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
        void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
        void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
        double ialpha = 0.99,
        unsigned int ikick = 0,
        bool sample_centre_x = true,
        bool sample_centre_y = true,
        bool sample_centre_z = true,
        bool sample_width = true,
        bool sample_height = true,
        bool sample_length = true,
        bool sample_yaw = true,
        bool sample_focal_length = true,
        bool sample_camera_pitch = true,
        bool sample_camera_roll = true
    );
    void run(unsigned int iterations);


private:
    void log_sample();
    double compute_likelihood();
    Parametric_parapiped local_pp;
    Perspective_camera local_camera;

    boost::function2<double, Parametric_parapiped &, Perspective_camera &> likelihood;
    boost::function2<void, Parametric_parapiped &, Perspective_camera &> get_parameters;
    boost::function3<void, const Parametric_parapiped &, const Perspective_camera &, double> logger;
};

class Parapiped_stretch_dynamics : public Likelihood_dynamics
{
public:
    Parapiped_stretch_dynamics
    (
        const kjb::Vector & ideltas,
        const kjb::Vector & ietas,
        double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
        void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
        void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
        bool idirection1,
        bool idirection2,
        bool idirection3,
        double ialpha = 0.99,
        unsigned int ikick = 0,
        bool sample_width = true,
        bool sample_height = true,
        bool sample_length = true
    );
    void run(unsigned int iterations);

private:
    void log_sample();
    double compute_likelihood();
    Parametric_parapiped local_pp;
    Perspective_camera local_camera;

    bool direction1;
    bool direction2;
    bool direction3;

    void stretch_x(double x_value);
    void stretch_y(double y_value);
    void stretch_z(double z_value);

    boost::function2<double, Parametric_parapiped &, Perspective_camera &> likelihood;
    boost::function2<void, Parametric_parapiped &, Perspective_camera &> get_parameters;
    boost::function3<void, const Parametric_parapiped &, const Perspective_camera &, double> logger;
};

class Focal_scale_dynamics : public Likelihood_dynamics
{
public:
    Focal_scale_dynamics
    (
        const kjb::Vector & ideltas,
        const kjb::Vector & ietas,
        double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
        void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
        void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
        double ialpha = 0.99,
        unsigned int ikick = 0
    );
    void run(unsigned int iterations);

private:
    void log_sample();
    double compute_likelihood();
    Parametric_parapiped local_pp;
    Perspective_camera local_camera;

    void update_focal_with_position(double ifocal);

    boost::function2<double, Parametric_parapiped &, Perspective_camera &> likelihood;
    boost::function2<void, Parametric_parapiped &, Perspective_camera &> get_parameters;
    boost::function3<void, const Parametric_parapiped &, const Perspective_camera &, double> logger;
};

class Discrete_change_size : public Likelihood_dynamics
{
public:
    Discrete_change_size
    (
        unsigned int axis,
        bool direction,
        double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
        void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
        void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
        double ialpha = 0.99,
        unsigned int ikick = 0
    );
    void run(unsigned int iterations = 1);

private:
    void log_sample();
    double compute_likelihood();
    Parametric_parapiped local_pp;
    Perspective_camera local_camera;
    unsigned int axis;
    bool direction;

    boost::function2<double, Parametric_parapiped &, Perspective_camera &> likelihood;
    boost::function2<void, Parametric_parapiped &, Perspective_camera &> get_parameters;
    boost::function3<void, const Parametric_parapiped &, const Perspective_camera &, double> logger;
};

}

#endif


