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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: test_ode_solver.cpp 21462 2017-07-01 20:40:03Z jguan1 $ */

/*
 * Kobus: 2019/06/07
 * Dumping some test code into some test modules for calling C++ from python.
 * Once we are done with that project, we can remove this file. 
*/

#include <l_cpp/l_test.h>

#include <vector>
#include <utility>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/odeint.hpp>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/time_util.h"

using namespace kjb;
using namespace kjb::ties;
using namespace std;

/* int test_ode_solver(int argc, const char** argv) */
int test_ode_solver(void)
{
    /*double freq_0 = -0.09;
    double freq_1 = -0.08;
    double damp_0 = -0.05;
    double damp_1 = -0.04;
    double coupling_01 = 0.01;
    double coupling_10 = -0.02;*/
    double freq_0 = 2.1;
    double freq_1 = 2.2;
    double damp_0 = 0.001;
    double damp_1 = 0.001;
    double coupling_01 = 0.001;
    double coupling_10 = 0.001;

    string out_fp = "./outputs/";
    /*
    if(argc >= 2)
    {
        out_fp = argv[1];
    }
    */
    ETX(kjb_c::kjb_mkdir(out_fp.c_str()));

    size_t time_length = 50;
    Double_v times(time_length);
    vector<size_t> time_indices(times.size());
    for(size_t i = 0; i < times.size(); i++)
    {
        times[i] = i * 1.0;
        time_indices[i] = i;
    }
    size_t i = 0;
    const size_t num_osc = 2;
    State_type init_state(2 * num_osc);
    /*init_state[i++] = 0.5; 
    init_state[i++] = 0.3; 
    init_state[i++] = 0.0; 
    init_state[i++] = 0.0;*/
    init_state[i++] = 9.51864034e-01; 
    init_state[i++] = 1.13013516e+00; 
    init_state[i++] = -3.33313807e-02; 
    init_state[i++] = 9.03557845e-01;
    //init_state[i++] = 0;
    //init_state[i++] = 0;

    //Double_v com_params(12, 0.0);

    int num_params = 8;
    bool use_modal = true;
    if(use_modal) num_params = 6;
    Double_v com_params(num_params, 0.0);
    i = 0;
    if(use_modal)
    {
        com_params[i++] = 1.2; 
        com_params[i++] = 2.2; 
        com_params[i++] = (2 * M_PI)/5.0;
        com_params[i++] = (2 * M_PI)/20.0;;
        com_params[i++] = 0.5;
        com_params[i++] = 0.2;
        /*com_params[i++] = 7.26925379e-01;
        com_params[i++] = 4.28113658e-01;
        com_params[i++] = 1.00219635e-01;
        com_params[i++] = 8.75907266e-01;
        com_params[i++] = 5.22828164e-02;
        com_params[i++] = -1.12191246e-01;*/

    }
    else
    {
        com_params[i++] = (2 * M_PI)/5.0;
        com_params[i++] = (2 * M_PI)/20.0;;
        com_params[i++] = 0.0;
        com_params[i++] = 0.0;
        com_params[i++] = 0.5;
        com_params[i++] = 0.2;
        com_params[i++] = 0.0;
        com_params[i++] = 0.0;
    }
    //Double_v com_params = coupled_oscillator_params(num_osc);

    Vector obs_sigma(1, 0.5);

    std::vector<std::string> obs_names(1);
    obs_names[0] = "dial";
    int poly_degree = -1;
    Coupled_oscillator_v clos(times.size() - 1, 
                        Coupled_oscillator(com_params, use_modal));

    if(use_modal)
    {
        BOOST_FOREACH(Coupled_oscillator& clo, clos)
        {
            clo.update_system_matrix_from_modal();
        }
    }

    Linear_state_space lss(times, 
                           init_state, 
                           clos, 
                           obs_names, 
                           obs_sigma, 
                           poly_degree);
    Coupled_oscillator_v& clos_1 = lss.coupled_oscillators();
    kjb_c::init_real_time();
    const State_vec_vec& states_1 = lss.get_states();
    const State_vec_vec& states_3 = lss.get_states(time_indices);
    long time = kjb_c::get_real_time(); 
    State_vec states_2; 
    State_vec states_4; 
    states_2.push_back(init_state);
    kjb_c::init_real_time();
    //struct timespec start, finish;
    //current_utc_time(&start);
    
    kjb_c::init_real_time();
    for(size_t i = 0; i < 1000; i++)
    {
        integrate_states_matrix_exp(clos_1, 0.0, lss.get_times(), init_state, states_2);
    }
    time = kjb_c::get_real_time(); 
    std::cout << "me takes: " << time/1000.0 << "s\n";

    if(use_modal)
    {
        kjb_c::init_real_time();
        for(size_t i = 0; i < 1000; i++)
        {
            integrate_states_modal(clos_1, 0.0, lss.get_times(), init_state, states_4);
        }
        time = kjb_c::get_real_time(); 
        std::cout << "me takes: " << time/1000.0 << "s\n";
    }

    //current_utc_time(&finish);
    //double time_1 = finish.tv_sec - start.tv_sec;
    //time_1 += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; 
    time = kjb_c::get_real_time(); 
    std::cout << "me takes: " << time/1000.0 << "s\n";

    TEST_TRUE(states_1.size() == states_2.size()); 
    const double thresh = 1.0;
    for(size_t i = 0; i < times.size(); i++)
    {
        for(size_t j = 0; j < states_1[i][0].size(); j++)
        {
            double v1 = states_1[i][0][j];
            double v2 = states_2[i][j];
            double v3 = states_3[i][0][j];
            TEST_TRUE(fabs(v1 - v3) < thresh);
            if(use_modal)
            {
                double v4 = states_4[i][j];
                std::cout << i << " " << v2 << " " << v4 << std::endl;
                TEST_TRUE(fabs(v2 - v4) < thresh);
            }
        }
    }

    // use half of the time 
    Double_v times2(time_length/2);
    vector<size_t> time_indices2(times2.size());
    for(size_t i = 0; i < times2.size(); i++)
    {
        times2[i] = i * 1.0;
        time_indices2[i] = i;
    }
    Linear_state_space lss2(times2, 
                            init_state, 
                            clos, 
                            obs_names, 
                            obs_sigma, 
                            poly_degree);
    State_vec_vec states = lss2.get_states(time_indices2);
    State_vec_vec states_lss2 = lss2.get_states();
    for(size_t i = 0; i < times2.size(); i++)
    {
        for(size_t j = 0; j < states_1[i][0].size(); j++)
        {
            double v1 = states[i][0][j];
            double v3 = states_lss2[i][0][j];
            TEST_TRUE(fabs(v1 - v3) < 1e-5);
        }
    }
    /*State_vec_vec states_pred = lss2.predict_states(times.back(), 1.0);
    std::copy(states_pred.begin(), states_pred.end(), back_inserter(states));

    std::cout << "states1: " << states_1.size() << " states: " << states.size() << std::endl;
    TEST_TRUE(states_1.size() == states.size()); 
    for(size_t i = 0; i < times.size(); i++)
    {
        for(size_t j = 0; j < states_1[i][0].size(); j++)
        {
            double v1 = states_1[i][0][j];
            double v3 = states[i][0][j];
            TEST_TRUE(fabs(v1 - v3) < 1e-5);
        }
    }*/
    lss2.write("./output");
    lss2.update_times(times);
    const State_vec_vec& states_all = lss2.get_states();
    TEST_TRUE(states_1.size() == states_all.size()); 
    for(size_t i = 0; i < times.size(); i++)
    {
        for(size_t j = 0; j < states_1[i][0].size(); j++)
        {
            double v1 = states_1[i][0][j];
            double v3 = states_all[i][0][j];
            //std::cout << i << " " << v1 << " " << v3 << " " << v5 << std::endl;
            TEST_TRUE(fabs(v1 - v3) < 1e-5);
        }
    }
    lss2.write("./output/all_states");

    RETURN_VICTORIOUSLY();
}
