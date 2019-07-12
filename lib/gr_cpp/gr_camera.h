/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */


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
|     Luca Del Pero, Kyle Simek
|
* =========================================================================== */


/**
 * @file
 *
 * @author Luca Del Pero, Kyle Simek
 *
 * @brief classes to interface camera models to opengl for rendering
 *
 */


#ifndef KJB_CAMERA_H
#define KJB_CAMERA_H

#include "m_cpp/m_matrix.h"
#include "gr_cpp/gr_rigid_object.h"


namespace kjb_c {
    struct KJB_image;
}

namespace kjb {

class Image;
class Vector;
class Int_matrix;
class Polygon;

/**
 * @class Base_gl_interface This provides the base interface between opengl and a camera
 * parametrized in terms of modelview matrix, projection matrix, and near and far clipping
 * planes.
 * This class manipulates the modelview and the projection matrices by accessing
 * them directly without calling glFrustum od gluPerspective. Please use it
 * only if you know what you are doing. The clipping planes are handled
 * by setting
 * projection_matrix(2,2) = near + far;
 * projection_matrix(2,3) = near*far;
 * projection_matrix(3,2) = -1
 * and then by using applying the glOrtho transformation with parameters
 *  (-viewport_width/2, viewport_width/2, -viewport_height/2, viewport_height/2, near, far)
 * This will clip stuff outside the viewing frustum and normalize the
 * frustum itself to the normalized device coordinates opengl expects.
 *
 * If you want to see the math, please go to
 * http://sightations.wordpress.com/2010/08/03/simulating-calibrated-cameras-in-opengl
 */
class Base_gl_interface
{
typedef Base_gl_interface Self;
public:
    Base_gl_interface(double inear = 10, double ifar = 10000);
    Base_gl_interface(const Base_gl_interface &);
    virtual Base_gl_interface& operator=(const Base_gl_interface& src);

    virtual ~Base_gl_interface() {}

    /** Swap contents with another Perspective_camera.
     * This is implemented to run much more quickly than:
     *         Perspective_camera tmp = b;
     *         b = a;
     *         a = tmp;
     */
    virtual void swap(Self& other)
    {
        using std::swap;

        modelview_matrix.swap(other.modelview_matrix);
        projection_matrix.swap(other.projection_matrix);
        swap(near, other.near);
        swap(far, other.far);

    }

    /** @brief Sets the gl modelview matrix based on the current camera parameters */
    virtual void set_gl_modelview() const;

    /** @brief Sets the gl projection matrix based on the current camera parameters */
    virtual void set_gl_projection() const;

    /** @brief Multiplies the current gl projection matrix with this camera's matrix */
    virtual void mult_gl_projection() const;

    /** @brief Prepares the opengl for rendering by setting the gl modelview and projection
     *  matrix based on the current camera parameters
     *  It also clears the depth and the color buffers, and sets the background color
     *  to black */
    void prepare_for_rendering(bool clean_buffers) const;


    /** @brief sets the near clipping plane distance from the camera */
    void set_near_clipping_plane(double inear);

    /** @brief sets the far clipping plane distance from the camera */
    void set_far_clipping_plane(double ifar);

    /** @brief sets the near and far clipping plane distances from the camera */
    void set_clipping_planes(double inear, double ifar);

    /** @brief gets the near clipping plane distance from the camera */
    inline double get_near() const {return near;}

    /** @brief gets the far clipping plane distance from the camera */
    inline double get_far() const {return far;}

    /** @brief returns the gl modelview matrix */
    inline const Matrix & get_modelview_matrix() const {return modelview_matrix; }
    /** @brief returns the projection matrix;  note: this needs to be preceeded by a glOrtho call with the window dimensions before sending this to opengl. */
    inline const Matrix & get_projection_matrix() const 
    {
        return  projection_matrix;
    }

    /** @brief returns the exact matrix that opengl uses for its projection matrix.  this is effectively glOrtho(-vp_width/2, vp_width/2, -vp_height/2, vp_height/2, znear, zfar), followed by glMultMatrix(get_projection_matrix().  Note that if the viewport size changes, you'll likely need to re-call this, or understand that the pixels sizes will change.
     */
    Matrix get_gl_projection_matrix() const;
    

