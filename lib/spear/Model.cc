/*!
 * @file Model.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include <iostream>
#include <sstream>
#include <algorithm>

#include "spear/Model.h"
#include "spear/CharUtils.h"
#include "spear/Head.h"

//#define USE_SYNONYMS 
#define MAX_SYNONYMS 1

using namespace std;
using namespace spear;

list<BankEdgePtr>::iterator 
Model::findNext(BankEdgePtr & edge,
		list<BankEdgePtr>::iterator it)
{
  for(++ it; (* it) != edge->getHead(); it ++){
    if(! isPunctuation(* it)) break;
  }
  return it;
}

list<BankEdgePtr>::reverse_iterator 
Model::findNext(BankEdgePtr & edge,
		list<BankEdgePtr>::reverse_iterator it)
{
  for(++ it; (* it) != edge->getHead(); it ++){
    if(! isPunctuation(* it)) break;
  }
  return it;
}

void Model::addHorizontalHistory(BankEdgePtr & edge)
{
  //COUT << "Inspecting edge:" << endl << edge << endl;

  if(edge->isTerminal() == true){
    return;
  }
  
  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      (* it) != edge->getHead(); it ++){
    
    list<BankEdgePtr>::iterator next = findNext(edge, it);
  
    if(! isPunctuation(* it) && (* next) != edge->getHead()){
      String nl = (* it)->getLabel() + (* next)->getLabel();
      (* it)->setLabel(nl);
    }
  }

  for(list<BankEdgePtr>::reverse_iterator it = edge->getChildren().rbegin();
      (* it) != edge->getHead(); it ++){

    list<BankEdgePtr>::reverse_iterator next = findNext(edge, it);
  
    if(! isPunctuation(* it) && (* next) != edge->getHead()){
      String nl = (* it)->getLabel() + (* next)->getLabel();
      (* it)->setLabel(nl);
    }
  }

  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    addHorizontalHistory(* it);
  }
}

static void labelGerundVerbs(BankEdgePtr & edge)
{
  if(edge->getLabel() == W("VP") &&
     edge->getHead()->getLabel() == W("VBG")){
    edge->setLabel("VPG");
  }

  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    labelGerundVerbs(* it);
  }
}

void Model::preprocessEdge(
    BankEdgePtr&               edge,
    std::vector<BankEdgePtr>&  sentence
    )
{
    pruneUnnecessaryNodes(edge);

    stripLabelAugmentation(edge);

    addBaseNounPhrases(edge);
    addBaseNounPhraseParents(edge);
    repairBaseNounPhrases(edge);

    // labelSubjectlessSentencess(edge);

    removeEmptyNodes(edge);

    // Construct a linearized list of sentence terminals
    // This operation must be performed before the punctuation changes, 
    // which might remove sentence elements
    sentence.clear();
    generateSentence(edge, sentence, true);

    list<BankEdgePtr> punctuations;
    punctuations.push_back(edge);
    raisePunctuation(
	edge,
	punctuations, 
	punctuations.begin(),
	punctuations.end()
	);

    // We might have empty nodes after punctuation raising
    removeEmptyNodes(edge);

    edge->setHead();

    // mark argument phrases (only for model 2)
    identifyArguments(edge);

    // relabel SG to S if they contain an argument (only for model 2)
    repairSubjectlessSentences(edge);

    // mark unary non terminals
    // markUnary(edge, 0);

    // normalize numbers to a unique lexem
    // normalizeNumbers(edge);

    // annotate tags with their parents
    // annotateTags(edge, W("NONE"));

    // COUT << "Edge before history:" << endl << edge << endl;
    // addHorizontalHistory(edge);
    // COUT << "Edge after history:" << endl << edge << endl;

    // labelGerundVerbs(edge);

#ifdef USE_SYNONYMS
    // detect synonyms for all words in this tree
    edge->findSynonyms();
#endif
}

void Model::extractTerminals(spear::BankEdgePtr & edge,
			     std::vector<spear::BankEdgePtr> & sentence)
{
  sentence.clear();
  stripLabelAugmentation(edge);
  generateSentence(edge, sentence, true);  
}

bool Model::isUnnecessaryNode(const BankEdgePtr & edge) const
{
  static const Char * unnecessary [] = { 
    W("."), W("''"), W("``"), NULL
  };

  for(int i = 0; unnecessary[i] != NULL; i ++){
    if(edge->getLabel() == unnecessary[i]){
      return true;
    }
  }

  return false;
}

bool Model::splitLabel(const String & label,
		       String & coreLabel,
		       String & labelAugment) const
{
  // do not modify labels that start with "-", i.e. "-NONE-", "-LRB-", "-RRB-"
  if(label.empty() == false && label[0] == W('-')){
    return false;
  }

  size_t posDash = label.find(L'-');
  size_t posEqual = label.find(L'=');

  size_t pos = posEqual;
  if(posDash < posEqual) pos = posDash;

  if(pos < label.size()){
    // wcout << "Found compound label: " << label << endl;
    labelAugment = label.substr(pos + 1);
    coreLabel = label.substr(0, pos);
    return true;
  }

  return false;
}

void Model::stripLabelAugmentation(BankEdgePtr & edge) const
{
  // inspect children
  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    stripLabelAugmentation(* it);
  }

  String coreLabel, labelAugment;
  if(splitLabel(edge->getLabel(), coreLabel, labelAugment)){
    edge->setLabelAugmentation(labelAugment);
    edge->setLabel(coreLabel);
  }
}

void Model::pruneUnnecessaryNodes(BankEdgePtr & edge)
{
    for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	it != edge -> getChildren().end(); it ++)
    {
	pruneUnnecessaryNodes(* it);
    }
    for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	it != edge->getChildren().end(); )
    {
	if(isUnnecessaryNode(* it))
	{
	    // learn this label before if not punctuation
	    if((* it) -> getLabel() != W("."))
	    {
		lexicon_.addLabel((* it) -> getLabel());
	    }

	    // wcout << "Removing unnecessary child from:" << endl;
	    // edge->display(wcout); wcout << endl;
	    it = edge -> getChildren().erase(it);
	} else {
	    it ++;
	}
    }
}

bool Model::isEmptyNode(const spear::BankEdgePtr & edge) const
{
  static const Char * empty [] = { 
    W("-NONE-"), NULL
  };

  if(edge->getChildren().empty() == true &&
     edge->getWord().empty() == true){
    return true;
  }

  for(int i = 0; empty[i] != NULL; i ++){
    if(edge->getLabel() == empty[i]){
      return true;
    }
  }

  return false;
}

void Model::removeEmptyNodes(spear::BankEdgePtr & edge) const
{
  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    removeEmptyNodes(* it);
  }

  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); ){
    if(isEmptyNode(* it)){
      // wcout << "Removing empty child from:" << endl;
      // edge->display(wcout); wcout << endl;
      // wcout << "The edge removed is:" << endl;
      // (* it)->display(wcout); wcout << endl;
      it = edge->getChildren().erase(it);
    } else{
      it ++;
    }
  }
}

bool Model::isPunctuation(const spear::BankEdgePtr & edge) const
{
  static const Char * empty [] = {W(","), W(":"), NULL};

  for(int i = 0; empty[i] != NULL; i ++){
    if(edge->getLabel() == empty[i]){
      return true;
    }
  }

  return false;
}

static bool compatibleLabels(
    const BankEdge* e1,
    const BankEdge* e2
    )
{
    /*
      unsigned int s1 = l1.size();
      unsigned int s2 = l2.size();

      if(s1 == s2){
      return (l1 == l2);
      }

      if(s1 < s2){
      return (l2.substr(0, s1) == l1);
      }

      if(s2 < s1){
      return (l1.substr(0, s2) == l2);
      }

      return false;
    */

    if(e1 -> isTerminal() && e2 -> isTerminal()) return true;
    return (e1 -> getLabel().substr(0, 1) == e2 -> getLabel().substr(0, 1));
}

