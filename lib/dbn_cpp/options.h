/* =========================================================================== *
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: options.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_OPTIONS_H
#define KJB_TIES_OPTIONS_H

#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <l_cpp/l_exception.h>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include "dbn_cpp/experiment.h"

namespace bpo = boost::program_options;
using std::string;

namespace kjb {
namespace ties {

/** @brief  Create options description for data options. */
inline
bpo::options_description make_options(Data_options& opts)
{
    bpo::options_description options("Data-reading parameters");
    options.add_options()
        ("data-dir,D", bpo::value<string>(&opts.data_dp),
            "data directory, contains all the couple data")
        ("id-list,L", 
            bpo::value<std::vector<std::string> >(&opts.id_list_fp)->multitoken(),
            "A file contains all the training couple ids")
        ("data-fname,F", bpo::value<string>(&opts.data_fname),
            "data file that contains individual dyad data, in \".txt\" format")
        ("data-csv-fname", bpo::value<string>(&opts.csv_fp),
            "CSV file that contains the whole data set")
        ("distinguisher", 
             bpo::value<string>(&opts.distinguisher)->default_value(""),
            "a string that distinguishes the two participants in a dyad. "
            " (ususaglly is marked as 0, 1, 2 .. in the field ")
        ("categorical-moderator", bpo::value<string>(&opts.grouping_var),
            "variable that represents data in groups")
        ("average-size", 
            bpo::value<size_t>(&opts.average_size)->default_value(1), 
            "number of data points to be averaged over")
        ("standardize-data", 
            bpo::bool_switch(&opts.standardize_data)->default_value(false), 
            "if present, standardize the data to have mean 0 and variance")
        ("mean-centered-data", 
            bpo::bool_switch(&opts.mean_centered_data)->default_value(false), 
            "if present, standardize the data to have mean 0")
        ("outcome", 
            bpo::value<std::vector<std::string> >(&opts.outcomes)->multitoken(), 
            "names of the outcome variables");

    return options;
}

/** @brief  Create options description for lss options. */
inline
bpo::options_description make_options(Lss_options& opts)
{
    bpo::options_description options("Parameters for a Linear state space");
    options.add_options()
        ("allow-drift", bpo::bool_switch(&opts.drift)->default_value(false), 
             "if present, allow CLO parameters to drift")
        ("init-period", 
             bpo::value<double>(&opts.init_period)->default_value(5), 
         "initial value for period of the oscillator model")
        ("init-damping", 
             bpo::value<double>(&opts.init_damping)->default_value(0.01), 
         "initial value for damping of the oscillator model")
        ("num-oscillators", 
            bpo::value<size_t>(&opts.num_oscillators)->default_value(2), 
            "number of oscillators(people) in a coupled group")
        ("polynomial-degree", 
            bpo::value<int>(&opts.polynomial_degree)->default_value(-1), 
            "the degree of the time dependent polynomial terms")
        ("ignore-clo", 
             bpo::bool_switch(&opts.ignore_clo)->default_value(false),
             "if present, ignore the CLO parameters (assure polynomial-degree >= 0).")
        ("use-modal-params", 
             bpo::bool_switch(&opts.use_modal)->default_value(false),
             "if present, use modal parameterization");

    return options;
}

/** @brief  Create options description for Lss_set options. */
inline
bpo::options_description make_options(Lss_set_options& opts)
{
    using std::vector;
    bpo::options_description options("Parameters for a Linear state space set");
    options.add_options()
        ("shared-moderator", 
             bpo::bool_switch(&opts.shared_moderator)->default_value(false),
             "if set, the specified moderator values are shared among the "
             "partners (e.g. bmiave)")
        ("moderator", 
            bpo::value<vector<std::string> >(&opts.moderators)->multitoken(), 
            "moderators for all CLO params. ")
        ("moderator-param", 
            bpo::value<vector<std::string> >(&opts.moderator_params)->multitoken(), 
            "moderators for the specific CLO params. ")
        ("mod-coef-prior-mean", 
             bpo::value<double>(&opts.mod_coef_prior_mean)->default_value(0.0), 
             "mean of the moderator coef prior distribution")
        ("mod-coef-prior-sigma", 
             bpo::value<double>(&opts.mod_coef_prior_sigma)->default_value(100.0), 
             "sigma of the moderator coef prior distribution ");
    return options;
}

