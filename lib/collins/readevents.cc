/* This code is the statistical natural language parser described in

   M. Collins. 1999.  Head-Driven
   Statistical Models for Natural Language Parsing. PhD Dissertation,
   University of Pennsylvania.

   Copyright (C) 1999 Michael Collins

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <assert.h>
#include "collins/readevents.h"
#include "collins/hardcoded.h"

static const int U_CODE = 3;
static const int D_CODE = 2;
static const int F_CODE = 6;
static const int G_CODE = 4;

void read_events_s(FILE *file,hash_table *hash, bool shared);
void read_events_d(FILE *file,hash_table *hash, bool shared);
void read_events_u(FILE *file,hash_table *hash, bool shared);
void read_events_g(FILE *file,hash_table *hash, bool shared);

int read_events_word(FILE *file);
int read_events_nt(FILE *file);

int read_events_word2(FILE *file);
int read_events_nt2(FILE *file);

void read_events(FILE *file,hash_table *hash,int max, bool shared)
{ // overall function to read in a training file
    int m=0;
    int code;

    while( fscanf(file,"%d",&code) != EOF && (m<max||max==-1) )
    { // read in a line at a time until end of file
	m++;

/*      printf("MMM %d\n",m);*/

//        if(((double) (m/100000))==(((double) m)/100000.0))
//  	fprintf(stderr,"Hash table: %d lines read\n",m);
	switch(code)
	{
	case(F_CODE): // in this case, 6
	    //read as a linear sequence of tags and words
	    read_events_s(file,hash, shared); break;
	case(D_CODE): // in this case, 2
	    // read in the line as a modifier dependency
	    read_events_d(file,hash, shared); break;
	case(U_CODE): // in this case, 3
	    // read in the line as a head event
	    read_events_u(file,hash, shared); break;
	case(G_CODE): // would be 4, but not used, currently
	    // read in the line as a gap event
	    read_events_g(file,hash, shared); break;
	default:
	    assert(0);
	}
    }
}


void read_events_s(FILE *file,hash_table *hash, bool shared)
{ // read in a line as a linear sequence of words and PoS tags
    int i,sentence_len,sentence_num;
    char word[1000],tag[1000];
    int wn,tn;

    fscanf(file,"%d",&sentence_num);
    fscanf(file,"%d",&sentence_len);

    for(i = 0; i < sentence_len; i++)
    {
	// read in two space-delimited elements
	// as word and PoS tag, respectively
	fscanf(file,"%s %s",word,tag); 

	// look up word in lexicon
	wn = find_word(word,&wordlex);

	if(!(wn>=0))
	{   // throws an error if word is not found
	    fprintf(stderr,"ERROR: %s not found in lexicon\n",word);
	    assert(0);
	}

	// look up PoS tag in nonterminal lexicon
	tn = find_word(tag,&nt_lex);
	if(!(tn>=0))
	{   // throw an error if tag is not found
	    fprintf(stderr,"ERROR: %s not found in lexicon\n",tag);
	    assert(0);
	}

	/*finally add counts for the word/tag pair to the hash table
	  add_tagword_entries2(wn,tn,hash);*/
	add_tagword_counts( wn, tn, hash, shared );
    }
}

void read_events_d(FILE *file,hash_table *hash, bool shared)
{ // reads in an event of type "2" from training file
  /* read in the following values:

     wm/tm, wh/th are modifer word/tag, head word/tag 
     p,ch,cm are parent, head and modifier non-terminals
     cc = 1 if coordination, 0 otherwise
     punc = 1 if punctuation, 0 otherwise
     
     wcc/tcc, wpunc/tpunc are coordinator word/tag, punctuation word/tag

     subcat and dist are the subcategorization and distance strings
  */

    int wm,tm,wh,th,p,ch,cm,cc,wcc = -1,tcc = -1,punc,wpunc = -1,tpunc = -1;
    int subcat,dist;
    int nt_num, parent_num, head_num;

    fscanf(file,"%d",&nt_num);
    fscanf(file,"%d",&parent_num);
    fscanf(file,"%d",&head_num);
    
    wm = read_events_word(file); // first entry is the modifier's head word
    tm = read_events_nt(file); // second entry is the modifier's head tag

    wh = read_events_word(file); // next is the word of the head being modified
    th = read_events_nt(file); // and then the PoS of the head being modified

    cm = read_events_nt(file); // then the constituent label for the modifier
    p = read_events_nt(file); // then the parent constituent
    ch = read_events_nt(file); // then the constituent of the head being modified

    assert( fscanf(file,"%d",&subcat) != EOF); // read in subcategorization frame

    assert( fscanf(file,"%d",&dist) != EOF ); // read in distance measure

    assert( fscanf(file,"%d",&cc) != EOF); // read in conjunction flag

    if(cc)
    {   // there may or may not be conjunction information after the above
	// if so, read it in
	// (but this does not appear to be used currently?)
	wcc = read_events_word(file);
	tcc = read_events_nt(file);
    }

    assert(fscanf(file,"%d",&punc)!=EOF); // read in punctuation flag

    if(punc)
    {   // there may or may not be punctuation information after the above
	// (not used currently?)
	wpunc = read_events_word(file);
	tpunc = read_events_nt(file);
    }

    if(cm == get_stop_nt()) punc = 0; // ignore punctuation if at a STOP

    // tabulate everything (calls function from prob.cc,
    // which in turn calls genprob.cc)
    add_dependency_counts(wm,tm,cm,
			  wh,th,
			  p,ch,
			  dist,subcat,
			  cc,wcc,tcc,
			  punc,wpunc,tpunc,
			  hash, shared);
}