static bool acceptsAnyCoord(const BankEdge * edge) 
{
    static const Char * labels [] = {W("UCP"), W("QP"), NULL};
    for(int i = 0; labels[i] != NULL; i ++)
    {
	if(edge->getLabel() == labels[i])
	{
	    return true;
	}
    }
    return false;
}

bool Model::isCoordination(
    std::list<spear::BankEdgePtr>::const_iterator begin,
    std::list<spear::BankEdgePtr>::const_iterator end,
    const BankEdge*                               previous,
    const BankEdge*                               parent
    ) const
{
    LASSERT(previous != NULL, "nil previous pointer");

    if((* begin)->getLabel() != W("CC")) return false;

    for(++begin; begin != end; begin++)
    {
	if(!isPunctuation(*begin))
	{
	    // anything is ok inside an UCP or an QP
	    if(parent != NULL && acceptsAnyCoord(parent)) return true;
	    // skip PRN
	    if((* begin)->getLabel() == W("PRN")) continue;
	    if(compatibleLabels(previous, (* begin).operator->())) return true;
	}
    }

    return false;
}

bool Model::isCoordination(
    std::list<spear::BankEdgePtr>::reverse_iterator begin,
    std::list<spear::BankEdgePtr>::reverse_iterator end,
    const BankEdge*                                 previous,
    const BankEdge*                                 parent
    ) const
{
    LASSERT(previous != NULL, "nil previous pointer");

    if((* begin)->getLabel() != W("CC")) return false;
    for(++ begin; begin != end; begin ++)
    {
	if(! isPunctuation(* begin))
	{
	    // anything is ok inside an UCP 
	    if(parent != NULL && acceptsAnyCoord(parent)) return true;
	    // skip PRN
	    if((* begin)->getLabel() == W("PRN")) continue;
	    if(compatibleLabels(previous, (* begin).operator->())) return true;
	}
    }
    return false;
}

