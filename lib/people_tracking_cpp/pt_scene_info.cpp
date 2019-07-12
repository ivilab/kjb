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

/* $Id$ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_scene_info.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_scene_diff.h"
#include "people_tracking_cpp/pt_scene_posterior.h"
#include "people_tracking_cpp/pt_position_prior.h"
#include "people_tracking_cpp/pt_direction_prior.h"
#include "people_tracking_cpp/pt_box_likelihood.h"
#include "people_tracking_cpp/pt_facemark_likelihood.h"
#include "people_tracking_cpp/pt_optical_flow_likelihood.h"
#include "people_tracking_cpp/pt_face_flow_likelihood.h"
#include "people_tracking_cpp/pt_color_likelihood.h"
#include "l_cpp/l_exception.h"

//#include <cmath>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>

using namespace kjb;
using namespace kjb::pt;

void Scene_info::set(const Scene_posterior& post, const Scene& sc, bool cmlh)
{
    const Position_prior& pos_pr = post.position_prior();
    const Direction_prior& dir_pr = post.direction_prior();
    const Face_direction_prior& fdir_pr = post.face_direction_prior();
    const Box_likelihood& bx_lh = post.box_likelihood();
    const Optical_flow_likelihood& of_lh = post.of_likelihood();
    const Face_flow_likelihood& ff_lh = post.ff_likelihood();
    const Facemark_likelihood& fm_lh = post.fm_likelihood();

    bool ih = post.infer_head();

    // main priors
    size_prior = post.use_dim_prior() ? post.dimension_prior(sc) : 0.0;
    pos_prior = post.use_pos_prior() ? pos_pr(sc) : 0.0;
    dir_prior = post.use_dir_prior() && ih ? dir_pr(sc) : 0.0;
    fdir_prior = post.use_fdir_prior() && ih ? fdir_pr(sc) : 0.0;

    // aux priors
    sp_prior = post.use_pos_prior() ? pos_pr.at_space(sc) : 0.0;
    ep_prior = post.use_pos_prior() ? pos_pr.at_endpoints(sc) : 0.0;

    // likelihoods
    box_lh = post.use_box_lh() ? bx_lh(sc) : 0.0;
    box_nlh = post.use_box_lh() ? bx_lh.at_noise(sc) : 0.0;
    oflow_lh = post.use_of_lh() ? of_lh(sc) : 0.0;
    fflow_lh = post.use_ff_lh() && ih ? ff_lh(sc) : 0.0;
    fmark_lh = post.use_fm_lh() && ih ? fm_lh(sc) : 0.0;
    fmark_nlh = post.use_fm_lh() && ih ?  fm_lh.at_noise(sc) : 0.0;

    // posterior
    pt = post(sc);

    // marginal
    marg_lh = cmlh ? lm_marginal_likelihood(sc, pt, ih) : 
        std::numeric_limits<double>::quiet_NaN();

    // counts
    dim = dims(sc, false, ih);
    num_aboxes = length(sc.association);
    num_nboxes = length(sc.association.get_available_data());
    num_afmarks = fm_lh.num_assigned_facemarks(sc);
    num_nfmarks = fm_lh.num_facemarks() - num_afmarks;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_info::set(const std::string& info)
{
    // set up maps for easy processing
    typedef std::map<std::string, double*> Sdp_map;
    typedef std::map<std::string, size_t*> Sip_map;

    Sdp_map facs;
    facs["size-prior"] = &size_prior;
    facs["pos-prior"] = &pos_prior;
    facs["dir-prior"] = &dir_prior;
    facs["fdir-prior"] = &fdir_prior;
    facs["sp-prior"] = &sp_prior;
    facs["ep-prior"] = &ep_prior;
    facs["box-lh"] = &box_lh;
    facs["box-nlh"] = &box_nlh;
    facs["oflow-lh"] = &oflow_lh;
    facs["fflow-lh"] = &fflow_lh;
    facs["fmark-lh"] = &fmark_lh;
    facs["fmark-nlh"] = &fmark_nlh;
    facs["pt"] = &pt;
    facs["marg-lh"] = &marg_lh;

    Sip_map cts;
    cts["dim"] = &dim;
    cts["num-aboxes"] = &num_aboxes;
    cts["num-nboxes"] = &num_nboxes;
    cts["num-afmarks"] = &num_afmarks;
    cts["num-nfmarks"] = &num_nfmarks;

    const size_t num_req = facs.size() + cts.size() - 1;

    // parse string
    size_t ct_req = 0;
    bool got_mlh = false;
    std::stringstream sstr(info);
    std::string tok;
    while(sstr >> tok)
    {
        size_t n = tok.find('=');
        if(n == std::string::npos)
        {
            KJB_THROW_2(
                Runtime_error, "Bad scene info; 'var=val' required");
        }
        else
        {
            std::string var = tok.substr(0, n);
            double val;
            try
            {
                val = boost::lexical_cast<double>(tok.substr(n + 1));
            }
            catch(const boost::bad_lexical_cast& lex)
            {
                KJB_THROW_2(
                    Runtime_error, "Bad scene info; value not valid number");
            }

            Sdp_map::iterator pr_p = facs.find(var);
            Sip_map::iterator pr_q = cts.find(var);
            if(pr_p == facs.end() && pr_q == cts.end())
            {
                KJB_THROW_2(
                    Runtime_error, "Bad scene info; var not valid element");
            }
            else if(pr_p != facs.end())
            {
                ASSERT(pr_q == cts.end());

                *pr_p->second = val;
                if(var == "marg-lh") got_mlh = true;
                else ++ct_req;
            }
            else
            {
                ASSERT(pr_q != cts.end());
                ASSERT(pr_p == facs.end());

                *pr_q->second = val;
                ++ct_req;
            }
        }
    }

    IFT(ct_req == num_req, Runtime_error, "Bad scene info; missing factors");
    if(!got_mlh) marg_lh = std::numeric_limits<double>::quiet_NaN();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool Scene_info::is_valid(const Scene& sc, bool ih) const
{
    const double eps = 1e-5;

    // main priors
    if(std::isnan(size_prior)
        || std::isnan(pos_prior)
        || std::isnan(dir_prior)
        || std::isnan(fdir_prior)
        || std::isnan(sp_prior)
        || std::isnan(ep_prior)
        || std::isnan(box_lh)
        || std::isnan(box_nlh)
        || std::isnan(oflow_lh)
        || std::isnan(fflow_lh)
        || std::isnan(fmark_lh)
        || std::isnan(fmark_nlh)
        || std::isnan(pt))
    {
        return false;
    }

    if(fabs(size_prior
                + pos_prior
                + dir_prior
                + fdir_prior
                + box_lh
                + oflow_lh
                + fflow_lh
                + fmark_lh
                - pt) > eps)
    {
        return false;
    }

    // counts
    if(dim != dims(sc, false, ih)) return false;
    if(num_aboxes != length(sc.association)) return false;
    if(num_nboxes != length(sc.association.get_available_data())) return false;
    //if(num_afmarks != fm_lh.num_assigned_facemarks(sc)) return false;
    //if(num_nfmarks != fm_lh.num_facemarks() - num_afmarks) return false;

    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::string Scene_info::to_string() const
{
    std::string ret;

    ret += "size-prior=" + boost::lexical_cast<std::string>(size_prior);
    ret += " pos-prior=" + boost::lexical_cast<std::string>(pos_prior);
    ret += " dir-prior=" + boost::lexical_cast<std::string>(dir_prior);
    ret += " fdir-prior=" + boost::lexical_cast<std::string>(fdir_prior);
    ret += " sp-prior=" + boost::lexical_cast<std::string>(sp_prior);
    ret += " ep-prior=" + boost::lexical_cast<std::string>(ep_prior);
    ret += " box-lh=" + boost::lexical_cast<std::string>(box_lh);
    ret += " box-nlh=" + boost::lexical_cast<std::string>(box_nlh);
    ret += " oflow-lh=" + boost::lexical_cast<std::string>(oflow_lh);
    ret += " fflow-lh=" + boost::lexical_cast<std::string>(fflow_lh);
    ret += " fmark-lh=" + boost::lexical_cast<std::string>(fmark_lh);
    ret += " fmark-nlh=" + boost::lexical_cast<std::string>(fmark_nlh);
    ret += " pt=" + boost::lexical_cast<std::string>(pt);
    ret += " marg-lh=" + boost::lexical_cast<std::string>(marg_lh);
    ret += " dim=" + boost::lexical_cast<std::string>(dim);
    ret += " num-aboxes=" + boost::lexical_cast<std::string>(num_aboxes);
    ret += " num-nboxes=" + boost::lexical_cast<std::string>(num_nboxes);
    ret += " num-afmarks=" + boost::lexical_cast<std::string>(num_afmarks);
    ret += " num-nfmarks=" + boost::lexical_cast<std::string>(num_nfmarks);

    return ret;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& kjb::pt::operator<<(std::ostream& ost, const Scene_info& info)
{
    ost << info.to_string();

    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& kjb::pt::operator>>(std::istream& ist, Scene_info& info)
{
    std::string str;
    if(std::getline(ist, str)) info.set(str);

    return ist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::scene_info_table(const Scene_info& info, std::ostream& ost)
{
    using namespace std;

    const size_t ww = 15;
    ost << left << setw(ww) << setfill(' ') << "  dim";
    ost << left << setw(ww) << setfill(' ') << info.dim << endl;

    ost << left << setw(ww) << setfill(' ') << "  boxes";
    ost << left << setw(ww) << setfill(' ') << info.num_aboxes + info.num_nboxes;
    ost << left << setw(ww) << setfill(' ') << info.num_aboxes;
    ost << left << setw(ww) << setfill(' ') << info.num_nboxes << endl;

    ost << left << setw(ww) << setfill(' ') << "  facemarks";
    ost << left << setw(ww) << setfill(' ') << info.num_afmarks + info.num_nfmarks;
    ost << left << setw(ww) << setfill(' ') << info.num_afmarks;
    ost << left << setw(ww) << setfill(' ') << info.num_nfmarks << endl;

    ost << left << setw(ww) << setfill(' ') << "  box LH";
    ost << left << setw(ww) << setfill(' ') << info.box_lh + info.box_nlh;
    ost << left << setw(ww) << setfill(' ') << info.box_lh;
    ost << left << setw(ww) << setfill(' ') << info.box_nlh << endl;

    ost << left << setw(ww) << setfill(' ') << "  FM LH";
    ost << left << setw(ww) << setfill(' ') << info.fmark_lh + info.fmark_nlh;
    ost << left << setw(ww) << setfill(' ') << info.fmark_lh;
    ost << left << setw(ww) << setfill(' ') << info.fmark_nlh << endl;

    ost << left << setw(ww) << setfill(' ') << "  OF LH";
    ost << left << setw(ww) << setfill(' ') << info.oflow_lh << endl;

    ost << left << setw(ww) << setfill(' ') << "  FF LH";
    ost << left << setw(ww) << setfill(' ') << info.fflow_lh << endl;

    ost << left << setw(ww) << setfill(' ') << "  space prior";
    ost << left << setw(ww) << setfill(' ') << info.sp_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  endpt prior";
    ost << left << setw(ww) << setfill(' ') << info.ep_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  pos prior";
    ost << left << setw(ww) << setfill(' ') << info.pos_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  dir prior";
    ost << left << setw(ww) << setfill(' ') << info.dir_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  fdir prior";
    ost << left << setw(ww) << setfill(' ') << info.fdir_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  size prior";
    ost << left << setw(ww) << setfill(' ') << info.size_prior << endl;

    ost << left << setw(ww) << setfill(' ') << "  joint LH";
    ost << left << setw(ww) << setfill(' ') << info.pt << endl;

    ost << left << setw(ww) << setfill(' ') << "  marg LH";
    ost << left << setw(ww) << setfill(' ') << info.marg_lh
                                                + info.box_nlh + info.fmark_nlh;
    ost << left << setw(ww) << setfill(' ') << info.marg_lh;
    ost << left << setw(ww) << setfill(' ') << info.box_nlh
                                                + info.fmark_nlh << endl;
}

