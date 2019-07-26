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
|     Luca Del Pero
|
* =========================================================================== */


/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief St_perspective_camera for modeling a perspective camera using the classic
 *         Forsyth and Ponce parametrization
 *
 * The Gl_perspective_camera is defined by a camera centre, three rotation angles (pitch,
 * yaw and roll), focal length, aspect ratio, principal point coordinates in the image plane,
 * skew angle.
 */

#ifndef ST_PARAPIPED_H_
#define ST_PARAPIPED_H_

#include <gr_cpp/gr_parapiped.h>
#include <g_cpp/g_quaternion.h>
#include <m_cpp/m_vector.h>
#include <st_cpp/st_renderable_model.h>
#include <gr_cpp/gr_parametric_frustum.h>

#define ST_PARAPIPED_PITCH 0
#define ST_PARAPIPED_YAW 1
#define ST_PARAPIPED_ROLL 2
#define ST_PARAPIPED_DISTANCE_FROM_THE_FLOOR 0.002

namespace kjb
{

class Perspective_camera;
class Manhattan_corner;
class Vanishing_point;

/**
 * @class Parametric_parapiped A Parapiped parametrized in terms of
 * width, height, length, position of the centre and rotation
 * angles around its axes (pitch, yaw and roll using classic
 * Tait-Bryan formulation).
 *
 * It provides a rendering interface to the underlying rendering
 * representation. So far, only OpenGL is supported
 */
class Parametric_parapiped : public Renderable_model, public Readable, public Writeable
{
public:

    Parametric_parapiped(double ix = 0.0, double iy = 0.0, double iz = 0.0, double iw = 1.0, double ih = 1.0, double il = 1.0,
                         double ipitch = 0.0, double iyaw = 0.0, double iroll = 0.0) throw(kjb::Illegal_argument);

    /** @brief Constructs a parametric_parapiped from an input file. */
    Parametric_parapiped(const char* fname) throw (kjb::Illegal_argument,
                kjb::IO_error);

    /** @brief Constructs a parametric_parapiped from an input stream. */
    Parametric_parapiped(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error);

    Parametric_parapiped(const Parametric_parapiped & src);
    virtual Parametric_parapiped & operator = (const Parametric_parapiped & src);

    /** @brief Reads this parametric_parapiped from an input stream. */
    virtual void read(std::istream& in) throw (kjb::Illegal_argument,
            kjb::IO_error);

    /** @brief Reads this parametric_parapiped from a file */
    virtual void read(const char * fname) throw (kjb::IO_error,  kjb::Illegal_argument)
    {
        Readable::read(fname);
    }

    /** @brief Writes this parametric_parapiped to a file. */
    virtual void write(std::ostream& out) const
       throw (kjb::IO_error);

    /** @brief Writes this parametric_parapiped to an output stream. */
    virtual void write(const char * fname) const
       throw (kjb::IO_error)
    {
        Writeable::write(fname);
    }

    /** @brief returns the rendering interface used to render this parametric_parapiped */
    virtual Abstract_renderable & get_rendering_interface() const;

    /** @brief returns the polymesh used to render this parametric_parapiped*/
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

    virtual Parametric_parapiped * clone() const;
    virtual ~Parametric_parapiped() { }

    /** @brief Returns the width of this parametric_parapiped */
    inline double get_width() const {return width; }
    /** @brief Returns the height of this parametric_parapiped */
    inline double get_height() const {return height; }
    /** @brief Returns the length of this parametric_parapiped */
    inline double get_length() const {return length; }

    /** @brief Returns the pitch of this parametric_parapiped */
    inline double get_pitch() const {return rotation_angles(ST_PARAPIPED_PITCH);}
    /** @brief Returns the yaw of this parametric_parapiped */
    inline double get_yaw() const {return rotation_angles(ST_PARAPIPED_YAW);}
    /** @brief Returns the roll of this parametric_parapiped */
    inline double get_roll() const {return rotation_angles(ST_PARAPIPED_ROLL);}

