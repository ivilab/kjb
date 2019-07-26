/* $Id$ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#include <m/m_incl.h>
#include <l/l_incl.h>
#include <sample/sample_gauss.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <deque>

#include <boost/concept_check.hpp>
#include <boost/ref.hpp>
#include "gnuplot_i.hpp" /*Gnuplot class handles POSIX-Pipe-communication with Gnuplot*/
#include "sample_cpp/sample_concept.h"
#include "potential.h"
#include "sample_cpp/sample_step.h"
#include "sample_cpp/sample_recorder.h"
#include "sample_cpp/sample_sampler.h"
#include "sample2_cpp/sample2_recorder.h"


#include <unistd.h>

using namespace kjb;
using namespace std;

double SIGMA = .05;

Gnuplot muller_plot;

// model
struct Point
{
    double x;
    double y;
};

double ANNEAL_T = 25;


double point_dimension(const Point& pt) { return 2; }
void move_point(Point& pt, size_t i, double delta)
{
    if(i == 0) pt.x += delta;
    else if(i == 1) pt.y += delta;
    else abort();
}

kjb::Vector point_step_size(const Point& pt)
{
    return Vector(2, 0.001);
}

kjb::Vector gradient(const Point& pt)
{
    Vector grad(2);
    grad_mullers_potential_d(&grad[0], &grad[1], pt.x, pt.y);
    return -grad/ANNEAL_T;
}

template <class ostream>
ostream& operator<<(ostream& ost, const Point& p)
{
    ost << p.x << ' ' << p.y << endl;
    return ost;
}

// likelihood = exp(-U);   log_likelihood = -U
double log_likelihood(const Point& m)
{
    return - mullers_potential_d(m.x, m.y)/ANNEAL_T;
}

// uniform prior
//struct log_prior
//{
//    double operator()(const Point& m) {return 0; }
//};
double log_prior(const Point& /*m*/) {return 0; }

Mh_proposal_result proposal(const Point& m_in, Point& m_out)
{
    Mh_proposal_result result;
    double delta = SIGMA * kjb_c::gauss_rand();
    double pdf   = 0;
    kjb_c::gaussian_log_pdf(&pdf, delta, 0, SIGMA);

    result.fwd_prob = pdf;

    m_out = m_in;
    m_out.x += delta;

    delta = SIGMA * kjb_c::gauss_rand();
    kjb_c::gaussian_log_pdf(&pdf, delta, 0, SIGMA);
    
    m_out.y += delta;

    result.fwd_prob += pdf;
    result.rev_prob = result.fwd_prob;

    return result;
}

typedef Basic_mh_step<Point> Muller_mh_step;

class Muller_log_plot_recorder : public Recent_log_recorder<Point>
{
public:
    typedef Muller_log_plot_recorder Self;
    typedef Recent_log_recorder<Point> Parent;

    Muller_log_plot_recorder(size_t n, size_t update_interval) :
        Parent(n),
        m_interval_counter(0),
        m_interval(update_interval),
        m_plot("lines")
    {
        m_plot.set_terminal_std("wxt");
    }


    void operator()(const Point& cur_pt, const Step_log<Point>& log)
    {
        Parent::operator()(cur_pt, log);

        m_interval_counter++;

        if(m_interval_counter % m_interval == 0)
        {
            update_plot();
            m_interval_counter = 0;
        }
    }

private:
    void update_plot()
    {
        m_plot.remove_tmpfiles();
        m_plot.reset_plot();
        
        vector<double> posteriors;
        Self& self = *this;

        for(size_t i = 0; i < size(); i++)
        {
            posteriors.push_back(-self[i].lt);
        }

        m_plot.plot_x(posteriors, "log-posterior");
    }
private:
    size_t m_n;
    size_t m_interval_counter;
    size_t m_interval;
    Gnuplot m_plot;

};


class Muller_model_plot_recorder : public Recent_model_recorder<Point>
{
public:
    typedef Muller_model_plot_recorder Self;
    typedef Recent_model_recorder<Point> Parent;

    Muller_model_plot_recorder(size_t n, size_t update_interval) :
        Parent(n),
        height_(),
        N_(n),
        m_interval_counter(0),
        m_interval(update_interval)
    {}


