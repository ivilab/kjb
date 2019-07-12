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

/* $Id: lss_set.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_distribution.h>
#include <n_cpp/n_svd.h>

#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>

#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/util.h"

using namespace kjb;
using namespace kjb::ties;

Lss_set::Lss_set
(
    const std::vector<size_t>& ids,
    const std::vector<std::vector<std::string> >& mod_names,
    const State_vec& init_states,
    const std::vector<Data>& data,
    double gp_scale,
    double clo_sigma,
    double poly_sigma,
    double outcome_sigma,
    const std::vector<std::string>& obs_names,
    bool fixed_clo,
    double training_percent,
    size_t num_oscillators,
    double init_period,
    double init_damping,
    const Vector& obs_noise_sigmas,
    bool allow_drift,
    size_t num_groups,
    int polynomial_degree,
    const std::vector<std::string>& outcome_names,
    bool ignore_clo,
    const std::string& categorical_moderator,
    const std::string& grouping_info_fp,
    bool use_modal
) : 
    ids_(ids),
    clo_param_size_(param_length(num_oscillators, use_modal)),
    lss_vec_(ids.size()),
    num_groups_(num_groups),
    mod_names_(mod_names),
    fixed_clo_(fixed_clo),
    lss_group_dirty_(true),
    categorical_moderator_(categorical_moderator),
    use_full_cov_(false),
    samples_all_(ids.size())
{
    Double_v gp_scales;
    Double_v gp_sigvars;
    if(ignore_clo) clo_param_size_ = 0;
    if(clo_param_size_ > 0 && allow_drift)
    {
        gp_scales.resize(clo_param_size_, gp_scale);
        gp_sigvars.resize(clo_param_size_, clo_sigma);
    }

    // initialize the individual Linear_state_space
    init_lss(init_states, 
            data, 
            gp_scales, 
            gp_sigvars,
            obs_names, 
            num_oscillators, 
            training_percent, 
            init_period, 
            init_damping, 
            obs_noise_sigmas,
            allow_drift,
            polynomial_degree, 
            outcome_names,
            ignore_clo,
            use_modal);

    // check the predictors dimesion to make sure they have the same dimesion
    // in case most dyads do not share the same moderator values and only 
    // a few share the same moderator values, we consider the moderator values
    // are not shared by the oscilaltors 
    check_predictors_dimension();

    params_.resize(num_groups_);
    const std::vector<Vector>& preds = lss_vec_[0].get_predictors();
    polynomial_coef_size_ = lss_vec_[0].num_polynomial_coefs();
    outcome_size_ = lss_vec_[0].num_outcomes();

    size_t num_params = preds.size() - lss_vec_[0].num_outcomes();

    // resize cached_data
    cached_data_.resize(num_groups, Cached_data(preds.size(), ids.size()));

    double weight = 1.0 / num_groups_;
    for(size_t g = 0; g < num_groups_; g++)
    {
        // initialize the pred coefs to be zero 
        params_[g].pred_coefs.resize(preds.size());
        params_[g].variances.resize(preds.size());
        for(size_t i = 0; i < num_params; i++)
        {
            params_[g].pred_coefs[i] = Vector(preds[i].size(), 0.0);
        }
        // for outcome 
        for(size_t i = num_params; i < preds.size(); i++)
        {
            params_[g].pred_coefs[i] = Vector(1, 0.0);
        }

        // initialize the variance
        if(!fixed_clo_)
        {
            // clo params
            double clo_var = clo_sigma * clo_sigma;
            double poly_var = poly_sigma * poly_sigma;
            double outcome_var = outcome_sigma * outcome_sigma;
            Vector var = build_vector(clo_param_size_, clo_var,
                                   polynomial_coef_size_, poly_var,
                                   outcome_size_, outcome_var);
            params_[g].variances = std::vector<double>(var.begin(), var.end());
        }
        // assume each cluster has equal weight
        params_[g].group_weight = weight;
    }

    // update the means and variances
    update_means();
    update_variances();

    // initialize the design matrix for the bayesian linear regression prior
    if(!fixed_clo_)
    {
        init_design_matrix();
    }

    // parse in the group info if the grouping info file exists
    if(kjb_c::is_file(grouping_info_fp.c_str()))
    {
        std::ifstream ifs(grouping_info_fp.c_str());
        ifs >> group_map_;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::init_lss
(
    const State_vec& init_states,
    const std::vector<Data>& data,
    const Double_v& gp_scales,
    const Double_v& gp_sigvars,
    const std::vector<std::string>& obs_names,
    size_t num_oscillators,
    double training_percent,
    double init_period,
    double init_damping,
    const Vector& obs_noise_sigmas,
    bool allow_drift,
    int polynomial_degree,
    const std::vector<std::string>& outcome_names,
    bool ignore_clo,
    bool use_modal
)
{
    size_t num_lss = data.size();

    IFT(lss_vec_.size() == num_lss, Illegal_argument,
            "The number of ids is different from the number "
            "of dyads");

    IFT(init_states.size() == num_lss, Illegal_argument, 
            "The number of initial states is different from "
            "the number of dyads");

    // record the index of each group (eg. starting with (0, 0) for two groups 
    std::vector<size_t> dyad_index(num_groups_, 0);
    for(size_t i = 0; i < num_lss; i++)
    {
        size_t length = std::ceil(data[i].times.size() * training_percent);
        Double_v times(length);
        double start_time = data[i].times.front();
        std::generate(times.begin(), times.end(), Increment<double>(start_time));

        Coupled_oscillator_v clos(times.size() - 1, 
                Coupled_oscillator(num_oscillators,
                                   init_period,
                                   init_damping,
                                   use_modal,
                                   false));
       
        int group_index = data[i].group_index;
        if(categorical_moderator_ == "")
        {
            group_index = 0;
        }
        lss_vec_[i] = Linear_state_space(
                                    times, 
                                    init_states[i], 
                                    clos,
                                    obs_names,
                                    obs_noise_sigmas,
                                    polynomial_degree,
                                    outcome_names,
                                    data[i].outcomes,
                                    group_index,
                                    ignore_clo);
        // init the predictors 
        lss_vec_[i].init_predictors(data[i].moderators, mod_names_);
        if(polynomial_degree >= 0)
        {
            std::string obs = data[i].observables.begin()->first;
            std::pair<Vector, Vector> stats = get_mean_variances(data[i], obs);
            //set the offset based on average
            for(size_t m = 0; m < lss_vec_[i].num_oscillators(); m++)
            {
                lss_vec_[i].set_polynomial_coefs(m, 0, stats.first[m]);
            }
        }

        // init GP
        if(!gp_scales.empty())
        {
            if(cached_data_.empty())
            {
                cached_data_.resize(num_groups_, 
                        Cached_data(lss_vec_[i].get_predictors().size(), lss_vec_.size()));
            }
            assert(allow_drift);
            IFT(gp_scales.size() == clo_param_size_, Illegal_argument, 
               "The size of gp_scales differ from person-dependent params.");
            IFT(gp_sigvars.size() == clo_param_size_, Illegal_argument, 
               "The size of gp_sigvars differ from person-dependent params.");

            std::vector<gp::Constant> gp_means(clo_param_size_, 0.0);
            // add noise to CLO parmas
            BOOST_FOREACH(Coupled_oscillator& clo, 
                          lss_vec_[i].coupled_oscillators())
            {
                //BOOST_FOREACH(double& param, clo.params())
                for(size_t jjj = 0; jjj < clo.num_params(); jjj++)
                {
                    double val = clo.get_param(jjj) + kjb_c::kjb_rand() * 1e-4;
                    clo.set_param(jjj, val);
                }
            }
            // will update the gp means in update_lss_mean
            lss_vec_[i].init_gp(gp_scales, gp_sigvars, gp_means);

            // init the covariance
            size_t group_index = lss_vec_[i].group_index();
            size_t& cur_index = dyad_index[group_index];
            for(size_t j = 0; j < clo_param_size_; j++)
            {
                cached_data_[group_index].K_inv_[j][cur_index] 
                                = get_covariance_matrix_inv(lss_vec_[i], j);
            }
            cur_index++;
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::read
(
    const std::string& indir,
    const std::vector<Data>& data_all,
    const Group_map& group_map
)
{
    using namespace std;
    using namespace boost;

    // clear stuff 
    //ids_.clear();
    //pred_coefs_.clear();

    // read in the ids
    string id_fp(indir + "/ids.txt");
    ids_ = parse_list(id_fp);

    // read in the group parameters
    string shared_fp(indir + "/params.txt");
    parse_group_params(shared_fp, params_);

    // read in each Linear_state_space 
    format couple_path_fmt(indir + "/%04d");
    assert(lss_vec_.size() == ids_.size());

    for(size_t i = 0; i < ids_.size(); i++)
    {
        string couple_fp = (couple_path_fmt % ids_[i]).str();
        double start_time = data_all[i].times.front(); 
        // parse in group info 
        if(!group_map.empty())
        {
            for(size_t g = 0; g < num_groups_; g++)
            {
                std::string group_name = group_map.left.find(g)->second;
                boost::format cur_out_fmt(indir + "/" + group_name + "/%04d/");
                couple_fp = (cur_out_fmt % ids_[i]).str();
                if(kjb_c::is_directory(couple_fp.c_str()))
                {
                    break;
                }
            }
        }
        lss_vec_[i].read(couple_fp, start_time);
        lss_vec_[i].init_predictors(data_all[i].moderators, mod_names_);
        // read in the samples if the sample dir exit 
        std::string sample_dir(couple_fp + "/samples/");
        if(kjb_c::is_directory(sample_dir.c_str()))
        {
            samples_all_[i] = read_lss_samples(sample_dir, start_time);
        }
    }
    check_predictors_dimension();
    init_design_matrix();
    update_means();
    update_variances();

    string group_fp(indir + "/group_params.txt");
    if(!kjb_c::is_file(group_fp.c_str()))
    {
        update_means();
        update_variances();
    }
    else
    {
        std::cout << "reading in group info: \n";
        ifstream group_ifs(group_fp.c_str());
        string line;
        getline(group_ifs, line);
        // number of groups
        num_groups_ = boost::lexical_cast<int>(line);
       
        // get the groups of each individual
        getline(group_ifs, line);
        istringstream istr(line);
        std::vector<int> groups;
        copy(istream_iterator<int>(istr), istream_iterator<int>(), 
                back_inserter(groups));
        assert(groups.size() == lss_vec_.size());
        for(size_t i = 0; i < lss_vec_.size(); i++)
        {
            lss_vec_[i].group_index() = groups[i];
        }
       
        // get the groups weights
        getline(group_ifs, line);
        istringstream istr_weights(line);
        group_weights_.clear();
        copy(istream_iterator<double>(istr_weights), istream_iterator<double>(), 
                back_inserter(group_weights_));
        // get the means
        group_means_.clear();
        for(int i = 0; i < num_groups_; i++)
        {
            Vector mean;
            stream_read_vector(group_ifs, mean);
            group_means_.push_back(mean);
        }
        // get the variances
        group_covariances_.clear();
        for(int i = 0; i < num_groups_; i++)
        {
            Matrix cov;
            stream_read_matrix(group_ifs, cov);
            group_covariances_.push_back(cov);
        }
        update_lss_group();
    }


}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::read_shared_params(const std::string& fp)
{
    std::ifstream ifs(fp.c_str());
    IFTD(ifs.is_open(), IO_error, "Can not open file %s", (fp.c_str()));
    for(size_t g = 0; g < num_groups_; g++)
    {
        parse_shared_params(ifs, params_[g]);
    }

    // update means and variances
    update_means();
    update_variances();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::update_lss_mean(size_t lss_index) const
{
    IFT(lss_index < lss_vec_.size(), Runtime_error, 
            "Index out of bound in Lss_set::update_clo\n");

    Linear_state_space& lss = lss_vec_[lss_index];
    // check the number of time points of each linear_state_space
    IFT(lss.get_times().size() > 1, Illegal_argument, 
            "Dyad has one or zero time points"); 
    // asseume the group ID starts from 1
    size_t group_index = lss.group_index();
    assert(group_index < params_.size());
    const std::vector<Vector>& preds = lss_vec_[lss_index].get_predictors();
    // get the parameters for the right cluster/group
    size_t num_poly_coefs = num_polynomial_params();
    size_t moderated_params = clo_param_size_ + num_poly_coefs;
    assert(preds.size() == params_[group_index].pred_coefs.size());

    // update the new values 
    size_t i = 0;
    while(i < clo_param_size_)
    {
        double val = dot(preds[i], params_[group_index].pred_coefs[i]);
        // i is a person-specific index, so there is corresponding mean 
        //if(persons_[i])
        if(!lss.allow_drift())
        {
            if(!fixed_clo_)
            {
                lss.set_clo_mean(i, val);
            }
            else
            {
                Coupled_oscillator_v& clos = lss.coupled_oscillators();
                BOOST_FOREACH(Coupled_oscillator& clo, clos)
                {
                    clo.set_param(i, val);
                }
                lss.changed_index() = 0;
            }
        }
        else //if(lss.allow_drift())
        {
            lss.set_gp_mean(i, gp::Constant(val));
        }
        i++;
    }
    // update the poly param mean
    while(i < moderated_params)
    {
        double val = dot(preds[i], params_[group_index].pred_coefs[i]);
        size_t index = i - clo_param_size_;
        size_t osc_index = index / lss.polynomial_dim_per_osc();
        size_t poly_index = index % lss.polynomial_dim_per_osc(); 
        if(!fixed_clo_)
        {
            lss.set_polynomial_coefs_mean(osc_index, poly_index, val);
        }
        else
        {
            lss.set_polynomial_coefs(osc_index, poly_index, val);
        }
        i++;
    }
    // update the outcome mean
    while(i < params_[group_index].pred_coefs.size())
    {
        // the outcomes only has one value even we stored it in a vector
        assert(params_[group_index].pred_coefs[i].size() == 1);
        double val = params_[group_index].pred_coefs[i].front();
        size_t index = i - moderated_params;
        lss.set_outcome_mean(index, val);
        i++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::update_lss_variance(size_t lss_index) const

{
    IFT(lss_index < lss_vec_.size(), Runtime_error, 
            "Index out of bound in Lss_set::update_clo\n");

    Linear_state_space& lss = lss_vec_[lss_index];
    // check the number of time points of each linear_state_space
    IFT(lss.get_times().size() > 1, Illegal_argument, 
            "Dyad has one or zero time points"); 

    size_t group_index = lss.group_index();
    assert(group_index >= 0 && group_index < params_.size());

    // get the parameters for the right cluster/group
    assert(params_[group_index].variances.size() == params_[group_index].pred_coefs.size());
    size_t num_poly_coefs = num_polynomial_params();
    size_t moderated_params = clo_param_size_ + num_poly_coefs;

    // update the new values 
    size_t i = 0;
    while(i < clo_param_size_)
    {
        double val = params_[group_index].variances[i];
        if(lss.allow_drift())
        {
            lss.set_gp_sigvar(i, val);
            lss.update_gp();
        }
        else 
        {
            lss.set_clo_variance(i, val);
        }
        i++;
    }
    // update the polynomial variances
    while(i < moderated_params) 
    {
        double val = params_[group_index].variances[i];
        size_t index = i - clo_param_size_;
        size_t osc_index = index / lss.polynomial_dim_per_osc();
        size_t poly_index = index % lss.polynomial_dim_per_osc(); 
        lss.set_polynomial_coefs_var(osc_index, poly_index, val);
        i++;
    }

    // outcome
    while(i < params_[group_index].variances.size())
    {
        double val = params_[group_index].variances[i];
        size_t index = i - moderated_params;
        lss.set_outcome_var(index, val);
        i++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::update_gps
(
    const Double_v& gp_scales, 
    const Double_v& gp_sigvars
) const
{
    assert(gp_scales.size() == gp_sigvars.size());
    for(size_t i = 0; i < lss_vec_.size(); i++)
    {
        for(size_t j = 0; j < gp_scales.size(); j++)
        {
            double scale = gp_scales[j];
            double sigvar = gp_sigvars[j];
            update_gp_scales(i, j, scale);
            update_gp_sigvars(i, j, sigvar);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix Lss_set::get_covariance_matrix_inv
(
    const Linear_state_space& lss, 
    size_t param_index
) const
{
    const gp::Inputs& inputs = lss.get_gp_inputs();
    // NOTE: we need to set the covariance variance to be 1.0 
    // since the variance is already encoded in the lss sigma
    gp::Squared_exponential cv(lss.gp_scales()[param_index], 1.0);
    Matrix K = apply_cf(cv, inputs.begin(), inputs.end());
    return K.inverse();
    /*Svd svd(K);
    const Matrix& U = svd.u();
    const Matrix& D = svd.d();
    const Matrix& V_t = svd.vt();

    Matrix D_0_inv(D.get_num_rows(), D.get_num_rows(), 0.0);
    const double eps = 1e-5;
    for(size_t i = 0; i < D_0_inv.get_num_rows(); i++)
    {
        if(D(i, 0) > eps)
        {
            D_0_inv(i, i) = 1.0/D(i, i);
        }
    }
    //K_inv_[param_index][i] = K.inverse();
    K_inv_[param_index][i] = V_t.transpose() * D_0_inv * U.transpose();*/
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::write
(
    const std::string& out_dir
) const
{
    ETX(kjb_c::kjb_mkdir(out_dir.c_str()));

    // output ids 
    std::string id_fp(out_dir + "/ids.txt");
    std::ofstream id_ofs(id_fp.c_str());
    IFTD(id_ofs.is_open(), IO_error, "can't open file %s", (id_fp.c_str()));
    BOOST_FOREACH(size_t id, ids_)
    {
        id_ofs << id << std::endl;
    }
    id_ofs.close();

    // output the bmi coefs 
    std::string shared_fp(out_dir + "/params.txt");
    std::ofstream shared_ofs(shared_fp.c_str());
    IFTD(shared_ofs.is_open(), IO_error, 
            "Can't open file %s", (shared_fp.c_str()));

    assert(!params_.empty());
    for(size_t g = 0; g < params_.size(); g++)
    {
        // lss variance
        if(!params_[g].variances.empty())
        {
            shared_ofs << "variance" << std::endl; 
            std::setw(15);
            std::setprecision(10);
            std::copy(params_[g].variances.begin(), 
                      params_[g].variances.end(),
                      std::ostream_iterator<double>(shared_ofs, " " ));
            shared_ofs << std::endl;
        }
        
        // coefs 
        shared_ofs << "coef" << std::endl;
        for(size_t i = 0; i < params_[g].pred_coefs.size() ; i++)
        {
            shared_ofs << params_[g].pred_coefs[i] << std::endl;
        }

        shared_ofs << "weight" << std::endl;
        shared_ofs << params_[g].group_weight << std::endl;
    }
    shared_ofs.close();

    // output the linear_state_space
    boost::format out_fmt(out_dir + "/%04d/");
    assert(ids_.size() == lss_vec_.size());

    for(size_t i = 0; i < lss_vec_.size(); i++)
    {
        std::string out_fp = (out_fmt % ids_[i]).str();
        // only when write to different groups when the categorical 
        // moderator is specified 
        if(categorical_moderator_ != "")
        {
            assert(num_groups_ > 1);
            std::string group_name = "";
            size_t group_index = lss_vec_[i].group_index();
            group_name = group_map_.left.find(group_index)->second;
            boost::format cur_out_fmt(out_dir + "/" + group_name + "/%04d/");
            out_fp = (cur_out_fmt % ids_[i]).str();
        }
        lss_vec_[i].write(out_fp);

        // write out the samples 
        /*if(!samples_all_[i].empty())
        {
            boost::format sample_fmt(out_fp + "/samples/%04d/");
            for(size_t s = 0; s < samples_all_[i].size(); i++)
            {
                std::string sample_fp = (sample_fmt % (s+1)).str();
                samples_all_[i][s].write(sample_fp);
            }
        }*/
    }

    // output the group params
    if(num_groups_ > 1 && use_full_cov_)
    {
        std::string group_fp(out_dir + "/group_params.txt");
        std::ofstream group_ofs(group_fp.c_str());
        IFTD(group_ofs.is_open(), IO_error, 
                "Can't open file %s", (group_fp.c_str()));
        group_ofs << group_means_.size() << std::endl;
        // write the weights
        group_ofs << group_weights_ << std::endl;
        BOOST_FOREACH(const Vector& mean, group_means_)
        {
            stream_write_vector(group_ofs, mean);
            group_ofs << std::endl;
        }
        //std::for_each(group_means_.begin(), group_means_.end(), 
                //boost::bind(stream_write_vector, boost::ref(group_ofs), _1));
        std::for_each(group_covariances_.begin(), group_covariances_.end(), 
                boost::bind(stream_write_matrix, boost::ref(group_ofs), _1));
        group_ofs.close();

        // write couple ids in groups (For Debug purposes)
        /*boost::format cluster_fp(out_dir + "/cluster_%d.txt");
        // open the cluster file
        boost::scoped_ptr<std::ofstream> cluster_fs[num_groups_];
        for(size_t k = 0; k < num_groups_; k++)
        {
            std::string fp = (cluster_fp % k).str();
            cluster_fs[k].reset(new std::ofstream(fp.c_str()));
            IFTD(cluster_fs[k]->is_open(), IO_error, 
                    "Can't open file %s ", (fp.c_str()));
        }
        for(size_t i = 0; i < lss_vec_.size(); i++)
        {
            int lss_group = lss_vec_[i].group();
            assert(group_map_.find(lss_group) != group_map_.end());
            size_t group_index = group_map_.find(lss_group)->second;
            if(lss_group < 0)
            {
                std::cerr << "WARNING: group is not initialized\n";
            }
            else
            {
                *cluster_fs[group_index] << ids_[i] << std::endl;
            }
        }*/
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

State_type Lss_set::init_state_mean() const
{
    State_type mean(4, 0.0);
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        const State_type& init_state = lss.init_state();
        for(size_t i = 0; i < init_state.size(); i++)
        {
            mean[i] += init_state[i];
        }
    }
    BOOST_FOREACH(double& val, mean)
    {
        val /= lss_vec_.size();
    }
    
    return mean;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<Vector> > Lss_set::get_lss_params() const 
{
    // indexed by [CLUSTER][PARAM][DYAD]
    std::vector<std::vector<Vector> > res(num_groups_);

    // clear the old CLO params (when drift)
    if(allow_drift())
    {
        for(size_t c = 0; c < num_groups_; c++)
        {
            cached_data_[c].y_T_K_inv_y_.clear();
            cached_data_[c].y_T_K_inv_y_.resize(clo_param_size_, 0.0);
        }
    }

    // record the index of each group
    std::vector<size_t> dyad_index(num_groups_, 0);

    // get the params
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        assert(clo_param_size_ == lss.num_clo_params());
        size_t num_params = clo_param_size_ +  
                            lss.num_polynomial_coefs() + 
                            lss.num_outcomes();

        size_t group_index = lss.group_index();
        //std::cout << " group_index: " << group_index << std::endl;
        assert(group_index < num_groups_);
        if(res[group_index].empty())
        {
            res[group_index] = std::vector<Vector>(num_params);
        }
        // get the CLO param
        if(lss.allow_drift())
        {
            size_t& cur_index = dyad_index[group_index];
            std::vector<Vector> y((int)clo_param_size_);
            BOOST_FOREACH(const Coupled_oscillator& co, 
                                lss.coupled_oscillators())
            {
                for(size_t i = 0; i < co.num_params(); i++)
                {
                    double clo_param = co.get_param(i);
                    y[i].push_back(clo_param);
                    res[group_index][i].push_back(clo_param);
                }
            }
            // compute y_T * K_inv * y
            for(size_t i = 0; i < clo_param_size_; i++)
            {
                Vector yt = y[i] * 
                    cached_data_[group_index].K_inv_[i][cur_index];
                cached_data_[group_index].y_T_K_inv_y_[i] += dot(yt, y[i]);
            }
            cur_index++;
        }
        else
        {
            for(size_t i = 0; i < clo_param_size_; i++)
            {
                res[group_index][i].push_back(lss.get_param(i));
            }
        }

        for(size_t i = clo_param_size_; i < num_params; i++)
        {
            res[group_index][i].push_back(lss.get_param(i));
        }
    }

    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*std::vector<std::vector<Vector> > Lss_set::get_clo_params() const 
{
    // get all the times
    std::vector<int> num_times(num_groups_, 0);
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        size_t group_index = lss.group_index();
        num_times[group_index] += 
            lss.allow_drift() ? lss.coupled_oscillators().size() : 1;
    }

    // indexed by [CLUSTER][PARAM][DYAD]
    std::vector<std::vector<Vector> > res(num_groups_);
    for(size_t c = 0; c < num_groups_; c++)
    {
        res[c] = std::vector<Vector>(clo_param_size_, 
                                    Vector(num_times[c], 0.0));
    }

    // clear out the y_T_K_inv_Y 
    if(allow_drift())
    {
        // PLEASE REMEMBER TO CLEAR BEFORE RESIZE 
        // since if the size matches, it won't set it to 0.0!!!!!!!
        for(size_t c = 0; c < num_groups_; c++)
        {
            cached_data_[c].y_T_K_inv_y_.clear();
            cached_data_[c].y_T_K_inv_y_.resize(clo_param_size_, 0.0);
        }
    }

    std::vector<size_t> dyad_id(num_groups_, 0);
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        size_t group_index = lss.group_index();
        assert(group_index < num_groups_);
        size_t& cur_id = dyad_id[group_index];
        if(lss.allow_drift())
        {
            std::vector<Vector> y((int)clo_param_size_);
            BOOST_FOREACH(const Coupled_oscillator& co, 
                                lss.coupled_oscillators())
            {
                for(size_t i = 0; i < co.num_params(); i++)
                {
                    y[i].push_back(co.get_param(i));
                }
            }
            for(size_t i = 0; i < clo_param_size_; i++)
            {
                cached_data_[group_index].y_T_K_inv_y_[i] 
                    += dot(y[i] * cached_data_[group_index].K_inv_[i][cur_id], 
                           y[i]);
            }
        }
        else
        {
            const Coupled_oscillator& co = lss.coupled_oscillators().front();
            for(size_t i = 0; i < co.num_params(); i++)
            {
                res[group_index][i][cur_id] = co.get_param(i);
            }
        }
        cur_id++;
    }
    return res;
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*std::vector<std::vector<Vector> > Lss_set::get_polynomial_coefs() const 
{
    // indexed by [CLUSTER][PARAM][DYAD]
    size_t total_params = lss_vec_.front().num_polynomial_coefs();
    std::vector<std::vector<Vector> > res(num_groups_, 
            std::vector<Vector>(total_params));
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        const std::vector<Vector>& terms = lss.polynomial_coefs();
        size_t group_index = lss.group_index();
        size_t index = 0;
        for(size_t i = 0; i < terms.size(); i++)
        {
            for(size_t j = 0; j < terms[i].size(); j++)
            {
                res[group_index][index].push_back(terms[i][j]);
                index++;
            }
        }
        assert(index == total_params);
    }
    return res;
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::parse_obs_coef(const std::string& obs_fname) const
{
    std::for_each(lss_vec_.begin(), lss_vec_.end(), 
            boost::bind(&Linear_state_space::parse_obs_coef, _1, obs_fname));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::init_design_matrix() const
{
    using namespace std;
    size_t num_params = params_.front().pred_coefs.size();

    vector<vector<Matrix> > X_all(num_groups_, vector<Matrix>(num_params, Matrix()));

    // Initialize the design matrix for the Bayesian linear regression model 
    size_t num_lss = lss_vec_.size();
    const vector<Vector>& pred_vals = lss_vec_.front().get_predictors();
    std::vector<vector<int> > lengths(num_groups_, 
            std::vector<int>(pred_vals.size(), 0));
    BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
    {
        size_t group_index = lss.group_index();
        for(size_t param_index = 0; param_index < pred_vals.size(); param_index++)
        {
            lengths[group_index][param_index] += 
                lss.allow_drift() && param_index < clo_param_size_ ? 
                lss.coupled_oscillators().size() : 1;
        }
    }

    for(size_t param_index = 0; param_index < pred_vals.size(); param_index++)
    {
        assert(param_index < params_.front().pred_coefs.size());
        std::vector<size_t> row_counter(num_groups_, 0);
        int D = pred_vals[param_index].size(); 
        for(size_t c = 0; c < num_groups_; c++)
        {
            int length = lengths[c][param_index];
            if(length > 0)
            {
                X_all[c][param_index] = Matrix(length, D, 0.0);
                cached_data_[c].X_t_K_inv_[param_index] = Matrix(D, length, 0.0);
            }
        }
        for(size_t i = 0; i < num_lss; i++)
        {
            const vector<Vector>& predictors = lss_vec_[i].get_predictors(); 
            size_t group_index = lss_vec_[i].group_index();
            // dimesion of the design matrix
            int cols = predictors[param_index].size();
            // number of time points
            int rows = lss_vec_[i].allow_drift() && param_index < clo_param_size_? 
                        lss_vec_[i].get_times().size() - 1: 1;
            Matrix X(rows, cols, 0.0);
            for(size_t k = 0; k < rows; k++)
            {
                X.set_row(k, predictors[param_index]);
                X_all[group_index][param_index].set_row(
                                           row_counter[group_index]++, 
                                           predictors[param_index]);
            }
            cached_data_[group_index].X_all_[param_index][i] = X;
        }
        //if(!lss_vec_.front().allow_drift())
        {
            for(size_t c = 0; c < num_groups_; c++)
            {
                cached_data_[c].X_t_K_inv_[param_index] = 
                                    X_all[c][param_index].transpose();
                cached_data_[c].X_t_K_inv_X_[param_index] = 
                   cached_data_[c].X_t_K_inv_[param_index] * X_all[c][param_index];

                /*std::cout << "X_t_K_inv_: " << param_index << ": " << 
                            cached_data_[c].X_t_K_inv_[param_index].get_num_cols()
                            << " " << 
                            cached_data_[c].X_t_K_inv_[param_index].get_num_rows()
                            << std::endl;*/
            }
        }
    }
    if(lss_vec_.front().allow_drift())
    {
        update_covariance_matrix();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::update_covariance_matrix() const 
{
    for(size_t param_index = 0; param_index < clo_param_size_; param_index++)
    {
        // initialize the matrix to be zero 
        int pred_length = params_.front().pred_coefs[param_index].size();
        for(size_t c = 0; c < num_groups_; c++)
        {
            cached_data_[c].X_t_K_inv_X_[param_index] = 
                                    Matrix(pred_length, pred_length, 0.0);
        }

        // go through each lss
        size_t counter = 0;
        size_t num_lss = lss_vec_.size();
        for(size_t i = 0; i < num_lss; i++)
        {
            size_t gindex = lss_vec_[i].group_index();
            cached_data_[gindex].K_inv_[param_index][i] = 
                        get_covariance_matrix_inv(lss_vec_[i], param_index);
            assert(lss_vec_[i].allow_drift());
            const Matrix& X = cached_data_[gindex].X_all_[param_index][i]; 
            const Matrix& K_inv = cached_data_[gindex].K_inv_[param_index][i];

            Matrix temp = X.transpose() * K_inv;
            cached_data_[gindex].X_t_K_inv_X_[param_index] += temp * X;
            assert(temp.get_num_rows() == pred_length);
            assert(cached_data_[gindex].X_t_K_inv_[param_index].get_num_rows() 
                        == pred_length);
            for(size_t n = 0; n < temp.get_num_cols(); n++)
            {
                for(size_t m = 0; m < temp.get_num_rows(); m++)
                {
                    cached_data_[gindex].X_t_K_inv_[param_index](m, counter + n) 
                                                            = temp(m, n);
                }
            }
            counter += temp.get_num_cols();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::check_predictors_dimension()
{
    size_t num_params = lss_vec_.front().get_predictors().size();
    for(size_t i = 0; i < num_params; i++)
    {
        std::map<size_t, size_t> size_map; 
        for(size_t j = 0; j < lss_vec_.size(); j++)
        {
            const std::vector<Vector>& preds = lss_vec_[j].get_predictors();
            // the size of the predictors for lss j param i 
            size_t sz = preds[i].size();
            std::map<size_t, size_t>::iterator it = size_map.find(sz);
            // if the size was not in the map, then store the sz into the map
            if(it == size_map.end())
            {
                size_map[sz] = 0;
            }
            else // if the size is in the map, increase the counts of this size
            {
                (it->second)++;
            }
        }
        // start from the end of the size map, since size are sorted in
        // increasing order
        std::map<size_t, size_t>::reverse_iterator itt = size_map.rbegin();
        // get the pred size from the presetted pred_coefs
        size_t pred_size = 0;
        if(!params_.empty())
        {
            pred_size = params_.front().pred_coefs[i].size();
        }
        // Take the maximum between the maximum size and the preset size of
        // pred_coefs
        size_t corrected_sz = std::max(pred_size, itt->first);
        itt++;
        //if(itt != size_map.rend())
        //{
        for(size_t j = 0; j < lss_vec_.size(); j++)
        {
            std::vector<Vector>& preds = lss_vec_[j].predictors();
            while(preds[i].size() < corrected_sz)
            {
                preds[i].push_back(preds[i].back());
            }
        }
        //}
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& kjb::ties::operator <<
(
    std::ostream& ost, 
    const Lss_set& lsss
)
{
    for(size_t g = 0; g < lsss.params_.size(); g++)
    {
        // write the shared coefs 
        std::copy(lsss.pred_coefs(g).begin(), lsss.pred_coefs(g).end(),
                  std::ostream_iterator<Vector>(ost, " "));
        // write the variance 
        std::copy(lsss.variances(g).begin(), lsss.variances(g).end(),
                    std::ostream_iterator<double>(ost, " "));
        // write the weights
        ost << lsss.group_params()[g].group_weight << " ";
    }

    // write the noise sigma
    ost << lsss.get_noise_sigmas() << " ";
    // observables
    if(lsss.num_observables() > 1)
    {
        for(size_t m = 1; m < lsss.obs_coefs().size(); m++)
        {
            BOOST_FOREACH(const Vector& v, lsss.obs_coefs()[m])
            {
                ost << v << " ";
            }
        }
    }

    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::parse_group_params
(
    const std::string& param_fp,
    std::vector<Group_params>& params
)
{
    boost::scoped_ptr<std::ifstream> shared_ifs, poly_ifs; 
    shared_ifs.reset(new std::ifstream(param_fp.c_str()));
    IFTD(!shared_ifs->fail(), IO_error, "Can't open file %s", (param_fp.c_str()));
   
    params.clear();
    size_t counts = 0;
    while(!shared_ifs->eof())
    {
        Group_params param;
        parse_shared_params(*shared_ifs, param);
        if(!param.pred_coefs.front().empty()) params.push_back(param);
        counts++;
    }
    assert(counts >= 1);
    counts--;
    num_groups_ = counts;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::add(const Lss_set& lsss)
{
    assert(lsss.num_groups() == num_groups_);
    // pred_coefs and variances 
    for(size_t g = 0; g < num_groups_; g++)
    {
        std::vector<Vector>& pred_coefs = lsss.pred_coefs(g);
        std::vector<double>& variances = lsss.variances(g);
        assert(pred_coefs.size() == variances.size());
        for(size_t i = 0; i < pred_coefs.size(); i++)
        {
            params_[g].pred_coefs[i] += pred_coefs[i];
            params_[g].variances[i] += variances[i];
        }
    }
    // obs noise
    Vector obs_noise = get_noise_sigmas() + lsss.get_noise_sigmas();
    set_noise_sigmas(obs_noise);

    // obs coefs
    for(size_t i = 0; i < obs_coefs().size(); i++)
    {
        for(size_t j = 0; j < obs_coefs()[i].size(); j++)
        {
            set_obs_coef(i, j, obs_coefs()[i][j] + lsss.obs_coefs()[i][j]);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::average(size_t n) 
{
    // pred_coefs and variances 
    for(size_t g = 0; g < num_groups_; g++)
    {
        for(size_t i = 0; i < params_[g].pred_coefs.size(); i++)
        {
            params_[g].pred_coefs[i] /= n;
            params_[g].variances[i] /= n;
        }
    }
    // obs noise
    Vector obs_noise = get_noise_sigmas()/n;
    set_noise_sigmas(obs_noise);

    // obs coefs
    for(size_t i = 0; i < obs_coefs().size(); i++)
    {
        for(size_t j = 0; j < obs_coefs()[i].size(); j++)
        {
            set_obs_coef(i, j, obs_coefs()[i][j]/n);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set::report_outcome_pred_errs(const std::string& out_dp) const
{
    if(lss_vec_.empty()) return;
    if(lss_vec_.front().num_outcomes() == 0) return;
    std::string out_fp(out_dp + "/outcome_errs.txt");
    const std::vector<std::string>& outcome_names = lss_vec_.front().outcome_names();
    std::ofstream ofs(out_fp.c_str());
    IFTD(ofs.is_open(), IO_error, "can't open file %s", (out_fp.c_str()));
    /*ofs << "id, ";
    std::copy(outcome_names.begin(), outcome_names.end(), 
              std::ostream_iterator<std::string>(ofs, ", "));
    ofs << std::endl;*/

    for(size_t i = 0; i < lss_vec_.size(); i++)
    {
        ofs << ids_[i] << " ";
        std::vector<Vector> outcome_err = compute_outcome_pred_error(
                params_, lss_vec_[i], false, false);
        // write the error 
        for(size_t j = 0; j < outcome_err.size(); j++)
        {
            for(size_t o = 0; o < outcome_err[j].size(); o++)
            {
                ofs << outcome_err[j][o] << " ";
            }
        }
        ofs << std::endl;
    }
    ofs.close();

    out_fp = std::string(out_dp + "/outcome_errs_ave.txt");
    std::ofstream ave_ofs(out_fp.c_str());
    IFTD(ave_ofs.is_open(), IO_error, "can't open file %s", (out_fp.c_str()));
    for(size_t i = 0; i < lss_vec_.size(); i++)
    {
        ave_ofs << ids_[i] << " ";
        std::vector<Vector> outcome_err = compute_outcome_pred_error(
                params_, lss_vec_[i], true, false);
        // write the error
        for(size_t j = 0; j < outcome_err.size(); j++)
        {
            for(size_t o = 0; o < outcome_err[j].size(); o++)
            {
                ave_ofs << outcome_err[j][o] << " ";
            }
        }
        ave_ofs << std::endl;
    }
    ave_ofs.close();

    out_fp = std::string(out_dp + "/outcome_errs_prior.txt");
    std::ofstream prior_ofs(out_fp.c_str());
    IFTD(prior_ofs.is_open(), IO_error, "can't open file %s", (out_fp.c_str()));
    for(size_t i = 0; i < lss_vec_.size(); i++)
    {
        prior_ofs << ids_[i] << " ";
        std::vector<Vector> outcome_err = compute_outcome_pred_error(
                params_, lss_vec_[i], false, true);
        // write the error
        for(size_t j = 0; j < outcome_err.size(); j++)
        {
            for(size_t o = 0; o < outcome_err[j].size(); o++)
            {
                prior_ofs << outcome_err[j][o] << " ";
            }
        }
        prior_ofs << std::endl;
    }
    prior_ofs.close();
}
