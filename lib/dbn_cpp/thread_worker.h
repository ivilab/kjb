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

/* $Id: thread_worker.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_THREAD_WORKER_H
#define KJB_TIES_THREAD_WORKER_H

#include <diff_cpp/diff_hessian.h>
#include <m_cpp/m_matrix.h>
#include <l/l_sys_time.h>

#include <vector>
#include <string>
#include <iterator>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/proposer.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/sample_lss.h"
#include "dbn_cpp/time_util.h"

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/record.h>
#else 
#error "ergo library is not available"
#endif

#include <boost/exception_ptr.hpp>
#include <boost/progress.hpp>

namespace kjb{
namespace ties{

/**
 * @brief   A thread class of a vector of hmc-step for a vector of
 *          Linear_state_sapce 
 *
 * @param   lsss        A vector of Linear_state_space 
 * @param   adapter     Allows us to treat our model as a vector. Must have
 *                      A::get, A::set, and A::size implemented. See
 *                      kjb::Vector_adapter for an example.
 * @param   posteriors  The vector of posteriors of each model. 
 *
 * more Doc coming ...
 *
 */
template <typename Func, typename Adapter, typename Gradient>
class Hmc_step_thread
{
public:
    Hmc_step_thread
    (
        std::vector<Linear_state_space>& lsss,
        const Adapter& adapter,
        const std::vector<Func>& posteriors,
        const std::vector<double>& grad_step_sizes,
        const std::vector<double>& hmc_step_sizes,
        const std::vector<std::string>& out_dirs,
        std::vector<std::vector<Linear_state_space> >& samples_all,
        const std::string& step_name,
        bool not_record,
        bool optimize,
        size_t num_leapfrogs,
        size_t num_samples = 0,
        double maximum_running_seconds = DBL_MAX
    ) : 
        lsss_(lsss),
        adapter_(adapter),
        posteriors_(posteriors),
        grad_step_sizes_(grad_step_sizes),
        hmc_step_sizes_(hmc_step_sizes),
        out_dirs_(out_dirs),
        samples_all_(samples_all),
        step_name_(step_name), 
        not_record_(not_record),
        optimize_(optimize),
        num_leapfrogs_(num_leapfrogs),
        num_samples_(num_samples),
        num_accepted_(lsss.size(), 0),
        total_num_iters_(lsss.size(), 1),
        max_seconds_(maximum_running_seconds),
        exceed_runtime_(false)
    {};
    
    /**
     * @brief   A helper function of creating threads of computing 
     *          the hessian for a vector of models
     */
    void run
    (
        size_t start, 
        size_t end, 
        boost::exception_ptr& ep
    );

    bool exceed_runtime() const { return exceed_runtime_; }

    void decrease_running_time(double elp) { max_seconds_ -= elp; }

private:
    std::vector<Linear_state_space>& lsss_;
    const Adapter& adapter_;
    const std::vector<Func>& posteriors_;
    const std::vector<double>& grad_step_sizes_;
    std::vector<double> hmc_step_sizes_;
    const std::vector<std::string>& out_dirs_;
    std::vector<std::vector<Linear_state_space> >& samples_all_;
    const std::string& step_name_;
    bool not_record_;
    bool optimize_;
    size_t num_leapfrogs_;
    size_t num_samples_;
    std::vector<size_t> num_accepted_; 
    std::vector<size_t> total_num_iters_;
    double max_seconds_;
    bool exceed_runtime_;
};

/**
 * @brief   A thread class of a vector of mh-step for a vector of
 *          Linear_state_sapce 
 *
 * @param   lsss        A vector of Linear_state_space 
 * @param   posteriors  The vector of posteriors of each model. 
 *
 * more Doc coming ...
 *
 */
