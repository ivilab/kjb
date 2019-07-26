/* $Id$ */
/* {{{=========================================================================== *
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <sample_cpp/sample_annealing.h>
#include <sample_cpp/sample_real.h>
#include <sample_cpp/sample_recorder.h>

#include <l/l_sys_rand.h>

typedef double Model;

class dummy_annealing_proposer
{
public:
    void set_temperature(double t) {t_ = t;}
    Mh_proposal_result operator()(const Model& in, Model& out) const
    {
        out = in + (kjb_c::kjb_rand() > 0.5 ? -1 * t_ : 1 * t_);

        return Mh_proposal_result();
    }

    double t_;
};

double target(const Model&) { return 0.0; }
kjb::Vector d_target(const Model&) { return kjb::Vector(1.0); }


int main()
{
    // create annealing proposer
    dummy_annealing_proposer proposer;

    // create annealing mh step
    typedef Annealing_mh_step<Model, dummy_annealing_proposer> Step;
    Step step(target, proposer);

    // create annealing hmc step
    Real_hmc_step<Model> hmc_step(target, 1, d_target, 0.1, 0.0);
    
    // create annealing sampler
    typedef Ostream_recorder<Current_model_recorder<Model> > Recorder ;
    Annealing_sampler<Model> sampler(0.0, 0.0);

    sampler.add_recorder(Recorder(Current_model_recorder<Model>(), std::cout));

    sampler.add_step(step, 1.0, "MH Step");
//    sampler.add_step(hmc_step, 0.5, "hmc Step");

    sampler.set_temperature(2.0);

    sampler.run(5);

    sampler.set_temperature(4.0);

    sampler.run(5);

    return 0;
}
