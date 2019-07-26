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

/* $Id: pt_util.h 20912 2016-10-30 17:52:31Z ernesto $ */

#ifndef PT_UTIL
#define PT_UTIL

#include <m_cpp/m_vector.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <flow_cpp/flow_integral_flow.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <camera_cpp/perspective_camera.h>
#include <detector_cpp/d_bbox.h>
#include <detector_cpp/d_deva_detection.h>
#include <detector_cpp/d_deva_facemark.h>
#include <boost/optional.hpp>

namespace kjb {
namespace pt {

/**
 * @brief   Changes Vector to standard (camera) coordinate system.
 */
inline
void standardize(Vector& v, double cam_width, double cam_height)
{
    Vector t(-cam_width/2.0, -cam_height/2.0);
    v += t;
    v[1] = -v[1];
}

/**
 * @brief   Changes box coordinates to standard (camera) coordinate system.
 */
inline
void standardize(Bbox& box, double cam_width, double cam_height)
{
    Vector t(-cam_width/2.0, -cam_height/2.0);

    // will flip y-axis
    Vector s(1.0, -1.0);

    translate(box, t);
    scale(box, s);
}

/**
 * @brief   Changes box coordinates to standard (camera) coordinate system.
 */
void standardize
(
    Deva_facemark& face,
    double cam_width,
    double cam_height
);

/**
 * @brief   Changes vector to unstandard (image) coordinate system.
 */
inline
void unstandardize(Vector& v, double cam_width, double cam_height)
{
    Vector t(cam_width/2.0, cam_height/2.0);
    v[1] = -v[1];
    v += t;
}

/**
 * @brief   Changes box coordinates to unstandard (image) coordinate system.
 */
inline
void unstandardize(Bbox& box, double cam_width, double cam_height)
{
    Vector t(cam_width/2.0, cam_height/2.0);

    // will flip y-axis
    Vector s(1.0, -1.0);
    scale(box, s);
    translate(box, t);
}

/**
 * @brief   Changes Deva_facemark coordinates to unstandard (image) coordinate system.
 */
void unstandardize
(
    Deva_facemark& face,
    double cam_width,
    double cam_height
);

/**
 * @brief   Changes box coordinates to unstandard (image) coordinate system.
 */
void standardize
(
    Deva_facemark& face,
    double cam_width,
    double cam_height
);

/**
 * @brief   Create data from GT box trajectories and compute association
 *          based upon it.
 */
Ascn make_gt_association
(
    const Box_data& data,
    const Box_trajectory_map& gt_btrajs,
    double width,
    double height,
    boost::optional<const Perspective_camera&> cam_p = boost::none,
    boost::optional<const Facemark_data&> fm_data_p = boost::none
);

/**
 * @brief   Create data from GT box trajectories and compute association
 *          based upon it.
 */
Box_data make_gt_data
(
    const Box_trajectory_map& gt_btrajs,
    double width,
    double height
);

/**
 * @brief   Propagated 2D information using optical flow
 */
struct Propagated_2d_info
{
    double x;
    double y_bottom;
    Bbox box;
    int origin_frame; // + foward, - backward
};

/**
 * @brief   Comparator for Propagated_2d_info
 */
struct Comparator_2d
{
    inline bool operator()
    (
        const Propagated_2d_info& p1,
        const Propagated_2d_info& p2
    ) const
    {
        return p1.y_bottom < p2.y_bottom;
    }
};

/**
 * @brief   Propagate the detection boxes based on optical flow to neighboring
 *          frames.
 */
std::map<int, std::vector<Propagated_2d_info> > propagate_boxes_by_flow
(
    const Target& target,
    const std::vector<Integral_flow>& x_flows,
    const std::vector<Integral_flow>& y_flows,
    const std::vector<Integral_flow>& back_x_flows,
    const std::vector<Integral_flow>& back_y_flows,
    size_t duration
);

/**
 * @brief   Given the chi-square distance between the histograms of two
 *          detection boxes
 * @return  the probability of the two detection boxes belong to the same person
 *
 */
inline
double chi_squared_dist_to_prob(double chi_square_dist)
{
    return 1.0/(1.0 + exp(-(3.50281 + chi_square_dist * (-64.49790))));
}

/**
 * @brief   Scores based on histogram
 */
typedef std::map<const Detection_box*, Matrix> Hist_map;
class Feature_score
{
public:
    Feature_score
    (
        const std::vector<Hist_map>& hist,
        const std::vector<Integral_flow>& flows_x,
        const std::vector<Integral_flow>& flows_y
    ) : m_hists(hist),
        m_flows_x(flows_x),
        m_flows_y(flows_y)
    {}

    std::vector<double> operator()
    (
        const Target* target_p,
        int t1,
        const Detection_box* candidate_p,
        int t2,
        size_t wsz = 1
    ) const;

private:
    const std::vector<Hist_map>& m_hists;
    const std::vector<Integral_flow>& m_flows_x;
    const std::vector<Integral_flow>& m_flows_y;
};

/**
 * @brief   Compute the average flow in a box, ignoring occluded parts.
 * @return  A kjb::Vector representing the average flow.
 *
 */
Vector average_box_flow
(
    const Integral_flow& fx,
    const Integral_flow& fy,
    const Bbox& box,
    const Visibility& vis
);

/**
 * @brief   Prune the deva boxes based on the average entity height
 */
void prune_by_height
(
    std::vector<Deva_detection>& deva_boxes,
    double image_width,
    double image_height,
    const Perspective_camera& camera,
    double average_height
);

/** @brief  Projects a 3D point into a 2D point using camera. */
inline
Vector project_point(const Perspective_camera& cam, const Vector& x)
{
    Vector temp = x;
    temp.resize(4, 1.0);
    return geometry::projective_to_euclidean_2d(cam.get_camera_matrix() * temp);
}

/**
 * @brief   Update facemark detections for face trajectories.
 * @return  The number of noisy detections
 */
void update_facemarks(const Ascn& ascn, const Facemark_data& fms);

/**
 * @brief   Update facemark detections for face trajectories.
 * @return  The number of noisy detections
 */
//void update_facemarks(const Scene& scene, const Facemark_data& fms);

/**
 * @brief   Clear facemark detections for face trajectories.
 */
void clear_facemarks(const Ascn& ascn);

/** @brief  Estimate looking subjects on all scene. */
void find_looking_subjects(const Scene& scene, double thresh = 0.3);

/** @brief  Find set of gaze intersections. */
std::vector<Vector> locations_3d(const Scene& scene, double thresh = 0.3);

}} // namespace kjb::pt

#endif /*PT_UTIL */

