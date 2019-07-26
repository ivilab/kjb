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

#ifndef DIFF_HESSIAN_IND_H
#define DIFF_HESSIAN_IND_H

#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_util.h>
#include <vector>

namespace kjb {

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, using
 *          finite differences.
 *
 * @param   f       A function that recieves a Model const-ref and two indices
 *                  and returns a double. f(x, i, j) may be any function that
 *                  differs from f(x) by constant offset for any values of
 *                  x_i and x_j. This permits terms not involving x_i and x_j
 *                  to be ignored, saving computation time.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default. Otherwise, provide a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Matrix hessian_ind
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    const size_t D = adapter.size(&x);

    Matrix H(D, D);
    Model y = x;

    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j)
            {
                double yi = adapter.get(&y, i);

                // compute at current spot
                double fx = f(y, i, i);

                // move right and compute
                move_param(y, i, dx[i], adapter);
                double fxp = f(y, i, i);

                // move left and compute
                move_param(y, i, -2*dx[i], adapter);
                double fxm = f(y, i, i);

                // hessian
                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                // move back to original spot
                adapter.set(&y, i, yi);
            }
            else
            {
                double yi = adapter.get(&y, i);
                double yj = adapter.get(&y, j);

                // move i and j ++ and compute
                move_params(y, i, j, dx[i], dx[j], adapter);
                double fxpp = f(y, i, j);

                // move i and j +- and compute
                move_param(y, j, -2*dx[j], adapter);
                double fxpm = f(y, i, j);

                // move i and j -- and compute
                move_param(y, i, -2*dx[i], adapter);
                double fxmm = f(y, i, j);

                // move i and j -+ and compute
                move_param(y, j, 2*dx[j], adapter);
                double fxmp = f(y, i, j);

                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);

                // move back
                adapter.set(&y, i, j, yi, yj);
            }
        }
    }

    return H;
}

/**
 * @brief   Computes the Hessian of a "independent" function, evaluated at
 *          a point, for a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Matrix hessian_ind
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return hessian_ind(f, x, dx, Vector_adapter<Vec>());
}

/**
 * @brief   Computes the Hessian of an "independent" function, evaluated
 *          at a point, using
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
Matrix hessian_symmetric_ind
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter
)
{
    const size_t D = adapter.size(&x);

    Matrix H(D, D);
    Model y = x;

    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j <= i; j++)
        {
            if(i == j)
            {
                double yi = adapter.get(&y, i);

                // compute at current spot
                double fx = f(y, i, i);

                // move right and compute
                move_param(y, i, dx[i], adapter);
                double fxp = f(y, i, i);

                // move left and compute
                move_param(y, i, -2*dx[i], adapter);
                double fxm = f(y, i, i);

                // hessian
                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                // move back to original spot
                adapter.set(&y, i, yi);
            }
            else
            {
                double yi = adapter.get(&y, i);
                double yj = adapter.get(&y, j);

                // move i and j ++ and compute
                move_params(y, i, j, dx[i], dx[j], adapter);
                double fxpp = f(y, i, j);

                // move i and j +- and compute
                move_param(y, j, -2*dx[j], adapter);
                double fxpm = f(y, i, j);

                // move i and j -- and compute
                move_param(y, i, -2*dx[i], adapter);
                double fxmm = f(y, i, j);

                // move i and j -+ and compute
                move_param(y, j, 2*dx[j], adapter);
                double fxmp = f(y, i, j);

                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);
                H(j, i) = H(i, j);

                // move back
                adapter.set(&y, i, j, yi, yj);
            }
        }
    }

    return H;
}

/**
 * @brief   Computes the Hessian of an "independent" function, evaluated
 *          at a point, for a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Matrix hessian_symmetric_ind
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx
)
{
    return hessian_symmetric_ind(f, x, dx, Vector_adapter<Vec>());
}

/**
 * @brief   Computes the Hessian diagonal of a function, evaluated at a
 *          point, using finite differences.
 *
 * @param   f       A function that recieves a Model const-ref and two indices
 *                  and returns a double. f(x, i) may be any function that
 *                  differs from f(x) by constant offset for any values of
 *                  x_i. This permits terms not involving x_i
 *                  to be ignored, saving computation time.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default. Otherwise, provide a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  Vector_adapater for more information.
 */
template<class Func, class Model, class Adapter>
Vector hessian_ind_diagonal
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t is,
    size_t ie
)
{
    const size_t D = adapter.size(&x);

    IFT(is < D && ie < D && ie > is, Runtime_error,
        "Cannot compute Hessian diagonal; bad indices.");

    Vector H(ie - is + 1);
    Model y = x;

    for(size_t i = is; i <= ie; i++)
    {
        double yi = adapter.get(&y, i);

        // compute at current spot
        double fx = f(y, i);

        // move right and compute
        move_param(y, i, dx[i], adapter);
        double fxp = f(y, i);

        // move left and compute
        move_param(y, i, -2*dx[i], adapter);
        double fxm = f(y, i);

        // hessian
        H[i - is] = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

        // move back to original spot
        adapter.set(&y, i, yi);
    }

    return H;
}

/**
 * @brief   Computes the Hessian diagonal of a "independent" function, evaluated
 *          at a point, for a vector-style model.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector hessian_ind_diagonal
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t is,
    size_t ie
)
{
    return hessian_ind_diagonal(f, x, dx, Vector_adapter<Vec>(), is, ie);
}

} //namespace kjb

#endif /*DIFF_HESSIAN_H_IND */

