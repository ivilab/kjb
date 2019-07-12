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

/* $Id: d_facecom.h 18724 2015-03-28 21:22:30Z jguan1 $ */

#ifndef D_FACECOM_H
#define D_FACECOM_H

#include <detector_cpp/d_bbox.h>

#include <iosfwd>
#include <vector>
#include <string>

namespace kjb {

class Perspective_camera;
/**
 * @class   Face_detection
 *
 * Class representing a face detection with an orientation.
 */
class Face_detection 
{
public: 
    /** @brief  DOCUMENT LATER! */
    Face_detection
    (
        const Bbox& box,
        double pitch = 0.0, 
        double yaw = 0.0,
        double roll = 0.0
    ) :
        box_(box),
        pitch_(pitch),
        yaw_(yaw),
        roll_(roll)
    {}

    /** @brief  DOCUMENT LATER! */
    const Bbox& box() const { return box_; }

    /** @brief  DOCUMENT LATER! */
    Bbox& box() { return box_; }

    /** @brief  DOCUMENT LATER! */
    double pitch() const { return pitch_; }

    /** @brief  DOCUMENT LATER! */
    double yaw() const { return yaw_; }

    /** @brief  DOCUMENT LATER! */
    double roll() const { return roll_; }

    /** @brief  DOCUMENT LATER! */
    double& pitch() { return pitch_; }

    /** @brief  DOCUMENT LATER! */
    double& yaw() { return yaw_; }

    /** @brief  DOCUMENT LATER! */
    double& roll() { return roll_; }

private:
    Bbox box_;
    double pitch_;
    double yaw_;
    double roll_;
};

/**
 * @brief   Computes the direction of the gaze.
 *
 * @return  Vector representing the gaze direction of the given face
 *          detection using the given camera.
 */
Vector gaze_direction
(
    const Face_detection& face,
    const Perspective_camera& camera
);


/**
 * @brief   Computes the 3D location of a face.
 *
 * @return  Vector representing the 3D location the given face detection
 *          using the given camera.
 */
Vector face_location_3d
(
    const Face_detection& face,
    const Perspective_camera& camera,
    const Bbox& body_box
);


/**
 * @brief   Writes the face into ostream 
 *
 * @note    Pitch, yaw, and roll are in degrees to be consistent with the
 *          input files
 */
std::ostream& operator<<(std::ostream& ost, const Face_detection& face);

/**
 * @brief   Compares to boxes using middle of box. Needed because we
 *          have associated containers of these.
 */
bool operator<(const Face_detection& f1, const Face_detection& f2);


/**
 * @brief   Parse a line of a file (in face.com format) into a Face_Detection.
 */
Face_detection parse_face_line(const std::string& line);


/** @brief  Read set of faces from file (in face.com format). */
std::vector<Face_detection> read_face_file(std::istream& is);

/**
 * @brief   Parse the output of the faces from multiple files
 *          into a vector of vector of Face_detection
 */
std::vector<std::vector<Face_detection> > read_face_files
(
    const std::vector<std::string>& face_fps
);

} // namespace kjb

#endif /*D_FACECOM_H */

