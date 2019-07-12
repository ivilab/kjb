
/* $Id: dynamics_moves.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

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
   Author: Luca Del Pero
* =========================================================================== */

#include "sample_cpp/dynamics_moves.h"

#define DYNAMICS_NUM_PP_DIMENSIONS 3

namespace kjb {

Parapiped_camera_dynamics::Parapiped_camera_dynamics
(
    const kjb::Vector & ideltas,
    const kjb::Vector & ietas,
    double  (*icompute_likelihood)(Parametric_parapiped & pp, Perspective_camera & c),
    void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
    void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
    double ialpha,
    unsigned int ikick,
    bool sample_centre_x,
    bool sample_centre_y,
    bool sample_centre_z,
    bool sample_width,
    bool sample_height,
    bool sample_length,
    bool sample_yaw,
    bool sample_focal_length,
    bool sample_camera_pitch,
    bool sample_camera_roll
) : Likelihood_dynamics(ialpha, ikick), local_pp(), local_camera()
{
    using namespace boost;

    if(ideltas.size() != (PCD_CAMERA_ROLL + 1) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of delta updates for sampling camera and parapiped, expected 9");
    }
    if(ietas.size() != (PCD_CAMERA_ROLL + 1) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of eta updates for sampling camera and parapiped, expected 9");
    }

    unsigned int counter = 0;
    if(sample_centre_x)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_centre_x, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_centre_x, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_X));
        etas.push_back(ietas(PCD_PARAPIPED_X));
        counter++;
    }

    if(sample_centre_y)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_centre_y, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_centre_y, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_Y));
        etas.push_back(ietas(PCD_PARAPIPED_Y));
        counter++;
    }

    if(sample_centre_z)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_centre_z, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_centre_z, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_Z));
        etas.push_back(ietas(PCD_PARAPIPED_Z));
        counter++;
    }

    if(sample_width)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_width, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_width, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_WIDTH));
        etas.push_back(ietas(PCD_PARAPIPED_WIDTH));
        counter++;
    }

    if(sample_height)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_height, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_height, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_HEIGHT));
        etas.push_back(ietas(PCD_PARAPIPED_HEIGHT));
        counter++;
    }

    if(sample_length)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_length, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_length, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_LENGTH));
        etas.push_back(ietas(PCD_PARAPIPED_LENGTH));
        counter++;
    }

    if(sample_yaw)
    {
        callbacks.push_back(bind(&Parametric_parapiped::set_yaw, &local_pp,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_yaw, &local_pp));
        deltas.push_back(ideltas(PCD_PARAPIPED_YAW));
        etas.push_back(ietas(PCD_PARAPIPED_YAW));
        counter++;
    }

    if(sample_focal_length)
    {
        callbacks.push_back(bind(&Perspective_camera::set_focal_length, &local_camera,_1));
        parameter_getters.push_back(bind(&Perspective_camera::get_focal_length, &local_camera));
        deltas.push_back(ideltas(PCD_CAMERA_FOCAL));
        etas.push_back(ietas(PCD_CAMERA_FOCAL));
        counter++;
    }

    if(sample_camera_pitch)
    {
        callbacks.push_back(bind(&Perspective_camera::set_pitch, &local_camera,_1));
        parameter_getters.push_back(bind(&Perspective_camera::get_pitch, &local_camera));
        deltas.push_back(ideltas(PCD_CAMERA_PITCH));
        etas.push_back(ietas(PCD_CAMERA_PITCH));
        counter++;
    }
    if(sample_camera_roll)
    {
        callbacks.push_back(bind(&Perspective_camera::set_roll, &local_camera,_1));
        parameter_getters.push_back(bind(&Perspective_camera::get_roll, &local_camera));
        deltas.push_back(ideltas(PCD_CAMERA_ROLL));
        etas.push_back(ietas(PCD_CAMERA_ROLL));
        counter++;
    }

    set_num_parameters(counter);

    likelihood = bind(icompute_likelihood, _1, _2);
    get_parameters = bind(get_pp_and_camera, _1, _2);
    logger = bind(log_results, _1, _2, _3);
}


