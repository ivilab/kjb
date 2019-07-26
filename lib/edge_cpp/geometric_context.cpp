/*
*@class - Comparable_omap.cpp
*
*@author - Josh Bowdish
*
*@brief - creates an omap from an image that can call compare_omap(compareto)
*   to see how similar it is.
*/

#include "edge_cpp/geometric_context.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_int_matrix.h"

using namespace kjb;

void Geometric_context::draw_geometric_context(Image & img) const
{
    img = Image::create_zero_image(seg_map.get_num_rows(), seg_map.get_num_cols());
    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            int segment = ((int)seg_map(i, j)) - 1;
            int index = 0;
            for(unsigned int k = 1; k < 7; k++)
            {
                if( k == OBJECT_2)
                {
                    continue;
                }
                if(probabilities(segment, index) < probabilities(segment, k))
                {
                    index = k;
                }
            }
            double pp = probabilities(segment, index);
            switch(index)
            {
                case FLOOR:
                    img(i, j, 0) = 255.0*pp;
                    break;
                case RIGHT:
                    img(i, j, 2) = 255.0*pp;
                    break;
                case CENTRAL:
                    img(i, j, 1) = 255.0*pp;
                    break;
                case LEFT:
                    img(i, j, 0) = 255.0*pp;
                    img(i, j, 1) = 255.0*pp;
                    break;
                case OBJECT_1:
                    img(i, j, 0) = 255.0*pp;
                    img(i, j, 1) = 255.0*pp;
                    img(i, j, 2) = 255.0*pp;
                    break;
                case OBJECT_2:
                    img(i, j, 0) = 255.0*pp;
                    img(i, j, 1) = 255.0*pp;
                    img(i, j, 2) = 255.0*pp;
                    break;
                case CEILING:
                    img(i, j, 1) = 255.0*pp;
                    img(i, j, 2) = 255.0*pp;
                    break;
                break;
            }
        }
    }
}

double Geometric_context::compute_score(const kjb::Image & img) const
{
    assert(img.get_num_rows() == seg_map.get_num_rows());
    assert(img.get_num_cols() == seg_map.get_num_cols());
    double score = 0.0;
    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            int segment = ((int)seg_map(i, j)) - 1;
            if( img(i, j, 0) > 200)
            {
                //RED CHANNEL is set
                if( img(i, j, 1) > 200)
                {
                    // YELLOW -> LEFT
                    score += probabilities(segment, LEFT );
                }
                else if( img(i, j, 2) > 2)
                {
                    //RED AND BLUE SET
                    continue;
                }
                else
                {
                    //ONLY RED -> FLOOR
                    score += probabilities(segment, FLOOR );
                }

            }
            else if( img(i, j, 1) > 200)
            {
                //GREEN CHANNEL IS SET
                if( img(i, j, 2) > 200)
                {
                    // LIGHT BLUE -> CEILING
                    score += probabilities(segment, CEILING );
                }
                else
                {
                    //ONLY GREEN -> CENTRAL
                    score += probabilities(segment, CENTRAL );
                }
            }
            else if( (img(i, j, 2) > 200))
            {
                //ONLY BLUE CHANNEL IS SET -> RIGHT
                score += probabilities(segment, RIGHT );
            }
            else
            {
                score += probabilities(segment, OBJECT_1 );
            }
        }
    }
    score /= (img.get_num_rows()*img.get_num_cols());
    return score;
}

double Geometric_context::compute_score(const kjb::Int_matrix & map) const
{
    assert(map.get_num_rows() == seg_map.get_num_rows());
    assert(map.get_num_cols() == seg_map.get_num_cols());
    double score = 0.0;
    for(unsigned int i = 0; i < map.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < map.get_num_cols(); j++)
        {
            int current = map(i, j);
            int entity = current&0x7f;
            int use_room_surface = current&0x80;
            int segment = ((int)seg_map(i, j)) - 1;

            if( (entity != 0) && (use_room_surface == 0))
            {
                //This is an object
                score += probabilities(segment, OBJECT_1 );
            }
            else
            {
                int surface = current>>11;
                surface = surface&0x1f;
                score += probabilities(segment, surface);
            }
        }
    }

    score /= (map.get_num_rows()*map.get_num_cols());
    return score;
}


double Geometric_context::compute_score
(
    const kjb::Int_matrix & map,
    kjb::Vector & individual_scores,
    int num_entities
) const
{
    assert(map.get_num_rows() == seg_map.get_num_rows());
    assert(map.get_num_cols() == seg_map.get_num_cols());
    kjb::Int_vector counters(num_entities, 0);
    double score = 0.0;
    for(unsigned int i = 0; i < map.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < map.get_num_cols(); j++)
        {
            int current = map(i, j);
            int entity = current&0x7f;
            int use_room_surface = current&0x80;
            int segment = ((int)seg_map(i, j)) - 1;

            if( (entity != 0) && (use_room_surface == 0))
            {
                //This is an object
                score += probabilities(segment, OBJECT_1 );
                entity = entity -1;
                assert(entity < num_entities);
                counters[entity]++;
                individual_scores[entity] +=  probabilities(segment, OBJECT_1 );
            }
            else
            {
                int surface = current>>11;
                surface = surface&0x1f;
                score += probabilities(segment, surface);
                /*if(entity != 0)
                {
                    entity = entity -1;
                    assert(entity < num_entities);
                    counters[entity]++;
                    individual_scores[entity] +=  1 - probabilities(segment, OBJECT_1 );
                }*/
            }
        }
    }

    for(int i = 0; i < num_entities; i++)
    {
        if(counters[i] > 0)
        {
            individual_scores[i] /= (double)(counters[i]);
        }
    }

    score /= (map.get_num_rows()*map.get_num_cols());
    return score;
}

