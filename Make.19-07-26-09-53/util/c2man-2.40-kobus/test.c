/* - A test file for compiler defines.  To compile: cc test.c -lcurses   */

/*  The only function in this program is called main
    It uses curses to print out some of your system
    parameters.

*/

/* #include <curses.h> */

main()
{
	initscr();

	move(10,10);

#ifdef HPUX
	attrset(A_BOLD);
    	printw("HPUX");
#define ok
#endif

#ifdef SYSV
attrset(A_UNDERLINE);
  	printw("SYSV");
#define ok
#endif

#ifdef hpux
	attrset(A_REVERSE);
  	printw("hpux");
#define ok
#endif

	attrset(0);

#ifdef ok
	printw(" was defined");
#else
	printw(" Nothing defined");
#endif

	refresh();
	endwin();
 
}

