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

/* $Id: cross_validate_util.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_HAVE_ERGO
#error "You need libergo to use this program"
#endif
#include <libgen.h>
#include <l_cpp/l_filesystem.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <boost/random.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "dbn_cpp/data.h"
#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set_sampler.h"
#include "dbn_cpp/options.h"
#include "dbn_cpp/experiment.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/cross_validate_util.h"
#include "dbn_cpp/typedefs.h"

using namespace kjb::ties;

void kjb::ties::fold_worker
(
    size_t fold,
    std::vector<Vector>& training_errors, 
    std::vector<Vector>& testing_errors, 
    std::vector<Vector>& sample_errors,
    std::vector<std::vector<Vector> >& obs_training_errors, 
    std::vector<std::vector<Vector> >& obs_testing_errors, 
    std::vector<std::vector<Vector> >& obs_sample_errors,
    const Ties_experiment& exp
)
{
    using namespace std;

    kjb_c::init_real_time();

    Ties_experiment exp_copy = exp;
    if(exp.likelihood.obs_names.empty())
    {
        KJB_THROW_2(Illegal_argument, "No observable is provided.");
    }
    double training_percent = exp_copy.likelihood.training_percent;
    string orig_in_dp;
    if(exp_copy.run.read_model)
    {
        orig_in_dp = exp_copy.run.in_dp;
        boost::format in_fmt(orig_in_dp + "/fold-%02d/training/");
        exp_copy.run.in_dp = (in_fmt % fold).str();
    }

    boost::format fold_fmt(exp_copy.fold_info_dp + "/fold-%02d/");
    boost::format out_fmt(exp_copy.out_dp + "/fold-%02d/");
    string fold_dp = (fold_fmt % fold).str();
    string out_dp = (out_fmt % fold).str();
    string training_fp = "training-list.txt";
    string testing_fp = "testing-list.txt";

    size_t num_data_groups = exp_copy.cluster.num_groups;
    std::vector<std::string> data_groups = 
                                get_data_group_dirs(exp_copy.data.data_dp);
    if(exp_copy.cluster.num_groups == 1 && exp_copy.data.grouping_var != "")
    {
        num_data_groups = data_groups.size();
        exp_copy.cluster.num_groups = num_data_groups;
    }
    string log_fp = out_dp + "/train_log.txt";
    exp_copy.run.iter_log_fname = log_fp;
    exp_copy.run.fit_err_fname = out_dp + "/train_err.txt";
    exp_copy.run.trace_fname = out_dp + "/train_trace.txt";
    exp_copy.data.id_list_fp = get_list_fps(
                                fold_dp, 
                                exp_copy.data.grouping_var, 
                                data_groups,
                                training_fp);
    // set up the output directory
    exp_copy.out_dp = out_dp + "/training/";
    ETX(kjb_c::kjb_mkdir(exp_copy.out_dp.c_str()));

    // @TODO hack!!!
    if(exp_copy.run.gp_params_fp != "")
    {
        exp_copy.run.gp_params_fp = exp_copy.out_dp + "/" + 
                                    exp_copy.run.gp_params_fp;
    }
    // Set the training percent before creating the sampler
    if(!exp_copy.run.opt_shared_params && training_errors.size() != 1 
            && exp_copy.run.obs_noise_sample_approach != "mh")
    {
        //exp_copy.likelihood.training_percent = 1.0;
        //std::cout << "setting training percent to 1.0\n";
    }
    // check option values 
    if(exp_copy.run.shared_sample_approach == "gibbs" &&
            exp_copy.prior.fixed_clo)
    {
        std::cerr << " [WARNING]: fixed-mean and gibbs sampling option "
            " can not be selected at the same time\n"
            " System is choosing the default MH sampler"
            " for the fixed-mean model\n";
        exp_copy.run.shared_sample_approach = "mh";
    }

    // verify id and data sizes match
    IFT(parse_list(exp_copy.data.id_list_fp).size() == parse_data(
             exp_copy.data.data_dp, exp_copy.data.id_list_fp,
             exp_copy.data.grouping_var).size(), Runtime_error,
                  "ids and data must have the same dimension.");
    
    Lss_set_sampler lss_set_sampler(exp_copy);
    Lss_set best_lss_set(lss_set_sampler.lss_set());
    if(exp_copy.prior.prior_fp != "")
    {
        std::cout << "Skipping training since prior_fp (" 
                  << exp_copy.prior.prior_fp << " ) is provided\n";
    }
    else
    {
        best_lss_set = lss_set_sampler.train_model(exp_copy.train_num_iterations);
    } 

    best_lss_set.write(exp_copy.out_dp);
    long st = kjb_c::get_real_time();
    std::cout << "Training takes: " << st / (1000.0 * 3600.0) << " hours.\n";
    double total_running_seconds = st / 1000.0;

    ///////////////////////////////////////////////////////////////////
    //         Compute the training error      
    ///////////////////////////////////////////////////////////////////
    kjb_c::init_real_time();
    const vector<Data>& data = lss_set_sampler.data();

    // compute errors of each observable 
    std::vector<Vector> obs_errors;
    Vector error = compute_ave_error(data, best_lss_set, obs_errors, 1.0);
    training_errors[fold-1] = error;
    obs_training_errors[fold-1] = obs_errors;
    

    const std::vector<std::string>& obs_names = exp_copy.likelihood.obs_names;
    size_t num_obs = obs_names.size();
    std::vector<std::vector<Vector> > obs_all_errors(obs_errors.size());
    for(size_t i = 0; i < obs_all_errors.size(); i++)
    {
        obs_all_errors[i] = std::vector<Vector>(1, obs_errors[i]);
    }

    string err_fp("training_errors.txt");
    report_errors(exp_copy.out_dp, err_fp, 
                  std::vector<Vector>(1, error), 
                  obs_all_errors,
                  obs_names);
    st = kjb_c::get_real_time();
    std::cout << "Reporting training error takes: " << st / (1000.0) << " s.\n";
    total_running_seconds += st/1000.0;

    // update the maximum running time for the testing
    exp_copy.run.maximum_running_minutes -= total_running_seconds / 60.0;

    ///////////////////////////////////////////////////////////////////
    //         Compute the testing error      
    ///////////////////////////////////////////////////////////////////
    // read in testing data
    exp_copy.out_dp = out_dp + "/testing/";
    exp_copy.data.id_list_fp = get_list_fps(
                                fold_dp, 
                                exp_copy.data.grouping_var, 
                                data_groups,
                                testing_fp);
    vector<size_t> testing_ids = parse_list(exp_copy.data.id_list_fp);
    if(testing_ids.empty()) return;

    kjb_c::init_real_time();
    if(exp_copy.run.read_model)
    {
        // only read in the model if no additional training
        if(exp_copy.train_num_iterations == 0)
        {
            boost::format in_fmt(orig_in_dp + "/fold-%02d/testing/");
            std::string test_dir = (in_fmt % fold).str();
            if(kjb_c::is_directory(test_dir.c_str()))
            {
                exp_copy.run.in_dp = test_dir;
            }
            else
            {
                exp_copy.run.read_model = false;
            }
            std::cout << "testing read model: " << exp_copy.run.read_model 
                      << std::endl;
        }
        else
        { 
            exp_copy.run.read_model = false;
        }
    }

    ETX(kjb_c::kjb_mkdir(exp_copy.out_dp.c_str()));
    log_fp = out_dp + "/test_log.txt";
    exp_copy.run.iter_log_fname = log_fp;
    exp_copy.run.fit_err_fname = out_dp + "/test_err.txt";

    size_t N_test = testing_ids.size();
    exp_copy.likelihood.training_percent = training_percent;
    exp_copy.num_person_thrds = N_test;

    // fit the start state and the oscillator parameteres
    Lss_set_sampler test_lss_set_sampler(exp_copy);
    Lss_set& test_lss_set = test_lss_set_sampler.lss_set();

    // Update the group-shared params based on training 
    const std::vector<Group_params>& group_params = best_lss_set.group_params();
    std::cout << "GROUP PARAMS: " << std::endl;
    for(size_t i = 0; i < group_params.size(); i++)
    {
        std::cout << group_params[i] << std::endl;
    }
    test_lss_set.set_group_params(best_lss_set.group_params());

    // Check the prediction dimensions
    // The reason that we are doing this is because in testing data, the
    // moderators for the parters might be the same, while they're different
    // in training data
    std::cout << "AFTER SETTING PRED_COEFS\n";
    test_lss_set.check_predictors_dimension();
    test_lss_set.update_means();
    test_lss_set.update_variances();
    //TODO
    //std::cout << "Writing test_lss_set to " << exp_copy.out_dp << "/init\n";
    //test_lss_set.write(std::string(exp_copy.out_dp + "/init"));

    // update the observation noise 
    const Vector& noise_sigmas = best_lss_set.get_noise_sigmas();
    test_lss_set.set_noise_sigmas(noise_sigmas);

    // get the fitted obs coef and set the test_lss_set to have these values
    const std::vector<vector<Vector> >& obs_coefs = best_lss_set.obs_coefs();
    for(size_t i = 0; i < obs_coefs.size(); i++)
    {
        for(size_t j = 0; j < obs_coefs[i].size(); j++)
        {
            test_lss_set.set_obs_coef(i, j, obs_coefs[i][j]);
        }
    }

    Lss_set test_best_lss_set(test_lss_set);
    if(exp_copy.lss.drift)
    {
        const Double_v& gp_scales = best_lss_set.gp_scales();
        const Double_v& gp_sigvars = best_lss_set.gp_sigvars();
        IFT(gp_scales.size() == gp_sigvars.size(), Runtime_error, 
                    "gp scales and gp sigvars must have the same dimension.");
        // update the variances and scales 
        test_lss_set.update_gps(gp_scales, gp_sigvars);
    }
    test_best_lss_set = test_lss_set_sampler.test_model(
                                        exp_copy.test_num_iterations);
    test_best_lss_set.write(exp_copy.out_dp);
    st = kjb_c::get_real_time();
    std::cout << "Testing takes: " << st / (1000.0 * 3600) << " hours.\n";

    ///////////////////////////////////////////////////////////////////
    //       compute the testing error from one sample
    ///////////////////////////////////////////////////////////////////
    
    // record the testing errors for each couple 
    kjb_c::init_real_time();
    const vector<Data>& test_data = test_lss_set_sampler.data();
    string test_fp("err_couples.txt");
    std::vector<Linear_state_space>& lss_set = test_best_lss_set.lss_vec();
    int num_lss = lss_set.size();
    assert(num_lss == test_data.size());

    std::vector<Vector> testing_errors_temp(num_lss);
    std::vector<std::vector<Vector> > obs_testing_errors_temp(num_obs);

    for(size_t i = 0; i < num_lss; i++)
    {
        std::vector<Vector> obs_errors;
        Vector testing_error = compute_ave_error(test_data[i], 
                                                 lss_set[i], 
                                                 obs_errors,
                                                 training_percent);
        testing_errors_temp[i] = testing_error;
        // observables
        for(size_t j = 0; j < obs_names.size(); j++)
        {
            obs_testing_errors_temp[j].push_back(obs_errors[j]);
        } 
    }

    report_errors(exp_copy.out_dp,
                  test_fp,
                  testing_errors_temp,
                  obs_testing_errors_temp, 
                  obs_names,
                  testing_ids); 

    string test_err_fp(exp_copy.out_dp + "/err_summary.txt");
    std::pair<Vector, Vector> stat = compute_and_record_errors(
                                                testing_errors_temp, 
                                                exp_copy.data.distinguisher,
                                                test_err_fp);
    testing_errors[fold-1] = stat.first;
    obs_testing_errors[fold-1].resize(num_obs);
    for(size_t i = 0; i < num_obs; i++)
    {
        string obs_fp(exp_copy.out_dp + "/" 
                        + obs_names[i] + "_err_summary.txt");
        std::pair<Vector, Vector> stat = compute_and_record_errors(
                                            obs_testing_errors_temp[i], 
                                            exp_copy.data.distinguisher,
                                            obs_fp);
        obs_testing_errors[fold-1][i] = stat.first;
    }
   
    ///////////////////////////////////////////////////////////////////
    //          compute the testing error from all samples 
    ///////////////////////////////////////////////////////////////////
    // indexed by <num_lss><num_samples>
    std::vector<std::vector<Linear_state_space> > all_samples = 
        test_best_lss_set.all_samples();

    if(!all_samples.front().empty())
    {
        string sampled_err_couple_fp("err_couples_sampled.txt");

        std::vector<Vector> sample_errors_temp(num_lss);
        std::vector<std::vector<Vector> > sp_obs_temp(num_obs);
        for(size_t i = 0; i < num_lss; i++)
        {
            std::vector<Vector> obs_errors;
            sample_errors_temp[i] = compute_ave_error(test_data[i], 
                                                      all_samples[i], 
                                                      obs_errors, 
                                                      training_percent);
            for(size_t j = 0; j < obs_errors.size(); j++)
            {
                sp_obs_temp[j].push_back(obs_errors[j]);
            }
        }

        report_errors(exp_copy.out_dp, 
                      sampled_err_couple_fp,
                      sample_errors_temp,
                      sp_obs_temp,
                      obs_names,
                      testing_ids);

        string sampled_err_fp(exp_copy.out_dp + "/err_summary_sampled.txt");
        std::pair<Vector, Vector> sample_stat = compute_and_record_errors(
                                                    sample_errors_temp, 
                                                    exp_copy.data.distinguisher,
                                                    sampled_err_fp);
        sample_errors[fold-1] = sample_stat.first;
        obs_sample_errors[fold-1].resize(num_obs);
        for(size_t i = 0; i < num_obs; i++)
        {
            string obs_fp(exp_copy.out_dp + "/" 
                            + obs_names[i] + "_err_summary_sampled.txt");
            std::pair<Vector, Vector> stat = compute_and_record_errors(
                                                sp_obs_temp[i], 
                                                exp_copy.data.distinguisher,
                                                obs_fp);
            obs_sample_errors[fold-1][i] = stat.first;
        }
    }
    // compute line fitting error for outcome
    if(!exp_copy.data.outcomes.empty())
    {
        Line line;
        double error = line_fitting_outcome(best_lss_set, line);
        std::string fp(exp_copy.out_dp + "/outcome_line.txt");
        std::ofstream ofs(fp.c_str());
        IFTD(ofs.is_open(), IO_error, "can't open file %s", (fp.c_str()));
        ofs << line.get_params() << " " << error << std::endl;
        size_t num_lss = test_best_lss_set.lss_vec().size();
        double pred_err = 0.0;
        fp = string(exp_copy.out_dp + "/outcome_line_pred.txt");
        std::ofstream pred_ofs(fp.c_str());
        IFTD(pred_ofs.is_open(), IO_error, "can't open file %s", (fp.c_str()));
        for(size_t i = 0; i < num_lss; i++)
        {
            const Linear_state_space& lss = test_best_lss_set.lss_vec()[i];
            double x = lss.get_outcome(0, 0);
            double y = line.compute_y_coordinate(x);
            double diff = y - lss.get_outcome(0, 1);
            pred_ofs << test_best_lss_set.ids()[i] << " " 
                     << diff * diff << std::endl;
        }
    }
       
    st = kjb_c::get_real_time();
    std::cout << "Reporting testing error takes: " << st / (1000.0) << " s.\n";
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::write_fold_list
(
    size_t fold,
    const std::string& training_fp, 
    const std::string& testing_fp,
    const std::vector<size_t>& ids,
    int K,
    size_t num_test, 
    size_t& last_end
)
{
    std::ofstream training_ofs(training_fp.c_str());
    std::ofstream testing_ofs(testing_fp.c_str());
    IFTD(!training_ofs.fail(), IO_error, "can't open file %s", 
            (training_fp.c_str()));
    IFTD(!testing_ofs.fail(), IO_error, "can't open file %s", 
            (testing_fp.c_str()));
    // at the last fold use the rest of the data  
    size_t cur_end = fold < K ? last_end + num_test : ids.size(); 
    for(size_t i = 0; i < last_end; i++)
    {
        training_ofs << ids[i] << std::endl;
    }
    // testing 
    for(size_t i = last_end; i < cur_end; i++)
    {
        testing_ofs << ids[i] << std::endl;
    }
    // training 
    for(size_t i = cur_end; i < ids.size(); i++)
    {
        training_ofs << ids[i] << std::endl;
    }
    last_end = cur_end;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

size_t kjb::ties::create_fold
(
    const std::string& out_dp,
    const std::string& list_fp,
    size_t num_folds,
    const std::string& group_name
)
{
    std::vector<size_t> ids = parse_list(list_fp);
    std::random_shuffle(ids.begin(), ids.end());

    size_t num_lss = ids.size();
    // create training and testing list 
    int K;
    size_t num_test;
    K = num_folds > num_lss ? num_lss : num_folds;
    if(K <= 0) 
    { 
        std::cerr << "WARINING: provided fold is not a positive number\n";
        std::cerr << "WARINING: use default number of folds (10)\n";
        // default 10 folds
        K = 10;
    }
    double temp = std::ceil(num_lss/(double)K);
    if(temp - num_lss/(double)K >= 0.5) temp = temp - 1;
    // if only one fold num_test is 0
    num_test = K == 1 ? 0 : temp;
    std::cout << "num_test: " << num_test << std::endl;

    boost::format out_fmt(out_dp + "/fold-%02d/");
    size_t last_end = 0;
    for(size_t fold = 1; fold <= K; fold++)
    {
        std::string cur_out_dp = (out_fmt % fold).str();
        ETX(kjb_c::kjb_mkdir(cur_out_dp.c_str()));
        std::string training_fp = (cur_out_dp + "/training-list.txt");
        std::string testing_fp = (cur_out_dp + "/testing-list.txt");
        if(group_name != "")
        {
            training_fp = cur_out_dp + "/" + group_name + "-training-list.txt";
            testing_fp = cur_out_dp + "/" + group_name + "-testing-list.txt";
        }

        write_fold_list(fold, 
                        training_fp, 
                        testing_fp, 
                        ids, 
                        K, 
                        num_test, 
                        last_end);
    }
    return K;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::run_cross_validate(const Ties_experiment& exp)
{
    using namespace std;
   
    // hack to make Condor respect the output
    ETX(kjb_c::kjb_mkdir(exp.out_dp.c_str()));
    std::ofstream touch((exp.out_dp + DIR_STR + "invocation.txt").c_str());
    touch << "Program invocation:\n" << "run_cross_validate" 
          << ' ' << exp.data.data_dp << ' '
          << exp.out_dp << '\n';
    Ties_experiment exp_copy = exp;

    // Check to see if the fold list is present in the current directory
    std::string out_fold_fmt(exp.fold_info_dp + "/fold-%02d/");
    std::vector<std::string> fold_dirs = dir_names_from_format(out_fold_fmt, 1);
    size_t K = fold_dirs.size();
    if(K == 0)
    {
        std::cerr << "WARNING: There is no fold-XX directories inside: \n"
                  << exp.fold_info_dp << "\n";
        return;
    }

    // Run each fold
    vector<Vector> training_errors(K);
    vector<Vector> testing_errors(K);
    vector<Vector> sample_errors(K);
    std::vector<std::vector<Vector> > obs_training_errors(K);
    std::vector<std::vector<Vector> > obs_testing_errors(K);
    std::vector<std::vector<Vector> > obs_sample_errors(K);
    size_t start_fold = exp_copy.fold > 0 ? exp.fold : 1;
    size_t end_fold = exp_copy.fold > 0 ? exp.fold : K;
    if(exp_copy.fold_thrd && start_fold != end_fold)
    {
        boost::thread_group thrds;
        for(size_t fold = start_fold; fold <= end_fold; fold++)
        {
            thrds.create_thread(boost::bind(fold_worker, 
                                            fold, 
                                            boost::ref(training_errors),
                                            boost::ref(testing_errors),
                                            boost::ref(sample_errors),
                                            boost::ref(obs_training_errors),
                                            boost::ref(obs_testing_errors),
                                            boost::ref(obs_sample_errors),
                                            exp_copy));
        }
        thrds.join_all();
    }
    else
    {
        for(size_t fold = start_fold; fold <= end_fold; fold++)
        {
            fold_worker(fold, 
                        training_errors, 
                        testing_errors, 
                        sample_errors,
                        obs_training_errors, 
                        obs_testing_errors, 
                        obs_sample_errors,
                        exp_copy);
        }
    }

    ////////////////////////////////////////////////////////
    //                  Plot the fitted model
    ////////////////////////////////////////////////////////
    boost::format out_lss_fmt(exp_copy.out_dp + "/fold-%02d/");

    for(size_t fold = start_fold; fold <= end_fold; fold++)
    {
        string fold_dp = fold_dirs[fold-1];
        string lss_dp = (out_lss_fmt % fold).str();

        std::cout << "Generate full states for plotting.\n";
        if(exp.data.grouping_var == "")
        {
            string list_fp = fold_dp + "/training-list.txt";
            string lss_dir = lss_dp + "/training/";
            plot_exp_dir(lss_dir, list_fp, exp.data.data_dp);
            list_fp = fold_dp + "/testing-list.txt";
            lss_dir = lss_dp + "/testing/";
            plot_exp_dir(lss_dir, list_fp, exp.data.data_dp);
        }
        else
        {
            boost::format train_fmt(fold_dp + "/" 
                    + exp.data.grouping_var + "-%d-training-list.txt");
            boost::format test_fmt(fold_dp + "/"
                    + exp.data.grouping_var + "-%d-testing-list.txt");
            std::vector<std::string> data_groups = 
                                            get_data_group_dirs(exp.data.data_dp);
            size_t num_groups = data_groups.size();
            for(size_t g = 0; g < num_groups; g++)
            {
                size_t found = data_groups[g].find_last_of("/\\");
                std::string group_name = data_groups[g].substr(found+1);

                std::string list_fp = fold_dp + "/" + group_name 
                                        + "-training-list.txt";
                string lss_dir = lss_dp + "/training/" + group_name + "/";
                plot_exp_dir(lss_dir, list_fp, data_groups[g]);

                list_fp = fold_dp + "/" + group_name + "-testing-list.txt";
                lss_dir = lss_dp + "/testing/" + group_name + "/";
                plot_exp_dir(lss_dir, list_fp, data_groups[g]);
            }
        }
    }
}

