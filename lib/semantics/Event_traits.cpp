/*!
 * @file Event_traits.cpp
 *
 * @author Colin Dawson 
 * $Id: Event_traits.cpp 17175 2014-07-29 19:11:35Z cdawson $ 
 */

#include <semantics/Event_traits.h>
#include <boost/range.hpp>
#include <string>

using namespace std;
using namespace boost;

namespace semantics
{
    
template<class T>
Key_slots::Map initialize_map(boost::array<std::string, T::size> names)
{
    Key_slots::Map result;
    for(int j = 0; j < T::size; j++)
    {
	result.insert(Key_slots::Map::value_type(j, names[j]));
    }
    return result;
}

#ifdef USE_SEMANTICS

const S1_traits::Var_list S1_traits::variable_names =
{{"TAG", "LABEL", "SEM", "PLABEL", "SEMARGS"}};
const S2_traits::Var_list S2_traits::variable_names =
{{"WORD", "TAG", "LABEL", "SEM", "PLABEL", "SEMARGS"}};
const U_traits::Var_list U_traits::variable_names =
{{"LABEL", "PLABEL", "SEM", "PSEM", "TAG", "SEMARGS", "PSEMARGS", "WORD"}};
const D1_traits::Var_list D1_traits::variable_names =
{{"TAG", "LABEL", "PUNC", "CONJ", "PLABEL", "SEM", "HLABEL", "PSEM", "HSEM", "DIST", "HTAG", "SEMARGS", "PSEMARGS", "MSEMARGS", "HWORD"}};
const D2_traits::Var_list D2_traits::variable_names = 
{{"WORD", "TAG", "LABEL", "PUNC", "CONJ", "PLABEL", "SEM", "HLABEL", "PSEM", "HSEM", "DIST", "HTAG", "SEMARGS", "PSEMARGS", "HSEMARGS", "HWORD"}};
const PCC1_traits::Var_list PCC1_traits::variable_names = 
{{"PCCTAG", "TYPE", "PLABEL", "HLABEL", "LABEL", "HTAG", "MTAG", "HWORD", "WORD"}};
const PCC2_traits::Var_list PCC2_traits::variable_names = 
{{"WORD", "PCCTAG", "TYPE", "PLABEL", "HLABEL", "LABEL", "HTAG", "MTAG", "HWORD", "MWORD"}};
const Hsem_traits::Var_list Hsem_traits::variable_names =
{{"STEPCODE", "ELABTYPE", "PSEM", "PLABEL", "PTAG", "PSEMARGS", "PWORD"}};
const Msem_traits::Var_list Msem_traits::variable_names =
{{"STEPCODE", "ELABTYPE", "PSEM", "PLABEL", "DIST", "HSEM", "HLABEL", "HTAG", "PSEMARGS", "HSEMARGS", "HWORD"}};
    
    boost::array<size_t, S1_traits::context_levels> tmp_s1 = {{2,1}};
    boost::array<size_t, S2_traits::context_levels> tmp_s2 = {{2,1,2}};
    boost::array<size_t, U_traits::context_levels> tmp_u = {{2,3,1,1}};
    boost::array<size_t, D1_traits::context_levels> tmp_d1 = {{2,4,4,1}};
    boost::array<size_t, D2_traits::context_levels> tmp_d2 = {{2,6,4,1}};
    boost::array<size_t, PCC1_traits::context_levels> tmp_pcc1 = {{1,5,2}};
    boost::array<size_t, PCC2_traits::context_levels> tmp_pcc2 = {{2,6,1}};
    boost::array<size_t, Hsem_traits::context_levels + 1> tmp_hsem = {{1,2,2,1}};
    boost::array<size_t, Msem_traits::context_levels + 1> tmp_msem = {{1,2,3,3,1}};

#else

const S1_traits::Var_list S1_traits::variable_names =
{{"TAG", "LABEL", "PLABEL"}};
const S2_traits::Var_list S2_traits::variable_names =
{{"WORD", "TAG", "LABEL", "PLABEL"}};
const U_traits::Var_list U_traits::variable_names =
{{"LABEL", "PLABEL", "TAG", "WORD"}};
const D1_traits::Var_list D1_traits::variable_names =
{{"TAG", "LABEL", "PUNC", "CONJ", "PLABEL", "HLABEL", "DIST", "HTAG", "HWORD"}};
const D2_traits::Var_list D2_traits::variable_names = 
{{"WORD", "TAG", "LABEL", "PUNC", "CONJ", "PLABEL", "HLABEL", "DIST", "HTAG", "HWORD"}};
const PCC1_traits::Var_list PCC1_traits::variable_names = 
{{"TAG", "TYPE", "PLABEL", "HLABEL", "LABEL", "HTAG", "MTAG", "HWORD", "MWORD"}};
const PCC2_traits::Var_list PCC2_traits::variable_names = 
{{"WORD", "TAG", "TYPE", "PLABEL", "HLABEL", "LABEL", "HTAG", "MTAG", "HWORD", "MWORD"}};
const Hsem_traits::Var_list Hsem_traits::variable_names =
{{"STEPCODE", "ELABTYPE", "PSEM", "PLABEL", "PTAG", "PSEMARGS", "PWORD"}};
const Msem_traits::Var_list Msem_traits::variable_names =
{{"STEPCODE", "ELABTYPE", "PSEM", "PLABEL", "DIST", "HSEM", "HLABEL", "HTAG", "PSEMARGS", "HSEMARGS", "HWORD"}};
    
