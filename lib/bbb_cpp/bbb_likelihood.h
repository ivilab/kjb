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

#ifndef B3_LIKELIHOOD_H
#define B3_LIKELIHOOD_H

#include "bbb_cpp/bbb_data.h"
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "l_cpp/l_exception.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_predictive.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_mean.h"
#include "m_cpp/m_vector.h"

namespace kjb {
namespace bbb {

class Likelihood
{
public:
    typedef gp::Predictive_nl<gp::Manual, gp::Sqex> Predictive;

public:
    /** @brief  Construct a likelihood with no data. */
    Likelihood(const Activity_library& library) :
        data_p(0),
        pred_(make_default_pred()),
        library_(library) {}

    /** @brief  Construct a likelihood with the given data. */
    Likelihood(const Data& data, const Activity_library& library) :
        data_p(&data),
        pred_(make_default_pred()),
        library_(library) {}

    /** @brief  Compute the likelihood of a description. */
    double operator()(const Description& desc) const;

    /** @brief  Set the data for this likelihood. */
    void set_data(const Data& data) { data_p = &data; }

    /** @brief  Get the data for this likelihood. */
    const Data& data() const
    {
        IFT(data_p, Runtime_error, "Likelihood data has not been set.");
        return *data_p;
    }

    /** @brief  Get the GP predictive used for this likelihood. */
    const Predictive& predictive() const { return pred_; }

    /** @brief  Get activity library. */
    const Activity_library& library() const { return library_; }

private:
    static Predictive make_default_pred()
    {
        using namespace gp;

        Inputs x(1);
        Vector f(1);
        return make_predictive_nl(Manual(x, f), Sqex(1, 1), x, f, x);
    }

    const Data* data_p;
    Predictive pred_;
    const Activity_library& library_;
};

/** @brief  Sample data from likelihood, given a description. */
Data sample(const Likelihood& likelihood, const Description& desc);

}} // namespace kjb::bbb

#endif /*B3_LIKELIHOOD_H */

