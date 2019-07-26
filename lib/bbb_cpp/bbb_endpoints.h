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

#ifndef B3_ENDPOINTS_H
#define B3_ENDPOINTS_H

#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "bbb_cpp/bbb_description.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <vector>
#include <boost/tuple/tuple.hpp>

namespace kjb {
namespace bbb {

/** @brief  Class that holds information for endpoints. */
struct Endpoint
{
    const Physical_activity* left_p;
    const Physical_activity* right_p;
    std::vector<size_t> incoming;
};

typedef std::vector<Endpoint> Endpoint_set;

/** @brief  Find the trajectory endpoints in a description. */
std::vector<size_t> trajectory_endpoints
(
    Endpoint_set& endpoints,
    const Intentional_activity& act,
    const Description& desc,
    const std::vector<size_t>& ic = std::vector<size_t>()
);

/** @brief  Find the trajectory endpoints in a description. */
boost::tuple<Vector, Vector, std::vector<size_t> > endpoint_mean
(
    const Endpoint_set& endpoints,
    const Description& desc,
    const Activity_library& lib
);

/** @brief  Find the trajectory endpoints in a description. */
Matrix endpoint_covariance
(
    const Endpoint_set& endpoints,
    const Activity_library& library
);

/** @brief  Compute distribution for endpoints. */
void endpoint_distribution
(
    Vector& mux,
    Vector& muy,
    Matrix& Kss,
    const std::vector<size_t> tgts
);

}} // namespace kjb::bbb

#endif /*B3_ENDPOINTS_H */