    /** @brief returns the ratio between the gl viewport width and the gl viewport height*/
    static double get_gl_viewport_aspect();
    /** @brief returns the gl viewport width*/
    static double get_gl_viewport_width();
    /** @brief returns the gl viewport height*/
    static double get_gl_viewport_height();
    /** @brief gets the gl viewport width and height*/
    static void get_gl_viewport_size(double * w, double *h);
    /** @brief gets all the g; viewport parameters (x,y,width,height)*/
    static void get_gl_viewport(double * x, double *y, double *w, double *h);

    /** @brief sets the gl viewport size */
    static void set_gl_viewport_size(double, double);
    /** @brief sets all the gl viewport parameters (x,y,width,length) */
    static void set_gl_viewport(double x, double y, double w, double h);

    /** @brief captures the gl view and stores it into a KJB_image */
    static void capture_gl_view(kjb_c::KJB_image** img_out);
    /** @brief captures the gl view and saves into a file */
    static void capture_gl_view(const char* fname);
    /** @brief  captures the gl view and saves into a file, by appending N to the filename */
    static void capture_gl_view(const char* fname_fmt, uint32_t N);
    /** @brief captures the gl view and stores it into a Int_matrix.
     * The four bytes used by GL to represent the four channels (R,G,B,A)
     * are packaged into a single integer.  */
    static void capture_gl_view(kjb::Int_matrix & matrix);
    /** @brief captures the gl view and stores it into a c++ image */
    static void capture_gl_view(Image & img_out);

    /** Puts the content of the input image in the GL frame buffer */
    static void set_gl_view(const kjb::Image & img_in);

    void project_point(double & x, double & y,double &z, const kjb::Vector & point3D, double img_height) const;

    /** Constructs an image from an int matrix. Each integer consists of four bytes, and
     *  each of them is interpreted as one of the GL channels (RGBA, or ABGR if the architecture
     *  is little endian). This is MOSTLY (if not only) used for DEBUGGING */
    static void construct_image_from_int_matrix(kjb::Image & im, kjb::Int_matrix & m);

    /** @brief  checks whether a given polygon is visible under the given camera parameters*/
    virtual bool Polygon_visibility_test(const kjb::Polygon & p, double epsilon = 0) const;

    /** @brief sets an entry of the modelview matrix */
    inline void set_modelview_entry(unsigned int row, unsigned int col, double value)
    {
        modelview_matrix(row, col) = value;
    }

    /** @brief sets an entry of the projection matrix */
    inline void set_projection_entry(unsigned int row, unsigned int col, double value)
    {
        projection_matrix(row, col) = value;
    }

    /** @brief returns an entry of the modelview matrix */
    inline double& get_modelview_entry(unsigned int row, unsigned int col)
    {
        return modelview_matrix(row, col);
    }

    /** @brief returns an entry of the modelview matrix */
    inline double get_modelview_entry(unsigned int row, unsigned int col) const
    {
        return modelview_matrix(row, col);
    }

    /** @brief returns an entry of the projection matrix */
    inline double& get_projection_entry(unsigned int row, unsigned int col)
    {
            return projection_matrix(row, col);
    }

    /** @brief returns an entry of the projection matrix */
    inline double get_projection_entry(unsigned int row, unsigned int col) const
    {
            return projection_matrix(row, col);
    }


protected:
    /** @brief constructs a Base_gl_interface from a modelview and a projection matrix */
    Base_gl_interface(const Matrix &, const Matrix &, double near, double far);

    /** @brief sets the modelview matrix */
    void set_modelview_matrix(const Matrix &mv);
    /** @brief sets the projection matrix */
    void set_projection_matrix(const Matrix &pm);

    /** @brief scales the modelview matrix using glScale */
    void scale_modelview(double xscale, double yscale, double zscale);



private:
    Matrix modelview_matrix;
    Matrix projection_matrix;
    double near;
    double far;

};

/**
 * @class Parametric_camera_gl_interface This class provides an interface
 * between OpenGL and a perspective camera parametrized in terms of extrinsic
 * parameters (three rotations and camera center) and intrinsic parameters
 * (focal length, principal point position, aspect ratio, skew). See Forsyth
 * and Ponce for further details (Computer Vision, a modern approach).
 * Rotations are handled using the classic Tait-Bryan formulation with
 * pitch, yaw and roll. Please see @class Rigid_object for further details
 *
 * Methods of this class update the gl modelview and projection parameters
 * so that they reflect the changes in the camera extrinsic and intrinsic
 * parameters
 */
class Parametric_camera_gl_interface : public Base_gl_interface, public Rigid_object
{
typedef Parametric_camera_gl_interface Self;
public:
    Parametric_camera_gl_interface(double inear = 10, double ifar = 10000);
    Parametric_camera_gl_interface(const Parametric_camera_gl_interface &);
    virtual Parametric_camera_gl_interface& operator=(const Parametric_camera_gl_interface& src);
    virtual Parametric_camera_gl_interface* clone() const;
    virtual ~Parametric_camera_gl_interface() {}

