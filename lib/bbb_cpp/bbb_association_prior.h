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

#ifndef B3_ASSOCIATION_PRIOR_H
#define B3_ASSOCIATION_PRIOR_H

#include "bbb_cpp/bbb_association.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_activity_library.h"


namespace kjb {
namespace bbb {

/**
 * @class   Association_prior
 *
 * Class represents prior over an assignment of individuals to groups.
 */
class Association_prior
{
public:
    /** @brief  Construct an empty associaiton prior. */
    Association_prior(const Activity_library& lib) : parent_p_(0), lib_(lib) {}

    /** @brief  Set parent activity. */
    void set_parent(const Intentional_activity& act) { parent_p_ = &act; }

    /** @brief  Get parent activity. */
    const Intentional_activity& parent() const { return *parent_p_; }

    /** @brief  Get activity library. */
    const Activity_library& library() const { return lib_; }

private:
    const Intentional_activity* parent_p_;
    const Activity_library& lib_;
};

/** @brief  Draw a sample from an association prior. */
Association sample(const Association_prior& prior);

}} // namespace kjb::bbb

#endif /*B3_ASSOCIATION_PRIOR_H */

