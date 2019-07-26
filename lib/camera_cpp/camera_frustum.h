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
|     Luca Del Pero, Yuanliu Liu
|
* =========================================================================== */


/**
 * @file
 *
 * @author Yuanliu Liu
 *
 * @brief CAMERA_FRUSTUM for modeling a truncated cone
 *
 * The truncated cone is approximated by polymeshes. It is defined by the number of vertices of the polygon to approximate the circle,
 * the center of the cone, the radius of the bottom surface and the top surface and its height.
 */

#ifndef CAMERA_CPP_CAMERA_FRUSTUM_H_
#define CAMERA_CPP_CAMERA_FRUSTUM_H_

#include <gr_cpp/gr_parametric_frustum.h>
#include <g_cpp/g_quaternion.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_renderable_model.h>

#define CAMERA_FRUSTUM_PITCH 0
#define CAMERA_FRUSTUM_YAW 1
#define CAMERA_FRUSTUM_ROLL 2

#define CAMERA_FRUSTUM_WIDTH 0
#define CAMERA_FRUSTUM_LENGTH 1
#define CAMERA_FRUSTUM_TOP_RADIUS 2
#define CAMERA_FRUSTUM_HEIGHT 3

namespace kjb
{

class Perspective_camera;
class Manhattan_corner;

/**
 * @class Parametric_parapiped A Truncated Cone parametrized in terms of
 * the number of vertices of the polygon to approximate the circle,
 * the radius of the bottom surface and the top surface, the center of the cone, its height 
 * and rotation angles around its axes (pitch, yaw and roll using classic
 * Tait-Bryan formulation).
 *
 * It provides a rendering interface to the underlying rendering
 * representation. So far, only OpenGL is supported
 */
class Parametric_frustum : public Renderable_model, public Readable, public Writeable
{
public:

    Parametric_frustum( unsigned int inv = 6, double ix = 0.0, double iy = 0.0,
    double iz = 0.0, double iw = 1.0, double il = 1.0, double iratio_top_bottom
    = 1.0, double ih = 1.0,
                         double ipitch = 0.0, double iyaw = 0.0, double iroll = 0.0) throw(kjb::Illegal_argument);

    /** @brief Constructs a parametric_frustum from an input file. */
    Parametric_frustum(const char* fname) throw (kjb::Illegal_argument,
                kjb::IO_error);

    /** @brief Constructs a parametric_frustum from an input stream. */
    Parametric_frustum(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error);

    Parametric_frustum(const Parametric_frustum & src);
    Parametric_frustum & operator = (const Parametric_frustum & src);

    /** @brief Reads this Parametric_frustum from an input stream. */
    virtual void read(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error);

    /** @brief Reads this Parametric_frustum from a file */
    virtual void read(const char * fname) throw (kjb::IO_error,  kjb::Illegal_argument)
    {
        Readable::read(fname);
    }

    /** @brief Writes this Parametric_frustum to a file. */
    virtual void write(std::ostream& out) const
       throw (kjb::IO_error);

    /** @brief Writes this Parametric_frustum to an output stream. */
    virtual void write(const char * fname) const
       throw (kjb::IO_error)
    {
        Writeable::write(fname);
    }

    /** @brief returns the rendering interface used to render this Parametric_frustum */
    virtual Abstract_renderable & get_rendering_interface() const;

    /** @brief returns the polymesh used to render this Parametric_frustum*/
    Polymesh & get_polymesh() const
    {
        update_if_needed();
        return rendering_interface;
    }

    /** @brief updates the rendering representation so that it reflects
     * the current values of the parameters. This is not done
     * any time a parameter changes for efficiency reasons
     */
    virtual void update_rendering_representation() const throw(kjb::KJB_error);

    virtual Parametric_frustum * clone() const;
    virtual ~Parametric_frustum() { }

    /** @brief Returns the width of this parametric frustum */
    inline double get_width() const {return height; }
    /** @brief Returns the length of this parametric frustum */
    inline double get_length() const {return length; }
    /** @brief Returns the ratio of the top and bottom surface of this parametric frustum */
    inline double get_ratio_top_bottom() const {return ratio_top_bottom; }
    /** @brief Returns the height of this parametric frustum */
    inline double get_height() const {return height; }

    /** @brief Returns the pitch of this parametric frustum */
    inline double get_pitch() const {return rotation_angles(CAMERA_FRUSTUM_PITCH);}
    /** @brief Returns the yaw of this parametric frustum */
    inline double get_yaw() const {return rotation_angles(CAMERA_FRUSTUM_YAW);}
    /** @brief Returns the roll of this parametric frustum */
    inline double get_roll() const {return rotation_angles(CAMERA_FRUSTUM_ROLL);}

    /** @brief Returns the centre of this Parametric_frustum */
    inline const Vector & get_centre() const {return centre;}
    /** @brief Returns the x coordinate of the centre of this Parametric_frustum */
    inline double get_centre_x() const {return centre(0); }
    /** @brief Returns the x coordinate of the centre of this Parametric_frustum */
    inline double get_centre_y() const {return centre(1); }
    /** @brief Returns the x coordinate of the centre of this Parametric_frustum */
    inline double get_centre_z() const {return centre(2); }