void Parapiped_camera_dynamics::run(unsigned int iterations)
{
    get_parameters(local_pp, local_camera);
    Likelihood_dynamics::run(iterations);
}

void Parapiped_camera_dynamics::log_sample()
{
    logger(local_pp, local_camera, likelihood(local_pp, local_camera));
}

double Parapiped_camera_dynamics::compute_likelihood()
{
    return likelihood(local_pp, local_camera);
}

Parapiped_stretch_dynamics::Parapiped_stretch_dynamics
(
    const kjb::Vector & ideltas,
    const kjb::Vector & ietas,
    double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
    void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
    void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
    bool idirection1,
    bool idirection2,
    bool idirection3,
    double ialpha,
    unsigned int ikick,
    bool sample_width,
    bool sample_height,
    bool sample_length
): Likelihood_dynamics(ialpha, ikick), local_pp(), local_camera()
{
    using namespace boost;

    if(ideltas.size() != (DYNAMICS_NUM_PP_DIMENSIONS) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of delta updates for stretching parapiped, expected 9");
    }
    if(ietas.size() != (DYNAMICS_NUM_PP_DIMENSIONS) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of eta updates for stretching parapiped, expected 9");
    }

    direction1 = idirection1;
    direction2 = idirection2;
    direction3 = idirection3;

    unsigned int counter = 0;
    if(sample_width)
    {
        callbacks.push_back(bind(&Parapiped_stretch_dynamics::stretch_x,this,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_width, &local_pp));
        deltas.push_back(ideltas(0));
        etas.push_back(ietas(0));
        counter++;
    }
    if(sample_height)
    {
        callbacks.push_back(bind(&Parapiped_stretch_dynamics::stretch_y,this,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_height, &local_pp));
        deltas.push_back(ideltas(1));
        etas.push_back(ietas(1));
        counter++;
    }
    if(sample_length)
    {
        callbacks.push_back(bind(&Parapiped_stretch_dynamics::stretch_z,this,_1));
        parameter_getters.push_back(bind(&Parametric_parapiped::get_length, &local_pp));
        deltas.push_back(ideltas(2));
        etas.push_back(ietas(2));
        counter++;
    }
    set_num_parameters(counter);

    likelihood = bind(icompute_likelihood, _1, _2);
    get_parameters = bind(get_pp_and_camera, _1, _2);
    logger = bind(log_results, _1, _2, _3);
}

void Parapiped_stretch_dynamics::stretch_x(double x_value)
{
    double amount = x_value - local_pp.get_width();
    local_pp.stretch_along_axis(0,amount,direction1);
}

void Parapiped_stretch_dynamics::stretch_y(double y_value)
{
    std::cout << "New height:" << y_value;
    std::cout << "Old height:" << local_pp.get_height();

    double amount = y_value - local_pp.get_height();
    std::cout << "Amount:" << amount << std::endl;
    std::cout << "Direction: " << direction2 << std::endl;
    local_pp.stretch_along_axis(1,amount,direction2);
}

void Parapiped_stretch_dynamics::stretch_z(double z_value)
{
    double amount = z_value - local_pp.get_length();
    local_pp.stretch_along_axis(2,amount,direction3);
}

void Parapiped_stretch_dynamics::run(unsigned int iterations)
{
    get_parameters(local_pp, local_camera);
    Likelihood_dynamics::run(iterations);
}

void Parapiped_stretch_dynamics::log_sample()
{
    logger(local_pp, local_camera, likelihood(local_pp, local_camera));
}

double Parapiped_stretch_dynamics::compute_likelihood()
{
    return likelihood(local_pp, local_camera);
}

