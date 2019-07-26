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

/* $Id: d_facecom.cpp 21296 2017-03-07 06:00:29Z jguan1 $ */

#include <detector_cpp/d_facecom.h>

#include <g_cpp/g_camera.h>
#include <camera_cpp/perspective_camera.h>
#include <camera_cpp/camera_backproject.h>
#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l_cpp/l_util.h>
#include <istream>
#include <ostream>
#include <sstream>
#include <iterator>

#include <ios>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace kjb;

std::vector<Face_detection> kjb::read_face_file
(
    std::istream& is
)
{
    std::string line;
    std::vector<Face_detection> faces;
    while(std::getline(is, line))
    {
        Face_detection fd = parse_face_line(line);
        if(fd.box().get_width() != 0.0 && fd.box().get_height() != 0.0)
        {
            faces.push_back(parse_face_line(line));
        }
    }

    return faces;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

std::vector<std::vector<Face_detection> > kjb::read_face_files
(
    const std::vector<std::string>& face_fps
)
{
    std::vector<std::vector<Face_detection> > faces;
    for(unsigned i = 0; i < face_fps.size(); i++)
    {
        std::ifstream face_ifs(face_fps[i].c_str());
        if(face_ifs.fail())
        {
            KJB_THROW_3(IO_error, "Could not open file: %s",
                        (face_fps[i].c_str()));
        }

        faces.push_back(read_face_file(face_ifs));
    }

    return faces;
}

Vector kjb::gaze_direction
(
    const Face_detection& face,
    const Perspective_camera& camera
)
{
    const double pitch = face.pitch();
    const double yaw = face.yaw();
    const double roll = face.roll();

    Quaternion face_orientation(-pitch, yaw, -roll, Quaternion::XYZR);
    Vector gaze_dir(3, 0.0);
    gaze_dir(2) = 1.0;
    gaze_dir = face_orientation.rotate(gaze_dir);
    Quaternion camera_orientation = camera.get_orientation();
    gaze_dir = camera_orientation.conj().rotate(gaze_dir);

    return gaze_dir;
}

Vector kjb::face_location_3d
(
    const Face_detection& face,
    const Perspective_camera& camera,
    const Bbox& body_box
)
{
    Vector body_bottom(body_box.get_bottom_center());
    body_bottom[0] = face.box().get_center()[0];
    double face_height_3d = get_3d_height(
                                    body_bottom,
                                    face.box().get_center(),
                                    camera);

    Ground_back_projector back_project(camera, 0.0);
    Vector pos_3d = back_project(body_bottom[0], body_bottom[1]);
    pos_3d[1] = face_height_3d;
    return pos_3d;
}

std::ostream& operator<<(std::ostream& ost, const Face_detection& face)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();
    ost << std::scientific;
   
    ost << face.box() << " " 
        << std::setw(16)<< std::setprecision(8)
        << face.pitch()*180.0/M_PI
        << std::setw(16)<< std::setprecision(8)
        << face.yaw()*180.0/M_PI
        << std::setw(16)<< std::setprecision(8)
        << face.roll()*180.0/M_PI;

    ost.width(w);
    ost.precision(p);
    ost.flags(f);

    return ost;
}

bool operator<(const Face_detection& f1, const Face_detection& f2)
{
    if(f1.box().get_top_center() < f2.box().get_top_center())
    {
        return true;
    }

    if(f1.box().get_top_center() > f2.box().get_top_center())
    {
        return false;
    }

    return f1.box().get_bottom_center() < f2.box().get_bottom_center();
}

Face_detection kjb::parse_face_line
(
    const std::string& line 
)
{
    using namespace std;

    istringstream istr(line);
    vector<double> elems;
    const size_t NUM_FIELDS = 7;
    copy(istream_iterator<double>(istr), istream_iterator<double>(),
         back_inserter(elems));

    KJB(ASSERT(elems.size() == NUM_FIELDS || elems.size() == NUM_FIELDS + 1));
    IFT((elems.size() == NUM_FIELDS || elems.size() == NUM_FIELDS + 1),
        Runtime_error,
        "Cannot read face trajectory element: line has wrong format.");

    size_t i = 0;
    double x = elems[i++];
    double y = elems[i++];
    double width = elems[i++];
    double height = elems[i++];
    double yaw = elems[i++]*M_PI/180.0;
    double pitch = elems[i++]*M_PI/180.0;
    double roll = elems[i++]*M_PI/180.0;
    Bbox face_box(Vector(x, y), width, height);

    return Face_detection(face_box, pitch, yaw, roll);
}

