/* $Id: model_edge.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Luca Del Pero
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "likelihood_cpp/model_edge.h"
#include <sstream>
#include <iostream>

using namespace kjb;
Model_edge::Model_edge(double x1, double y1, double x2, double y2, bool isilhouette, bool isvisible, bool iflagged)
{
    _degenerate = false;
    if( (x1 == x2) && (y1 == y2))
    {
        /** In case this is a degenerate segment, we add a little perturbation
         * to one coordinate so that line_segment will not cause any exception.
         * This is sort of a hack, but this is such a rare case that we can
         * afford it
         */
        y2 = y2 + FLT_EPSILON;
        _degenerate = true;
        std::cout << "Degenerate!" << std::endl;
    }
    init_from_end_points(x1, y1, x2, y2);
    _silhouette = isilhouette;
    _visible = isvisible;
    _flagged = iflagged;
}

/** @brief Reads this Model segment from an input stream. */
void Model_edge::read(std::istream& in)
{
    Line_segment::read(in);

    using std::istringstream;

    const char* field_value;

    // Model segment silhouette indicator
    if (!(field_value = read_field_value(in, "silhouette")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Line segment Silhouette indicator");
    }
    istringstream ist(field_value);
    ist >> _silhouette;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment Silhouette indicator");
    }
    ist.clear(std::ios_base::goodbit);

    // Model segment visibility indicator
    if (!(field_value = read_field_value(in, "visible")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Line segment Visibility indicator");
    }
    ist.str(field_value);
    ist >> _visible;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment Visibility indicator");
    }
    ist.clear(std::ios_base::goodbit);
}

/** @brief Writes this Model segment to an output stream. */
void Model_edge::write(std::ostream& out) const
{
    Line_segment::write(out);
    out << " silhouette: " << _silhouette << '\n';
    out << " visible: " << _visible << '\n';
}


/** @brief Update the detectd parts when projecting an image segment onto the 
 *   model edge
 *  @return the overlapped length between teh image_edge_segment and the 
 *  already detected parts 
 *  TODO: need to think of a more efficient way to check the overlapping parts
 */
