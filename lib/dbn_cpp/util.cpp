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

/* $Id: util.cpp 22559 2019-06-09 00:02:37Z kobus $ */
#include <l_cpp/l_exception.h>
#include <l/l_sys_debug.h>
#include <l/l_sys_def.h>
#include <p/p_plot.h>
#include <n_cpp/n_svd.h>
#include <m/m_convolve.h>

#include <vector>
#include <utility>
#include <fstream>
#include <ostream>
#include <string>
#include <algorithm>
#include <iterator>

#include <ergo/mh.h>
#include <ergo/record.h>

#include "dbn_cpp/util.h"
#include "dbn_cpp/drift_sampler.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace kjb;
using namespace kjb::ties;

std::vector<std::pair<size_t, size_t> > kjb::ties::parse_topics(const std::string& topics_fp)
{
    using namespace std;
    vector<pair<size_t, size_t> > times;
    ifstream ifs(topics_fp.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "Can't open file %s", (topics_fp.c_str()));
    }
    string line;
    while(getline(ifs, line))
    {
        vector<string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(", \n"), boost::token_compress_on);
        assert(tokens.size() == 2);
        size_t start = boost::lexical_cast<size_t>(tokens[0]);
        size_t end = boost::lexical_cast<size_t>(tokens[1]);
        times.push_back(make_pair(start, end));
    }
    return times;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::plot_data
