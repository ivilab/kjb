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
#include "bbb_cpp/bbb_endpoints.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <utility>
#include <vector>
#include <iterator>
#include <boost/variant.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

using namespace kjb;
using namespace kjb::bbb;

std::vector<size_t> kjb::bbb::trajectory_endpoints
(
    Endpoint_set& endpoints,
    const Intentional_activity& act,
    const Description& desc,
    const std::vector<size_t>& ic
)
{
    typedef Description::Act_tree::const_iterator As_it;
    typedef Intentional_activity Ia;
    typedef Physical_activity Pa;

    std::vector<size_t> og;

    std::pair<As_it, As_it> rg = desc.children(act);
    for(As_it pr_p = rg.first; pr_p != rg.second; pr_p++)
    {
        const Activity_sequence& aseq = pr_p->second;
        std::vector<size_t> cur_ic = ic;
        for(size_t j = 0; j < aseq.size(); j++)
        {
            const Ia* act_p = boost::get<Ia>(&aseq.activity(j));
            if(act_p)
            {
                cur_ic = trajectory_endpoints(endpoints, *act_p, desc, cur_ic);
                if(j == aseq.size() - 1)
                {
                    og.insert(og.end(), cur_ic.begin(), cur_ic.end());
                }
            }
            else
            {
                Endpoint ei;

                if(j == 0)
                {
                    ei.left_p = 0;
                    ei.incoming = ic;
                }
                else
                {
                    const Pa* pa_p = boost::get<Pa>(&aseq.activity(j - 1));
                    if(pa_p)
                    {
                        ei.left_p = pa_p;
                    }
                    else
                    {
                        ei.left_p = 0;
                        ei.incoming = cur_ic;
                    }
                }

                ei.right_p = boost::get<Pa>(&aseq.activity(j));
                endpoints.push_back(ei);

                if(j == aseq.size() - 1 || boost::get<Ia>(&aseq.activity(j+1)))
                {
                    Endpoint nei;
                    nei.left_p = ei.right_p;
                    nei.right_p = 0;
                    endpoints.push_back(nei);

                    if(j == aseq.size() - 1)
                    {
                        og.push_back(endpoints.size() - 1);
                    }
                    else
                    {
                        ASSERT(boost::get<Ia>(&aseq.activity(j + 1)) != 0);
                        cur_ic.clear();
                        cur_ic.push_back(endpoints.size() - 1);
                    }
                }
            }
        }
    }

    return og;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

boost::tuple<Vector, Vector, std::vector<size_t> > kjb::bbb::endpoint_mean
(
    const Endpoint_set& endpoints,
    const Description& desc,
    const Activity_library& lib
)
{
    const size_t nepts = endpoints.size();

    Vector mux(static_cast<int>(nepts), 0.0);
    Vector muy(static_cast<int>(nepts), 0.0);
    std::vector<size_t> tgts;
    for(size_t i = 0; i < nepts; i++)
    {
        const Endpoint& ei = endpoints[i];

        // check for final activities in sequences
        if(!ei.right_p)
        {
            ASSERT(ei.left_p);

            std::vector<const Intentional_activity*> ias;
            desc.ancestors(*ei.left_p, std::back_inserter(ias), 1);

            if(lib.has_target(ias.front()->name()) &&
                ias.front()->end() == ei.left_p->end())
            {
                const Vector& tg = ias.front()->parameters();
                ASSERT(tg.get_length() == 2);

                mux[i] = tg[0];
                muy[i] = tg[1];

                tgts.push_back(i);
            }
        }
    }

    return boost::make_tuple(mux, muy, tgts);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::bbb::endpoint_covariance
(
    const Endpoint_set& endpoints,
    const Activity_library& library
)
{
    // first build graph
    typedef boost::adjacency_list<
                boost::vecS,
                boost::vecS,
                boost::undirectedS,
                boost::no_property,
                boost::property<boost::edge_weight_t, double> > Graph;
    typedef std::pair<int, int> Edge;

    std::vector<Edge> edges;
    std::vector<double> weights;

    // find edges and weights
    const size_t num_epts = endpoints.size();
    for(size_t i = 0; i < num_epts; ++i)
    {
        const Endpoint& ept = endpoints[i];
        if(ept.right_p && ept.right_p == endpoints[i + 1].left_p)
        {
            ASSERT(i != num_epts - 1);
            edges.push_back(std::make_pair(i, i + 1));

            const Physical_activity& pa = *ept.right_p;
            double sg = library.trajectory_kernel(pa.name()).scale();
            weights.push_back(pa.size() / sg);
        }

        for(size_t j = 0; j < ept.incoming.size(); ++j)
        {
            edges.push_back(std::make_pair(i, ept.incoming[j]));
            double sg = library.trajectory_kernel("STAND").scale();
            weights.push_back(1 / sg);
            //weights.push_back(0.01);
        }
    }

    // build graph
    Graph graph(edges.begin(), edges.end(), weights.begin(), num_epts);

    // compute covariances
    Matrix K(num_epts, num_epts);
    for(size_t i = 0; i < num_epts; ++i)
    {
        std::vector<double> dists(boost::num_vertices(graph));
        boost::dijkstra_shortest_paths(
            graph, boost::vertex(i, graph),
            boost::distance_map(
                boost::make_iterator_property_map(
                    dists.begin(), boost::get(boost::vertex_index, graph))));

        //double ssc = library.trajectory_kernel("STAND").scale();
        //double ssg = library.trajectory_kernel("STAND").signal_sigma();
        //double ssg = 50.0;
        double ssg = 1.0;
        //K(i, i) = ssg*ssg + 1e-4;
        K(i, i) = ssg * ssg;
        for(size_t j = i + 1; j < num_epts; ++j)
        {
            K(i, j) = ssg * ssg * exp(-0.5 * dists[j] * dists[j]);
            K(j, i) = K(i, j);
        }
    }

    return K;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::bbb::endpoint_distribution
(
    Vector& mux,
    Vector& muy,
    Matrix& Kss,
    const std::vector<size_t> tgts
)
{
    const size_t num_tgts = tgts.size();
    if(num_tgts == 0)
    {
        return;
    }

    Vector fx((int)num_tgts);
    Vector fy((int)num_tgts);
    Matrix K((int)num_tgts, (int)num_tgts);
    Matrix Kns((int)num_tgts, Kss.get_num_cols());
    for(size_t i = 0; i < num_tgts; ++i)
    {
        fx[i] = mux[tgts[i]];
        fy[i] = muy[tgts[i]];
        Kns.set_row(i, Kss.get_row(tgts[i]));
        for(size_t j = 0; j < num_tgts; ++j)
        {
            K(i, j) = Kss(tgts[i], tgts[j]);
        }

        K(i, i) += 25e-2;
    }

    Matrix Ksn = matrix_transpose(Kns);
    Matrix K_inv = matrix_inverse(K);

    mux = (Ksn * K_inv) * fx;
    muy = (Ksn * K_inv) * fy;
    Kss = Kss - Ksn*K_inv*Kns;
}

