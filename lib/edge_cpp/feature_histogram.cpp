/*
*@class - Feature_histogram.cpp
*
*@author - Luca Del Pero
*
*@brief - creates an omap from an image that can call compare_omap(compareto)
*   to see how similar it is.
*/

#include "edge_cpp/feature_histogram.h"
#include "l_cpp/l_int_matrix.h"
#include <iostream>

using namespace kjb;

#define WALL_TO_DIFFERENT_WALL_WEIGHT 1.0
#define WALL_TO_SAME_WALL_WEIGHT 1.0
#define FLOOR_TO_FLOOR_WEIGHT 1.0
#define CEILING_TO_CEILING_WEIGHT 1.0
#define WALL_TO_FLOOR 1.0
#define WALL_TO_CEILING 1.0
#define FLOOR_TO_CEILING 1.0
#define ROOM_TO_OBJECT 1.0
#define OBJECT_TO_DIFFERENT_OBJECT 1.0
#define OBJECT_TO_SAME_OBJECT 1.0

Feature_histogram::Feature_histogram
(
    DTLib::CImg<DTLib::FloatCHistogramPtr> & histo,
    unsigned int dart_window_size,
    unsigned int num_rows,
    unsigned int num_cols,
    int padding
)
{
    if( (dart_window_size == 0) || ((dart_window_size%2) == 0) )
    {
        KJB_THROW_2(Illegal_argument, "Dart window size must be an odd integer larger than 0");
    }
    _num_rows = num_rows;
    _num_cols = num_cols;
    _padding = padding;
    int row_increment = dart_window_size/2.0;
    int column_increment = row_increment;
    for(int i = (row_increment + _padding); i < (_num_rows - _padding); i += dart_window_size)
    {
        std::cout << i << " ";
        for(int j = (column_increment + padding); j < (_num_cols - _padding); j += dart_window_size)
        {
            darts.push_back(Image_dart(i, j));
        }
    }
    diff_matrix.zero_out(darts.size(), darts.size());
    differential_matrix.zero_out(darts.size(), darts.size());
    prepare_diff_matrix(histo);
}

void Feature_histogram::draw_darts(kjb::Image & img)
{
    img = Image::create_initialized_image(_num_rows, _num_cols, 0.0, 0.0, 0.0);
    for(unsigned int i = 0; i < darts.size(); i++)
    {
        img(darts[i]._row, darts[i]._col, 0) = 255.0;
    }
}

void Feature_histogram::prepare_diff_matrix(DTLib::CImg<DTLib::FloatCHistogramPtr> & histo)
{
    all_different = 0.0;
    for(unsigned int i = 0; i < darts.size(); i++)
    {
        int position_i = ( (darts[i]._row - _padding)*histo.Width()) + (darts[i]._col - _padding);
        DTLib::FloatCHistogramPtr histo_i = histo.GetPix(position_i);
        for(unsigned int j = (i + 1); j < darts.size(); j++)
        {
            int position_j = ( (darts[j]._row - _padding)*histo.Width()) + (darts[j]._col - _padding);
            DTLib::FloatCHistogramPtr histo_j = histo.GetPix(position_j);
            diff_matrix(i, j) = histo_i->ChiSquareCompare(histo_j);
            differential_matrix(i, j) = 1.0 - 2*(diff_matrix(i, j));
            all_different += diff_matrix(i, j);
        }
    }
}

double Feature_histogram::compute_differential
(
    const kjb::Matrix & assignment_matrix
) const
{
    double the_differential = all_different;
    for(unsigned int i = 0; i < assignment_matrix.get_num_rows(); i++)
    {
        for(unsigned int j = 1; j < assignment_matrix(i, 0); j++)
        {
            for(unsigned k = j+1; k <= assignment_matrix(i, 0); k++)
            {
                the_differential += differential_matrix(assignment_matrix(i,j), assignment_matrix(i,k));
            }
        }
    }

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();
    return the_differential/((double)num_elements);
}

