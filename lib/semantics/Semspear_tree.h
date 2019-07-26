#ifndef SEMSPEAR_TREE_H_
#define SEMSPEAR_TREE_H_

/*!
 * @file Semspear_tree.h
 *
 * @author Colin Dawson 
 * $Id: Semspear_tree.h 17357 2014-08-22 00:12:57Z cdawson $ 
 */

#include "m_cpp/m_vector.h"
#include "semantics/Syntactic_event.h"
#include "semantics/Semantic_step_event.h"
#include "semantics/Elaboration_tree.h"
#include "semantics/SemanticIO.h"
#include "semantics/Event_parser.h"
#include <boost/make_shared.hpp>
#include <list>
#include <map>


namespace semantics
{

    class Semantic_step_proposal;
    
    void initialize_special_symbols();
    
    /** @class Semspear_tree
     *  @brief The main syntactic tree object for semantic parsing
     */
    class Semspear_tree
    {
    public:
	/*------------------------------------------------------------
	 * TYPEDEFS
	 *------------------------------------------------------------*/
	typedef boost::shared_ptr<Semspear_tree> Self_ptr;
	typedef Syntactic_event::Word_type Word;
	typedef boost::shared_ptr<Word> Word_ptr;
	typedef Syntactic_event::Label_type Label;
	typedef boost::shared_ptr<Label> Label_ptr;
	typedef std::list<Self_ptr> Child_list;
	typedef Syntactic_event::Event_ptr Syn_event_ptr;
	typedef Semantic_step_event::Event_ptr Sem_event_ptr;
	typedef Syntactic_event::Node_data Node_data;
	typedef boost::shared_ptr<Elaboration_tree> Sem_tree_ptr;
	typedef Elaboration_tree::Elab_ptr Elab_ptr;
	typedef Elaboration_tree::Elab_ptr_const Elab_ptr_const;
	typedef Elaboration_tree::Hash_pair Hash_pair;
	typedef std::map<Lexicon_db::Val_type, bool> LF_map_t;
	typedef std::map<Word, size_t> Freq_map;
	typedef std::list<Token_map::Key_type> Symbol_list;
	
	enum Role {TOP, ROOT, UNARY, DEPENDENCY, PUNCTUATION, COORD, NUM_ROLES};

	/** @brief should debugging messages be printed?
	 */
	static bool VERBOSE;

    public:
	
	/*------------------------------------------------------------
	 * CONSTRUCTORS/DESTRUCTOR
	 *------------------------------------------------------------*/
	
	/** @brief default constructor, creates an empty tree
	 *  @param learn governs whether to update model using this tree
	 */
	Semspear_tree(
            const Role& role,
            const bool& learn = true,
            const bool& collins = false
            );
	
	/** @brief constructor for terminal nodes, which directly specify word
	 *  @param word the literal word at this node (terminals only)
	 *  @param label the non- or preterminal label
	 *  @param learn governs whether to update model using this tree
	 */
	Semspear_tree(
	    const Word&   word,
	    const Label&  label,
	    const Role&   role,
	    const bool&   learn = true,
            const bool&   collins = false
            );

        /** @brief constructor for nonterminals, which only specify label
	 */
	Semspear_tree(
	    const Label& label,
	    const Role&  role,
	    const bool&  learn = true,
            const bool&  collins = false
	    );

	/** @brief copy constructor
	 *  @remarks
	 *      copies self-contained data and creates new copies of child
	 *      trees (rather than simply copying the pointers).  Resulting
	 *      copy defaults to have learn_ = false so that any training
	 *      tree is only counted once.
	 */
	Semspear_tree(
            const Self_ptr other,
            const bool&    learn = false
            );

	/*! @brief decrements word count when destructed (if learn_ is true)
	 */
	~Semspear_tree();
	

	/*------------------------------------------------------------
	 * ACCESSORS
	 *------------------------------------------------------------*/
	
	Word word() const {if(word_ != NULL) return *word_; else return Word();}

        /** @brief returns the label field
	 */
	Label label() const
	{
	    if(label_ != NULL) return *label_;
	    else return Label();
	}

        /** @brief returns a pointer to the head subtree
	 */
	const Self_ptr head() const
	{
	    if(head_ == children_.end()) return Self_ptr();
	    else return *head_;
	}

        /** @brief returns the list of children
	 */
	const Child_list& children() const { return children_; }

