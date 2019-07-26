/* $Id: gr_frustum.h 21599 2017-07-31 00:44:30Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifdef KJB_HAVE_OPENGL
#ifndef KJB_GR_CPP_GR_FRUSTUM
#define KJB_GR_CPP_GR_FRUSTUM

#include "l/l_sys_debug.h"  /* For ASSERT */`
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector_d.h"
#include "i_cpp/i_image.h"
#include "gr_cpp/gr_opengl_texture.h"
#include "gr_cpp/gr_renderable.h"

#include <boost/shared_ptr.hpp>

namespace kjb 
{
namespace opengl
{
/**
 * A simple class that visualizes a camera frustum wireframe, optionally with an 
 * image on the near plane.  To render on the far plane, just swap the near and
 * far values.  
 *
 * One common visualization is to display a camera as a pyramid, 
 * with the tip at the camera center and the base representing the virtual 
 * image plane.  This can be achieved by setting the FAR plane to 0,
 * and the near plane some positive distance from the camera.
 *
 * If an image is provided, the image is displayed on the near plane so that
 * if it is viewed from the camera itself, it appears exactly undistorted.
 */
class Frustum_display : public kjb::Renderable
{
public:

    /**
     * @param center Camera center in world coordinates
     * @param cam Full camera matrix
     * @param image_width width of the camera's image plane in pixels (or screen width)
     * @param image_height width of the camera's image plane in pixels (or screen width)
     * @param near distance to near plane, or the plane which will contain the image (positive values for front of camera)
     * @param far distance to far plane (positive values for front of camera)
     */
    Frustum_display(
            const kjb::Vector& center,
            const kjb::Matrix& cam,
            double image_width,
            double image_height,
            double near,
            double far) :
        center_(),
        z_near_(near),
        z_far_(far),
        image_(),
        m_inv_(),
        width_(image_width),
        height_(image_height),
        show_frame_(true),
        image_alpha_(0.5)
    {
        set_camera(center, cam);
    }

    void show_frame(bool b)
    {
        show_frame_ = b;
    }

    void set_image_opacity(double alpha)
    {
        image_alpha_ = alpha;
    }

    void set_camera(const kjb::Vector& center, const kjb::Matrix& cam)
    {
        ASSERT(center.size() >= 3 && center.size() <= 4);
        std::copy(center.begin(), center.begin() + 3, center_.begin());

        m_inv_ = cam;
        m_inv_ = kjb::matrix_inverse(m_inv_.resize(3,3));
        m_inv_.resize(4,4,0.0);
        m_inv_(3,3) = 1.0;

    }

    void set_image(const kjb::Image& img)
    {
        image_.reset(new kjb::opengl::Texture());
        image_->set(img);
    }

    void set_near(double near)
    {
        z_near_ = near; 
    }

    void render() const;

private:
    kjb::Vector3 center_;
    double z_near_;
    double z_far_;
    boost::shared_ptr<kjb::opengl::Texture> image_;

    kjb::Matrix m_inv_;
    double width_;
    double height_;
    bool show_frame_;
    double image_alpha_;
};

}
}
#endif
#endif /* KJB_HAVE_OPENGL */