class Mh_step_thread
{
public:
    Mh_step_thread
    (
        std::vector<Linear_state_space>& lsss,
        const std::vector<Posterior>& posteriors,
        const std::vector<std::string>& out_dirs,
        double init_pro_sigma, 
        double clo_pro_sigma,
        double poly_term_sigma,
        std::vector<std::vector<Linear_state_space> >& samples_all,
        bool record_log,
        bool record_trace,
        bool optimize,
        const Cluster_prior& cluster_prior = Cluster_prior(),
        const std::vector<Group_params>& group_params = std::vector<Group_params>(),
        bool sample_cluster = false,
        bool adapt_prop_sigma = false,
        size_t num_samples = 0,
        bool sample_state = true,
        bool sample_clo = true,
        bool sample_poly_terms = false,
        size_t check_iter = 500,
        double maximum_running_seconds = DBL_MAX,
        bool record_samples = false,
        bool record_proposals = false
    ) :
        lsss_(lsss),
        posteriors_(posteriors),
        out_dirs_(out_dirs),
        init_pro_sigma_(init_pro_sigma),
        clo_pro_sigma_(clo_pro_sigma),
        poly_term_pro_sigma_(poly_term_sigma),
        samples_all_(samples_all),
        record_log_(record_log),
        record_trace_(record_trace),
        optimize_(optimize),
        sample_cluster_(sample_cluster),
        adapt_prop_sigma_(adapt_prop_sigma),
        num_samples_(num_samples),
        num_accepted_(lsss.size()),
        total_num_iters_(lsss.size(), 0),
        burned_in(lsss.size(), false),
        sample_state_(sample_state),
        sample_clo_(sample_clo),
        sample_poly_terms_(sample_poly_terms),
        check_iter_(check_iter),
        max_seconds_(maximum_running_seconds),
        exceed_runtime_(false),
        prop_sigmas_(lsss.size()),
        record_samples_(record_samples),
        record_proposals_(record_proposals),
        nth_batch_(0),
        cluster_step_(cluster_prior, group_params, true)
    {
        std::cout << "sample_cluster_ " << sample_cluster_ << std::endl;
        std::cout << "sample_clo_ " << sample_clo_ << std::endl;
        std::cout << "sample_poly_terms_ " << sample_poly_terms_ << std::endl;


        std::cout << "Mh_step_thread (lsss size="<<lsss_.size()<<")"<<std::endl;
        boost::progress_display mh_step_thread_progress_bar(lsss_.size());
        for(size_t i = 0; i < lsss_.size(); i++)
        {
            int num_params = 0;
            if(sample_poly_terms_)
            {
                int num_poly = lsss_[i].num_polynomial_coefs() ;
                Vector poly_prop_sigmas(num_poly, poly_term_pro_sigma_);
                std::copy(poly_prop_sigmas.begin(), poly_prop_sigmas.end(), 
                          std::back_inserter(prop_sigmas_[i]));
                num_params += num_poly;
            }
            if(sample_clo_)
            {
                int num_param = lsss_[i].coupled_oscillators()[0].num_params();
                Vector clo_prop_sigmas(num_param, clo_pro_sigma_);
                std::copy(clo_prop_sigmas.begin(), clo_prop_sigmas.end(), 
                          std::back_inserter(prop_sigmas_[i]));
                num_params += num_param;
            }
            if(sample_state_)
            {
                int num_init_states = lsss_[i].init_state().size();
                Vector init_prop_sigmas(num_init_states, init_pro_sigma_);
                std::copy(init_prop_sigmas.begin(), init_prop_sigmas.end(), 
                          std::back_inserter(prop_sigmas_[i]));
                num_params += num_init_states;
            }
            num_accepted_[i] = std::vector<size_t>(num_params, 0);

            // Records
            std::string proposals_dir = out_dirs_[i] + "/proposals";
            std::string samples_dir = out_dirs_[i] + "/samples";

            if(record_samples_ || num_samples_ > 0)
            {
                ETX(kjb_c::kjb_mkdir(samples_dir.c_str()));
            }
            if(record_proposals_)
            {
                ETX(kjb_c::kjb_mkdir(proposals_dir.c_str()));
            }

            sample_recorders_.push_back(
                Linear_state_space_recorder(out_dirs_[i] + "/samples/"));
            proposal_recorders_.push_back(
                Linear_state_space_recorder(out_dirs_[i] + "/proposals/"));
            ++mh_step_thread_progress_bar;
        }
    };
  
