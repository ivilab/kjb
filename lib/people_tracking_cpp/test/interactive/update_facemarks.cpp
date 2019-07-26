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
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: update_facemarks.cpp 19002 2015-05-05 14:23:57Z jguan1 $ */

/**
 * @file Test update_facemarks function
 */

#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_face_2d.h>
#include <people_tracking_cpp/pt_scene.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_filesystem.h>
#include <detector_cpp/d_deva_facemark.h>
#include <st_cpp/st_perspective_camera.h>
#include <m_cpp/m_vector_d.h>

#include <vector>
#include <string> 

#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/assign.hpp>

using namespace kjb;
using namespace kjb::pt;
using namespace std;

const boost::array<Vector3, 7> COLOR_ORDER_ = boost::assign::list_of
    (Vector3(0.0, 0.0, 1.0))
    (Vector3(0.0, 0.5, 0.0))
    (Vector3(1.0, 0.0, 0.0))
    (Vector3(0.0, 0.75, 0.75))
    (Vector3(0.75, 0.0, 0.75))
    (Vector3(0.75, 0.75, 0.0))
    (Vector3(0.25, 0.25, 0.25));

using namespace kjb;
using namespace kjb::pt;
using namespace std;

void draw_point
(
    Image& img, 
    const Vector& pt,
    const Image::Pixel_type& pix,
    int pw
);

void draw_facemark
(
    Image& img, 
    const Deva_facemark& face, 
    const Image::Pixel_type& pix
);

void draw_face
(   
    Image& img, 
    const Face_2d& face, 
    const Image::Pixel_type& pix
); 

void draw_point
(
    Image& img, 
    const Vector& pt,
    const Image::Pixel_type& pix,
    int pw
)
{
    assert(pt.size() == 2);
    img.draw_point(pt(1), pt(0), pw, pix);
}

void draw_facemark
(
    Image& img, 
    const Deva_facemark& face, 
    const Image::Pixel_type& pix
)
{
    Deva_facemark dfm(face);
    double imw = img.get_num_cols();
    double imh = img.get_num_rows();
    unstandardize(dfm, imw, imh);
    const Vector& lem = dfm.left_eye_mark();
    const Vector& rem = dfm.right_eye_mark();
    const Vector& nm = dfm.nose_mark();
    const Vector& lmm = dfm.left_mouth_mark();
    const Vector& rmm = dfm.right_mouth_mark();
    const int px = 2;
    if(!lem.empty()) draw_point(img, lem, pix, px);
    if(!rem.empty()) draw_point(img, rem, pix, px);
    if(!nm.empty()) draw_point(img, nm, pix, px);
    if(!lmm.empty()) draw_point(img, lmm, pix, px);
    if(!rmm.empty()) draw_point(img, rmm, pix, px);
}

void draw_face
(   
    Image& img, 
    const Face_2d& face, 
    const Image::Pixel_type& pix
)
{
    double imw = img.get_num_cols();
    double imh = img.get_num_rows();
    Bbox fbox(face.bbox);
    unstandardize(fbox, imw, imh);

    // draw face box
    img.draw_aa_rectangle_outline(fbox.get_top(), fbox.get_left(),
                          fbox.get_bottom(), fbox.get_right(),
                          pix);
    // draw face features
    Vector le = face.left_eye;
    Vector re = face.right_eye;
    Vector lm = face.left_mouth;
    Vector rm = face.right_mouth;
    Vector ns = face.nose;

    unstandardize(le, imw, imh);
    unstandardize(re, imw, imh);
    unstandardize(lm, imw, imh);
    unstandardize(rm, imw, imh);
    unstandardize(ns, imw, imh);

    img.draw_point(le[1], le[0], 1, pix);
    img.draw_point(re[1], re[0], 1, pix);
    img.draw_point(lm[1], lm[0], 1, pix);
    img.draw_point(rm[1], rm[0], 1, pix);
    img.draw_point(ns[1], ns[0], 1, pix);

    // draw facemark
    if(face.facemark)
    {
        draw_facemark(img, *face.facemark, pix);
    }
}

int main(int argc, char** argv)
{
#ifdef TEST
    kjb_c::kjb_init();
    //kjb_c::kjb_l_set("heap-checking", "off");
    //kjb_c::kjb_l_set("initialization-checking", "off");
#endif
    if(argc != 5)
    {
        cout << "Usage: " << argv[0] 
             << " movie-dp scene-dp detector-dir out-dir\n";
        return EXIT_SUCCESS;
    }
    string movie_dp = argv[1];
    string scene_dp = argv[2];
    string detector_str = argv[3];
    string outdir = argv[4];

    // frames 
    vector<string> frame_imgs
        = file_names_from_format(movie_dp + "/frames/%05d.jpg");

    // Image dimension
    Image temp_img(frame_imgs[0]);
    assert(frame_imgs.size() > 0);
    int img_width = temp_img.get_num_cols();
    int img_height = temp_img.get_num_rows();

    // read data boxes
    Box_data data(img_width, img_height, 1.0);
    data.read(file_names_from_format(
        movie_dp + "/detection_boxes/" + detector_str + "/%05d.txt"));

    // scene
    Scene scene(Ascn(data), Perspective_camera(), 1.0, 1.0, 1.0);
    read_scene(scene, scene_dp);

    // parse facemark detections in standardized coordinate
    vector<string> face_fps 
         = file_names_from_format(movie_dp + "/features/deva_face/%05d.txt");

    vector<vector<Deva_facemark> > all_faces 
         = parse_deva_facemarks(face_fps, img_width, img_height);

    //vector<vector<const Deva_facemark*> > noise_faces 
    update_facemarks(scene.association, all_faces);

    kjb_c::kjb_mkdir(outdir.c_str());
    boost::format out_img_fmt(outdir + "/%05d.jpg");

    // visualize the results
    size_t num_frames = frame_imgs.size();
    for(size_t i = 0; i < num_frames; i++)
    {
        Image img(frame_imgs[i]);
        size_t tj = 0;
        BOOST_FOREACH(const Target& tg, scene.association)
        {
            const Face_2d_trajectory& ftraj = tg.face_trajectory();
            if(ftraj.at(i))
            {
                 // draw faces
                Image::Pixel_type pix = {255.0, 255.0, 255.0};
                pix.r = COLOR_ORDER_[tj % 7][0] * pix.r;
                pix.g = COLOR_ORDER_[tj % 7][1] * pix.g;
                pix.b = COLOR_ORDER_[tj % 7][2] * pix.b;

                draw_face(img, ftraj.at(i)->value, pix);
            }
            tj++;
        }
        // draw noise faces
        //const vector<const Deva_facemark*>& noise_face = noise_faces[i];
        /*BOOST_FOREACH(const Deva_facemark* face, noise_face)
        {
            Image::Pixel_type pix = {0.0, 0.0, 0.0};
            draw_facemark(img, *face, pix);
        }
        */
        string out_fp = (out_img_fmt % (i+1)).str();
        img.write(out_fp);
    }

    //std::cout << " num of noise_face: " << num_noise_faces << std::endl;

#ifdef TEST
    kjb_c::kjb_cleanup();
#endif
    return EXIT_SUCCESS;
}

