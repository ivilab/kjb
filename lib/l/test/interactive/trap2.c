
/* $Id: trap2.c 21602 2017-07-31 20:36:24Z kobus $ */



#include "l/l_incl.h" 

static TRAP_FN_RETURN_TYPE trap_fn_0(int); 
static TRAP_FN_RETURN_TYPE trap_fn_1(int); 
static TRAP_FN_RETURN_TYPE trap_fn_2(int); 
static TRAP_FN_RETURN_TYPE trap_fn_3(int); 
static TRAP_FN_RETURN_TYPE trap_fn_4(int); 
static TRAP_FN_RETURN_TYPE trap_fn_5(int); 
static TRAP_FN_RETURN_TYPE trap_fn_6(int); 
static TRAP_FN_RETURN_TYPE trap_fn_7(int); 
static TRAP_FN_RETURN_TYPE trap_fn_8(int); 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line [ 100 ];
    kjb_int32 res; 
    kjb_int32 level = 0; 
    TRAP_FN_RETURN_TYPE (*trap_fns[ 9 ])(); 


    trap_fns[ 0 ] = (void(*)())trap_fn_0; 
    trap_fns[ 1 ] = (void(*)())trap_fn_1; 
    trap_fns[ 2 ] = (void(*)())trap_fn_2; 
    trap_fns[ 3 ] = (void(*)())trap_fn_3; 
    trap_fns[ 4 ] = (void(*)())trap_fn_4; 
    trap_fns[ 5 ] = (void(*)())trap_fn_5; 
    trap_fns[ 6 ] = (void(*)())trap_fn_6; 
    trap_fns[ 7 ] = (void(*)())trap_fn_7; 
    trap_fns[ 8 ] = (void(*)())trap_fn_8; 

    set_atn_trap(trap_fn_0, RESTART_AFTER_SIGNAL); 

    while ((res = BUFF_STDIN_GET_LINE("trap> ", line)) != EOF)
    {
        dbi(res);
        dbs(line); 

        if (STRCMP_EQ(line, "s"))
        {
            level++;
            EPE(set_atn_trap((void(*)(int))trap_fns[ level ], RESTART_AFTER_SIGNAL)); 

            dbi(level); 
        }
        else if (STRCMP_EQ(line, "u"))
        {
            if (level > 0 ) level--;
            EPE(unset_atn_trap()); 
            dbi(level); 
        }
        else if (STRCMP_EQ(line, "d"))
        {
            EPE(dont_restart_on_atn());
        }
        else if (STRCMP_EQ(line, "r"))
        {
            EPE(restart_on_atn());
        }
        else if (STRCMP_EQ(line, "e"))
        {
            EPE(reset_restart_on_atn());
        }
        else
        {
            dbm("No level change.\n"); 
        }
    }

    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_0(TRAP_FN_ARGS) 
{

    p_stderr("Zero.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_1(TRAP_FN_ARGS) 
{

    p_stderr("One.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_2(TRAP_FN_ARGS) 
{

    p_stderr("Two.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_3(TRAP_FN_ARGS) 
{

    p_stderr("Three.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_4(TRAP_FN_ARGS) 
{

    p_stderr("Four.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_5(TRAP_FN_ARGS) 
{

    p_stderr("Five.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_6(TRAP_FN_ARGS) 
{

    p_stderr("Six.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_7(TRAP_FN_ARGS) 
{

    p_stderr("Seven.\n"); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE trap_fn_8(TRAP_FN_ARGS) 
{

    p_stderr("Eight.\n"); 
}




