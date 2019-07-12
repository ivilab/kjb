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

/* $Id */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_exception.h>
#include <gp_cpp/gp_mean.h>
#include <m_cpp/m_vector.h>
#include <g_cpp/g_line.h>

#ifdef KJB_HAVE_ERGO
#include <ergo/mh.h>
#include <ergo/record.h>
#else
#error "Need ERGO library"
#endif

#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "dbn_cpp/data.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/options.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/experiment.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/base_line_models.h"
#include "dbn_cpp/sample_lss.h"
#include "dbn_cpp/proposer.h"
#include "dbn_cpp/thread_worker.h"


using namespace kjb;
using namespace kjb::ties;

double kjb::ties::line_fitting
(
    const std::vector<Vector>& points, 
    Line& fitted_line
)
{
    double error = 0.0;

    // Fit a line to all points in the consensus_set 
    // using least squares 

    double sum_x = 0.0; 
    double sum_y = 0.0; 
    double sum_x_square = 0.0;
    double sum_xy = 0.0;
    size_t num_points = points.size();
    for (size_t i = 0; i < num_points; i++)
    {
        double x = points[i](0);
        double y = points[i](1);
        sum_x += x;
        sum_y += y;
        sum_x_square += x*x;
        sum_xy += x*y;
    }
    double denominator = (num_points*sum_x_square) - (sum_x*sum_x);
    if (fabs(denominator) < FLT_EPSILON)
    {
        // The line is perpendicular to the x axis
        fitted_line.set_a(0.0);
        fitted_line.set_b(-1.0);
        // Use the average x 
        fitted_line.set_c(sum_x/num_points);

        // compute the error 
        // Here we use the difference along the x axis 
        for(size_t i = 0; i < num_points; i++)
        {
            double line_x = fitted_line.get_c();
            error += (points[i](0) - line_x) * (points[i](0) - line_x);
        }
    }
    else
    {
        double intercept = ((sum_x_square*sum_y) - (sum_x*sum_xy))/denominator;
        double slope = (num_points*sum_xy - sum_x*sum_y)/denominator;
        fitted_line.set_a(slope);
        fitted_line.set_b(-1.0);
        fitted_line.set_c(intercept);

        // compute the error
        for(size_t i = 0; i < num_points; i++)
        {
            double x = points[i](0);
            double y_p = points[i](1);
            double y_at_x = fitted_line.compute_y_coordinate(x);
            //error += fabs(y_p - y_at_x);
            error += (y_p - y_at_x) * (y_p - y_at_x);
        }
    }
    //error /= num_points;
    error = std::pow((error/num_points), 0.5);
    return error;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::average_model_fitting
(
    const std::vector<Data>& data,
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent,
    size_t num_obs
)
{
    using namespace std;
    boost::format out_fmt(output_dir + "/" + obs_str + "/%04d/");

    // hack to make Condor respect the output
    ETX(kjb_c::kjb_mkdir(output_dir.c_str()));
    std::ofstream touch((output_dir + DIR_STR + "invocation.txt").c_str());
    touch << "Program invocation:\n" << " average_model_fitting " << ' ' 
          << output_dir << ' ' << obs_str << '\n';

    size_t num_lss = data.size();
    std::vector<Vector> all_errors(num_lss);

    std::cout << "average_model_fitting (num_lss="<< num_lss << ")" << std::endl;
    boost::progress_display progress(num_lss);
    for (size_t d = 0; d < num_lss; d++)
    {
        const Double_v& cur_times = data[d].times;
        Obs_map::const_iterator it = data[d].observables.find(obs_str);
        if(it == data[d].observables.end())
        {
            std::cerr << " Invalid observable " << obs_str << std::endl;
            return;
        }

        const Vector_v& obs = it->second; 
        size_t num_times = cur_times.size();
        if(num_times == 0) continue;
        size_t num_oscillators = obs.size();

        std::vector<double> aves(num_oscillators, 0.0); 
        size_t N = cur_times.size();
        size_t tN = N * training_percent; 
        std::vector<size_t> dN(num_oscillators, 0);
        for(size_t i = 0; i < tN; i++)
        {
            for(size_t j = 0; j < num_oscillators; j++)
            {
                if(!invalid_data(obs[j][i]))
                {
                    aves[j] += obs[j][i];
                    dN[j]++;
                }
            }
        }
        for(size_t i = 0; i < aves.size(); i++)
        {
            aves[i] /= dN[i];
        }

        std::string res_dir = (out_fmt % data[d].dyid).str();
        ETX(kjb_c::kjb_mkdir(res_dir.c_str()));

        std::string state_fp = res_dir + "/states.txt";
        std::ofstream state_ofs(state_fp.c_str());
        IFTD(state_ofs.is_open(), IO_error, "Can't open %s\n", (state_fp.c_str()));
        std::string params_fp = res_dir + "/com_params.txt";
        std::ofstream params_ofs(params_fp.c_str());
        IFTD(params_ofs.is_open(), IO_error, "Can't open %s\n", (params_fp.c_str()));
        size_t num_params = param_length(num_oscillators);

        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = 0; j < num_oscillators; j++)
            {
                state_ofs << setw(20) << setprecision(8) << aves[j];
            }
            for (size_t j = 0; j < num_oscillators; j++)
            {
                state_ofs << setw(20) << setprecision(8) << "0";
            }
            state_ofs << std::endl;

            if(i < data[d].times.size() - 1)
            {
                for(size_t j = 0; j < num_params; j++)
                {
                    params_ofs << "1 ";
                }
                params_ofs << std::endl;
            }
        }
        // obs.txt
        std::string obs_fp = res_dir + "/obs.txt";
        std::ofstream obs_ofs(obs_fp.c_str());
        IFTD(obs_ofs.is_open(), IO_error, "Can't open %s\n", (obs_fp.c_str()));
        obs_ofs << obs_str << std::endl;
        std::vector<std::vector<Vector> > obs_coefs(1, 
                                      std::vector<Vector>((int)num_oscillators, Vector(1, 1.0)));
        for(size_t i = 0; i < obs_coefs.size(); i++)
        {
            for(size_t j = 0; j < obs_coefs.size(); j++)
            {
                obs_ofs << obs_coefs[i][j] << endl;
            }
        }

        // Compute the error of current couple
        Linear_state_space lss;
        lss.read(res_dir);
        lss.changed_index() = lss.get_times().size() - 1;
        std::vector<Vector> obs_errors;
        all_errors[d] = compute_ave_error(data[d], lss, obs_errors, training_percent);
        std::string err_fp(res_dir + "/rms_error.txt");
        std::ofstream ofs(err_fp.c_str());
        IFTD(ofs.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
        ofs << data[d].dyid << " " << all_errors[d] << std::endl;
        plot_data_and_model(data[d], lss, res_dir);
        lss.write(std::string(res_dir + "/all_states"));
        ++progress;
    }

    // Compute the mean and variance of the errors of all couples 
    std::string error_out(output_dir + "/errors");
    ETX(kjb_c::kjb_mkdir(error_out.c_str()));

    if (!all_errors.empty())
    {
        std::string err_fp(error_out + "/err_couples.txt");
        ofstream err_of(err_fp.c_str());
        IFTD(err_of.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
        for(size_t i = 0; i < all_errors.size(); i++)
        {
            err_of << setw(2) << data[i].dyid << " " << all_errors[i] << std::endl;
        }

        std::string err_stat_fp(error_out + "/" + obs_str + "_err_ave.txt");
        compute_and_record_errors(all_errors, distinguisher, err_stat_fp);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::line_model_fitting
(
    const std::vector<Data>& data, 
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent,
    size_t num_obs
)
{
    using namespace std;
    boost::format out_fmt(output_dir + "/" + obs_str + "/%04d/");

    // hack to make Condor respect the output
    ETX(kjb_c::kjb_mkdir(output_dir.c_str()));
    std::ofstream touch((output_dir + DIR_STR + "invocation.txt").c_str());
    touch << "Program invocation:\n" << "line_model_fitting" << ' ' 
          << output_dir << ' ' << obs_str << '\n';

    // Read in data
    size_t num_lss = data.size();
    std::vector<Vector> all_errors(num_lss);

    for(size_t d = 0; d < num_lss; d++)
    {
        Obs_map::const_iterator it = data[d].observables.find(obs_str);
        if(it == data[d].observables.end())
        {
            std::cerr << " Invalid observable " << obs_str << std::endl;
            return;
        }

        // Create vectors of data
        size_t num_oscillators = it->second.size();
        std::vector<std::vector<Vector> > points(num_oscillators);

        // Using the first 70% of the data for fitting the line parameters
        size_t num_points = data[d].times.size() * training_percent;

        for(size_t i = 0; i < num_oscillators; i++)
        {
            for(size_t j = 0; j < num_points; j++)
            {
                if(!invalid_data(it->second[i][j]))
                {
                    points[i].push_back(Vector(data[d].times[j], it->second[i][j]));
                }
            }
        }

        std::vector<Line> lines(num_oscillators);
        for(size_t i = 0; i < num_oscillators; i++)
        {
            line_fitting(points[i], lines[i]);
        }

        std::string res_dir = (out_fmt % data[d].dyid).str();
        ETX(kjb_c::kjb_mkdir(res_dir.c_str()));

        std::string state_fp = res_dir + "/states.txt";
        ofstream state_ofs(state_fp.c_str());
        IFTD(state_ofs.is_open(), IO_error, "Can't open %s\n", (state_fp.c_str()));
        std::string params_fp = res_dir + "/com_params.txt";
        ofstream params_ofs(params_fp.c_str());
        IFTD(params_ofs.is_open(), IO_error, "Can't open %s\n", (params_fp.c_str()));

        size_t num_params = param_length(num_oscillators);
        for(size_t i = 0; i < data[d].times.size(); i++)
        {
            double x_value = data[d].times[i];
            for(size_t j = 0; j < num_oscillators; j++)
            {
                double model_value = lines[j].compute_y_coordinate(x_value);
                state_ofs << setw(20) << setprecision(8) << model_value;
            }
            for(size_t j = 0; j < num_oscillators; j++)
            {
                state_ofs << " 0 "; 
            }
            state_ofs << std::endl;
            if(i < data[d].times.size() - 1)
            {
                for(size_t j = 0; j < num_params; j++)
                {
                    if(j < num_oscillators)
                    {
                        params_ofs << "1 ";
                    }
                    else
                    {
                        params_ofs << "0 ";
                    }
                }
                params_ofs << std::endl;
            }
        }

        // obs.txt
        std::string obs_fp = res_dir + "/obs.txt";
        std::ofstream obs_ofs(obs_fp.c_str());
        IFTD(obs_ofs.is_open(), IO_error, "Can't open %s\n", (obs_fp.c_str()));
        obs_ofs << obs_str << std::endl;
        std::vector<std::vector<Vector> > obs_coefs(1, 
                                      std::vector<Vector>((int)num_oscillators, Vector(1, 1.0)));
        for(size_t i = 0; i < obs_coefs.size(); i++)
        {
            for(size_t j = 0; j < obs_coefs.size(); j++)
            {
                obs_ofs << obs_coefs[i][j] << endl;
            }
        }

        // compute the error of current couple
        Linear_state_space lss;
        lss.read(res_dir);
        lss.changed_index() = lss.get_times().size() - 1;
        std::vector<Vector> obs_errors;
        all_errors[d] = compute_ave_error(data[d], lss, obs_errors, training_percent);
        std::string err_fp(res_dir + "/rms_error.txt");
        ofstream ofs(err_fp.c_str());
        IFTD(ofs.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
        ofs << data[d].dyid << " " << all_errors[d] << std::endl;

        // plot the data and linear state space
        plot_data_and_model(data[d], lss, res_dir);
        lss.write(std::string(res_dir + "/all_states"));
    }

    // compute the mean and variance of the errors of all couples 
    std::string error_out(output_dir + "/errors");
    ETX(kjb_c::kjb_mkdir(error_out.c_str()));
    if(!all_errors.empty())
    {
        std::string err_fp(error_out + "/err_couples.txt");
        ofstream err_of(err_fp.c_str());
        IFTD(err_of.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
        for(size_t i = 0; i < all_errors.size(); i++)
        {
            err_of << setw(2) << data[i].dyid << " " 
                   << all_errors[i] << std::endl;
        }

        std::string err_stat_fp(error_out + "/" + obs_str + "_err_line.txt");
        compute_and_record_errors(all_errors, distinguisher, err_stat_fp);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::lss_mh_fitting
(
    const std::vector<Data>& data,  
    const Ties_experiment& exp
)
{
    using namespace std;
    try
    {
        size_t num_lss = data.size();
        std::vector<Vector> all_errors(num_lss); 
        std::vector<Vector> all_sampled_errors(num_lss);
        std::vector<std::vector<Vector> > all_obs_errors(num_lss);
        std::vector<std::vector<Vector> > all_obs_sampled_errors(num_lss);
        boost::format out_fmt(exp.out_dp + "/%04d");
        for(size_t d = 0; d < num_lss; d++)
        {
            lss_mh_fitting(data[d], 
                           exp, 
                           all_errors[d], 
                           all_sampled_errors[d],
                           all_obs_errors[d], 
                           all_obs_sampled_errors[d]);
        }

        //////////////////////////////////////////////////////
        //  Output the error statistics 
        //////////////////////////////////////////////////////
        std::string error_out(exp.out_dp + "/errors");
        ETX(kjb_c::kjb_mkdir(error_out.c_str()));
        if(!all_errors.empty())
        {
            std::string err_fp(error_out + "/err_couples.txt");
            write_couple_error(err_fp, all_errors, data);

            std::string err_stat_fp(error_out + "/err_ind-clo_" 
                                              + exp.likelihood.obs_names[0] 
                                              + ".txt");
            compute_and_record_errors(all_errors, 
                                      exp.data.distinguisher, 
                                      err_stat_fp);
        }
        if(!all_sampled_errors.empty())
        {
            std::string sample_err_fp(error_out + "/sampled_err_couples.txt");
            write_couple_error(sample_err_fp, all_sampled_errors, data);

            std::string err_stat_fp(error_out + "/sampled_err_ind-clo_"
                                              + exp.likelihood.obs_names[0] 
                                              + ".txt");
            compute_and_record_errors(all_sampled_errors, 
                                      exp.data.distinguisher, 
                                      err_stat_fp);
        }

        //////////////////////////////////////////////////////
        //  Output the error statistics for each observation
        //////////////////////////////////////////////////////
        // rearragne the observable errors
        size_t num_obs = all_obs_errors[0].size();
        if(num_obs > 1)
        {
            for(size_t i = 0; i < num_obs; i++)
            {
                std::vector<Vector> all_obs_errors_t(num_lss);
                std::vector<Vector> all_obs_sampled_errors_t(num_lss);
                for(size_t j = 0; j < num_lss; j++)
                {
                    all_obs_errors_t.push_back(all_obs_errors[j][i]);
                    all_obs_sampled_errors_t.push_back(
                                            all_obs_sampled_errors[j][i]);
                }

                std::string obs_err_fp(error_out + "/" + exp.likelihood.obs_names[i] 
                                                       + "_err_couples.txt");
                write_couple_error(obs_err_fp, all_obs_errors_t, data);

                std::string err_stat_fp(error_out + "/" + exp.likelihood.obs_names[i] 
                                                        + "_err_summary.txt");
                compute_and_record_errors(all_obs_errors_t, 
                                          exp.data.distinguisher, 
                                          err_stat_fp);

                std::string err_stat_latex_fp(error_out + "/" + 
                                              exp.likelihood.obs_names[i] + 
                                              "_err_summary_means.txt");
                compute_and_record_errors_latex(all_obs_sampled_errors_t,
                                         err_stat_latex_fp);

            }
        }


    }
    catch(Option_exception& e)
    {
        cerr << e.get_msg() << endl;
    }
    catch(IO_error& e)
    {
        cerr << e.get_msg() << endl;
    }
    catch(Exception& e)
    {
        cerr << "Uncaught exception: " << e.get_msg() << endl;
    }
    catch(exception& e)
    {
        cerr << "Uncaught exception: " << e.what() << endl;
    }
    catch(boost::exception& e)
    {
        cerr << "Uncaught exception: "
            << boost::diagnostic_information(e) << endl;
    }
    catch(...)
    {
        cerr << "Unknown error occurred" << endl;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::lss_mh_fitting
(
    const Data& data,
    const Ties_experiment& exp,
    Vector& errors,
    Vector& sampled_errors,
    std::vector<Vector>& obs_errors,
    std::vector<Vector>& obs_sampled_errors
)
{
    // check if the option obs_name is in the data
    for(size_t i = 0; i < exp.likelihood.obs_names.size(); i++)
    {
        const string& name = exp.likelihood.obs_names[i];
        if(data.observables.find(name) == data.observables.end())
        {
            KJB_THROW_3(Illegal_argument, 
                        "Invalid observable %s", (name.c_str()));
        }
    }

    // create Linear_state_space 
    Linear_state_space cur_lss;
    if(exp.run.in_dp != "")
    {
        cur_lss.read(exp.run.in_dp);
    }
    else
    {
        size_t length = std::ceil(data.times.size() * 
                                exp.likelihood.training_percent);
        Double_v times(length);
        if(length == 0)
        {
            std::cerr << "Couple " << data.dyid << 
                         " does not contain any time series.\n";
            return;
        }
        double start_time = data.times.front();
        std::generate(times.begin(), times.end(), Increment<double>(start_time));

        int num_oscillators = data.observables.begin()->second.size();
     
        bool random = true;
        Coupled_oscillator_v clos(times.size() - 1, 
                Coupled_oscillator(num_oscillators,
                                   exp.lss.init_period,
                                   exp.lss.init_damping,
                                   exp.lss.use_modal,
                                   random));

        // Use the dial values as the initial values
        State_type init_state;
        if(exp.lss_state_dp != "")
        {
            Linear_state_space init_state_lss;
            init_state_lss.read(exp.lss_state_dp);
            init_state = init_state_lss.init_state();
        }
        else
        {
            Obs_map::const_iterator it = data.observables.begin();
            KJB(ASSERT(it != data.observables.end()));
            State_type mean_state;
            if(exp.lss.polynomial_degree >= 0)
            {
                std::pair<Vector, Vector> mean_vars = 
                                get_mean_variances(data, it->first);
                mean_state = mean_vars.first;
            }
            init_state = estimate_init_states(
                                         data, 
                                         exp.likelihood.training_percent,
                                         it->first,
                                         mean_state);
        }

        Vector obs_sigmas((int)exp.likelihood.obs_names.size(), 
                          exp.likelihood.init_noise_sigma);
        cur_lss = Linear_state_space(times, 
                                     init_state, 
                                     clos, 
                                     exp.likelihood.obs_names,
                                     obs_sigmas,
                                     exp.lss.polynomial_degree,
                                     exp.data.outcomes,
                                     data.outcomes);
        if(exp.lss.polynomial_degree >= 0)
        {
            std::string obs = data.observables.begin()->first;
            std::pair<Vector, Vector> stats = get_mean_variances(data, obs);
            for(size_t m = 0; m < exp.lss.num_oscillators; m++)
            {
                cur_lss.set_polynomial_coefs(m, 0, stats.first[m]);
            }
        }
        if(exp.lss.drift)
        {
            const Coupled_oscillator& co = cur_lss.coupled_oscillators()[0];
            size_t num_params = cur_lss.num_clo_params();
            // DEBUG TODO for drift version without zero coupling terms
            assert(co.num_params() == num_params);
            std::vector<gp::Constant> gp_means(num_params, gp::Constant(0));
            std::copy(co.cbegin(), co.cend(), gp_means.begin());
            Double_v gp_sigvars((int)num_params, exp.prior.clo_sigma);
            Double_v gp_scales((int)num_params, exp.prior.gp_scale);
            cur_lss.init_gp(gp_scales, gp_sigvars, gp_means);
        }
    }

    // Default init-state prior
    Init_prior init_prior(exp.prior.init_state_mean, 
                          exp.prior.init_state_sigma,
                          exp.lss.num_oscillators);

    // cluster prior
    Cluster_prior cluster_prior(exp.cluster.group_lambda, exp.cluster.num_groups);

    // noise prior 
    size_t D = exp.likelihood.obs_names.size(); 
    std::vector<double> noise_shape(D, exp.prior.obs_noise_shape);
    std::vector<double> noise_scale(D, exp.prior.obs_noise_scale);
    Shared_noise_prior noise_prior(noise_shape, noise_scale);
    // Default Lss prior
    bool use_lss_prior = false;

    std::vector<size_t> person_indices;
    size_t num_params = cur_lss.num_clo_params(); 
    std::vector<Group_params> group_params(exp.cluster.num_groups);
    // read in shared priors if available 
    if(exp.prior.prior_fp != "")
    {
        use_lss_prior = true;
        cur_lss.init_predictors(data.moderators, 
                                exp.lss_set.get_all_moderators(
                                    exp.lss.ignore_clo,
                                    exp.lss.num_oscillators,
                                    exp.lss.num_params));

        std::ifstream prior_fs(exp.prior.prior_fp.c_str());
        IFTD(prior_fs.is_open(), IO_error, "can't open file %s", 
                (exp.prior.prior_fp.c_str()));

        // parse in the prior distributions
        for(size_t g = 0; g < exp.cluster.num_groups; g++)
        {
            parse_shared_params(prior_fs, group_params[g]);
            std::cout << group_params[g] << std::endl;
        }

        size_t group_index = cur_lss.group_index();
        size_t num_poly_coefs = cur_lss.num_polynomial_coefs();
        const std::vector<Vector>& preds = cur_lss.get_predictors();
        // means and sigmas
        for(size_t i = 0; i < group_params[group_index].pred_coefs.size(); i++)
        {
            double val = dot(preds[i], group_params[group_index].pred_coefs[i]);
            if(i < exp.lss.num_params)
            {
                if(cur_lss.allow_drift())
                {
                    cur_lss.set_gp_mean(i, gp::Constant(val));
                    cur_lss.set_gp_sigvar(i, 
                            group_params[group_index].variances[i]);
                }
                else
                {
                    cur_lss.set_clo_mean(i, val);
                    cur_lss.set_clo_variance(i, 
                            group_params[group_index].variances[i]);
                }
            }
            else if(i < exp.lss.num_params + num_poly_coefs)
            {
                size_t index = i - exp.lss.num_params;
                size_t osc_index = index / cur_lss.polynomial_dim_per_osc();
                size_t poly_index = index % cur_lss.polynomial_dim_per_osc(); 
                cur_lss.set_polynomial_coefs_mean(osc_index, poly_index, val);
                cur_lss.set_polynomial_coefs_var(osc_index, poly_index, 
                        group_params[group_index].variances[i]);
            }
            else
            {
                size_t index = i - exp.lss.num_params - num_poly_coefs;
                cur_lss.set_outcome_mean(index, val);
                double var_val = group_params[group_index].variances[i];
                cur_lss.set_outcome_var(index, var_val);
            }
        }
    }

    if(exp.run.obs_coef_fp != "")
    {
        cur_lss.parse_obs_coef(exp.run.obs_coef_fp);
    }

    // likelihood
    //double s_sigma = exp.likelihood.init_noise_sigma * 10.0;
    //size_t s_length = (size_t)std::ceil(6 * s_sigma - 1);
    //Data sdata = smooth_data(data, s_length, s_sigma);
    Likelihood likelihood(data, exp.run.num_sampled_length);

    // posterior
    bool use_init_prior = true;
    bool use_likelihood = true;
    bool use_pred = false;
    bool use_drift = exp.lss.drift ? true : false;
    bool use_group_prior = false;
    Posterior posterior(likelihood, 
                        init_prior, 
                        use_init_prior,
                        use_lss_prior, 
                        use_group_prior,
                        use_drift, 
                        use_likelihood,
                        use_pred);

    // Create vectors of Linear state space and posteriors 
    // to use Mh_step_thread
    std::vector<Linear_state_space> lss_vec;
    lss_vec.push_back(cur_lss);
    Linear_state_space final_best(cur_lss);
    std::vector<Posterior> posteriors;
    posteriors.push_back(posterior);
    size_t num_samples = 100;
    std::vector<std::vector<Linear_state_space> > samples(1);
    // set up the output dirs 
    std::vector<std::string> lss_out_dirs(1);
    boost::format out_fmt(exp.out_dp + "/%04d");
    string sub_out_dp = (out_fmt % data.dyid).str();
    lss_out_dirs[0] = sub_out_dp;
    ETX(kjb_c::kjb_mkdir(lss_out_dirs[0].c_str()));

    bool optimize = true;
    bool sample_state = true;
    bool sample_clo = true;
    bool sample_poly_terms = true;

    // Create a time-invariant CLO sampler
    double max_run_seconds = (exp.run.maximum_running_minutes * 60.0) * 0.90;
    bool sample_cluster = exp.run.sample_cluster && use_lss_prior ? true : false;
    std::cout << "sample_cluster: " << sample_cluster << std::endl;
    Mh_step_thread person_thrd(lss_vec, posteriors, lss_out_dirs, 
            exp.run.state_prop_sigma, 
            exp.run.clo_param_prop_sigma,
            exp.run.poly_term_prop_sigma,
            samples,
            !exp.not_record,
            !exp.not_write_trace,
            optimize,
            cluster_prior,
            group_params,
            exp.run.sample_cluster,
            exp.run.adapt_mh,
            num_samples,
            sample_state,
            sample_clo,
            sample_poly_terms,
            exp.run.mh_converge_iter,
            max_run_seconds,
            exp.run.write_samples,
            exp.run.write_proposals);

    // create a drift sampler
    Drift_step_thread drift_thrd(
        lss_vec,
        posteriors,
        lss_out_dirs, 
        exp.run.state_prop_sigma, 
        exp.run.poly_term_prop_sigma,
        samples,
        exp.drift_sampler.ctr_pt_length,
        exp.drift_sampler.num_burn_iters, 
        exp.drift_sampler.num_sample_iters,
        exp.not_record,
        optimize,
        num_samples,
        sample_state,
        sample_poly_terms,
        exp.run.mh_converge_iter,
        max_run_seconds
    );

    // run the thread
    boost::exception_ptr err;
    double best_lp = -DBL_MAX;
    double pre_best_lp = best_lp;
    kjb_c::init_real_time();
    double lp = 0.0;

    std::cout << "lss_mh_fitting (exp.num_iterations="<<exp.num_iterations<<")"<<std::endl;
    boost::progress_display progress_bar(exp.num_iterations);
    for (size_t i = 0; i < exp.num_iterations; i++)
    {
        lss_vec[0] = cur_lss;
        lp = posteriors[0](lss_vec[0]);
        std::cout << "iter " << i <<  " : " << lp << std::endl;
        // reseed random seed
        kjb_c::kjb_seed_rand_2_with_tod();
        long ltime = time(NULL);
        int stime = (unsigned) ltime/2;  
        srand(stime);
        ergo::global_rng<boost::mt19937>().seed(stime);

        // generate a sample from the prior 

        std::cout << "iter: " << i << " clo_params: " << lss_vec[0] << std::endl;
        try
        {
            if(exp.lss.drift)
            {
                run_threads(drift_thrd, 1, 1, err);
            }
            else
            {
                run_threads(person_thrd, 1, 1, err);
            }
        }
        catch(boost::exception& exp)
        {
            std::cerr << boost::diagnostic_information(exp);
        }
        
        lp = posteriors[0](lss_vec[0]);
        if (lp > best_lp)
        {
            best_lp = lp; 
            final_best = lss_vec[0];
        }
        final_best.write(sub_out_dp);

        // Reset lss_vec
        std::cout << "\t\t" << " : " << lp << std::endl;
        ++progress_bar;
    }

    final_best.write(sub_out_dp);
    // plot
    Linear_state_space lss_all = plot_data_and_model(data, 
                                                     //lss_vec[0], 
                                                     final_best, 
                                                     sub_out_dp);
    lss_all.write(string(sub_out_dp + "/all_states"));
    long st = kjb_c::get_real_time();

    // Compute the error 
    errors = compute_ave_error(data, lss_all, obs_errors, 
                               exp.likelihood.training_percent);
    std::string err_fp(sub_out_dp + "/rms_error.txt");
    std::ofstream err_of(err_fp.c_str());
    IFTD(err_of.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
    err_of << data.dyid << " " << errors << std::endl;

    // errors for each observable
    if(lss_all.obs_names().size() > 1)
    {
        for(size_t j = 0; j < obs_errors.size(); j++)
        {
            std::string out_fp(sub_out_dp + "/" + lss_all.obs_names()[j] 
                                       + "_rms_error.txt");
            std::ofstream out_ofs(out_fp.c_str(), std::ostream::app);
            IFTD(out_ofs.is_open(), IO_error, "Can't open file %s", 
                                        (out_fp.c_str()));
            out_ofs << data.dyid << " "; 
            for(size_t k = 0; k < obs_errors[j].size(); k++)
            {
                out_ofs << obs_errors[j][k] << " ";
            }
            out_ofs << std::endl;
        }
    }
    std::cout << "Execution time fitting: " << st / 3600000.0 << " hours\n";

    // Compute the predictive distribution
    kjb_c::init_real_time();
    string sample_dp(sub_out_dp + "/pred_samples");
    ETX(kjb_c::kjb_mkdir(sample_dp.c_str()));
        
    boost::format sample_fmt(sample_dp + "/sample_%04d/");
    std::string pred_fp(sample_dp + "/sampled_rms_error.txt");
    std::ofstream ofs(pred_fp.c_str());
    sampled_errors = compute_ave_error(data, samples[0], obs_sampled_errors, 
                                       exp.likelihood.training_percent);
    // compute the distribution 
    /*if(exp.run.write_samples)
    {
        for(size_t i = 0; i < num_samples; i++)
        {
            string sample_sub_dp = (sample_fmt % (i+1)).str();
            samples[0][i].write(sample_sub_dp);
        }
    }*/
    ofs << data.dyid << " " << sampled_errors << std::endl;

    // sample error for each observation
    if(lss_all.obs_names().size() > 1)
    {
        for(size_t j = 0; j < obs_errors.size(); j++)
        {
            std::string out_fp(sample_dp + "/" + exp.likelihood.obs_names[j] 
                                       + "_sampled_rms_error.txt");
            std::ofstream out_ofs(out_fp.c_str(), std::ostream::app);
            IFTD(out_ofs.is_open(), IO_error, "Can't open file %s", 
                                        (out_fp.c_str()));
            out_ofs << data.dyid << " "; 
            for(size_t k = 0; k < obs_errors[j].size(); k++)
            {
                out_ofs << obs_errors[j][k] << " ";
            }
            out_ofs << std::endl;
        }
    }

    // compute the outcome err

    st = kjb_c::get_real_time();
    std::cout << "Execution time of predicting samples: " 
              << st / 60000.0 << " minutes" << std::endl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::ties::line_fitting_outcome
(
    const Lss_set& lss_set,
    Line& line,
    size_t outcome_type_index
)
{
    const std::vector<Linear_state_space>& lss_vec = lss_set.lss_vec();
    std::vector<Vector> points(lss_vec.size());

    for(size_t i = 0; i < points.size(); i++)
    {
        int num_oscillator = lss_vec[i].num_oscillators();
        Vector v(num_oscillator, 0.0);
        for(int o = 0; o < lss_vec[i].num_oscillators(); o++)
        {
            v[o] = lss_vec[i].get_outcome(outcome_type_index, o);
        }
        points[i] = v;
    }
    return line_fitting(points, line);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> kjb::ties::line_fitting_outcome
(
    const Lss_set& lss_set,
    std::vector<Line>& lines
)
{
    size_t outcome_type = lss_set.num_outcome_type();
    lines.resize(outcome_type);
    std::vector<double> errors(outcome_type, 0.0);
    for(size_t t = 0; t < outcome_type; t++)
    {
        errors[t] = line_fitting_outcome(lss_set, lines[t], t);
    }
    return errors;
}