    virtual void swap(Self& other)
    {
        Base_gl_interface::swap(other);
        Rigid_object::swap(other);
        camera_center_in_world_coordinates.swap(other.camera_center_in_world_coordinates);
    }

    //Intrinsic parameters
    /** @brief sets the focal length */
    void set_focal_length(double focal_length, double skew, double aspect_ratio);
    /** @brief sets the aspect ratio */
    void set_aspect_ratio(double aspect_ratio, double focal_length, double skew);
    /** @brief sets the skew */
    void set_skew(double skew, double aspect_ratio, double focal_length);

    /** @brief sets the x coordinate of the principal point */
    void set_principal_point_x(double px);
    /** @brief sets the y coordinate of the principal point */
    void set_principal_point_y(double py);
    /** @brief sets both coordinates of the principal point */
    inline void set_principal_point(double px, double py)
    {
        set_principal_point_x(px);
        set_principal_point_y(py);
    }

    /** @brief sets the focal length assuming no skew */
    void set_focal_no_skew(double focal_length, double aspect_ratio);

    /** @brief sets the focal length assuming aspect_ratio=1*/
    void set_focal_no_aspect_ratio(double focal_length, double skew);

    /** @brief sets the focal length assuming aspect_ratio=1 and no skew*/
    void set_focal_no_aspect_ratio_no_skew(double focal_length);

    /** @brief sets all the intrinsic parameters of the camera */
    void set_intrinsic_parameters(
            double focal_length,
            double aspect_ratio,
            double skew,
            double px,
            double py);

    //Extrinsic parameters

    /** @brief Sets the position of the camera center */
    void set_camera_center(const kjb::Vector & center);

    /** @brief Sets the world origin in camera coordinates (the "t" vector in Forsyth) */
    void set_world_origin(const kjb::Vector & center);

    /** @brief Sets the world scale along the three major axes using glScaled
     * @note you must call this after calling prepare_for_rendering or set_gl_modelview.*/
    inline void set_world_scale(double xscale, double yscale, double zscale)
    {
        scale_modelview(xscale, yscale, zscale);
    }

    /** @brief Sets the rotation angles of this camera */
    void set_rotation_angles(double pitch, double yaw, double roll);

    /** @brief Transform a point from the world coordinate system to the camera coordinate system */
    void transform_point_to_camera_frame(kjb::Vector & point) const;

    /** @brief Rotates a point from the world coordinate system to the camera coordinate system */
    void rotate_point_to_camera_frame(kjb::Vector & point) const;

    /** @brief Transforms a point in camera coordinates to world coordinates */
    void get_point_in_world_coordinates
    (
        const kjb::Vector & point_in_camera_coordinates,
        kjb::Vector & point_in_world_coordinates
    ) const;

    bool Polygon_visibility_test(const kjb::Polygon & p, double epsilon = 0) const;

    /** @brief Given a point in world coordinates, it returns true if the point is in the viewing frustum, false otherwise */
    bool is_point_in_camera_frustum
    (
            const kjb::Vector & point_in_world_coordinates,
            double & ox,
            double & oy,
            unsigned int num_rows,
            unsigned int num_cols
    ) const;


private:
    /** @brief Sets the angles from a rotation matrix. The use of this function
     * should be limited to this class scope
     */
    void set_rotation_matrix(const kjb::Matrix & rotation_matrix);

    /** @brief Translates the current camera center */
    void translate(double dx, double dy, double dz);

    /** @brief sets the rotation angles and translates the current camera center */
    void set_rotations_and_translate(double pitch, double yaw, double roll, double dx, double dy, double dz);

    /** @brief Transforms the current camera center and frame using the input 4X4 transformation matrix */
    void transform(const kjb::Matrix & M);

    // Stores the camera center in world coordinates
    Vector camera_center_in_world_coordinates;
};



}

#endif
