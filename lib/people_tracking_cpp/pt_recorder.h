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
|     Ernesto Brau, Jinyan Guan
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_RECORDER_H
#define PT_RECORDER_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_info.h>

namespace kjb {
namespace pt {

/**
 * @class   Scene_info_recorder
 * @brief   Records the info about the scene
 */
template <class OutputIterator>
class Scene_info_recorder
{
public:
    typedef Scene_info record_type;

    /** @brief  Constructs a recorder. */
    Scene_info_recorder(OutputIterator it, const Scene_posterior& post) :
        it_(it), post_(post), increment_(true)
    {}

    /** @brief  Force replacing of recorded value every time. */
    Scene_info_recorder& replace()
    {
        increment_ = false;
        return *this;
    }

    /** @brief  Records a step. */
    template <class Step>
    void operator()(const Step&, const Scene& sc, double)
    {
        info_.set(post_, sc, true);
        *it_ = info_;
        if(increment_) it_++;
    }

private:
    Scene_info info_;
    OutputIterator it_;
    const Scene_posterior& post_;
    bool increment_;
};

/** @brief  Convenience function to create a Scene_info_recorder. */
template <class OutputIterator>
inline
Scene_info_recorder<OutputIterator> make_si_recorder
(
    OutputIterator it,
    const Scene_posterior& post
)
{
    return Scene_info_recorder<OutputIterator>(it, post);
}

/**
 * @class   Proposed_info_recorder
 * @brief   Records the info about the scene
 */
template <class OutputIterator>
class Proposed_info_recorder
{
public:
    typedef Scene_info record_type;

    /** @brief  Constructs a recorder. */
    Proposed_info_recorder(OutputIterator it, const Scene_posterior& post) :
        it_(it), post_(post), increment_(true)
    {}

    /** @brief  Force replacing of recorded value every time. */
    Proposed_info_recorder& replace()
    {
        increment_ = false;
        return *this;
    }

    /** @brief  Records a step. */
    template <class Step>
    void operator()(const Step& step, const Scene&, double)
    {
        info_.set(post_, *step.proposed_model(), true);
        *it_ = info_;
        if(increment_) it_++;
    }

private:
    Scene_info info_;
    OutputIterator it_;
    const Scene_posterior& post_;
    bool increment_;
};

/** @brief  Convenience function to create a Proposed_info_recorder. */
template <class OutputIterator>
inline
Proposed_info_recorder<OutputIterator> make_pi_recorder
(
    OutputIterator it,
    const Scene_posterior& post
)
{
    return Proposed_info_recorder<OutputIterator>(it, post);
}

}} // namespace kjb::pt

#endif /* PT_RECORDER_H */

