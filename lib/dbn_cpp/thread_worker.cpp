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

/* $Id: thread_worker.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

#include <string>
#include <vector>

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/record.h>
#else 
#error "ergo library is not available"
#endif

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/thread_worker.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/drift_sampler.h"
#include "dbn_cpp/proposer.h"

#ifdef KJB_HAVE_BOOST
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#else 
#error "Boost library is not available"
#endif

using namespace kjb;
using namespace kjb::ties;
 

void Mh_step_thread::run
(
    size_t start, 
    size_t end, 
    boost::exception_ptr& ep
)
{
    assert(start < lsss_.size() && end < lsss_.size());
    long start_time, elapsed;
    struct timespec begin, finish;
    const double NANOS = 1e9;
    get_current_time(&begin);
    start_time = begin.tv_sec * NANOS + begin.tv_nsec;
    double allowed_seconds = max_seconds_ / ((int)end - (int)start + 1.0);
    if(allowed_seconds < 0) 
    {
        std::cout << "NO EXTRA TIME \n";
        exceed_runtime_ = true;
        return;
    }
    for(size_t i = start; i <= end; i++)
    {
        const std::string& out_dir = out_dirs_[i];
        /*
         * Changes on 6 Aug 2015, predoehl
         * Trying to make the code less bursty with respect to disk I/O.
         * Idea: do not open output files that we won't write to.
         */
        // same name, now pointers
        boost::scoped_ptr<std::ofstream> log_fs, bst_fs, trace_fs, accept_fs; 
        typedef std::ostream_iterator<ergo::step_detail> sd_it;
        boost::scoped_ptr<ergo::default_detail_recorder<sd_it> > gibbs_rec_pt;
        if(record_log_)
        {
            std::string log_fname = out_dir + "/sample_log.txt",
                        bst_fname = out_dir + "/ll.txt";

            log_fs.reset(new std::ofstream(log_fname.c_str(), std::ofstream::app));
            bst_fs.reset(new std::ofstream(bst_fname.c_str(), std::ofstream::app));
            gibbs_rec_pt.reset(new ergo::default_detail_recorder<sd_it>(
                        "gibbs", sd_it(*log_fs, "\n")));


            if(log_fs->fail())
            {
                KJB_THROW_2(IO_error, "Can't open file " + log_fname);
            }
            if(bst_fs->fail()) 
            {
                KJB_THROW_2(IO_error, "Can't open file " + bst_fname);
            }
        }
        if(record_trace_)
        {
            std::string trace_fname = out_dir + "/trace.txt",
                        accept_fname = out_dir + "/acceptance.txt";
            trace_fs.reset(new std::ofstream(trace_fname.c_str(), std::ofstream::app));
            accept_fs.reset(new std::ofstream(accept_fname.c_str(), std::ofstream::app));

            if(trace_fs->fail()) 
            {
                KJB_THROW_2(IO_error, "Can't open file " + trace_fname);
            }
            if(accept_fs->fail()) 
            {
                KJB_THROW_2(IO_error, "Can't open file " + accept_fname);
            }
        }
        
        // lss param propose
        Lss_mh_proposer proposer(prop_sigmas_[i], 
                                 sample_state_,
                                 sample_clo_,
                                 sample_poly_terms_);

        assert(num_accepted_[i].size() == prop_sigmas_[i].size());
        ergo::mh_step<Linear_state_space> step(posteriors_[i], 
                                               boost::ref(proposer));

        Linear_state_space best_lss(lsss_[i]);
        // recorders
        if(record_log_)
        {
            step.add_recorder(make_mh_detail_recorder(
                std::ostream_iterator<ergo::step_detail>(*log_fs, "\n")));
        }

        if(record_proposals_)
        { 
            step.store_proposed(true);
        }

        const double OPTIMAL_RATE = 0.44;
        size_t D = prop_sigmas_[i].size();
        std::vector<size_t> param_indices(D);
        std::vector<double> acceptance(D, 0.0);
        std::generate(param_indices.begin(), param_indices.end(), 
                      Increment<size_t>(0));
        std::random_shuffle(param_indices.begin(), param_indices.end());
        size_t time_length = lsss_[i].get_times().size();
        size_t max_num_iters = optimize_ ?  time_length * 1e4 : 1;
        //size_t max_num_iters = 2;
        const size_t batch_size = 10;
        size_t& cur_iter = total_num_iters_[i];
        size_t end_iter = cur_iter + max_num_iters;
        size_t burn_iter = 1e4; 
        double pos = posteriors_[i](lsss_[i]);


        // parameter for the cluster prior
        double old_cluster_prior = 0.0;
        size_t old_cluster = 0;
        // add in the weights prior 
        double best_pos = pos; // + old_cluster_prior;
        double prev_best_pos = best_pos;

        long long start_propose; 
        get_current_time(&begin);
        start_propose = begin.tv_sec * NANOS + begin.tv_nsec;
        while(cur_iter < end_iter && !burned_in[i])
        {
            // sample over the cluster
            if(sample_cluster_)// && sample(Uniform_distribution()) < 0.1)
            {
                cluster_step_(lsss_[i], pos);
                if(record_log_)
                {
                    (*gibbs_rec_pt)(cluster_step_, lsss_[i], pos);
                }

                //logging
                if(pos >= best_pos)
                {
                    best_pos = pos;
                    best_lss = lsss_[i];
                }
            }
            // Update each dimension
            for(size_t s = 0; s < D; s++)
            {
                size_t param_index = s;
                proposer.sample_index() = param_index;
                step(lsss_[i], pos);
              
                //logging
                if(pos + old_cluster_prior > best_pos)
                {
                    best_pos = pos + old_cluster_prior;
                    best_lss = lsss_[i];
                }

                if(step.accepted())
                {
                    assert(param_index < num_accepted_[i].size());
                    num_accepted_[i][param_index]++;
                }
                // recording 
                if(record_log_)
                {
                    if(!optimize_)
                    {
                        *bst_fs << pos << "\n";
                    }
                    else
                    {
                        *bst_fs << best_pos << "\n";
                    }
                    log_fs -> flush();
                    bst_fs -> flush();
                }

                if(cur_iter % 1000 == 0 && param_index == D - 1)
                {
                    if(record_samples_)
                    {
                        sample_recorders_[i](lsss_[i]);
                    }
                    if(record_proposals_)
                    {
                        proposal_recorders_[i](*step.proposed_model());
                    }
                }
                const double min_sigma = 1e-5;
                const double max_sigma = 100.0;
                // adapt the proposal sigmas
                if(cur_iter % batch_size == 0)
                {
                    acceptance[param_index] = 1.0 * num_accepted_[i][param_index] / batch_size;
                    num_accepted_[i][param_index] = 0;
                    double old_sigma = prop_sigmas_[i][param_index];
                    // to make sure the proposal sigma to be in a certain range
                    if(adapt_prop_sigma_ && nth_batch_ >= 10 
                            && old_sigma > min_sigma && old_sigma < max_sigma)
                    {
                        double delta = std::min(0.01, 1.0/std::sqrt(nth_batch_));
                        double old_sigma = proposer.prop_sigmas()[param_index];
                        double log_old_sigma = std::log(old_sigma);
                        double new_sigma;
                        if(acceptance[param_index] > OPTIMAL_RATE)
                        {
                            // set both the proposer sigma and the
                            // class member propose sigma 
                            new_sigma = std::exp(log_old_sigma + delta);
                            prop_sigmas_[i][param_index] = new_sigma;
                            proposer.prop_sigmas()[param_index] = new_sigma;
                        }
                        else
                        {
                            new_sigma = std::exp(log_old_sigma - delta);
                            prop_sigmas_[i][param_index] = new_sigma;
                            proposer.prop_sigmas()[param_index] = new_sigma;
                        }
                    }
                }
            }
            if(cur_iter % batch_size == 0) 
            {
                nth_batch_++;
            }

            if(record_trace_)
            {
                // record the acceptance rate and prop sigmas
                if(cur_iter % batch_size == 0)
                {
                    for(size_t s = 0; s < D; s++)
                    {
                        size_t param_index = s;
                        *accept_fs //<< param_index << ": " 
                                   << std::setw(5) << std::setprecision(3) 
                                   << acceptance[param_index] << " " 
                                   << std::setw(5) << std::setprecision(3) 
                                   << proposer.prop_sigmas()[param_index] << "; ";
                    }
                    *accept_fs << "\n";
                    accept_fs->flush();
                }

                // record the current sample values 
                if(cur_iter % 100 == 0)
                {
                    *trace_fs << cur_iter << " " 
                              << lsss_[i].coupled_oscillators()[0] 
                              << lsss_[i].init_state() << " ";
                    if(sample_poly_terms_ && !lsss_[i].polynomial_coefs().empty())
                    {
                        std::copy(lsss_[i].polynomial_coefs().begin(),
                                  lsss_[i].polynomial_coefs().end(),
                                  std::ostream_iterator<Vector>(*trace_fs, " "));
                    }
                    *trace_fs << "\n";
                    trace_fs->flush();
                }
            }
            

            /////////////////////////////////////////////////////////////
            //  Check convergence 
            /////////////////////////////////////////////////////////////
            if(optimize_ && cur_iter > burn_iter && cur_iter % check_iter_ == 0) 
            {
                if(fabs(best_pos - prev_best_pos) < 1e-1)
                {
                    burned_in[i] = true;
                    break;
                }
                else
                {
                    if(best_pos > prev_best_pos)
                    {
                        prev_best_pos = best_pos;
                    }
                }
            }

            get_current_time(&finish);
            elapsed = (finish.tv_sec * NANOS + finish.tv_nsec - start_time)/NANOS;
            if(elapsed > allowed_seconds)
            {
                if(allowed_seconds < 0) std::cout << "SURPISE\n";
                std::cout << "[WARNING]: Exceeds maximum running time (" 
                          << allowed_seconds << " seconds) for [" << out_dir 
                          << "]" << std::endl;
                exceed_runtime_ = true;
                break;
            }
            cur_iter++;
        } 

        get_current_time(&finish);
        elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start_propose;

        // Generate samples
        if(num_samples_ > 0)
        {
            Linear_state_space lss(lsss_[i]);
            std::vector<Linear_state_space> samples(num_samples_);
            for(size_t s = 0; s < num_samples_; s++)
            {
                step(lss, pos);
                if(sample_cluster_)
                {
                    cluster_step_(lss, pos);
                    if(record_log_)
                    {
                        (*gibbs_rec_pt)(cluster_step_, lss, pos);
                    }
                }
                samples[s] = lss;

            }
            // record all the samples
            std::copy(samples.begin(), samples.end(), 
                        std::back_inserter(samples_all_[i]));
        }

        if(!burned_in[i] && optimize_)
        {
            std::cout << "[WARNING]: Individual dyad parameters for [" << out_dir
                      << "] have not achieve the optimial value through MH\n";
        }

        if(optimize_)
        {
            lsss_[i] = best_lss;
            clear_iter_info(i);
        }

        if(record_log_)
        {
            log_fs->flush();
            bst_fs->flush();
        }

        if(record_trace_)
        {
            trace_fs->flush();
            accept_fs->flush();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Drift_step_thread::run
(
    size_t start, 
    size_t end, 
    boost::exception_ptr& ep
)
{
    IFT(start < posteriors_.size(), Illegal_argument, "Out of index");
    IFT(end < posteriors_.size(), Illegal_argument, "Out of index");
    typedef boost::optional<const std::string&> Opt_str;

    //long long start_time, elapsed; 
    double start_time, elapsed; 
    struct timespec begin, finish;
    const double NANOS = 1e9;
    get_current_time(&begin);
    start_time = begin.tv_sec * NANOS + begin.tv_nsec;
    double allowed_seconds = max_seconds_ / ((int)end - (int)start + 1.0);
    if(allowed_seconds < 0) 
    {
        std::cout << "NO EXTRA TIME \n";
        exceed_runtime_ = true;
        return;
    }
    for(size_t i = start; i <= end; i++)
    {
        boost::scoped_ptr<std::ofstream> bst_pos_ofs, log_fs, bst_fs;
        try
        {
            std::string log_fname(out_dirs_[i] + "/sample_log.txt");
            std::string bst_fname(out_dirs_[i] + "/ll.txt");
            std::string bst_pos_fname(out_dirs_[i] + "/pos.txt");
            Opt_str opt_log_fp;
            Opt_str opt_ll_fp;
            if(!not_record_)
            {
                opt_log_fp = Opt_str(log_fname);
                opt_ll_fp = Opt_str(bst_fname);

                log_fs.reset(new std::ofstream(log_fname.c_str(), 
                                std::ostream::app));
                bst_fs.reset(new std::ofstream(bst_fname.c_str(), 
                                std::ostream::app));
                bst_pos_ofs.reset(new std::ofstream(bst_pos_fname.c_str(), 
                                    std::ofstream::app));
                IFTD(!bst_pos_ofs->fail(), IO_error, "can't open file %s", 
                                            (bst_pos_fname.c_str()));
                IFTD(!log_fs->fail(), IO_error, "can't open file %s", 
                                            (log_fname.c_str()));
                IFTD(!bst_fs->fail(), IO_error, "can't open file %s", 
                                            (bst_fname.c_str()));

            }
            else
            {
                opt_log_fp = boost::none;
                opt_ll_fp = boost::none;
            }

            // set the drift prior to be false
            posteriors_[i].use_clo_drift_prior(false);

            Drift_sampler sampler(posteriors_[i], 
                                  lsss_[i], 
                                  ctr_pt_length_,
                                  num_burn_iters_, 
                                  opt_log_fp,
                                  opt_ll_fp);

            Init_state_proposer proposer(init_pro_sigma_);
            ergo::mh_step<Linear_state_space> init_step(
                    boost::ref(posteriors_[i]), proposer);

            Lss_polynomial_mh_proposer poly_proposer(poly_term_pro_sigma_);
            ergo::mh_step<Linear_state_space> poly_step(
                    boost::ref(posteriors_[i]), poly_proposer);

            if(!not_record_)
            {
                // best posterior recorder
                ergo::target_recorder<std::ostream_iterator<double> > 
                    target_recorder(std::ostream_iterator<double>(*bst_fs, "\n"));

                // add in log recorder
                init_step.add_recorder(make_mh_detail_recorder(
                    std::ostream_iterator<ergo::step_detail>(*log_fs, "\n")));
                // add in best posterior recorder
                init_step.add_recorder(target_recorder);
            }

            double init_lp = posteriors_[i](lsss_[i]);
            Linear_state_space best_lss(lsss_[i]);
            double best_lp = init_lp;
            double cur_lp = init_lp;
            double prev_best_lp = best_lp;
            size_t num_params = param_length(lsss_[i].num_oscillators());
            size_t num_control_pts = sampler.num_control_pts();
            int sample_iter = num_params * num_control_pts * 
                              std::ceil(lsss_[i].get_times().size() * 0.3);
            size_t num_iters = num_sample_iters_ * sample_iter;
            if(!optimize_) num_iters = 1;
            size_t drift_iters = num_params;  
            if(optimize_) drift_iters = num_params * num_control_pts;

            for (size_t j = 0; j < num_iters; j++)
            {
                if(sample_state_)
                {
                    Linear_state_space cur_lss = sampler.current();
                    for(size_t k = 0; k < cur_lss.init_state().size(); k++)
                    {
                        init_step(cur_lss, cur_lp);
                    }
                    if(cur_lp > best_lp)
                    {
                        best_lss = cur_lss;
                        best_lp = cur_lp;
                    }
                    if(!not_record_)
                    {
                        *bst_pos_ofs << best_lp << '\n';
                        log_fs->flush();
                        bst_fs->flush();
                    }
                    sampler.current() = cur_lss;
                }
                // sample the polynomials)
                if(sample_poly_terms_ && lsss_[i].num_polynomial_coefs() > 0)
                {
                    Linear_state_space cur_lss = sampler.current();
                    for(size_t k = 0; k < cur_lss.num_polynomial_coefs(); k++)
                    {
                        poly_step(cur_lss, cur_lp);
                    }
                    if(cur_lp > best_lp)
                    {
                        best_lss = cur_lss;
                        best_lp = cur_lp;
                    }
                    if(!not_record_)
                    {
                        *bst_pos_ofs << best_lp << '\n';
                        log_fs->flush();
                        bst_fs->flush();
                    }
                    sampler.current() = cur_lss;
                }

                sampler.set_sample_index(sample_indices_[i]);
                for(size_t k = 0; k < drift_iters; k++)
                {
                    try
                    {
                        sampler.sample();
                    }
                    catch(Exception& err)
                    {
                        std::cerr << "Exception in CLO sampler\n";
                        std::cerr << err.what();
                    }
                }
                cur_lp = posteriors_[i](sampler.best());
                if(cur_lp > best_lp)
                {
                    best_lp = cur_lp;
                    best_lss = sampler.best();
                }
                if(!not_record_)
                {
                    *bst_pos_ofs << best_lp << '\n';
                    log_fs->flush();
                    bst_fs->flush();
                }
                // set sample indices
                sample_indices_[i] = sampler.get_sample_index();
                if((j+1) % check_iter_ == 0)
                {
                    std::cout << "check iter: " << j << " " <<  prev_best_lp << 
                        " vs " << best_lp << std::endl;
                    if(fabs(prev_best_lp - best_lp) < 1e-1) 
                    {
                        break;
                    }
                    else
                    {
                        prev_best_lp = best_lp;
                    }
                    if(optimize_)
                    {
                        best_lss.write(out_dirs_[i]);
                    }
                }

                // check to see if it's within time
                get_current_time(&finish);
                elapsed = (finish.tv_sec * NANOS + finish.tv_nsec - start_time)/NANOS;
                if(elapsed > allowed_seconds)
                {
                    std::cout << "[WARNING]: Exceeds maximum running time (" 
                              << allowed_seconds << " seconds) for [" 
                              << out_dirs_[i] << "]" << std::endl;
                    exceed_runtime_ = true;
                    break;
                }

                if(!not_record_)
                {
                    bst_pos_ofs->flush();;
                }
            }
            if(optimize_)
            {
                lsss_[i] = best_lss;
            }
            else
            {
                lsss_[i] = sampler.current();
            }
            // generate samples
            if(num_samples_ > 0)
            {
                assert(optimize_);
                std::vector<Linear_state_space> samples(num_samples_);
                for(size_t j = 0; j < num_samples_; j++)
                {
                    if(sample_state_)
                    {
                        init_step(lsss_[i], cur_lp);
                    }
                    if(sample_poly_terms_ && lsss_[i].num_polynomial_coefs() > 0)
                    {
                        poly_step(lsss_[i], cur_lp);
                    }
                    sampler.current() = lsss_[i];
                    sampler.set_sample_index(sample_indices_[i]);
                    sampler.sample();
                    sample_indices_[i] = sampler.get_sample_index();
                    samples[j] = sampler.current();
                    if(!not_record_)
                    {
                        log_fs->flush();
                        bst_fs->flush();
                    }
                }
                samples_all_[i] = samples;
                if(samples_all_[i].size() == 0)
                {
                    std::cout << "thread_worker:\n" << i << " " << out_dirs_[i]
                              << "\nnum_samples_: " << num_samples_ << '\n';
                }
            }
            ep = boost::exception_ptr();
            if(!not_record_)
            {
                log_fs->close();
                bst_fs->close();
            }

            // set the drift prior to be false
            posteriors_[i].use_clo_drift_prior(true);
        } 
        catch(kjb::KJB_error& err)
        {
            ep = boost::current_exception();
        }
    }
}

