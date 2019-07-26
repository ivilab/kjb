/*!
 * @file Trainer.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include <fstream>

#include "spear/Trainer.h"
#include "spear/EdgeParser.h"
#include "spear/Exception.h"
#include "spear/CharUtils.h"

using namespace std;
using namespace spear;

Trainer::Trainer(const spear::ModelPtr & model)
    : lexicon_(new Lexicon()), 
      model_(model), 
      verbose_(VERBOSE_MEDIUM)
{
}

#ifdef PRINT_SENTENCE_FOR_CLUSTERING


static void printSentence(
    OStream&                                os,
    const std::vector<spear::BankEdgePtr>&  sentence)
// prints verbs (?)
{
    for(unsigned int i = 0; i < sentence.size(); i ++)
    {
	if(sentence[i]->getLabel().substr(0, 2) == W("VB"))
        {
	    os << sentence[i]->getWord() << W(" ");
	}
    }
}

#endif

static void printTerminals(OStream & os,
			   const BankEdgePtr & edge)
{
    // print word and label if edge is at a terminal
    if(edge->isTerminal())
    {
	os << edge->getWord();
	//os << toUpper(edge->getWord());
	os << "\t" << edge->getLabel() << "\n";
	return;
    }
    // recursively expand otherwise
    for(list<BankEdgePtr>::const_iterator it = edge->getChildren().begin();
        it != edge->getChildren().end(); it ++)
    {
	printTerminals(os, * it);
    }
}

bool Trainer::learn(
    IStream& syn_stream,
    const std::string& modelDir
    )
{
    // where to put the full record of all events
    string en = modelDir + "/events.unzipped";
    OFStream es(en.c_str());
  
    // record of productions
    string gn = modelDir + "/grammar.grm";
    OFStream gs(gn.c_str());

    // record of nonterminals
    string ln = modelDir + "/grammar.nts";
    OFStream ls(ln.c_str());

    // record of individual words
    string wn = modelDir + "/grammar.lexicon";
    OFStream ws(wn.c_str());

    //
    // Learning the lexicon and the grammar
    //
    try 
    {
	if(verbose_ > VERBOSE_LOW)
	{
	    cerr << "Learning lexicon and grammar..." << endl;
	}
	EdgeParser<BankEdge> parser(syn_stream); 
	// Creates edge to point to a node in a parse tree
	BankEdgePtr edge;
	int sentence_count = 0;
	int node_count = 0;

	while((edge = parser.parseEdge()) != (const BankEdge *) NULL)
	{
	    if(verbose_ == VERBOSE_HIGH)
	    {
		COUT << "Edge before preprocessing:" << endl;
		edge -> display(COUT);
		COUT << endl;
	    }
	    
	    vector<BankEdgePtr> sentence;

	    // calls functions in Model.cc to do preprocessing on
	    // edge and write results to sentence
	    model_ -> preprocessEdge(edge, sentence);

#ifdef PRINT_SENTENCE_FOR_CLUSTERING
	    printSentence(COUT, sentence);
#endif
	    // If you enable this (to train TnT), 
	    //   comment out "pruneUnnecessaryNodes();" in Model::preprocessEdge()
	    //printTerminals(COUT, edge);
	    //COUT << "\n";
	
	    if(verbose_ == VERBOSE_HIGH)
	    {
		COUT << "Edge after preprocessing:" << endl;
		edge->display(COUT);
		COUT << endl;
	    }
	    
	    // Once preprocessing is done, generate
	    // the event sequence for edge in sentence
	    // and output to es.  This deals with one entire sentence
	    // via recursion down the tree.
	    model_ -> generate_events(es, edge, sentence);
	
	    sentence_count ++; // increment the number of trees seen
	    if(verbose_ > VERBOSE_LOW && sentence_count % 1000 == 0)
	    {
		cerr << "Processed " << sentence_count
		     << " parse trees" << endl;
	    }
	} 

    } catch(Exception & e) {
	cerr << e.getMessage() << endl;
	return false;
    }

    model_->storeGrammar(gs, ls, ws);
    
    // success
    return true;
}