double Feature_histogram::compute_score
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & previous_darts,
    std::vector<Fh_type> & new_darts,
    std::vector<bool> changed_darts,
    double previous_score
)
{
    unsigned int i = 0;
    unsigned int j = 0;

    assert(previous_darts.size() == darts.size());
    assert(changed_darts.size() == darts.size());

    for(i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
        if( (new_darts[i].element_1 == previous_darts[i].element_1) &&
            (new_darts[i].element_2 == previous_darts[i].element_2) &&
            (new_darts[i].element_3 == previous_darts[i].element_3) )
        {
            changed_darts[i] = false;
        }
        else
        {
            changed_darts[i] = true;
        }
    }

    for(i = 0; i < darts.size(); i++)
    {
        if(changed_darts[i])
        {
            for(j = (i + 1); j < darts.size(); j++)
            {
                //adjust the score of i-j
                previous_score += diff_matrix(i, j)*
                                    (find_weight(new_darts[i], new_darts[j]) - find_weight(previous_darts[i], previous_darts[j]));
            }
            for(j = 0; j < i; j++)
            {
                if(!changed_darts[i])
                {
                    //adjust the score of j-i
                    previous_score += diff_matrix(j, i)*
                                        (find_weight(new_darts[i], new_darts[j]) - find_weight(previous_darts[i], previous_darts[j]));
                }
            }
        }
    }

    return previous_score;
}

double Feature_histogram::compute_score
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    new_darts.resize(darts.size(), Fh_type());

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        for(j = (i + 1); j < darts.size(); j++)
        {
            //adjust the score of i-j
            score += diff_matrix(i, j)*find_weight(new_darts[i], new_darts[j]) ;
        }
    }

    return score;
}

double Feature_histogram::find_weight(const Fh_type & dart1, const Fh_type & dart2)
{
    if(dart1.element_1 == dart2.element_1)
    {
        if(dart1.element_1 == 0)
        {
            //Two darts belonging to the room
            if(dart1.element_3 == dart2.element_3)
            {
                //Two darts belonging to the same face of the room
                if(dart1.element_3 == 2)
                {
                    //Both darts belong to the ceiling
                    return CEILING_TO_CEILING_WEIGHT;
                }
                else if(dart1.element_3 == 3)
                {
                    //Both darts belong to the floor
                    return FLOOR_TO_FLOOR_WEIGHT;
                }
                else
                {
                    //Both darts belong to the same wall
                    return WALL_TO_SAME_WALL_WEIGHT;
                }
            }
            else
            {
                //Two darts belonging to a different face of the room
                //Now differentiate between wall-wall and wall-floor/ceiling
                if(dart1.element_3 == 2 )
                {
                    //The ith dart belongs to the ceiling
                    if(dart2.element_3 == 3)
                    {
                        //The jth dart belongs to the floor
                        return FLOOR_TO_CEILING;
                    }
                    else
                    {
                        //The jth dart belongs to a wall
                        return WALL_TO_CEILING;
                    }
                }
                else if(dart1.element_3 == 3 )
                {
                    //The ith dart belongs to the floor
                    if(dart2.element_3 == 2)
                    {
                        //The jth dart belongs to the ceiling
                        return FLOOR_TO_CEILING;
                    }
                    else
                    {
                        //The jth dart belongs to a wall
                        return WALL_TO_FLOOR;
                    }
                }
                else
                {
                    //The ith dart belongs to a wall
                    if(dart2.element_3 == 2)
                    {
                        //The jth dart belongs to the ceiling
                        return WALL_TO_CEILING;
                    }
                    else if(dart2.element_3 == 3)
                    {
                        //The jth dart belongs to the floor
                        return WALL_TO_FLOOR;
                    }
                    else
                    {
                        //Both the ith and the jth dart belong to a wall, but a different one
                        return WALL_TO_DIFFERENT_WALL_WEIGHT;
                    }
                }
            }
        }
        else
        {
            //Two darts belonging to the same objects
            return OBJECT_TO_SAME_OBJECT;
        }
    }
    else
    {
        if( (dart1.element_1 == 0) || (dart2.element_2 == 0))
        {
            // room dart and object dart
            return ROOM_TO_OBJECT;
        }
        else
        {
            // darts belong to two different objects
            return OBJECT_TO_DIFFERENT_OBJECT;
        }
    }

    return 1.0;
}