    /**
     * @brief   Run the thread for job from start to end.
     */
    void run
    (
        size_t start, 
        size_t end, 
        boost::exception_ptr& ep
    );

    bool& sample_state() { return sample_state_; }
    bool& sample_clo() { return sample_clo_; }

    bool exceed_runtime() const { return exceed_runtime_; }
    void decrease_running_time(double elp) { max_seconds_ -= elp; }
    
    /*void set_group_prior_p(const Gaussian_mixture_prior& group)
    {
        cluster_p_ = &group;
    }*/

    void clear_iter_info(size_t i) 
    {
        // clear the iter iteration
        assert(i < num_accepted_.size());
        int num_params = num_accepted_[i].size();
        assert(prop_sigmas_[i].size() == num_params);
        num_accepted_[i].clear();
        num_accepted_[i].resize(num_params, 0);
        assert(i < burned_in.size());
        burned_in[i] = false;
        assert(i < total_num_iters_.size());
        total_num_iters_[i] = 1;
        nth_batch_ = 0;

        // clear the prop sigmas
        prop_sigmas_[i].clear();
        num_params = 0;
        if(sample_poly_terms_)
        {
            int num_poly = lsss_[i].num_polynomial_coefs() ;
            Vector poly_prop_sigmas(num_poly, poly_term_pro_sigma_);
            std::copy(poly_prop_sigmas.begin(), poly_prop_sigmas.end(), 
                      std::back_inserter(prop_sigmas_[i]));
            num_params += num_poly;
        }
        if(sample_clo_)
        {
            int num_clo_params = lsss_[i].coupled_oscillators()[0].num_params();
            Vector clo_prop_sigmas(num_clo_params, clo_pro_sigma_);
            std::copy(clo_prop_sigmas.begin(), clo_prop_sigmas.end(), 
                      std::back_inserter(prop_sigmas_[i]));
            num_params += num_clo_params;
        }
        if(sample_state_)
        {
            int num_init_states = lsss_[i].init_state().size();
            Vector init_prop_sigmas(num_init_states, init_pro_sigma_);
            std::copy(init_prop_sigmas.begin(), init_prop_sigmas.end(), 
                      std::back_inserter(prop_sigmas_[i]));
            num_params += num_init_states;
        }
    }

private:
    std::vector<Linear_state_space>& lsss_;
    const std::vector<Posterior>& posteriors_;
    const std::vector<std::string>& out_dirs_;
    double init_pro_sigma_;
    double clo_pro_sigma_;
    double poly_term_pro_sigma_;
    std::vector<std::vector<Linear_state_space> >& samples_all_;
    bool record_log_;
    bool record_trace_;
    bool optimize_;
    bool sample_cluster_;
    bool adapt_prop_sigma_;
    size_t num_samples_;
    std::vector<std::vector<size_t> > num_accepted_; 
    std::vector<size_t> total_num_iters_;
    std::vector<bool> burned_in; 
    bool sample_state_;
    bool sample_clo_;
    bool sample_poly_terms_;
    size_t check_iter_;
    double max_seconds_;
    bool exceed_runtime_;
    std::vector<Vector> prop_sigmas_;
    //const Gaussian_mixture_prior* cluster_p_;
    bool record_samples_;
    bool record_proposals_;
    size_t nth_batch_;
    std::vector<Linear_state_space_recorder> sample_recorders_;
    std::vector<Linear_state_space_recorder> proposal_recorders_;
    Lss_cluster_step cluster_step_;
};

/**
 * @brief   A thread class of a vector of drift_sampler for a vector of
 *          Linear_state_sapce 
 *
 * @param   lsss        A vector of Linear_state_space 
 * @param   posteriors  The vector of posteriors of each model. 
 *
 * more Doc coming ...
 *
 */