/** @brief  Create options description for Prior options. */
inline
bpo::options_description make_options(Prior_options& opts)
{
    bpo::options_description options(
            "Parameters for a Linear state space priors");
    options.add_options()
        ("lss-set-prior-fp", 
             bpo::value<std::string>(&opts.prior_fp), 
             "file contains the prior information of the parameters")
        ("gp-scale", 
             bpo::value<double>(&opts.gp_scale)->default_value(10.0), 
             "scale of gaussian process prior")
        ("clo-sigma", 
             bpo::value<double>(&opts.clo_sigma)->default_value(0.1), 
             "initial value of lss sigma")
        ("clo-sigma-shape", 
             bpo::value<double>(&opts.clo_sigma_shape)->default_value(0.0001), 
             "shape of the hyper prior of the variance of the CLO params")
        ("clo-sigma-scale", 
             bpo::value<double>(&opts.clo_sigma_scale)->default_value(0.0001), 
             "scale of the hyper prior of the variance of the CLO params")
        ("gp-scale-shape", 
             bpo::value<double>(&opts.gp_scale_shape)->default_value(0.0001), 
             "shape of the hyper prior of gp-scale")
        ("gp-scale-scale", 
             bpo::value<double>(&opts.gp_scale_scale)->default_value(0.0001), 
             "scale of the hyper prior of gp-scale")
        ("poly-sigma", 
             bpo::value<double>(&opts.poly_sigma)->default_value(10.0), 
             "initial value of the sigma of the polynomial coefficients")
        ("poly-sigma-shape", 
             bpo::value<double>(&opts.poly_sigma_shape)->default_value(0.0001), 
             "shape of the hyper prior of the variance of the polynomial coefs")
        ("poly-sigma-scale", 
             bpo::value<double>(&opts.poly_sigma_scale)->default_value(0.0001), 
             "scale of the hyper prior of the variance of the polynomial coefs")
        ("outcome-sigma", 
             bpo::value<double>(&opts.outcome_sigma)->default_value(10.0), 
             "initial value of the sigma of the outcomenomial coefficients")
        ("outcome-sigma-shape", 
             bpo::value<double>(&opts.outcome_sigma_shape)->default_value(0.0001), 
             "shape of the hyper prior of the variance of the outcomenomial coefs")
        ("outcome-sigma-scale", 
             bpo::value<double>(&opts.outcome_sigma_scale)->default_value(0.0001), 
             "scale of the hyper prior of the variance of the outcomenomial coefs")
        ("init-state-mean", 
             bpo::value<double>(&opts.init_state_mean)->default_value(0.0), 
             "mean value of the distribution of the starting state")
        ("init-state-sigma", 
             bpo::value<double>(&opts.init_state_sigma)->default_value(1000.0), 
             "sigma value of the distribution of the starting state")
        ("obs-noise-shape", 
             bpo::value<double>(&opts.obs_noise_shape)->default_value(0.0001), 
             "shape of the prior distribution of the variance in observation")
        ("obs-noise-scale", 
             bpo::value<double>(&opts.obs_noise_scale)->default_value(0.0001), 
             "scale of the prior distribution of the variance in observation")
        ("variance-prior-type", 
             bpo::value<string>(&opts.variance_prior_type)->
             default_value("inverse-gamma"), 
             "prior for the variance of the model parameters "
             " (options: inverse-gamma and  inverse-chi-squared")
        ("fixed-clo", 
             bpo::bool_switch(&opts.fixed_clo)->default_value(false),
             "if set, the parameters of the latent State space are fixed "
             " (i.e. there is no variance among individuals ")
        ("log-transformed", 
             bpo::bool_switch(&opts.sigma_log_transformed)->default_value(false),
             "if set, the variance parameters are transfored into log space");

    return options;
}

/** @brief  Create options description for Regime options. */

