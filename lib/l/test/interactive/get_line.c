
/* $Id: get_line.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h"

/* -------------------------------------------------------------------------- */

static void print_read_result(int res);

/* -------------------------------------------------------------------------- */

/*ARGSUSED*/  /* Usually have "sig" as "int" as first arg (always on UNIX) */
static TRAP_FN_RETURN_TYPE atn_fn( TRAP_FN_ARGS )
{

    p_stderr("Interupted\n");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
int main(int argc, char **argv)
{
    FILE* fp;
    char  buff[ 10000 ];
    unsigned int i;
    int          res;
    unsigned int num_chars;
    int          would_block_count = 0;


    kjb_l_set("debug_level", "1"); 
    kjb_l_set("page", "off"); 

    num_chars = 2000;

    for (i=0; i < sizeof(buff) - ROOM_FOR_NULL; i++)
    {
        buff[ i ] = 'X';
    }

    buff[ sizeof(buff) - ROOM_FOR_NULL ] = '\0';

    /*
    //  p_stderr("\n -----------   TEST ZERO ------------------  \n"); 
    //  p_stderr("Testing reads from terminal.\n");
    //  p_stderr("First try term_get_line without ");
    //  p_stderr("term_set_raw_mode_with_no_echo.\n");
    //  EPE(res = term_get_line("one> ",buff, num_chars));
    //  print_read_result(res);
    */

    /*
    ----------------------------------------------------------------------------
    |                           TEST ONE 
    ----------------------------------------------------------------------------
    */
    p_stderr("\n -----------   TEST ONE  ------------------  \n"); 

    set_atn_trap(atn_fn, RESTART_AFTER_SIGNAL);
    ERE(term_set_raw_mode_with_no_echo());

    p_stderr("Using term_get_line(\"> \", buff, %d)\n", num_chars);

    would_block_count = 0;

    while ((res = term_get_line("one> ",buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    p_stderr("Do it (and all subsequent tests) with a small max_len parm.\n"); 
    num_chars = 30;

    p_stderr("Using term_get_line(\"> \", buff, %d)\n", num_chars);

    while ((res = term_get_line("two> ",buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    p_stderr("Try non-blocking now\n");
    term_set_no_blocking();


    while ((res = term_get_line("three> ",buff, num_chars)) != EOF)
    {
        if (res == WOULD_BLOCK) would_block_count++;

        if (res != WOULD_BLOCK)
        {
            print_read_result(res);
            p_stderr("BUFF: %s\n", buff);
        }
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    p_stderr("Try non-restarting atn now (back to blocking also) \n");
    dont_restart_on_atn();
    term_set_blocking();

    while ((res = term_get_line("four> ",buff, num_chars)) != EOF)
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    term_reset();

    restart_on_atn();
    p_stderr("Resetting to restart_on_atn.\n");

    /*
    ----------------------------------------------------------------------------
    |                           TEST TWO 
    ----------------------------------------------------------------------------
     */

    p_stderr("\n -----------   TEST TWO  ------------------  \n"); 


    NPETE(fp = kjb_fopen("/dev/tty", "r"));
    p_stderr("Using fget_line(buff, %d, fp) from %F\n", num_chars, fp); 

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    kjb_fclose(fp);

#ifdef SKIP 
    p_stderr("Try non-blocking now.\n");
    NPETE(fp = kjb_fopen("/dev/tty", "r"));
    set_no_blocking(fileno(fp));

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) 
        {
            would_block_count++;
        }
        else
        {
            print_read_result(res);
            p_stderr("BUFF: %s\n", buff);
        }

        clearerr(fp); 

        nap(100);
    }

    set_blocking(fileno(fp)); 
    kjb_fclose(fp);
#endif


    p_stderr("Try non-restarting atn now (and back to blocking) \n");

    NPETE(fp = kjb_fopen("/dev/tty", "r"));
    dont_restart_on_atn();

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;
    kjb_fclose(fp); 
    restart_on_atn();

    /*
    ----------------------------------------------------------------------------
    |                           TEST THREE 
    ----------------------------------------------------------------------------
    */

    p_stderr("\n -----------   TEST THREE ------------------  \n"); 

    if (argc < 2)
    {
        p_stderr("Skipping test three.\n");
        goto test_four;
    }

    NPETE(fp = kjb_fopen(argv[ 1 ], "r"));
    p_stderr("Using fget_line(buff, %d, fp) on %F\n", num_chars, fp); 

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    rewind(fp); 

    p_stderr("Try different buffer sizes. \n"); 

    i = 0;
    p_stderr("Buffer size is %d\n", i); 

    while ((res = fget_line(fp, buff, i)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        i++; 
        p_stderr("Buffer size is %d\n", i); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    rewind(fp); 

#ifdef SKIP 
    p_stderr("Try non-blocking now.\n");
    set_no_blocking(fileno(fp));

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) 
        {
            would_block_count++;
        }
        else
        {
            print_read_result(res);
            p_stderr("BUFF: %s\n", buff);
        }

        clearerr(fp); 

        nap(100);
    }


    rewind(fp); 

    set_blocking(fileno(fp)); 
#endif

    p_stderr("Try non-restarting atn now (and back to blocking) \n");

    NPETE(fp = kjb_fopen(argv[ 1 ], "r"));
    dont_restart_on_atn();

    while ((res = fget_line(fp, buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;
    kjb_fclose(fp); 
    restart_on_atn();


    /*
    ----------------------------------------------------------------------------
    |                           TEST FOUR
    ----------------------------------------------------------------------------
    */

test_four:
    p_stderr("\n -----------   TEST FOUR ------------------  \n"); 

    NPETE(fp = kjb_fopen("/dev/tty", "r"));
    p_stderr("Using dget_line(buff, %d, fp) on %D\n", num_chars, fileno(fp)); 

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        nap(100);
    }

    clearerr(fp); 
    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    p_stderr("Try different buffer sizes. \n"); 

    i = 0;
    p_stderr("Buffer size is %d\n", i); 

    while ((res = dget_line(fileno(fp), buff, i)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        i++; 
        p_stderr("Buffer size is %d\n", i); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    clearerr(fp); 
    would_block_count = 0;

    p_stderr("Try non-restarting atn now (still blocking) \n");

    set_blocking(fileno(fp)); 
    dont_restart_on_atn();

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;
    kjb_fclose(fp); 
    restart_on_atn();

    /*
    ----------------------------------------------------------------------------
    |                           TEST FIVE 
    ----------------------------------------------------------------------------
    */

    p_stderr("\n -----------   TEST FIVE ------------------  \n"); 

    if (argc < 2)
    {
        p_stderr("Skipping test five.\n");
        goto test_six;
    }

    NPETE(fp = kjb_fopen(argv[ 1 ], "r"));
    p_stderr("Using dget_line(buff, %d, fp) on %D\n", num_chars, fileno(fp)); 

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    rewind(fp); 

    p_stderr("Try different buffer sizes. \n"); 

    i = 0;
    p_stderr("Buffer size is %d\n", i); 

    while ((res = dget_line(fileno(fp), buff, i)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        i++; 
        p_stderr("Buffer size is %d\n", i); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    rewind(fp); 

    p_stderr("Try non-restarting atn now (still blocking) \n");

    NPETE(fp = kjb_fopen(argv[ 1 ], "r"));
    dont_restart_on_atn();

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;
    kjb_fclose(fp); 
    restart_on_atn();

    /*
    ----------------------------------------------------------------------------
    |                           TEST SIX  
    ----------------------------------------------------------------------------
    */

test_six: 

#ifdef TEST    /* kjb_popen is not used for prime time yet. */

    p_stderr("\n -----------   TEST SIX  ------------------  \n"); 

    NPETE(fp = kjb_popen("slow_write", "r"));

    p_stderr("Using dget_line(buff, %d, fp) on %D\n", num_chars, fileno(fp)); 

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    kjb_pclose(fp);

    p_stderr("Try different buffer sizes. \n"); 
    would_block_count = 0;
    NPETE(fp = kjb_popen("slow_write", "r"));

    i = 0;
    p_stderr("Buffer size is %d\n", i); 

    while ((res = dget_line(fileno(fp), buff, i)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        i++; 
        p_stderr("Buffer size is %d\n", i); 
        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;

    kjb_pclose(fp);
    NPETE(fp = kjb_popen("slow_write", "r"));


    p_stderr("Try non-blocking now.\n");

    set_no_blocking(fileno(fp));

    i = 0; 

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) 
        {
            would_block_count++;

            if (i >= 5) i++;
            if (i == 10) 
            {
                p_stderr("Breaking because i==10\n");
                break; 
            }
        }
        else
        {
            print_read_result(res);
            p_stderr("BUFF: %s\n", buff);
            i++;
        }

        clearerr(fp); 

        nap(100);
    }

    set_blocking(fileno(fp)); 

    kjb_pclose(fp);


    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    p_stderr("Try non-restarting atn now (and back to blocking) \n");

    would_block_count = 0;

    NPETE(fp = kjb_popen("slow_write", "r"));

    dont_restart_on_atn();

    while ((res = dget_line(fileno(fp), buff, num_chars)) != EOF) 
    {
        if (res == WOULD_BLOCK) would_block_count++;

        print_read_result(res);
        p_stderr("BUFF: %s\n", buff);
        clearerr(fp); 

        nap(100);
    }

    p_stderr("%d WOULD_BLOCK's\n", would_block_count);
    would_block_count = 0;
    kjb_pclose(fp); 

    restart_on_atn();

#endif 

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void print_read_result(int res)
{
    char status_str[ 100 ]; 

    get_status_string(res, status_str, sizeof(status_str)); 
    EPETE(p_stderr("Read result is: %s.\n", status_str)); 
}