bool Model::isBaseNounPhrase(const spear::BankEdgePtr & edge) const
{
    if(edge->getLabel() != W("NP"))
    {
	return false;
    }

    for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	it != edge -> getChildren().end();
	it ++)
    {
	//
	// An NPB can not include another NP* unless it is a posessive
	//
	if((* it) -> getLabel().substr(0, 2) == W("NP") &&
	   ((* it) -> getChildren().empty() == true ||
	    (* it) -> getChildren().back() -> getLabel() != W("POS")))
	{
	    return false;
	}
    }

    return true;
}

void Model::addBaseNounPhrases(const spear::BankEdgePtr& edge) const
{
    // Must inspect children first
    for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
	it != edge->getChildren().end();
	it ++)
    {
	addBaseNounPhrases(*it);
    }

    if(isBaseNounPhrase(edge))
    {
	edge->setLabel("NPB");
    }
}

bool Model::isCoordinatedNounPhrase(const spear::BankEdgePtr & edge) const
{
  BankEdgePtr head = findHead<BankEdge>(edge->getLabel(), edge->getChildren());
  LASSERT(head != (const BankEdge *) NULL, "nil head pointer");

  // Is there a conjunction following the head?
  for(list<BankEdgePtr>::reverse_iterator it = 
	edge->getChildren().rbegin(); it != edge->getChildren().rend(); it ++){
    if((* it) == head){
      break;
    }

    if(it != edge->getChildren().rbegin() &&
       (* it)->getLabel() == W("CC")){
      return true;
    }
  }

  // Note that a non-initial CC that immediately precedes the head is not
  // possible. Suc situations are detected by the head finding heuristic which
  // migrates the head to the edge before the CC.

  return false;
}

void Model::addBaseNounPhraseParents(const spear::BankEdgePtr & edge) const
{
    if(edge->getLabel() != W("NPB"))
    {
	for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	    it != edge -> getChildren().end(); )
	{
	    //CERR << "Looking at edge:\n";
	    //edge->display(CERR);
	    //CERR << endl;

	    if((*it) -> getLabel() == W("NPB") &&
	       (edge -> getLabel() != W("NP") ||
		findHead<BankEdge>(
		    edge -> getLabel(),
		    edge -> getChildren()
		    ) != *it ||
		isCoordinatedNounPhrase(edge)))
	    {
		BankEdgePtr np(new BankEdge());
		BankEdgePtr npb = *it;
		np -> setLabel(W("NP"));
		np -> addChild(npb);
		it = edge -> getChildren().erase(it);
		edge -> getChildren().insert(it, np);

		// wcout << "CONSTRUCTED NP:\n";
		// np->display(wcout);
		// wcout << endl;

	    } else {
		it ++;
	    }
	}
    }

    for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	it != edge -> getChildren().end(); it ++)
    {
	addBaseNounPhraseParents(* it);
    }
}

void Model::repairBaseNounPhrases(const spear::BankEdgePtr & edge) const
{
  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); ){
    if((* it)->getLabel() == W("NPB") &&
       (* it)->getChildren().empty() == false &&
       (* it)->getChildren().back()->getLabel().substr(0, 1) == W("S")){
      //CERR << "Found broken in phrase:\n";
      //edge->display(CERR);
      //CERR << endl;

      BankEdgePtr s = (* it)->getChildren().back();
      (* it)->getChildren().pop_back();
      edge->getChildren().insert(++ it, s);

      //CERR << "The fixed edge:\n";
      //edge->display(CERR);
      //CERR << endl;
    } else{
      it ++;
    }
  }

  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    repairBaseNounPhrases(* it);
  }
}

bool Model::isSentenceWithNilSubject(const spear::BankEdgePtr & edge) const
{
  BankEdgePtr head;  
  
  //
  // The edge must be an S with a VP head
  //
  if(edge->getLabel().substr(0, 1) == W("S") &&
     edge->getChildren().empty() == false &&
     (head = findHead<BankEdge>(edge->getLabel(), edge->getChildren())) != (const BankEdge *) NULL &&
     head->getLabel() == W("VP")){

    //
    // Make sure the edge had an empty subject
    //
    for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
	it != edge->getChildren().end(); it ++){
      
      // The subject is a (NP (NPB/SBJ ...)) due to the NPB processing phrase
      if((* it)->getLabel() == W("NP") &&
	 (* it)->getChildren().size() == 1 &&
	 (* it)->getChildren().front()->getLabelAugmentation().find(W("SBJ")) < 
	 (* it)->getChildren().front()->getLabelAugmentation().size() &&
	 (* it)->getChildren().front()->getChildren().size() == 1 &&
	 (* it)->getChildren().front()->getChildren().front()->getLabel() == W("-NONE-")){
	return true;
      }
    }
  }

  return false;
}