inline
bpo::options_description make_options(Cluster_options& opts)
{
    bpo::options_description options("Parameters for group ");
    options.add_options()
        ("group-prior-lambda", 
             bpo::value<double>(&opts.group_lambda)->default_value(100.0), 
             "param of the mixture weight prior in the group model")
        ("group-prior-kappa", 
             bpo::value<double>(&opts.group_kappa_o)->default_value(0.0001), 
             "kappa of the group prior")
        ("num-groups", 
             bpo::value<size_t>(&opts.num_groups)->default_value(1), 
             "default number of groups")
        ("infinite-clusters", 
             bpo::bool_switch(&opts.infinite_clusters)->default_value(false),
             "if set, number of clusters is infinite")
        ("collapsed-gibbs", 
             bpo::bool_switch(&opts.collapsed_gibbs)->default_value(false),
             "if set, use the collapsed gibbs for group sampling")
        ("fixed-group-params", 
             bpo::bool_switch(&opts.fixed_mixture_params)->default_value(false),
             "if set, the parameters of each group is fixed "
             "(not updated during sampling");
    return options;
}

/** @brief  Create options description for Likelihood options. */
inline
bpo::options_description make_options(Likelihood_options& opts)
{
    bpo::options_description options("Parameters for likelihood ");
    options.add_options()
    ("noise-sigma", 
         bpo::value<double>(&opts.init_noise_sigma)->default_value(0.5),
         "sigma of Gaussian noise in observations")
    ("observable", 
         bpo::value<std::vector<std::string> >(&opts.obs_names)->multitoken(), 
         "observation name (such as dial)")
    ("training-percent", 
         bpo::value<double>(&opts.training_percent)->default_value(0.8),
         "percentage of data in compute likelihood of CLO parameters");

    return options;
}

