
/*
    Kobus. We put this first (if we put it anywhere). It is good to have this
    somewhere because it catches name clashes and similar problems early.
    However, right now I am putting it here to make sure we have DBL_MIN and
    MINDOUBLE. 
*/
#include "l/l_incl.h" 

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

/* 
   Kobus. This was for MINDOUBLE. I think MINDOUBLE might be an old name for the
   more standard DBL_MIN. I have patched this in l_incl.h above. 
*/
/* #include <values.h> */

#include "collins/Collins.h"
#include "collins/SharedMemory.h"
#include "spear/CharUtils.h"

#include "collins/lexicon.h"
#include "collins/grammar.h"
#include "collins/mymalloc.h"
#include "collins/mymalloc_char.h"
#include "collins/hash.h"
#include "collins/prob.h"
#include "collins/readevents.h"
#include "collins/sentence.h"
#include "collins/chart.h"

using namespace std;
using namespace spear;

bool Collins::initialized = false;

bool Collins::initialize(
    const std::string&  modelDirectory,
    double              beam
    )
{
    if(initialized == true)
    {
	cerr << "Parser already initialized." << endl;
	return true;
    }

    initialized = true;

    BEAMPROB = log(beam);
    PUNC_FLAG = 1;
    DISTAFLAG = 1;
    DISTVFLAG = 1;

    mymalloc_init();
    mymalloc_char_init();

    /* this is the cache!! */
    effhash_make_table(1000003,&eff_hash);

    string grammarName = modelDirectory + "/grammar";
    if(! read_grammar(grammarName.c_str()))
    {
	cerr << "Cannot read grammar files" << endl;
	return false;
    }

#ifdef USE_SHARED_EVENTS

    void * address = NULL;
    bool isNew = false;
  
    if((address = SharedMemory::make(false)) == NULL)
    {
	cerr << "Failed to attach shared memory segments. "
	     << "Trying to create..." << endl;

	if((address = SharedMemory::make(true)) == NULL)
	{
	    cerr << "Failed to create shared memory segments." << endl;
	    return false;
	}

	isNew = true;
    } 

    // the events hash is the first thing allocated in shared memory
    if(isNew == false)
    {
	// if already created just use the first valid address
	new_hash = (hash_table *) address;
    } else {
	// create the hash in shared memory
	new_hash = (hash_table *) SharedMemory::alloc(sizeof(hash_table));
	hash_make_shared_table(8000007, new_hash);

	string eventsName = modelDirectory + "/events.unzipped";
	FILE * eventStream = NULL;
	if((eventStream = fopen(eventsName.c_str(), "r")) == NULL)
	{
	    cerr << "Cannot open event file: " << eventsName << endl;
	    return false;
	}

	// allocate all events in shared memory
	read_events(eventStream, new_hash, -1, true);
	fclose(eventStream);
    }

#else

    new_hash = (hash_table *) malloc(sizeof(hash_table));
    hash_make_table(8000007,new_hash);

    string eventsName = modelDirectory + "/events.unzipped";
    FILE * eventStream = NULL;
    if((eventStream = fopen(eventsName.c_str(), "r")) == NULL)
    {
	cerr << "Can not open event file: " << eventsName << endl;
	return false;
    }
    read_events(eventStream, new_hash, -1);
    fclose(eventStream);

#endif

    return true;
}

static String normalizeWord(
    const String&  word,
    const String&  tag)
{
    // number normalization is not used by default, so just return the word
    return word;

    if(tag == W("CD") && isNumber(word))
    {
	CERR << "Normalized " << word << " to XX" << endl;
	return W("XX");
    }

    return word;
}

static bool convertToCollins(
    const std::vector<spear::Word>& api,
    sentence_type&                  collins)
{
    collins.nws = api.size();
    assert(collins.nws < PMAXWORDS);

    for(unsigned int i = 0; i < api.size(); i ++)
    {
	String normalized = normalizeWord(api[i].getWord(), api[i].getTag());
	collins.words[i] = strdup(normalized.c_str());
	assert(collins.words[i] != NULL);
	collins.tags[i] = strdup(api[i].getTag().c_str());
	assert(collins.tags[i] != NULL);
    }

    convert_sentence(&collins);
    return true;
}