void Model::labelSubjectlessSentencess(const spear::BankEdgePtr & edge) const
{
  if(isSentenceWithNilSubject(edge)){
    edge->setLabel(W("SG"));

    // CERR << "Found subjectless sentence:\n";
    // edge->display(CERR);
    // CERR << endl;
  }

  for(list<BankEdgePtr>::iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    labelSubjectlessSentencess(* it);
  }
}

void Model::raisePunctuation(
    const spear::BankEdgePtr&               edge,
    std::list<spear::BankEdgePtr>&          parentChildren,
    std::list<spear::BankEdgePtr>::iterator begin,
    std::list<spear::BankEdgePtr>::iterator end
    ) const
{
    //
    // check whether any children have punctuation that needs raising
    //
    for(list<BankEdgePtr>::iterator it = edge -> getChildren().begin();
	it != edge -> getChildren().end(); it ++)
    {
	list<BankEdgePtr>::iterator next = it;
	next++;
	raisePunctuation(*it, edge -> getChildren(), it, next);
    }

    //
    // raise punctuation on the left side
    //
    while(edge->getChildren().empty() == false)
    {
	BankEdgePtr front = edge -> getChildren().front();

	if(isPunctuation(front))
	{
	    // found punctuation. move it up, then remove from current edge

	    // CERR << "Found LEFT punctuation in edge:" << endl;
	    // edge->display(CERR); CERR << endl;

	    parentChildren.insert(begin, front);
	    edge -> getChildren().pop_front();
	} else {
	    // found the first non-punctuation node
	    break;
	}
    }

    //
    // raise punctuation on the right side
    //
    while(edge -> getChildren().empty() == false)
    {
	BankEdgePtr back = edge->getChildren().back();

	if(isPunctuation(back))
	{
	    // found punctuation. move it up, then remove from current edge

	    // CERR << "Found RIGHT punctuation in edge:" << endl;
	    // edge->display(CERR); CERR << endl;

	    end = parentChildren.insert(end, back);
	    edge -> getChildren().pop_back();
	} else {
	    // found the first non-punctuation node
	    break;
	}
    }
}

void Model::identifyArguments(const spear::BankEdgePtr & edge) const
{
}

void Model::repairSubjectlessSentences(const spear::BankEdgePtr & edge) const
{
}

void Model::generateSentence(const spear::BankEdgePtr & edge,
			     std::vector<spear::BankEdgePtr> & sentence,
			     bool setIndex) const
{
    if(edge->isTerminal())
    {

	//
	// This method might be called before empty nodes are removed
	// Skip empty nodes
	//
	if(isEmptyNode(edge) == false)
	{
	    if(setIndex == true)
	    {
		edge->setSentenceIndex(sentence.size());
	    }
	    sentence.push_back(edge);
	}
    }

    for(list<BankEdgePtr>::const_iterator it = edge->getChildren().begin();
	it != edge->getChildren().end();
	it ++)
    {
	generateSentence(* it, sentence, setIndex);
    }
}

void Model::generate_events(
    OStream&                          os, 
    spear::BankEdgePtr&               edge,
    std::vector<spear::BankEdgePtr>&  sentence)
{
    // Begin line with sentence number, followed by "6" denoting a sentence,
    // followed by sentence length
    os << "6 " << sentences_seen_++ << " " << sentence.size();
    for(unsigned int i = 0; i < sentence.size(); i ++)
    {
	if(! isPunctuation(sentence[i]))
	{
	    // For non-punctuation, write word and PoS tag
	    os << " " << sentence[i]->getWord()
	       << " " << sentence[i]->getLabel();
	}
    }
    for(unsigned int i = 0; i < sentence.size(); i ++)
    {
	if(isPunctuation(sentence[i]))
	{
	    // Write punctuation at the end
	    os << " " << sentence[i]->getWord()
	       << " " << sentence[i]->getLabel();
	}
    }
    // End of line
    os << endl;
    
    BankEdgePtr topParent;
    // topParent points to TOP (i.e., root node)
    // generateDependencyEvents() recursively traverses tree
    generateDependencyEvents(os, edge, topParent, 0);
    
    learnLexicon(edge);
    
    //edge->display(os); os << endl;
}