class Drift_step_thread
{
public:
    Drift_step_thread
    (
        std::vector<Linear_state_space>& lss_vec,
        const std::vector<Posterior>& posteriors,
        const std::vector<std::string>& out_dirs,
        double init_sigma, 
        double poly_sigma,
        std::vector<std::vector<Linear_state_space> >& samples_all,
        size_t ctr_pt_length,
        size_t num_burn_its, 
        size_t num_sample_its,
        bool not_record = false,
        bool optimize = false,
        size_t num_samples = 0,
        bool sample_state = true,
        bool sample_poly_terms = false,
        size_t check_iter = 500,
        double maximum_running_seconds = DBL_MAX
    ) :
        lsss_(lss_vec),
        posteriors_(posteriors),
        out_dirs_(out_dirs),
        init_pro_sigma_(init_sigma),
        poly_term_pro_sigma_(poly_sigma),
        samples_all_(samples_all),
        ctr_pt_length_(ctr_pt_length),
        num_burn_iters_(num_burn_its),
        num_sample_iters_(num_sample_its),
        not_record_(not_record),
        optimize_(optimize),
        num_samples_(num_samples),
        sample_state_(sample_state),
        sample_poly_terms_(sample_poly_terms),
        sample_indices_(out_dirs.size(), 0),
        check_iter_(check_iter),
        max_seconds_(maximum_running_seconds),
        exceed_runtime_(false)
    {
        IFT((posteriors.size() == lss_vec.size() &&
                lss_vec.size() == out_dirs.size()), 
                Dimension_mismatch,
                "dimension mismatch in constructing a Drft_step_thread");
    }

    /**
     * @brief   Run the drift-sampler for each linear_state_space
     */
    void run(size_t start, size_t end, boost::exception_ptr& ep);

    bool& sample_state() { return sample_state_; }

    bool exceed_runtime() const { return exceed_runtime_; }
    void decrease_running_time(double elp) { max_seconds_ -= elp; }
private:
    std::vector<Linear_state_space>& lsss_;
    const std::vector<Posterior>& posteriors_;
    const std::vector<std::string>& out_dirs_;
    std::vector<std::vector<Linear_state_space> >& samples_all_;
    double init_pro_sigma_;
    double poly_term_pro_sigma_;
    size_t ctr_pt_length_;
    size_t num_burn_iters_;
    size_t num_sample_iters_;
    bool not_record_;
    bool optimize_;
    size_t num_samples_;
    bool sample_state_;
    bool sample_poly_terms_;
    mutable std::vector<size_t> sample_indices_;
    size_t check_iter_;
    double max_seconds_;
    mutable bool exceed_runtime_;
};

/**
 * @brief   Helper function of constructing hmc-step for a vector of models 
 *
 * @param   posteriors  The posteriors of each model. 
 * @param   adapter     Allows us to treat our model as a vector. Must have
 *                      A::get, A::set, and A::size implemented. See
 *                      kjb::Vector_adapter for an example.
 *
 */
template <typename Func, typename Model, typename Adapter>
void hessian_worker
(
    std::vector<Matrix>& hess,
    const std::vector<Func>& posteriors, 
    const std::vector<Model>& lss_vec,
    const std::vector<double>& lss_step_sizes,
    const Adapter& adapter,
    size_t start,
    size_t end
);

/**
 * @brief   Run the thread
 */