    /** @brief Returns the centre of this parametric_parapiped */
    inline const Vector & get_centre() const {return centre;}
    /** @brief Returns the x coordinate of the centre of this parametric_parapiped */
    inline double get_centre_x() const {return centre(0); }
    /** @brief Returns the x coordinate of the centre of this parametric_parapiped */
    inline double get_centre_y() const {return centre(1); }
    /** @brief Returns the x coordinate of the centre of this parametric_parapiped */
    inline double get_centre_z() const {return centre(2); }

    /** @brief Sets the width of this parametric_parapiped */
    void set_width(double iwidth) throw(kjb::Illegal_argument);
    /** @brief Sets the height of this parametric_parapiped */
    void set_height(double iheight) throw(kjb::Illegal_argument);
    /** @brief Sets the length of this parametric_parapiped */
    void set_length(double ilength)throw(kjb::Illegal_argument);

    /** @brief Sets the pitch of this parametric_parapiped */
    void set_pitch(double ip);
    /** @brief Sets the yaw of this parametric_parapiped */
    void set_yaw(double iy);
    /** @brief Sets the roll of this parametric_parapiped */
    void set_roll(double ir);

    /** @brief rotates the parapiped around its x-axis */
    virtual void rotate_around_x_axis(double theta);

    /** @brief rotates the parapiped around its x-axis */
    virtual void rotate_around_y_axis(double theta);

    /** @brief rotates the parapiped around its x-axis */
    virtual void rotate_around_z_axis(double theta);

    /** @brief rotates the parapiped around its x,y,z axes in this order */
    virtual void rotate_around_parapiped_axes(double thetax, double thetay, double thetaz);

    /** @brief Sets the centre of this parametric_parapiped */
    void set_centre(const kjb::Vector & icentre) throw(kjb::Illegal_argument);
    /** @brief Sets the x coordinate of the centre of this parametric_parapiped */
    void set_centre_x(double ix) throw(kjb::Illegal_argument);
    /** @brief Sets the y coordinate of the centre of this parametric_parapiped */
    void set_centre_y(double iy) throw(kjb::Illegal_argument);
    /** @brief Sets the z coordinate of the centre of this parametric_parapiped */
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

    void draw_orientation_map() const;

    void draw_left_right_orientation_map() const;

    void draw_CMU_orientation_map() const;

    void draw_geometric_context_map() const;

    void get_lines(std::vector<Line3d> & lines);

    void get_vertices(std::vector<Vector> & vertices);

    static int get_num_rendering_interface_edges()
    {
        return Parapiped::get_num_edges();
    }

    static void get_rendering_interface_edge_indexes
    (
        std::vector<int> & base_edge_indexes,
        std::vector<int> & vertical_edge_indexes,
        std::vector<int> & top_edge_indexes
    )
    {
        Parapiped::get_edge_indexes(base_edge_indexes,
                vertical_edge_indexes, top_edge_indexes);
    }

private:

    /** @brief Given 3 angles, computes the corresponding Euler angles given the
     *         curren Euler convention */
    void compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll);

    double width;
    double height;
    double length;

    kjb::Vector centre;

    /* The rotation angle around the object's x axis (pitch), the
     * object's y axis (yaw) and the object's z axis (roll). Stored
     * in this order */
    kjb::Vector rotation_angles;

    /**
     * The interface to the representation used for rendering this parametric_parapiped
     */
    mutable Parapiped rendering_interface;

};

double propose_parapiped_and_camera_from_orthogonal_corner_good
(
    kjb::Parametric_parapiped &pp,
    kjb::Perspective_camera & camera,
    const kjb::Manhattan_corner & corner,
    double focal_length,
    unsigned int num_rows,
    unsigned int num_cols,
    double corner_3D_z_position,
    double p_width_ratio,
    double p_height_ratio,
    double p_length_ratio
);

bool propose_parapiped_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room = true
);

