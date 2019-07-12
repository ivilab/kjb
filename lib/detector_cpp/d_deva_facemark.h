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

/* $Id: d_deva_facemark.h 19991 2015-10-29 19:50:11Z jguan1 $ */

#ifndef D_DEVA_FACEMARK_H
#define D_DEVA_FACEMARK_H

#include <m_cpp/m_vector.h>

#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

namespace kjb
{
class Deva_facemark
{
public:
    Deva_facemark
    (
        const std::vector<Vector>& parts,
        double yaw,
        double score = -DBL_MAX
    );

    // const getters
    const std::vector<Vector>& left_eye() const { return left_eye_; }
    const std::vector<Vector>& right_eye() const { return right_eye_; }
    const std::vector<Vector>& nose() const { return nose_; }
    const std::vector<Vector>& mouth() const { return mouth_; }
    const std::vector<Vector>& chin() const { return chin_; }
    double yaw() const { return yaw_; }
    double score() const { return score_; }

    // non-const getters
    std::vector<Vector>& left_eye() { return left_eye_; }
    std::vector<Vector>& right_eye() { return right_eye_; }
    std::vector<Vector>& nose() { return nose_; }
    std::vector<Vector>& mouth() { return mouth_; }
    std::vector<Vector>& chin() { return chin_; }
    double& yaw() { return yaw_; }
    double& score() { return score_; }

    /** @brief   Return the mark of the left eye */
    const Vector& left_eye_mark() const { return left_eye_mark_; }
    Vector& left_eye_mark() { return left_eye_mark_; }

    /** @brief   Return the mark of the right eye */
    const Vector& right_eye_mark() const { return right_eye_mark_; }
    Vector& right_eye_mark() { return right_eye_mark_; }

    /** @brief   Return the mark of the tip of the nose */
    const Vector& nose_mark() const { return nose_mark_; }
    Vector& nose_mark() { return nose_mark_; }

    /** @brief   Return the left corner of the mouth */
    const Vector& left_mouth_mark() const { return left_mouth_mark_; }
    Vector& left_mouth_mark() { return left_mouth_mark_; }

    /** @brief   Return the right corner of the mouth */
    const Vector& right_mouth_mark() const { return right_mouth_mark_; }
    Vector& right_mouth_mark() { return right_mouth_mark_; }

private:
    void compute_marks();
        
private:
    std::vector<Vector> left_eye_;
    std::vector<Vector> right_eye_;
    std::vector<Vector> mouth_;
    std::vector<Vector> nose_;
    std::vector<Vector> chin_;
    double yaw_;
    double score_;
    Vector left_eye_mark_;
    Vector right_eye_mark_;
    Vector nose_mark_;
    Vector left_mouth_mark_;
    Vector right_mouth_mark_;

}; //class Deva_facemark

/**
 * @brief   Parse the output from Deva's face detector 
 *          into a vector of Deva_facemark
 */
std::vector<Deva_facemark> parse_deva_facemark
(
    std::istream& is
);

/**
 * @brief   Parse the output from Deva's face detector 
 */
Deva_facemark parse_deva_facemark_line
(
    const std::string& line
);

/**
 * @brief   Write the output from Deva's face detector 
 *          into a vector of Deva_facemark
 */
void write_deva_facemark
(
    const std::vector<Deva_facemark>& faces,
    std::ostream& os
);

/**
 * @brief   Write the output from Deva's face detector 
 */
void write_deva_facemark_line
(
    const Deva_facemark& face,
    std::ostream& os
);

} //namepsace kjb

#endif
