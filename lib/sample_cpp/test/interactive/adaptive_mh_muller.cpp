/* $Id: adaptive_mh_muller.cpp 17425 2014-08-30 00:34:38Z predoehl $ */
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
#include "sample_cpp/sample_adaptive_mh.h"
#include "sample_cpp/sample_recorder.h"
#include "sample_cpp/sample_sampler.h"


#include <unistd.h>

//using namespace kjb;
using namespace std;

double SIGMA = 500;

Gnuplot muller_plot;

class Point : public kjb::Vector
{
public:
    Point() : kjb::Vector(2, 0.0) {}
    Point(const kjb::Vector& other) : kjb::Vector(other) {}

    double& x()       { return (*this)[0]; }
    double  x() const { return (*this)[0]; }

    double& y()       { return (*this)[1]; }
    double  y() const { return (*this)[1]; }

};

template <class ostream>
ostream& operator<<(ostream& ost, const Point& p)
{
    ost << p.x() << ' ' << p.y() << endl;
    return ost;
}

// likelihood = exp(-U);   log_likelihood = -U
double log_likelihood(const Point& m)
{
    return - mullers_potential_d(m.x(), m.y()) / 10;
}

// uniform prior
//struct log_prior
//{
//    double operator()(const Point& m) {return 0; }
//};
double log_prior(const Point& /*m*/) {return 0; }

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
        m_interval_counter(0),
        m_interval(update_interval)
    {}


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
        ofstream out("/tmp/muller.out");
        Self& self = *this;

        for(size_t i = 0; i < size(); i++)
        {
            out << self[i].x() << " " << self[i].y() << " " << 0 << endl; 
        }

        out.close();

        muller_plot.set_terminal_std("wxt");
        muller_plot.cmd("splot '/tmp/muller.out' title \"Metropolis-hastings\" with lines lw 1, '/tmp/contour2.dat' notitle with lines lw 1 lc 1");
        muller_plot.set_terminal_std("wxt");
    }
private:
    size_t m_interval_counter;
    size_t m_interval;

    // can't copy construct
    Muller_model_plot_recorder(const  Muller_model_plot_recorder&) :
        Recent_model_recorder<Point>()
    {}
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
    Muller_log_plot_recorder r2(10000, 100);
    recorder.push_back(boost::ref(r2));

    // sleep for 500 micro-seconds after every iteration (better for demos)
    Sleep_recorder r3(500);
    recorder.push_back(boost::ref(r3));

    // Store the best model
    Best_model_recorder<Point> r4;
    recorder.push_back(r4);

    // Same as previous, but pass recorder by reference.
    Best_model_recorder<Point> r5;
    recorder.push_back(boost::ref(r5));

// SET UP MODEL AND METROPOLIS-HASTINGS STEP
    Point model;
    double model_log_likelihood = log_likelihood(model);

    Simple_adaptive_mh_step<Point> mh_step(log_likelihood, create_diagonal_matrix(kjb::Vector(2, SIGMA)), 0.22);

// BUILD SAMPLER
    Single_step_sampler<Point> sampler(
            boost::ref(mh_step),
            model,
            model_log_likelihood);

    sampler.add_recorder(recorder);

// RUN
    sampler.run(100000);

    cout << "Best position (normal version)" << sampler.get_recorder<Multi_model_recorder<Point> >(0).get<Best_model_recorder<Point> >(3) << endl;
    cout << "Best position (ref version)" << sampler.get_recorder<Multi_model_recorder<Point> >(0).get<Best_model_recorder<Point> >(4) << endl;

    return EXIT_SUCCESS;
}
