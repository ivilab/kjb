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

/* $Id: d_deva_facemark.cpp 21296 2017-03-07 06:00:29Z jguan1 $ */

#include <m_cpp/m_vector.h>
#include <detector_cpp/d_deva_facemark.h>
#include <l_cpp/l_exception.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <fstream>
#include <iostream>

using namespace kjb;

void Deva_facemark::compute_marks()
{
    // compute left eye mark
    size_t start_index = 5;
    Vector mark(2, 0.0);
    size_t N = 0;
    for(size_t i = start_index; i < left_eye_.size(); i++)
    {
        mark += left_eye_[i];
        N++;
    }
    if(N > 0)
    {
        left_eye_mark_ = mark/N;
    }

    // compute right eye mark
    mark.set(0.0, 0.0);
    N = 0;
    for(size_t i = start_index; i < right_eye_.size(); i++)
    {
        mark += right_eye_[i];
        N++;
    }
    if(N > 0)
    {
        right_eye_mark_ = mark/N;
    }

    // compute nose mark
    mark.set(0.0, 0.0);
    start_index = 4;
    N = 0;
    for(size_t i = start_index; i < nose_.size(); i++)
    {
        if(!nose_[i].empty())
        {
            mark += nose_[i];
            N++;
        }
    }
    if(N > 0)
    {
        nose_mark_ = mark/N;
    }

    // compute left mouth mark
    if(yaw_ < 60) left_mouth_mark_ = mouth_[10];

    // compute right mouth mark
    if(mouth_.size() == 20)
    {
        right_mouth_mark_ = mouth_[16];
    }
    else
    {
        if(yaw_ >= 60)
        {
            right_mouth_mark_ = mouth_[10];
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

//Vector Deva_facemark::left_eye_mark() const
//{
//    if(left_eye_.empty()) return Vector();
//
//    const size_t start_index = 5;
//    Vector mark(2, 0.0);
//    size_t N = 0;
//    for(size_t i = start_index; i < left_eye_.size(); i++)
//    {
//        mark += left_eye_[i];
//        N++;
//    }
//    return mark/N;
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//Vector Deva_facemark::right_eye_mark() const
//{
//    if(right_eye_.empty()) return Vector();
//
//    const size_t start_index = 5;
//    Vector mark(2, 0.0);
//    size_t N = 0;
//    for(size_t i = start_index; i < right_eye_.size(); i++)
//    {
//        mark += right_eye_[i];
//        N++;
//    }
//    return mark/N;
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//Vector Deva_facemark::nose_mark() const
//{
//    const size_t start_index = 4;
//    Vector mark(2, 0.0);
//    size_t N = 0;
//    for(size_t i = start_index; i < nose_.size(); i++)
//    {
//        mark += nose_[i];
//        N++;
//    }
//    return mark/N;
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//Vector Deva_facemark::left_mouth_mark() const
//{
//    if(yaw_ < 60) return mouth_[10];
//    return Vector();
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//Vector Deva_facemark::right_mouth_mark() const
//{
//    if(mouth_.size() == 20)
//    {
//        return mouth_[16];
//    }
//    else
//    {
//        if(yaw_ >= 60)
//        {
//            return mouth_[10];
//        }
//        else
//        {
//            return Vector();
//        }
//    }
//
//}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Deva_facemark::Deva_facemark
(
    const std::vector<Vector>& parts,
    double yaw,
    double score
) : yaw_(yaw),
    score_(score)
{
    // profile face
    if(yaw_ >= 60 || yaw <= -60)
    {
        /*IFT(parts.size() == 39, Illegal_argument,
                "profile face must have 39 parts");*/
        for(size_t i = 0; i < parts.size(); i++)
        {
            if(i <= 10)
            {
                chin_.push_back(parts[i]);
            }
            else if(i <= 23)
            {
                mouth_.push_back(parts[i]);
            }
            else if(i <= 32)
            {
                if(yaw >= 60)
                {
                    right_eye_.push_back(parts[i]);
                }
                else // yaw <= -60
                {
                    left_eye_.push_back(parts[i]);
                }
            }
            else
            {
                nose_.push_back(parts[i]);
            }
        }
    }
    else  // frontal face
    {
        IFT(parts.size() == 68, Illegal_argument,
                "frontal face must have 68 parts");
        for(size_t i = 0; i < parts.size(); i++)
        {
            if(i <= 16)
            {
                chin_.push_back(parts[i]);
            }
            else if(i <= 36)
            {
                mouth_.push_back(parts[i]);
            }
            else if(i <= 47)
            {
                left_eye_.push_back(parts[i]);
            }
            else if(i <= 58)
            {
                right_eye_.push_back(parts[i]);
            }
            else
            {
                nose_.push_back(parts[i]);
            }
        }
    }

    // compute the face marks
    compute_marks();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Deva_facemark> kjb::parse_deva_facemark
(
    std::istream& is
)
{
    std::vector<Deva_facemark> facemarks;
    std::string line;

    while(std::getline(is, line))
    {
        Deva_facemark cur_facemark = parse_deva_facemark_line(line);
        facemarks.push_back(cur_facemark);
    }

    return facemarks;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Deva_facemark kjb::parse_deva_facemark_line
(
    const std::string& line
)
{
    std::string cur_line(line);

    // split line on whitespace
    std::vector<std::string> tokens;
    boost::trim(cur_line);
    boost::split(tokens, cur_line,
            boost::is_any_of("\t \n\r"), boost::token_compress_on);
    const size_t N_1 = 39;
    const size_t N_2 = 68;
    size_t num_tokens = tokens.size();
    assert(num_tokens == N_1*2 + 2 || num_tokens == N_2*2 + 2 ||
           num_tokens == N_1*2 + 3 || num_tokens == N_2*2 + 3);
    std::vector<Vector> parts;
    size_t score_token = 0;
    double score = -DBL_MAX;
    if(num_tokens == N_1*2 + 3 || num_tokens == N_2*2 + 3)
    {
        score_token = 1;
    }
    for(size_t i = 1; i < tokens.size() - 1 - score_token; i = i+2)
    {
        double x = boost::lexical_cast<double>(tokens[i]);
        double y = boost::lexical_cast<double>(tokens[i+1]);
        parts.push_back(Vector(x, y));
    }
    if(score_token == 1)
    {
        score = boost::lexical_cast<double>(tokens[num_tokens-2]);
    }
    double yaw = boost::lexical_cast<double>(tokens.back());
    return Deva_facemark(parts, yaw, score);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::write_deva_facemark
(
    const std::vector<Deva_facemark>& faces,
    std::ostream& os
)
{
    BOOST_FOREACH(const Deva_facemark& face, faces)
    {
        write_deva_facemark_line(face, os);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::write_deva_facemark_line
(
    const Deva_facemark& face,
    std::ostream& os
)
{
    const std::vector<Vector>& le = face.left_eye();
    const std::vector<Vector>& re = face.right_eye();
    const std::vector<Vector>& no = face.nose();
    const std::vector<Vector>& mo = face.mouth();
    const std::vector<Vector>& ch = face.chin();
    os << " 1 ";

    std::copy(ch.begin(), ch.end(), std::ostream_iterator<Vector>(os, "\t"));
    std::copy(mo.begin(), mo.end(), std::ostream_iterator<Vector>(os, "\t"));
    std::copy(le.begin(), le.end(), std::ostream_iterator<Vector>(os, "\t"));
    std::copy(re.begin(), re.end(), std::ostream_iterator<Vector>(os, "\t"));
    std::copy(no.begin(), no.end(), std::ostream_iterator<Vector>(os, "\t"));
    if(face.score() != -DBL_MAX) os << face.score() << "\t";
    os << face.yaw() << std::endl;
}