static void generate_modifier_event(
    OStream&        os,
    const int       event_num,  // number of the event
    const BankEdge* p,         // parent
    const int       parent_num, // event_num for parent event
    const BankEdge* h,         // head
    const int       head_num,   // event_num for sister head event
    const BankEdge* m,         // node corresponding to this event
    bool            toLeft,    // whether we are left of head
    bool            adjacent,  // whether adjacent to head?
    bool            hasVerb,   // whether there is an intervening verb
    bool            hasCoord,  // whether there is a coordinating conjunction
    const String&   coordLabel,// constituent of coordinating conj.
    const String&   coordWord, // word of coordinating conj.
    bool            hasPunc,   // whether there is intervening punctuation
    const String&   puncLabel, // constituent of punctuation
    const String&   puncWord   // word of punctuation
    )
{
    vector<String> modifWords;
    vector<String> headWords;
    
    modifWords.push_back(m->getHeadWord());
    headWords.push_back(h->getHeadWord());
    
#ifdef USE_SYNONYMS
  
    const vector<String>& modifSyns = m->getHeadSynonyms();
    const vector<String>& headSyns = h->getHeadSynonyms();

    for(int i = 0; i < MAX_SYNONYMS && i < modifSyns.size(); i++)
    {
	modifWords.push_back(modifSyns[i]);
    }
    
    for(int i = 0; i < MAX_SYNONYMS && i < headSyns.size(); i++)
    {
	headWords.push_back(headSyns[i]);
    }
    
#endif
    // create pointers pointing to head and modifier tags
    const String& modifTag = m -> getHeadTerminalLabel();
    const String& headTag = h -> getHeadTerminalLabel();

    // loop only once at each level unless there are synonyms
    for(unsigned int i = 0; i < modifWords.size(); i ++)
    {
	for(unsigned int j = 0; j < headWords.size(); j ++)
	{
	    // "2" denotes a dependency event
	    os << "2" 
	       << " " << event_num
	       << " " << parent_num
	       << " " << head_num
	       << " " << modifWords[i] // write modifier word
	       << " " << modifTag // write modifier tag
	       << " " << headWords[j] // write head word
	       << " " << headTag // write head tag
	       << " " << m->getLabel() // modifier constituent
	       << " " << p->getLabel() // parent constituent
	       << " " << h->getLabel() // head's constituent
	       << " 000000 " // not used (could contain subcategorization, etc.)
	       << toLeft << adjacent << hasVerb << " "; //binary markers

	    // If there are conjunctions/punctuation, write them.
	    if(hasCoord)
	    {
	    	os << hasCoord << " " << coordWord << " " << coordLabel << " ";
	    } else
	    {
	    	os << hasCoord << " ";
	    }
	    // os << 0 << " ";
	    
	    if(hasPunc)
	    {
		os << hasPunc << " " << puncWord << " " << puncLabel;
	    } else
	    {
		os << hasPunc;
	    }
	    os << endl;
	}
    }
}

static bool isAnyNpb(const String & l)
{
  if(l.substr(0, 3) == W("NPB")){
    return true;
  }

  return false;
}

bool Model::containsVerb(const BankEdgePtr & e) const
{
    if(isAnyNpb(e->getLabel())) return false;
    if(e->isTerminal() && e->getLabel().substr(0, 2) == W("VB")) return true;
    for(list<BankEdgePtr>::const_iterator it = e->getChildren().begin();
	it != e->getChildren().end(); it ++)
    {
	if(containsVerb(* it)) return true;
    }
    return false;
}

