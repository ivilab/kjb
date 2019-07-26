/* $Id: is_interactive.c 15880 2013-10-24 04:56:28Z predoehl $
 */
#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>

int main(int argc, char** argv)
{
	kjb_printf("is_interactive() returns %d.\n", is_interactive());
	return EXIT_SUCCESS;
}

