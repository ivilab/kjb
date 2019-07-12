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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_file_util.h 17393 2014-08-23 20:19:14Z predoehl $ */


#ifndef PT_FILE_UTIL_H_
#define PT_FILE_UTIL_H_

#include <people_tracking_cpp/pt_entity.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <l_cpp/l_filesystem.h>

namespace kjb
{
namespace pt
{

/**
 * @brief   Represents the input directory for the people tracking project
 */
class Input_directory
{
public:
    /** @brief  Does nothing. Does not initialize anything! */
    Input_directory() {}

    /** @brief  Creates input directory in given movie dir path. */
    Input_directory(const std::string& movie_dp) : base_dir_(movie_dp)
    {}

    /** @brief  Returns movie path. */
    const std::string& get_movie_dp() const { return base_dir_; }

    /** @brief  Returns path to camera file. */
    std::string get_camera_fp() const
    {
        return base_dir_ + "/camera/camera.txt";
    }

    /** @brief  Returns path to ransac camera file. */
    std::string get_ransac_camera_fp() const
    {
        return base_dir_ + "/camera/ransac_camera.txt";
    }

    /** @brief  Returns paths to files containing detections of given entity. */
    std::vector<std::string> get_hboxes_fps(Entity_type entity_type) const
    {
        const std::string& entity_name = get_entity_type_name(entity_type); 
        return file_names_from_format((base_dir_+ "/detectors/"
                                                + entity_name + "/%05d.txt"));
    }

    /** @brief  Returns paths to files containing detections of given entity. */
    std::vector<std::string> get_hboxes_fps(const std::string& entity_name) const
    {
        Entity_type type = boost::lexical_cast<Entity_type>(entity_name);
        return get_hboxes_fps(type);
    }

    /** @brief  Returns paths to files containing detections of given entity. */
    std::vector<std::string> get_detection_boxes_fps(Entity_type entity_type) const
    {
        const std::string& entity_name = get_entity_type_name(entity_type); 
        return get_detection_boxes_fps(entity_name);
    }

    /** @brief  Returns paths to files containing detections of given entity. */
    std::vector<std::string> get_detection_boxes_fps(const std::string& entity_name) const
    {
        return file_names_from_format((base_dir_+ "/detection_boxes/"
                                                + entity_name + "/%05d.txt"));
    }

    /** @brief  Returns paths to images of frames. */
    std::vector<std::string> get_frames_fps() const
    {
        return file_names_from_format(base_dir_ + "/frames/%05d.jpg");
    }

    /** @brief  Returns paths to detected face files. */
    std::vector<std::string> get_face_fps() const
    {
        return file_names_from_format(base_dir_ + "/features/faces/%05d.txt");
    }

    /** @brief  Returns paths to detected face files. */
    std::vector<std::string> get_face_fps(const std::string& face_subdir) const
    {
        return file_names_from_format(base_dir_ + "/features/"
                                                + face_subdir + "/%05d.txt");
    }

    /** @brief  Returns paths to detected face files. */
    std::vector<std::vector<std::string> > get_opencv_face_fps() const
    {
        std::vector<std::vector<std::string> > face_fps;

        face_fps.push_back(file_names_from_format(base_dir_ + 
                    "/features/face_opencv/frontal_alt/%05d.txt"));
        face_fps.push_back(file_names_from_format(base_dir_ + 
                    "/features/face_opencv/frontal_alt2/%05d.txt"));
        face_fps.push_back(file_names_from_format(base_dir_ + 
                    "/features/face_opencv/frontal_alt_tree/%05d.txt"));
        face_fps.push_back(file_names_from_format(base_dir_ + 
                    "/features/face_opencv/frontal_default/%05d.txt"));
        face_fps.push_back(file_names_from_format(base_dir_ + 
                    "/features/face_opencv/profile/%05d.txt"));

        return face_fps;
    }

    /** @brief  Returns paths to images of frames. */
    std::vector<std::string> get_optical_flow_fps(size_t frame_rate=1 ) const
    {
        //boost::format dp_fmt(base_dir_ + "/features/optical_flow/%02d/");
        //std::string dp = (dp_fmt % frame_rate).str();
        std::string dp = base_dir_ + "/features/optical_flow/brox/sparse";

        return file_names_from_format(dp + "/%05d.txt");
    }

    /** @brief  Returns paths histogram files of given entity and bins. */
    std::vector<std::string> get_histogram_fps
    (
        const std::string& entity_name,
        const std::string& num_bins
    ) const
    {
        return file_names_from_format(base_dir_ + "/features/color_hist/"
                                                + entity_name + "/" + num_bins
                                                + "/%05d_color_hist.txt");
    }

private:
    std::string base_dir_;
};

/**
 * @brief   Represents the output directory for the PT tracking project
 */
class Output_directory
{
public:
    /** @brief  Does nothing. Does not initialize anything! */
    Output_directory() {}

    /** @brief  Creates directory for given video and entity. */
    Output_directory(const std::string& experiment_dp) : base_dir_(experiment_dp)
    {}

    /** @brief  Returns video directory path. */
    const std::string& get_experiment_dp() const { return base_dir_; }

    /** @brief  Returns path to directory containing tracks. */
    std::string get_trajectories_dp() const
    {
        return base_dir_ + "/tracks";
    }

    /** @brief  Returns path to file containing the inferred association. */
    std::string get_association_fp() const
    {
        return get_trajectories_dp() + "/association.txt";
    }

    /** @brief  Returns path to file containing the inferred camera. */
    std::string get_camera_fp() const
    {
        return get_trajectories_dp() + "/camera.txt";
    }

    /** @brief  Returns path to file containing the inferred parameters. */
    std::string get_parameters_fp() const
    {
        return get_trajectories_dp() + "/params.txt";
    }

    /** @brief  Returns path to file containing best posteriors. */
    std::string get_best_posteriors_fp() const
    {
        return base_dir_ + "/best_posteriors.txt";
    }

    /** @brief  Returns path to file containing all sample logs. */
    std::string get_sample_logs_fp() const
    {
        return base_dir_ + "/sample_log.txt";
    }

    /** @brief  Returns path to file containing all sample MOTA values. */
    std::string get_sample_mota_vals_fp() const
    {
        return base_dir_ + "/sample_mota_vals.txt";
    }

    /** @brief  Returns path to file containing all sample MOTP values. */
    std::string get_sample_motp_vals_fp() const
    {
        return base_dir_ + "/sample_motp_vals.txt";
    }

    /** @brief  Returns path to summary file. */
    std::string get_summary_fp() const
    {
        return base_dir_ + "/summary.txt";
    }

    /** @brief  Returns path to the visualization directory. */
    std::string get_visualization_dp() const
    {
        return base_dir_ + "/visualization";
    }

    /** @brief  Returns path to visualization frames directory. */
    std::string get_visualization_frames_dp() const
    {
        return get_visualization_dp() + "/frames";
    }

    /** @brief  Returns path to tracks video. */
    std::string get_tracks_video_fp() const
    {
        return get_visualization_dp() + "/tracks.avi";
    }

    /** @brief  Returns path to metadata file. */
    std::string get_metadata_fp() const
    {
        return base_dir_ + "/metadata.txt";
    }

    /** @brief  Returns the path to the sample history directory. */
    std::string get_all_samples_dp() const
    {
        return base_dir_ + "/samples";
    }

    /** @brief  Returns the path to the proposal history directory. */
    std::string get_proposals_dp() const
    {
        return base_dir_ + "/proposals";
    }

private:
    std::string base_dir_;
};

}} // namespace kjb::pt

#endif /*PT_FILE_UTIL_H_ */

