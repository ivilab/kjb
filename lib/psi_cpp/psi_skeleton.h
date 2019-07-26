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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: psi_skeleton.h 18278 2014-11-25 01:42:10Z ksimek $ */


#ifndef PSI_HUMAN_BODY_PARTS_H
#define PSI_HUMAN_BODY_PARTS_H

#include <vector>
#include <iostream>

#include <camera_cpp/camera_backproject.h>
//#include <psi_cpp/psi_util.h>
#include <psi_cpp/psi_bbox.h>
#include <people_tracking_cpp/pt_util.h>

#include <l_cpp/l_exception.h>

#include <string>
namespace kjb
{
namespace psi
{

enum Body_part_entity {BODY, HEAD, CHEST, LEFT_ELBOW, LEFT_HAND, LEFT_KNEE, LEFT_FOOT, RIGHT_ELBOW, RIGHT_HAND, RIGHT_KNEE, RIGHT_FOOT};

/**
 * @brief Body parts
 */
struct Psi_body_part
{
    Psi_body_part()
        : box(Bbox()),
          lost(1),
          occluded(1),
          label("NONE")
    {}

    Psi_body_part
    (
        const Bbox& box_,
        int lost_, 
        int occluded_, 
        const std::string& label_
    ) : box(box_),
        lost(lost_),
        occluded(occluded_),
        label(label_)
    {}

    /**
     * @brief Get the face position in 3D based on the body 2D bounding box
     * Note: Since the face_box and body_box are not standardized, we need to 
     *       pass in the image width and height
     */
    inline
    Vector get_3d_location
    (
        const Perspective_camera& camera,
        const Bbox& body_box,
        size_t image_width, 
        size_t image_height
    ) const
    {
        Bbox body(body_box);
        Bbox part(box);
        pt::standardize(body, image_width, image_height);
        pt::standardize(part, image_width, image_height);
        
        Vector body_bottom(body.get_bottom_center());
        body_bottom[0] = part.get_center()[0];
        double part_height_3d = get_3d_height(body_bottom, part.get_center(), camera);

        Ground_back_projector back_project(camera, 0.0);
        Vector pos_3d = back_project(body_bottom[0], body_bottom[1]);
        if(pos_3d.empty())
            return pos_3d;
        Vector part_pos_3d(pos_3d);
        part_pos_3d[1] = part_height_3d;

        return part_pos_3d;
    }

    Bbox box;
    int lost;
    int occluded;
    std::string label;

};

/**
 * @brief Skeleton class, a vector of body parts
 */
class Psi_skeleton : public std::vector<Psi_body_part>
{
private: 
    typedef std::vector<Psi_body_part> Base;
    friend std::ostream& operator<<(std::ostream& ost, const Psi_skeleton& skeleton);
    friend std::istream& operator<<(std::istream& ist, const Psi_skeleton& skeleton);

public:
    Psi_skeleton() : 
        Base(11)
    {}

    ~Psi_skeleton(){}

    void set_body_part(Body_part_entity body_part, const Psi_body_part& part)
    {
        (*this)[body_part] = part;
    }

    const Psi_body_part& get_body_part(const std::string& label) const
    {
        Body_part_entity body_part = get_body_part_entity(label);
        return (*this)[body_part];
    }
    
    Body_part_entity get_body_part_entity(const std::string& label) const;
    
    void set_body_part(const std::string& label, const Psi_body_part& body_part);
             
};

/** @brief writes part into out stream */
inline
std::ostream& operator<<(std::ostream& ost, const Psi_body_part& part)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();
    ost << std::scientific;

    const Bbox& box = part.box;
    Vector offset(box.get_width()/2.0, box.get_height()/2.0);
    Vector tl = box.get_center() - offset;
    Vector br = box.get_center() + offset;

    ost << part.label << " "<< part.lost << " "
        << part.occluded << " " << tl << " " << br;

    ost.width( w );
    ost.precision( p );
    ost.flags( f );

    return ost;
}

/** @brief read in Psi_body_part from stream  */
/*inline
std::istream& operator>>(const std::istream& ist, Psi_body_part& part)
{
    ist >> part.label;
    ist >> part.lost;
    ist >> part.occluded; 
    ist >> part.box;
}
*/

/** @brief writes skeleton into out stream */
std::ostream& operator<<(std::ostream& out, const Psi_skeleton& skeleton);

/** @brief read skeleton from stream */
//std::istream& operator>>(const std::istream& in, Psi_skeleton& skeleton);

std::vector<Psi_skeleton> parse_skeleton(std::istream& ist);

}// namespace psi
}// namespace kjb

#endif /*PSI_HUMAN_SKELETON_H */
