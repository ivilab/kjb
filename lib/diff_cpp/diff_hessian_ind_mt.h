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
   |  Author:  Ernesto Brau, Jinyan 
 * =========================================================================== */

/* $Id$ */

#ifndef DIFF_HESSIAN_IND_MT_H
#define DIFF_HESSIAN_IND_MT_H

#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_util.h>
#include <vector>
#include <boost/thread.hpp>
#include <boost/ref.hpp>

namespace kjb {

/** @brief  Helper function for hessian_ind_mt */
template<class Func, class Model, class Adapter>
void hessian_ind_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t istart, 
    size_t iend,
    Matrix& H
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;
    size_t D = dx.size();

    for(size_t i = istart; i <= iend; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j)
            {
                double yi = aptr.get(&y, i);

                // compute at current spot
                double fx = g(y, i, i);

                // move right and compute
                move_param(y, i, dx[i], aptr);
                double fxp = g(y, i, i);

                // move left and compute
                move_param(y, i, -2*dx[i], aptr);
                double fxm = g(y, i, i);

                // hessian
                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                // move back to original spot
                aptr.set(&y, i, yi);
            }
            else
            {
                double yi = aptr.get(&y, i);
                double yj = aptr.get(&y, j);

                // move i and j ++ and compute
                move_params(y, i, j, dx[i], dx[j], aptr);
                double fxpp = g(y, i, j);

                // move i and j +- and compute
                move_param(y, j, -2*dx[j], aptr);
                double fxpm = g(y, i, j);

                // move i and j -- and compute
                move_param(y, i, -2*dx[i], aptr);
                double fxmm = g(y, i, j);

                // move i and j -+ and compute
                move_param(y, j, 2*dx[j], aptr);
                double fxmp = g(y, i, j);

                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);

                // move back
                aptr.set(&y, i, j, yi, yj);
            }
        }
    }
}

/** @brief  Helper function for hessian_symmetric_ind_mt */
template<class Func, class Model, class Adapter>
void hessian_symmetric_ind_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t i_start,
    size_t i_end,
    Matrix& H
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;

    for(size_t i = i_start; i <= i_end; i++)
    {
        for(size_t j = 0; j <= i; j++)
        {
            if(i == j)
            {
                double yi = aptr.get(&y, i);

                // compute at current spot
                double fx = g(y, i, i);

                // move right and compute
                move_param(y, i, dx[i], aptr);
                double fxp = g(y, i, i);

                // move left and compute
                move_param(y, i, -2*dx[i], aptr);
                double fxm = g(y, i, i);

                // hessian
                H(i, i) = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

                // move back to original spot
                aptr.set(&y, i, yi);
            }
            else
            {
                double yi = aptr.get(&y, i);
                double yj = aptr.get(&y, j);

                // move i and j ++ and compute
                move_params(y, i, j, dx[i], dx[j], aptr);
                double fxpp = g(y, i, j);

                // move i and j +- and compute
                move_param(y, j, -2*dx[j], aptr);
                double fxpm = g(y, i, j);

                // move i and j -- and compute
                move_param(y, i, -2*dx[i], aptr);
                double fxmm = g(y, i, j);

                // move i and j -+ and compute
                move_param(y, j, 2*dx[j], aptr);
                double fxmp = g(y, i, j);

                H(i, j) = (fxpp - fxpm - fxmp + fxmm) / (4*dx[i]*dx[j]);
                H(j, i) = H(i, j);

                // move back
                aptr.set(&y, i, j, yi, yj);
            }
        }
    }
}

/** @brief  Helper function for hessian_ind_diagonal_mt */
template<class Func, class Model, class Adapter>
void hessian_ind_diagonal_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t is,
    size_t i_start,
    size_t i_end,
    Vector& H
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;

    const size_t D = dx.size();

    IFT(i_start < D && i_end < D && i_start >= is, Runtime_error,
        "Cannot compute Hessian diagonal; bad indices.");

    for(size_t i = i_start; i <= i_end; i++)
    {
        double yi = aptr.get(&y, i);

        // compute at current spot
        double fx = g(y, i);

        // move right and compute
        move_param(y, i, dx[i], aptr);
        double fxp = g(y, i);

        // move left and compute
        move_param(y, i, -2*dx[i], aptr);
        double fxm = g(y, i);

        // hessian
        H[i - is] = (fxp - 2*fx + fxm) / (dx[i]*dx[i]);

        // move back to original spot
        aptr.set(&y, i, yi);
    }
}

