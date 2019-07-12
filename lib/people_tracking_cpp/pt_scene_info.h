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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_SCENE_INFO_H
#define PT_SCENE_INFO_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <string>
#include <iostream>

namespace kjb {
namespace pt {

/** @brief  Data structure that represents the breakdown of a posterior. */
struct Scene_info
{
    void set(const Scene_posterior& post, const Scene& sc, bool cmlh);
    void set(const std::string& info);

    bool is_valid(const Scene& sc, bool ih) const;
    std::string to_string() const;

    double size_prior;
    double pos_prior;
    double dir_prior;
    double fdir_prior;
    double sp_prior;
    double ep_prior;
    double box_lh;
    double box_nlh;
    double oflow_lh;
    double fflow_lh;
    double fmark_lh;
    double fmark_nlh;
    double pt;
    double marg_lh;

    size_t dim;
    size_t num_aboxes;
    size_t num_nboxes;
    size_t num_afmarks;
    size_t num_nfmarks;
};

/** @brief  Put operator. */
std::ostream& operator<<(std::ostream& ost, const Scene_info& info);

/** @brief  Get operator. */
std::istream& operator>>(std::istream& ist, Scene_info& info);

/** @brief  Print scene info in a table layout. */
void scene_info_table(const Scene_info& info, std::ostream& ost);

}} // namespace kjb::pt

#endif /*PT_SCENE_INFO_H */

