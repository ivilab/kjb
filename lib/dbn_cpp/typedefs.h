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
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: typedefs.h 22555 2019-06-08 22:10:51Z adarsh $ */

#ifndef KJB_TIES_TYPEDEFS_H
#define KJB_TIES_TYPEDEFS_H

#ifdef KJB_HAVE_TBB
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#endif 
#include <m_cpp/m_vector.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>

using namespace kjb;
typedef gp::Prior<gp::Constant, gp::Squared_exponential> Gpp;

// foward declaration 

typedef kjb::Vector Double_v;
#ifdef KJB_HAVE_TBB
typedef std::vector<Double_v, tbb::scalable_allocator<Double_v> > Double_vv;
typedef std::vector<Double_vv, tbb::scalable_allocator<Double_vv> > Double_vvv;
typedef std::vector<Vector, tbb::scalable_allocator<Vector> > Vector_v; 
typedef std::vector<Gpp, tbb::scalable_allocator<Gpp> > Gpp_v; 
#else 
typedef std::vector<Double_v> Double_vv;
typedef std::vector<Double_vv> Double_vvv;
typedef std::vector<Vector> Vector_v; 
typedef std::vector<Gpp> Gpp_v; 
#endif // KJB_HAVE_TBB

typedef Double_v State_type;
typedef Double_vv State_vec; 
typedef Double_vvv State_vec_vec; 

struct Group_params 
{
    // Default constructor
    Group_params(size_t num_groups = 1) : 
        pred_coefs(num_groups, Vector()),
        variances(num_groups, 100.0),
        group_weight(1.0)
    {}

    // indexed by [PARAM_INDEX][PREDICTOR]
    // [CLO_PARAM_1][PREDICTOR]
    // [CLO_PARAM_2][PREDICTOR]
    // ...
    // [POLY_PARAM_1][PREDICTOR]
    // [POLY_PARAM_2][PREDICTOR]
    // ...
    std::vector<Vector> pred_coefs;

    // indexed by [OSCILLATOR][D][PREDICTOR]

    // indexed by [CLO_PARAM] 
    // [CLO_PARAM_1]
    // [CLO_PARAM_1]
    // ...
    // [POLY_PARAM_1]
    // [POLY_PARAM_2]
    // ...
    std::vector<double> variances;
    double group_weight;
};

#endif // KJB_TIES_TYPEDEFS_H