        /** @brief get the head word for this subtree
	 */
	Word head_word() const;

        /** @brief get the head tag for this subtree
	 */
	Label head_tag() const;

	/** @brief get a pointer to the root of the associated semantic tree
	 */
	Elab_ptr_const semantic_root() const;

	/** @brief get the pair of semantic codes at the root of this subtree
	 */
	const Hash_pair& semantic_codes() const { return semantic_codes_; }

        /** @brief returns a pointer to the probabilistic event at the root
	 */
	const Syn_event_ptr syntactic_event() const { return event_; }
		
	/*------------------------------------------------------------
	 * QUERIES
	 *------------------------------------------------------------*/
	
	/*! @brief is this subtree a leaf?
	 */
	bool is_terminal() const {return children_.empty() && word_ != NULL;}

	/*! @brief is this a punctuation node?
	 */
	bool is_punctuation() const {return is_punc_;}

	/*! @brief is this a coordinating conjunction?
	 */
	bool is_coordination() const {return is_coord_;}

	/*------------------------------------------------------------
	 * CALCULATION
	 *------------------------------------------------------------*/
	
	/*! @brief compute log prob of this node by itself
	 */
	double node_log_probability() const;

	/*! @brief compute log prob of this tree according to events
	 */
#ifdef USE_SEMANTICS
	double subtree_log_probability(
            const Elab_ptr_const semantic_parent
            ) const;
#else
	double subtree_log_probability() const;
#endif

	/*------------------------------------------------------------
	 * MEMBER ASSIGNMENT
	 *------------------------------------------------------------*/
	
	/*! @brief assign a new value to the word field
	 */
	void set_word(const Word& word);

        /*! @brief assign a new value to the label field
	 */
	void set_label(const Label& label);

        /*! @brief assign a new head and acquire its info
	 */
	void set_head(const Child_list::const_iterator head);

	/*! @brief set the syntactic role of this node
	 */
	void set_role(const Role& role) { role_ = role; }

	/*! @brief set the coordinated phrase flag
	 */
	void set_coord(const bool new_value)
	{
	    is_coord_ = new_value;
	    if(is_coord_ && !is_base_np_) set_role(COORD);
	}
	
	/*! @brief associate this syntactic tree w/ a new semantic tree
	 */
	void set_semantic_tree(const Sem_tree_ptr new_tree);

	/*! @brief set semantic data for this node
	 */
	void set_semantic_data(
	    const Elab_ptr_const semantic_parent,
	    const Step_code_t&   step_code
	    );
	

	/*------------------------------------------------------------
	 * MANIPULATION
	 *------------------------------------------------------------*/
	
        /*! @brief append a new child subtree
	 */
	void add_child(const Self_ptr& new_child, bool on_left = false);
	
	/*! @brief do some preprocessing in accordance with Collins' model
	 */
	void preprocess_tree();

	/*! @brief signals that tree is fully built, and to process data
	 */
	void complete_tree()
	{
	    add_stops();
	    rebuild_events_recursively(NULL, this);
	}

	void release_event_counts()
	{
	    if(event_ != NULL) event_ -> release_view_counts();
	    if(sem_event_ != NULL) sem_event_ -> release_view_counts();
	    learn_ = false;
	    std::for_each(
		children_.begin(), children_.end(),
		boost::bind(&Semspear_tree::release_event_counts, _1)
		);
	}

	void reacquire_event_counts()
	{
	    if(event_ != NULL) event_->reacquire_view_counts();
	    if(sem_event_ != NULL) sem_event_->reacquire_view_counts();
	    learn_ = true;
	}

	void reacquire_event_counts_recursively()
	{
	    reacquire_event_counts();
	    std::for_each(
		children_.begin(), children_.end(),
		boost::bind(
		    &Semspear_tree::reacquire_event_counts_recursively,
		    _1)
		);
	}

	void update_event_views_recursively();

	/*------------------------------------------------------------
	 * DISPLAY
	 *------------------------------------------------------------*/
	
	/*! @brief print as constituency tree (marking head)
	 */
	void print_constituency_tree(
	    std::ostream& os,
	    bool          is_head = false,
	    int           indent_level = 0
	    ) const;

	/*! @brief display as dependency tree
	 */
	void print_dependency_tree(
	    std::ostream& os,
	    bool          is_head = false,
	    int           indent_level = 0
	    ) const;

