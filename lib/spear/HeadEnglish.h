#ifndef HEAD_ENGLISH_H_
#define HEAD_ENGLISH_H_

/*!
 * @file HeadEnglish.h
 *
 * @author Mihai Surdeanu
 * $Id: HeadEnglish.h 18301 2014-11-26 19:17:13Z ksimek $ 
 */

#include <list>
#include <string>

#include "spear/RCIPtr.h"
#include "spear/Assert.h"
#include "spear/Wide.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace std;

#define TCI typename std::list< spear::RCIPtr<T> >::const_iterator
#define TCRI typename std::list< spear::RCIPtr<T> >::const_reverse_iterator

namespace spear {

  template <typename T>
    spear::RCIPtr<T> 
    traverseRightToLeft(const std::list< spear::RCIPtr<T> > & children, 
			const Char * labels [],
			bool traverseLabelsFirst = true)
    {
      if(traverseLabelsFirst == true){
	//
	// Traverse labels than children
	//
	for(int i = 0; labels[i] != NULL; i ++){
	  for(TCRI it = (TCRI) children.rbegin(); 
	      it != (TCRI) children.rend(); it ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      } else{
	//
	// Traverse children than labels
	//
	for(TCRI it = (TCRI) children.rbegin(); 
	    it != (TCRI) children.rend(); it ++){
	  for(int i = 0; labels[i] != NULL; i ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      }

      return spear::RCIPtr<T>();
    }

  template <typename T>
    spear::RCIPtr<T> 
    traverseLeftToRight(const std::list< spear::RCIPtr<T> > & children, 
			const Char * labels [],
			bool traverseLabelsFirst = true)
    {
      if(traverseLabelsFirst == true){
	//
	// Traverse labels than children
	//
	for(int i = 0; labels[i] != NULL; i ++){
	  for(TCI it = (TCI) children.begin(); 
	      it != (TCI) children.end(); it ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      } else{
	//
	// Traverse children than labels
	//
	for(TCI it = (TCI) children.begin(); 
	    it != (TCI) children.end(); it ++){
	  for(int i = 0; labels[i] != NULL; i ++){
	    if((* it)->getLabel() == labels[i]){
	      return (* it);
	    }
	  }
	}
      }

      return spear::RCIPtr<T>();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishNP(const String & parent,
		      const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] =
	{ W("NN"), W("NNP"), W("NNPS"), W("NNS"), W("NX"), W("POS"), W("JJR"), NULL };
      static const Char * row2 [] = 
	{ W("NP"), W("NPB"), NULL };
      static const Char * row3 [] = 
	{ W("$"), W("ADJP"), W("PRN"), NULL };
      static const Char * row4 [] = 
	{ W("CD"), NULL };
      static const Char * row5 [] = 
	{ W("JJ"), W("JJS"), W("RB"), W("QP"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseLeftToRight<T>(children, row2, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row3, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row4, false)) != 
	 (const T *) NULL)
	return head;

      if((head = traverseRightToLeft<T>(children, row5, false)) != 
	 (const T *) NULL)
	return head;

      return children.back();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishADJP(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = {  
	W("NNS"), W("QP"), W("NN"), W("$"), W("ADVP"), W("VBN"), W("JJ"), W("PRT"), 
	W("ADVP|PRT"), W("VBG"), W("ADJP"), W("JJR"), W("NP"), W("NPB"), W("JJS"), W("DT"), 
	W("FW"), W("RBR"), W("RBS"), W("SBAR"), W("RB"), W("IN"), W("VBD"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishADVP(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("RB"), W("RBR"), W("RBS"), W("FW"), W("ADVP"), W("PRT"), W("ADVP|PRT"), W("TO"), 
	W("CD"), W("JJR"), W("JJ"), W("IN"), W("NP"), W("NPB"), W("JJS"), W("NN"), W("PP"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishCONJP(const String & parent,
			 const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("RB"), W("IN"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishFRAG(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.back();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishINTJ(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.front();
    }
  
  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishLST(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("LS"), W(":"), W(","), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishNAC(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("NN"), W("NNS"), W("NNP"), W("NNPS"), W("NP"), W("NPB"), W("NAC"), W("EX"), W("$"), 
	W("CD"), W("QP"), W("PRP"), W("VBG"), W("JJ"), W("JJS"), W("JJR"), W("ADJP"), W("FW"), 
	NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishPP(const String & parent,
		      const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = 
	{ W("IN"), W("TO"), W("VBG"), W("VBN"), W("RP"), W("FW"), W("PP"), W("ADJP"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.back();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishPRN(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishQP(const String & parent,
		      const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("$"), W("IN"), W("NNS"), W("NN"), W("JJ"), W("RB"), W("DT"), W("CD"), W("QP"), W("JJR"), 
	W("JJS"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishRRC(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = {
	W("VP"), W("NP"), W("NPB"), W("ADVP"), W("PRT"), W("ADVP|PRT"), W("ADJP"), W("PP"), 
	NULL 
      };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishS(const String & parent,
		     const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("TO"), W("IN"), W("VP"), W("S"), W("SBAR"), W("ADJP"), W("UCP"), W("NP"), W("NPB"), 
	NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishSBAR(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("WHNP"), W("WHPP"), W("WHADVP"), W("WHADJP"), W("IN"), W("DT"), W("S"), W("SQ"), 
	W("SINV"), W("SBAR"), W("FRAG"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishSBARQ(const String & parent,
			 const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("SQ"), W("S"), W("SINV"), W("SBARQ"), W("FRAG"), W("SBAR"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishSINV(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("VBZ"), W("VBD"), W("VBP"), W("VB"), W("MD"), W("VP"), W("S"), W("SINV"), W("ADJP"), 
	W("NP"), W("NPB"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishSQ(const String & parent,
		      const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("VBZ"), W("VBD"), W("VBP"), W("VB"), W("MD"), W("VP"), W("SQ"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishUCP(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.back();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishVP(const String & parent,
		      const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { 
	W("TO"), W("VBD"), W("VBN"), W("MD"), W("VBZ"), W("VB"), W("VBG"), W("VBP"), W("VP"), W("VPB"), 
	W("ADJP"), W("NN"), W("NNS"), W("NP"), W("NPB"), NULL
      };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishWHADJP(const String & parent,
			  const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("WRB"), W("JJ"), W("ADJP"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishWHADVP(const String & parent,
			  const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("CC"), W("WRB"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishWHNP(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = 
	{ W("WDT"), W("WP"), W("WP$"), W("WHADJP"), W("WHPP"), W("WHNP"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseLeftToRight<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishWHPP(const String & parent,
			const std::list< spear::RCIPtr<T> > & children) 
    {
      static const Char * row1 [] = { W("IN"), W("TO"), W("FW"), NULL };

      spear::RCIPtr<T> head;

      if((head = traverseRightToLeft<T>(children, row1)) != 
	 (const T *) NULL)
	return head;

      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishTOP(const String & parent,
		       const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishX(const String & parent,
		     const std::list< spear::RCIPtr<T> > & children) 
    {
      return children.front();
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglishNoCoord(const String & parent,
			   const std::list< spear::RCIPtr<T> > & children) 
    {
      if(parent == W("NP") || 
	 parent == W("NPB")){
	return findHeadEnglishNP<T>(parent, children);
      } else if(parent == W("ADJP")){
	return findHeadEnglishADJP<T>(parent, children);
      } else if(parent == W("ADVP") || 
		parent == W("PRT") || 
		parent == W("ADVP|PRT") ||
		parent == W("PRT|ADVP")){
	return findHeadEnglishADVP<T>(parent, children);
      } else if(parent == W("CONJP")){
	return findHeadEnglishCONJP<T>(parent, children);
      } else if(parent == W("FRAG")){
	return findHeadEnglishFRAG<T>(parent, children);
      } else if(parent == W("INTJ")){
	return findHeadEnglishINTJ<T>(parent, children);
      } else if(parent == W("LST")){
	return findHeadEnglishLST<T>(parent, children);
      } else if(parent == W("NAC")){
	return findHeadEnglishNAC<T>(parent, children);
      } else if(parent == W("PP")){
	return findHeadEnglishPP<T>(parent, children);
      } else if(parent == W("PRN")){
	return findHeadEnglishPRN<T>(parent, children);
      } else if(parent == W("QP")){
	return findHeadEnglishQP<T>(parent, children);
      } else if(parent == W("RRC")){
	return findHeadEnglishRRC<T>(parent, children);
      } else if(parent == W("S") ||
		parent == W("SG")){
	return findHeadEnglishS<T>(parent, children);
      } else if(parent == W("SBAR")){
	return findHeadEnglishSBAR<T>(parent, children);
      } else if(parent == W("SBARQ")){
	return findHeadEnglishSBARQ<T>(parent, children);
      } else if(parent == W("SINV")){
	return findHeadEnglishSINV<T>(parent, children);
      } else if(parent == W("SQ")){
	return findHeadEnglishSQ<T>(parent, children);
      } else if(parent == W("UCP")){
	return findHeadEnglishUCP<T>(parent, children);
      } else if(parent == W("VP")){
	return findHeadEnglishVP<T>(parent, children);
      } else if(parent == W("WHADJP")){
	return findHeadEnglishWHADJP<T>(parent, children);
      } else if(parent == W("WHADVP")){
	return findHeadEnglishWHADVP<T>(parent, children);
      } else if(parent == W("WHNP")){
	return findHeadEnglishWHNP<T>(parent, children);
      } else if(parent == W("WHPP")){
	return findHeadEnglishWHPP<T>(parent, children);
      } else if(parent == W("TOP")){
	return findHeadEnglishTOP<T>(parent, children);
      } else if(parent == W("X") ||
		parent == W("NX")){
	return findHeadEnglishX<T>(parent, children);
      } else{
	CERR << "Can not find head edge for label " << parent << endl;
	CERR << "The children are:\n";
	for(TCI it = (TCI) children.begin(); 
	    it != (TCI) children.end(); it ++){
	  (* it)->display(CERR);
	  CERR << endl;
	}
	LASSERT(false, "unknown edge label");
      }
      
      return children.front();
    }

  template <typename T>
    TCRI
    isCoordinatedHead(const std::list< spear::RCIPtr<T> > & children,
		      TCRI current)
    {
      // Advance over head
      current ++;
      if(current == children.rend()) return children.rend();

      // Advance over CC
      if((* current)->getLabel() == W("CC")){
	current ++;
	if(current == children.rend()) return children.rend();
      } else{
	return children.rend();
      }
      
      // Advance over any commas
      for(; current != (TCRI) children.rend() && 
	    (* current)->getLabel() == W(",");
	  current ++);
      if(current == children.rend()) return children.rend();

      return current;
    }

  template <typename T>
    spear::RCIPtr<T>
    findHeadEnglish(const String & parent,
		    const std::list< spear::RCIPtr<T> > & children) 
    {
      LASSERT(children.empty() == false, "empty children set");
      
      // Find the head edge before the CC condition
      spear::RCIPtr<T> head = 
	findHeadEnglishNoCoord<T>(parent, children);

      // Check for coordinations
      for(TCRI it = (TCRI) children.rbegin(); 
	  it != (TCRI) children.rend(); it ++){

	// Found the head 
	if((* it) == head){
	  TCRI newHead;

	  // If it is a valid CC change the head to the edge before the CC
	  if((parent != W("NPB")) &&
	     (newHead = isCoordinatedHead<T>(children, it)) != children.rend()){
	    return (* newHead);
	  }

	  return head;
	}
      }

      return head;
    }

} // end namespace spear

#endif /* ENGLISH_HEAD_H */