void read_events_u(FILE *file,hash_table *hash, bool shared)
{
    int wh,th,p,ch,lsubcat,rsubcat;
    int nt_num, parent_num;

    fscanf(file,"%d",&nt_num);
    fscanf(file,"%d",&parent_num);
    
    wh = read_events_word(file); // first entry is head word
    th = read_events_nt(file); // next is head PoS
    // at top level, wh is generated conditional on ch and th.
    // after top level, wh and th are passed down as context
    // to generate next ch

    // next is parent constituent.
    // at top level this is just TOP, and add_unary_counts
    // uses s1_string and s2_string
    p = read_events_nt(file);
    
    // next is head constituent.  At top level, this is generated
    // first together with th.  After top level, this is generated
    // conditional on wh and th, which are passed all the way down.
    ch = read_events_nt(file); 

    assert(fscanf(file,"%d",&lsubcat)!=EOF);
    assert(fscanf(file,"%d",&rsubcat)!=EOF);

    /*now add the counts*/

    // update the hash table to reflect these counts
    add_unary_counts( ch, wh, th, p, hash, shared );
    add_subcat_counts( lsubcat, ch, wh, th, p, 0, hash, shared );
    add_subcat_counts( rsubcat, ch, wh, th, p, 1, hash, shared );

/*  u.p = p;
    u.ch = ch;
    u.wh = wh;
    u.th = th;

    add_entries_U(&u,hash,HUR);
    add_entries_S(&u,hash,lsubcat,rsubcat);*/
}

void read_events_g(FILE *file,hash_table *hash, bool shared)
{
  int wh,th,p,ch,gap;

  assert(fscanf(file,"%d",&gap)!=EOF);
  wh=read_events_word(file);
  th=read_events_nt(file);

  p=read_events_nt(file);
  ch=read_events_nt(file);

  /*now add the counts*/

  add_gap_counts(gap,ch,wh,th,p,hash, shared);
}



int read_events_word(FILE *file)
{   // reads in an entry as a word
    char buffer[1000];
    int w;

    assert( fscanf(file,"%s",buffer) != EOF);

    if( strcmp(buffer,"#STOP#") == 0 )
    {   // if token is #STOP#, substitute the special STOP code
	w = get_stop_word();
    } else
    {
	// otherwise look up the word in the lexicon,
	// and return the corresponding code
	w = find_word(buffer,&wordlex);
    }
    if( !(w>=0) )
    {   // if it is not found, throw an error
	fprintf(stderr,"ERROR: word %s not found in lexicon\n",buffer);
	assert(0);
    }

    return w;
}

int read_events_nt(FILE *file)
{   // read in a token as a nonterminal
    char buffer[1000];
    int w;

    assert( fscanf(file,"%s",buffer) != EOF );
    if(strcmp(buffer,"#STOP#")==0)
    {   // if it is #STOP#, replace with the code for a 
	// #STOP# nonterminal
	w = get_stop_nt();
    } else
    {   // otherwise, look up the nonterminal in the 
	// nonterminal lexicon, and return its code
	w = find_word(buffer,&nt_lex);
    }
    if(!(w>=0))
    {   // if it's not found, throw an error
	fprintf(stderr,"ERROR: %s not found in lexicon\n",buffer);
	assert(0);
    }

    assert(w>=0);
    return w;
}

int read_events_word2(FILE *file)
{   // alternative version to read in literally,
    // instead of looking up a code
    char buffer[1000];

    assert(fscanf(file,"%s",buffer)!=EOF);

    return 0;
}

int read_events_nt2(FILE *file)
{   // alternative version to read in literally
    // instead of looking up a code
    char buffer[1000];

    assert(fscanf(file,"%s",buffer)!=EOF);

    return 0;

}