static TreePtr extractBestParse(double& prob)
{
    int best = find_best_parse(prob);

    /** no parse exists */
    if(best == -1)
    {
	prob = MINDOUBLE;
	return TreePtr();
    }
  
    TreePtr tree = buildTree(best);

    return tree;
};

static TreePtr makeDummy(const std::vector<spear::Word> & sentence)
{
    // make a S with all terminals
    TreePtr s(new Tree(W("S")));
    for(unsigned int i = 0; i < sentence.size(); i ++)
    {
	TreePtr c(new Tree(sentence[i].getWord(), sentence[i].getTag()));
	s->addChild(c);
    }
    if(s->getChildren().empty() == false)
    {
	s->setHead(s->getChildren().front());
    }

    // make a TOP
    TreePtr top(new Tree(W("TOP")));
    top->addChild(s);
    top->setHead(top->getChildren().front());

    return top;
}

static void restoreWords(
    const TreePtr & edge,
    const std::vector<spear::Word> & sentence,
    unsigned int & position
    )
{
    if(edge->isTerminal())
    {
	edge->setWord(sentence[position ++].getWord());
    } else {
	for(list<TreePtr>::const_iterator it = edge->getChildren().begin();
	    it != edge->getChildren().end(); it ++)
	{
	    restoreWords(* it, sentence, position);
	}
    }
}

static void restoreLabels(const TreePtr & edge)
{
    if(edge->getLabel() == W("SG"))
    {
	edge->setLabel(W("S"));
    } else if(edge->getLabel() == W("VPG")) {
	edge->setLabel(W("VP"));
    }

    for(list<TreePtr>::const_iterator it = edge->getChildren().begin();
	it != edge->getChildren().end();
	it ++)
    {
	restoreLabels(* it);
    }
}

/** Removes the NP edge in constructs such as (NP (NPB ...)) */
static void removeSingleNp(const TreePtr & edge)
{
    for(list<TreePtr>::iterator it = edge->children().begin();
	it != edge->children().end();
	)
    {

	// (NP (NPB ...)) found
	if((* it)->getLabel() == W("NP") &&
	   (* it)->children().size() == 1 &&
	   (* it)->children().front()->getLabel() == W("NPB"))
	{
	    // the embedded NPB
	    TreePtr npb = (* it)->children().front();
      
	    // change head if necessary
	    if(edge->getHead() == * it)
	    {
		edge->setHead(npb);
	    }
      
	    // remove the redundant NP
	    it = edge->children().erase(it);
	    edge->children().insert(it, npb);
      
	} else {
	    it++;
	}
    }

    for(list<TreePtr>::const_iterator it = edge->getChildren().begin();
	it != edge->getChildren().end();
	it ++)
    {
	removeSingleNp(* it);
    }
}

/** Changes NPB labels to NP */
static void changeNpb(const TreePtr & edge)
{
    if(edge->getLabel() == W("NPB"))
    {
	edge->setLabel(W("NP"));
    }

    for(list<TreePtr>::const_iterator it = edge->getChildren().begin();
	it != edge->getChildren().end(); it ++)
    {
	changeNpb(* it);
    }
}

TreePtr Collins::parse(
    const std::vector<spear::Word>& sentence,
    double&                         prob, 
    bool                            discardNpbFlag,
    bool                            traceFlag)
{
    // if true removes NPB labels from generated trees
    set_treebankoutputflag(discardNpbFlag);

    // if true displays the trees in Collins' "raw" format
    set_traceoutput(traceFlag);

    // Construct the representation required by Collins
    sentence_type collinsSentence;
    convertToCollins(sentence, collinsSentence);

    // The actual parsing code
    pthresh = -5000000;
    parse_sentence(&collinsSentence);

    TreePtr best = extractBestParse(prob);

    // create a dummy tree if the parse failed
    if(best == (const Tree *) NULL)
    {
	best = makeDummy(sentence);
	prob = MINDOUBLE;
    } else {
	unsigned int position = 0;
	restoreWords(best, sentence, position);
	restoreLabels(best);
	if(discardNpbFlag == true)
	{
	    removeSingleNp(best);
	    changeNpb(best);
	}
    }

    // Cleanup
    free_sentence(&collinsSentence);
    return best;
}