bool propose_frame_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    unsigned int & face_number,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & idesired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols
);

bool propose_frame_inside_parapiped_from_orthogonal_corner_good
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    unsigned int & face_number,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & idesired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    bool wall_x,
    bool expand_to_the_ground
);

/** Propose supported parapiped inside parapiped from orthogonal_corner
 * The height of the support planes are chosen from feasible ranges */
bool propose_parapiped_at_different_depth_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room // if it is allowed to expand the room box 
);

bool propose_parapiped_onside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
);

bool propose_supported_parapiped_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    //kjb::Vector & expansion_deltas,
    //st::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    /*bool expand_right,
    bool expand_z_up,
    bool expand_towards_camera,*/
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
);

bool propose_parapiped_inside_parapiped_from_orthogonal_corner_against_wall
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room,
    bool expand_right,
    bool expand_z_up,
    bool expand_towards_camera
);

bool propose_supported_parapiped_inside_parapiped_from_three_corners
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    double desired_height,
    unsigned int num_rows,
    unsigned int num_cols,
    double base_height
);

bool propose_parapiped_inside_parapiped_from_three_corners_on_the_floor
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    double desired_height,
    unsigned int num_rows,
    unsigned int num_cols
);

bool propose_parapiped_inside_parapiped_from_two_corners_on_the_floor
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    double desired_height,
    double desired_other_dimension_ratio,
    bool direction,
    bool & is_diagonal,
    bool & is_dx,
    unsigned int num_rows,
    unsigned int num_cols
);

    bool propose_parapiped_onside_parapiped_from_one_corner_in_the_center
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
);

bool propose_parapiped_inside_parapiped_from_one_corner_in_the_centre_on_the_floor // one peg in the centre
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    double desired_height,
    double desired_width,
    double desired_length,
    bool & is_dx,
    unsigned int num_rows,
    unsigned int num_cols
);

//bool propose_frustum_inside_parapiped_from_bounding_box_on_the_supporter // eg., the table lamp
//(
//    kjb::Vector & centre,
//    kjb::Vector & dimensions,
//    kjb::Vector & expansion_deltas,
//    std::vector<bool> & expansion_directions,
//    Parametric_parapiped & new_pp,
//    const Parametric_parapiped &pp,
//    const Perspective_camera & camera,
//    const kjb::Vector & bottom_center, // bottom center of the bounding box in the image plane
//    const kjb::Vector & top_center,
//    const int supporter_height           // the height of the supporter relative to the room box
//);

void expand_towards_camera
(
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    unsigned int num_rows,
    unsigned int num_cols,
    bool & expand_tw_camera,
    bool & expand_right,
    bool & expand_z_up
);

double propose_parapiped_and_camera_from_vanishing_points
(
    kjb::Parametric_parapiped & pp,
    kjb::Perspective_camera & camera,
    const std::vector<Vanishing_point> & vpts,
    double focal_length,
    unsigned int num_rows,
    unsigned int num_cols
);

bool find_height_from_two_corners
(
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & floor_corner,
    const Manhattan_corner & top_corner,
    double & desired_height,
    unsigned int num_rows,
    unsigned int num_cols
);

bool shift_parapiped_to_match_corner
(
    kjb::Parametric_parapiped & pp,
    kjb::Perspective_camera & camera,
    unsigned int num_rows,
    unsigned int num_cols,
    const kjb::Vector & corner
);

bool propose_frame_inside_parapiped_from_three_corners_on_the_wall
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    unsigned int & face_number,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    unsigned int num_rows,
    unsigned int num_cols
);

bool backproject_point_to_plane // find out the intersection between the view line and a plane in room coordinate
(
    kjb::Vector & intersection,
    kjb::Vector point2D,
    kjb::Vector plane,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    int num_rows,
    int num_cols
);

void update_focal_with_position(Perspective_camera& cam, double ifocal, Parametric_parapiped & pp);


}

#endif
