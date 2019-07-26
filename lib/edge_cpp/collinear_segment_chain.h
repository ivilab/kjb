/* $Id */

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
|  Author: Jinyan Guan, Ernesto Brau
* =========================================================================== */

#ifndef COLLINEAR_SEGMENT_CHAIN_H_INCLUDED
#define COLLINEAR_SEGMENT_CHAIN_H_INCLUDED

#include <l_cpp/l_exception.h>
#include <edge_cpp/line_segment.h>
#include <algorithm>

namespace kjb {

/**
 * @class Collinear_segment_chain
 *
 * @brief Represent a collinear line segment, a collinear line segment is
 * inherited from an Line_segment 
 */
class Collinear_segment_chain: public  Line_segment 
{
public:
    
    /** 
     * @brief Constructs a new Collinear_segment_chain 
     */ 
    Collinear_segment_chain
    (
        const std::vector<Line_segment>& segments 
    );

    /** 
     * @brief Copy Constructor constructrs a new Collinear_segment_chain
     * from an existing Collinear_segment_chain 
     */ 
    Collinear_segment_chain(const Collinear_segment_chain& csc) :
        Line_segment(csc), m_segments(csc.m_segments)
    {}

    /** 
     * @brief Assignment operator Assigns a Collinear_segment_chain to 
     * another Collinear_segment_chain 
     */
    Collinear_segment_chain& operator=(const Collinear_segment_chain & csc)
    {
        if(&csc != this)
        { 
            Line_segment::operator=(csc); 
            m_segments = csc.m_segments; 
        }
        return (*this);
    }

    /**
     * @brief   Nothing.
     */
    ~Collinear_segment_chain() {}

    /**
     * @brief   Returns underlying vector of Line_segment's.
     */
    const std::vector<Line_segment>& get_segments() const
    {
        return m_segments;
    }

    /** 
     * @brief Reads this Collinear_segment_chain from an input stream. 
     */
    void read(std::istream& in);

    /** 
     * @brief Writes this Collinear_segment_chain to an output stream. 
     */ 
    void write(std::ostream& out) const;

private:

    /**
     * @brief Compare the starting point of two Line_segment
     */
    class Compare_starting_point
    {
    public:
        Compare_starting_point(double orientation)
            : m_orientation(orientation)
        {}

        bool operator()
        (   
            const Line_segment& s1, 
            const Line_segment& s2
        ) const
        {
            if(m_orientation < M_PI_4 || m_orientation > M_PI_4*3)
            {
                return (s1.get_start_x() < s2.get_start_x());
            }

            // else
            return (s1.get_start_y() < s2.get_start_y());
        }
    private:
        double m_orientation;
    };

    /** 
     * @brief The edge segments in this collinear line segment 
     */  
    std::vector<Line_segment> m_segments; 
};

/**
 * @brief Find a set of collinear_segments_chains
 */
template<class InputIterator>
std::vector<Collinear_segment_chain> find_collinear_segment_chains
(
    InputIterator first, 
    InputIterator last,
    double distance_threshold,
    double orientation_threshold
)
{
    typedef std::vector<Line_segment> Segment_vector;

    std::vector<Segment_vector> candidates(1, Segment_vector(1, *first));

    for(InputIterator it = first; it != last; it++)
    {
        const Line_segment& seg = *it;
        bool found = false;

        //Check whether seg belongs to one of the candidates
        for(unsigned int j = 0; j < candidates.size(); j++)
        {
            //Edge_segment_set segments = candidates[j];
            Segment_vector chains = candidates[j];
            for(unsigned int k = 0; k < chains.size(); k++)
            {
                const Line_segment& seg_to_compare = chains[k]; 
                bool is_collinear = false;
                if(fabs(seg.get_orientation() - seg_to_compare.get_orientation())
                                                            < orientation_threshold)
                {
                    const Vector& center = seg.get_centre();
                    Line line2(seg_to_compare.get_start(), seg_to_compare.get_end());
                    double dist = line2.find_distance_to_point(center);
                    if(dist < distance_threshold)
                    {
                        is_collinear = true;
                    }
                }
                if(!found && is_collinear)
                {
                    Vector seg_center = seg.get_centre();
                    Vector center = seg_to_compare.get_centre();
                    {
                        candidates[j].push_back(seg);
                        found = true;
                        break;
                    }
                }
            }
        }

        if(!found)
        {
            candidates.push_back(Segment_vector(1, seg));
        }
    }

    // Construct the collinear line segments from the candindates 
    std::vector<Collinear_segment_chain> chains(candidates.begin(), candidates.end());
    //std::copy(candidates.begin(), candidates.end(), chains.begin());
    
    return chains;
}

} //namespace kjb
                             
#endif /*COLLINEAR_SEGMENT_CHAIN */