void Geometric_context::count_segments()
{
    num_segments = 0;
    for(unsigned int i = 0; i < seg_map.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < seg_map.get_num_cols(); j++)
        {
            int current_segment = seg_map(i, j);
            if(current_segment > num_segments)
            {
                num_segments = current_segment;
            }
        }
    }
}
void Geometric_context::draw_segmentation(Image & img) const
{
    for(unsigned int i = 0; i < (img.get_num_rows()-1); i++)
    {
        for(unsigned int j = 0; j < (img.get_num_cols()-1); j++)
        {
            int current_segment = seg_map(i,j);
            int right_segment = seg_map(i, j+1);
            int down_segment = seg_map(i+1, j);
            if( (current_segment != right_segment) || (current_segment != down_segment))
            {
                img(i,j, 0) = 255.0;
                img(i,j, 1) = 0.0;
                img(i,j, 2) = 0.0;
            }
        }
    }
}

void Geometric_context::draw_segment(Image & img, int segment_index) const
{
    for(unsigned int i = 0; i < img.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < img.get_num_cols(); j++)
        {
            int current_segment = seg_map(i,j);
            if(current_segment == segment_index)
            {
                img(i,j, 0) = 255.0;
                img(i,j, 1) = 0.0;
                img(i,j, 2) = 0.0;
            }
        }
    }
}

void Geometric_context::init_context()
{
    for(unsigned int i = 0; i < probabilities.get_num_rows(); i++)
    {
        probabilities(i, OBJECT_1) += probabilities(i, OBJECT_2);
        probabilities(i, OBJECT_2) = 0.0;
    }
    probabilities.scale_matrix_rows_by_sums();
    for(unsigned int i = 0; i < probabilities.get_num_rows(); i++)
    {
        double sum = 0.0;
        for(int k = 0; k < 7; k++)
        {
            sum += probabilities(i, k);
        }
        if(fabs(sum - 1.0) > DBL_EPSILON)
        {
            for(int k = 0; k < 7; k++)
            {
                probabilities(i, k);
            }
        }
    }
    count_segments();
}

void Geometric_context::convert_map_to_image
(
    const Int_matrix & imap,
    Image & img
)
{
    //img.create_zero_image(imap.get_num_rows(), imap.get_num_cols());
    for(unsigned int i = 0; i < imap.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < imap.get_num_cols(); j++)
        {
            int current = imap(i, j);
            int entity = current&0x7f;
            int use_room_surface = current&0x80;
            if((entity != 0) && (use_room_surface == 0))
            {

                img(i, j, 0) = 0.0;
                img(i, j, 1) = 0.0;
                img(i, j, 2) = 0.0;
                continue;
            }

            int surface = current>>11;
            surface = surface&0x1f;
            switch(surface)
            {
                case CENTRAL:
                    img(i, j, 0) = 0.0;
                    img(i, j, 1) = 255.0;
                    img(i, j, 2) = 0.0;
                    break;
                case LEFT:
                    img(i, j, 0) = 255.0;
                    img(i, j, 1) = 255.0;
                    img(i, j, 2) = 0.0;
                    break;
                case RIGHT:
                    img(i, j, 0) = 0.0;
                    img(i, j, 1) = 0.0;
                    img(i, j, 2) = 255.0;
                    break;
                case FLOOR:
                    img(i, j, 0) = 255.0;
                    img(i, j, 1) = 0.0;
                    img(i, j, 2) = 0.0;
                    break;
                case CEILING:
                    img(i, j, 0) = 0.0;
                    img(i, j, 1) = 255.0;
                    img(i, j, 2) = 255.0;
                    break;
            }

        }
    }
}

void Geometric_context::convert_map_to_edges
(
    const Int_matrix & imap,
    Image & img
)
{
    //img.create_zero_image(imap.get_num_rows(), imap.get_num_cols());
    for(unsigned int i = 0; i < imap.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < imap.get_num_cols(); j++)
        {
            int current = imap(i, j);
            int entity = current>>16;
            entity = entity&0xffff;
            if(entity > 0)
            {
                img(i, j, 0) = 255.0;
                img(i, j, 1) = 0.0;
                img(i, j, 2) = 0.0;
            }
            else
            {
                img(i, j, 0) = 0.0;
                img(i, j, 1) = 0.0;
                img(i, j, 2) = 0.0;
            }

        }
    }
}
