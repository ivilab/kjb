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
|     Ernesto Brau, Jinyan Guan
|
* =========================================================================== */

/* $Id: pt_box_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_box_likelihood.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "l_cpp/l_exception.h"

#include <iostream>
#include <string>
#include <cmath>
#include <utility>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

double Box_likelihood::operator()(const Scene& scene) const
{
    double lh = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        lh += at_target(target);
    }

    return lh;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::at_noise(const Scene& scene) const
{
    const Ascn& w = scene.association;

    Ascn::Available_data f_alarms = w.get_available_data();
    size_t fa_sz = f_alarms.size();
    double lh = 0.0;
    for(size_t t = 0; t < fa_sz; t++)
    {
        BOOST_FOREACH(const Detection_box* dbox_p, f_alarms[t])
        {
            lh += single_noise(*dbox_p);
        }
    }

    return lh;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::at_target(const Target& target) const
{
    double bl = 0.0;
    BOOST_FOREACH(const Target::value_type& pr, target)
    {
        ASSERT(target.body_trajectory()[pr.first - 1]);
        const Detection_box& dbox = *pr.second;
        const Body_2d& b2d = target.body_trajectory()[pr.first - 1]->value;

        bl += at_model(dbox, b2d);
    }

    return bl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::at_frame(const Target& target, size_t frame) const
{
    typedef Target::const_iterator T_cit;

    double l = 0.0;
    const Body_2d& b2d = target.body_trajectory()[frame - 1]->value;

    std::pair<T_cit, T_cit> pr = target.equal_range(frame);
    for(T_cit pr_p = pr.first; pr_p != pr.second; pr_p++)
    {
        const Detection_box& dbox = *pr_p->second;
        l += at_model(dbox, b2d);
    }

    return l;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::at_model
(
    const Detection_box& dbox,
    const Body_2d& model
) const
{
    const Bbox& mbox = model.full_bbox;
    double v = model.visibility.visible;
    double bl;
    if(v == 1.0)
    {
        bl = single_box(dbox, mbox);
    }
    else
    {
        bl = std::log(
                v*std::exp(single_box(dbox, mbox))
                + (1 - v)*std::exp(single_noise(dbox)));
    }

    return bl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Box_likelihood::get_params
(
    const std::string& type,
    Vector& params_x,
    Vector& params_top,
    Vector& params_bot
) const
{
    if(type == "deva_box")
    {
        if(m_dist_name == "laplace")
        {
            params_x[0] = -6.54008996e-01;
            params_x[1] = 9.52501390e+00; 
            params_x[2] = 0.0;  
            params_x[3] = 1.58625311e-02; 

            params_top[0] = -1.06048387e+01; 
            params_top[1] = 2.09605061e+01; 
            params_top[2] = 8.06451615e-03; 
            params_top[3] = -1.05040480e-02; 

            params_bot[0] = 1.58648260e+01; 
            params_bot[1] = 7.94557435e+00; 
            params_bot[2] = -7.02586043e-02; 
            params_bot[3] = 4.16786978e-02; 
        }
        else if(m_dist_name == "gauss")
        {
            params_x[0] = 2.73560040e+00; 
            params_x[1] = 1.44369189e+01; 
            params_x[2] = -1.53220536e-02; 
            params_x[3] = 1.89104221e-02; 

            params_top[0] = -2.94550585e+01;
            params_top[1] = 3.06129142e+01; 
            params_top[2] = 5.34296561e-02; 
            params_top[3] = -3.94251049e-03; 

            params_bot[0] = 1.74397099e+01; 
            params_bot[1] = 2.22385049e+01; 
            params_bot[2] = -6.95086265e-02; 
            params_bot[3] = 2.74385500e-02; 
        }
        else
        {
            KJB_THROW_3(
                Illegal_argument, 
                "Likelihood distribution %s is not available.",
                (m_dist_name.c_str()));
        }
    }
    else if(type == "person_inria" || type == "person_2x_inria" )
    {
       if(m_dist_name == "laplace")
       {
           params_x[0] = 1.77222641e-01;
           params_x[1] = 2.49931033e+00;
           params_x[2] = -1.72867248e-03;
           params_x[3] = 1.03831467e-02;

           params_top[0] = 1.02421705e+00;
           params_top[1] = 7.57142174e+00;
           params_top[2] = -1.72715870e-02;
           params_top[3] = 5.52067949e-03;

           params_bot[0] = 1.15093659e+00;
           params_bot[1] = 3.40537700e+00;
           params_bot[2] = -8.74177155e-03;
           params_bot[3] = 2.25860139e-02;
       }
       else if(m_dist_name == "gauss")
       {
           params_x[0] = 2.97438282e-01; 
           params_x[1] = 4.80713056e+00;
           params_x[2] = -1.71073777e-03;
           params_x[3] = 1.04417381e-02;

           params_top[0] = -1.35000453e+01;
           params_top[1] = 1.99967077e+01;
           params_top[2] = 2.76588757e-02; 
           params_top[3] = -7.63791865e-03;

           params_bot[0] = 7.66448014e+00; 
           params_bot[1] = 1.52839968e+01;
           params_bot[2] = -3.30944713e-02; 
           params_bot[3] = 1.26103993e-02;
       }
       else
       {
           KJB_THROW_3(
               Illegal_argument, 
               "Likelihood distribution %s is not available.",
               (m_dist_name.c_str()));
       }
    }
    else if(type == "person_inria_0.5" || type == "person_2x_inria" )
    {
        if(m_dist_name == "laplace")
        {
            params_x[0] = -1.78510206e-01; 
            params_x[1] = 1.95749040e+00; 
            params_x[2] = -2.46467049e-03; 
            params_x[3] = 8.58389440e-03; 

            params_top[0] = -5.38529783e-01; 
            params_top[1] = 6.65509619e+00; 
            params_top[2] = -7.05752489e-03; 
            params_top[3] = -7.62395197e-03;

            params_bot[0] = 2.48589580e+00;
            params_bot[1] = 2.67081458e+00; 
            params_bot[2] = -1.51238158e-02; 
            params_bot[3] = 2.00440694e-02; 
        }
        else if(m_dist_name == "gauss")
        {
            params_x[0] = -3.10202980e-01; 
            params_x[1] = 3.41201438e+00;
            params_x[2] = 0.0;
            params_x[3] = 6.56517520e-03;

            params_top[0] = -8.81753420e+00;
            params_top[1] = 1.26804051e+01; 
            params_top[2] = 3.68426754e-02;
            params_top[3] = -1.80301448e-02; 

            params_bot[0] = 5.54678390e+00;
            params_bot[1] = 9.30329982e+00; 
            params_bot[2] = -4.10090383e-02; 
            params_bot[3] = 9.17969854e-03; 
        }
        else
        {
            KJB_THROW_3(
                Illegal_argument, 
                "Likelihood distribution '%s' is not available.",
                (m_dist_name.c_str()));
        }
    }
    else if(type == "ground_truth")
    {
        if(m_dist_name == "laplace")
        {
            params_x[0] = 0; 
            params_x[1] = 2; 
            params_x[2] = 0; 
            params_x[3] = 0; 

            params_top[0] = 0; 
            params_top[1] = 2; 
            params_top[2] = 0; 
            params_top[3] = 0;

            params_bot[0] = 0;
            params_bot[1] = 2; 
            params_bot[2] = 0; 
            params_bot[3] = 0; 
        }
        else if(m_dist_name == "gauss")
        {
            params_x[0] = 0; 
            params_x[1] = 2;
            params_x[2] = 0;
            params_x[3] = 0;

            params_top[0] = 0;
            params_top[1] = 2; 
            params_top[2] = 0;
            params_top[3] = 0; 

            params_bot[0] = 0;
            params_bot[1] = 2; 
            params_bot[2] = 0; 
            params_bot[3] = 0; 
        }
        else
        {
            KJB_THROW_3(
                Illegal_argument, 
                "Likelihood distribution '%s' is not available.",
                (m_dist_name.c_str()));
        }
    }
    else
    {
        KJB_THROW_3(
            Illegal_argument, 
            "Box type '%s' is not available.",
            (type.c_str()));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::single_box
(
    const Detection_box& dbox,
    const Bbox& mbox
) const
{
    Vector params_x(4, 0.0);
    Vector params_top(4, 0.0);
    Vector params_bot(4, 0.0);
    get_params(dbox.type, params_x, params_top, params_bot);

    double dx = dbox.bbox.get_center()[0] - mbox.get_center()[0];
    double dt = dbox.bbox.get_top() - mbox.get_top();
    double db = dbox.bbox.get_bottom() - mbox.get_bottom();
    double height = mbox.get_top() - mbox.get_bottom();

    if(height <= 0.0)
    {
        std::cerr << "WARNING: person behind camera!" << std::endl;
        height = -height;
    }

    ASSERT(height > 0.0);

    return get_log_pdf(params_x, m_dist_name, height, dx)
           + get_log_pdf(params_top, m_dist_name, height, dt)
           + get_log_pdf(params_bot, m_dist_name, height, db);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Box_likelihood::get_log_pdf
(
    const Vector& params,
    const std::string& dist_name,
    double model_height,
    double value
) const
{
    ASSERT(params.size() == 4);
    double mean = params[0] + params[2] * model_height;
    double b = params[1] + params[3] * model_height;
    if(b < FLT_EPSILON)
    {
        std::cout << "WARNING: " << "scale in box likelihood is negative\n";
        return -std::numeric_limits<double>::max();
    }
    if(dist_name == "gauss")
    {
        return log_pdf(Normal_distribution(mean, b), value);
    }
    else if(dist_name == "laplace")
    {
        return log_pdf(Laplace_distribution(mean, b), value);
    }

    // should never get here
    ASSERT(false);
}