void Model_edge::update_detected_segments
(
    const Edge_segment& image_edge_segment, 
    double              collinear_distance_threshold,
    Vector&             previous_center,
    bool&               mapped,
    double              noisy_length
)
{
    noisy_length = 0.0;
//    std::cout<<"Existing detected segments: "<<std::endl;
//    for (unsigned int i = 0; i < detected_segments.size(); i++)
//    {
//        std::cout<<detected_segments[i].get_start()<<" "<<detected_segments[i].get_end()<<std::endl;
//    }

    //Step 1: Project the ending point of the image_edge_segment to this model edge
    std::pair<Vector, Vector> projected_points;
    double length_inside = 0.0;
    double length_outside = 0.0;
    

    Line_segment::project_line_segment_onto_line_segment(image_edge_segment, *this,
            projected_points, length_inside, length_outside);
    
//    std::cout<<"projected_points.first: "<<projected_points.first<<std::endl;
//    std::cout<<"Model start: "<<get_start()<<std::endl;
//    std::cout<<"projected_points.second: "<<projected_points.second<<std::endl;
//    std::cout<<"Model end: "<<get_end()<<std::endl;
     
    //Step 2: Check whether the projected points are all outside the model segments
    if(Line_segment::less_than(projected_points.second, get_start())
        || Line_segment::less_than(get_end(), projected_points.first))
    {
//        std::cout<<"both points outside starting"<<std::endl; 
        noisy_length += (projected_points.first-projected_points.second).magnitude();
    }
    else
    {
        bool overlapped = false;
        std::vector<bool> to_remove_indexes(detected_segments.size(), false);
        Vector new_start(3); 
        Vector new_end(3);
        
        if(Line_segment::less_than(projected_points.first, get_start()))
        {
            if(detected_segments.size() == 0)
            {
                //a new mapping, update previous_center
                previous_center = image_edge_segment.get_centre();
            }

            if ((image_edge_segment.get_centre()-previous_center).magnitude() < collinear_distance_threshold)
            {
//                std::cout<<"first point is outside starting point"<<std::endl;
                noisy_length += (projected_points.first-get_start()).magnitude(); 
                new_start = get_start();
                unsigned int num_overlapped = 0;

                //Check the ending point 
                for(unsigned int i = 0; i < detected_segments.size(); i++)
                {
                    if(Line_segment::less_than(projected_points.second, detected_segments[i].get_start()))
                    {
                        new_end = projected_points.second;
                        break;
                    }
                    else
                    {
                        to_remove_indexes[i] = true;
                        overlapped = true;                
                        if(Line_segment::less_than(projected_points.second, detected_segments[i].get_end()))
                        {
                            new_end = detected_segments[i].get_end();
                            noisy_length += (projected_points.second-detected_segments[i].get_start()).magnitude();
                            break;
                        }
                        else
                        {
                            noisy_length += detected_segments[i].get_length();
                            num_overlapped ++;
                        }
                    }
                }
                if(num_overlapped == detected_segments.size())
                {
                    if(Line_segment::less_than(projected_points.second, get_end()))
                    {
                        new_end = projected_points.second;
                    }
                    else
                    {
                        new_end = get_end();
                        noisy_length += (projected_points.second-get_end()).magnitude();
                    }
                }
                //remove the segments 
                if(overlapped)
                {
                    std::vector<Line_segment> old_segments = detected_segments;
                    detected_segments.clear();
                    for(unsigned int k = 0; k < detected_segments.size(); k++)
                    {
                        if(!to_remove_indexes[k])
                        {
                           detected_segments.push_back(old_segments[k]);
                        }
                    }
                }
                detected_segments.push_back(Line_segment(new_start, new_end));
                std::sort(detected_segments.begin(), detected_segments.end());
            } 
            else // not collinear, count as noisy edges
            {
                noisy_length += image_edge_segment.get_length();
            }
         }
         else //Projected point is inside the model segment 
         {
             if(detected_segments.size() == 0)
             {
                 //update the previous_center 
                 previous_center = image_edge_segment.get_centre();
             }
            
             if((image_edge_segment.get_centre()-previous_center).magnitude() < collinear_distance_threshold)
             {
                 new_start = projected_points.first;
                 //Check the starting point  
                 for (unsigned int i = 0; i < detected_segments.size(); i++)
                 {
//                     std::cout<<"Inside the model segment: "<<std::endl;
                     if(Line_segment::less_than(projected_points.first, detected_segments[i].get_start()))
                     {
                         if((image_edge_segment.get_centre()-previous_center).magnitude() < collinear_distance_threshold)
                         {   
                             new_start = projected_points.first;
                             unsigned int num_overlapped_segments = 0;
                             for (unsigned int j = i; j < detected_segments.size(); j++)
                             {
                                 if(Line_segment::less_than(projected_points.second, detected_segments[j].get_start()))
                                 {
                                     new_end = projected_points.second;
                                     break;
                                 }
                                 else 
                                 {
                                     overlapped = true;
                                     to_remove_indexes[j] = true;
                                     if( Line_segment::less_than(projected_points.second, detected_segments[j].get_end()))
                                     {
                                         noisy_length += (detected_segments[j].get_start() - projected_points.second).magnitude();
                                         new_end = detected_segments[j].get_end();
                                         break;
                                     }
                                     else
                                     {
                                         noisy_length += detected_segments[j].get_length();
                                         num_overlapped_segments++;
                                     }
                                 }
                             }
                             
                             if(num_overlapped_segments == detected_segments.size()-i)
                             {
                                 if(Line_segment::less_than(projected_points.second, get_end()))
                                 {
                                     new_end = projected_points.second;
                                 }
                                 else
                                 {
                                     new_end = get_end();
                                     noisy_length += (projected_points.second - get_end()).magnitude();
                                 }
                             }
                             
                             if(overlapped) //remove the overlapped detected segments
                             {
                                 std::vector<Line_segment> old_segments = detected_segments;
                                 detected_segments.clear();
                                 for(unsigned int k = 0; k < detected_segments.size(); k++)
                                 {
                                     if(!to_remove_indexes[k])
                                     {
                                         detected_segments.push_back(old_segments[k]);
                                     }
                                 }
                             }
                             detected_segments.push_back(Line_segment(new_start, new_end));
                             std::sort(detected_segments.begin(), detected_segments.end());
                             break;
                         }
                         else  //not collinear
                         {
                             noisy_length += image_edge_segment.get_length();
                             break;
                         }
                     }
                     else if(Line_segment::less_than(projected_points.first, detected_segments[i].get_end()))
                     {
                         if(Line_segment::less_than(projected_points.second, detected_segments[i].get_end()))
                         {
                             noisy_length += (projected_points.first-projected_points.second).magnitude();
                             break;
                         }
                         
                         if((image_edge_segment.get_centre()-previous_center).magnitude() >= collinear_distance_threshold)
                         {
                             noisy_length += image_edge_segment.get_length();
                             break;
                         }
                         else  
                         {
                             new_start = detected_segments[i].get_start(); 
                             noisy_length += (projected_points.first - detected_segments[i].get_end()).magnitude();
                             unsigned int overlapped_segments = 0; 

                             for (unsigned int j = i+1; j < detected_segments.size(); j++)
                             {
                                 to_remove_indexes[j] = true; 
                                 if(Line_segment::less_than(projected_points.second, detected_segments[j].get_start()))
                                 {
                                     new_end = projected_points.second;
                                     break;
                                 }
                                 else if(Line_segment::less_than(detected_segments[j].get_start(), projected_points.second))
                                 {
                                     noisy_length += (detected_segments[j].get_start()-projected_points.second).magnitude();
                                     new_end = detected_segments[j].get_end();
                                     break;
                                 }
                                 else
                                 {
                                     noisy_length += detected_segments[j].get_length();
                                     overlapped_segments ++;
                                 }
                             }
                             if(overlapped_segments == detected_segments.size()-(i+1))
                             {
                                 if(Line_segment::less_than(projected_points.second, get_end()))
                                 {
                                     new_end = projected_points.second;
                                 }
                                 else
                                 {
                                     new_end = get_end();
                                     noisy_length += (projected_points.second - get_end()).magnitude();
                                 }
                             }
                             // remove overlapped detected segments
                             std::vector<Line_segment> old_segments = detected_segments;
                             detected_segments.clear();
                             for(unsigned int j = 0; j < detected_segments.size(); j++)
                             {
                                 if(!to_remove_indexes[j])
                                     detected_segments.push_back(old_segments[j]);
                             }
                             detected_segments.push_back(Line_segment(new_start, new_end));
                             std::sort(detected_segments.begin(), detected_segments.end());
                         }
                     }
                 }
                 if(detected_segments.size()==0)
                 {
                     //need to check whether the second point of the porjected point is outside the model segment
                     if(Line_segment::less_than(projected_points.second, get_end()))
                     {
                          new_end = projected_points.second;
                     }
                     else
                     {
                          new_end = get_end();
                          noisy_length += (projected_points.second-get_end()).magnitude();
                     }
                     detected_segments.push_back(Line_segment(new_start, new_end));
                     std::sort(detected_segments.begin(), detected_segments.end());
                 }
            }
            else
            {
                noisy_length += image_edge_segment.get_length();
            }
        }
    }
//    std::cout<<"noisy length: "<<noisy_length <<std::endl;
    double projected_length = (projected_points.first - projected_points.second).magnitude();
//    std::cout<<"projected_length: "<<projected_length <<" noisy_length: "<<noisy_length<< std::endl;
//    ASSERT(projected_length - noisy_length > 0.0);
    if((projected_length - noisy_length) > 1)
        mapped = true;
}

/** @brief Return the detected length of this model segment */
double Model_edge::get_detected_length() 
{
//    std::cout<<"Model edge: "<<get_start() <<" "<<get_end()<<std::endl; 
//    std::cout<<"Model length: "<<get_length()<<std::endl; 
    double length = 0.0;
    for (unsigned int i = 0; i < detected_segments.size(); i++)
    {
//        std::cout<<i<<"'s detected_segments: "<<detected_segments[i].get_start()<<"**"<<detected_segments[i].get_end()<<std::endl;
        ASSERT(detected_segments[i].get_start_x() >= get_start_x() && detected_segments[i].get_end_x() <= get_end_x());
        length += detected_segments[i].get_length();
    }
//    std::cout<<"Detected length: "<<length<<std::endl;
    return length;
}
