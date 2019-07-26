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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#ifndef DIFF_HESSIAN_H
#define DIFF_HESSIAN_H

#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_util.h>
#include <vector>

namespace kjb {

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, using
 *          finite differences.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default. Otherwise, provide a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Matrix hessian
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    const size_t D = adapter.size(&x);

    Matrix H(D, D);
    double fx = f(x);
    Model y = x;

    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j)
            {
                move_param(y, i, dx[i], adapter);
                double fxp = f(y);

                move_param(y, i, -2*dx[i], adapter);
                double fxm = f(y);

                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                move_param(y, i, dx[i], adapter);
            }
            else
            {
                move_params(y, i, j, dx[i], dx[j], adapter);
                double fxpp = f(y);

                move_param(y, j, -2*dx[j], adapter);
                double fxpm = f(y);

                move_param(y, i, -2*dx[i], adapter);
                double fxmm = f(y);

                move_param(y, j, 2*dx[j], adapter);
                double fxmp = f(y);

                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);

                move_params(y, i, j, dx[i], -dx[j], adapter);
            }
        }
    }

    return H;
}

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, for
 *          a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Matrix hessian
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return hessian(f, x, dx, Vector_adapter<Vec>());
}

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, using
 *          finite differences. This function assumes that the Hessian is
 *          SYMMETRIC, and only computes the lower triangle of it.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default. Otherwise, provide a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Matrix hessian_symmetric
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    const size_t D = adapter.size(&x);

    Matrix H(D, D);
    double fx = f(x);
    Model y = x;

    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j <= i; j++)
        {
            if(i == j)
            {
                move_param(y, i, dx[i], adapter);
                double fxp = f(y);

                move_param(y, i, -2*dx[i], adapter);
                double fxm = f(y);

                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                move_param(y, i, dx[i], adapter);
            }
            else
            {
                move_params(y, i, j, dx[i], dx[j], adapter);
                double fxpp = f(y);

                move_param(y, j, -2*dx[j], adapter);
                double fxpm = f(y);

                move_param(y, i, -2*dx[i], adapter);
                double fxmm = f(y);

                move_param(y, j, 2*dx[j], adapter);
                double fxmp = f(y);

                // symmetry assumption
                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);
                H(j, i) = H(i, j);

                move_params(y, i, j, dx[i], -dx[j], adapter);
            }
        }
    }

    return H;
}

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, for
 *          a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Matrix hessian_symmetric
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return hessian_symmetric(f, x, dx, Vector_adapter<Vec>());
}

/**
 * @brief   Computes the diagonal Hessian of a function, evaluated at a point,
 *          using finite differences.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default. Otherwise, provide a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Matrix hessian_diagonal
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    const size_t D = adapter.size(&x);

    Matrix H(D, D);
    double fx = f(x);
    Model y = x;

    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j)
            {
                move_param(y, i, dx[i], adapter);
                double fxp = f(y);

                move_param(y, i, -2*dx[i], adapter);
                double fxm = f(y);

                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                move_param(y, i, dx[i], adapter);
            }
        }
    }
    return H;
}

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, for
 *          a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Matrix hessian_diagonal
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return hessian_diagonal(f, x, dx, Vector_adapter<Vec>());
}

} //namespace kjb

#endif /*DIFF_HESSIAN_H */

