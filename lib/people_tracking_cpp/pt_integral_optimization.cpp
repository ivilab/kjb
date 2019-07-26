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

/* $Id: pt_integral_optimization.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "people_tracking_cpp/pt_integral_optimization.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_scene_info.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_scene_diff.h"
#include "people_tracking_cpp/pt_scene_adapter.h"
#include "people_tracking_cpp/pt_scene_posterior.h"
#include "integral_cpp/integral_riemann.h"
#include "integral_cpp/integral_marginal.h"
#include "l_cpp/l_exception.h"
#include "l/l_sys_io.h"

#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cmath>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

#ifdef KJB_HAVE_ERGO
#include "ergo/mh.h"
#include "ergo/record.h"
#endif

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::mcmcda;

double Optimize_likelihood::operator()(const Scene& scene) const
{
#ifdef KJB_HAVE_ERGO

    //// MAXIMIZE likelihood
    // open files and create iterators
    std::ofstream log_fs;
    std::ofstream info_fs;
    if(record_log_ || record_samples_ || record_proposals_ || record_info_)
    {
        kjb_c::kjb_mkdir(out_dir_.c_str());
        if(record_log_) log_fs.open((out_dir_ + "/sample_log.txt").c_str());
        if(record_info_) info_fs.open((out_dir_ + "/scene_info.txt").c_str());

        IFT(log_fs, IO_error, "Could not create sample log file. Exiting!");
        IFT(info_fs, IO_error, "Could not create sample info file. Exiting!");
    }

    std::ostream_iterator<ergo::step_detail> log_out(log_fs, "\n");
    Write_scene_iterator sample_out(out_dir_ + "/samples");
    Write_scene_iterator proposal_out(out_dir_ + "/proposals");
    std::ostream_iterator<Scene_info> info_out(info_fs, "\n");

    // sample
    sample_scenes_(
            scene,
            boost::make_optional(record_log_, log_out),
            boost::make_optional(record_samples_, sample_out),
            boost::make_optional(record_proposals_, proposal_out),
            boost::make_optional(record_info_, info_out));

    //// COMPUTE OPTIMIZATION USING HESSIAN
    const Scene_posterior& posterior = sample_scenes_.scene_posterior();

    // turn off dimension prior
    bool udp = posterior.use_dim_prior();
    posterior.use_dim_prior() = false;

    // step size for hessian 
    std::vector<double> hess_step_sizes; 
    Scene_adapter adapter(posterior.vis_off());
    if(estimate_hess_step_size_)
    {
        hess_step_sizes = trajectory_gradient_step_sizes(scene, infer_head_);
    }
    else
    {
        hess_step_sizes.resize(adapter.size(&scene), hess_step_size_);
    }
    //using namespace std;
    //cout << "after optimization" << endl;
    //cout << "proposal break-down" << endl;
    //write_posterior_details(posterior, scene, 0.0, std::cout);

    // make sure we are at a maximum
    // we need a const cast here to avoid an extra copy of the scene
    // this only works if scene was NOT declared const, which should be
    // the case
    make_max(scene, posterior, hess_step_sizes, infer_head_);

    // Hessian and its determinant
    Scene_hessian hess(posterior, hess_step_sizes, hess_nthreads_);
    hess.set_diagonals(scene);
    double dl = lm_marginal_likelihood(scene, posterior(scene), infer_head_);

    // restore prior
    posterior.use_dim_prior() = udp;

    return dl;
#else
    KJB_THROW_2(Missing_dependency, "libergo");
#endif
}

