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
| Authors:
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_SCENE_DIFF_H
#define PT_SCENE_DIFF_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <diff_cpp/diff_gradient.h>
#include <diff_cpp/diff_gradient_mt.h>
#include <m_cpp/m_vector.h>
#include <diff_cpp/diff_hessian_ind.h>
#include <diff_cpp/diff_hessian_ind_mt.h>
#include <vector>
#include <algorithm>

namespace kjb {
namespace pt {

/** @brief  Wrapper for generic gradient function. */
class Scene_gradient
{
public:
    Scene_gradient
    (
        const Scene_posterior& post,
        const std::vector<double>& dx,
        size_t num_threads = 1
    ) :
        post_(post), dx_(dx), nthreads_(num_threads)
    {}

    std::vector<double> operator()(const Scene& scene) const
    {
        Vector gv;
        if(nthreads_ == 1)
        {
            gv = gradient_ind_cfd(post_, scene, dx_, post_.adapter());
        }
        else
        {
            gv = gradient_ind_cfd_mt(
                        post_, scene, dx_, post_.adapter(), nthreads_);
        }
        post_.reset();
        return std::vector<double>(gv.begin(), gv.end());
    }

private:
    Scene_posterior_ind post_;
    std::vector<double> dx_;
    size_t nthreads_;
};

/** @brief  Wrapper for generic hessian function. */
class Scene_hessian
{
public:
    Scene_hessian
    (
        const Scene_posterior& post,
        const std::vector<double>& dx,
        size_t num_threads = 1
    ) :
        post_(post), dx_(dx), nthreads_(num_threads)
    {}

    Matrix operator()(const Scene& scene) const
    {
        Matrix H;
        if(nthreads_ == 1)
        {
            H = hessian_symmetric_ind(post_, scene, dx_, post_.adapter());
        }
        else
        {
            H = hessian_symmetric_ind_mt(
                    post_, scene, dx_, post_.adapter(), nthreads_);
        }

        post_.reset();
        return H;
    }

    void set_diagonals(const Scene& scene) const;

private:
    Scene_posterior_ind post_;
    std::vector<double> dx_;
    size_t nthreads_;
};

/** @brief  Compute marginal likelihood of scene. */
double lm_marginal_likelihood(const Scene& scene, double pt, bool ih);

///**
// * @class   Gradient_adapter
// * @brief   Wraps Scene_gradient to work with ergo::hmc_step.
// */
//template<class G>
//class Gradient_adapter
//{
//private:
//    typedef std::vector<double> vec_t;
//
//public:
//    /** @brief  Construct a gradient adapter that wraps 'grad'. */
//    Gradient_adapter(const G& grad) : m_grad(grad)
//    {}
//
//    /** @brief  Computes the gradient numerically. */
//    template<class M>
//    vec_t operator()(const M& m) const
//    {
//        Vector g1 = m_grad(m);
//        vec_t g2(g1.size());
//
//        std::copy(g1.begin(), g1.end(), g2.begin());
//
//        return g2;
//    }
//
//private:
//    const G& m_grad;
//};
//
///**
// * @brief   Utility function that makes a gradient adapter.
// */
//template<class G>
//inline
//Gradient_adapter<G> make_gradient_adapter(const G& grad)
//{
//    return Gradient_adapter<G>(grad);
//}

/** @brief  Functor designed to move 3D points by pixels in image plane. */
class Pixel_move
{
public:
    /**
     * @brief   Construct a pixel-move functor with the given step size and
     *          camera.
     */
    Pixel_move(const Matrix& C) : H(3, 3)
    {
        H.set_col(0, C.get_col(0));
        H.set_col(1, C.get_col(2));
        H.set_col(2, C.get_col(3));
        K = matrix_inverse(H);

        ej1 = K.get_col(0);
        ej2 = K.get_col(1);
        eh31 = ej1[2];
        eh32 = ej2[2];
        ej1.resize(2);
        ej2.resize(2);

        set_point(Vector().set(0.0, 0.0));
    }

    /** @brief  Set the point to move. */
    void set_point(const Vector& pt)
    {
        r = pt;

        Vector rr = r;
        rr.resize(3, 1.0);
        w_G = 1 / dot(H.get_row(2), rr);
    }

    /** @brief  Move point in the x-axis of the image. */
    Vector x(double eps) const
    {
        return (w_G*r + eps*ej1) / (w_G + eps*eh31);
    }

    /** @brief  Move point in the x-axis of the image. */
    Vector y(double eps) const
    {
        return (w_G*r + eps*ej2) / (w_G + eps*eh32);
    }

private:
    Matrix H;
    Matrix K;
    Vector ej1;
    Vector ej2;
    double eh31;
    double eh32;
    Vector r;
    double w_G;
};

/**
 * @brief   Compute the step sizes that equivalent to movement of 1 pixel
 *          in image coordinate
 */
std::vector<double> trajectory_gradient_step_sizes
(
    const Scene& scene, 
    bool infer_head = true
);

/** @brief  Ensures posterior is at max (wrt a given step size). */
size_t make_max
(
    const Scene& scene,
    const Scene_posterior& pt,
    const std::vector<double>& ss,
    bool infer_head
);

}} // namespace kjb::pt

#endif // PT_SCENE_DIFF_H

