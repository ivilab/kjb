/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#ifndef MCMCDA_FS_H_INCLUDED
#define MCMCDA_FS_H_INCLUDED

#include <vector>
#include <string>
#include <l/l_sys_io.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>

namespace kjb {
namespace mcmcda {

/** @brief  Later. */
class Association_directory
{
public:
    std::string experiment_dp;
    std::string association_dn;

    std::string association_dp;
    std::string all_samples_dp;
    std::string tracked_images_dp;
    std::string curved_images_dp;
    std::string curves_dp;

    std::string association_fp;
    std::string all_samples_fpf;
    std::string tracked_images_fpf;
    std::string curved_images_fpf;
    std::string curves_fpf;

    std::vector<std::string> all_samples_fps;
    std::vector<std::string> tracked_images_fps;
    std::vector<std::string> curved_images_fps;
    std::vector<std::string> curves_fps;

    Association_directory(){}

    Association_directory
    (
        const std::string& exp_dp,
        const std::string& aname
    ) :
        experiment_dp(exp_dp),
        association_dn(aname),
        //association_dp(exp_dp + "/" + association_dn),
        association_dp(association_dn),
        all_samples_dp(association_dp + "/" + all_samples_dn),
        tracked_images_dp(association_dp + "/" + tracked_images_dn),
        curved_images_dp(association_dp + "/" + curved_images_dn),
        //curves_dp(association_dp + "/" + curves_dn),
        curves_dp(association_dp),
        association_fp(association_dp + "/" + association_fn),
        all_samples_fpf(all_samples_dp + "/" + all_samples_fnf),
        tracked_images_fpf(tracked_images_dp + "/" + tracked_images_fnf),
        curved_images_fpf(curved_images_dp + "/" + curved_images_fnf),
        curves_fpf(curves_dp + "/" + curves_fnf),
        // note regarding the '0' in lines below:
        //  file_names_from_format now tolerates a missing file on index '0',
        //  so it's safe to specify '0' as the first index, even if the
        //  true first index is 1.  Also, some old pollen data starts at zero,
        //  so we need to try zero.
        //  --Kyle  Nov 18, 2014
        all_samples_fps(kjb::file_names_from_format(all_samples_fpf,0)),
        tracked_images_fps(kjb::file_names_from_format(tracked_images_fpf,0)),
        curved_images_fps(kjb::file_names_from_format(curved_images_fpf,0)),
        curves_fps(kjb::file_names_from_format(curves_fpf,0))
    {}

    void make_dirs() const
    {
        ETX(kjb_c::kjb_mkdir(experiment_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(association_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(all_samples_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(tracked_images_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(curved_images_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(curves_dp.c_str()));
    }

public:
    // sub directory names
    const static std::string all_samples_dn;
    const static std::string tracked_images_dn;
    const static std::string curved_images_dn;
    const static std::string curves_dn;

    // file name formats
    const static std::string association_fn;
    const static std::string all_samples_fnf;
    const static std::string tracked_images_fnf;
    const static std::string curved_images_fnf;
    const static std::string curves_fnf;
};

/** @brief  Later. */
class Experiment_directory
{
public:
    std::string experiment_dp;

    std::string preprocessed_dp;
    std::string blobbed_dp;
    std::string pixels_dp;

    std::string points_fpf;
    std::string preprocessed_fpf;
    std::string blobbed_fpf;
    std::string pixels_fpf;

    std::vector<std::string> points_fps;
    std::vector<std::string> preprocessed_fps;
    std::vector<std::string> blobbed_fps;
    std::vector<std::string> pixels_fps;

    Experiment_directory(const std::string& exp_dir_path) :
        experiment_dp(exp_dir_path),
        preprocessed_dp(experiment_dp + "/" + preprocessed_dn),
        blobbed_dp(experiment_dp + "/" + blobbed_dn),
        pixels_dp(experiment_dp + "/" + pixels_dn),
        points_fpf(experiment_dp + "/" + points_fnf),
        preprocessed_fpf(preprocessed_dp + "/" + preprocessed_fnf),
        blobbed_fpf(blobbed_dp + "/" + blobbed_fnf),
        pixels_fpf(pixels_dp + "/" + pixels_fnf),
        points_fps(kjb::file_names_from_format(points_fpf,0)),
        preprocessed_fps(kjb::file_names_from_format(preprocessed_fpf,0)),
        blobbed_fps(kjb::file_names_from_format(blobbed_fpf,0)),
        pixels_fps(kjb::file_names_from_format(pixels_fpf,0))
    {
        ETX(kjb_c::kjb_mkdir(experiment_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(blobbed_dp.c_str()));
        ETX(kjb_c::kjb_mkdir(pixels_dp.c_str()));
    }

    Association_directory get_association_dir(const std::string& name)
    {
        return Association_directory(experiment_dp, name);
    }

private:
    // sub directory names
    static const std::string preprocessed_dn;
    static const std::string blobbed_dn;
    static const std::string pixels_dn;

    // file name formats
    static const std::string points_fnf;
    static const std::string preprocessed_fnf;
    static const std::string blobbed_fnf;
    static const std::string pixels_fnf;
};

}} //namespace kjb::mcmcda

#endif /*MCMCDAFS_H_INCLUDED */

