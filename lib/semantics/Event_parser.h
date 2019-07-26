#ifndef EVENT_PARSER_H_
#define EVENT_PARSER_H_

/*!
 * @file Event_parser.h
 *
 * @author Colin Dawson 
 * $Id: Event_parser.h 16947 2014-06-03 05:13:51Z cdawson $ 
 */

#include "semantics/Syntactic_event.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

#define PRINT(a) std::cout << #a << " = " << a << std::endl

namespace semantics
{

    class Lexicon_db;
    class Nonterminal_db;
    
    /*! @class Event_db
     *  @brief Database class to read and count parse tree events
     */
    class Event_db
    {
	// Pointer to generic Syntactic_event
	typedef boost::shared_ptr<Syntactic_event> Event_ptr;
	// Containers for syntactic event pointers
	typedef std::vector<Event_ptr> Event_list;
	// The low frequency word map
	typedef std::map<Lexicon_db::Val_type, bool> LF_map_t;
    public:

	/*! @brief default ctor, creates empty database
	 */
	Event_db() : event_list_(), tree_list_(), num_events_(0), learn_(false)
	{}
	
	/*! @brief constructs an event database object
	 *  @param event_file (path-to-)filename as string
	 *  @param learn if true, counts events and builds a probability model
	 */
	Event_db(
	    std::string event_file,
	    std::string lexicon_file,
	    bool        learn = true,
	    int         num_lines = 10000
	    ) : event_list_(), tree_list_(), num_events_(0), learn_(learn)
	{
	    read_lexicon(lexicon_file);
	    event_list_.push_back(Event_ptr());
	    read_events(event_file, num_lines);
	}

	/*! @brief gets pointer to tree in position @a i in the list
	 *  @return pointer to the root event of tree i
	 */
	Event_ptr get_tree(const int& i) const {return tree_list_[i];}

	/*! @brief print tree in position i to ostream os
	 */
	void print_tree(const int& i, std::ostream& os) const
	{
	    tree_list_[i] -> print_subtree(os);
	}

	/*! @brief returns number of trees in list
	 */
	size_t num_trees() const {return tree_list_.size();}

	/*! @brief returns number of events
	 */
	size_t num_events() const {return num_events_;}
    public:
        /*! @brief returns a reference to the map between words and codes
	 */
	static Lexicon_db& lexicon() {return Syntactic_event::lexicon();}

        /*! @brief returns a reference to the map between nonterminals and codes
	 */
	static Nonterminal_db& nt_lexicon()
	{
	    return Syntactic_event::nt_lexicon();
	}
	/*! @brief accessor to a global map flagging low frequency words
	 */
	static LF_map_t& lf_word_map()
	{
	    static boost::shared_ptr<LF_map_t> lfwm(new LF_map_t);
	    return *lfwm;
	}
	static bool VERBOSE;
    private:
	Event_list event_list_; /*!< contains ptrs to events being processed*/
	Event_list tree_list_;  /*!< container of ptrs to root nodes */
	size_t num_events_;     /*!< total events in complete trees */
	bool learn_;            /*!< does this contain training events? */
	
	/*! @brief read through lexicon file and initialize lf word map
	 */
	void read_lexicon(const std::string& lexicon_file);
	
	/*! @brief top-level function to process data contained in event_file
	 *  @param event_file path to a file in spear events format
	 *  @param num_lines maximum number of lines to read
	 */
	void read_events(const std::string& event_file, const int& num_lines);
	
	/*! @brief called by read_events repeatedly; reads one event
	 *  @param event_stream a single line from a spear events file
	 */
	bool read_event(std::istringstream& event_stream);
	
	/*! @brief reads event if type is f (called by read_event)
	 *  @return true if successful, false otherwise
	 */
	bool read_event_f(std::istringstream&)
	{
	    num_events_ += event_list_.size();
	    event_list_.clear();
	    return true;
	}
	
	/*! @brief reads event if type is u (unary: handles u and s types)
	 *  @param input a single line of a spear events file
	 *  @return true if successful, false otherwise
	 */
	bool read_event_u(std::istringstream& input);
	
	/*! @brief reads event if type is d (dependency event)
	 *  @param input a single line of a spear events file
	 *  @return true if successful, false otherwise
	 */
	bool read_event_d(std::istringstream& input);
    };

    /// Free functions
    
    Event_db process_event_file(
	std::string train_path,
	std::string lexicon_file,
	size_t      max_events,
	std::string test_path = ""
	);
};

#endif
