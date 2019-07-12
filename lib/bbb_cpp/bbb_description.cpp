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

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "bbb_cpp/bbb_data.h"
#include "l_cpp/l_functors.h"

#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <algorithm>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace kjb;
using namespace kjb::bbb;

std::ostream& kjb::bbb::operator<<(std::ostream& ost, const Description& desc)
{
    // assign ids to activities
    std::map<const Intentional_activity*, size_t> ids;
    size_t id = 0;
    BOOST_FOREACH(const Description::Act_tree::value_type& pr, desc.tree_)
    {
        ids[pr.first] = id++;
    }

    // write stuff
    ost << "ROOT: " << desc.root_.name() << "-" << ids[&desc.root_] << std::endl;

    const Intentional_activity* curia_p = 0;
    BOOST_FOREACH(const Description::Act_tree::value_type& pr, desc.tree_)
    {
        const Intentional_activity& parent = *pr.first;
        const Activity_sequence& aseq = pr.second;

        if(&parent != curia_p)
        {
            ost << "============================================";
            ost << std::endl << std::endl;
            ost << "ACTIVITY: " << parent.name() << "-" << ids[&parent];
            ost << std::endl << std::endl;
            curia_p = &parent;
        }

        ost << aseq << std::endl;
    }

    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Description::copy
(
    const Description& desc,
    const Intentional_activity& local_act,
    const Intentional_activity& desc_act
)
{
    typedef const Activity_sequence* Asp;
    typedef const Intentional_activity* Iap;
    typedef Act_tree::const_iterator As_it;

    std::pair<As_it, As_it> rg = desc.children(desc_act);
    for(As_it pr_p = rg.first; pr_p != rg.second; pr_p++)
    {
        const Activity_sequence& aseq = pr_p->second;
        Asp aseq_p = add_child_sequence(&local_act, aseq);
        for(size_t j = 0; j < aseq_p->size(); j++)
        {
            Iap act_p = boost::get<Intentional_activity>(&aseq_p->activity(j));
            Iap act_q = boost::get<Intentional_activity>(&aseq.activity(j));

            if(act_q)
            {
                ASSERT(act_p);
                copy(desc, *act_p, *act_q);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Description_info::fill_tree
(
    const Intentional_activity& act,
    size_t id,
    const Description& desc,
    const Activity_library& lib
)
{
    typedef Description::Act_tree::const_iterator As_it;
    typedef Intentional_activity Ia;

    As_it chbegin;
    As_it chend;
    boost::tie(chbegin, chend) = desc.children(act);
    for(As_it pr_p = chbegin; pr_p != chend; ++pr_p)
    {
        const Activity_sequence& aseq = pr_p->second;
        for(size_t j = 0; j < aseq.size(); ++j)
        {
            using boost::apply_visitor;
            const Activity_sequence::Activity& cur_act = aseq.activity(j);
            Activity_info ai;
            ai.id = ++index;
            apply_visitor(Get_name(&ai.name), cur_act);
            apply_visitor(Get_start(&ai.start), cur_act);
            apply_visitor(Get_end(&ai.end), cur_act);
            ai.parent = id;
            ai.role = lib.role_index(aseq.role());
            apply_visitor(Get_trajectories(&ai.trajs), cur_act);
            info_set.insert(ai);

            const Ia* iact_p = boost::get<Ia>(&cur_act);
            if(iact_p)
            {
                fill_tree(*iact_p, ai.id, desc, lib);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Description_info::extract_tree
(
    const Intentional_activity& act,
    size_t id,
    Description& desc,
    const Activity_library& lib
) const
{
    using namespace std;

    vector<Activity_sequence> seqs;
    vector<vector<size_t> > ids;

    get_child_sequences(id, back_inserter(seqs), back_inserter(ids), lib);
    for(size_t j = 0; j < seqs.size(); ++j)
    {
        const Activity_sequence* aseq
                            = desc.add_child_sequence(&act, seqs[j]);

        for(size_t k = 0; k < aseq->size(); ++k)
        {
            typedef Intentional_activity Ia;
            const Ia* ia_p = boost::get<Ia>(&aseq->activity(k));
            if(ia_p)
            {
                extract_tree(*ia_p, ids[j][k], desc, lib);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Description kjb::bbb::make_trivial_description(const Data& data)
{
    const size_t sf = 0;
    const size_t ef = data.end_frame();
    const size_t J = data.size();

    std::vector<size_t> vec(J);
    std::generate(vec.begin(), vec.end(), Increment<size_t>(0));
    Description desc(sf, ef, Traj_set(vec.begin(), vec.end()));

    const Intentional_activity& root = desc.root_activity();
    const Traj_set& rts = root.trajectories();
    Traj_set::const_iterator tr_p = rts.begin();
    for(size_t j = 0; j < J; ++j, ++tr_p)
    {
        // create PA
        Traj_set wts;
        wts.insert(*tr_p);
        Physical_activity walk("WALK", data.trajectory(j), wts);

        // create and fill ASEQ
        Activity_sequence aseq("FFAER");
        aseq.add(walk);

        // add to description
        desc.add_child_sequence(&root, aseq);
    }

    return desc;
}