    boost::array<size_t, S1_traits::context_levels> tmp_s1 = {{1}};
    boost::array<size_t, S2_traits::context_levels> tmp_s2 = {{1,2}};
    boost::array<size_t, U_traits::context_levels> tmp_u = {{1,1,1}};
    boost::array<size_t, D1_traits::context_levels> tmp_d1 = {{3,1,1}};
    boost::array<size_t, D2_traits::context_levels> tmp_d2 = {{1,7,1}};
    boost::array<size_t, PCC1_traits::context_levels> tmp_pcc1 = {{1,5,2}};
    boost::array<size_t, PCC2_traits::context_levels> tmp_pcc2 = {{2,6,1}};
    boost::array<size_t, Hsem_traits::context_levels + 1> tmp_hsem = {{1,2,2,1}};
    boost::array<size_t, Msem_traits::context_levels + 1> tmp_msem = {{1,2,1,2,3,1}};
#endif


const Key_slots::Map S1_traits::variable_map =
    initialize_map<S1_traits>(S1_traits::variable_names);
const Key_slots::Map S2_traits::variable_map =
    initialize_map<S2_traits>(S2_traits::variable_names);
const Key_slots::Map U_traits::variable_map =
    initialize_map<U_traits>(U_traits::variable_names);
const Key_slots::Map D1_traits::variable_map =
    initialize_map<D1_traits>(D1_traits::variable_names);
const Key_slots::Map D2_traits::variable_map =
    initialize_map<D2_traits>(D2_traits::variable_names);
const Key_slots::Map PCC1_traits::variable_map =
    initialize_map<PCC1_traits>(PCC1_traits::variable_names);
const Key_slots::Map PCC2_traits::variable_map =
    initialize_map<PCC2_traits>(PCC2_traits::variable_names);
const Key_slots::Map Hsem_traits::variable_map =
    initialize_map<Hsem_traits>(Hsem_traits::variable_names);
const Key_slots::Map Msem_traits::variable_map =
    initialize_map<Msem_traits>(Msem_traits::variable_names);

    
const S1_traits::Step_sizes S1_traits::step_sizes(tmp_s1.begin(), tmp_s1.end());
const S2_traits::Step_sizes S2_traits::step_sizes(tmp_s2.begin(), tmp_s2.end());
const U_traits::Step_sizes U_traits::step_sizes(tmp_u.begin(), tmp_u.end()); 
const D1_traits::Step_sizes D1_traits::step_sizes(tmp_d1.begin(), tmp_d1.end());
const D2_traits::Step_sizes D2_traits::step_sizes(tmp_d2.begin(), tmp_d2.end());
const PCC1_traits::Step_sizes PCC1_traits::step_sizes(
    tmp_pcc1.begin(), tmp_pcc1.end());
const PCC2_traits::Step_sizes PCC2_traits::step_sizes(
    tmp_pcc2.begin(), tmp_pcc2.end());
const Hsem_traits::Step_sizes Hsem_traits::step_sizes(
    tmp_hsem.begin(), tmp_hsem.end());
const Msem_traits::Step_sizes Msem_traits::step_sizes(
    tmp_msem.begin(), tmp_msem.end());

void resample_all_event_alphas()
{
    Alpha_sampler<S1_view>::recursively_resample_all_alphas(
        create_dummy_template_class<S1_traits::context_levels>());
    Alpha_sampler<S2_view>::recursively_resample_all_alphas(
        create_dummy_template_class<S2_traits::context_levels>());
    Alpha_sampler<U_view>::recursively_resample_all_alphas(
        create_dummy_template_class<U_traits::context_levels>());
    Alpha_sampler<D1_view>::recursively_resample_all_alphas(
        create_dummy_template_class<D1_traits::context_levels>());
    Alpha_sampler<D2_view>::recursively_resample_all_alphas(
        create_dummy_template_class<D2_traits::context_levels>());
    Alpha_sampler<PCC1_view>::recursively_resample_all_alphas(
        create_dummy_template_class<PCC1_traits::context_levels>());
    Alpha_sampler<PCC2_view>::recursively_resample_all_alphas(
        create_dummy_template_class<PCC2_traits::context_levels>());
    Alpha_sampler<Hsem_view>::recursively_resample_all_alphas(
        create_dummy_template_class<Hsem_traits::context_levels>());
    Alpha_sampler<Msem_view>::recursively_resample_all_alphas(
        create_dummy_template_class<Msem_traits::context_levels>());
}

}; //namespace semantics
