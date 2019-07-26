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
   |  Author:  Ernesto Brau, Zewei Jiang
 * =========================================================================== */

/* $Id$ */

#ifndef DIFF_GRADIENT_H
#define DIFF_GRADIENT_H

#include <m_cpp/m_vector.h>
#include <diff_cpp/diff_util.h>
#include <vector>

namespace kjb {

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  Model const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default kjb::Vector_adapter. Otherwise, provide
 *                  a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  kjb::Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Vector gradient_cfd
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    Model y = x;

    const size_t D = adapter.size(&y);
    Vector G(D);

    for(size_t i = 0; i < D; i++)
    {
        double yi = adapter.get(&y, i);

        move_param(y, i, dx[i], adapter);
        double fxp = f(y);

        move_param(y, i, -2*dx[i], adapter);
        double fxm = f(y);

        G[i] = (fxp - fxm) / (2*dx[i]);

        // return to original spot
        adapter.set(&y, i, yi);
    }

    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences, for a vectory-style model.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  vector-style object const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_cfd
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return gradient_cfd(f, x, dx, Vector_adapter<Vec>());
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          forward finite differences.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  Model const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default kjb::Vector_adapter. Otherwise, provide
 *                  a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  kjb::Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Vector gradient_ffd
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    Model y = x;

    const size_t D = adapter.size(&y);
    Vector G(D);
    double fx = f(y);

    for(size_t i = 0; i < D; i++)
    {
        double yi = adapter.get(&y, i);

        move_param(y, i, dx[i], adapter);
        double fxp = f(y);

        G[i] = (fxp - fx) / dx[i];

        // return to original spot
        adapter.set(&y, i, yi);
    }

    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          forward finite differences, for a vectory-style model.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  vector-style object const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_ffd
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return gradient_ffd(f, x, dx, Vector_adapter<Vec>());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/*                        INDEPENDENT VERSIONS OF GRADIENT                    */
/*                                                                            */
/* These versions use "smart" functions that may exploit indepdendence prop-  */
/* erties to avoid full function evaluation.  For any dimension i, f(x) may   */
/* be decomposed the sum of terms involing x_i and those not.  The latter     */
/* terms cancel in the finite-differences equation, and can be ignored.  Thus */
/* the functions below receive f(x,i) which omits terms not involving x_i,    */
/* often at great performance advantage.                                      */
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences.
 *
 * @param   f       A function f(x,i) that recieves a Model const-ref and an index
 *                  and returns a double. f(x, i) may be any function that differs 
 *                  from f(x) by constant offset for any value of x_i. This permits
 *                  terms not involving x_i to be ignored, saving computation.
 * @param   x       The point at which the gradient is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default kjb::Vector_adapter. Otherwise, provide
 *                  a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  kjb::Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Vector gradient_ind_cfd
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    Model y = x;

    const size_t D = adapter.size(&y);
    Vector G(D);

    for(size_t i = 0; i < D; i++)
    {
        double yi = adapter.get(&y, i);

        move_param(y, i, dx[i], adapter);
        double fxp = f(y, i);

        move_param(y, i, -2*dx[i], adapter);
        double fxm = f(y, i);

        G[i] = (fxp - fxm) / (2*dx[i]);

        // return to original spot
        adapter.set(&y, i, yi);
    }

    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences for a vector-style model.
 *
 * @param   f       A function f(x,i) that recieves a Model const-ref and an index
 *                  and returns a double. f(x, i) may be any function that differs 
 *                  from f(x) by constant offset for any value of x_i. This permits
 *                  terms not involving x_i to be ignored, saving computation.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_ind_cfd
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return gradient_ind_cfd(f, x, dx, Vector_adapter<Vec>());
}

} //namespace kjb

#endif /*DIFF_GRADIENT_H */

