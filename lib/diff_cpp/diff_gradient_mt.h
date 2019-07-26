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

#ifndef DIFF_GRADIENT_MT_H
#define DIFF_GRADIENT_MT_H

#include <m_cpp/m_vector.h>
#include <diff_cpp/diff_util.h>
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/time.h>

namespace kjb {

/** @brief  Helper function for gradient_cfd_mt */
template<class Func, class Model, class Adapter>
void gradient_cfd_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t i_start,
    size_t i_end,
    Vector& v
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;

    for(size_t i = i_start; i <= i_end; i++)
    {
        double yi = aptr.get(&y, i);

        move_param(y, i, dx[i], aptr);
        double fxp = g(y);

        move_param(y, i, -2*dx[i], aptr);
        double fxm = g(y);

        v[i] = (fxp-fxm)/(2*dx[i]);

        // return to original spot
        aptr.set(&y, i, yi);
    }
}

/** @brief  Helper function for gradient_ffd_mt */
template<class Func, class Model, class Adapter>
void gradient_ffd_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    double fx,
    size_t i_start,
    size_t i_end,
    Vector& v
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;

    for(size_t i = i_start; i <= i_end; i++)
    {
        double yi = adapter.get(&y, i);

        move_param(y, i, dx[i], aptr);
        double fxp = g(y);

        v[i] = (fxp - fx) / dx[i];

        // return to original spot
        adapter.set(&y, i, yi);
    }
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences. Multi-threaded version.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  Model const-ref and return a double. This function is
 *                  responsible for handling its own rewritable state; i.e.,
 *                  make sure f is thread-safe.
 * @param   x       The point at which the gradient is to be evaluated.
 * @param   dx      The step sizes in each of the dimensions of x.
 * @param   adapter Adapts a model type to behave as a vector. If the model
 *                  type has operator[] and size() implemented, then use
 *                  the default kjb::Vector_adapter. Otherwise, provide
 *                  a class which implements
 *                  get(), set(), and size() for your model type. See
 *                  kjb::Vector_adapater for more information.
 * @param   nt      The number of threads. If not set, use the number of
 *                  hardware threads available on the current system 
 */
template<class Func, class Model, class Adapter>
Vector gradient_cfd_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t nt
)
{
    using namespace boost;

    size_t D = adapter.size(&x);
    Vector G(D);

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
            gradient_cfd_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), boost::cref(adapter),
            (D/nt) * i, l - 1, boost::ref(G)));
    }

    // join threads before returning
    thrds.join_all();

    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences, for a vectory-style model.
 *          Multi-threaded version.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  vector-style object const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_cfd_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx, 
    size_t nt = 0
)
{
    return gradient_cfd_mt(f, x, dx, Vector_adapter<Vec>(), nt);
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          forward finite differences. Multi-threaded version.
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
Vector gradient_ffd_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t nt
)
{
    using namespace boost;

    size_t D = adapter.size(&x);
    Vector G(D);
    double fx = f(x);

    size_t avail_core = thread::hardware_concurrency();
    if(nt > avail_core )
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
            gradient_ffd_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), boost::cref(adapter),
            fx, (D/nt) * i, l - 1, boost::ref(G)));
    }

    // join threads before returning
    thrds.join_all();
    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          forward finite differences, for a vectory-style model.
 *          Multi-threaded version.
 *
 * @param   f       The function whose gradient is desired. It must receive a
 *                  vector-style object const-ref and return a double.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_ffd_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t nt = 0
)
{
    return gradient_ffd_mt(f, x, dx, Vector_adapter<Vec>(), nt);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/*                        INDEPENDENT VERSIONS OF GRADIENT                    */
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Helper function for gradient_ind_cfd_mt */
template<class Func, class Model, class Adapter>
void gradient_ind_cfd_mt_worker
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t i_start,
    size_t i_end,
    Vector& v
)
{
    // copy to avoid concurrent access
    Func g = f;
    Model y = x;
    Adapter aptr = adapter;

    for(size_t i = i_start; i <= i_end; i++)
    {
        double yi = aptr.get(&y, i);

        move_param(y, i, dx[i], aptr);
        double fxp = g(y,i);

        move_param(y, i, -2*dx[i], aptr);
        double fxm = g(y,i);

        v[i] = (fxp-fxm)/(2*dx[i]);

        // return to original spot
        aptr.set(&y, i, yi);
    }
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences. Multithreaded version.
 *
 * @param   f       A function that recieves a Model const-ref and an index
 *                  and returns a double. Semantically, f(x, i)
 *                  returns the result of evaluating the true function
 *                  on a model that only differs from x in dimension i.
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
Vector gradient_ind_cfd_mt
(
    const Func& f,
    const Model& x,
    const std::vector<double>& dx,
    const Adapter& adapter,
    size_t nt 
)
{
    using namespace boost;

    size_t D = adapter.size(&x);
    Vector G(D);

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
            gradient_ind_cfd_mt_worker<Func, Model, Adapter>,
            boost::cref(f), boost::cref(x), boost::cref(dx), 
            boost::cref(adapter),
            (D/nt) * i, l - 1, boost::ref(G)));
    }

    // join threads before returning
    thrds.join_all();

    return G;
}

/**
 * @brief   Computes the gradient of a function, evaluated at a point, using
 *          central finite differences for a vector-style model.
 *          Multi-threaded version.
 *
 * @param   f       A function that recieves a Model const-ref and an index
 *                  and returns a double. Semantically, f(x, i)
 *                  returns the result of evaluating the true function
 *                  on a model that only differs from x in dimension i.
 * @param   x       The point at which the gradient is to be evaluated.
 *                  Class Vec must have size() and operator[] defined.
 * @param   dx      The step sizes in each of the dimensions of x.
 */
template<class Func, class Vec>
inline
Vector gradient_ind_cfd_mt
(
    const Func& f,
    const Vec& x,
    const std::vector<double>& dx,
    size_t nt = 0
)
{
    return gradient_ind_cfd_mt(f, x, dx, Vector_adapter<Vec>(), nt);
}

} //namespace kjb

#endif /*DIFF_GRADIENT_MT_H */

