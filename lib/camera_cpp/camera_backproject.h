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

/* $Id$ */

#ifndef KJB_CPP_CAMERA_BACKPROJECT
#define KJB_CPP_CAMERA_BACKPROJECT

#include <camera_cpp/perspective_camera.h>
#include <l_cpp/l_algorithm.h>
#include <l_cpp/l_util.h>
#include <l/l_sys_io.h>
#include <l/l_sys_lib.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <g_cpp/g_line.h>
#include <g_cpp/g_camera.h>
#include <limits>

namespace kjb {

/** @class  Back_projector
 *
 * Functor class used to back project points and intersect the resulting ray
 * with a plane.
 */
class Back_projector
{
public:
    Back_projector
    (
        const Perspective_camera& cam, 
        const Vector& plane,
        const Vector& plane_normal
    ) :
        ground_plane_(plane),
        ground_normal_(plane_normal),
        camera_center_(cam.get_camera_centre()),
        M(cam.build_camera_matrix())
    {
        camera_center_.resize(3);
    }

    /**
     * @bried Backproject a 2D point(u, v) on the image plane to a point in 3D 
     */
    Vector operator()(double u, double v) const
    {
        Vector intersection;
        double t;

        // Init point on screen
        Vector screen_point(u, v);

        screen_point.resize(3, 1.0);

        // Get 3D directional vector from the center of the camera to the point
        Vector camera_dir = backproject(screen_point, M);

        // Line-plane intersection in 3D
        t = intersect_line_with_plane(
                            camera_center_,
                            camera_dir,
                            ground_plane_,
                            ground_normal_);

        // Check for valid intersection
        if (t != t)
        {
            /* t == NaN */
            /* line and plane are coincident */
            return Vector();
        }
        else if(t == std::numeric_limits<double>::infinity())
        {
            /* t == infinity */
            /* line and plane are parallel */
            return Vector();
        }
        else
        {
            return camera_center_ + t * camera_dir;
        }
    }

protected:
    Vector ground_plane_;
    Vector ground_normal_;
    Vector camera_center_;
    Matrix M;
};

/** @class  Ground_back_projector
 *
 * Functor class used to back project points and intersect the resulting ray
 * with the ground plane.
 */
class Ground_back_projector : public Back_projector
{
    typedef Back_projector Base;
public:
    Ground_back_projector
    (
        const Perspective_camera& cam,
        double ground_height
    ) :
        Back_projector(
                cam,
                Vector(0.0, ground_height, 0.0),
                Vector(0.0, 1.0, 0.0)),
            y_behind_camera_()
    {
        init(cam);
    }

    /**
     * @bried Backproject a 2D point(u, v) on the image plane to a point in 3D 
     */
    Vector operator()(double u, double v) const
    {
        Vector result = Base::operator()(u, v);
        if(result.size() == 0) return result;

        if(result[2] > y_behind_camera_)
        {
            return Vector();
        }

        return result;
    }

private:
    void init(const Perspective_camera& cam)
    {
        camera_center_.resize(3);
        Matrix R = cam.get_modelview_matrix();
        R.resize(3,3);

        // get y-value t
        Vector cam_down = R.transpose() * Vector(0.0, -1.0, 0.0);
        double t = intersect_line_with_plane(camera_center_, cam_down,
                                             ground_plane_, ground_normal_);

        // !nan; i.e. line and plane are coincident
        assert(!(t != t)); 

        // line and plane are parallel
        assert(t != std::numeric_limits<double>::infinity()); 

        y_behind_camera_ = (camera_center_ + t * cam_down)[2];
    }

    double y_behind_camera_;
};

/**
 * @brief   Back project the 2d points to find the height in 3D.
 *          Assume that the bottom is on the ground
 */
inline
double get_3d_height
(
    const Vector& bottom_2d, 
    const Vector& top_2d,
    const Perspective_camera& camera
)
{
    KJB(ASSERT((bottom_2d(1) < top_2d(1))));
    Ground_back_projector ground_back_projector(camera,  0.0);
    Vector bottom_3d = ground_back_projector(bottom_2d(0), bottom_2d(1));
    if(bottom_3d.empty())
    {
        return 0.0;
    }

    Vector height_normal(0.0, 0.0, -1.0);
    Back_projector height_back_projector(camera, bottom_3d, height_normal);

    Vector top_3d = height_back_projector(top_2d(0), top_2d(1));
    if(top_3d.empty())
    {
        return 0.0;
    }

    return top_3d(1);
}

} // namespace kjb

#endif /*KJB_CPP_CAMERA_BACKPROJECT */

