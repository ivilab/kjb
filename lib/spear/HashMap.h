#ifndef HASH_MAP_H_
#define HASH_MAP_H_

/*!
 * @file HashMap.h
 *
 * @author Mihai Surdeanu
 * $Id: HashMap.h 17380 2014-08-22 22:01:21Z cdawson $ 
 */

// #include <ext/hash_map>
#include <boost/unordered_map.hpp>

namespace spear {

  template <class K, class V, class F, class E>
  // class HashMap : public __gnu_cxx::hash_map<K, V, F, E>
  class HashMap : public boost::unordered_map<K, V, F, E>
    {
    };

} // end namespace spear

#endif
