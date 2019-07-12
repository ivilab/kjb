/*!
 * @file Semspear_tree.cpp
 *
 * @author Colin Dawson 
 * $Id: Semspear_tree.cpp 21596 2017-07-30 23:33:36Z kobus $ 
 */

#include "l/l_sys_debug.h"
#include "m_cpp/m_vector.h"
#include "semantics/Semspear_tree.h"
#include "semantics/Root_event.h"
#include "semantics/Unary_event.h"
#include "semantics/Dependency_event.h"
#include "semantics/Punctuation_event.h"
#include "semantics/TOP_event.h"
#include "semantics/Head_semantic_event.h"
#include "semantics/Mod_semantic_event.h"
#include "semantics/Null_semantic_event.h"
#include "semantics/Semantic_step_proposal.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <cstdlib>

using namespace boost;
using namespace std;

namespace spear
{
    extern size_t LF_WORD_THRESHOLD;
}

namespace semantics
{
    /*------------------------------------------------------------
     * LOCAL TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef Semspear_tree::Symbol_list Symbol_list;
    typedef Semantic_step_proposal::Self_ptr Move_proposal_ptr;

    /*------------------------------------------------------------
     * STATIC MEMBER VARIABLE INITIALIZATION
     *------------------------------------------------------------*/
    bool Semspear_tree::VERBOSE = false;
    
    const boost::array<Token_map::Key_type, 2> punc = {{",", ":"}};
    const boost::array<Token_map::Key_type, 1> base_np = {{"NPB"}};
    const boost::array<Token_map::Key_type, 3> unnec = {{".", "''", "``"}};
    const boost::array<Token_map::Key_type, 1> coord = {{"CC"}};
    const boost::array<Token_map::Key_type, 2> acc_any = {{"UCP", "QP"}};

    const Symbol_list punctuation(punc.begin(), punc.end());
    const Symbol_list base_np_labels(base_np.begin(), base_np.end());
    const Symbol_list coordination(coord.begin(), coord.end());
    const Symbol_list accepts_any_coord(acc_any.begin(), acc_any.end());

    const Symbol_list& unnecessary_nodes()
    {
	static Symbol_list sl(unnec.begin(), unnec.end());
	return sl;
    }

    /*------------------------------------------------------------
     * DEFINITION OF EXTERNAL FREE FUNCTIONS
     *------------------------------------------------------------*/
    boost::tuple<Semspear_tree::Self_ptr, kjb::Vector>
    propose_new_tree(const Semspear_tree::Self_ptr source)
    {
	typedef kjb::Vector Forward_reverse_probs;
	ASSERT(source->head() != NULL);

	bool tree_is_altered = false;
	boost::shared_ptr<Semspear_tree> dest =
	    boost::make_shared<Semspear_tree>(source, false);
	Forward_reverse_probs forward_reverse_probs =
	    Semspear_tree::propose_new_associations(
		source, dest,
		source->semantic_root(), dest->semantic_root(),
		tree_is_altered);
	dest->update_event_views_recursively();
	return boost::make_tuple(dest, forward_reverse_probs);
    }

    void resample_event_tables(Semspear_tree::Self_ptr& source)
    {
        source->event_->resample_table_assignments();
        source->sem_event_->resample_table_assignments();
    }

    /*------------------------------------------------------------
     * LOCAL HELPER FUNCTIONS
     *------------------------------------------------------------*/
    namespace
    {
	bool is_in(
	    const Token_map::Key_type&  item,
	    const Symbol_list&          container
	    )
	{
	    return container.end() !=
		std::find(
		    container.begin(), container.end(),
		    item);
	}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