/** @brief  Create options description for Run options. */
inline
bpo::options_description make_options(Run_options& opts)
{
    bpo::options_description options("Parameters for this run");
    options.add_options()
        ("input-dir,I",
             bpo::value<string>(&opts.in_dp), "Input directory of model")
        ("iter-log",
             bpo::value<string>(&opts.iter_log_fname), "Logging file of iterations")
        ("write-samples", 
             bpo::bool_switch(&opts.write_samples)->default_value(false),
             "if set, all samples will be written to disk")
        ("write-proposals", 
             bpo::bool_switch(&opts.write_proposals)->default_value(false),
             "if set, MH proposals will be written to disk")
        ("num-hmc-iterations,N",
             bpo::value<size_t>(&opts.num_hmc_iter)->default_value(10),
             "Number of hmc iterations")
        ("shared-num-leapfrog-steps",
             bpo::value<size_t>(&opts.shared_num_leapfrog_steps)->default_value(5),
             "Number of leapfrog steps in one shared hmc iterations")
        ("person-num-leapfrog-steps",
             bpo::value<size_t>(&opts.person_num_leapfrog_steps)->default_value(5),
             "Number of leapfrog steps in one person hmc iterations")
        ("state-num-leapfrog-steps",
             bpo::value<size_t>(&opts.state_num_leapfrog_steps)->default_value(10),
             "Number of leapfrog steps in one state hmc iterations")
        ("person-grad-size", 
             bpo::value<double>(&opts.person_grad_size)->default_value(0.0001),
             "gradient step size of person specific parameter")
        ("shared-grad-size", 
             bpo::value<double>(&opts.shared_grad_size)->default_value(0.0001),
             "gradient step size of shared coefficients")
        ("state-grad-size", 
             bpo::value<double>(&opts.state_grad_size)->default_value(0.0001),
             "gradient step size of state coefficients")
        ("obs-noise-grad-size", 
             bpo::value<double>(&opts.obs_noise_grad_size)->default_value(0.0001),
             "gradient step size of observation noise")
        ("obs-coef-grad-size", 
             bpo::value<double>(&opts.obs_coef_grad_size)->default_value(0.0001),
             "gradient step size of observation coef")
        ("estimate-grad-step", 
             bpo::bool_switch(&opts.estimate_grad_step)->default_value(false),
             "if set, estimate the grad step size by current state")
        ("person-hmc-size", 
             bpo::value<double>(&opts.person_hmc_size)->default_value(0.0001),
             "HMC step size person-param-step")
        ("shared-hmc-size", 
             bpo::value<double>(&opts.shared_hmc_size)->default_value(0.0001),
             "HMC step size for shared-param-step")
        ("state-hmc-size", 
            bpo::value<double>(&opts.state_hmc_size)->default_value(0.0001),
            "HMC step size for init-state-step")
        ("obs-noise-hmc-size", 
            bpo::value<double>(&opts.noise_sigma_hmc_size)->default_value(0.0001),
            "HMC step size for obsevation noise")
        ("obs-coef-hmc-size", 
            bpo::value<double>(&opts.obs_coef_hmc_size)->default_value(0.0001),
            "HMC step size for obsevation coefficients")
        ("state-prop-sigma", 
            bpo::value<double>(&opts.state_prop_sigma)->default_value(0.02),
            "Sigma of MH proposer of the initial state")
        ("clo-param-prop-sigma", 
            bpo::value<double>(&opts.clo_param_prop_sigma)->default_value(0.02),
            "Sigma of MH proposer of the CLO parameters")
        ("poly-term-prop-sigma", 
            bpo::value<double>(&opts.poly_term_prop_sigma)->default_value(100),
            "Sigma of MH proposer of the polynomial terms")
        ("mh-bmi-coef-sigma", 
            bpo::value<double>(&opts.mh_bmi_coef_sigma)->default_value(0.1),
            "Sigma of MH proposer of the BMI coefficients")
        ("mh-var-sigma", 
            bpo::value<double>(&opts.mh_var_sigma)->default_value(0.001),
            "Sigma of MH proposer of the CLO variance")
        ("mh-obs-coef-sigma", 
            bpo::value<double>(&opts.mh_obs_coef_sigma)->default_value(0.1),
            "Sigma of MH proposer of the observation coefs")
        ("mh-noise-sigma", 
            bpo::value<double>(&opts.mh_noise_sigma)->default_value(0.1),
            "Sigma of MH proposer of the noise sigma")
        ("mh-converge-iteration", 
            bpo::value<size_t>(&opts.mh_converge_iter)->default_value(1000),
            "Sigma of MH proposer of the CLO parameters")
        ("shared-param-sampling-method", 
            bpo::value<std::string>(
                &opts.shared_sample_approach)->default_value("gibbs"),
            "Sampling method used in fitting the shared parameters.")
        ("obs-noise-sampling-method", 
            bpo::value<std::string>(
                &opts.obs_noise_sample_approach)->default_value("hmc"),
            "Sampling method used in fitting the observation noise.")
        ("obs-coef-sampling-method", 
            bpo::value<std::string>(
                &opts.obs_coef_sample_approach)->default_value("mh"),
            "Sampling method used in fitting the observation coef.")
        ("adapt-MH", 
            bpo::bool_switch(&opts.adapt_mh)->default_value(false), 
            "If set the variance of the proposal distribution of MH will "
            "be changed adaptively.")
        ("gp-params-fp", bpo::value<std::string>(&opts.gp_params_fp),
            "file that contains gp params (in log space)")
        ("opt-shared-params", 
            bpo::bool_switch(&opts.opt_shared_params)->default_value(false), 
            "If set optimize the predictive posterior distribution with "
            "respect to the shared parameters")
        ("fit-ind-clo", 
            bpo::bool_switch(&opts.fit_ind_clo)->default_value(false), 
            "If set learn the noise sigma ")
        //("learn-cluster", 
            //bpo::bool_switch(&opts.learn_cluster)->default_value(false), 
            //"If set learn the cluster model")
        ("obs-coef-fp", 
            bpo::value<std::string>(&opts.obs_coef_fp),
            "File name of the observable coefs")
        ("num-folds", 
            bpo::value<int>(&opts.num_folds)->default_value(0),
            "Number of folds in cross-validation")
        ("num-sampled-length", 
            bpo::value<size_t>(&opts.num_sampled_length)->default_value(0),
            "Number of folds in cross-validation")
        ("num-converge-iters", 
            bpo::value<size_t>(&opts.num_converge_iters)->default_value(500),
            "If the best posterior does not increase (check against a threshold) for "
            "for this number of iterations, the sampled is assumed to be converged.")
        ("maximum-running-minutes", 
            bpo::value<double>(&opts.maximum_running_minutes)->default_value(6000.0),
            "The program will terminate and write results after"
            "this maximum-running-minutes")
        ("test-sampling-method", 
            bpo::value<std::string>(
                &opts.test_sample_approach)->default_value("MH"),
            "Sample method used in fitting individual dyads params during testing.")
        ("person-step-sampling-method", 
            bpo::value<std::string>(
                &opts.person_step_approach)->default_value("MH"),
            "Sample method used in fitting individual dyads params during testing.")
        ("linear-model", 
            bpo::bool_switch(&opts.linear_model)->default_value(false), 
            "If set only fit a linear model (ignore the clo parameters")
        ("sample-cluster", 
            bpo::bool_switch(&opts.sample_cluster)->default_value(false), 
            "If set sample the cluster assignments");

    return options;
}