	/*! @brief print events and associated log probabilities
	 */
	void print_events_with_probabilities(std::ostream& os) const;

	/*! @brief print count info at each node
	 */
	void print_subtree_view_counts(std::ostream& os) const;


	/*------------------------------------------------------------
	 * FRIEND FUNCTIONS
	 *------------------------------------------------------------*/
	
	friend void initialize_special_symbols();

	friend boost::tuple<Self_ptr, kjb::Vector> propose_new_tree(
	    const Self_ptr source
	    );

        friend void resample_event_tables(Self_ptr& source);
	
    public:
	/*------------------------------------------------------------
	 * STATIC ACCESSORS
	 *------------------------------------------------------------*/

	/** @brief return a reference to the underlying terminal lexicon
	 */
	static Lexicon_db& lexicon() {return Syntactic_event::lexicon();}

	/** @brief return a reference to the underlying nonterminal lexicon
	 */
	static Nonterminal_db& nt_lexicon();

	/** @brief return a reference to the map indexing whether words are rare
	 */
	static LF_map_t& lf_word_map() {return Event_db::lf_word_map();}

	/** @brief return a reference to the map indexing actual word frequencies
	 */
	static Freq_map& word_freq_map()
	{
	    static boost::shared_ptr<Freq_map> wfm(new Freq_map());
	    return *wfm;
	};

    private:


	/*! @brief return a (rightward) iterator to the head child
	 */
	Child_list::iterator head_iterator()
	{
	    return head_;
	}

        /*! @brief return a (leftward) iterator to the head child
	 */
	Child_list::reverse_iterator head_riterator()
	{
	    return std::reverse_iterator<Child_list::iterator>(head_);
	}

	/*! @brief add stop nodes
	 */
	void add_stops();

	/*! @brief resample semantic move and return proposal probability
	 *  @return the forward probability of the local proposal
	 */
	double resample_semantic_move(
	    const Elab_ptr_const& semantic_parent,
	    bool&                 altered
	    );

	/*! @brief get the probability of sampling this node's sem. association
	 */
	double evaluate_reverse_move(
	    const Elab_ptr_const&  semantic_parent,
	    const bool             tree_is_altered
	    ) const;

    private:

	/*------------------------------------------------------------
	 * UPDATE-RELATED HELPERS
	 *------------------------------------------------------------*/
	
	/** @brief update the event at the root
	 *  @param parent a pointer to the parent node
	 *  @param sister_head a pointer to the sister node representing the head
	 *  @param dist_code an integral code for Collins' "distance" variable
	 *  @param punc_flag boolean representing whether there is punctuation
	 *         between this node and the sister head
	 *  @param punc_data if punc_flag == true, then this parameter contains
	 *         the tag and word associated with the intervening punctuation
	 *  @param coord_data boolean representing whether there is a
	 *         coordinating conjunction between this node and its sister head
	 *  @param depth the number of levels this node is below the root
	 */
	void update_local_event(
	    Semspear_tree*   parent,
	    Semspear_tree*   sister_head,
	    const int&       dist_code,
	    bool             punc_flag,
	    const Node_data& punc_data,
	    bool             coord_flag,
	    const Node_data& coord_data,
	    const int&       depth
	    );

	/** @brief sends messages to associated events to update semantic history
	 *  @param parent_semantics the hash code pair for the new semantic
	 *         node associated with the syntactic parent
	 *  @param head_semantics the hash code pair for the new semantic
	 *         node associated with the sister head node
	 */
	void update_semantic_context(
	    const Hash_pair& parent_semantics,
	    const Hash_pair& head_semantics
	    );

	/** @brief run through and update the association to semantic events
         *  @param semantic_parent a pointer to the semantic node associated with
         *         this node's syntactic parent
         *  @param resample_step_codes flag indicating whether to resample the
         *         step type associated with this node (and its children).  If
         *         false, the existing steps will be applied to a (potentially)
         *         new subtree.  If true, steps are sampled uniformly from all
         *         legal steps (for the parent semantic node type)
	 */
	void update_semantic_association
        (
            Elab_ptr_const semantic_parent,
            bool           resample_step_codes = true   
        );
	
