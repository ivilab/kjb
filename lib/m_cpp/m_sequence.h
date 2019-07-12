/* $Id: m_sequence.h 18280 2014-11-25 03:36:33Z ksimek $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#ifndef KJB_CPP_M_SEQUENCE
#define KJB_CPP_M_SEQUENCE

#include <vector>
#include <stdlib.h>

namespace kjb
{
/**
 * @file  A set of classes representing numeric sequences
 */

/**
 * A generic numeric sequence.
 */
class Sequence
{
public:
    /** 
     * Retrieve the i'th element of the sequence
     */
    virtual double operator[](size_t index) = 0;
};



/**
 * A sequence of numbers defined by a starting value, ending value and an interval.  
 */
class Interval_sequence
{
public:
    /**
     * define a sequence 
     *     {start, start + interval, start + 2 * interval,
     *      ..., start + N * interval}
     * where
     *     end - interval < N * interval <= end
     *
     * This is simlar to matlab's "start:interval:end" notation for generating sequences
     *
     * It also provides a concise way to populate a vector with a sequence:
     *      vec = Interval_sequence(0,2,100).to_vector()
     *
     * Storage is constant, so any size sequence is possible (up to the limits of double).
     *
     * @author Kyle Simek
     */
    Interval_sequence(double start, double interval, double end);

    virtual double operator[](size_t index) const;

    std::vector<double> to_vector() const;
private:
    double start_;
    double end_;
    double interval_;
};

} // namespace kjb

#endif