    void operator()(const Point& cur_pt, const Step_log<Point>& log)
    {
        Parent::operator()(cur_pt, log);

        height_.push_back(log[0].lt);

        if(height_.size() > N_) height_.pop_front();

        m_interval_counter++;

        if(m_interval_counter % m_interval == 0)
        {
            update_plot();
            m_interval_counter = 0;
        }
    }

private:
    void update_plot()
    {
        ofstream out("/tmp/muller.out");
        Self& self = *this;

        for(size_t i = 0; i < size(); i++)
        {
            out << self[i].x << " " << self[i].y << " " <<  -height_[i]*ANNEAL_T << endl; 
        }

        out.close();

        muller_plot.set_terminal_std("wxt");
        muller_plot.cmd("splot '/tmp/muller.out' title \"Metropolis-hastings\" with lines lw 2 lc rgb \"blue\", '/tmp/contour2.dat' notitle with lines lw 1 lc 1");
        muller_plot.set_terminal_std("wxt");
    }
private:
    std::deque<double> height_;
    size_t N_;
    size_t m_interval_counter;
    size_t m_interval;

    // can't copy construct
    Muller_model_plot_recorder(const  Muller_model_plot_recorder&) :
        Recent_model_recorder<Point>()
    {}
};

class Reject_recorder : public Null_recorder<Point>
{
public:
    Reject_recorder() :
        counter_(0)
    {}


    void operator()(const Point& cur_pt, const Step_log<Point>& log)
    {
        if(log[0].accept)
            counter_++;
        else
        {
            cout << counter_ << endl;
            counter_ = 0;
        }
    }

private:
    size_t counter_;
};


class Sleep_recorder
{
    typedef int Generic_model_type; 
public:
    typedef Generic_model_type Model_type;
    typedef Generic_model_type Value_type;

    Sleep_recorder(size_t usec) :
        usec_(usec)
    { }

    template <class Model>
    void operator()(const Model& /* m */, const Step_log<Model>& /* l */)
    {
        usleep(usec_);
    }

    Value_type get() const {return Value_type(); }
protected:
    size_t usec_;
};

int main (int /* argc */, char ** /*argv */)
{
    using boost::ref;

    /* Gnuplot g1("lines");
    Gnuplot g2("lines");

    g1.set_terminal_std("wxt");
    g2.set_terminal_std("wxt"); */

    kjb_c::kjb_init();

    muller_plot.cmd("load \"muller-V.gnu\"");

    BOOST_CONCEPT_ASSERT((BaseModel<Point>));


// SET UP RECORDING
    Multi_model_recorder<Point> recorder;

    // plot the accepted points
    Muller_model_plot_recorder r1(200, 100);
    recorder.push_back(boost::ref(r1));

    // plot the likelihood
    Target_plot_recorder<Point> r2(640,480);
    r2.set_culling(10000,8000);
//    Muller_log_plot_recorder r2(10000, 100);
    recorder.push_back(boost::ref(r2));

    // sleep for 500 micro-seconds after every iteration (better for demos)
    Sleep_recorder r3(100);
    recorder.push_back(boost::ref(r3));

    // Store the best model
    Best_model_recorder<Point> r4;
    recorder.push_back(r4);

    // Same as previous, but pass recorder by reference.
    Best_model_recorder<Point> r5;
    recorder.push_back(boost::ref(r5));

    recorder.push_back(Reject_recorder());

// SET UP MODEL AND METROPOLIS-HASTINGS STEP
    Point model = {0.,0.};
    double model_log_likelihood = log_likelihood(model);

    Muller_mh_step mh_step(log_likelihood, proposal);
   Basic_hmc_step<Point> sd_step(log_likelihood, 1, gradient, move_point, point_step_size, point_dimension, 0.9999);

// BUILD SAMPLER
//    Single_step_sampler<Point, Multi_model_recorder<Point> > sd_sampler(
//            mh_step,
//            model,
//            model_log_likelihood,
//            recorder);
    typedef Single_step_sampler<Point> Sampler;
    Sampler sd_sampler(
            sd_step,
            model,
            model_log_likelihood);

    sd_sampler.add_recorder(recorder);
// RUN
    // (run inside glut idle loop; allows glut-based plot to work)
    kjb::opengl::Glut::set_idle_callback(boost::bind(&Sampler::run, &sd_sampler, 1));
//    sd_sampler.run(100000);
    glutMainLoop();

    cout << "Best position (normal version)" << sd_sampler.get_recorder<Multi_model_recorder<Point> >(0).get<Best_model_recorder<Point> >(3) << endl;
    cout << "Best position (ref version)" << sd_sampler.get_recorder<Multi_model_recorder<Point> >(0).get<Best_model_recorder<Point> >(4) << endl;

    return EXIT_SUCCESS;
}