(
    const Data& data,
    const std::string& out_dir
)
{ 
#if !defined(KJB_ELGATO)
    Vector X(data.times.begin(), data.times.end());

    size_t length = data.times.size();
    const Obs_map& observables = data.observables; 

    // COLLECT THE DATA
    size_t coef_index = 0;
    BOOST_FOREACH(const Obs_map::value_type& obs, observables)
    {
        std::string obs_name = obs.first;
        double min_val = DBL_MAX;
        double max_val = -DBL_MAX;

        Obs_map::const_iterator data_it = observables.find(obs_name);
        IFTD(data_it != observables.end(), Illegal_argument, 
                "%s is not a valid observable", (obs_name.c_str()));

        // data 
        const Vector_v& data_obs = data_it->second;
        size_t num_oscs = data_obs.size();
        std::vector<Vector> Y_data(num_oscs, Vector((int)length, 0.0));
        for(size_t i = 0; i < num_oscs; i++)
        {
            for(size_t j = 0; j < length; j++)
            {
                if(!invalid_data(data_obs[i][j]))
                {
                    Y_data[i][j] = data_obs[i][j];
                    if(Y_data[i][j] < min_val)
                    {
                        min_val = Y_data[i][j];
                    }
                    if(Y_data[i][j] > max_val)
                    {
                        max_val = Y_data[i][j];
                    }
                }
            }
        }

        // set up the plot
        try
        {
            std::string data_str = "data-" + obs_name;
            int id = kjb_c::plot_open();
            KJB(ETX(set_colour_plot()));
            KJB(ETX(plot_set_range(id, X[0], X[length - 1], min_val, max_val)));

            // plot dial
            for(size_t j = 0; j < num_oscs; j++)
            {
                std::string data_type_name = data_str +  
                                    ("-" + boost::lexical_cast<std::string>(j));
                KJB(ETX(plot_curve(id, X.get_c_vector(), 
                                            Y_data[j].get_c_vector(), 
                                            data_type_name.c_str())));
            }

            std::string type_string = obs_name;
            boost::format fname_fmt(out_dir + "/%04d_" + type_string + ".ps");
            std::string fname = (fname_fmt % data.dyid).str();
            KJB(ETX(save_plot(id, fname.c_str())));
            KJB(ETX(plot_close(id)));
        }
        catch(Exception& e)
        {
            std::cerr << "KJB PLOT error: " << e.get_msg() << std::endl;
        }
    }
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::plot_model
(
    const Linear_state_space& lss, 
    const std::string& out_dir
)
{
#if !defined(KJB_ELGATO)
    using namespace std;
    const State_vec_vec& all_states = lss.get_states();
    const std::vector<std::string>& obs_names = lss.obs_names();
    Vector X(lss.get_times().begin(), lss.get_times().end());
    size_t length = lss.get_times().size();
    KJB(ASSERT(length == all_states.size()));

    ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
    size_t num_oscs = lss.num_oscillators();
    size_t coef_index = 0;
    for(size_t i = 0; i < obs_names.size(); i++)
    {
        std::string type_name = obs_names[i];
        string fname = out_dir + "/" + type_name + ".ps";
        // set up the plot
        int id = kjb_c::plot_open();
        KJB(EPETE(set_colour_plot()));
        double min_val = DBL_MAX;
        double max_val = -DBL_MAX;
        // <time><osc>
        std::vector<Vector> Ys(num_oscs, Vector((int)length, 0.0));
        std::vector<std::string> type_names(num_oscs, type_name);
        for(size_t j = 0; j < num_oscs; j++)
        {
            type_names[j] += ("-" + boost::lexical_cast<string>(j));
            for(size_t t = 0; t < length; t++)
            {
                KJB(ASSERT(all_states[t][i].size() == num_oscs));
                Ys[j][t] = all_states[t][i][j];
            }

            // Find the ranges of the data
            KJB(ASSERT(!Ys[j].empty()));
            double cur_min_val = min(Ys[j]);
            double cur_max_val = max(Ys[j]);
            if(cur_min_val < min_val)
            {
                min_val = cur_min_val;
            }
            if(cur_max_val > max_val)
            {
                max_val = cur_max_val;
            }
        }

        KJB(EPETE(plot_set_range(id, X[0], X[length - 1], min_val, max_val)));
        for(size_t j = 0; j < num_oscs; j++)
        {
            KJB(EPETE(plot_curve(id, X.get_c_vector(), Ys[j].get_c_vector(), 
                      type_names[j].c_str())));
        }
        KJB(EPETE(save_plot(id, fname.c_str())));
        KJB(EPETE(plot_close(id)));
    }
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Linear_state_space kjb::ties::plot_data_and_model
(
    const Data& data,
    const Linear_state_space& lss,
    const std::string& out_dir,
    bool plot_test
)
{
    using namespace std;

    Linear_state_space lss_copy(lss);
    // create data y vector
    int length = data.times.size();
    const State_vec_vec& states = lss_copy.get_states();
    size_t train_length = states.size();
    double training_percent = 1.0;
    bool plot_test_line = plot_test;
    if(train_length < length)
    {
        training_percent = 1.0 * train_length / length;
        if(plot_test)
        {
            plot_test_line = true;
            lss_copy.update_times(data.times);
        }
        else
        {
            length = train_length;
        }
    }
    Vector X(data.times.begin(), data.times.begin() + length);

    const State_vec_vec& all_states = lss_copy.get_states();
    const Obs_map& observables = data.observables; 
    size_t num_oscs = lss.num_oscillators();
    size_t num_obs = lss.obs_names().size();

    // COLLECT THE DATA
    size_t coef_index = 0;
    for(size_t o = 0; o < num_obs; o++)
    {
        std::string obs_name = lss.obs_names()[o];
        double min_val = DBL_MAX;
        double max_val = -DBL_MAX;

        Obs_map::const_iterator data_it = observables.find(obs_name);
        IFTD(data_it != observables.end(), Illegal_argument, 
                "%s is not a valid observable", (obs_name.c_str()));

        // data 
        const Vector_v& data_obs = data_it->second;
        KJB(ASSERT(data_obs.size() == num_oscs));

        std::vector<Vector> Y_data(num_oscs, Vector(length, 0.0));
        for(size_t i = 0; i < num_oscs; i++)
        {
            for(size_t j = 0; j < length; j++)
            {
                if(!invalid_data(data_obs[i][j]))
                {
                    Y_data[i][j] = data_obs[i][j];
                    if(Y_data[i][j] < min_val)
                    {
                        min_val = Y_data[i][j];
                    }
                    if(Y_data[i][j] > max_val)
                    {
                        max_val = Y_data[i][j];
                    }
                }
            }
        }

        // model
        std::vector<Vector> Y_model(num_oscs, Vector((int)length, 0.0));
        for(size_t j = 0; j < num_oscs; j++)
        {
            for(size_t t = 0; t < length; t++)
            {
                Y_model[j][t] = all_states[t][o][j];
            }
            // Find the ranges of the data
            KJB(ASSERT(!Y_model[j].empty()));
            double cur_min_val = min(Y_model[j]);
            double cur_max_val = max(Y_model[j]);
            if(cur_min_val < min_val)
            {
                min_val = cur_min_val;
            }
            if(cur_max_val > max_val)
            {
                max_val = cur_max_val;
            }
        }
#if 0
#endif
    }
    return lss_copy;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::plot_exp_dir
(
    const std::string& exp_dir, 
    const std::string& list_fp,
    const std::string& data_dir
)
{
    boost::format data_fmt(data_dir + "/%04d.txt");
    std::vector<size_t> ids = parse_list(list_fp);
    boost::format lss_fmt(exp_dir+ "/%04d/");
    for(size_t i = 0; i < ids.size(); i++)
    {
        std::string fname = (data_fmt % ids[i]).str();
        Data data = parse_data(fname);
        std::string lss_dir = (lss_fmt % ids[i]).str();
        Linear_state_space lss;
        lss.read(lss_dir, data.times.front());
        Linear_state_space lss_all = plot_data_and_model(data, 
                                                         lss, 
                                                         lss_dir);
        lss_all.write(std::string(lss_dir + "/all_states"));
    }
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Linear_state_space> kjb::ties::plot_data_and_model
(
    const std::vector<Data>& data,
    const Lss_set& lss_set,
    const std::string& out_dir
)
{
    const std::vector<Linear_state_space>& lss_vec = lss_set.lss_vec();
    boost::format figure_fmt(out_dir + "/%04d/");
    size_t N = data.size();
    std::vector<Linear_state_space> lss_all_states(N);
    for(size_t i = 0; i < N; i++)
    {
        std::string figure_fp = (figure_fmt % data[i].dyid).str();
        KJB(EPETE(kjb_mkdir(figure_fp.c_str())));
        lss_all_states[i] = plot_data_and_model(data[i], lss_vec[i], figure_fp);
    }
    return lss_all_states;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::standardize
(
    Data& data, 
    const std::string& type,
    bool convert_mean,
    bool convert_var
)
{
    std::pair<Vector, Vector> stats = get_mean_variances(data, type);
    Obs_map::iterator it = data.observables.find(type);
    if(it == data.observables.end())
    {
        KJB_THROW_3(Illegal_argument, 
                "observable %s is invalid", (type.c_str()));
    }

    Vector means = stats.first;
    Vector variances = stats.second;
    KJB(ASSERT(means.size() == variances.size()));
    Vector_v& obs = it->second; 
    for(size_t o = 0; o < variances.size(); o++)
    {
        Vector& vals = obs[o];
        double sigma = std::pow(variances[o], 0.5);
        double mean = means[o];
        BOOST_FOREACH(double& val, vals)
        {
            if(!invalid_data(val))
            {
                if(convert_mean && convert_var)
                {
                    val = (val - mean)/sigma;
                }
                else if(convert_mean)
                {
                    val = val - mean;
                }
            }
        }
    }
    return stats;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::standardize
(
    std::vector<Data>& data_all, 
    const std::string& type,
    bool convert_mean,
    bool convert_var
)
{
    if(data_all.empty()) return std::pair<Vector, Vector>();
    // number of oscillators
    int num_oscs = data_all[0].observables.at(type).size();
    Vector means(num_oscs, 0.0);
    Vector variances(num_oscs, 0.0);
    std::vector<size_t> valid_data(num_oscs, 0);
    // means
    BOOST_FOREACH(const Data& data, data_all)
    {
        Obs_map::const_iterator it = data.observables.find(type);
        if(it == data.observables.end())
        {
            KJB_THROW_3(Illegal_argument, "observable %s is invalid", (type.c_str()));
        }

        const Vector_v& obs = it->second; 
        for(size_t i = 0; i < obs.size(); i++)
        {
            const Vector& vals = obs[i];
            assert(vals.size() == data.times.size());
            BOOST_FOREACH(double val, vals)
            {
                if(!invalid_data(val))
                {
                    means[i] += val;
                    valid_data[i]++;
                }
            }
        }
    }
    for(size_t i = 0; i < num_oscs; i++)
    {
        means[i] /= valid_data[i];
    }

    // variances
    BOOST_FOREACH(const Data& data, data_all)
    {
        Obs_map::const_iterator it = data.observables.find(type);
        if(it == data.observables.end())
        {
            KJB_THROW_3(Illegal_argument, "observable %s is invalid", (type.c_str()));
        }

        const Vector_v& obs = it->second; 
        assert(obs.size() == num_oscs);
        for(size_t i = 0; i < obs.size(); i++)
        {
            const Vector& vals = obs[i];

            assert(vals.size() == data.times.size());
            BOOST_FOREACH(double val, vals)
            {
                if(!invalid_data(val))
                {
                    variances[i] += std::pow(val - means[i], 2);
                }
            }
        }
    }

    for(size_t i = 0; i < num_oscs; i++)
    {
        variances[i] /= (valid_data[i] - 1);
    }
    // convert
    BOOST_FOREACH(Data& data, data_all)
    {
        Obs_map::iterator it = data.observables.find(type);
        if(it == data.observables.end())
        {
            KJB_THROW_3(Illegal_argument, "observable %s is invalid", (type.c_str()));
        }

        Vector_v& obs = it->second; 
        assert(obs.size() == num_oscs);
        for(size_t i = 0; i < obs.size(); i++)
        {
            Vector& vals = obs[i];
            assert(obs.size() == num_oscs);
            BOOST_FOREACH(double& val, vals)
            {
                if(!invalid_data(val))
                {
                    if(convert_mean && convert_var)
                    {
                        val = (val - means[i])/std::pow(variances[i], 0.5);
                    }
                    else if(convert_mean)
                    {
                        val = val - means[i];
                    }
                }
            }
        }
    }

    return std::make_pair(means, variances);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::compute_mean_sem
(
    const std::vector<Vector>& errors
)
{
    Vector mean_all;
    Vector sem_all;
    IFT(!errors.empty(), Illegal_argument, "size of errors is zero.");
    size_t N = errors.size();
    int D = errors.front().size();
    if(D == 0) return std::make_pair(mean_all, sem_all);
    Vector mean = std::accumulate(errors.begin(), errors.end(), Vector(D, 0.0));
    mean /= N;
    Vector sem = Vector(D, 0.0);
    for(size_t i = 0; i < errors.size(); i++)
    {
        Vector t = errors[i] - mean;
        for(size_t j = 0; j < D; j++)
        {
            sem[j] += t[j] * t[j]; 
        }
    }
    sem /= N;
    for(size_t i = 0; i < sem.size(); i++)
    {
        sem[i] = std::sqrt(sem[i])/std::sqrt(N);
    }
    // compute the mean 
    size_t d = D/2;
    double mean_fit = std::accumulate(mean.begin(), mean.begin() + d, 0.0)/d;
    double sem_fit = std::accumulate(sem.begin(), sem.begin() + d, 0.0)/d;
    double mean_pred = std::accumulate(mean.begin() + d, mean.end(), 0.0)/d;
    double sem_pred = std::accumulate(sem.begin() + d, sem.end(), 0.0)/d;
    std::copy(mean.begin(), mean.begin() + d, std::back_inserter(mean_all));
    mean_all.push_back(mean_fit);
    std::copy(mean.begin() + d, mean.end(), std::back_inserter(mean_all));
    mean_all.push_back(mean_pred);

    std::copy(sem.begin(), sem.begin() + d, std::back_inserter(sem_all));
    sem_all.push_back(sem_fit);
    std::copy(sem.begin() + d, sem.end(), std::back_inserter(sem_all));
    sem_all.push_back(sem_pred);
    // return the mean and standard error of the mean
    return std::make_pair(mean_all, sem_all);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::compute_error
(
    const Linear_state_space& real_lss, 
    const Linear_state_space& lss,
    double train_percent
)
{
    const Double_v& train_times_real = real_lss.get_times(); 
    const Double_v& train_times = lss.get_times(); 
    assert(train_times_real.size() == train_times.size());
    size_t train_length = train_times.size();
    if(train_percent < 1.0) train_length *= train_percent;
    const State_vec_vec& states_1 = real_lss.get_states();
    const State_vec_vec& states_2 = lss.get_states();

    size_t num_oscs = real_lss.num_oscillators();
    int num_errors = 2 * num_oscs;
    size_t num_obs = lss.obs_names().size();
    std::vector<Vector> errors(num_obs);

    for(size_t k = 0; k < num_obs; k++)
    {
        Vector cur_errors(num_errors, 0.0);
        std::vector<size_t> num_data(num_errors, 0);
        for(size_t i = 0; i < train_times_real.size(); i++)
        {
            for(size_t j = 0; j < num_oscs; j++)
            {
                double diff = states_1[i][k][j] - states_2[i][k][j];
                if(i < train_length)
                {
                    // trianing error
                    cur_errors[j] += diff ;
                    num_data[j]++;
                }
                else
                {
                    // testing error
                    cur_errors[j + num_oscs] += diff * diff;
                    num_data[j + num_oscs]++;
                }
            }
        }

        for(size_t i = 0; i < num_errors; i++)
        {
            if(num_data[i] == 0) continue;
            cur_errors[i] = pow(cur_errors[i] / num_data[i], 0.5);
        }
        errors[k] = cur_errors;
    }

    return errors;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::compute_error
(
    const Data& data, 
    const Linear_state_space& lss,
    double train_percent
)
{
    Linear_state_space lss_copy(lss);
    const Double_v& train_times = lss_copy.get_times(); 
    int length = data.times.size();
    int train_length = train_times.size();
    assert(train_length <= length);
    if(train_length == length)
    {
        if(train_percent < 1.0)
            train_length *= train_percent;
    }
    else //if(train_length < length)
    {
        lss_copy.update_times(data.times);
    }

    const State_vec_vec& all_preds = lss_copy.get_states(); 
    size_t num_oscs = lss_copy.num_oscillators();

    const Obs_map& observables = data.observables;
    size_t num_obs = lss_copy.obs_names().size();

    int num_errors = 2 * num_oscs;
    Vector errors(num_errors, 0.0);

    // indexed by <obs> <oscillator>
    std::vector<Vector> all_errors(num_obs, errors);
    std::vector<std::vector<size_t> > num_data(num_obs, 
                    std::vector<size_t>(num_errors, 0));

    for(size_t i = 0; i < length; i++)
    {
        //BOOST_FOREACH(const Obs_map::value_type& value_type, observables)
        for(size_t k = 0; k < lss_copy.obs_names().size(); k++)
        {
            const std::string& obs_name = lss_copy.obs_names()[k];
            //const std::string& obs_name = value_type.first;
            Obs_map::const_iterator f_it = observables.find(obs_name);
            IFTD(f_it != observables.end(), Illegal_argument, 
                    "%s is not a valid observable", (obs_name.c_str()));
            const Vector_v& obs_vals = f_it -> second; 

            KJB(ASSERT(num_oscs = obs_vals.size()));
            for(size_t j = 0; j < num_oscs; j++)
            {
                double data_val = obs_vals[j][i];
                double model_val = all_preds[i][k][j];
                if(!invalid_data(data_val))
                {
                    double diff = model_val - data_val;
                    double diff_squared = diff * diff; 
                    // training 0%-80%
                    if(i < train_length)
                    {
                        all_errors[k][j] += diff_squared;
                        num_data[k][j]++;
                    }
                    // testing 80%-100%
                    else
                    {
                        all_errors[k][j + num_oscs] += diff_squared;
                        num_data[k][j + num_oscs]++;
                    }
                }
            }
        }
    }

    for(size_t i = 0; i < all_errors.size(); i++)
    {
        for(size_t j = 0; j < all_errors[i].size(); j++)
        {
            if(num_data[i][j] == 0) continue;
            all_errors[i][j] = pow(all_errors[i][j] / num_data[i][j], 0.5);
        }
    }

    return all_errors;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::compute_error
(
    const std::vector<Data>& data, 
    const Lss_set& trained_lss_set,
    double train_percent
)
{
    const std::vector<Linear_state_space>& lss_vec = trained_lss_set.lss_vec();
    IFT(lss_vec.size() == data.size(), Dimension_mismatch, 
            "lss_set and data set has different dimesion.");
    if(data.empty()) return std::vector<Vector>();
    std::vector<Vector> all_errors = compute_error(data[0], lss_vec[0], train_percent);
    for(size_t d = 1; d < lss_vec.size(); d++)
    {
        std::vector<Vector> errors_per_couple = 
                                compute_error(data[d], lss_vec[d], train_percent);
        KJB(ASSERT(errors_per_couple.size() == all_errors.size()));
        for(size_t i = 0; i < all_errors.size(); i++)
        {
            all_errors[i] += errors_per_couple[i];
        }
    }

    for(size_t i = 0; i < all_errors.size(); i++)
    {
        all_errors[i] /= lss_vec.size();
    }

    return all_errors;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::ties::compute_ave_error
(
    const Data& data, 
    const std::vector<Linear_state_space>& samples,
    std::vector<Vector>& obs_errors,
    double train_percent
)
{
    size_t num_samples = samples.size();
    if(num_samples == 0) return Vector();

    // first sample
    Vector ave_errors = compute_ave_error(data,
                                          samples[0], 
                                          obs_errors,
                                          train_percent);
    size_t num_obs = 0;
    for(size_t i = 1; i < samples.size(); i++)
    {
        std::vector<Vector> obs_errors_per;
        ave_errors += compute_ave_error(data,
                                        samples[i], 
                                        obs_errors_per,
                                        train_percent);
        num_obs = obs_errors_per.size();
        for(size_t j = 0; j < obs_errors_per.size(); j++)
        {
            obs_errors[j] += obs_errors_per[j];
        }
    }

    for(size_t j = 0; j < num_obs; j++)
    {
        obs_errors[j] /= num_samples;
    }
    
    return ave_errors / num_samples;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::report_errors
(
    const std::string& err_dp,
    const std::string& err_fp_str,
    const std::vector<Vector>& errors,
    const std::vector<std::vector<Vector> >& obs_errors,
    const std::vector<std::string>& obs_names,
    const std::vector<size_t>& dyids
)
{
    using namespace std;

    IFT(obs_names.size() == obs_errors.size(), Runtime_error, 
            "The number of average errors is different than the number "
            "of each observable errors");
    bool write_id = false;
    size_t N = errors.size();
    if(!dyids.empty())
    {
        IFT(N == dyids.size(), Runtime_error, 
                "The number of average errors is different than the number "
                "of lss");
        write_id = true;
    }

    // create the ofstream for the average error
    string err_fp(err_dp + "/" + err_fp_str);
    ofstream err_ofs(err_fp.c_str());
    IFTD(!err_ofs.fail(), IO_error, "Can't open file %s", (err_fp.c_str())); 

    // create the ofstream for the observable error
    vector<boost::shared_ptr<ofstream> > obs_err_ofs(obs_names.size());
    for(size_t i = 0; i < obs_names.size(); i++)
    {
        string obs_fp(err_dp + "/" + obs_names[i]+ "_" + err_fp_str);
        obs_err_ofs[i] = boost::make_shared<ofstream>(obs_fp.c_str());
        IFTD(!obs_err_ofs[i]->fail(), IO_error, "Can't open file %s", (obs_fp.c_str())); 
    }

    for(size_t i = 0; i < N; i++) 
    {
        if(write_id)
        {   
            err_ofs << dyids[i] << " ";
        }
        err_ofs << errors[i] << "\n";

        // create a vector of ofstreams
        for(size_t j = 0; j < obs_names.size(); j++)
        {
            if(write_id)
            {
                *obs_err_ofs[j] << dyids[i] << " " << obs_errors[j][i] << "\n"; 
            }
            else
            {
                *obs_err_ofs[j] << obs_errors[j][i] << "\n";
            }
        } 
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::report_cross_validate_errors
(
    const std::string& err_dp,
    const std::string& obs_name,
    const std::string& distinguisher,
    const std::vector<std::string>& fold_dirs,
    size_t K,
    bool write_latex_error
)
{
    using namespace std;
    std::string model_obs_str = obs_name + "_";
    if(obs_name == "")
    {
        model_obs_str = "";
    }
    std::string test_err_couple_fp = 
        string(err_dp + "/" + model_obs_str + "err_couples.txt");
    std::ofstream cofs(test_err_couple_fp.c_str());
    IFTD(cofs.is_open(), IO_error, "can't open file %s", 
                                (test_err_couple_fp.c_str()));

    std::string sampled_err_couple_fp = 
        string(err_dp + "/" + model_obs_str + "err_couples_sampled.txt");
    std::ofstream scofs(sampled_err_couple_fp.c_str());
    IFTD(scofs.is_open(), IO_error, "can't open file %s", 
                                (sampled_err_couple_fp.c_str()));

    vector<Vector> testing_errors(K);
    vector<Vector> training_errors(K);
    vector<Vector> sampled_errors(K);
    vector<Vector> testing_errors_per_couple;
    bool sampled_err = false;
    for(size_t fold = 1; fold <= K; fold++)
    {
        string out_dp = fold_dirs[fold-1];
        // training errors 
        string tfp(out_dp + "/training/training_errors.txt");
        if(obs_name != "")
        {
            tfp = out_dp + "/training/" + obs_name + "_training_errors.txt";
        }
        std::ifstream tifs(tfp.c_str());
        IFTD(tifs.is_open(), IO_error, "can't open file %s", (tfp.c_str()));
        std::string line;
        getline(tifs, line);
        std::vector<double> elems;
        istringstream ist(line);
        std::copy(istream_iterator<double>(ist), istream_iterator<double>(), 
                back_inserter(elems));
        training_errors[fold-1] = Vector(elems);
       
        // testing errors 
        // get the testing error of each observable
        string cfp(out_dp + "/testing/err_couples.txt");
        if(obs_name != "")
        {
            cfp = out_dp + "/testing/" + obs_name + "_err_couples.txt";
        }
        std::ifstream cifs(cfp.c_str());
        IFTD(cifs.is_open(), IO_error, "can't open file %s", (cfp.c_str()));
        getline(cifs, line);
        cofs << line << "\n";
        std::vector<double> first_elems;
        istringstream first_ist(line);
        std::copy(istream_iterator<double>(first_ist), istream_iterator<double>(), 
                back_inserter(first_elems));
        Vector testing_error(first_elems.begin() + 1, first_elems.end());
        testing_errors_per_couple.push_back(testing_error);
        size_t num_couples = 1;
        while(getline(cifs, line))
        {
            cofs << line << "\n";
            std::vector<double> elems;
            istringstream ist(line);
            std::copy(istream_iterator<double>(ist), istream_iterator<double>(), 
                    back_inserter(elems));
            std::vector<double>::iterator eit= elems.begin();
            std::vector<double> error(eit+1, elems.end());
            Vector temp(error);
            testing_error += temp;
            num_couples++;
            testing_errors_per_couple.push_back(temp);
        }
        testing_errors[fold-1] = testing_error/num_couples;

        // sampled errors
        string scfp(out_dp + "/testing/err_couples_sampled.txt");
        if(obs_name != "")
        {
            scfp = out_dp + "/testing/" + obs_name + "_err_couples_sampled.txt";
        }
        if(kjb_c::is_file(scfp.c_str()))
        {
            std::ifstream scifs(scfp.c_str());
            IFTD(scifs.is_open(), IO_error, "can't open file %s", (scfp.c_str()));
            getline(scifs, line);
            scofs << line << "\n";
            first_elems.clear();
            istringstream tist(line);
            std::copy(istream_iterator<double>(tist), istream_iterator<double>(), 
                    back_inserter(first_elems));
            if(!first_elems.empty())
            {
                Vector stesting_error(first_elems.begin() + 1, first_elems.end());
                while(getline(scifs, line))
                {
                    scofs << line << "\n";
                    std::vector<double> elems;
                    istringstream ist(line);
                    std::copy(istream_iterator<double>(ist), istream_iterator<double>(), 
                            back_inserter(elems));
                    std::vector<double>::iterator eit= elems.begin();
                    std::vector<double> error(eit+1, elems.end());
                    stesting_error += Vector(error);
                }
                sampled_errors[fold-1] = stesting_error/num_couples;
                sampled_err = true;
            }
            else
            {
                std::cerr << "WARNING: Failed to compute sampled predictive error.\n";
            }
        }
    }

    std::string err_fp = err_dp + "/" + model_obs_str + "training_errors.txt";
    compute_and_record_errors(training_errors, distinguisher, err_fp);

    err_fp = err_dp + "/" + model_obs_str + "err_fold_means.txt";
    compute_and_record_errors(testing_errors_per_couple, distinguisher, err_fp);

    if(write_latex_error)
    {
        err_fp = err_dp + "/" + model_obs_str + "err_fold_means_latex.txt";
        compute_and_record_errors_latex(testing_errors_per_couple, err_fp);
    }

    if(sampled_err)
    {
        err_fp = err_dp + "/" + model_obs_str + "err_fold_means_sampled.txt"; 
        compute_and_record_errors(sampled_errors, distinguisher, err_fp);
    }

    // write outcome errors 
    std::vector<std::string> err_fps(fold_dirs.size());
    for(size_t k = 0; k < fold_dirs.size(); k++)
    {
        std::string fp = fold_dirs[k] + "/testing/outcome_errs.txt";
        err_fps[k] = fp;
    }
    err_fp = err_dp + "/outcome_pred_errors.txt";
    report_outcome_errors(err_fps, err_fp, distinguisher);

    // write ave outcome errors 
    err_fps.clear();
    err_fps.resize(fold_dirs.size());
    for(size_t k = 0; k < fold_dirs.size(); k++)
    {
        std::string fp = fold_dirs[k] + "/testing/outcome_errs_ave.txt";
        err_fps[k] = fp;
    }
    err_fp = err_dp + "/outcome_pred_errors_ave.txt";
    report_outcome_errors(err_fps, err_fp, distinguisher);

    // write ave outcome errors 
    err_fps.clear();
    err_fps.resize(fold_dirs.size());
    for(size_t k = 0; k < fold_dirs.size(); k++)
    {
        std::string fp = fold_dirs[k] + "/testing/outcome_errs_prior.txt";
        err_fps[k] = fp;
    }
    err_fp = err_dp + "/outcome_pred_errors_prior.txt";
    report_outcome_errors(err_fps, err_fp, distinguisher);

    // read in the line predictors 
    double pred_err = 0.0;
    size_t n = 0;
    bool outcome = true;
    for(size_t k = 0; k < fold_dirs.size(); k++)
    {
        std::string fp = fold_dirs[k] + "/testing/outcome_line_pred.txt";
        if(!kjb_c::is_file(fp.c_str())) 
        {
            outcome = false;
            assert(k == 0);
            break;
        }
        std::ifstream ifs(fp.c_str());
        IFTD(ifs.is_open(), IO_error, "can't open file %s", (fp.c_str()));
        std::string line;
        while(std::getline(ifs, line))
        {
            vector<double> elems;
            istringstream ist(line);
            copy(istream_iterator<double>(ist), 
                 istream_iterator<double>(), 
                 back_inserter(elems));
            pred_err += elems[1];
            n++;
        }
    }
    if(outcome)
    {
        pred_err = std::pow((pred_err/n), 0.5);
        std::string fp = err_dp + "/outcome_line_pred.txt";
        std::ofstream ofs(fp.c_str());
        IFTD(ofs.is_open(), IO_error, "can't open file %s", (fp.c_str()));
        ofs << pred_err << std::endl;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::report_outcome_errors
(
    const std::vector<std::string>& couple_err_fps, 
    const std::string& out_fp,
    const std::string& distinguisher
)
{
    std::vector<Vector> outcome_per_couple;
    for(size_t k = 0; k < couple_err_fps.size(); k++)
    {
        std::string fp = couple_err_fps[k];
        std::vector<Vector> errors = read_multiple_pair_error(fp);
        if(errors.empty()) 
        {
            assert(k == 0);
            return;
        }
        std::copy(errors.begin(), errors.end(), 
                  back_inserter(outcome_per_couple));
    }
    compute_and_record_errors(outcome_per_couple, distinguisher, out_fp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::read_couple_errors
(
     const std::string& lss_dir,
     const std::string& err_fp_str,
     const std::vector<std::string>& obs_names,
     Vector& ave_error,
     std::vector<Vector>& obs_errors
)
{
    try
    {
        std::string err_fp(lss_dir + "/" + err_fp_str);
        ave_error = read_couple_error(err_fp);

        if(ave_error.empty()) return false;

        obs_errors.resize(obs_names.size());
        // parse in the obs errors
        if(obs_names.size() > 1)
        {
            for(size_t j = 0; j < obs_names.size(); j++)
            {
                std::string obs_err_fp(lss_dir + "/" + obs_names[j]
                                       + "_" + err_fp_str);
                obs_errors[j] = read_couple_error(obs_err_fp);
                if(obs_errors[j].empty()) return false;
            }
        }
        return true;
    }
    catch(IO_error& e)
    {
        std::cout << "IO exception: " << e.get_msg() << std::endl;
        return false;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::read_multiple_pair_error
(
    const std::string& in_fp
)
{
    using namespace std;
    std::vector<Vector> res;
    if(!kjb_c::is_file(in_fp.c_str())) return res;
    ifstream cifs(in_fp.c_str());
    IFTD(cifs.is_open(), IO_error, "can't open file %s", (in_fp.c_str()));
    string line;
    while(getline(cifs, line))
    {
        vector<string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(", \n"), 
                     boost::token_compress_on);
        vector<double> error;
        // skip ID 
        for(size_t i = 1; i < tokens.size(); i++)
        {
            if(tokens[i] != "")
            {
                error.push_back(boost::lexical_cast<double>(tokens[i]));
            }
        }
        Vector temp(error);
        res.push_back(temp);
    }
    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

State_type kjb::ties::estimate_init_states
(
    const Data& data, 
    double training_percent,
    const std::string& obs_name,
    const State_type& mean_state
)
{
    size_t init_time_index = 0;
    size_t length = std::ceil(data.times.size() * training_percent);
    size_t end_index = length - 1;

    Obs_map::const_iterator it = data.observables.find(obs_name);
    if(it == data.observables.end())
    {
        KJB_THROW_3(Illegal_argument, "observable %s is invalid", 
                                       (obs_name.c_str()));
    }
  
    const Vector_v& obs = it->second; 
    KJB(ASSERT(!obs.empty()));
    size_t num_oscs = obs.size();
    assert(num_oscs >= 2);

    // init states
    State_type init_state((int)num_oscs * 2, 0.0);
    // Find a valid intial state
    while(init_time_index <= end_index)
    {
        bool valid = true;
        for(size_t i = 0; i < num_oscs; i++) 
        {
            if(invalid_data(obs[i][init_time_index])
               || invalid_data(obs[i][init_time_index+1]))
            {
                valid = false;
            }
        }
        if(!valid) init_time_index++;
        else break;
    }

    // estimate the initial values by dial values
    for(size_t i = 0; i < num_oscs; i++)
    {
        init_state[i] = obs[i][init_time_index];
        if(!mean_state.empty())
        {
            init_state[i] -= mean_state[i];
        }
        init_state[num_oscs + i] = (obs[i][init_time_index + 1] - 
                            obs[i][init_time_index]);
    }
      
    return init_state;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::is_shared_moderator
(
    const Mod_map& moderators,
    const std::string& mod_name
)
{
    bool shared = true;
    Mod_map::const_iterator mod_iter = moderators.find(mod_name);
    if(mod_iter != moderators.end())
    {
        std::string name = mod_iter->first;
        const Double_v& mod_vals = moderators.at(name); 
        assert(mod_vals.size() >= 2);
        for(size_t j = 0; j < mod_vals.size() - 1; j++)
        {
            double temp = fabs(mod_vals[j] - mod_vals[j+1]);
            if(fabs(mod_vals[j] - mod_vals[j+1]) > FLT_EPSILON)
            {
                shared = false;
                return shared;
            }
        }
    }
    return shared;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

State_vec kjb::ties::estimate_init_states
(
    const std::vector<Data>& data, 
    double training_percent,
    const std::string& obs_name,
    int polynomial_degree
)
{
    size_t N = data.size();
    State_vec states(N);
    for(size_t i = 0; i < N; i++)
    {
        Obs_map::const_iterator it = data[i].observables.begin();
        KJB(ASSERT(it != data[i].observables.end()));
        State_type mean_state;
        if(polynomial_degree >= 0)
        {
            std::pair<Vector, Vector> mean_vars = 
                            get_mean_variances(data[i], it->first);
            mean_state = mean_vars.first;
        }

        states[i] = estimate_init_states(data[i], 
                                        training_percent, 
                                        obs_name, 
                                        mean_state);
    }
    return states;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<std::pair<double, double> > >
kjb::ties::get_percentile_moderator
(
    const std::vector<Data>& data,
    const std::vector<std::string>& moderator_strs,
    size_t num_oscillators,
    bool shared_moderator
)
{
    size_t C;
    if(shared_moderator)
    {
        C = std::pow(2.0, (int)moderator_strs.size());
    }
    else
    {
        C = std::pow(2.0, (int)moderator_strs.size() * (int)num_oscillators); 
    }

    std::vector<std::vector<std::pair<double, double> > > mod_percentile_values(C);
    if(data.empty() || moderator_strs.empty()) 
    {
        return mod_percentile_values;
    }

    // get the indices of certain percentiles 
    size_t num_data = data.size();
    size_t num_mod_strs = moderator_strs.size();
    size_t num_mods = shared_moderator ? num_mod_strs : 
                        num_mod_strs * num_oscillators; 
    std::vector<std::vector<double> > mods(num_mods);
    for(size_t i = 0; i < num_data; i++)
    {
        const Mod_map& moderators = data[i].moderators;
        for(size_t j = 0; j < num_mods; j++)
        {
            const Double_v& mod_vals = moderators.at(moderator_strs[j % num_mod_strs]);
            assert(j < num_mods);
            if(shared_moderator)
            {
                mods[j].push_back(mod_vals[0]);
                //break;
            }
            else
            {
                IFT(mod_vals.size() == num_oscillators, Runtime_error, 
                        "Number of oscillators diffs from number of"
                        "moderator values.");
                mods[j].push_back(mod_vals[j]);
            }
        }
    }

    // sort
    for(size_t i = 0; i < mods.size(); i++)
    {
        std::sort(mods[i].begin(), mods[i].end());
    }
    
    for(size_t j = 0; j < C; j++)
    {
        for(size_t i = 0; i < mods.size(); i++)
        {
            if(j >> i & 1)
            {
                size_t index = num_data * 0.75;
                double value = mods[i][index];
                mod_percentile_values[j].push_back(std::make_pair(0.75, value));
            }
            else
            {
                size_t index = num_data * 0.25;
                double value = mods[i][index];
                mod_percentile_values[j].push_back(std::make_pair(0.25, value));
            }
        }
    }

    return mod_percentile_values;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::get_init_state_least_square_fitting_coef
(
    const std::vector<Data>& data,
    double training_percent,
    const std::vector<std::string>& moderator_strs,
    const std::string& obs_name,
    bool shared_moderator,
    size_t num_oscillators
)
{
    int num_data = data.size();
    size_t D = num_oscillators * 2;
    // [oscillator][dyad]
    std::vector<Vector> Ys(D, Vector(num_data, 0.0)); 
    // [oscillator][dyad]
    size_t num_mod_strs = moderator_strs.size();
    int num_mods = shared_moderator ? num_mod_strs : 
                        num_mod_strs * num_oscillators; 
    std::vector<Matrix> Xs(D, Matrix(num_data, 1 + num_mods));

    for(size_t i = 0; i < num_data; i++)
    {
        // constructing the Y vector
        std::pair<Vector, Vector> mean_vars = get_mean_variances(
                        data[i], obs_name);
        State_type init_state = estimate_init_states(data[i], 
                        training_percent, obs_name, mean_vars.first);
        IFT(init_state.size() == D, Dimension_mismatch, 
                "Missmatch dimension in get_init_state_least_square_fitting_coef");
        for(size_t j = 0; j < D; j++)
        {
            Ys[j][i] = init_state[j];
        }

        // construction the X vector
        const Mod_map& mod_map = data[i].moderators;
        for(size_t j = 0; j < D; j++)
        {
            Xs[j](i, 0) = 1.0;
            BOOST_FOREACH(const std::string& mod_name, moderator_strs)
            {
                Mod_map::const_iterator it = mod_map.find(mod_name);
                IFT(it != mod_map.end(), Runtime_error, 
                        "moderator is not provided in the data.");
                const Double_v& mod_vals = it->second;
                for(size_t k = 0; k < num_mods; k++)
                {
                    Xs[j](i, k+1) = mod_vals[k % num_mods];
                }
            }
        }
    }

    std::vector<Vector> coefs(D);
    const double epsilon = 1e-5;
    for(size_t i = 0; i < D; i++)
    {
        // Since Xs[i] is ill-conditioned, we need to use SVD to compute 
        // its psudoinverse
        Svd svd(Xs[i]);
        Vector S = svd.vec_d;
        Matrix U = svd.mat_u;
        Matrix Vt = svd.mat_vt;
        BOOST_FOREACH(double& v, S)
        {
            if(v > epsilon)
            {
                v = 1.0/v;
            }
            else
            {
                v = 0.0;
            }
        }
        Matrix S_inv = create_diagonal_matrix(S);
        coefs[i] = Vt.transpose() * S_inv * U.transpose() * Ys[i];
    } 
    return coefs;
    
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<size_t> kjb::ties::get_sampled_time_indices
(
    const Double_v& all_times, 
    size_t segment_length
) 
{
    //Double_v times;
    std::vector<size_t> times;
    if(segment_length == 0 || segment_length > all_times.size())
    {
        return times;
    }
    size_t N = all_times.size();

    assert(segment_length > 0);
    for(size_t i = 0; i < N; i += segment_length)
    {

        size_t st = i;
        size_t et = i + segment_length - 1 < all_times.size() ?
                    i + segment_length - 1 : all_times.size() - 1;
        Categorical_distribution<size_t> dist(st, et, 1);
        times.push_back(sample(dist));
    }
    return times;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::compute_and_record_errors
(
    const std::vector<Vector>& errors,
    const std::string& distinguisher,
    const std::string& err_fp 
)
{
    using namespace std;
    std::pair<Vector, Vector> stat = compute_mean_sem(errors);
    std::ofstream ofs(err_fp.c_str());
    IFTD(ofs.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
    if(!errors.empty())
    {
        size_t num_oscs = errors.front().size() / 2;
        ofs << setw(10) << " ";
        for(size_t i = 0; i < num_oscs; i++)
        {
            ofs << setw(10) << "fit-" << distinguisher << "-" << i;
        }
        // report the average
        ofs << setw(15) << "fit-average";
        for(size_t i = 0; i < num_oscs; i++)
        {
            ofs << setw(10) << "pred-" << distinguisher << "-" << i;
        }
        ofs << setw(16) << "pred-average";
        ofs << "\n";

        ofs << setw(10) << "MEAN:";
        ofs.setf(std::ios::fixed, std::ios::floatfield);
        for(size_t i = 0; i < stat.first.size(); i++)
        {
            ofs << setw(15) << setprecision(3) << stat.first[i] << " ";
        } 
        ofs << "\n";
        ofs << setw(10) << "SEM:";
        for(size_t i = 0; i < stat.second.size(); i++)
        {
            ofs << setw(15) << setprecision(3) << stat.second[i] << " ";
        } 
        ofs << "\n";
        //    << setw(16) << setprecision(3) << stat.second << "\n";
    }
    return stat; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::compute_and_record_errors_latex
(
    const std::vector<Vector>& errors,
    const std::string& err_fp 
)
{
    using namespace std;
    std::pair<Vector, Vector> stat = compute_mean_sem(errors);
    std::ofstream ofs(err_fp.c_str());
    IFTD(ofs.is_open(), IO_error, "can't open file %s", (err_fp.c_str()));
    if(!errors.empty())
    {
        size_t num_oscs = errors.front().size() / 2;
        ofs << setw(10) << " ";
        for(size_t i = 0; i < num_oscs; i++)
        {
            ofs << setw(16) << "fit-" << i;
        }
        // report the average
        ofs << setw(15) << "fit-average";
        for(size_t i = 0; i < num_oscs; i++)
        {
            ofs << setw(16) << "pred-" << i;
        }
        ofs << setw(16) << "pred-average";
        ofs << "\n";

        ofs << setw(10) << "   ";
        size_t num_osc = stat.first.size() / 2;
        ofs << std::fixed;
        ofs << std::setprecision(2);
        for(size_t i = 0; i < stat.first.size(); i += num_osc)
        {
            const Vector& mean = stat.first;
            const Vector& std = stat.second;
            for(size_t j = 0; j < num_osc; j++)
            {
                ofs << mean[i+j] << " $\\pm$ "
                    << std[i+j] << " & ";
            }
        }
        ofs << "\n";
    }
    return stat; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::ties::construct_diagonal_matrix(const std::vector<Matrix>& Ks)
{
    size_t N = 0;
    BOOST_FOREACH(const Matrix& K, Ks)
    {
        // only work for diaonal matrix now 
        assert(K.get_num_cols() == K.get_num_rows());
        N += K.get_num_cols();
    }

    Matrix K_all(N, N);
    size_t total_length = 0;
    BOOST_FOREACH(const Matrix& K, Ks)
    {
        size_t length = K.get_num_rows();
        // for each couple 
        for(int k = total_length; k < total_length + length; k++)
        {
            for(int kk = total_length; kk < total_length + length; kk++)
            {
                K_all(k, kk) = K(k - total_length, kk - total_length);
            }
        }
        total_length += length;
    }

    return K_all;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Data kjb::ties::smooth_data(const Data data, size_t length, double sigma)
{
    Data smooth_data(data);
    Vector mask;
    ETX(kjb_c::get_1D_gaussian_mask(
                &mask.get_underlying_representation_with_guilt(), length, sigma));
    BOOST_FOREACH(Obs_map::value_type& value_type, smooth_data.observables)
    {
        Obs_map::const_iterator it = data.observables.find(value_type.first);
        assert(it != data.observables.end());
        const Vector_v orig_obs = it->second;
        for(size_t i = 0; i < value_type.second.size(); i++)
        {
            Vector& out = value_type.second[i];
            ETX(kjb_c::convolve_vector(
                        &out.get_underlying_representation_with_guilt(), 
                        orig_obs[i].get_c_vector(), mask.get_c_vector()));

        }
    }
    return smooth_data;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::in_range(const Linear_state_space& lss, double max, double min)
{
    const State_vec_vec& states = lss.get_states();
    size_t time_length = states.size();
   
    double cur_max = -DBL_MAX;
    double cur_min = DBL_MAX;

    size_t counts = 0;
    size_t N = 0;
    for(size_t m = 0; m < time_length; m++)
    {
        for(size_t j = 0; j < states[m].size(); j++)
        {
            for(size_t n = 0; n < states[m][j].size(); n++)
            {
                double model_val = states[m][j][n]; 
                if(std::isnan(model_val))
                {
                    return false;
                }
                if(model_val > cur_max)
                {
                    cur_max = model_val;
                }
                if(model_val < cur_min)
                {
                    cur_min = model_val;
                }
            }
        }

        if(cur_max > max || cur_min < min)
        {
            return false;
        }
    }
    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& kjb::ties::parse_shared_params
(
    std::istream& ist,
    Group_params& group_params
)
{
    using namespace std;
    std::string line;

    // parse in the variances 
    if(!getline(ist, line)) return ist;
    assert(line == "variance");

    // get the values
    getline(ist, line);
    group_params.variances.clear();
    istringstream istr_2(line);
    std::vector<double> vars((istream_iterator<double>(istr_2)),
                             (istream_iterator<double>()));
    for(size_t i = 0; i < vars.size(); i++)
    {
        group_params.variances.push_back(vars[i]);
    }

    size_t num_vars = group_params.variances.size();

    // parse in the pred_coefs
    getline(ist, line);
    assert(line == "coef");
    group_params.pred_coefs.clear();
    while(group_params.pred_coefs.size() < num_vars)
    {
        getline(ist, line);
        istringstream istr(line);
        std::vector<double> elems((istream_iterator<double>(istr)), 
                                  (istream_iterator<double>()));
        group_params.pred_coefs.push_back(Vector(elems.begin(), elems.end()));
    }
    // parse in the group weights
    if(!getline(ist, line)) 
    {
        group_params.group_weight = 1.0;
        return ist;
    }
    else
    {
        assert(line == "weight");
        getline(ist, line);
        istringstream istr(line);
        std::vector<double> weights((istream_iterator<double>(istr)), 
                                   (istream_iterator<double>()));
        assert(weights.size() == 1);
        group_params.group_weight = weights[0];
    }
    return ist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::string> kjb::ties::get_list_fps
(
    const std::string& out_dp, 
    const std::string& grouping_var,
    const std::vector<std::string>& data_groups,
    const std::string& list_fp
)
{
    std::vector<std::string> fps;
    size_t num_groups = data_groups.size();
    if(grouping_var == "") 
    {
        fps.push_back(std::string(out_dp + "/" + list_fp));
        return fps;
    }
    fps.reserve(num_groups);
    for(size_t i = 0; i < num_groups; i++)
    {
        size_t found = data_groups[i].find_last_of("/\\");
        std::string group_name = data_groups[i].substr(found+1);
        std::string fp = out_dp + "/" + group_name + "-" + list_fp;
        fps.push_back(fp);
    }
    return fps;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Vector, Vector> kjb::ties::get_mean_variances
(
    const Data& data,
    const std::string& type
)
{
    size_t num_data = data.times.size();
    Obs_map::const_iterator it = data.observables.find(type);
    if(it == data.observables.end())
    {
        KJB_THROW_3(Illegal_argument, "observable %s is invalid", (type.c_str()));
    }

    const Vector_v& obs = it->second; 
    int num_oscs = obs.size();
    Vector means(num_oscs, 0.0);
    Vector variances(num_oscs, 0.0);
    size_t i = 0;
    BOOST_FOREACH(const Vector& vals, obs)
    {
        double mean = 0.0;
        size_t valid_data = 0;
        KJB(ASSERT(num_data == vals.size()));
        BOOST_FOREACH(double val, vals)
        {
            if(!invalid_data(val))
            {
                mean += val;
                valid_data++;
            }
        }
        mean /= valid_data;

        double variance = 0.0;
        BOOST_FOREACH(double val, vals)
        {
            if(!invalid_data(val))
            {
                variance += std::pow(val - mean, 2);
            }
        }
        variance /= valid_data;

        double sigma = std::pow(variance, 0.5);

        means[i] = mean;
        variances[i] = variance;
        i++;
    }
    return std::make_pair(means, variances);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::randomize_starting_states
(   
    std::vector<Linear_state_space>& lss_set, 
    const std::vector<Data>& data_all,
    double training_percent,
    const std::string& obs_name
)
{
    assert(lss_set.size() == data_all.size());
    for(size_t i = 0; i < data_all.size(); i++)
    {
        lss_set[i].init_state() = 
            estimate_init_states(data_all[i], training_percent, obs_name);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Linear_state_space> kjb::ties::read_lss_samples
(
     const std::string& sample_dir,
     double start_time
)
{
    std::vector<Linear_state_space> all_lss;

    std::string fmt_str(sample_dir + "/%05d/");
    std::vector<std::string> fps = dir_names_from_format(fmt_str);
    for(size_t i = 0; i < fps.size(); i++)
    {
        Linear_state_space lss;
        lss.read(fps[i], start_time);
        all_lss.push_back(lss);
    }
    return all_lss;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::create_characteristic_models
(
    const Ties_experiment& exp, 
    const std::vector<Data>& data,
    const Lss_set& lss_set
)
{
    using namespace std;
    string char_out_dir(exp.out_dp + "/characteristic_models/");
    ETX(kjb_c::kjb_mkdir(char_out_dir.c_str()));

    // get the moderator representative
    vector<vector<pair<double, double> > > mods 
        = get_percentile_moderator(data, exp.lss_set.moderators, 
                exp.lss.num_oscillators, exp.lss_set.shared_moderator);

    for(size_t i = 0; i < mods.size(); i++)
    {
        for(size_t j = 0; j < mods[i].size(); j++)
        {
            std::cout << mods[i][j].first << " " 
                      << mods[i][j].second << "; ";
        }
        std::cout << std::endl;
    }

    // Create idealized the Linear_state_space
    // initial state dimension
    size_t inD = exp.lss.num_oscillators * 2;
    // get the observation state based on the first observable
    std::vector<Vector> lsf_coefs = get_init_state_least_square_fitting_coef(
            data, 
            exp.likelihood.training_percent,
            exp.lss_set.moderators,
            exp.likelihood.obs_names[0],
            exp.lss_set.shared_moderator,
            exp.lss.num_oscillators);

    boost::format sub_fmt(char_out_dir + "/%02d/");
    for(size_t i = 0; i < mods.size(); i++)
    {
        string sub_dir = (sub_fmt % i).str();
        ETX(kjb_c::kjb_mkdir(sub_dir.c_str()));
        string info_fp = sub_dir + "/moderator_info.txt";
        ofstream info_ofs(info_fp.c_str());
        IFTD(info_ofs.is_open(), IO_error, "can't open file %s", 
                (info_fp.c_str()));
        if(exp.lss_set.shared_moderator)
        {
            BOOST_FOREACH(const std::string& mod, exp.lss_set.moderators)
            {
                info_ofs << setw(25) << right << mod;
            }
        }
        else
        {
            BOOST_FOREACH(const std::string& mod, exp.lss_set.moderators)
            {
                for(int j = 0; j < exp.lss.num_oscillators; j++)
                {
                    std::string str(mod + "-" + exp.data.distinguisher + "-" 
                            + boost::lexical_cast<string>(j));
                    info_ofs << setw(25) << right << str << " ";
                }
            }
        }
        info_ofs << std::endl;

        Vector cur_mod((int)mods[i].size() + 1, 1.0);
        for(size_t j = 0; j < mods[i].size(); j++)
        {
            info_ofs << setw(25) << right << mods[i][j].first;
            cur_mod[j + 1] = mods[i][j].second;
        }
        info_ofs << std::endl;

        for(size_t j = 0; j < mods[i].size(); j++)
        {
            info_ofs << setw(25) << right << mods[i][j].second;
        }

        // construct the moderator map
        Mod_map mod_rep;
        size_t k = 0;
        BOOST_FOREACH(const std::string& mod, exp.lss_set.moderators)
        {
            mod_rep[mod] = Double_v((int)exp.lss.num_oscillators, 0.0);
            for(size_t j = 0; j < exp.lss.num_oscillators; j++)
            {
                size_t j_index = exp.lss_set.shared_moderator ? k : j;
                mod_rep[mod][j] = mods[i][j_index].second;
            }
            k++;
        }

        // compute the init state based on the moderators
        State_type init_state(inD);
        if(mods.size() > 1)
        {
            for(size_t j = 0; j < inD; j++)
            {
                init_state[j] = dot(lsf_coefs[j], cur_mod);
            }
        }
        else
        {
            init_state = estimate_average_init_state(
                                             data, 
                                             exp.likelihood.training_percent, 
                                             exp.likelihood.obs_names[0]);
        }

        // initialize the times for the character model 
        Double_v time(exp.character_model_length);
        for(size_t j = 0; j < exp.character_model_length; j++)
        {
            time[j] = j; 
        }

        Coupled_oscillator_v clos(time.size() - 1, 
                Coupled_oscillator(exp.lss.num_oscillators,
                                   exp.lss.init_period,
                                   exp.lss.init_damping,
                                   exp.lss.use_modal,
                                   true));
        Vector obs_sigmas((int)exp.likelihood.obs_names.size(), 
                          exp.likelihood.init_noise_sigma);
        Linear_state_space lss(time, 
                               init_state, 
                               clos,
                               exp.likelihood.obs_names,
                               obs_sigmas,
                               exp.lss.polynomial_degree);
        // initialize the predictors based on the characteer model 
        lss.init_predictors(data.front().moderators, 
                exp.lss_set.get_all_moderators(
                    exp.lss.ignore_clo,
                    exp.lss.num_oscillators,
                    exp.lss.num_params));

        // set the observation coefs 
        const std::vector<vector<Vector> >& obs_coefs = lss_set.obs_coefs();
        for(size_t j = 0; j < obs_coefs.size(); j++)
        {
            for(size_t k = 0; k < obs_coefs[j].size(); k++)
            {
                lss.set_obs_coef(j, k, obs_coefs[j][k]);
            }
        }

        // update the CLO params 
        const std::vector<Vector>& preds = lss.get_predictors();
        std::vector<std::string> group_dirs = get_group_dirs(sub_dir,
                                    lss_set.group_map());
        for(size_t g = 0; g < lss_set.num_groups(); g++)
        {
            // get the predictors
            const std::vector<Vector>& pred_coefs = lss_set.pred_coefs(g);
             
            for(size_t j = 0; j < lss.num_clo_params(); j++)
            {
                double val = dot(preds[j], pred_coefs[j]);
                if(!lss.allow_drift())
                {
                    Coupled_oscillator_v& clos = lss.coupled_oscillators();
                    BOOST_FOREACH(Coupled_oscillator& clo, clos)
                    {
                        clo.set_param(j, val);
                    }
                    lss.changed_index() = 0;
                }
                else 
                {
                    lss.set_gp_mean(j, gp::Constant(val));
                }
            }
            // set the polynomial coefs
            if(lss_set.num_groups() > 1)
            {
                if(group_dirs.size() > 1)
                {
                    lss.write(group_dirs[g-1]);
                }
                else
                {
                    std::string out_dir = sub_dir + "/Cluster-" +
                                          boost::lexical_cast<std::string>(g);
                    lss.write(out_dir);
                }
            }
            else
            {
                lss.write(sub_dir);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> kjb::ties::get_responsibilities
(
    const std::vector<Group_params>& group_params,
    const Linear_state_space& lss,
    bool exclude_outcome
)
{
    size_t num_clusters = group_params.size();
    std::vector<double> resps(num_clusters, 0.0);
    const std::vector<Vector>& preds = lss.get_predictors();
    size_t num_outcomes = lss.num_outcomes();
    for(size_t k = 0; k < num_clusters; k++)
    {
        double log_p = std::log(group_params[k].group_weight);
        const std::vector<Vector>& pred_coefs = group_params[k].pred_coefs;
        const std::vector<double>& vars = group_params[k].variances;
        // Get the Bayesian linear regression prior for the current cluster
        assert(preds.size() == pred_coefs.size());
        assert(pred_coefs.size() >= num_outcomes);
        // exclude the influence from the outcome variables if 
        // exclude_outcome is true
        size_t num_params = exclude_outcome ? pred_coefs.size() - num_outcomes : 
                            pred_coefs.size();
        for(size_t j = 0; j < num_params; j++)
        {
            double val = dot(preds[j], pred_coefs[j]);
            Gaussian_distribution dist(val, std::sqrt(vars[j]));
            double theta = lss.get_param(j);
            log_p += log_pdf(dist, theta);
        }
        resps[k] = log_p; 
    }

    double sum = log_sum(resps.begin(), resps.end());
    for(size_t k = 0; k < num_clusters; k++)
    {
        resps[k] = std::exp(resps[k] - sum);
    }

    return resps;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::ties::compute_outcome_pred_error
(
    const std::vector<Group_params>& group_params, 
    const Linear_state_space& lss,
    bool average,
    bool prior
)
{
    int group_index = lss.group_index();
    size_t num_outcome_type = lss.num_outcome_type();
    size_t num_outcomes = lss.num_outcomes();
    size_t num_oscillators = lss.num_oscillators();
    size_t num_params = group_params[group_index].pred_coefs.size();
    size_t start_index = num_params - num_outcomes;
    std::vector<Vector> errs(num_outcome_type, Vector((int)num_oscillators, 0.0));
    std::vector<double> resps = get_responsibilities(group_params, lss, true);
    for(size_t j = 0; j < num_outcomes; j++)
    {
        size_t osc_index = j / num_outcome_type;
        size_t outcome_index = j % num_outcome_type;
        double data = lss.get_outcome(outcome_index, osc_index);
        double model = 0.0;
        size_t param_index = start_index + j;
        if(average)
        {
            // marginalizing out the cluster assignment
            for(size_t g = 0; g < resps.size(); g++)
            {
                model += resps[g] * group_params[g].pred_coefs[param_index][0];
            }
        }
        else if(prior) // use prior of the cluster assignment to predict 
        {
            for(size_t g = 0; g < resps.size(); g++)
            {
                model += group_params[g].group_weight * 
                    group_params[g].pred_coefs[param_index][0];
            }
        }
        else
        {
            // use the MAP of the cluster assignment to predict 
            model = group_params[group_index].pred_coefs[param_index][0];
        }
        double diff = data - model;
        errs[outcome_index][osc_index] = fabs(diff); //diff * diff;
    }
    return errs;
}