	template<typename Iter>
	void mark_target_if_coord_phrase(
	    const Iter&                 target,
	    const Iter&                 end,
	    const Token_map::Key_type&  hlabel,
	    bool                        head_is_terminal,
	    bool                        parent_accepts_any
	    )
	{
	    static const Semspear_tree::Label cc_code =
		Semspear_tree::nt_lexicon().encode("CC");
	    static const Semspear_tree::Label prn_code =
		Semspear_tree::nt_lexicon().encode("PRN");
	    if(*target == NULL)
	    {
		std::cerr << "Invalid target iterator in Semspear_tree::"
			  << "is_target_a_coord_phrase()" << std::endl;
		return;
	    }
	    if((*target) -> label() != cc_code)
	    {
		(*target) -> set_coord(false);
		return;
	    }
	    for(Iter it = boost::next(target); it != end; ++it)
	    {
		if(*it == NULL)
		{
		    std::cerr << "Invalid iterator in "
			      << "is_target_a_coord_phrase()"
			      << std::endl;
		    continue;
		}
		if(!((*it) -> is_punctuation()))
		{
		    if(parent_accepts_any)
		    {
			(*target) -> set_coord(true);
			return;
		    }
		    if((*it) -> label() == prn_code) continue;
		    Token_map::Key_type ilabel =
			Semspear_tree::nt_lexicon().decode((*it) -> label());
		    if((head_is_terminal && (*it) -> is_terminal())
		       || hlabel.substr(0,1) == ilabel.substr(0,1))
		    {
			(*target) -> set_coord(true);
			return;
		    }
		}
	    }
	    (*target) -> set_coord(false);
	    return;
	}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

	Semspear_tree::Self_ptr get_a_copy(
	    Semspear_tree::Self_ptr other,
	    bool learn = false
	    )
	{
	    if(other == NULL) return Semspear_tree::Self_ptr();
	    return Semspear_tree::Self_ptr(new Semspear_tree(other, learn));
	}
    };
    
    /*------------------------------------------------------------
     * MEMBER FUNCTION DEFINITION
     *------------------------------------------------------------*/
    