/** @brief  Create options description for Drift_sampler options. */
inline
bpo::options_description make_options(Drift_sampler_options& opts)
{
    bpo::options_description options("Parameters for drift sampler");
    options.add_options()
        ("num-sample-iters",
             bpo::value<size_t>(&opts.num_sample_iters)->default_value(1000), 
             "Number of iterations for running the sampler after burnin")
        ("num-burn-iters",
             bpo::value<size_t>(&opts.num_burn_iters)->default_value(100), 
             "Number of iterations for burn-in")
        ("ctr-pt-length",
             bpo::value<size_t>(&opts.ctr_pt_length)->default_value(5), 
             "Input directory of model");

    return options;
}

/** @brief  Process key-value option file. */
inline
void process_config_file
(
    const string& config_file_name,
    const bpo::options_description& config_file_options,
    bpo::variables_map& vm
)
{
    std::ifstream config_fs(config_file_name.c_str());
    IFTD(config_fs.is_open(), IO_error, "Error opening configuration file: %s.",
                                        (config_file_name.c_str()));

    bpo::store(bpo::parse_config_file(config_fs, config_file_options), vm);
}

/** @brief  Process command-line options for tracker. */
inline
Ties_experiment experiment_from_cl_options(int argc, const char** argv)
{
    Ties_experiment exp;
    bpo::variables_map vm;

    try
    {
        // Hidden options
        bpo::options_description hidden_options("Hidden options");
        hidden_options.add_options()
            ("output-dir,O",
                bpo::value<string>(&exp.out_dp)->default_value("./"),
                "Output directory");

        // General options
        bpo::options_description generic_options("General options");
        generic_options.add_options()
            ("help,h", "produce help message")
            ("config-file,C",
                bpo::value<string>(),
                "path of file containing program options in opt=val format")
            ("lss-state-dir,S",
                bpo::value<string>(&exp.lss_state_dp),
                "Directory of Linear_state_space of the initial state")
            ("random-seed", 
                bpo::value<size_t>(&exp.rand_seed)->default_value(1000), 
                "random seed")
            ("train-num-iterations", 
                bpo::value<size_t>(&exp.train_num_iterations)->default_value(8000), 
                "number of training iterations")
            ("test-num-iterations", 
                bpo::value<size_t>(&exp.test_num_iterations)->default_value(100), 
                "number of testing iterations")
            ("num-iterations", 
                bpo::value<size_t>(&exp.num_iterations)->default_value(100000), 
                "number of total fitting iterations")
            //("num-shared-posterior-threads", 
                //bpo::value<size_t>(&exp.num_shared_pos_thrds)->default_value(1), 
                //"number of threads in shared posterior computation")
            ("num-person-threads", 
                bpo::value<size_t>(&exp.num_person_thrds)->default_value(16), 
                "number of threads of sampling over couples when " 
                "fixing the shared priors")
            ("fold-threading", 
                bpo::bool_switch(&exp.fold_thrd)->default_value(false), 
                "run cross-validation K fold in parallel")
            ("turn-off-sampling-logging", 
                bpo::bool_switch(&exp.not_record)->default_value(false), 
                "if set, will not write the training sample log and best targets log")
            ("turn-off-trace-logging", 
                bpo::bool_switch(&exp.not_write_trace)->default_value(false), 
                "if set, will not write the training sample log and best targets log")
            ("sample-noise-sigma", 
                bpo::bool_switch(&exp.sample_noise_sigma)->default_value(false), 
                "if set, will sample over the noise sigmas")
            ("write-latex-error", 
                bpo::bool_switch(&exp.write_latex_error)->default_value(false), 
                "If set, write out the error into a latex formatted file.")
            ("fold", bpo::value<size_t>(&exp.fold)->default_value(0), 
                "The n'th fold of cross validation.")
            ("model", bpo::value<std::string>(&exp.model_name), 
                "The model of to be fitted for (average, line, clo-ind, clo-shared")
            ("character-model-length", 
                bpo::value<size_t>(&exp.character_model_length)->default_value(50), 
                "The length of the idealized data")
            ("fold-info-dp", bpo::value<std::string>(&exp.fold_info_dp), 
                "The directory that contains the fold information ")
            ("run-average-model", 
                bpo::bool_switch(&exp.run_average_model)->default_value(false), 
                "If set, run average model")
            ("run-line-model", 
                bpo::bool_switch(&exp.run_line_model)->default_value(false), 
                "If set, run linear model")
            ("fit-all-data", 
                bpo::bool_switch(&exp.fit_all_data)->default_value(false), 
                "If set, use all data to learn the model parameters");

        bpo::options_description data_options = make_options(exp.data);
        bpo::options_description lss_options = make_options(exp.lss);
        bpo::options_description lss_set_options = make_options(exp.lss_set);
        bpo::options_description prior_options = make_options(exp.prior);
        bpo::options_description cluster_options = make_options(exp.cluster);
        bpo::options_description likelihood_options = make_options(exp.likelihood);
        bpo::options_description run_options = make_options(exp.run);
        bpo::options_description drift_options = make_options(exp.drift_sampler);

        // Combine options
        bpo::options_description visible_options;
        visible_options.add(generic_options)
                       .add(data_options)
                       .add(lss_options)
                       .add(lss_set_options)
                       .add(prior_options)
                       .add(cluster_options)
                       .add(likelihood_options)
                       .add(drift_options)
                       .add(run_options);

        bpo::options_description cmdline_options;
        cmdline_options.add(visible_options).add(hidden_options);

        bpo::positional_options_description pstnl;
        pstnl.add("output-dir", 1);

        // process options
        bpo::store(bpo::command_line_parser(argc, argv)
                .options(cmdline_options).positional(pstnl).run(), vm);

        if(argc == 0 || vm.count("help"))
        {
            std::cout << "Usage: " << argv[0] << " " << hidden_options << "\n"
                      << visible_options << "\n"
                      << "For questions or complaints please contact "
                      << "jguan1@email.arizona.edu.\n" << std::endl;

            //kjb_c::kjb_cleanup();
            exit(EXIT_SUCCESS);
        }

        // open config file if it is given
        if(vm.count("config-file"))
        {
            string config_file_name = vm["config-file"].as<string>();
            process_config_file(config_file_name, visible_options, vm);
        }

        if(vm.count("input-dir"))
        {
            exp.run.read_model = true;
        }
        else
        {
            exp.run.read_model = false;
        }

        if(vm.count("linear-model"))
        {
            exp.lss.polynomial_degree = 1;
            exp.lss.ignore_clo = true;
        }

        // notify
        bpo::notify(vm);

        // initialize the num_params 
        exp.lss.num_params = exp.lss.ignore_clo ? 0 :
            param_length(exp.lss.num_oscillators,
                         exp.lss.use_modal); 
        if(exp.lss.polynomial_degree >= 0)
        {
            exp.lss.num_params += (exp.lss.num_oscillators * 
                                  (exp.lss.polynomial_degree + 1));
        }

        return exp;
    }
    catch(const bpo::error& err)
    {
        KJB_THROW_2(Exception, err.what());
    }    
    catch(const std::exception& ex)
    {
        throw ex;
    }    
}
}} //namespace kjb::ties
#endif // KJB_TIES_OPTIONS_H

