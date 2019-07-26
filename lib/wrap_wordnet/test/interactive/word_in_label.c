/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
| Authors:
|     Luca Del Pero
|
* =========================================================================== */

#include <wrap_wordnet/kjb_wn.h>

#ifdef KJB_HAVE_WN
#include "wn.h"

int main(int argc, char **argv)
{
    Array * words = NULL;

    if (! is_interactive()) 
    {
        p_stderr("This test program only works interactively.\n");
        return EXIT_CANNOT_TEST;
    }
    
	wninit();
	printf("%d\r\n",morphinit());
	if(!read_region_labels_to_array(argv[1], 1, 1, &words))
	{
		printf("Could not read label file\r\n");
		return 1;
	}

    printf("Is it in? %d\r\n",recursively_find_word_in_label_list(words, "person", 1 ));
    free_array(words,free);
}

#else

int main(int argc, char **argv)
{
	printf("No wordnet available!\r\n");
	return 1;
}

#endif