/**
 * @brief   Computes the Hessian of a function, evaluated at a point, using
 *          finite differences. Multi-threaded version.
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
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Model, class Adapter>
Matrix hessian_ind_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t nt
)
{
    using namespace boost;

    //size_t D = adapter.size(&x);
    size_t D = dx.size();
    Matrix H(D, D);

    size_t avail_core = thread::hardware_concurrency();
    if(nt == 0 || nt > avail_core )
    {
        nt = avail_core;
    }

    if(D < nt)
    {
        nt = D;
    }

    if(nt == 0)
    {
        nt = 1;
    }

    // send computation to threads
    thread_group thrds;
    for(size_t i = 0; i < nt; i++)
    {
        using boost::cref;
        using boost::ref;

        size_t l = (i == nt - 1 ? D : (D/nt) * (i+1));
        thrds.create_thread(bind(
            hessian_ind_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), boost::cref(adapter),
            (D/nt) * i, l - 1, boost::ref(H)));
    }

    // join threads before returning
    thrds.join_all();

    return H;
}

/**
 * @brief   Computes the Hessian of a "independent" function, evaluated at
 *          a point, for a vector-style model.
 *          Multi-threaded version.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Vec>
inline
Matrix hessian_ind_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t nt = 0
)
{
    return hessian_ind_mt(f, x, dx, Vector_adapter<Vec>(), nt);
}


/**
 * @brief   Computes the Hessian of an "independent" function, evaluated
 *          at a point, using
 *          finite differences. This function assumes that the Hessian is
 *          SYMMETRIC, and only computes the lower triangle of it.
 *          Multi-threaded version
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
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Model, class Adapter>
Matrix hessian_symmetric_ind_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t nt
)
{
    using namespace boost;

    //size_t D = adapter.size(&x);
    size_t D = dx.size();
    Matrix H(D, D);

    size_t avail_core = thread::hardware_concurrency();
    if(nt == 0 || nt > avail_core )
    {
        nt = avail_core;
    }

    if(D < nt)
    {
        nt = D;
    }

    if(nt == 0)
    {
        nt = 1;
    }

    // send computation to threads
    thread_group thrds;
    for(size_t i = 0; i < nt; i++)
    {
        size_t l = (i == nt - 1 ? D : (D/nt) * (i+1));
        thrds.create_thread(bind(
            hessian_symmetric_ind_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), 
            boost::cref(adapter),
            (D/nt) * i, l - 1, boost::ref(H)));
    }

    // join threads before returning
    thrds.join_all();

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
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Vec>
inline
Matrix hessian_symmetric_ind_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t nt = 0
)
{
    return hessian_symmetric_ind_mt(f, x, dx, Vector_adapter<Vec>(), nt);
}

/**
 * @brief   Computes the Hessian diagonal of a function, evaluated at a
 *          point, using finite differences. Multi-threaded version.
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
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Model, class Adapter>
Vector hessian_ind_diagonal_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t is,
    size_t ie,
    size_t nt 
)
{
    using namespace boost;

    //const size_t D = adapter.size(&x);
    const size_t D = dx.size();

    IFT(is < D && ie < D && ie > is, Runtime_error,
        "Cannot compute Hessian diagonal; bad indices.");

    size_t rD = ie - is + 1;
    Vector H(rD);

    size_t avail_core = thread::hardware_concurrency();
    if(nt == 0 || nt > avail_core )
    {
        nt = avail_core;
    }

    if(rD < nt)
    {
        nt = rD;
    }

    if(nt == 0)
    {
        nt = 1;
    }

    // send computation to threads
    thread_group thrds;
    for(size_t i = 0; i < nt; i++)
    {
        size_t l = (i == nt - 1 ? rD : (rD/nt) * (i+1));

        thrds.create_thread(bind(
            hessian_ind_diagonal_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), 
            boost::cref(adapter), is,
            is + (rD/nt) * i, is + l - 1, boost::ref(H)));
    }
    thrds.join_all();

    return H;
}

/**
 * @brief   Computes the Hessian diagonal of a "independent" function, evaluated
 *          at a point, for a vector-style model. Multi-threaded version.
 *
 * @param   f       A function that recieves a Model const-ref and returns
 *                  a double.
 * @param   x       The point at which the Hessian is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Vec>
inline
Vector hessian_ind_diagonal_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t is,
    size_t ie,
    size_t nt = 0
)
{
    return hessian_ind_diagonal_mt(f, x, dx, Vector_adapter<Vec>(), is, ie, nt);
}

} //namespace kjb

#endif /*DIFF_HESSIAN_H_IND */