    /** @brief Sets the width of the base of this Parametric_frustum */
    void set_width(double iwidth) throw(kjb::Illegal_argument);
	/** @brief Sets the length of the base of this Parametric_frustum */
    void set_length(double ilength)throw(kjb::Illegal_argument);
	/** @brief Sets the ratio between the top and the base of this Parametric_frustum */
    void set_ratio_top_bottom(double iratio_top_bottom)throw(kjb::Illegal_argument);
    /** @brief Sets the height of this Parametric_frustum */
    void set_height(double iheight) throw(kjb::Illegal_argument);

    /** @brief Sets the pitch of this Parametric_frustum */
    void set_pitch(double ip);
    /** @brief Sets the yaw of this Parametric_frustum */
    void set_yaw(double iy);
    /** @brief Sets the roll of this Parametric_frustum */
    void set_roll(double ir);

    /** @brief rotates the truncated cone around its x-axis */
    virtual void rotate_around_x_axis(double theta);

    /** @brief rotates the truncated cone around its x-axis */
    virtual void rotate_around_y_axis(double theta);

    /** @brief rotates the parapiped around its x-axis */
    virtual void rotate_around_z_axis(double theta);

    /** @brief rotates the parapiped around its x,y,z axes in this order */
    virtual void rotate_around_frustum_axes(double thetax, double thetay, double thetaz);

    /** @brief Sets the centre of this parametric_frustum */
    void set_centre(const kjb::Vector & icentre) throw(kjb::Illegal_argument);
    /** @brief Sets the x coordinate of the centre of this parametric_frustum */
    void set_centre_x(double ix) throw(kjb::Illegal_argument);
    /** @brief Sets the y coordinate of the centre of this parametric_frustum */
    void set_centre_y(double iy) throw(kjb::Illegal_argument);
    /** @brief Sets the z coordinate of the centre of this parametric_frustum */
    void set_centre_z(double iz) throw(kjb::Illegal_argument);

    /** @brief sets the rotation angles from an input quaternion */
    virtual void set_angles_from_quaternion(const kjb::Quaternion & q);

    /** @brief returns the rotations of this parapiped as a quaternion */
    inline const kjb::Quaternion & get_rotations_as_a_quaternion() const
    {
        update_rendering_representation();
        return rendering_interface.get_orientation();
    }

    /** Sets the rotation mode. This will have an impact on how
     * pitch, yaw and roll are interpreted (standard is Euler
     * Mode XYZ with rotating axes, see class Quaternion).
     * No matter what mode you specify here, pitch
     * will be interpreted as Euler angle 1, yaw as Euler angle 2,
     * and roll as Euler angle 3. This function (except for
     * standard mode XYZR) is not adequately tested
     */
    void set_rotation_mode(kjb::Quaternion::Euler_mode imode)
    {
        KJB(UNTESTED_CODE());
        rendering_interface.set_rotation_mode(imode);
    }

    void stretch_along_axis(unsigned int axis,  double amount, bool direction);

    /** @briefTransforms a point in world coordinates to a coordinate
     *  system where the parapiped centre is the origin, and the
     *  axes are defined by the parapiped axes */
    void get_point_in_parapiped_coordinates
    (
        const kjb::Vector & point_in_world_coordinates,
        kjb::Vector & point_in_parapiped_coordinates
    ) const;

    /** @brief Transforms a point in parapiped coordinates to
     *  world coordinates */
    void get_point_in_world_coordinates
    (
        const kjb::Vector & point_in_parapiped_coordinates,
        kjb::Vector & point_in_world_coordinates
    ) const;

/*    void draw_orientation_map() const;

    void draw_left_right_orientation_map() const;

    void draw_geometric_context_map() const;*/

    void get_lines(std::vector<Line3d> & lines);

    void get_vertices(std::vector<Vector> & vertices);

    static int get_num_rendering_interface_edges(int num_facets)
    {
        return Frustum::get_num_edges(num_facets);
    }

    static int get_rendering_interface_edge_indexes
    (
        std::vector<int> & base_edge_indexes,
        std::vector<int> & vertical_edge_indexes,
        std::vector<int> & top_edge_indexes,
        int num_facets
    )
    {
        return Frustum::get_edge_indexes(base_edge_indexes,
            vertical_edge_indexes, top_edge_indexes, num_facets);
    }

private:

    /** @brief Given 3 angles, computes the corresponding Euler angles given the
     *         curren Euler convention */
    void compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll);

    unsigned int num_vertices;
    //double radius_bottom;
    //double radius_top;
    double width;     // the diameter of the bottom ellipse along the x axis
    double length;    // the diameter of the bottom ellipse along the z axis
    double ratio_top_bottom;
    double height;

    kjb::Vector centre;

    /* The rotation angle around the object's x axis (pitch), the
     * object's y axis (yaw) and the object's z axis (roll). Stored
     * in this order */
    kjb::Vector rotation_angles;

    /**
     * The interface to the representation used for rendering this parametric_frustum
     */
    mutable kjb::Frustum rendering_interface;

};


}

#endif /* CAMERA_CPP_CAMERA_FRUSTUM_H_ */