template <typename Thread>
void run_threads
(
    Thread& thread, 
    size_t num_threads, 
    size_t num_jobs, 
    boost::exception_ptr& ep
);

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Func, typename Adapter, typename Gradient>
void Hmc_step_thread<Func, Adapter, Gradient>::run
(
    size_t start, 
    size_t end,
    boost::exception_ptr& ep
)
{
    assert(start < lsss_.size() && end < lsss_.size());
    double allowed_seconds = max_seconds_ / (end - start + 1.0);
    long long start_time, elapsed; 
    struct timespec begin, finish;
    const double NANOS = 1e9;
    get_current_time(&begin);
    start_time = begin.tv_sec * NANOS + begin.tv_nsec;
    for(size_t i = start; i <= end; i++)
    {
        const std::string& out_dir = out_dirs_[i];
        std::string log_fname = out_dir + "/sample_log.txt";
        std::string bst_fname = out_dir + "/ll.txt";

        std::ofstream log_fs(log_fname.c_str(), std::ofstream::app);
        std::ofstream bst_fs(bst_fname.c_str(), std::ofstream::app);

        IFTD(!log_fs.fail(), IO_error, "Can't open file %s", 
                            (log_fname.c_str()));
        IFTD(!bst_fs.fail(), IO_error, "Can't open file %s", 
                            (bst_fname.c_str()));

        std::vector<double> hmc_sizes(adapter_.size(&lsss_[i]), 
                                      hmc_step_sizes_[i]);
        ergo::hmc_step<Linear_state_space> step(
                                            adapter_, 
                                            posteriors_[i], 
                                            Gradient(
                                                posteriors_[i], 
                                                adapter_, 
                                                grad_step_sizes_[i]),
                                            hmc_sizes,
                                            num_leapfrogs_,
                                            0.0);

        Linear_state_space best_lss(lsss_[i]);
        // recorders
        if(!not_record_)
        {
            step.add_recorder(make_hmc_detail_recorder(
                std::ostream_iterator<ergo::step_detail>(log_fs, "\n")));

            if(optimize_)
            {
                ergo::best_target_recorder<std::ostream_iterator<double> > 
                    best_target_recorder(
                            std::ostream_iterator<double>(bst_fs, "\n"));
                step.add_recorder(best_target_recorder);
                step.add_recorder(
                        ergo::make_best_sample_recorder<Linear_state_space*>(
                            &best_lss).replace());
            }
            else
            {
                ergo::target_recorder<std::ostream_iterator<double> > 
                    target_recorder(std::ostream_iterator<double>(bst_fs, "\n"));
                step.add_recorder(target_recorder);
            }
        }

        step.rename(step_name_);
        Linear_state_space lss = lsss_[i];

        size_t num_iters = optimize_ ?  (lss.get_times().size()) * 50 : 10;
        bool burnin = optimize_ ? false : true; 
        size_t max_num_iters = (lss.get_times().size()) * 800;
        size_t finished_iter = 0;

        double pos = posteriors_[i](lss);
        double best_pos = pos;
        double prev_best_pos = best_pos;
        const size_t iter_to_check = 10;
        const size_t burn_iter = lss.get_times().size() * 100;
        do 
        {
            for(size_t j = 0; j < num_iters; j++)
            {
                step(lss, pos);
                if(pos > best_pos)
                { 
                    best_pos = pos;
                }
                
                if((finished_iter + 1) % burn_iter == 0) 
                {
                    if(fabs(best_pos - prev_best_pos) < 1e-5 && 
                            num_accepted_[i] > 0)
                    {
                        burnin = true;
                        break;
                    }
                    prev_best_pos = best_pos;
                }
               
                // adapt hmc step size 
                adapt_hmc_step_size<Linear_state_space>(
                                                   step, 
                                                   log_fs, 
                                                   total_num_iters_[i], 
                                                   num_accepted_[i], 
                                                   iter_to_check, 
                                                   not_record_);
                log_fs << num_accepted_[i] << std::endl;
                if(!not_record_)
                {
                    log_fs.flush();
                    bst_fs.flush();
                }

                total_num_iters_[i]++;
                finished_iter++;
                get_current_time(&finish);
                elapsed = (finish.tv_sec * NANOS + finish.tv_nsec - start_time)/NANOS;
                if(elapsed > allowed_seconds)
                {
                    std::cout << "[WARNING]: Exceeds maximum running time (" 
                              << allowed_seconds << " seconds) for [" << out_dir 
                              << "]" << std::endl;
                    exceed_runtime_ = true;
                    break;
                }
            }
        } while(optimize_ && !burnin && finished_iter < max_num_iters 
                && !exceed_runtime_);

        if(!burnin)
        {
            std::cout << out_dir << " WARNING: Linear_state_space parameters have not " 
                      << "achieve the optimial value through HMC\n";
        }

        if(optimize_)
        {
            lsss_[i] = best_lss;
        }
        else
        {
            lsss_[i] = lss;
        }

        // write the results
        lsss_[i].write(out_dir);

        // generate samples and compute the coveraince matrix
        if(num_samples_ > 0)
        {
            std::vector<Linear_state_space> samples;
            if(!not_record_)
            {
                std::string out_dir = out_dirs_[i] + "/samples";
                ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
                std::string sample_target_fp = out_dir + "/ll.txt";
                std::string sample_log_fp = out_dir + "/sample_log.txt";

                typedef boost::optional<const std::string&> Opt_str;
                samples = generate_lss_samples(lss, posteriors_[i], num_samples_,
                                     Opt_str(sample_target_fp),
                                     Opt_str(sample_log_fp),
                                     not_record_);
            }
            else
            {
                samples = generate_lss_samples(lss, posteriors_[i], num_samples_,
                                     boost::none,
                                     boost::none,
                                     not_record_);
            }
            samples_all_[i] = samples;
        }
 
        // update hmc step size
        hmc_step_sizes_[i] = step.step_sizes().front();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Func, typename Model, typename Adapter>
void hessian_worker
(
    std::vector<Matrix>& hess,
    const std::vector<Func>& posteriors, 
    const std::vector<Model>& lss_vec,
    const std::vector<double>& lss_step_sizes,
    const Adapter& adapter,
    size_t start,
    size_t end
)
{
    using std::string;
    assert(lss_vec.size() == posteriors.size());
    assert(start < lss_vec.size() && end < lss_vec.size());
    for(size_t i = start; i <= end; i++)
    {
        double step_size = lss_step_sizes[i]; 
        std::vector<double> step_sizes(adapter.size(&lss_vec[i]), step_size);
        hess[i] = hessian_symmetric(posteriors[i],
                                    lss_vec[i],
                                    step_sizes,
                                    adapter);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Thread>
void run_threads
(
    Thread& thread, 
    size_t num_threads, 
    size_t num_jobs, 
    boost::exception_ptr& ep
)
{
    using namespace boost;

    double start_time, elapsed;
    struct timespec begin, finish;
    const double NANOS = 1e9;
    get_current_time(&begin);
    start_time = begin.tv_sec * NANOS + begin.tv_nsec;
    if(num_threads == 1)
    {
        thread.run(0, num_jobs - 1, ep);
    }
    else
    {
        thread_group thrds;
        size_t avail_cores = boost::thread::hardware_concurrency();
        num_threads = num_threads > avail_cores ? avail_cores : num_threads;
        num_threads = num_threads > num_jobs ? num_jobs : num_threads;
        for(size_t t = 0; t < num_threads; t++)
        {
            size_t st = num_jobs/num_threads * t;
            size_t l = (t == num_threads - 1 ? num_jobs : 
                        (num_jobs/num_threads) * (t+1));
            size_t en = l >= 1 ? l - 1 : st;
            thrds.create_thread(bind(&Thread::run, 
                                     boost::ref(thread), 
                                     st, en, 
                                     boost::ref(ep)));
        }
        // join threads
        thrds.join_all();
        // decrease time as a whole
        if(ep) boost::rethrow_exception(ep);
    }
    get_current_time(&finish);
    elapsed = (finish.tv_sec * NANOS + finish.tv_nsec - start_time)/NANOS;
    thread.decrease_running_time(elapsed);
}
}} // namespace kjb::ties

#endif // KJB_TIES_THREAD_WORKER_H
