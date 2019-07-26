#ifndef EDGE_H_
#define EDGE_H_

/**
 * @file Edge.h
 * Declaration of the Edge class
 * The Edge class is the building block of the parse tree
 * $Id: Edge.h 18301 2014-11-26 19:17:13Z ksimek $
 */

#include <list>

#include "spear/RCIPtr.h"


namespace spear {

class Edge : public RCObject 
{
 public:

  Edge() { };//std::cout << "Edge::constr\n"; }
  ~Edge() { };//std::cout << "Edge::destr\n"; }
      
 private:

  /** 
   * Type of this edge: 
   */
  char _type;
  
  /**
   * List of children of this edge
   */
  std::list< RCIPtr<Edge> > _children;
};

typedef RCIPtr<Edge> EdgePtr;

} // end namespace spear

#endif /* EDGE_H */
