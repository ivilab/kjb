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

/* $Id: pt_facemark_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "people_tracking_cpp/pt_facemark_likelihood.h"
#include "detector_cpp/d_deva_facemark.h"
#include "prob_cpp/prob_pdf.h"

using namespace kjb;
using namespace kjb::pt;

double Facemark_likelihood::operator()(const Scene& scene) const
{
    double ll = 0.0;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        ll += at_trajectory(tg);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Facemark_likelihood::at_trajectory(const Target& target) const
{
    const Face_2d_trajectory ftraj = target.face_trajectory();
    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();

    double ll = 0.0;
    for(size_t cur_frame = sf; cur_frame <= ef; cur_frame++)
    {
        const Face_2d& face_2d = ftraj[cur_frame - 1]->value;
        ll += at_face(face_2d);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Facemark_likelihood::at_face(const Face_2d& face_2d) const
{
    double cll = 0.0;

    if(face_2d.facemark != NULL)
    {
        const Vector& lem = face_2d.facemark->left_eye_mark();
        const Vector& rem = face_2d.facemark->right_eye_mark();
        const Vector& nm = face_2d.facemark->nose_mark();
        const Vector& lmm = face_2d.facemark->left_mouth_mark();
        const Vector& rmm = face_2d.facemark->right_mouth_mark();

        if(face_2d.visibility.visible >= 0.5)
        {
            cll += at_mark(lem, face_2d.left_eye, m_eye_x_dist, m_eye_y_dist);
            cll += at_mark(rem, face_2d.right_eye, m_eye_x_dist, m_eye_y_dist);
            cll += at_mark(nm, face_2d.nose, m_nose_x_dist, m_nose_y_dist);
            cll += at_mark(
                    lmm, face_2d.left_mouth, m_mouth_x_dist, m_mouth_y_dist); 
            cll += at_mark(
                    rmm, face_2d.right_mouth, m_mouth_x_dist, m_mouth_y_dist); 
        }
        else
        {
            cll += at_mark(lem, Vector(), m_eye_x_dist, m_eye_y_dist);
            cll += at_mark(rem, Vector(), m_eye_x_dist, m_eye_y_dist);
            cll += at_mark(nm, Vector(), m_nose_x_dist, m_nose_y_dist);
            cll += at_mark(lmm, Vector(), m_mouth_x_dist, m_mouth_y_dist); 
            cll += at_mark(rmm, Vector(), m_mouth_x_dist, m_mouth_y_dist); 
        }
    }

    return cll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Facemark_likelihood::at_mark
(
    const Vector& dmark,
    const Vector& fmark,
    const Gaussian_distribution& N_x, 
    const Gaussian_distribution& N_y
) const
{
    // when detection is missing
    if(dmark.empty()) return 0.0;

    // when model mark is invisible
    if(fmark.empty())
    {
        return single_noise();
    }

    double error_x = dmark[0] - fmark[0];
    double error_y = dmark[1] - fmark[1];
    return log_pdf(N_x, error_x) + log_pdf(N_y, error_y);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

size_t Facemark_likelihood::num_assigned_facemarks(const Scene& scene) const
{
    const Ascn& w = scene.association;
    size_t num_assigned_face = 0;
    BOOST_FOREACH(const Target& tg, w)
    {
        Face_2d_trajectory& ftraj = tg.face_trajectory();
//      size_t assigned = 0;
        for(size_t i = 0; i < ftraj.size(); i++)
        {
            if(ftraj[i] && ftraj[i]->value.facemark != NULL)
            {
                num_assigned_face++;
//              assigned++;
            }
        }
//      size_t start = tg.get_start_time();
//      size_t end = tg.get_end_time();
//      std::cout << "\t\t\t\t\t target [" << start << "-" << end << 
//                   "] has faces: " << assigned << " || ";
    }

//  std::cout << "\t\t\t\t\t total num_assigned_faces: " << num_assigned_face << std::endl;

//  int ns_faces = (int)m_total_facemarks - (int)num_assigned_face;
//  std::cout << "\t\t\t\t\t total num_noisy_faces: " << ns_faces << std::endl;
//  IFT(ns_faces >= 0, Runtime_error, 
//          "number of noisy faces must be nonnegative.");

    return num_assigned_face;
}