Focal_scale_dynamics::Focal_scale_dynamics
(
    const kjb::Vector & ideltas,
    const kjb::Vector & ietas,
    double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
    void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
    void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
    double ialpha,
    unsigned int ikick
): Likelihood_dynamics(ialpha, ikick), local_pp(), local_camera()
{
    using namespace boost;

    if(ideltas.size() != (1) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of delta updates for sampling focal length over scale, expected 9");
    }
    if(ietas.size() != (1) )
    {
        KJB_THROW_2(Illegal_argument, "Wrong number of eta updates for sampling focal length over scale, expected 9");
    }

    unsigned int counter = 0;

    callbacks.push_back(bind(&Perspective_camera::update_focal_with_scale,&local_camera,_1));
    //callbacks.push_back(bind(&Focal_scale_dynamics::update_focal_with_position,this,_1));
    //callbacks.push_back(bind(&Perspective_camera::set_focal_length,&local_camera,_1));
    parameter_getters.push_back(bind(&Perspective_camera::get_focal_length, &local_camera));
    deltas.push_back(ideltas(0));
    etas.push_back(ietas(0));
    counter++;

    set_num_parameters(counter);

    likelihood = bind(icompute_likelihood, _1, _2);
    get_parameters = bind(get_pp_and_camera, _1, _2);
    logger = bind(log_results, _1, _2, _3);
}

void Focal_scale_dynamics::update_focal_with_position(double ifocal)
{
    ::kjb::update_focal_with_position(local_camera, ifocal, local_pp);
}

void Focal_scale_dynamics::run(unsigned int iterations)
{
    get_parameters(local_pp, local_camera);
    Likelihood_dynamics::run(iterations);
}

void Focal_scale_dynamics::log_sample()
{
    logger(local_pp, local_camera, likelihood(local_pp, local_camera));
}

double Focal_scale_dynamics::compute_likelihood()
{
    return likelihood(local_pp, local_camera);
}

Discrete_change_size::Discrete_change_size
(
    unsigned int iaxis,
    bool idirection,
    double  (*icompute_likelihood)(Parametric_parapiped & pp,Perspective_camera & c),
    void  (*get_pp_and_camera)(Parametric_parapiped & pp, Perspective_camera & c),
    void  (*log_results)(const Parametric_parapiped & pp, const Perspective_camera & c, double ll),
    double ialpha,
    unsigned int ikick
): Likelihood_dynamics(ialpha, ikick), local_pp(), local_camera()
{
    using namespace boost;

    axis = iaxis;
    direction = idirection;

    likelihood = bind(icompute_likelihood, _1, _2);
    get_parameters = bind(get_pp_and_camera, _1, _2);
    logger = bind(log_results, _1, _2, _3);
}

void Discrete_change_size::run(unsigned int /* iterations */)
{
    get_parameters(local_pp, local_camera);
    /* double best_size; */
    double start_size = local_pp.get_width();
    if(axis == 1)
    {
        start_size = local_pp.get_height();
    }
    else if(axis == 2)
    {
        start_size = local_pp.get_length();
    }
    double start_ratio = 0.2;//0.3
    double number_of_iterations = 20;//9
    bool found = false;
    double best_ll = compute_likelihood();

    Parametric_parapiped temp_pp = local_pp;
    double new_size = start_size*start_ratio;
    double step_size = start_size*0.05;//0.1
    for(unsigned int i =0; i < number_of_iterations; i++)
    {
        double stretch_size = step_size;
        if(!found)
        {
            stretch_size = new_size - start_size;
            found = true;
        }
        std::cout << stretch_size << std::endl;
        local_pp.stretch_along_axis(axis, stretch_size, direction);
        double ll = compute_likelihood();
        std::cout << ll << std::endl;
        if(ll > best_ll)
        {
            temp_pp = local_pp;
            best_ll = ll;
        }
    }
    local_pp = temp_pp;
    log_sample();
}

void Discrete_change_size::log_sample()
{
    logger(local_pp, local_camera, likelihood(local_pp, local_camera));
}

double Discrete_change_size::compute_likelihood()
{
    return likelihood(local_pp, local_camera);
}

} // namespace kjb 