void Model::generateDependencyEvents(
    OStream&                   os,
    const spear::BankEdgePtr&  edge,
    const spear::BankEdgePtr&  parent,
    int                        parent_num)
{
    std::list<int> child_nums;
    // learn this label
    lexicon_.addLabel(edge -> getLabel());
    
    // there are no dependencies for preterminals
    if(edge -> isTerminal()) return;
    
    // check for NULL head
    if(edge -> getHead() == (const BankEdge*) NULL)
    {
	CERR << "Found NULL head for edge:" << endl;
	edge->display(CERR);
	CERR << endl;
	LASSERT(
	    edge->getHead() != (const BankEdge*) NULL,
	    "nil head pointer");
    }
    
    /////////////////////////////////////////////////////////////////////////
    //
    // head event
    //
    /////////////////////////////////////////////////////////////////////////

    int this_head_num = ++nts_seen_;
    child_nums.push_back(this_head_num);
    
    // "3" is the code for head events
    os << "3"
       << " " << this_head_num
       << " " << parent_num
       << " " << edge->getHeadWord() // write the head word itself
       << " " << edge->getHeadTerminalLabel() // write head word PoS
       << " " << edge->getLabel() // write constituent that head is head of
       << " " << edge->getHead()->getLabel() // write constituent of head itself
       << " 00000  00000 " << endl; // unused stuff (subcat, etc.)
#ifdef USE_SYNONYMS
    
    const std::vector<String>& syns = edge -> getHeadSynonyms();
    const String& headTag = edge -> getHeadTerminalLabel();
    for(int i = 0; i < MAX_SYNONYMS && i < syns.size(); i ++)
    {
	os << "3 " << syns[i] << " " << headTag
	   << " " << edge->getLabel() << " " << edge->getHead()->getLabel() 
	   << " 00000  00000 " << endl;
    }
    
#endif

    // Colin: I'm not sure what is done with these buffers.

    // unary grammar rule
    OStringStream buffer;
    buffer << "U " << edge->getLabel() 
	   << " " << edge->getHead()->getLabel();
    modifiers_.set(buffer.str().c_str(), 1);
    
    // left subcats rule
    OStringStream leftSubcatBuffer;
    leftSubcatBuffer << "X " << edge->getLabel()
		     << " " << edge->getHead()->getLabel()
		     << " 00000";
    modifiers_.set(leftSubcatBuffer.str().c_str(), 1);
    
    // right subcats rule
    OStringStream rightSubcatBuffer;
    rightSubcatBuffer << "Y " << edge->getLabel()
		      << " " << edge->getHead()->getLabel()
		      << " 00000";
    modifiers_.set(rightSubcatBuffer.str().c_str(), 1);

    /////////////////////////////////////////////////////////////////////////
    //
    // left events
    //
    /////////////////////////////////////////////////////////////////////////  
  
    // set to the previous edge in the current traversal
    const BankEdge* previous = edge->getHead().operator->();
    // set to true if there is a punctuation between two phrases
    bool hasPunc = false;
    // if there is a punctuation in between, this is the punctuation label 
    String puncLabel;
    // if there is a punctuation in between, this is the punctuation word
    String puncWord;
    // set to true if there is a CC between two phrases
    bool hasCoord = false;
    // if there is a CC in between, this is the CC label 
    String coordLabel;
    // if there is a CC in between, this is the CC word
    String coordWord;
    // set to true if the current edge is adjacent to the head
    bool isAdj = true;
    // set to true if there is a verb between the head and the crt edge
    bool hasInterveningVerb = false;
    
    //
    // Modifiers on the left side of the head for NPB phrases
    //
    int previous_num = this_head_num;
    if(isAnyNpb(edge->getLabel()))
    {
	// NPBs are treated differently: condition on previous modifier
	// instead of head itself
	for(list<BankEdgePtr>::reverse_iterator it =
		edge->getHeadReverseIterator();
	    it != edge->getChildren().rend();
	    it ++)
	{
	    if(*it != edge->getHead())
	    {
		// do not generate events for punctuation signs
		if(!isPunctuation(*it))
		{
		    LASSERT(previous != NULL, "nil previous pointer");
		    generate_modifier_event(
			os,
			++nts_seen_,
			edge.operator->(), // parent
			parent_num,
			previous, // head is actually adjacent item
			previous_num,
			(* it).operator->(), // modifier
			true, true, false, // left? adjacent? verb?
			false, W(""), W(""), // CC? CC word + label
			hasPunc, puncLabel, puncWord
			); // punc? punc word + label
		    
		    // left grammar rule for NPBs
		    OStringStream leftBuffer;
		    leftBuffer << "L " << edge->getLabel() 
			       << " " << previous->getLabel() 
			       << " " << (* it)->getLabel();
		    modifiers_.set(leftBuffer.str().c_str(), 1);
		    hasPunc = false;
		    previous = (* it).operator->();
		    previous_num = nts_seen_;
		} else
		{
		    if(hasPunc == false)
		    {
			hasPunc = true; // flip flag if punc is encountered
			puncLabel = (* it)->getLabel(); // set punc content
			puncWord = (* it)->getWord();
		    }
		}
		child_nums.push_front(nts_seen_);
	    }
	}
    }
    
    //
    // Modifiers on the left side of the head for non-NPB phrases
    //
    else
    { // ! NPB
	for(list<BankEdgePtr>::reverse_iterator it =
		edge->getHeadReverseIterator();
	    it != edge->getChildren().rend();
	    it ++)
	{
	    
	    // generate events only to the left side of the head
	    if(* it != edge->getHead())
	    {
		// do not generate events for punctuation signs
		// do not generate events for CC signs 
		if((! isPunctuation(* it)) && 
		   (! isCoordination(
		       it,
		       edge->getChildren().rend(), 
		       previous,
		       edge.operator->())))
		{
		    generate_modifier_event(
			os,
			++nts_seen_,
			edge.operator->(), // parent
			parent_num,
			edge->getHead().operator->(), // head
			this_head_num,
			(* it).operator->(), // modifier
			// isLeft?, isAdjacent?, hasVerb?
			true, isAdj, hasInterveningVerb, 
			hasCoord, coordLabel, coordWord, // CC? CC word + label
			hasPunc, puncLabel, puncWord // Punc? PP word + label
			); 
		    
		    // left grammar rule for non NPBs
		    OStringStream leftBuffer;
		    leftBuffer << "L " << edge->getLabel() 
			       << " " << edge->getHead()->getLabel() 
			       << " " << (* it)->getLabel();
		    modifiers_.set(leftBuffer.str().c_str(), 1);
		    
		    hasPunc = false;
		    hasCoord = false;
		    isAdj = false;
		    if(containsVerb(* it)) hasInterveningVerb = true;
		    previous = (* it).operator->();
		} else if(isPunctuation(* it))
		{
		    if(hasPunc == false)
		    {
			hasPunc = true;
			puncLabel = (* it)->getLabel();
			puncWord = (* it)->getWord();
		    }
		} else
		{
		    if(hasCoord == false)
		    {
			hasCoord = true;
			coordLabel = (* it)->getLabel();
			coordWord = (* it)->getWord();
		    }
		}
		child_nums.push_front(nts_seen_);
	    } 
	}
    }
    
    // the left stop
    // get here after all other modifiers have been traversed
    if(isAnyNpb(edge->getLabel()))
    {
	LASSERT(previous != NULL, "nil previous pointer");
	generate_modifier_event(
	    os,
	    ++nts_seen_,
	    edge.operator->(), // parent
	    parent_num,
	    previous, // head is previous for NPBs
	    previous_num,
	    BankEdge::getStopPhrase().operator->(), // create STOP modifier
	    true, true, false, // isLeft? isAdjacent? hasVerb?
	    false, W(""), W(""), // CC? CC word + label
	    hasPunc, puncLabel, puncWord // Punc? Punc word + label
	    );
    } else
    { // ! NPB
	generate_modifier_event(
	    os,
	    ++nts_seen_,
	    edge.operator->(), // parent
	    parent_num,
	    edge->getHead().operator->(), // head is actual head
	    this_head_num,
	    BankEdge::getStopPhrase().operator->(), // create STOP
	    true, isAdj, hasInterveningVerb, // isLeft, isAdjacent? hasVerb?
	    hasCoord, coordLabel, coordWord, // CC? CC word + label
	    hasPunc, puncLabel, puncWord // Punc? Punc word + label
	    );
    }

    /////////////////////////////////////////////////////////////////////////
    //
    // right events
    //
    ///////////////////////////////////////////////////////////////////////// 

    // initialize
    previous = NULL;
    hasCoord = false;
    hasPunc = false;
    isAdj = true;
    hasInterveningVerb = false;
    previous = edge->getHead().operator->();
    previous_num = this_head_num;

    //
    // Modifiers on the right side of the head for NPB phrases
    //
    
    // This is analogous to left side modifiers, except that
    // isLeft is false
    if(isAnyNpb(edge->getLabel())){

	for(list<BankEdgePtr>::iterator it = edge->getHeadIterator(); 
	    it != edge->getChildren().end();
	    it ++)
	{

	    // generate events only to the right side of the head
	    if(* it != edge->getHead())
	    {

		// do not generate events for punctuation signs
		if(! isPunctuation(* it))
		{
		    LASSERT(previous != NULL, "nil previous pointer");
		    generate_modifier_event(
			os,
			++nts_seen_,
			edge.operator->(),
			parent_num,
			previous,
			previous_num,
			(* it).operator->(),
			false, true, false,
			false, W(""), W(""),
			hasPunc, puncLabel, puncWord
			);

		    // right grammar rule for NPBs
		    OStringStream rightBuffer;
		    rightBuffer << "R " << edge->getLabel() 
				<< " " << previous->getLabel() 
				<< " " << (* it)->getLabel();
		    modifiers_.set(rightBuffer.str().c_str(), 1);
		    hasPunc = false;
		    previous = (* it).operator->();
		    previous_num = nts_seen_;
		} else {
		    if(hasPunc == false)
		    {
			hasPunc = true;
			puncLabel = (* it)->getLabel();
			puncWord = (* it)->getWord();
		    }
		}
		child_nums.push_back(nts_seen_);
	    } 
	}
    }
  
    //
    // Modifiers on the right side of the head for non-NPB phrases
    //
    else
    { // ! NPB
	for(list<BankEdgePtr>::iterator it = edge->getHeadIterator();
	    it != edge->getChildren().end(); it ++)
	{

	    // generate events only to the right side of the head
	    if(* it != edge->getHead())
	    {
	
		// do not generate events for punctuation signs
		// do not generate events for CC signs
		if((! isPunctuation(* it)) && 
		   (! isCoordination(
		       it, edge->getChildren().end(), 
		       previous, edge.operator->()
		       )
		       )
		    )
		{
		    generate_modifier_event(
			os,
			++nts_seen_,
			edge.operator->(),
			parent_num,
			edge->getHead().operator->(),
			this_head_num,
			(* it).operator->(),
			false, isAdj, hasInterveningVerb, 
			hasCoord, coordLabel, coordWord,
			hasPunc, puncLabel, puncWord
			);

		    // right grammar rule for non NPBs
		    OStringStream rightBuffer;
		    rightBuffer << "R " << edge->getLabel() 
				<< " " << edge->getHead()->getLabel() 
				<< " " << (* it)->getLabel();
		    modifiers_.set(rightBuffer.str().c_str(), 1);

		    isAdj = false;
		    hasPunc = false;
		    hasCoord = false;
		    if(containsVerb(* it)) hasInterveningVerb = true;
		    previous = (* it).operator->();
		} else if(isPunctuation(* it))
		{
		    if(hasPunc == false)
		    {
			hasPunc = true;
			puncLabel = (* it)->getLabel();
			puncWord = (* it)->getWord();
		    }
		} else
		{
		    if(hasCoord == false){
			hasCoord = true;
			coordLabel = (* it)->getLabel();
			coordWord = (* it)->getWord();
		    }
		}
		child_nums.push_back(nts_seen_);
	    }
	}
    }
 
    // the right stop
    if(isAnyNpb(edge->getLabel())){
	LASSERT(previous != NULL, "nil previous pointer");
	generate_modifier_event(
	    os,
	    ++nts_seen_,
	    edge.operator->(),
	    parent_num,
	    previous,
	    previous_num,
	    BankEdge::getStopPhrase().operator->(),
	    false, true, false,
	    false, W(""), W(""),
	    hasPunc, puncLabel, puncWord
	    );
    } else{
	generate_modifier_event(
	    os,
	    ++nts_seen_,
	    edge.operator->(),
	    parent_num,
	    edge->getHead().operator->(),
	    this_head_num,
	    BankEdge::getStopPhrase().operator->(),
	    false, isAdj, hasInterveningVerb, 
	    hasCoord, coordLabel, coordWord,
	    hasPunc, puncLabel, puncWord
	    );
    }

    for(list<BankEdgePtr>::const_iterator it = edge->getChildren().begin();
	it != edge->getChildren().end(); it++)
    {
	// if(
	//     isPunctuation(*it) ||
	//     isCoordination(
	// 	std::reverse_iterator<list<BankEdgePtr>::const_iterator>(it),
	// 	edge->getChildren().rbegin(), 
	// 	previous, edge.operator->()
	// 	) ||
	//     isCoordination(
	// 	it,
	// 	edge->getChildren().end(), 
	// 	previous, edge.operator->()
	// 	)
	//     )
	// {
	//     continue;
	// }
	generateDependencyEvents(os, *it, edge, child_nums.front());
	child_nums.pop_front();
    }
}