    Semspear_tree::Semspear_tree(
	const Role& role,
	const bool& learn,
        const bool& collins
	) : role_(role),
	    word_(new Word()),
	    label_(new Label()),
	    children_(),
	    head_(children_.end()),
	    semantic_tree_(Sem_tree_ptr()),
	    semantic_node_(null_semantic_terminal()),
	    semantic_codes_(Semantic_data_base::Hash_pair()),
	    step_code_(Step_code::IDENTITY),
	    event_(),
	    sem_event_(),
	    is_punc_(false),
	    is_coord_(false),
	    is_base_np_(false),
	    learn_(learn),
            collins_(collins)
    {}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Semspear_tree(
	const Semspear_tree::Word&  word,
	const Semspear_tree::Label& label,
	const Role&                 role,
	const bool&                 learn,
        const bool&                 collins
	) : role_(role),
	    word_(new Word(word)),
	    label_(new Label(label)),
	    children_(),
	    head_(children_.end()),
	    semantic_tree_(Sem_tree_ptr()),
	    semantic_node_(null_semantic_terminal()),
	    semantic_codes_(Semantic_data_base::Hash_pair()),
	    step_code_(Step_code::IDENTITY),
	    event_(),
	    sem_event_(),
	    is_punc_(false),
	    is_coord_(false),
	    is_base_np_(false),
	    learn_(learn),
            collins_(collins)
    {
	word_freq_map()[*word_] += 1;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Semspear_tree(
	const Semspear_tree::Label& label,
	const Role&                 role,
	const bool&                 learn,
        const bool&                 collins
	) : role_(role),
	    word_(new Word()),
	    label_(new Label(label)),
	    children_(),
	    head_(children_.end()),
	    semantic_tree_(Sem_tree_ptr()),
	    semantic_node_(null_semantic_terminal()),
	    semantic_codes_(Semantic_data_base::Hash_pair()),
	    step_code_(Step_code::IDENTITY),
	    event_(),
	    sem_event_(),
	    is_punc_(false),
	    is_coord_(false),
	    is_base_np_(false),
	    learn_(learn),
            collins_(collins)
    {}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Semspear_tree(
        const Self_ptr other,
        const bool&    learn
        )
	: role_(other->role_),
	  word_(new Word(other->word())),
	  label_(new Label(other->label())),
	  children_(),
	  head_(children_.end()),
	  semantic_tree_(other->semantic_tree_),
	  semantic_node_(other->semantic_node_),
	  semantic_codes_(other->semantic_codes_),
	  step_code_(other->step_code_),
	  event_(),
	  sem_event_(),
	  is_punc_(other->is_punc_),
	  is_coord_(other->is_coord_),
	  is_base_np_(other->is_base_np_),
	  learn_(learn),
          collins_(other->collins_)
    {
	if(other->event_ != NULL)
	{
	    event_ = other->event_->get_a_copy(learn);
	}
	if(other->sem_event_ != NULL)
	{
	    sem_event_ = other->sem_event_->get_a_copy(learn);
	}
	std::transform(
	    other->children().begin(),
	    other->children().end(),
	    std::back_inserter(children_),
	    boost::bind(get_a_copy, _1, learn)
	    );
	head_ = children_.begin();
	for(Child_list::const_iterator it = other->children().begin();
	    it != other->head_; ++it)
	{
	    head_++;
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::~Semspear_tree()
    {
	if(learn_ == true && word_ != NULL) word_freq_map()[*word_] -= 1;
    }
    
    Nonterminal_db& Semspear_tree::nt_lexicon()
    {
	static Nonterminal_db& ntlex = Syntactic_event::nt_lexicon();
	multi_encode(ntlex, unnecessary_nodes());
	return Syntactic_event::nt_lexicon();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::add_child(const Self_ptr& new_child, bool on_left)
    {
	if(on_left)
	{
	    children_.push_front(new_child);
	} else {
	    children_.push_back(new_child);
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Word Semspear_tree::head_word() const
    {
	ASSERT(head() != NULL || word_ != NULL);
	if(head() == NULL) return word();
	else return head() -> head_word();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Label Semspear_tree::head_tag() const
    {
	ASSERT(head() != NULL || label_ != NULL);
	if(head() == NULL) return label();
	else return head() -> head_tag();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Semspear_tree::Elab_ptr_const Semspear_tree::semantic_root() const
    {
	if(semantic_tree_ == NULL)
	    std::cerr << "Null semantic tree!" << std::endl;
	
	return semantic_tree_ -> root();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::set_word(const Semspear_tree::Word& word)
    {
	if(word_ == NULL)
	{
	    word_ = boost::make_shared<Word>(word);
	} else {
	    if(learn_ == true) word_freq_map()[*word_] -= 1;
	    *word_ = word;
	}
	if(learn_ == true) word_freq_map()[*word_] += 1;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::set_label(const Semspear_tree::Label& label)
    {
	if(label_ == NULL) label_ = boost::make_shared<Label>(label);
	else *label_ = label;
	if(is_in(nt_lexicon().decode(label), punctuation)) role_ = PUNCTUATION;
	if(is_in(nt_lexicon().decode(label), base_np_labels)) is_base_np_ = true;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::set_head(
	const Semspear_tree::Child_list::const_iterator new_head)
    {
	head_ = children_.begin();
	while(head_ != new_head) head_++;
	if(head() != NULL)
	{
	    head() -> set_role(role_ == TOP ? ROOT : UNARY);
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::set_semantic_tree(const Sem_tree_ptr new_tree)
    {
	ASSERT(head() != NULL);
	semantic_tree_ = new_tree;
	ASSERT(semantic_tree_->root() != NULL);
	set_semantic_data(semantic_tree_->root(), Step_code::IDENTITY);
	// head()->update_semantic_association(semantic_node_);
	head()->set_semantic_data(semantic_node_, Step_code::IDENTITY);
	head()->head()->update_semantic_association(semantic_node_);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::set_semantic_data(
	const Elab_ptr_const semantic_parent,
	const Step_code_t&  step_code
	)
    {
	boost::tie(semantic_node_, semantic_codes_) =
	    Elaboration_tree::take_a_step(
		semantic_parent,
		step_code
		);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::add_stops()
    {
	if(children_.empty()) return;
	if(children_.front() -> label() != 0)
	{
	    Self_ptr lstop =
		boost::make_shared<Semspear_tree>(0, 0, DEPENDENCY, learn_);
	    add_child(lstop, true);
	}
	if(children_.back() -> label() != 0)
	{
	    Self_ptr rstop =
		boost::make_shared<Semspear_tree>(0, 0, DEPENDENCY, learn_);
	    add_child(rstop, false);
	}
	std::for_each(
	    children_.begin(), children_.end(),
	    boost::bind(&Semspear_tree::add_stops, _1)
	    );
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::update_semantic_association(
        Elab_ptr_const semantic_parent,
        bool           resample_step_codes
        )
    {
    	ASSERT(semantic_parent != NULL);
        if(resample_step_codes)
        {
            Step_code::Weights::Data_type b = {{0,1,1,1,1,1,1}};
            Step_code::Weights f = semantic_parent->step_filter();
            Step_code::Weights w(b);
            w *= f;
            Semantic_step_proposal::Proposal_dist weights(w.begin(), w.end());
            step_code_ = Step_code::codes[weights(Semantic_step_proposal::urng)];
    	std::cerr << enum_to_string<Step_code_t>(step_code_)
    		  << " : "
    		  << weights.probabilities()[static_cast<int>(step_code_)]
    		  << std::endl;
        }
    	tie(semantic_node_, semantic_codes_) =
    	    Elaboration_tree::take_a_step(semantic_parent, step_code_);
    	ASSERT(semantic_node_ != NULL);
    	std::for_each(
    	    children_.begin(),
    	    children_.end(),
    	    boost::bind(
    		&Semspear_tree::update_semantic_association,
    		_1, semantic_node_, resample_step_codes
    		)
    	    );
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    kjb::Vector Semspear_tree::propose_new_associations(
	const Self_ptr                source,
	Self_ptr&                     dest,
	const Elab_ptr_const          source_semantic_parent,
	const Elab_ptr_const          dest_semantic_parent,
	const bool                    tree_is_altered
	)
    {
	typedef kjb::Vector Result;
	bool altered = tree_is_altered;
	Result result(0.0, 0.0);
	if(source->role_ == UNARY || source->role_ == DEPENDENCY)
	{
	    result =
		Result(
		    dest->resample_semantic_move(
			dest_semantic_parent,
			altered),
		    source->evaluate_reverse_move(
			source_semantic_parent,
			tree_is_altered)
		    );
	}
	if(dest->head() != NULL)
	{
	    std::vector<double> child_probs;
	    dest->head()->
		update_semantic_context(
		    dest->semantic_codes(),
		    dest->semantic_codes()
		    );
	    result +=
		Semspear_tree::propose_new_associations(
		    source->head(), *(dest->head_),
		    source->semantic_node_, dest->semantic_node_,
		    altered
		    );
	    result +=
		Semspear_tree::propose_associations_in_child_range(
		    source, dest,
		    source->head_riterator(), source->children_.rend(),
		    dest->head_riterator(), dest->children_.rend(),
		    altered
		    );
	    result +=
		Semspear_tree::propose_associations_in_child_range(
		    source, dest,
		    source->head_iterator(), source->children_.end(),
		    dest->head_iterator(), dest->children_.end(),
		    altered
		    );
	}
	return result;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::update_event_views_recursively()
    {
	event_->update_event_views();
	sem_event_->update_event_views();
	std::for_each(
	    children_.begin(), children_.end(),
	    boost::bind(&Semspear_tree::update_event_views_recursively, _1)
	    );
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::rebuild_events_recursively(
	Semspear_tree* const       parent,
	Semspear_tree* const       sister_head,
	const int&                 dist_code,
	bool                       punc_flag,
	const Node_data&           punc_data,
	bool                       coord_flag,
	const Node_data&           coord_data,
	const int&                 depth
	)
    {
	update_local_event(
	    parent,
	    sister_head,
	    dist_code,
	    punc_flag,
	    punc_data,
	    coord_flag,
	    coord_data,
	    depth
	    );

	std::string lead(4 * depth, ' ');
	
        /// Recurse down the tree
	if(!children_.empty())
	{
	    // Rebuild event for head child
	    if(VERBOSE)
	    {
		cerr << lead << "Head subtree:" << endl;
	    }
	    head() ->
		rebuild_events_recursively(
		    this,
		    head().get(),
		    0,
		    false,
		    Node_data(0, 0, 0),
		    false,
		    Node_data(0, 0, 0),
		    depth + 1
		    );
	
	    // Rebuild events for left children
	    if(VERBOSE)
	    {
		cerr << lead << "Rebuilding left children:"
			  << endl;
	    }
	    
	    update_events_in_child_range(
		Child_list::const_reverse_iterator(head_riterator()),
		Child_list::const_reverse_iterator(children_.rend()),
		100,
		depth
		);
	
	    // Rebuild events for right children
	    if(VERBOSE)
	    {
		cerr << lead << "Rebuilding right children:"
		     << endl;
	    }

	    update_events_in_child_range(
		Child_list::const_iterator(head_iterator()),
		Child_list::const_iterator(children_.end()),
		000,
		depth
		);
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    double Semspear_tree::node_log_probability() const
    {
	return event_->log_probability(collins_);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    double Semspear_tree::subtree_log_probability(
#ifdef USE_SEMANTICS
	const Elab_ptr_const semantic_parent
#endif
	) const
    {
	if(event_ == NULL)
	{
	    cerr << "Tree has no associated events."
		 << endl;
	    return 0;
	}
	// count the probability from the syntactic event
	double return_val = event_ -> log_probability(collins_);
	std::list<double> child_probs(children_.size());
#ifdef USE_SEMANTICS
	ASSERT(semantic_parent != NULL);
	// count probability from the semantic event
	return_val += sem_event_->log_probability(collins_);
	// recursively gather subtree probabilities
	std::transform(
	    children_.begin(), children_.end(),
	    child_probs.begin(),
	    boost::bind(
		&Semspear_tree::subtree_log_probability,
		_1, semantic_node_)
	    );
#else
	// recursively gather subtree probabilities
	std::transform(
	    children_.begin(), children_.end(),
	    child_probs.begin(),
	    boost::bind(&Semspear_tree::subtree_log_probability, _1)
	    );
#endif
	return std::accumulate(
	    child_probs.begin(), child_probs.end(),
	    return_val
	    );
    }
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::print_constituency_tree(
	ostream& os,
	bool          is_head, 
	int           indent_level
	) const
    {

	if(is_terminal() && word() != 0)
	{
	    os << string(4*indent_level, ' ')
	       << nt_lexicon().decode(label())
	       << " " << lexicon().decode(word())
	       << endl;
	    if(is_head) os << " *";
	} else if(word() != 0) {
	    os << string(4*indent_level, ' ')
	       << nt_lexicon().decode(label())
	       << endl;
	    if(is_head) os << " *";
	    for(Child_list::const_iterator it = children_.begin();
		it != children_.end(); it++)
	    {
		if(*it == NULL)
		{
		    std::cerr << "Invalid iterator in Semspear_tree::"
			      << "process_npbs" << std::endl;
		} else {
		    os << endl;
		    bool h = false;
		    if(head() == (*it)) h = true;
		    (*it) -> print_constituency_tree(os, h, indent_level + 1);
		}
	    }
	}
    }
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::print_dependency_tree(
	ostream& os,
	bool is_head,
	int indent_level
	) const
    {
	if(is_terminal() && (word() == 0 || is_punc_)) return;
	string lead(4*indent_level, ' ');
	os << lead;
	if(event_ != NULL)
	{
	    event_ -> print(os);
	}
	else if(is_terminal()) {
	    if(is_head) os << "*";
	    os << nt_lexicon().decode(label()) << " "
	       << lexicon().decode(word());
	} else {
	    if(is_head) os << "*";
	    os << nt_lexicon().decode(label()) << "("
	       << nt_lexicon().decode(head_tag()) << ","
	       << lexicon().decode(head_word()) << ")";
	}
	if(sem_event_ != NULL && event_ != NULL)
	{
	    event_ -> print_semantics(os);
	}
	os << std::endl;
	for(Child_list::const_iterator it = children_.begin();
	    it != children_.end();
	    it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "print_dependency_tree" << std::endl;
	    } else {
		bool h = false;
		if(*it == head()) h = true;
		(*it) -> print_dependency_tree(os, h, indent_level + 1);
	    }
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::print_subtree_view_counts(ostream& os) const
    {
	event_ -> print_view_counts(os);
	for(Child_list::const_iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "print_subtree_view_counts()" << std::endl;
	    } else {
		(*it) -> print_subtree_view_counts(os);
	    }
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::print_events_with_probabilities(
        std::ostream& os
        ) const
    {
	if(sem_event_ != NULL)
	{
	    os << sem_event_ << " : " << sem_event_->log_probability(collins_)
	       << std::endl;
	}
	if(event_ != NULL)
	{
	    os << event_ << " : " << event_->log_probability(collins_) << std::endl;
	}
	for(Child_list::const_iterator it = children_.begin();
	    it != children_.end(); ++it)
	{
	    if(*it != NULL) (*it) -> print_events_with_probabilities(os);
	}
    }
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    bool Semspear_tree::contains_verb() const
    {
	static const string v_str = "VB";
	if(is_in(nt_lexicon().decode(label()), base_np_labels)) return false;
	if(is_terminal() && nt_lexicon().decode(label()).substr(0, 2) == v_str)
	{
	    return true;
	}
	for(Child_list::const_iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "contains_verb()" << std::endl;
	    } else if((*it) -> contains_verb()) {
		return true;
	    }
	}
	return false;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    bool Semspear_tree::should_be_npb() const
    {
	static const Label np_code = nt_lexicon().encode("NP");
	static const Label npb_code = nt_lexicon().encode("NPB");
	static const Label pos_code = nt_lexicon().encode("POS");
	if(label() != np_code) return false;
	for(Child_list::const_iterator it = children_.begin();
	    it != children_.end();
	    it++ )
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "should_be_npb" << std::endl;
	    } else if(((*it) -> label() == np_code ||
		       (*it) -> label() == npb_code) &&
		      ((*it) -> children().empty() == true ||
		       (*it) -> children().back() -> label() != pos_code)) {
		return false;
	    }
	}
	return true;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::process_npbs()
    {
	static const Label npb_code = nt_lexicon().encode("NPB");
	for(Child_list::iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "process_npbs" << std::endl;
	    } else {
		(*it) -> process_npbs();
	    }
	}

	if(should_be_npb()) set_label(npb_code);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::insert_npb_parents()
    {
	static const Label np_code = nt_lexicon().encode("NP");
	static const Label npb_code = nt_lexicon().encode("NPB");
	bool at_head = false;
	if(label() != npb_code)
	{
	    for(Child_list::iterator it = children_.begin();
		it != children_.end(); )
	    {
		if(*it == NULL)
		{
		    std::cerr << "Invalid iterator in Semspear_tree::"
			      << "insert_npb_parents()" << std::endl;
		} else {
		    if((*it) -> label() == npb_code &&
		       label() != np_code)
		    {
			if((*it) == head()) at_head = true;
			Self_ptr np =
			    boost::make_shared<Semspear_tree>(DEPENDENCY, learn_);
			Self_ptr npb = *it;
			np -> set_label(np_code);
			np -> add_child(npb);
			np -> set_head(np -> children_.begin());
			it = children_.erase(it);
			children_.insert(it, np);
			if(at_head) set_head(it);
		    } else {
			it++;
		    }
		}
	    }
	}

	for(Child_list::iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "insert_npb_parents()" << std::endl;
	    } else {
		(*it) -> insert_npb_parents();
	    }
	}
    }
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::prune_unnecessary_nodes()
    {
	for(Child_list::iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "prune_unnecessary_nodes" << std::endl;
	    } else {
		(*it) -> prune_unnecessary_nodes();
	    }
	}
	for(Child_list::iterator it = children_.begin();
	    it != children_.end(); )
	{
	    if(is_in(nt_lexicon().decode((*it)->label()), unnecessary_nodes()))
	    {
		// in case head_ points to an unecessary node, advance
		// it to avoid a dangling iterator (not sure why this
		// happens at all in the first place --- bug in Collins?)
		if(it == head_) head_++;
		it = children_.erase(it);
	    } else {
		it++;
	    }
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::raise_punctuation(
	Child_list&           siblings,
	Child_list::iterator  begin,
	Child_list::iterator  end
	)
    {
	for(Child_list::iterator it = children_.begin();
	    it != children_.end(); it++)
	{
	    if(*it == NULL)
	    {
		std::cerr << "Invalid iterator in Semspear_tree::"
			  << "raise_punctuation()" << std::endl;
	    } else {
		Child_list::iterator next = it;
		next++;
		(*it) -> raise_punctuation(children_, it, next);
	    }
	}

	while(!children_.empty())
	{
	    Self_ptr front = children_.front();
	    if(front -> is_punctuation())
	    {
		siblings.insert(begin, front);
		children_.pop_front();
	    } else {
		break;
	    }
	}

	while(!children_.empty())
	{
	    Self_ptr back = children_.back();
	    if(back -> is_punctuation())
	    {
		siblings.insert(end, back);
		children_.pop_back();
	    } else {
		break;
	    }
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::mark_coordinated_phrase_children()
    {
	if(head() == NULL) return;
	Token_map::Key_type hlabel = nt_lexicon().decode(head() -> label());
	bool head_is_terminal = head() -> is_terminal();
	bool accepts_any = is_in(nt_lexicon().decode(label()), accepts_any_coord);
	Child_list::const_iterator cend = children_.end();
	Child_list::const_reverse_iterator crend = children_.rend();
	for(Child_list::const_reverse_iterator it =
		boost::next(head_riterator());
	    it != crend; ++it)
	{
	    mark_target_if_coord_phrase(
		it, crend, hlabel, head_is_terminal, accepts_any
		);
	}
	for(Child_list::const_iterator it =
		boost::next(head_iterator());
	    it != cend; it++)
	{
	    mark_target_if_coord_phrase(
		it, cend, hlabel, head_is_terminal, accepts_any
		);
	}
	std::for_each(
	    children_.begin(), children_.end(),
	    boost::bind(
		&Semspear_tree::mark_coordinated_phrase_children,
		_1
		)
	    );
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::preprocess_tree()
    {
	prune_unnecessary_nodes();
	process_npbs();
	insert_npb_parents();
	Child_list punctuation;
	raise_punctuation(
	    punctuation,
	    punctuation.begin(),
	    punctuation.end()
	    );
	mark_coordinated_phrase_children();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 
    
    double Semspear_tree::resample_semantic_move(
	const Elab_ptr_const&   semantic_parent,
	bool&                   altered
	)
    {
	if(VERBOSE)
	{
	    std::cerr << "Sampling with event " << event_
		      << " and sem_event " << sem_event_
		      << std::endl;
	}
	Move_proposal_ptr move =
	    boost::make_shared<Semantic_step_proposal>();
	altered =
	    move->propose(
		semantic_parent, sem_event_, event_,
		step_code_, altered, collins_
		);
	boost::tie(step_code_, semantic_node_, sem_event_, event_)
	    = move->state();
	reacquire_event_counts();
	return log(move->prob());
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    double Semspear_tree::evaluate_reverse_move(
	const Elab_ptr_const&  semantic_parent,
	const bool             tree_is_altered
	) const
    {
	return log(
	    Semantic_step_proposal::evaluate_a_proposal(
		semantic_parent,
		sem_event_,
		event_,
		step_code_,
		tree_is_altered,
                collins_
		)
	    );
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::update_local_event(
	Semspear_tree* const parent,
	Semspear_tree* const sister_head,
	const int&           dist_code,
	bool                 punc_flag,
	const Node_data&     punc_data,
	bool                 coord_flag,
	const Node_data&     coord_data,
	const int&           depth
	)
    {
	// Collect the relevant info
	const Label tag = head_tag();
	const Word word = head_word();
	const Word proxy_word =
	    (lf_word_map()[word] == false && word != 0) ?
	    Lexicon_db::UNKNOWN_TOKEN_VAL : word;
	const Label plabel = ((parent != NULL) ? parent -> label() : 0);
	const Label hlabel = sister_head -> label();
	const Label htag = sister_head -> head_tag();
	const Word hword = sister_head -> head_word();
	Syntactic_event::Node_data node_proxy(proxy_word, tag, label());
	Syntactic_event::Node_data node_data(word, tag, label());
	Syntactic_event::Node_data parent_data(hword, htag, plabel);
	Syntactic_event::Node_data head_data(hword, htag, hlabel);
#ifdef USE_SEMANTICS	
	Elaboration_tree::Hash_pair sem_pair, psem_pair, hsem_pair;
	sem_pair = semantic_codes();
	if(parent != NULL) psem_pair = parent -> semantic_codes();
	if(sister_head != NULL) hsem_pair = sister_head -> semantic_codes();
#endif

	std::string lead(4 * depth, ' ');
	if(VERBOSE)
	{
	    cerr << lead << "This node is "
		 << nt_lexicon().decode(label()) << "("
		 << nt_lexicon().decode(tag) << ", "
		 << lexicon().decode(word) << ")"
		 << endl;
	}
	
        // Build a new event at this node
	switch(role_)
	{
	case PUNCTUATION: // build a punctuation event
	    if(VERBOSE) cerr << lead << "Building punctuation event.";
	    event_ = boost::make_shared<Punctuation_event>(0);
	    sem_event_ = boost::make_shared<Null_semantic_event>();
	    break;
	case COORD:
	    if(VERBOSE) cerr << lead << "Building coordination event.";
	    event_ = boost::make_shared<Punctuation_event>(0);
	    sem_event_ = boost::make_shared<Null_semantic_event>();
	    break;
	case TOP:
	    event_ = boost::make_shared<TOP_event>(0);
	    sem_event_ = boost::make_shared<Null_semantic_event>();
	    break;
	case ROOT:
	    if(VERBOSE) cerr << lead << "Building root event: ";
#ifdef USE_SEMANTICS
	    event_ =
		boost::make_shared<Root_event>(node_proxy, sem_pair, 0, learn_);
	    sem_event_ =
		boost::make_shared<Head_semantic_event>(
		    step_code_,
		    parent->semantic_node_->type_code(),
		    parent_data, psem_pair, learn_
		    );
#else
	    event_ = boost::make_shared<Root_event>(node_proxy, 0, learn_);
#endif
	    break;
	case UNARY:
	    ASSERT(parent != NULL);
	    if(VERBOSE)
	    {
		cerr << lead << "Building unary event: ";
		cerr << lead << "Parent is "
		     << nt_lexicon().decode(plabel) << "("
		     << nt_lexicon().decode(htag) << ", "
		     << lexicon().decode(hword) << ")"
		     << endl;
	    }
#ifdef USE_SEMANTICS
	    event_ =
		boost::make_shared<Unary_event>(
		    node_data, sem_pair, psem_pair, plabel, 0, learn_
		    );
	    sem_event_ =
		boost::make_shared<Head_semantic_event>(
		    step_code_,
		    parent->semantic_node_->type_code(),
		    parent_data, psem_pair, learn_
		    );
#else
	    event_ =
		boost::make_shared<Unary_event>(node_data, plabel, 0, learn_);
#endif
	    break;
	case DEPENDENCY:
	    ASSERT(parent != NULL && sister_head != this);
	    if(VERBOSE)
	    {
		cerr << lead << "Building dependency event: ";
		cerr << lead << "Parent is "
		     << nt_lexicon().decode(plabel) << "("
		     << nt_lexicon().decode(htag) << ", "
		     << lexicon().decode(hword) << ")"
		     << endl;
		cerr << lead << "Head is "
		     << nt_lexicon().decode(hlabel) << "("
		     << nt_lexicon().decode(htag) << ", "
		     << lexicon().decode(hword) << ")"
		     << endl;
	    }
#ifdef USE_SEMANTICS
	    event_ = Syn_event_ptr(
		new Dependency_event(
		    node_proxy, parent_data, head_data,
		    sem_pair, psem_pair, hsem_pair,
		    dist_code,
		    punc_flag, punc_data,
		    coord_flag, coord_data,
		    0, learn_
		    )
		);
	    sem_event_ =
		boost::make_shared<Mod_semantic_event>(
		    step_code_,
		    parent->semantic_node_->type_code(),
		    parent_data, head_data,
		    psem_pair, hsem_pair, dist_code, learn_
		    );
#else
	    event_ = Syn_event_ptr(
		new Dependency_event(
		    node_proxy, parent_data, head_data,
		    dist_code,
		    punc_flag, punc_data,
		    coord_flag, coord_data,
		    0, learn_
		    )
		);
#endif
	case NUM_ROLES:
	default:
	    break;
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Semspear_tree::update_semantic_context(
	const Hash_pair& parent_semantics,
	const Hash_pair& head_semantics
	)
    {
	if(event_ != NULL)
	{
	    event_->update_semantic_context(
		parent_semantics,
		head_semantics
		);
	}
	if(sem_event_ != NULL)
	{
	    sem_event_->update_semantic_context(
		parent_semantics,
		head_semantics
		);
	}
    }
};