	/*! @brief read the tree and update event counting
	 *  @param parent pointer to parent of this node
	 *  @param sister_head smart pointer to head of parent
	 *  @param is_head is this node the head of its parent?
	 *  @param left_of_head is this node to the left of the head?
	 */
	void rebuild_events_recursively(
	    Semspear_tree* const  parent       = NULL,
	    Semspear_tree* const  sister_head  = NULL,
	    const int&            dist_code    = 0,
	    bool                  punc_flag    = false,
	    const Node_data&      punc_data    = Node_data(0, 0, 0),
	    bool                  coord_flag   = false,
	    const Node_data&      coord_data   = Node_data(0, 0, 0),
	    const int&            depth        = 0
	    );
	
	/** @brief recursively updates events to one side of the head child
	 *  @param start an iterator (forward or reverse) to the head child
	 *  @param either a forward iterator to children_.end(), or a reverse
	 *         iterator to children_.rend()
	 *  @param dist_base the "hundreds part" of the Collins' distance
	 *         variable, indicating whether the range is left (100) or right
	 *         (000) of the head child
	 *  @param depth the number of levels we are below the root
	 */
	template<typename Iter>
	void update_events_in_child_range(
	    const Iter& start,
	    const Iter& end,
	    const int&  dist_base,
	    const int&  depth
	    );

	/*------------------------------------------------------------
	 * PROPOSAL HELPERS
	 *------------------------------------------------------------*/

	/** @brief generates proposed semantic steps in a child range
	 *  @param source the original tree from which the proposals are moving
	 *  @param dest initially a copy of source, but modified via proposals
	 *  @param source_start an iterator to the beginning of the child range
	 *         within the source tree
	 *  @param source_end an iterator to the end of the child range
	 *         within the source tre
	 *  @param dest_start an iterator to the beginning of the child range
	 *         within the destination tree
	 *  @param dest_end an iterator to the end of the child range
	 *         within the destination tree
	 *  @param tree_is_altered a flag indicating whether any changes have
	 *         been made yet "up-tree".  Affects proposal probs.
	 */
	template<typename Iter>
	static kjb::Vector propose_associations_in_child_range(
	    const Self_ptr    source,
	    Self_ptr&         dest,
	    Iter              source_start,
	    Iter              source_end,
	    Iter              dest_start,
	    Iter              dest_end,
	    bool              tree_is_altered
	    );

	/** @brief recursively propose new associations throughout tree
	 *  @param source ptr to the original tree from which proposals move
	 *  @param dest initially a copy of source, but modified via proposals
	 *  @param source_semantic_parent the semantic node associated with
	 *         the syntactic parent of the source subtree
	 *  @param dest_semantic_parent the semantic node currently associated
	 *         with the syntactic parent of the dest subtree
	 *  @param tree_is_altered a flag indicating whether any changes have
	 *         been made yet "up-tree".  Affects proposal probs.
	 */
	static kjb::Vector propose_new_associations(
	    const Self_ptr        source,
	    Self_ptr&             dest,
	    const Elab_ptr_const  source_semantic_parent,
	    const Elab_ptr_const  dest_semantic_parent,
	    const bool            tree_is_altered
	    );
	
	/*------------------------------------------------------------
	 * PREPROCESSING HELPER FUNCTIONS
	 *------------------------------------------------------------*/
	
	/** @brief does this subtree contain a verb anywhere?
	 */
	bool contains_verb() const;
	
	/** @brief check to see whether this node should be an NPB
	 */
	bool should_be_npb() const;

        /** @brief dive down and remove terminals with certain predefined labels
	 */
	void prune_unnecessary_nodes();
	
	/** @brief recurse through tree and replace NPs with NPBs as appropriate
	 */
	void process_npbs();

        /** @brief recurse through tree and insert NPs above NPBs as needed
	 */
	void insert_npb_parents();

        /** @brief move punctuation nodes up the tree, following Collins
	 */
	void raise_punctuation(
	    Child_list&           siblings,
	    Child_list::iterator  begin,
	    Child_list::iterator  end
	    );

	/** @brief find children that are coordinated phrases and set their flags
	 */
	void mark_coordinated_phrase_children();

