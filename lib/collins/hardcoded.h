
#ifndef HARD_CODED_H
#define HARD_CODED_H

/*a function that returns 1 if the tag is a verb*/
int isverb(int tag);

/** 
 * returns 1 if this label is any form of a non-NPB NP, mihai
 */
int is_high_np(int label);

/**
 * returns 1 if this label is a NPB, mihai
 */
int is_npb(int label);

int is_cc(int label);

int is_top(int label);

int get_stop_nt(void);

int get_stop_word(void);

int get_start_nt(void);

int is_lrb(int label);

int is_rrb(int label);

void learn_hardcoded(const char * label, int id);

#endif
