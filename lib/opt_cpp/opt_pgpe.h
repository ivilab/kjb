/* $Id: opt_pgpe.h 18545 2015-02-09 18:18:16Z jguan1 $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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

#ifndef KJB_CPP_PGPE_H
#define KJB_CPP_PGPE_H

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <m_cpp/m_vector.h>

#include <fstream>
#include <string>

/**
 * @class Pgpe 
 *
 * @brief This class implements an gradient-based optimization method called 
 * Parameter-exploring Policy Gradients (Sehnke etc., 2010), is ported from an 
 * python implement in pybrain (http://pybrain.org/).
 *
 * Note: Evaluator needs to have a ()(Vector parameters) operator 
 */
namespace kjb
{
template <class Evaluator>
class Pgpe
{
public:
    //Pgpe(){};
    
    /* @brief Constructs a new pgpe */
    Pgpe
    (
        Vector  init_params,
        Vector  init_sigmas,
        const Evaluator&   eval,
        double  mean_learning_rate,
        double  sigma_learning_rate,
        size_t  max_steps,
        const std::string& log_fp = std::string("")
    ):
        parameter_means_(init_params),
        parameter_stds_(init_sigmas),
        evaluator_(eval),
        baseline_(0.0),
        w_decay_(0.0),
        mu_alpha_(mean_learning_rate),
        sigma_alpha_(sigma_learning_rate),
        max_learning_steps_(max_steps),
        baseline_initialized_(false),
        mean_updated_(false),
        log_fp_(log_fp)
    {}

    /** 
     * @brief   Run the optimizor 
     * @param   param_fp  The optimizer will write the intermediate results to
     *                    this file
     */
    void run(const std::string& param_fp = std::string(""));
    
    /* @brief   Returns the learning step */
    void learn();
    
    /* @brief   Return the best parameters found so far */
    inline Vector get_best_parameters() const
    {
        return best_parameters_;
    }
    
    /** @brief  Returns the best fitting score. */
    inline double get_best_score() const
    {
        return best_fitting_;
    }

private:

    Vector parameter_means_;
    Vector parameter_stds_;
    
    Vector best_parameters_;
    double best_fitting_;

    Evaluator evaluator_;
    
    /* brief Optimizer parameters */
    double baseline_;
    double fakt_;
    double fakt2_;
    double w_decay_;

    // Learning rate for the parameter means
    double mu_alpha_; 

    // Learning rate for parameter sigmas 
    double sigma_alpha_; 
    size_t max_learning_steps_;
    bool baseline_initialized_;
    bool mean_updated_;

    // log file 
    std::string log_fp_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Evaluator> 
void Pgpe<Evaluator>::run(const std::string& param_fp)
{
    using namespace std;
    size_t step = 0;
    best_fitting_ = evaluator_(parameter_means_);
    best_parameters_ = parameter_means_;

    std::ofstream log_ofs;
    if(log_fp_ != "")
    {
        log_ofs.open(log_fp_.c_str());
    }

    while(step < max_learning_steps_)
    {
        learn();
        if(mean_updated_)
        {
            double cur_ll = evaluator_(parameter_means_);
            if(cur_ll > best_fitting_)
            {
                best_fitting_ = cur_ll;
                best_parameters_ = parameter_means_;
                //cout << step <<"'s best fitting: "<<best_fitting_<<endl;
                //cout<<"best params: "<<best_parameters_<<endl;
            }
        }
        if(log_fp_ != "")
        {
            log_ofs << best_fitting_ << std::endl;
        }
        step++;
        if(param_fp != "")
        {
            std::ofstream param_ofs(param_fp.c_str());
            IFTD(param_ofs.is_open(), IO_error, "can't open file %s", (param_fp.c_str()));
            param_ofs << best_parameters_ << std::endl;
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Evaluator> 
void Pgpe<Evaluator>::learn()
{
    int num_params = parameter_means_.size();
   
    // First, get random deltas
    Vector deltas(num_params, 0.0);
    
    for(int i = 0; i < num_params; i++)
    {
        Gaussian_distribution dist(0, parameter_stds_(i));
        deltas(i) = sample(dist);
    }

    // Calculate goodness of parameters based on data
    Vector r = parameter_means_ + deltas;
    Vector l = parameter_means_ - deltas;

    double reward1 = evaluator_(r);
    double reward2 = evaluator_(l);

    // Check if we are amazingly good
    if(reward1 > best_fitting_)
    {
        best_fitting_ = reward1;
        best_parameters_ = parameter_means_;
    }
    else if(reward2 > best_fitting_)
    {
        best_fitting_ = reward2;
        best_parameters_ = parameter_means_;
    }
    else
    {
        double mreward = (reward1 + reward2)/ 2.0;
        // Initialize baseline
        if(!baseline_initialized_)
        {
            fakt_ = 0.0;
            fakt2_ = 0.0;
            baseline_ = mreward;
            baseline_initialized_ = true;
        }
        else
        {
            fakt_ = 0.0;
            // Calculate the gradients
            if(std::abs(reward1 - reward2) > FLT_EPSILON)
            {
                fakt_ = (reward1 - reward2) / 
                    (2.0 * best_fitting_ - reward1 - reward2);
            }
            
            double norm = (best_fitting_ - baseline_);
            fakt2_ = 0.0;
            if(std::abs(norm) > FLT_EPSILON)
            {
                fakt2_ = (mreward - baseline_)/norm;
            }

            // update baseline -- running average without exponential decay
            baseline_ = 0.9 * baseline_ + 0.1 * mreward;
            
            // update baseline -- running average w/ exponential decay
            for(int i = 0; i < parameter_means_.size(); i++)
            {
                double delta = mu_alpha_*(fakt_ * deltas[i] - 
                        parameter_means_[i]*parameter_stds_[i] * w_decay_);
                if(fabs(delta) > FLT_EPSILON)
                {
                    parameter_means_[i] += delta;
                    mean_updated_ = true;
                }
                else
                {
                    mean_updated_ = false;
                }
            }
              
            // update the sigmas, follows only positive gradients
            if(fakt2_ > 0.0)
            {
                for(int i = 0; i < parameter_stds_.size(); i++)
                {
                    double temp = std::pow(deltas[i], 2.0) - 
                                    std::pow(parameter_stds_[i], 2.0); 
                    parameter_stds_[i] += sigma_alpha_*(fakt2_ * temp / parameter_stds_[i]);
                }
            }
        }
    }
}
} // namespace kjb
#endif /*KJB_PGPE_H */
