#ifndef HEAD_H_
#define HEAD_H_

/*!
 * @file Head.h
 *
 * @author Mihai Surdeanu
 * $Id: Head.h 18301 2014-11-26 19:17:13Z ksimek $ 
 */

#include <list>
#include <string>

#include "spear/RCIPtr.h"

/* Kobus. */ 
/* The code expected this to be defined via compile line, but I am hard-coding
 * it in for now. If we need to switch it on the fly, we should think of a
 * different way than the compile line. Perhaps a runtime option. 
*/
#define LANG_ENGLISH

#ifdef LANG_ENGLISH
#include "spear/HeadEnglish.h"
#endif

namespace spear {

  /**
   * This function implements the head finding heuristics
   * The type T can be any edge type that implements the following methods:
   *   const std::wstring & T::getLabel() const
   */
  template <typename T>
    spear::RCIPtr<T> 
    findHead(const String & parent,
	     const std::list< spear::RCIPtr<T> > & children) {
#ifdef LANG_ENGLISH
    return findHeadEnglish<T>(parent, children);
#else
#error Head finding heuristics not implemented for this language!
#endif
  }

} // end namespace spear

#endif /* EDGE_HEAD_H */