void Model::learnLexicon(const spear::BankEdgePtr & e)
{
  if(e->isTerminal()){
    lexicon_.addLexem(e->getWord(), e->getLabel());

#ifdef USE_SYNONYMS

    for(int i = 0; i < MAX_SYNONYMS && i < e->getSynonyms().size(); i ++){
      lexicon_.addLexem(e->getSynonyms()[i], e->getLabel());
    }

#endif

  }

  for(list<BankEdgePtr>::const_iterator it = e->getChildren().begin();
      it != e->getChildren().end(); it ++){
    learnLexicon(* it);
  }
}

void Model::storeGrammar(
    OStream& rulesStream,
    OStream& labelsStream,
    OStream& lexiconStream)
{
    CERR << "Sorting grammar rules..." << endl;
    vector<String> sortedRules;

    for(StringMap<int>::const_iterator it = modifiers_.begin();
	it != modifiers_.end(); it ++)
    {
	sortedRules.push_back((* it).second->getKey());
    }

    stable_sort(sortedRules.begin(), sortedRules.end());

    for(vector<String>::const_iterator it = sortedRules.begin();
	it != sortedRules.end(); it ++)
    {
	rulesStream << (* it) << endl;
    }

    CERR << "Sorting nonterminals..." << endl;
    vector<String> sortedLabels;

    lexicon_.sortLabels(sortedLabels);

    for(vector<String>::const_iterator it = sortedLabels.begin();
	it != sortedLabels.end(); it ++){
	labelsStream << (* it) << endl;
    }

    CERR << "Sorting lexicon..." << endl;
    vector<String> sortedLex;

    lexicon_.sortLexems(sortedLex, lf_thresh_);

    for(vector<String>::const_iterator it = sortedLex.begin();
	it != sortedLex.end(); it ++){
	lexiconStream << (* it) << endl;
    }
}

void Model::markUnary(const spear::BankEdgePtr & e,
		      int siblingCount) const
{
  if(e->isTerminal()){

    if(e->getLabel().substr(0, 2) == W("RB") && siblingCount == 1){
      String label = e->getLabel() + W("-U");
      e->setLabel(label);
    }

    return;
  }

  int count = e->getChildren().size();

  /*
  if(e->getLabel() != W("TOP") && count == 1){
    String label = e->getLabel() + W("-U");
    e->setLabel(label);
  }
  */

  for(list<BankEdgePtr>::const_iterator it = e->getChildren().begin();
      it != e->getChildren().end(); it ++){
    markUnary(* it, count);
  }
}

void Model::normalizeNumbers(const spear::BankEdgePtr & edge) const
{
  if(edge->isTerminal() && 
     edge->getLabel() == W("CD") &&
     isNumber(edge->getWord())){
    // CERR << "Found number: " << edge->getWord() << endl;
    edge->setWord(W("XX"));
  }

  for(list<BankEdgePtr>::const_iterator it = edge->getChildren().begin();
      it != edge->getChildren().end(); it ++){
    normalizeNumbers(* it);
  }
}

void Model::annotateTags(const spear::BankEdgePtr & edge,
			 const String & parent) const
{
  
}
