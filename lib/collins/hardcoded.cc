
#include <assert.h>
#include "collins/hardcoded.h"
#include "collins/grammar.h"

/* now some specific non-terminals: this needs to be generalized but for now
   we'll list them
*/
// Replaced these defines with the variables below
/*
#define PVB 45
#define PVBZ 50

#define NT_NP 22
#define NT_NPB 70
#define NT_CC 5
#define NT_TOP 43
*/

#define MAX_VERBS 1024
static int verb_labels [MAX_VERBS];
static int verb_count = 0;

#define MAX_NPS 1024
static int np_labels [MAX_NPS];
static int np_count = 0;

#define MAX_NPBS 1024
static int npb_labels [MAX_NPBS];
static int npb_count = 0;

#define MAX_CCS 1024
static int cc_labels [MAX_CCS];
static int cc_count = 0;

#define MAX_LRB 1024
static int lrb_labels [MAX_LRB];
static int lrb_count = 0;

#define MAX_RRB 1024
static int rrb_labels [MAX_RRB];
static int rrb_count = 0;

static int NT_TOP = 0;

// Some unused NT id to be used the STOP NT
#define NT_STOP 253

// Some unused lexicon id to be used as the STOP word
#define STOP_WORD 50002

#define LDEBUG 0

/**
 * Learns the non-terminals required by the parser
 * The goal is to remove all hardcoded info
 */
void learn_hardcoded(const char * label, 
		     int id)
{
  if(strncmp(label, "TOP", 3) == 0){
    NT_TOP = id;
    if(LDEBUG) fprintf(stderr, "TOP = %d\n", id);
  }

  else if(strncmp(label, "VB", 2) == 0){
    verb_labels[verb_count ++] = id;
    assert(verb_count < MAX_VERBS);
    if(LDEBUG) fprintf(stderr, "VB* = %d\n", id);
  }

  else if(strncmp(label, "NPB", 3) == 0){
    npb_labels[npb_count ++] = id;
    assert(npb_count < MAX_NPBS);
    if(LDEBUG) fprintf(stderr, "NPB* = %d\n", id);
  }

  else if(strncmp(label, "NP", 2) == 0){
    np_labels[np_count ++] = id;
    assert(np_count < MAX_NPS);
    if(LDEBUG) fprintf(stderr, "NP* = %d\n", id);
  }

  else if(strncmp(label, "CC", 2) == 0){
    cc_labels[cc_count ++] = id;
    assert(cc_count < MAX_CCS);
    if(LDEBUG) fprintf(stderr, "CC* = %d\n", id);
  }

  else if(strncmp(label, "-LRB-", 5) == 0){
    lrb_labels[lrb_count ++] = id;
    assert(lrb_count < MAX_LRB);
    if(LDEBUG) fprintf(stderr, "LRB* = %d\n", id);
  }

  else if(strncmp(label, "-RRB-", 5) == 0){
    rrb_labels[rrb_count ++] = id;
    assert(rrb_count < MAX_RRB);
    if(LDEBUG) fprintf(stderr, "RRB* = %d\n", id);
  }
}

/**
 * returns 1 if this label is a NPB, mihai
 */
int is_npb(int label)
{
  int i;
  for(i = 0; i < npb_count; i ++){
    if(label == npb_labels[i]) return 1;
  }

  return 0;
}

/** 
 * returns 1 if this label is any form of a non-NPB NP, mihai
 */
int is_high_np(int label)
{
  int i;
  for(i = 0; i < np_count; i ++){
    if(label == np_labels[i]) return 1;
  }

  return 0;
}

int isverb(int label)
{
  int i;
  for(i = 0; i < verb_count; i ++){
    if(label == verb_labels[i]) return 1;
  }

  return 0;
}

int is_top(int label)
{
  if(label == NT_TOP){
    return 1;
  }

  return 0;
}

int is_cc(int label)
{
  int i;
  for(i = 0; i < cc_count; i ++){
    if(label == cc_labels[i]) return 1;
  }

  return 0;
}

int is_lrb(int label)
{
  int i;
  for(i = 0; i < lrb_count; i ++){
    if(label == lrb_labels[i]) return 1;
  }

  return 0;
}

int is_rrb(int label)
{
  int i;
  for(i = 0; i < rrb_count; i ++){
    if(label == rrb_labels[i]) return 1;
  }

  return 0;
}

int get_stop_nt(void)
{
  return NT_STOP;
}

int get_stop_word(void)
{
  return STOP_WORD;
}

int get_start_nt(void)
{
  return NT_TOP;
}

