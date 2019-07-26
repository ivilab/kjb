#ifndef EVENT_TRAITS_H_
#define EVENT_TRAITS_H_

#define USE_SEMANTICS

/*!
 * @file Event_traits.h
 *
 * @author Colin Dawson 
 * $Id: Event_traits.h 18301 2014-11-26 19:17:13Z ksimek $ 
 */

#include <semantics/Marginal_cell.h>
#include <semantics/Event_view.h>
#include <semantics/Cell_traits.h>
#include <semantics/Semantic_traits.h>
#include <boost/array.hpp>
#include <boost/bimap.hpp>
#include <string>

namespace
{
    const double SYN_MIN_PROB = 0.0000000000000000001;
    const double SEM_MIN_PROB = 1.0/7;
    const int DIVERSITY_SMOOTHER = 5;
}

    
namespace semantics
{

    namespace Key_slots
    {
	typedef boost::bimap<int, std::string> Map;
    }
    
class Root_event;
class Unary_event;
class Dependency_event;
class Head_semantic_event;
class Mod_semantic_event;
class S1_event;
class S2_event;
class U_event;
class D1_event;
class D2_event;
class PCC1_event;
class PCC2_event;
class Hsem_event;
class Msem_event;
    
template<class T> struct View_traits;

template<>
struct View_traits<S1_event>
{
    enum
    {
	out_size = 2,
#ifndef USE_SEMANTICS	
	size = 3,
	context_levels = 1
#else
	size = 5,
	context_levels = 2
#endif
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER; return dw;
    }
};


template<>
struct View_traits<S2_event>
{
    enum
    {
	out_size = 1,
#ifndef USE_SEMANTICS	
	size = 4,
	context_levels = 2
#else
	size = 6,
	context_levels = 3
#endif
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};
    
template<>
struct View_traits<U_event>
{
    enum
    {
	out_size = 1,
#ifndef USE_SEMANTICS
	size = 4,
	context_levels = 3
#else
	size = 8,
	context_levels = 4
#endif
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

template<>
struct View_traits<D1_event>
{
    enum
    {
	out_size = 4,
#ifndef USE_SEMANTICS
	size = 9,
	context_levels = 3
#else
	size = 15,
	context_levels = 4
#endif
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};


template<>
struct View_traits<D2_event>
{
    enum
    {
	out_size = 1,
#ifndef USE_SEMANTICS
	size = 10,
	context_levels = 3
#else
	size = 16,
	context_levels = 4
#endif
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

template<>
struct View_traits<PCC1_event>
{
    enum
    {
	out_size = 1,
	size = 9,
	context_levels = 3
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

template<>
struct View_traits<PCC2_event>
{
    enum
    {
	out_size = 1,
	size = 10,
	context_levels = 3
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t&, const size_t&)
    {
	static double pp = SYN_MIN_PROB;
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

template<>
struct View_traits<Hsem_event>
{
    enum
    {
	out_size = 1,
	size = 7,
	context_levels = 3
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t& val, const size_t& type)
    {
	const double& pp = step_code_priors()[type][val];
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

template<>
struct View_traits<Msem_event>
{
    enum
    {
	out_size = 1,
	size = 11,
	context_levels = 4
    };
    typedef Cell::Step_sizes Step_sizes;
    typedef boost::array<std::string, size> Var_list;
    static const Step_sizes step_sizes;
    static const Var_list variable_names;
    static const Key_slots::Map variable_map;
    static const double& prior_prob(const size_t& val, const size_t& type)
    {
	const double& pp = step_code_priors()[type][val];
	return pp;
    }
    static const double& diversity_weight()
    {
	static double dw = DIVERSITY_SMOOTHER;
	return dw;
    }
};

typedef View_traits<S1_event> S1_traits;
typedef View_traits<S2_event> S2_traits;
typedef View_traits<U_event> U_traits;
typedef View_traits<D1_event> D1_traits;
typedef View_traits<D2_event> D2_traits;
typedef View_traits<PCC1_event> PCC1_traits;
typedef View_traits<PCC2_event> PCC2_traits;
typedef View_traits<Hsem_event> Hsem_traits;
typedef View_traits<Msem_event> Msem_traits;

typedef Event_view<S1_event> S1_view;
typedef Event_view<S2_event> S2_view;
typedef Event_view<U_event> U_view;
typedef Event_view<D1_event> D1_view;
typedef Event_view<D2_event> D2_view;
typedef Event_view<PCC1_event> PCC1_view;
typedef Event_view<PCC2_event> PCC2_view;
typedef Event_view<Hsem_event> Hsem_view;
typedef Event_view<Msem_event> Msem_view;


// Reroute contingency tables for D2 events at the last smoothing level
// to be shared with S2 events at their last smoothing level

template<>
struct Cell_traits<semantics::D2_view, 2>
{
    typedef Marginal_cell<semantics::S2_view, 1> Margin_type;
};

void resample_all_event_alphas();

}; // namespace semantics

#endif /* _EVENT_TRAITS_H_ */