double Feature_histogram::find_probability(const Fh_type & dart1, const Fh_type & dart2, double diff)
{
    bool same = false;
    if( (dart1.element_1 == dart2.element_1) )
    {
        if(dart1.element_1 == 0)
        {
            //Both darts belong to the room
            if(dart1.element_3 != dart2.element_3)
            {
                //Different room faces
                //dart2 belongs to the floor
                if( ((dart1.element_3 < 2) || (dart1.element_3 > 3))
                    && ((dart2.element_3 < 2) || (dart2.element_3 > 3)))
                {
                    //Both darts belong to a wall
                    //SAME
                     same = true;
                }
            }
            else
            {
                //SAME
                same = true;
            }
            /*if(dart1.element_3 == dart2.element_3)
            {
                //SAME
                same = true;
            }*/
        }
        else
        {
            //Both darts belong to the same object
            //SAME
            same = true;
        }
    }
    if(same)
    {
        return (1.0 - diff);
    }
    else
    {
        return diff;
    }
}

bool Feature_histogram::find_probability2(const Fh_type & dart1, const Fh_type & dart2, double diff, double & prob)
{
    bool same = false;
    if( (dart1.element_1 == dart2.element_1) )
    {
        if(dart1.element_1 == 0)
        {
            //Both darts belong to the room
            if(dart1.element_3 != dart2.element_3)
            {
                //Different room faces
                //dart2 belongs to the floor
                if( ((dart1.element_3 < 2) || (dart1.element_3 > 3))
                    && ((dart2.element_3 < 2) || (dart2.element_3 > 3)))
                {
                    //Both darts belong to a wall
                    //SAME
                     same = true;
                }
            }
            else
            {
                //SAME
                same = true;
            }
            /*if(dart1.element_3 == dart2.element_3)
            {
                //SAME
                same = true;
            }*/
        }
        else
        {
            //Both darts belong to the same object
            //SAME
            same = true;
        }
    }
    if(same)
    {
        prob = (1.0 - diff);
    }
    else
    {
        prob = diff;
    }
    return same;
}

bool Feature_histogram::find_probability3(const Fh_type & dart1, const Fh_type & dart2, double diff, double & prob)
{
    bool same = false;
    if( (dart1.element_1 == dart2.element_1) )
    {
        if(dart1.element_1 == 0)
        {
            //Both darts belong to the room
            if(dart1.element_3 == dart2.element_3)
            {
                same = true;
            }
        }
        else
        {
            //Both darts belong to the same object
            //SAME
            same = true;
        }
    }
    if(same)
    {
        prob = (1.0 - diff);
    }
    else
    {
        prob = diff;
    }
    return same;
}


double Feature_histogram::compute_score2
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & previous_darts,
    std::vector<Fh_type> & new_darts,
    std::vector<bool> changed_darts,
    double previous_score
)
{
    unsigned int i = 0;
    unsigned int j = 0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();
    previous_score *= (double)num_elements;

    assert(previous_darts.size() == darts.size());
    assert(changed_darts.size() == darts.size());

    for(i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
        if( (new_darts[i].element_1 == previous_darts[i].element_1) &&
            (new_darts[i].element_2 == previous_darts[i].element_2) &&
            (new_darts[i].element_3 == previous_darts[i].element_3) )
        {
            changed_darts[i] = false;
        }
        else
        {
            changed_darts[i] = true;
        }
    }

    for(i = 0; i < darts.size(); i++)
    {
        if(changed_darts[i])
        {
            for(j = (i + 1); j < darts.size(); j++)
            {
                //adjust the score of i-j
                previous_score += find_probability(new_darts[i], new_darts[j],diff_matrix(i, j)) -
                                  find_probability(previous_darts[i], previous_darts[j],diff_matrix(i, j));
            }
            for(j = 0; j < i; j++)
            {
                if(!changed_darts[j])
                {
                    //adjust the score of j-i
                    previous_score += find_probability(new_darts[i], new_darts[j],diff_matrix(j, i)) -
                              find_probability(previous_darts[i], previous_darts[j],diff_matrix(j, i));
                }
            }
        }
    }

    return previous_score/((double)num_elements);
}

