#ifndef MODEL_H_
#define MODEL_H_

/*!
 * @file Model.h
 *
 * @author Mihai Surdeanu
 * $Id: Model.h 18301 2014-11-26 19:17:13Z ksimek $ 
 */

#include <iostream>
#include <vector>

#include "spear/RCIPtr.h"
#include "spear/BankEdge.h"
#include "spear/StringMap.h"
#include "spear/Lexicon.h"
#include "spear/Wide.h"

namespace spear {

    class Model : public RCObject 
    {
    public:

	Model(size_t lf_thresh = 6)
	    : sentences_seen_(0),
	      nts_seen_(0),
	      lf_thresh_(lf_thresh)
	{}

	virtual void preprocessEdge(
	    spear::BankEdgePtr&              edge,
	    std::vector<spear::BankEdgePtr>& sentence);

	virtual void extractTerminals(
	    spear::BankEdgePtr&              edge,
	    std::vector<spear::BankEdgePtr>& sentence);

	virtual void generate_events(
	    OStream&                         os, 
	    spear::BankEdgePtr&              edge,
	    std::vector<spear::BankEdgePtr>& sentence);

	virtual void storeGrammar(
	    OStream& rulesStream,
	    OStream& labelsStream,
	    OStream& lexiconStream);

	virtual bool isEmptyNode(const spear::BankEdgePtr& edge) const;

    protected:

	virtual void pruneUnnecessaryNodes(spear::BankEdgePtr& edge);
	virtual bool isUnnecessaryNode(const spear::BankEdgePtr& edge) const;
	virtual void stripLabelAugmentation(spear::BankEdgePtr& edge) const;
	virtual void removeEmptyNodes(spear::BankEdgePtr& edge) const;
	virtual void addBaseNounPhrases(const spear::BankEdgePtr& edge) const;
	virtual bool isBaseNounPhrase(const spear::BankEdgePtr& edge) const;
	virtual void addBaseNounPhraseParents(
	    const spear::BankEdgePtr& edge) const;
	virtual bool isCoordinatedNounPhrase(
	    const spear::BankEdgePtr& edge) const;
	virtual void repairBaseNounPhrases(
	    const spear::BankEdgePtr& edge) const;
	virtual void labelSubjectlessSentencess(
	    const spear::BankEdgePtr& edge) const;
	virtual bool isSentenceWithNilSubject(
	    const spear::BankEdgePtr& edge) const;
	virtual void raisePunctuation(
	    const spear::BankEdgePtr&               edge,
	    std::list<spear::BankEdgePtr>&          parentChildren,
	    std::list<spear::BankEdgePtr>::iterator begin,
	    std::list<spear::BankEdgePtr>::iterator end) const;
	virtual bool isPunctuation(const spear::BankEdgePtr & edge) const;
	virtual void identifyArguments(const spear::BankEdgePtr & edge) const;
	virtual void repairSubjectlessSentences(
	    const spear::BankEdgePtr & edge) const;
	virtual void generateSentence(
	    const spear::BankEdgePtr&         edge,
	    std::vector<spear::BankEdgePtr>&  sentence,
	    bool                              setIndex
	    ) const;
	virtual void generateDependencyEvents(
	    OStream&                   os,
	    const spear::BankEdgePtr&  edge,
	    const spear::BankEdgePtr&  parent,
	    int                        parent_num);
	virtual bool isCoordination(
	    std::list<spear::BankEdgePtr>::const_iterator begin,
	    std::list<spear::BankEdgePtr>::const_iterator end,
	    const BankEdge*                               previous,
	    const BankEdge*                               parent
	    ) const;
	virtual bool isCoordination(
	    std::list<spear::BankEdgePtr>::reverse_iterator begin,
	    std::list<spear::BankEdgePtr>::reverse_iterator eoi,
	    const BankEdge*                                 previous,
	    const BankEdge*                                 parent
	    ) const;
	virtual bool containsVerb(const spear::BankEdgePtr & e) const;
	virtual bool splitLabel(
	    const String& label,
	    String&       coreLabel,
	    String&       labelAugment
	    ) const;
	virtual void learnLexicon(const spear::BankEdgePtr & e);
	virtual void markUnary(const spear::BankEdgePtr & edge, int) const;
	virtual void normalizeNumbers(const spear::BankEdgePtr & edge) const;
	virtual void annotateTags(
	    const spear::BankEdgePtr& edge,
	    const String& parent
	    ) const;
	virtual void addHorizontalHistory(BankEdgePtr & edge);
	virtual std::list<spear::BankEdgePtr>::iterator 
	findNext(
	    spear::BankEdgePtr&                     edge,
	    std::list<spear::BankEdgePtr>::iterator it
	    );
	virtual std::list<spear::BankEdgePtr>::reverse_iterator 
	findNext(
	    spear::BankEdgePtr&                             edge,
	    std::list<spear::BankEdgePtr>::reverse_iterator it
	    );
	int sentences_seen() const { return sentences_seen_;}
	int nts_seen() const { return nts_seen_;}
    protected:
	spear::StringMap<int> modifiers_;
	spear::Lexicon lexicon_;
	int sentences_seen_;
	int nts_seen_;
	size_t lf_thresh_;
    };

typedef RCIPtr<Model> ModelPtr;

} // end namespace spear

#endif /* MODEL_H */
