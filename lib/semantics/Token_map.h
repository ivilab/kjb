#ifndef TOKEN_MAP_H_
#define TOKEN_MAP_H_

/*!
 * @file Token_map.h
 *
 * @author Colin Dawson 
 * $Id: Token_map.h 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <algorithm>
#include <string>

namespace semantics
{

class Token_map
{
public:
    typedef std::string Key_type;
    typedef size_t Val_type;
    typedef boost::bimap<Key_type, Val_type> Map;
    typedef Map::value_type Key_val_pair;
    typedef Map::right_value_type Val_key_pair;
    typedef Map::const_iterator const_iterator;

    static const Val_type UNKNOWN_TOKEN_VAL;
    
    /*! @brief construct an empty token map
     */
    Token_map() : map_(), next_val_(UNKNOWN_TOKEN_VAL + 1) {}
    /*! @brief translate key into code
     *  @return returns code associated with key, with constant UNKNOWN code
     */
    virtual ~Token_map() {}

    /*! @brief look up code associated with key, or return UNKNOWN_TOKEN_VAL
     */
    virtual Val_type get_code(const Token_map::Key_type& key);
    
    /*! @brief look up code associated with key, adding it if not found
     */
    virtual Val_type encode(const Key_type& key, bool learn = false);
    /*! @brief look up key associated with code
     */
    virtual const Key_type& decode(const Val_type& val);
    /*! @brief gets the next unused code available (e.g., for UNKNOWN)
     */
    virtual Val_type next_val() {return next_val_;}
    friend std::ostream& operator<<(std::ostream& os, Token_map& map);
    
    const_iterator begin() {return map_.begin();}
    const_iterator end(){return map_.end();}
    
protected:
    virtual const Key_type& unknown_key() = 0;
    Map map_;
    Val_type next_val_;
};

    template<class T>
    void multi_encode(Token_map& map, const T& keys)
    {
	std::for_each(
	    keys.begin(), keys.end(),
	    boost::bind(&Token_map::encode, &map, _1, true)
	    );
    }

    template<class T, class U, class M>
    T multi_map_encode(M maps, U keys)
    {
	T vals;
	std::transform(
	    keys.begin(), keys.end(), maps.begin(), vals.begin(),
	    boost::bind(&Token_map::encode, _1, _2, false)
	    );
	return vals;
    }

    template<class U, class T, class M>
    U multi_map_decode(M maps, T vals)
    {
	U keys;
	std::transform(
	    vals.begin(), vals.end(), maps.begin(), keys.begin(),
	    boost::bind(&Token_map::decode, _2, _1)
	    );
	return keys;
    }
    
};
    
#endif