double Feature_histogram::compute_score2
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        for(j = (i + 1); j < darts.size(); j++)
        {
            //adjust the score of i-j
            score += find_probability(new_darts[i], new_darts[j],diff_matrix(i, j)) ;
        }
    }

    return score/((double)num_elements);
}

double Feature_histogram::compute_score3
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = 0;

    new_darts.resize(darts.size(), Fh_type());

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    double prob;

    for(i = 0; i < darts.size(); i++)
    {
        for(j = (i + 1); j < darts.size(); j++)
        {
            //adjust the score of i-j

            if(find_probability2(new_darts[i], new_darts[j],diff_matrix(i, j), prob))
            {
                score += prob;
                num_elements++;
            }
        }
    }

    return score/((double)num_elements);
}

double Feature_histogram::compute_score4
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = 0;

    new_darts.resize(darts.size(), Fh_type());

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    double prob;

    for(i = 0; i < darts.size(); i++)
    {
        for(j = (i + 1); j < darts.size(); j++)
        {
            //adjust the score of i-j

            if(find_probability3(new_darts[i], new_darts[j],diff_matrix(i, j), prob))
            {
                score += prob;
                num_elements++;
            }
        }
    }

    return score/((double)num_elements);
}

void Feature_histogram::draw_room
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    kjb::Image & img
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if(new_darts[i].element_1 == 0)
        {
            img(darts[i]._row, darts[i]._col, 0) = 255.0;
        }
    }

}

void Feature_histogram::draw_object
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    unsigned int obj_index,
    kjb::Image & img
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if(new_darts[i].element_1 == (obj_index + 1))
        {
            img(darts[i]._row, darts[i]._col, 0) = 255.0;
        }
    }
}


void Feature_histogram::draw_object_polymesh
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    unsigned int obj_index,
    unsigned int polymesh_index,
    kjb::Image & img
) const
{
    unsigned int i = 0;
    unsigned int j = 0;

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if(new_darts[i].element_1 == (obj_index + 1))
        {
            if(new_darts[1].element_2 == (polymesh_index))
            {
        	    img(darts[i]._row, darts[i]._col, 0) = 255.0;
            }
        }
    }
}

void Feature_histogram::draw_room_floor
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    kjb::Image & img
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if( (new_darts[i].element_1 == 0) && (new_darts[i].element_3 == 3))
        {
            img(darts[i]._row, darts[i]._col, 0) = 255.0;
        }
    }

}

void Feature_histogram::draw_room_ceiling
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    kjb::Image & img
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if( (new_darts[i].element_1 == 0) && (new_darts[i].element_3 == 2))
        {
            img(darts[i]._row, darts[i]._col, 0) = 255.0;
        }
    }

}

void Feature_histogram::draw_room_walls
(
    const kjb::Int_matrix & map,
    std::vector<Fh_type> & new_darts,
    kjb::Image & img
)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double score = 0.0;

    int num_elements = (darts.size()*darts.size())/2.0 - darts.size();

    new_darts.resize(darts.size(), Fh_type());

    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            img(i, j, 0) = 0.0;
            img(i, j, 1) = 0.0;
            img(i, j, 2) = 0.0;
        }
    }

    for(int i = 0; i < darts.size(); i++)
    {
        new_darts[i].element_1 = map(darts[i]._row, darts[i]._col)&0x7f;
        new_darts[i].element_2 = (map(darts[i]._row, darts[i]._col)>>11)&0x1f;
        new_darts[i].element_3 = (map(darts[i]._row, darts[i]._col)>>8)&0x07;
    }

    for(i = 0; i < darts.size(); i++)
    {
        if( (new_darts[i].element_1 == 0) && (new_darts[i].element_3 != 2) && (new_darts[i].element_3 != 3))
        {
            img(darts[i]._row, darts[i]._col, 0) = 255.0;
        }
    }

}
