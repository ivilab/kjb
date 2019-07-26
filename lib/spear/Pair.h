#ifndef PAIR_H_
#define PAIR_H_

/*!
 * @file Pair.h
 *
 * @author Mihai Surdeanu
 * $Id: Pair.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */

namespace spear {

  /**
   * Wrapper for the non-standard pair
   */
  template <class T1, class T2>
    class Pair
    {
     public:

      Pair(const T1 & t1, const T2 & t2) : _first(t1), _second(t2) {};

      const T1 & getFirst() const { return _first; };

      const T2 & getSecond() const { return _second; };

      T2 & getSecond() { return _second; };

      void setFirst(const T1 & t1) { _first = t1; }

      void setSecond(const T2 & t2) { _second = t2; }

     private:
      
      T1 _first;

      T2 _second;
    };

} // end namespace spear

#endif
