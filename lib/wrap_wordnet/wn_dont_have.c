
#ifndef KJB_HAVE_WN

#include "wrap_wordnet/wn_dont_have.h" 

/* Kobus. I added this after discovering that a few bits of this library were
 * not gaurded. Furhter study revealed that most of it is guarded using
 * "add_error()". However, I think this method is a bit better, and so I am
 * leaving it in case someone wants to clean it up to be consistent. This will
 * not take long. 
*/
void set_dont_have_wn_error(void)
{
    set_error("Operation failed because the program was built without the ");
    add_error("WordNet libraries and headers readily available.");
    add_error("Appropriate installation, file manipulation and re-compiling ");
    add_error("is needed to fix this.)");
}

#endif 