    private:
	Role role_; /*!< What type of Syntactic_event is event_? */
	Word_ptr word_; /*!< If a terminal, what word? */
	Label_ptr label_; /*!< What nonterminal label is here? */
	Child_list children_; /*!< Container of pointers to child subtrees */
	Child_list::iterator head_; /*!< Iterator in children_ to head*/
	Sem_tree_ptr semantic_tree_; /*!< Ptr to associated Elaboration_tree */
	Elab_ptr_const semantic_node_; /*!< Ptr to associated Semantic_elaboration */
	Hash_pair semantic_codes_; /*!< Hash codes associated w/ semantic_node_*/
	Step_code_t step_code_; /*!< Enum code for associated step event */
	Syn_event_ptr event_; /*!< Ptr to event representation*/
	Sem_event_ptr sem_event_; /*!< Ptr to semantic event*/
	bool is_punc_; /*!< Is this a punctuation node? */
	bool is_coord_; /*!< Is this a coordinating conjunction? */
	bool is_base_np_; /*!< Is this a NPB node? */
	bool learn_; /*!< Are the associated events counted for probabilities?*/
        bool collins_; /*!< Should probabilities be computed according to Collins?*/
    };

    /*------------------------------------------------------------
     * FREE FUNCTIONS
     *------------------------------------------------------------*/
    
    boost::tuple<Semspear_tree::Self_ptr, kjb::Vector>
    propose_new_tree(const Semspear_tree::Self_ptr source);

    void resample_event_tables(Semspear_tree::Self_ptr& source);

    /*------------------------------------------------------------
     * TEMPLATE MEMBER DEFINITIONS
     *------------------------------------------------------------*/
    
    template<typename Iter>
    void Semspear_tree::update_events_in_child_range(
	const Iter& start,
	const Iter& end,
	const int&  dist_base,
	const int&  depth
	)
    {
	Semspear_tree* hh = head().get();
	bool adjacent_to_head = true;
	bool has_intervening_verb = false;
	bool next_punc_flag = false;
	Node_data next_punc_data = Node_data(0, 0, 0);
	bool next_coord_flag = false;
	Node_data next_coord_data = Node_data(0, 0, 0);
	int dist = dist_base + 10*adjacent_to_head + 1*has_intervening_verb;
	for(Iter it = start; it != end; it++)
	{
	    if(*it == head()) continue;
	    (*it) ->
		rebuild_events_recursively(
		    this,
		    hh,
		    dist,
		    next_punc_flag,
		    next_punc_data,
		    next_coord_flag,
		    next_coord_data,
		    depth + 1
		    );
	    if(!((*it) -> is_punctuation())
	       && (!(*it) -> is_coordination() || is_base_np_))
	    {
		adjacent_to_head = false;
		next_punc_flag = false;
		next_punc_data = Node_data(0, 0, 0);
		next_coord_flag = false;
		next_coord_data = Node_data(0, 0, 0);
		if(is_base_np_)
		{
		    hh = (*it).get();
		    adjacent_to_head = true;
		    has_intervening_verb = false;
		} else if((*it) -> contains_verb())
		{
		    has_intervening_verb = true;
		}
	    } else if((*it) -> is_punctuation() && !next_punc_flag) {
		assert((*it) -> word() != 0);
		assert((*it) -> label() != 0);
		next_punc_flag = true;
		next_punc_data =
		    Node_data((*it) -> word(), (*it) -> label(), 0);
	    } else if((*it) -> is_coordination() && !next_coord_flag) {
		next_coord_flag = true;
		next_coord_data =
		    Node_data((*it) -> word(), (*it) -> label(), 0);
	    }
	    dist = dist_base + 10*adjacent_to_head + 1*has_intervening_verb;
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    template<typename Iter>
    kjb::Vector Semspear_tree::propose_associations_in_child_range(
	const Self_ptr source,
	Self_ptr&      dest,
	Iter           source_child_start,
	Iter           source_child_end,
	Iter           dest_child_start,
	Iter           dest_child_end,
	const bool     tree_is_altered
	)
    {
	typedef kjb::Vector Result;
	Result result(0.0, 0.0);
	Semspear_tree* hh = dest->head().get();
	Iter s_it = source_child_start, d_it = dest_child_start;
	for( ;
	    s_it != source_child_end && d_it != dest_child_end;
	    ++s_it, ++d_it)
	{
	    if((*s_it) == source->head() || (*s_it)->is_punctuation()) continue;
	    (*d_it)->update_semantic_context(
		dest->semantic_codes(),
		hh->semantic_codes()
		);
	    result +=
		Semspear_tree::propose_new_associations(
		    *s_it, *d_it,
		    source->semantic_node_,
		    dest->semantic_node_,
		    tree_is_altered
		    );
	    if(dest->is_base_np_)
	    {
		hh = (*d_it).get();
	    }
	}
	return result;
    }
};

#endif
