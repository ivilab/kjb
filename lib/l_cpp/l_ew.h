/* $Id: l_ew.h 17249 2014-08-07 16:03:32Z predoehl $ */
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#ifndef KJB_L_EW_DEFINED
#define KJB_L_EW_DEFINED

/**
 * @file 
 * @brief   Functions to implement element-wise computations.
 * @author  Ernesto Brau
 */

namespace kjb
{

/**
 * @addtogroup kjbLinearAlgebra
 *
 * @{
 */

/**
 * @brief Multiply the elements of two Indexable things. Must
 *        be indexed via operator() and assignable.
 */
template<class Indexable>
Indexable ew_multiply(const Indexable& I, const Indexable& J)
{
    Indexable K = I;
    for(int i = 0; i < I.get_length(); i++)
    {
        K(i) = K(i) * J(i);
    }

    return K;
}

/**
 * @brief Element-wise abosulte value
 */
template<class Indexable>
void ew_abs_ow(Indexable& I)
{
    for(int i = 0; i < I.get_length(); i++)
    {
        I(i) = abs(I(i));
    }
}

/**
 * @brief Square the elements of an indexable class in place. I
 * must be indexed via operator().
 */
template<class Indexable>
void ew_square_ow(Indexable& I)
{
    for(int i = 0; i < I.get_length(); i++)
    {
        I(i) = I(i) * I(i);
    }
}

/**
 * @brief Square the elements of an indexable class; the result
 * is returned. I must be indexed via operator().
 */
template<class Indexable>
Indexable ew_square(const Indexable& I)
{
    Indexable M = I;
    ew_square_ow(M);
    return M;
}

/**
 * @brief Take the square root of the elements of an indexable
 * class in place. I must be indexed via operator().
 */
template<class Indexable>
void ew_sqrt_ow(Indexable& I)
{
    for(int i = 0; i < I.get_length(); i++)
    {
        using ::sqrt;
        I(i) = sqrt(I(i));
    }
}

/**
 * @brief Take the square root the elements of an indexable class;
 * the result is returned. I must be indexed via operator().
 */
template<class Indexable>
Indexable ew_sqrt(const Indexable& I)
{
    Indexable M = I;
    ew_sqrt_ow(M);
    return M;
}

/// @}

} //namespace kjb

#endif /* KJB_L_EW_DEFINED */

