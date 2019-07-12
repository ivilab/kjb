
/* $Id: p_plot.c 16078 2013-11-23 21:01:35Z kobus $ */

 
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
* =========================================================================== */

#include "p/p_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "p/p_plot.h"
#include "m/m_hist.h"
#include "wrap_X11/wrap_X11.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_MAX_NUM_PLOTS            1000
#define DEFAULT_MAX_NUM_PLOTS         100
#define PLOT_COMMAND_BUFF_SIZE      20000
#define SET_PLOT_RANGE(x)        (x>-1e29)

/* -------------------------------------------------------------------------- */

typedef struct Plot
{
    int    dim;
    int    plot_pid;
    int    create_pid;
    int    plot_id;
    int    plot_write_des;
    int    plot_read_des;
    int    num_temp_files;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    double z_min;
    double z_max;
    void*  attached_data_ptr;
    void   (*attached_data_disposal_fn)(void *);
    char   plot_dir_path[ 2048 ]; 
}
Plot;

/* -------------------------------------------------------------------------- */

static int            fs_colour_plot_flag        = TRUE;
static int            fs_max_plot_num            = 0;
static int            fs_current_plot            = NOT_SET;

static int            fs_max_num_plots           = DEFAULT_MAX_NUM_PLOTS;

static Plot**         fs_plot_info_array         = NULL;
static int            fs_display_plot_flag       = TRUE;

static Queue_element* fs_display_plot_flag_stack_head = NULL;

#ifdef TRACK_MEMORY_ALLOCATION
    static int        fs_display_plot_flag_stack_first_use = TRUE;
#endif

/* -------------------------------------------------------------------------- */

static int plot_open_with_dimension
(
    int dim,
    int x_size,
    int y_size,
    int x_tlc,
    int y_tlc
);

static Plot* pp_plot_open
(
    int plot_num,
    int x_size,
    int y_size,
    int x_tlc,
    int y_tlc,
    int dim
);

static Plot* get_plot_ptr(int plot_num);

static int pp_plot_points
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
);

static int pp_plot3_points
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
);

static int pp_plot3_curve
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
);

static int pp_plot_segments
(
    Plot*         plot_ptr,
    const Vector* x1_vp,
    const Vector* y1_vp,
    const Vector* x2_vp,
    const Vector* y2_vp,
    const char*   name
);

static int pp_plot_update(Plot* plot_ptr);
static int pp_plot_clear(Plot* plot_ptr);
static int pp_plot_clear_data(Plot* plot_ptr);
static int pp_plot_clear_plot(Plot* plot_ptr);
static int pp_plot_close(Plot* plot_ptr);

static int pp_plot_write(Plot* plot_ptr, const char* buff);

static FILE* open_plot_temp_file(int plot_id, const char* file_name);

static int get_plot_temp_file_path
(
    char*       file_path_buff,
    size_t      file_path_buff_size,
    int         plot_id,
    const char* file_name
);

static int get_plot_temp_file_dir
(
    char*  dir_buff,
    size_t dir_buff_size,
    int    plot_id
);

static int get_plot_temp_file_name
(
    char*  file_name_buff,
    size_t file_name_buff_size,
    int    file_num
);

static int plot_remove_data_files(Plot* plot_ptr);

static int wait_for_plot(Plot* plot_ptr);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_display_plot_flag_stack(void);
#endif

/* -------------------------------------------------------------------------- */

int set_colour_plot(void)
{

    fs_colour_plot_flag = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_monochrome_plot(void)
{

    fs_colour_plot_flag = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_max_num_plots(int max_num_plots)
{

    if (max_num_plots < 1)
    {
        set_error("Max num plots must be at least one.");
        return ERROR;
    }
    else if (max_num_plots > MAX_MAX_NUM_PLOTS)
    {
        set_error("Maximum value of max num plots is %d.", MAX_MAX_NUM_PLOTS);
        return ERROR;
    }
    else if ((fs_max_plot_num > 0) && (max_num_plots < fs_max_plot_num))
    {
        set_error("The maximum number of open plots (%d) has exceeded %d.",
                  fs_max_plot_num, max_num_plots);
        add_error("The max number of plots is not changed.");
        return ERROR;
    }

    fs_max_num_plots = max_num_plots;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            push_display_plot_flag
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int push_display_plot_flag(int flag)
{
    int* save_flag_ptr;


    NRE(save_flag_ptr = INT_MALLOC(1));
    *save_flag_ptr = fs_display_plot_flag;

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_display_plot_flag_stack_first_use)
    {
        fs_display_plot_flag_stack_first_use = FALSE;
        add_cleanup_function(free_display_plot_flag_stack);
    }
#endif

    ERE(insert_into_queue(&fs_display_plot_flag_stack_head,
                          (Queue_element**)NULL,
                          (void*)save_flag_ptr));

    fs_display_plot_flag = flag;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            pop_display_plot_flag
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int pop_display_plot_flag(void)
{
    Queue_element* cur_elem;

    if (fs_display_plot_flag_stack_head == NULL)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    NRE(cur_elem = remove_first_element(&fs_display_plot_flag_stack_head,
                                        (Queue_element**)NULL));

    fs_display_plot_flag = *((int*)(cur_elem->contents));

    free_queue_element(cur_elem, kjb_free);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                            plot_open
 *
 * Returns an integer handle for a plot
 *
 * This routine opens a 2D plot. It is necessary before any plotting can be
 * done. Most routines require a plot handle as an argument. It provides an
 * integeger handle that is used to modify the plot. Plots should be dispossed
 * of by plot_close().
 *
 * Returns:
 *     The plot handle non-negative, or ERROR (negative).
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_open(void)
{
    return plot_open_with_dimension(2, NOT_SET, NOT_SET, NOT_SET, NOT_SET);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_open3
 *
 * Returns an integer handle for a 3D plot
 *
 * This routine opens a 3D plot. It is otherwise similar plot_open().
 *
 * Returns:
 *     The plot handle non-negative, or ERROR (negative).
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_open3(void)
{
    return plot_open_with_dimension(3, NOT_SET, NOT_SET, NOT_SET, NOT_SET);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            special_plot_open
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int special_plot_open
(
    int x_size,
    int y_size,
    int x_tlc,
    int y_tlc,
    int dim
)
{
    return plot_open_with_dimension(x_size, y_size, x_tlc, y_tlc, dim);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      plot_open_with_dimension
 * -----------------------------------------------------------------------------
 */

static int plot_open_with_dimension
(
    int dim,
    int x_size,
    int y_size,
    int x_tlc,
    int y_tlc
)
{
    static int victim   = 0;
    Plot*      plot_ptr;
    int        i;
    int        plot_num = NOT_SET;


    if (fs_max_num_plots < 1)
    {
        set_bug("Max number of plots is not positive.");
        return ERROR;
    }
    else if (fs_max_num_plots > MAX_MAX_NUM_PLOTS)
    {
        SET_CANT_HAPPEN_BUG();
    }

    if (fs_plot_info_array == NULL)
    {
        SKIP_HEAP_CHECK_2();
        NRE(fs_plot_info_array = N_TYPE_MALLOC(Plot*, MAX_MAX_NUM_PLOTS));
        CONTINUE_HEAP_CHECK_2();

        for (i=0; i<MAX_MAX_NUM_PLOTS; i++)
        {
            fs_plot_info_array[ i ] = NULL;
        }
    }

    for (i=0; i < fs_max_num_plots; i++)
    {
        if (fs_plot_info_array[ i ] == NULL)
        {
            plot_num = i;
            break;
        }
    }

    if (plot_num == NOT_SET)
    {
        plot_num = victim % fs_max_num_plots;
        ERE(plot_close(plot_num));
        victim++;
    }

    fs_current_plot = plot_num;

    fs_max_plot_num = MAX_OF(fs_max_plot_num, plot_num);

    if (x_size <= 0) x_size = 800;
    if (y_size <= 0) y_size = 600;
    if (x_tlc  <= 0) x_tlc  = 20+120*(plot_num%4);
    if (y_tlc  <= 0) y_tlc  = 20+120*(plot_num/4);

    NRE(plot_ptr = pp_plot_open(plot_num, x_size, y_size, x_tlc, y_tlc, dim));

    fs_plot_info_array[ plot_num ] = plot_ptr;

    return plot_ptr->plot_id;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Plot* pp_plot_open
(
    int plot_num,
    int x_size,
    int y_size,
    int x_tlc,
    int y_tlc,
    int dim
)
{
#ifdef UNIX
    static int first_time         = TRUE;
    int        plot_pipe[2];
    int        error_pipe[2];
    int        plot_pid;
    char       dir_buff[ MAX_FILE_NAME_SIZE ];


    if (first_time)
    {
        add_cleanup_function(plot_close_all);
        first_time = FALSE;
    }

    ERN(get_plot_temp_file_dir(dir_buff, sizeof(dir_buff), plot_num + 1));
    ERN(kjb_mkdir(dir_buff));

    if ((pipe(plot_pipe) == EOF) || (pipe(error_pipe) == EOF))
    {
        set_error("Creation of pipe for a gnuplot process failed.%S");
        return NULL;
    }

    /*
    dbi(plot_pipe[ READ_END ]);
    dbi(plot_pipe[ WRITE_END ]);
    dbi(error_pipe[ READ_END ]);
    dbi(error_pipe[ WRITE_END ]);
    */

    verbose_pso(2, "Verbose level before fork for plotting is %d.\n", 
                kjb_get_verbose_level());

    plot_pid = kjb_fork();

    if (plot_pid < 0)
    {
        set_error("Fork of gnuplot process failed.%S");
        return NULL;
    }

    if (IS_CHILD(plot_pid))
    {
        char system_command[ 256 ];

        if (dup2(plot_pipe[READ_END],fileno(stdin)) == -1)
        {
            set_bug("Can't duplicate read end of plot pipe as stdin.%S");
            kjb_print_error();
            _exit(EXIT_FAILURE);
        }

        if (close(error_pipe[READ_END]) == -1)
        {
            set_bug("Can't close read end of plot error pipe.%S");
            kjb_print_error();
            _exit(EXIT_FAILURE);
        }

        if (X11_is_ok())
        {
            if (fs_colour_plot_flag)
            {
                if (kjb_sprintf(system_command, sizeof(system_command),
                                "gnuplot -geometry %dx%d+%d+%d",
                                x_size, y_size, x_tlc, y_tlc) == ERROR)
                {
                    close(error_pipe[WRITE_END]); 
                    kjb_print_error();
                    _exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (kjb_sprintf(system_command, sizeof(system_command),
                                "gnuplot -geometry %dx%d+%d+%d -mono",
                                x_size, y_size, x_tlc, y_tlc) == ERROR)
                {
                    close(error_pipe[WRITE_END]); 
                    kjb_print_error();
                    _exit(EXIT_FAILURE);
                }
            }
        }
        else
        {
            if (fs_colour_plot_flag)
            {
                if (kjb_sprintf(system_command, sizeof(system_command),
                                "gnuplot") == ERROR)
                {
                    close(error_pipe[WRITE_END]); 
                    kjb_print_error();
                    _exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (kjb_sprintf(system_command, sizeof(system_command),
                                "gnuplot -mono") == ERROR)
                {
                    close(error_pipe[WRITE_END]); 
                    kjb_print_error();
                    _exit(EXIT_FAILURE);
                }
            }
        }

        if (close(plot_pipe[WRITE_END]) == -1)
        {
            set_bug("Can't close write end of plot pipe.%S");
            kjb_print_error();
            /*
             * Give time for the error message to stderr which may go through
             * a pipe to the parent to be read on some systems. 
            */
            sleep(3); 
            _exit(EXIT_FAILURE);
        }

        /*
         * TOOD  
         *
         * Figure out why this is sometimes true and sometimes not! 
         *
         * As soon as we execute the next bit, writing to stderr may be writing
         * to a pipe to the parent. 
        */
        if (dup2(error_pipe[WRITE_END], fileno(stderr)) == -1)
        {
            set_bug("Can't duplicate write end of plot error pipe as stderr.");
            kjb_print_error();
            _exit(EXIT_FAILURE);
        }

        if (kjb_exec(system_command) == ERROR)
        {
            /*
             * We cannot use KJB library routines here because if stderr is a
             * terminal, then we write to the terminal instead. But we must
             * have the file descriptor to be that of stderr in all cases.
            */
            char buff[ LARGE_IO_BUFF_SIZE ]; 

            kjb_get_error(buff, sizeof(buff)); 
            fputs(buff, stderr); 
            fflush(stderr); 
        }
        else
        {
            fputs("Can't happen condition in p_plot.c.\n", stderr);
        }

        /*
         * Give time for the error message to stderr (which now may go through a
         * pipe to the parent) to be read on some systems. 
        */
        sleep(2); 

        _exit(EXIT_FAILURE);

        /*NOTREACHED*/
        return NULL;   /* Keep error checkers happy. */
    }
    else  /* Parent */
    {
        char  plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];
        int   plot_write_des;
        int   plot_read_des;
        Plot* plot_ptr;
        int   check_child_res = NOT_SET;
        char  read_buff[ 50000 ];
        int   read_res; 
        int   count = 0; 
        int   result = NO_ERROR; 


        nap(200); 

        if (close(plot_pipe[ READ_END ]) == -1)
        {
            set_bug("Can't close read end of plot pipe.%S");
            return NULL;
        }

        if (close(error_pipe[ WRITE_END ]) == -1)
        {
            set_bug("Can't close write end of plot error pipe.%S");
            return NULL;
        }

        plot_write_des = plot_pipe[ WRITE_END ];
        plot_read_des  = error_pipe[ READ_END ];

        set_no_blocking(plot_read_des);

        /*
        // Next swallow any messages for us, especially ones that say that the
        // process has died.
        */

        /*CONSTCOND*/
        while (((count < 5) && (result == NO_ERROR)) || (count < 20))
        {
            /*
            safe_pipe_write(plot_write_des, "\n", 1);
            */

            nap(100);

            read_buff[ 0 ] = '\0';
            ERN(read_res = BUFF_DGET_LINE(plot_read_des, read_buff));

            if (read_res > 0)
            {
                if (    (STRCMP_EQ(read_buff, "Broken Pipe"))
                     || (HEAD_CMP_EQ(read_buff, "XIO:"))
                   )
                {
                    set_error("Failure writing to plot pipe.");
                    add_error("It looks like that process is dead. ");
                    cat_error("Killing it to make sure.");
                    terminate_child_process(plot_pid);
                    return NULL;
                }
                else
                {
                    if (result != ERROR)
                    {
                        result = ERROR;
                        set_error("Unexpected message from plot process.");
                        add_error("Assuming error (message follows)."); 
                    }

                    add_error(read_buff);
                }
            }
            else if ((read_res == 0) && (result == ERROR))
            {
                break; 
            }
            else if ((result != ERROR) && ((check_child_res = check_child(plot_pid)) != NO_ERROR))
            {
                set_error("Gnuplot process does not seem to be starting.");
                return NULL;
            }

            count++;
        }

        if (result == ERROR) return NULL; 

        NRN(plot_ptr = TYPE_MALLOC(Plot));
        plot_ptr->num_temp_files = 0;

        plot_ptr->plot_write_des = plot_write_des;
        plot_ptr->plot_read_des = plot_read_des;
        plot_ptr->plot_id = plot_num + 1;
        plot_ptr->plot_pid = plot_pid;
        plot_ptr->create_pid = MY_PID;
        plot_ptr->attached_data_ptr = NULL;
        plot_ptr->attached_data_disposal_fn = NULL;
        plot_ptr->x_min = 0.0;
        plot_ptr->x_max = 0.0;
        plot_ptr->y_min = 0.0;
        plot_ptr->y_max = 0.0;
        plot_ptr->z_min = 0.0;
        plot_ptr->z_max = 0.0;
        plot_ptr->dim = dim;

        BUFF_CPY(plot_ptr->plot_dir_path, dir_buff); 

        ERN(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                        "cd \"%s\"\n", dir_buff));

        ERN(pp_plot_write(plot_ptr, plot_command_buff));

        ERN(pp_plot_write(plot_ptr, "set key\n"));

        if ((! X11_is_ok() ) || (! fs_display_plot_flag))
        {
            ERN(pp_plot_write(plot_ptr, "set terminal dumb\n"));
            ERN(pp_plot_write(plot_ptr, "set output \'/dev/null\'\n"));
        }
        else
        {
            ERN(pp_plot_write(plot_ptr, "set terminal x11\n"));
        }

        ERN(pp_plot_write(plot_ptr, "set style data line\n"));
        ERN(pp_plot_write(plot_ptr, "set autoscale\n"));

        return plot_ptr;
    }

#else /* Case NOT UNIX follows. */

    set_error("Plotting is only implemented on UNIX.");
    return NULL;

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int plot_set_attached_data
(
    int   plot_id,
    void* data_ptr,
    void  (*data_disposal_fn) (void* )
)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (    (plot_ptr->attached_data_ptr != NULL)
         && (plot_ptr->attached_data_disposal_fn != NULL)
       )
    {
        (*(plot_ptr->attached_data_disposal_fn))(plot_ptr->attached_data_ptr);
    }

    plot_ptr->attached_data_ptr = data_ptr;
    plot_ptr->attached_data_disposal_fn = data_disposal_fn;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int plot_get_attached_data(int plot_id, void** data_ptr_ptr)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    *data_ptr_ptr = plot_ptr->attached_data_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      get_plot_ptr
 * -----------------------------------------------------------------------------
 */

static Plot* get_plot_ptr(int plot_id)
{
    Plot* plot_ptr = NULL;

    if (plot_id == CURRENT_PLOT)
    {
        if (fs_current_plot < 0)
        {
            set_error("No current plot.");
            return NULL;
        }

        plot_ptr = fs_plot_info_array[ fs_current_plot ];

        if (plot_ptr == NULL)
        {
            set_error("The current plot no longer exists.");
        }

        return plot_ptr;
    }
    else if ((plot_id <= 0) || (plot_id > fs_max_plot_num + 1))
    {
        set_error("%d is an invalid plot number.", plot_id);
        add_error("Currently valid plot numbers are 1 through %d.",
                  1 + fs_max_plot_num);
        add_error("(Not all of these plots necessarily currenly exist.)");
        return NULL;
    }
    else
    {
        plot_ptr = fs_plot_info_array[ plot_id - 1 ];

        if (plot_ptr == NULL)
        {
            set_error("Plot %d no longer exists.", plot_id);
        }

        return plot_ptr;
    }

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              save_plot
 *
 * Saves a plot in postscript or EPS form.
 *
 * This routine is used to save plots in postscript or EPS form. If the filename
 * ends in ".eps", then EPS is used; otherwise PS is used. If the filename is
 * not specified, then "plot.ps" is used.
 *
 * Due to ugliness with gnuplot, the colours are not exactly what you may
 * expect. I have implemented a hack to help things a little, but there still is
 * room for problems. One problem is that under X-windows, the colours are
 * cycled past 7, but with PS, they are cycled past 8. The colours do not match,
 * and of course the best colour selection for paper is not that same as it is
 * for the screen. Nonetheless, some of the programs which use this routine rely
 * on the fact that the first three colours are red, green, and blue, and hence
 * the hack forces this.
 *
 * An obvious extension to this routine is to be able to specify the colours,
 * but I have not done this yet.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

/*
// Proposed change: Add width and height parms.
//
// To specify a different size, we must change the bounding box, the translation
// and scale parameters in the EPS file.
*/
int save_plot(int plot_id, const char* file_name)
{

    /*
    // This specifies the translation of the nine colours used for postscript to
    // the desired colours.
    */
    static const struct
    {
        const char* in;
        const char* out;
    }
    substitute_strings[] = {
                                { "0 1 0",             "1 0 0"    },
                                { "0 0 1",             "0 1 0"    },
                                { "1 0 0",             "0 0 1"    },
                                { "1 0 1",             "1 .3 0"   },
                                { "0 1 1",             "1 0 1"    },
                                { "1 1 0",             "0 1 1"    },
                                { "0 0 0",             ".2 .2 .3" },
                                { "1 0\\.3 0",         ".8 1 0"   },
                                { "0\\.5 0\\.5 0\\.5", "1 .2 .3"  }    };
    const int num_substitute_strings = sizeof(substitute_strings)/
                                                sizeof(substitute_strings[ 0 ]);
    char  temp_file_name_path[ MAX_FILE_NAME_SIZE ];
    char  file_name_path[ MAX_FILE_NAME_SIZE ];
    char  fixed_file_name_path[ MAX_FILE_NAME_SIZE ];
    char  fixed_file_name[ MAX_FILE_NAME_SIZE ];
    char  root_file_name[ MAX_FILE_NAME_SIZE ];
    char  suffix[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    Plot* plot_ptr;
    int   use_eps = FALSE;
    char  fix_command[ 5000 ];
    char  fix_substitute[ 500 ];
    int   i;
    FILE* fp;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        file_name = "plot.ps";
    }

    if (strlen(file_name) > 4)
    {
        const char* file_name_pos;

        file_name_pos = file_name + strlen(file_name) - 4;

        if (STRCMP_EQ(file_name_pos, ".eps"))
        {
            use_eps = TRUE;
        }
    }

    if (FIND_CHAR_YES(file_name, '.'))
    {
        BUFF_CPY(root_file_name, file_name);
        BUFF_GEN_GET_LAST_TOKEN(root_file_name, suffix, ".");
        root_file_name[ strlen(root_file_name) - 1 ] = '\0';

        BUFF_CPY(fixed_file_name, root_file_name);
        BUFF_CAT(fixed_file_name, "-fixed.");
        BUFF_CAT(fixed_file_name, suffix);
    }
    else
    {
        BUFF_CPY(fixed_file_name, file_name);
        BUFF_CAT(fixed_file_name, "-fixed");
    }

    NRE(fp = kjb_fopen(file_name, "w"));
    ERE(kjb_fclose(fp));

    NRE(fp = kjb_fopen(fixed_file_name, "w"));
    ERE(kjb_fclose(fp));

    BUFF_REAL_PATH(file_name, file_name_path);
    BUFF_REAL_PATH(fixed_file_name, fixed_file_name_path);

    if (use_eps)
    {
        pso("Using EPS\n");

        ERE(pp_plot_write(plot_ptr,
                       "set terminal postscript eps color \"Times-Roman\"\n"));
    }
    else
    {
        ERE(pp_plot_write(plot_ptr,
                       "set terminal postscript color \"Times-Roman\"\n"));
     }

    ERE( BUFF_GET_TEMP_FILE_NAME( temp_file_name_path ) );

    /*
    pso("Plot output file (must be full path): %s\n", file_name_path);
    pso("Plot fixed output file: %s\n", fixed_file_name_path);
    */

    BUFF_CPY(plot_command_buff, "set output \"");
    BUFF_CAT(plot_command_buff, temp_file_name_path);
    BUFF_CAT(plot_command_buff, "\"\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));
    ERE(pp_plot_write(plot_ptr, "replot\n"));

    ERE(wait_for_plot(plot_ptr));

    BUFF_CPY(fix_command, "/bin/cp ");
    BUFF_CAT(fix_command, temp_file_name_path);
    BUFF_CAT(fix_command, " ");
    BUFF_CAT(fix_command, file_name_path);

    ERE(kjb_system(fix_command));

    fix_command[ 0 ] = '\0';

    for (i=0; i<num_substitute_strings; i++)
    {
        if (i != 0) BUFF_CAT(fix_command, " | ");
        BUFF_CAT(fix_command, "sed ");

        ERE(kjb_sprintf(fix_substitute, sizeof(fix_substitute),
                        " 's/LT%d\\(.*\\)%s DL/LT%d\\1%s DL/' ",
                        i, substitute_strings[ i ].in,
                        i, substitute_strings[ i ].out));

        BUFF_CAT(fix_command, fix_substitute);

        if (i == 0) BUFF_CAT(fix_command, file_name_path);
    }

    BUFF_CAT(fix_command, " > ");
    BUFF_CAT(fix_command, fixed_file_name_path);

    verbose_pso(200, "\n%s\n\n", fix_command);

    ERE(kjb_system(fix_command));

    EPE(kjb_unlink(temp_file_name_path));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              save_plot_as_pbm
 *
 * Saves a plot in PBM form
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int save_plot_as_pbm(int plot_id, const char* file_name)
{
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    verbose_pso(1, "Purging plot buffer before saving as PBM.\n");
    ERE(wait_for_plot(plot_ptr)); 

    ERE(pp_plot_write(plot_ptr, "set terminal pbm small color\n"));
    BUFF_CPY(plot_command_buff, "set output \"");
    BUFF_CAT(plot_command_buff, file_name);
    BUFF_CAT(plot_command_buff, "\"\n");
    ERE(pp_plot_write(plot_ptr, plot_command_buff));
    ERE(pp_plot_write(plot_ptr, "replot\n"));

    verbose_pso(1, "Purging save to PDB plot buffer commands.\n");
    ERE(wait_for_plot(plot_ptr)); 

    if ((! X11_is_ok()) || (! is_interactive()) || ( ! fs_display_plot_flag))
    {
        /*
        // This seems to cause problems, perhaps due to a bug in gnuplot?
        // We can do without it for most situations.
        //
        ERE(pp_plot_write(plot_ptr, "set terminal dumb\n"));
        ERE(pp_plot_write(plot_ptr, "set output \'/dev/null\'\n"));
        */
    }
    else
    {
        ERE(pp_plot_write(plot_ptr, "set output\n"));
        /* HACK. It is not clear why this is helps. */
        verbose_pso(1, "Resetting output.\n");
        ERE(wait_for_plot(plot_ptr)); 

        ERE(pp_plot_write(plot_ptr, "set terminal x11\n"));

        /* HACK. It is not clear why this is helps. */
        verbose_pso(1, "Resetting terminal to x11.\n");
        ERE(wait_for_plot(plot_ptr)); 

    }

    ERE(pp_plot_write(plot_ptr, "replot\n"));
    verbose_pso(1, "Replotting on screen.\n");
    ERE(wait_for_plot(plot_ptr)); 
    verbose_pso(1, "Plot has been replotted on screen.\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              save_plot_dir
 *
 * Copies the curent plot directory to a specified directory
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int save_plot_dir(int plot_id, const char* dir_name)
{
    Plot* plot_ptr;
    char command_buff[ 2000 ];


    ERE(kjb_sprintf(command_buff, sizeof(command_buff), 
                    "/bin/rm -r -f %s", dir_name));

    ERE(kjb_system(command_buff));

    NRE(plot_ptr = get_plot_ptr(plot_id));

    ERE(kjb_sprintf(command_buff, sizeof(command_buff), 
                    "/bin/cp -r %s %s",
                    plot_ptr->plot_dir_path,
                    dir_name));

    ERE(kjb_system(command_buff));

    verbose_pso(1, "Information for plot %d has been copied to %s.\n",
                plot_id, dir_name);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            send_command_to_plot
 *
 * Sends a command to a plot
 *
 * This routine is for all else fails. Ideally one would modify the library code
 * to provide the needed facility. However, if hacking is required, a command to
 * the gnuplot(1) process doing the plotting can be sent via this command. The
 * obvious problem is that the command will be wrong or irrelavent if gnuplot
 * was no longer the plotting system, which will likely be under user control at
 * some point.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int send_command_to_plot(int plot_id, const char* command)
{
    Plot* plot_ptr;
    char  plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (command == NULL)
    {
        set_bug("Null command argument passed to send_command_to_current_plot");
        return ERROR;
    }

    BUFF_CPY(plot_command_buff, command);
    BUFF_CAT(plot_command_buff, "\n");
    ERE(pp_plot_write(plot_ptr, plot_command_buff));
    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_set_title
 *
 * Sets the plot title
 *
 * This routine is used to give a plot a title. In addition to the plot handle,
 * and the title string, coordinates for adjusting the title position, relative
 * to where gnuplot(1) was going to put it. Thus 0,0 gives the default.
 * The units are character size.
 *
 * Note:
 *     This is  too gnuplot() centric and will likely change at some point.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_set_title
(
    int         plot_id,
    const char* title,
    int         x_offset,
    int         y_offset
)
{
    char plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];
    char offset_string[ 100 ];

    if (x_offset == NOT_SET) 
    {
        x_offset = 0; 
    }

    if (y_offset == NOT_SET) 
    {
        y_offset = 0; 
    }

    BUFF_CPY(plot_command_buff, "set title \"");

    if (title != NULL)
    {
        char temp_buff [ 256 ];


        BUFF_CPY(temp_buff, title);
        char_for_char_translate(temp_buff, '"', '\'');

        BUFF_CAT(plot_command_buff, temp_buff);
    }
    else
    {
        BUFF_CAT(plot_command_buff, "Untitled");
    }

    /* The "offset" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(offset_string, sizeof(offset_string), "%d,%d",
                    x_offset, y_offset));
#else 
    ERE(kjb_sprintf(offset_string, sizeof(offset_string), "offset %d,%d",
                    x_offset, y_offset));
#endif 


    BUFF_CAT(plot_command_buff, "\"  ");
    BUFF_CAT(plot_command_buff, offset_string);
    BUFF_CAT(plot_command_buff, " \n");

    ERE(plot_write(plot_id, plot_command_buff));
    ERE(plot_update(plot_id));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_set_x_legend
 *
 * This routine labels the Y axis.
 *
 * This routine labels the Y axis. It needs a plot handle and a string for the
 * lable.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_set_x_legend(int plot_id, const char* legend)
{
    char plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];
    char temp_buff[ 1000 ];


    BUFF_CPY(plot_command_buff, "set xlabel \"");

    if (legend != NULL)
    {
        BUFF_CPY(temp_buff, legend);
        char_for_char_translate(temp_buff, '"', '\'');

        BUFF_CAT(plot_command_buff, temp_buff);
    }
    else
    {
        BUFF_CAT(plot_command_buff, "");
    }

    BUFF_CAT(plot_command_buff, "\"");

    /*
     * Position is tricky because it gets jerked around with the range. Try the
     * default for a while.
    ERE(kjb_sprints(temp_buff, sizeof(temp_buff),
                    "% 0,%.3f \n",
                    expression_based on range.

    BUFF_CAT(plot_command_buff, " 0,-1 \n");
    */
    BUFF_CAT(plot_command_buff, " \n");

    ERE(plot_write(plot_id, plot_command_buff));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_set_y_legend
 *
 * This routine labels the Y axis.
 *
 * This routine labels the Y axis. It needs a plot handle and a string for the
 * lable.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_set_y_legend(int plot_id, const char* legend)
{
    char  plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];


    BUFF_CPY(plot_command_buff, "set ylabel \"");

    if (legend != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, legend);
        char_for_char_translate(temp_buff, '"', '\'');

        BUFF_CAT(plot_command_buff, temp_buff);
    }
    else
    {
        BUFF_CAT(plot_command_buff, "");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, " -1,0 \n");

    ERE(plot_write(plot_id, plot_command_buff));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_add_label
 *
 * Adds a lable to a plot.
 *
 * This routine adds a lable to a plot. It nees a plot handle, a character
 * string for the label, and a X and Y coordinate for the position of the
 * string. The coordinates are in the plot coordinate systing.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_add_label(int plot_id, const char* label, double x, double y)
{
    char plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char coord_buff[ 256 ];
    char temp_buff [ 256 ];


    ERE(kjb_sprintf(coord_buff, sizeof(coord_buff), "%f,%f", x, y));
    BUFF_CPY(plot_command_buff, "set label \"");

    BUFF_CPY(temp_buff, label);
    char_for_char_translate(temp_buff, '"', '\'');

    BUFF_CAT(plot_command_buff, temp_buff);

    BUFF_CAT(plot_command_buff, "\" at ");
    BUFF_CAT(plot_command_buff, coord_buff);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(plot_write(plot_id, plot_command_buff));
    ERE(plot_update(plot_id));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_add_label_2
 *
 * Adds a lable to a plot.
 *
 * This routine adds a lable to a plot. It is similar to plot_add_label(), but
 * the label coordinates are now relative to the plot range. Thus, for example
 * X=0.5 and Y=0.5 is the middle of the plot.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_add_label_2(int plot_id, const char* label, double x, double y)
{
    char plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char coord_buff[ 256 ];
    char temp_buff [ 256 ];
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    ERE(kjb_sprintf(coord_buff, sizeof(coord_buff), "%f,%f",
                    plot_ptr->x_min + (plot_ptr->x_max - plot_ptr->x_min) * x,
                    plot_ptr->y_min + (plot_ptr->y_max - plot_ptr->y_min) * y));

    BUFF_CPY(plot_command_buff, "set label \"");

    BUFF_CPY(temp_buff, label);
    char_for_char_translate(temp_buff, '"', '\'');

    BUFF_CAT(plot_command_buff, temp_buff);

    BUFF_CAT(plot_command_buff, "\" at ");
    BUFF_CAT(plot_command_buff, coord_buff);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(plot_write(plot_id, plot_command_buff));
    ERE(plot_update(plot_id));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_set_range
 *
 * Sets the range of the plot
 *
 * This routine adjusts the range of the plot. THe plotting system normally
 * automatically sets the range, but this is not nessarily how you want it. Any
 * of the four arguments can be DBL_NOT_SET (-99.0) which means that it should
 * be ignored.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_set_range
(
    int    plot_id,
    double x_min,
    double x_max,
    double y_min,
    double y_max
)
{
    char plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char num_buff[ 256 ];
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if ((SET_PLOT_RANGE(x_min)) || (SET_PLOT_RANGE(x_max)))
    {
        BUFF_CPY(plot_command_buff, "set xrange [");

        if (SET_PLOT_RANGE(x_min))
        {
            if (x_min < plot_ptr->x_min)
            {
                plot_ptr->x_min = x_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(x_max))
        {
            if (x_max > plot_ptr->x_max)
            {
                plot_ptr->x_max = x_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] \n");

        ERE(pp_plot_write(plot_ptr, plot_command_buff));
    }

    if ((SET_PLOT_RANGE(y_min)) || (SET_PLOT_RANGE(y_max)))
    {
        BUFF_CPY(plot_command_buff, "set yrange [");

        if (SET_PLOT_RANGE(y_min))
        {
            if (y_min < plot_ptr->y_min)
            {
                plot_ptr->y_min = y_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(y_max))
        {
            if (y_max > plot_ptr->y_max)
            {
                plot_ptr->y_max = y_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] \n");

        ERE(pp_plot_write(plot_ptr, plot_command_buff));
    }

    if (plot_ptr->num_temp_files >  1)
    {
        ERE(pp_plot_update(plot_ptr));
    }


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_get_range
 *
 * Gets the range of the plot
 *
 * If you need to query the system for the plot range, you can use this routine.
 * Note that this is the kjb library interfaces understanding of the range. The
 * exact range plotted can be different.
 *
 * Any of the 4 range parameter pointers can be NULL if you are not interesed in
 * that value.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_get_range
(
    int     plot_id,
    double* x_min_ptr,
    double* x_max_ptr,
    double* y_min_ptr,
    double* y_max_ptr
)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (x_min_ptr != NULL)
    {
        *x_min_ptr = plot_ptr->x_min;
    }

    if (x_max_ptr != NULL)
    {
        *x_max_ptr = plot_ptr->x_max;
    }

    if (y_min_ptr != NULL)
    {
        *y_min_ptr = plot_ptr->y_min;
    }

    if (y_max_ptr != NULL)
    {
        *y_max_ptr = plot_ptr->y_max;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_set_range3
 *
 * Sets the range of 3D plots.
 *
 * This routine is the 3D version of plot_set_range(3).
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_set_range3
(
    int    plot_id,
    double x_min,
    double x_max,
    double y_min,
    double y_max,
    double z_min,
    double z_max
)
{
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char  num_buff[ 256 ];
    Plot* plot_ptr;


    UNTESTED_CODE();

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim < 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((SET_PLOT_RANGE(x_min)) || (SET_PLOT_RANGE(x_max)))
    {
        BUFF_CPY(plot_command_buff, "set xrange [");

        if (SET_PLOT_RANGE(x_min))
        {
            if (plot_ptr->x_min < x_min)
            {
                plot_ptr->x_min = x_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(x_max))
        {
            if (plot_ptr->x_max > x_max)
            {
                plot_ptr->x_max = x_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] \n");

        ERE(pp_plot_write(plot_ptr, plot_command_buff));
    }

    if ((SET_PLOT_RANGE(y_min)) || (SET_PLOT_RANGE(y_max)))
    {
        BUFF_CPY(plot_command_buff, "set yrange [");

        if (SET_PLOT_RANGE(y_min))
        {
            if (plot_ptr->y_min < y_min)
            {
                plot_ptr->y_min = y_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(y_max))
        {
            if (plot_ptr->y_max > y_max)
            {
                plot_ptr->y_max = y_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] \n");
    }

    if ((SET_PLOT_RANGE(z_min)) || (SET_PLOT_RANGE(z_max)))
    {
        BUFF_CPY(plot_command_buff, "set yrange [");

        if (SET_PLOT_RANGE(z_min))
        {
            if (plot_ptr->z_min < z_min)
            {
                plot_ptr->z_min = z_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",z_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(z_max))
        {
            if (plot_ptr->z_max > z_max)
            {
                plot_ptr->z_max = z_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",z_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] \n");

        ERE(pp_plot_write(plot_ptr, plot_command_buff));
    }

    if (plot_ptr->num_temp_files >  1)
    {
        ERE(pp_plot_update(plot_ptr));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_function_string
 *
 * Plots a function
 *
 * This routine plots the function in the string. It understands common
 * mathematical constructs. For details see the gnuplot() documentation.
 *
 * Note:
 *     These strings are necessarily gnuplot(1) specific.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_function_string
(
    int         plot_id,
#ifdef DEF_OUT
    double      x_min,
    double      x_max,
    double      y_min,
    double      y_max,
#endif
    const char* function_string,
    int         width, 
    const char* name
)
{
    char plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char temp_file_name[ MAX_FILE_NAME_SIZE ]; 
    FILE* temp_fp; 
    char temp_buff [ 1000 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /*
     * HACK 
     *
     * We don't necessarily need a temp file for this, but we are using them to
     * keep track of whether this is a new plot or a replot.
    */
    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));
    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));
    ERE(kjb_fprintf(temp_fp,"%s\n", function_string));
    ERE(kjb_fclose(temp_fp));

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

#ifdef DEF_OUT  /* No different than setting the range in a separate call? */
    if ((SET_PLOT_RANGE(x_min)) || (SET_PLOT_RANGE(x_max)))
    {
        BUFF_CAT(plot_command_buff, " [x=");

        if (SET_PLOT_RANGE(x_min))
        {
            if (plot_ptr->x_min < x_min)
            {
                plot_ptr->x_min = x_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(x_max))
        {
            if (plot_ptr->x_max > x_max)
            {
                plot_ptr->x_max = x_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",x_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] ");
    }

    if ((SET_PLOT_RANGE(y_min)) || (SET_PLOT_RANGE(y_max)))
    {
        BUFF_CAT(plot_command_buff, " [y=");

        if (SET_PLOT_RANGE(y_min))
        {
            if (plot_ptr->y_min < y_min)
            {
                plot_ptr->y_min = y_min;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_min));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, ":");

        if (SET_PLOT_RANGE(y_max))
        {
            if (plot_ptr->y_max > y_max)
            {
                plot_ptr->y_max = y_max;
            }

            ERE(kjb_sprintf(num_buff, sizeof(num_buff), "%f",y_max));
            BUFF_CAT(plot_command_buff, num_buff);
        }

        BUFF_CAT(plot_command_buff, "] ");
    }
#endif

    BUFF_CAT(plot_command_buff, function_string);

    if (width > 1)
    {
        ERE(kjb_sprintf(temp_buff, sizeof(temp_buff), 
                        " with lines linewidth %d ", width)); 
    }

    BUFF_CAT(plot_command_buff, temp_buff);

    if (name == NULL)
    {
        name = "";
    }

    BUFF_CPY(temp_buff, name);
    char_for_char_translate(temp_buff, '"', '\'');
    BUFF_CAT(plot_command_buff, " t \"");
    BUFF_CAT(plot_command_buff, temp_buff);
    BUFF_CAT(plot_command_buff, "\" ");

    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));
    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      plot_selected_mulitple_histograms
 *
 * Plots selected multiple histograms
 *
 * This routine allows multiple histograms to be plotted together. The source
 * data for the histograms are supplied in the vector of vectors in the second
 * argument. If the fourth parameter (sigma) is positive, it is used to smooth
 * the histograms. If names_ptr is not NULL, than it is used to label the
 * histograms. If enable_vp is not NULL, then enables/disables the
 * corresponding histogram. Finally, if bin_size_ptr is not NULL, then it
 * retrieves the bin size, as a function of the number of bins requested, and
 * all the data used.
 *
 * Note: 
 *     If enable_vp is NULL, this routine is equivalent to
 *     plot_multiple_histograms(). 
 *
 * Note:
 *     When multiple histograms are being plotted, we make it so that the bins
 *     for the various groups are the same. This means that the bins are a
 *     function of the groups, which means that the histograms are also a
 *     function of the group. 
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_selected_multiple_histograms
(
    int                  plot_id,
    const Vector_vector* vvp,
    int                  num_bins,
    double               sigma,
    const Word_list*     names_ptr,
    const Int_vector*    enable_vp, 
    double*              bin_size_ptr
)
{


    if (enable_vp == NULL)
    {
        return plot_multiple_histograms(plot_id, vvp, num_bins, sigma, names_ptr,
                                        bin_size_ptr); 
    }
    else 
    {
        Vector_vector* selected_vvp       = NULL;
        Word_list*     selected_names_ptr = NULL;
        int            result             = NO_ERROR;

        result = select_from_vector_vector(&selected_vvp, vvp, enable_vp);

        if (result != ERROR)
        {
            result = select_from_word_list(&selected_names_ptr, 
                                           names_ptr, enable_vp);
        }
        
        if (result != ERROR)
        {
            result = plot_multiple_histograms(plot_id, selected_vvp, 
                                              num_bins, sigma,
                                              selected_names_ptr, 
                                              bin_size_ptr);
        }

        free_vector_vector(selected_vvp);
        free_word_list(selected_names_ptr); 

        return result; 
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         plot_mulitple_histograms
 *
 * Plots multiple histograms on the same scale
 *
 * This routine allows multiple histograms to be plotted together. The source
 * data for the histograms are supplied in the vector of vectors in the second
 * argument. If the fourth parameter (sigma) is positive, it is used to smooth
 * the histograms. If names_ptr is not NULL, than it is used to label the
 * histograms. Finally, if bin_size_ptr is not NULL, then it retrieves the bin
 * size, as a function of the number of bins requested, and all the data used.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Note:
 *     When multiple histograms are being plotted, we make it so that the bins
 *     for the various groups are the same. This means that the bins are a
 *     function of the groups, which means that the histograms are also a
 *     function of the group. 
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_multiple_histograms
(
    int                  plot_id,
    const Vector_vector* vvp,
    int                  num_bins,
    double               sigma,
    const Word_list*     names_ptr,
    double*              bin_size_ptr
)
{
    double  min, max;
    double  step;
    double  range;
    double  d_num_bins = num_bins;
    Vector_vector* hist_vvp    = NULL;
    Vector_vector* x_vvp = NULL;
    int     count;
    int     result = NO_ERROR;
    double  bar_width;
    int     num_histograms = (vvp == NULL) ? NOT_SET : vvp->length;
    int     i, j;


    if (num_histograms < 1) return NO_ERROR;

    ERE(get_target_vector_vector(&hist_vvp, num_histograms));

    min = min_vector_element(vvp->elements[ 0 ]);
    max = max_vector_element(vvp->elements[ 0 ]);

    for (count = 1; count < vvp->length; count++)
    {
        min = MIN_OF(min, min_vector_element(vvp->elements[ count ]));
        max = MAX_OF(max, max_vector_element(vvp->elements[ count ]));
    }

    range = max - min;
    step  = range / d_num_bins;

    if (step < 1.0e2 * DBL_MIN)
    {
        set_error("Bin size is too small for histogram plot.");
        return ERROR;
    }

    bar_width = step / (1.0 + num_histograms);

    for (i = 0; i < num_histograms; i++)
    {
        result = get_1D_hist(&(hist_vvp->elements[ i ]),
                             vvp->elements[ i ],
                             min, max, num_bins, sigma);
        if (result == ERROR) { NOTE_ERROR(); break; }
    }

    if (result != ERROR)
    {
        result = get_target_vector_vector(&x_vvp, num_histograms);
    }

    if (result != ERROR)
    {
        for (i = 0; i < num_histograms; i++)
        {
            const Vector* x_vp;

            result = get_target_vector(&(x_vvp->elements[ i ]), num_bins);
            if (result == ERROR) { NOTE_ERROR(); break; }

            x_vp = x_vvp->elements[ i ];

            for (j = 0; j < num_bins; j++)
            {
               x_vp->elements[ j ] = min + bar_width / 2 + i * bar_width + j * step;
            }
        }
    }

    if (result != ERROR)
    {
        result = plot_multiple_bars_2(plot_id, x_vvp, hist_vvp, 0.8 * bar_width, names_ptr);
    }

    free_vector_vector(hist_vvp);
    free_vector_vector(x_vvp);

    if (bin_size_ptr != NULL)
    {
        *bin_size_ptr = step;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         plot_mulitple_bars
 *
 * Plots multiple bar graphs on the same scale
 *
 * This routine allows multiple bar graphs to be plotted together. The source
 * data for the bars are supplied in the vector of vectors in the second
 * argument is the x coordinate of the first bar, and bars are spaced "step"
 * units apart.  If names_ptr is not NULL, than it is used to label the
 * bars.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_multiple_bars
(
    int                  plot_id,
    const Vector_vector* y_vvp,
    double               offset,
    double               step,
    const Word_list*     names_ptr
)
{
    Vector_vector* x_vvp = NULL;
    int     result = NO_ERROR;
    double  bar_width;
    int     num_bars = (y_vvp == NULL) ? NOT_SET : y_vvp->length;
    int     i, j;
    int     num_bins = NOT_SET;


    if (num_bars < 1) return NO_ERROR;

    if (step < 1.0e2 * DBL_MIN)
    {
        set_error("Step size is too small for bar plot.");
        return ERROR;
    }

    bar_width = step / (1.0 + num_bars);

    if (result != ERROR)
    {
        result = get_target_vector_vector(&x_vvp, num_bars);
    }

    if (result != ERROR)
    {
        for (i = 0; i < num_bars; i++)
        {
            const Vector* x_vp;

            if (i == 0)
            {
                num_bins = y_vvp->elements[ 0 ]->length;
            }
            else
            {
                if (y_vvp->elements[ i ]->length != num_bins)
                {
                    SET_ARGUMENT_BUG();
                    result = ERROR;
                    NOTE_ERROR();
                    break;
                }
            }

            result = get_target_vector(&(x_vvp->elements[ i ]), num_bins);
            if (result == ERROR) { NOTE_ERROR(); break; }

            x_vp = x_vvp->elements[ i ];

            for (j = 0; j < num_bins; j++)
            {
               x_vp->elements[ j ] = offset + bar_width / 2 + i * bar_width + j * step;
            }
        }
    }

    if (result != ERROR)
    {
        result = plot_multiple_bars_2(plot_id, x_vvp, y_vvp, 0.8 * bar_width, names_ptr);
    }

    free_vector_vector(x_vvp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         plot_mulitple_bars_2
 *
 * Plots multiple bar graphs on the same scale
 *
 * This routine is an alternative method to specfiy multiple bar graphs to be
 * plotted together. The X ordinates for the bars are specified directly in the
 * second argument, x_vvp, with the Y ordinates (bar height) provided as the
 * third argument (y_vvp). The bar_width is supplied as an argument, instead of
 * being computed automatically.  If names_ptr is not NULL, than it is used to
 * label the bars.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_multiple_bars_2
(
    int                  plot_id,
    const Vector_vector* x_vvp,
    const Vector_vector* y_vvp,
    double               bar_width,
    const Word_list*     names_ptr
)
{
    int     count;
    int     result = NO_ERROR;
    double  max_y = DBL_MOST_NEGATIVE;
    double  dy;
    int     num_bars = (x_vvp == NULL) ? NOT_SET : x_vvp->length;

    if (num_bars < 1) return NO_ERROR;

    if (y_vvp->length != num_bars)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (count = 0; count < num_bars; count++)
    {
        const Vector* y_vp = y_vvp->elements[ count ];

        if (y_vp == NULL) continue;

        max_y = MAX_OF(max_y, max_vector_element(y_vp));
    }

#define MAX_NUM_STRIPES  50

    dy = max_y / MAX_NUM_STRIPES;

    for (count = 0; count < num_bars; count++)
    {
        const Vector* y_vp = y_vvp->elements[ count ];
        const Vector* x_vp = x_vvp->elements[ count ];
        const char* name = (names_ptr == NULL) ? NULL : names_ptr->words[ count ];

        if (result == ERROR) { NOTE_ERROR(); break; }

        if ((x_vp == NULL) || (y_vp == NULL)) continue;

        result = plot_bars_2(plot_id, x_vp, y_vp, bar_width,
                             IS_ODD(count) ? dy : DBL_NOT_SET,
                             name);
        if (result == ERROR) { NOTE_ERROR(); break; }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_histogram
 *
 * Plots a histogram
 *
 * This plots a histogram for the data in the second parameter. The number of
 * bins is supplied as the third parameter.  If the fourth parameter (sigma) is
 * positive, it is used to smooth the histograms. If name is not NULL, than it
 * is used to label the histogram.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_histogram
(
    int           plot_id,
    const Vector* vp,
    int           num_bins,
    double        sigma,
    const char*   name
)
{
    double  min, max;
    double  step;
    double  range;
    double  d_num_bins = num_bins;
    Vector* hist_vp    = NULL;
    int     result;


    ERE(get_min_vector_element(vp, &min));
    ERE(get_max_vector_element(vp, &max));

    range = max - min;
    step  = range / d_num_bins;

    if (step < 1.0e2 * DBL_MIN)
    {
        set_error("Bin size is too small for histogram plot.");
        return ERROR;
    }

    ERE(get_1D_hist(&hist_vp, vp, min, max, num_bins, sigma));

    result = plot_bars(plot_id, hist_vp, min, step, name);

    free_vector(hist_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_bars
 *
 * Plots bars
 *
 * This routine plots the bars with hieghts given in y_vp.  The thrid argument
 * (ofset) is the x coordinate of the first bar, and bars are spaced "step"
 * units apart.  If label_str is not NULL, than it is used to label the bar.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_bars
(
    int           plot_id,
    const Vector* y_vp,
    double        offset,
    double        step,
    const char*   label_str
)
{
    int len = y_vp->length;
    int i;
    Vector* x_vp = NULL;
    int result = NO_ERROR;


    ERE(get_target_vector(&x_vp, len));

    for (i = 0; i < len; i++)
    {
        x_vp->elements[ i ] = offset + i * step;
    }

    result = plot_bars_2(plot_id, x_vp, y_vp,
                         DBL_NOT_SET, DBL_NOT_SET, label_str);
    free_vector(x_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_bars_2
 *
 * Plots bars
 *
 * This routine is an alternative method to specfiy bars.  The X ordinates for
 * the bars are specified directly in the second argument, x_vp, with the Y
 * ordinates (bar heights) are provided as the third argument (y_vp). The
 * bar_width is supplied as an argument, instead of being computed
 * automatically. If the argument stripe_width is positive, the bars are striped
 * with lines this distance apart. If label_str is not NULL, than it is used to
 * label the bars.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_bars_2
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    double        bar_width,
    double        stripe_width,
    const char*   label_str
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    char  line_type_str[ 256 ];
    Plot* plot_ptr;
    int len = x_vp->length;


    if (y_vp->length != len)
    {
        set_bug("X and Y vectors have different lengths in plot_bars_2.");
        return ERROR;
    }

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i < len; i++)
    {
        if (bar_width > 0.0)
        {
            ERE(kjb_fprintf(temp_fp,"%e %e %e\n",
                            (x_vp->elements)[i],
                            (y_vp->elements)[i],
                            bar_width));
        }
        else
        {
            ERE(kjb_fprintf(temp_fp,"%e %e\n",
                            (x_vp->elements)[i],
                            (y_vp->elements)[i]));

        }
    }

    if (stripe_width > 0)
    {
        ERE(kjb_fprintf(temp_fp,"#\n#Extra bars for horizontal stripes.\n#\n"));

        for (i=0; i<x_vp->length; i++)
        {
            double y = stripe_width;

            while (y < (y_vp->elements)[i])
            {
                if (bar_width > 0.0)
                {
                    ERE(kjb_fprintf(temp_fp,"%e %e %e\n",
                                    (x_vp->elements)[i],
                                    y, bar_width));
                }
                else
                {
                    ERE(kjb_fprintf(temp_fp,"%e %e\n",
                                    (x_vp->elements)[i], y));
                }

                y += stripe_width;
            }
        }
    }


    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * min_vector_element(x_vp);
    temp_x_max = 1.1 * max_vector_element(x_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * min_vector_element(y_vp);
    temp_y_max = 1.1 * max_vector_element(y_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (label_str != NULL)
    {
        char temp_buff [ 1000 ];

        BUFF_CPY(temp_buff, label_str);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w boxes ");

#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_vector
 *
 * Plots a vector
 *
 * This routine plots a vector assuming that the elements are uniformaly spaced.
 * That spacing is given by an offset (x_offset) that provides the X-ordinate
 * for the first element, and a step size (x_step) that gives the spacing. If
 * name is not NULL, it us used to label the resulting line.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_vector
(
    int           plot_id,
    const Vector* vp,
    double        x_offset,
    double        x_step,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_num_str[ 256 ];
    char  plot_command_buff [ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    char  line_type_str[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e\n",x_offset+i*x_step,
                        (vp->elements)[i]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = x_offset;
    temp_x_max = x_offset + x_step * vp->length;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = 0.9 * temp_x_min;
        x_max = 1.1 * temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if ((name == NULL) || (*name == '\0'))
    {
        ERE(kjb_sprintf(plot_num_str, sizeof(plot_num_str), "%d",
                        (plot_ptr->num_temp_files)));
        BUFF_CAT(plot_command_buff, plot_num_str);
    }
    else
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_point_list
 *
 * Plots a group of points that form the rows of a Nx2 matrix
 *
 * This routine plots the points in the Nx2 matrix "point_mp". If names is not
 * NULL, then it supplies labels for the points.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_point_list(int plot_id, const Matrix* point_mp, char** names)
{
    FILE*   temp_fp;
    char    temp_file_name[ MAX_FILE_NAME_SIZE ];
    char    plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double    x_min;
    double    x_max;
    double    temp_x_min;
    double    temp_x_max;
    char    point_type[ 256 ];
    int     i;
    int     num_points;
    Vector* max_vp = NULL;
    Vector* min_vp = NULL;
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_points = point_mp->num_rows;

    if (num_points == 0) return NO_ERROR;

    /*
    // Memory leak on failure is OK, because we only expect to fail on
    // allocation failure.
    */
    ERE(get_min_matrix_col_elements(&min_vp, point_mp));
    ERE(get_max_matrix_col_elements(&max_vp, point_mp));

    temp_x_min = 0.9 * (min_vp->elements)[ 0 ];
    temp_x_max = 1.1 * (max_vp->elements)[ 0 ];

    free_vector(min_vp);
    free_vector(max_vp);

    if (plot_ptr->num_temp_files == 0)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    for (i=0; i<num_points; i++)
    {
        if (plot_ptr->num_temp_files == 0)
        {
            BUFF_CPY(plot_command_buff, "plot ");
        }
        else
        {
            BUFF_CPY(plot_command_buff, "replot ");
        }

        (plot_ptr->num_temp_files)++;

        ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                    plot_ptr->num_temp_files));

        NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id,
                                          temp_file_name));

        ERE(kjb_fprintf(temp_fp,"%e %e\n",
                        (point_mp->elements)[i][0],
                        (point_mp->elements)[i][1]));

        ERE(kjb_fclose(temp_fp));

        BUFF_CAT(plot_command_buff, "\"");
        BUFF_CAT(plot_command_buff, temp_file_name);
        BUFF_CAT(plot_command_buff, "\" t \"");

        if (names != NULL)
        {
            BUFF_CAT(plot_command_buff, names[i]);
        }

        BUFF_CAT(plot_command_buff, "\" w points ");
        ERE(kjb_sprintf(point_type, sizeof(point_type), "%d\n",
                        plot_ptr->num_temp_files));
        BUFF_CAT(plot_command_buff, point_type);

        ERE(pp_plot_write(plot_ptr, plot_command_buff));
    }

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_vector_point
 *
 * Plots a point
 *
 * This routine plots the point in the vector "point_vp". If name is not NULL,
 * it is used to label the point.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_vector_point
(
    int           plot_id,
    const Vector* point_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double  x;
    double  y;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    char  point_type[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    x = (point_vp->elements)[ 0 ];
    y = (point_vp->elements)[ 1 ];

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    ERE(kjb_fprintf(temp_fp,"%e %e\n",x,y));

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * x;
    temp_x_max = 1.1 * y;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w points ");
    ERE(kjb_sprintf(point_type, sizeof(point_type), "%d",
                    plot_ptr->num_temp_files));
    BUFF_CAT(plot_command_buff, point_type);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_point
 *
 * Plots a point
 *
 * This routine plots the point (x,y). If name is not NULL, it is used to label
 * the point.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_point(int plot_id, double x, double y, const char* name)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    char  point_type[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    ERE(kjb_fprintf(temp_fp,"%e %e\n",x,y));

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * x;
    temp_x_max =1.1 *  y;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff[ 256 ];


        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w points ");
    ERE(kjb_sprintf(point_type, sizeof(point_type), "%d",
                    plot_ptr->num_temp_files));
    BUFF_CAT(plot_command_buff, point_type);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_row_points
 *
 * Plots a group of points that form the rows of a Nx2 matrix
 *
 * This routine plots the points in the Nx2 matrix "point_mp". If name is not
 * NULL, then it supplies label for all the points.
 *
 * Note:
 *     This is very similar to plot_point_list(). The main difference is whether
 *     one wants to lable a lot of points the same, or plot a few points, each
 *     with their own label.
 *
 * Note:
 *     Some name rationalization may occur in the future.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_row_points
(
    int           plot_id,
    const Matrix* mp,
    const char*   name
)
{
    Vector* x_vp = NULL;
    Vector* y_vp = NULL;
    int result;
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (mp->num_cols != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    result = get_matrix_col(&x_vp, mp, 0);

    if (result != ERROR)
    {
        result = get_matrix_col(&y_vp, mp, 1);
    }

    if (result != ERROR)
    {
        result = pp_plot_points(plot_ptr, x_vp, y_vp, name);
    }

    free_vector(x_vp);
    free_vector(y_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_points
 *
 * Plots points
 *
 * This routine is another interface to plot points. The points are given by two
 * vectors, one for the X-ordinates and one for the Y-ordinates. If name is not
 * NULL, it labels the point group.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_points
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
)
{
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_points(plot_ptr, x_vp, y_vp, name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_points
 * -----------------------------------------------------------------------------
 */

static int pp_plot_points
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    char  point_type[ 256 ];


    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (x_vp->length != y_vp->length)
    {
        set_bug("X and Y vectors have different lengths in plot_points.");
        return ERROR;
    }

    if (x_vp->length <= 0)
    {
        return NO_ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i<x_vp->length; i++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e\n",(x_vp->elements)[i],
                        (y_vp->elements)[i]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * min_vector_element(x_vp);
    temp_x_max = 1.1 * max_vector_element(x_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * min_vector_element(y_vp);
    temp_y_max = 1.1 * max_vector_element(y_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set yrange [%f:%f]\n", y_min, y_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w points pt ");
    ERE(kjb_sprintf(point_type, sizeof(point_type), "%d",
                    plot_ptr->num_temp_files));
    BUFF_CAT(plot_command_buff, point_type);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_curve
 *
 * Plots a curve by linking points
 *
 * This routine plots the curve implied by liking the points whose X-ordinates
 * are in the argument "x_vp" and whoese Y-ordinates are in the argument "y_vp"
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_curve
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    char  line_type_str[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (x_vp->length != y_vp->length)
    {
        set_bug("X and Y vectors have different lengths in plot_curve.");
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i<x_vp->length; i++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e\n",(x_vp->elements)[i],
                        (y_vp->elements)[i]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * min_vector_element(x_vp);
    temp_x_max = 1.1 * max_vector_element(x_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * min_vector_element(y_vp);
    temp_y_max = 1.1 * max_vector_element(y_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 1000 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_line
 *
 * Plots a line segment
 *
 * This routine plots the line segment from (x1,y1) to (x2,y2). If name is not
 * NULL, it is used to label the line.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_line
(
    int         plot_id,
    double      x1,
    double      y1,
    double      x2,
    double      y2,
    const char* name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    char  line_type_str[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));
    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));
    ERE(kjb_fprintf(temp_fp,"%e %e\n%e %e\n", x1, y1, x2, y2));
    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * MIN_OF(x1, x2);
    temp_x_max = 1.1 * MAX_OF(x1, x2);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * MIN_OF(y1, y2);
    temp_y_max = 1.1 * MAX_OF(y1, y2);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_multi_segment_curve
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_multi_segment_curve
(
    int                  plot_id,
    const Matrix_vector* segments,
    const char*          name
)
{
    int     num_segments;
    FILE*   temp_fp;
    char    temp_file_name[ MAX_FILE_NAME_SIZE ];
    char    plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double    x;
    double    y;
    double    x_min;
    double    x_max;
    double    temp_x_min;
    double    temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    int     i;
    int     count;
    int     num_rows;
    Matrix* segment_mp;
    char    line_type_str[ 256 ];
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    temp_x_max = temp_x_min = segments->elements[ 0 ]->elements[ 0 ][ 0 ];
    temp_y_max = temp_y_min = segments->elements[ 0 ]->elements[ 0 ][ 1 ];

    num_segments = segments->length;

    for (count=0; count<num_segments; count++)
    {
        segment_mp = segments->elements[ count ];

        num_rows = segment_mp->num_rows;

        for (i=0; i<num_rows; i++)
        {
            x = (segment_mp->elements)[i][ 0 ];
            y = (segment_mp->elements)[i][ 1 ];

            if (x < temp_x_min)
            {
                temp_x_min = x;
            }

            if (x > temp_x_max)
            {
                temp_x_max = x;
            }

            if (y < temp_y_min)
            {
                temp_y_min = y;
            }

            if (y > temp_y_max)
            {
                temp_y_max = y;
            }

            kjb_fprintf(temp_fp,"%e %e\n",x, y);
        }
        kjb_fprintf(temp_fp, "\n");
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min *= 0.9;
    temp_x_max *= 1.1;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    temp_y_min *= 0.9;
    temp_y_max *= 1.1;

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_vector_list_cols
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_vector_list_cols
(
    int             plot_id,
    int             list_len,
    Matrix_vector** matrix_vector_list,
    const char*     name
)
{
    int      count;
    int      i;
    int      j;
    Vector*  x1_vp;
    Vector*  x2_vp;
    Vector*  y1_vp;
    Vector*  y2_vp;
    int      num_segments;
    Matrix** mp_array;
    int      result;
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_segments = 0;

    for (i=0; i<list_len; i++)
    {
        num_segments += matrix_vector_list[ i ]->length;
    }

    NRE(x1_vp = create_vector(num_segments));
    NRE(x2_vp = create_vector(num_segments));
    NRE(y1_vp = create_vector(num_segments));
    NRE(y2_vp = create_vector(num_segments));

    count = 0;

    for (i=0; i<list_len; i++)
    {
        mp_array     = (matrix_vector_list[ i ])->elements;
        num_segments = (matrix_vector_list[ i ])->length;

        for (j=0; j<num_segments; j++)
        {
            (x1_vp->elements)[count] = ((mp_array[ j ])->elements)[0][0];
            (y1_vp->elements)[count] = ((mp_array[ j ])->elements)[0][1];
            (x2_vp->elements)[count] = ((mp_array[ j ])->elements)[1][0];
            (y2_vp->elements)[count] = ((mp_array[ j ])->elements)[1][1];

            count++;
        }
    }

    result = pp_plot_segments(plot_ptr, x1_vp, y1_vp, x2_vp, y2_vp, name);

    free_vector(x1_vp);
    free_vector(x2_vp);
    free_vector(y1_vp);
    free_vector(y2_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_vector_cols
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_vector_cols
(
    int                  plot_id,
    const Matrix_vector* matrix_vector,
    const char*          name
)
{
    int      num_segments;
    int      i;
    Matrix** mp_array;
    Vector*  x1_vp;
    Vector*  x2_vp;
    Vector*  y1_vp;
    Vector*  y2_vp;
    int      result;
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_segments = matrix_vector->length;
    mp_array =     matrix_vector->elements;

    NRE(x1_vp = create_vector(num_segments));
    NRE(x2_vp = create_vector(num_segments));
    NRE(y1_vp = create_vector(num_segments));
    NRE(y2_vp = create_vector(num_segments));

    for (i=0; i<num_segments; i++)
    {
        (x1_vp->elements)[i] = ((mp_array[ i ])->elements)[0][0];
        (y1_vp->elements)[i] = ((mp_array[ i ])->elements)[0][1];
        (x2_vp->elements)[i] = ((mp_array[ i ])->elements)[1][0];
        (y2_vp->elements)[i] = ((mp_array[ i ])->elements)[1][1];
    }

    result = pp_plot_segments(plot_ptr,x1_vp, y1_vp, x2_vp, y2_vp, name);

    free_vector(x1_vp);
    free_vector(x2_vp);
    free_vector(y1_vp);
    free_vector(y2_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_segments
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_segments
(
    int           plot_id,
    const Vector* x1_vp,
    const Vector* y1_vp,
    const Vector* x2_vp,
    const Vector* y2_vp,
    const char*   name
)
{
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_segments(plot_ptr, x1_vp, y1_vp, x2_vp, y2_vp, name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_segments
 * -----------------------------------------------------------------------------
 */

static int pp_plot_segments
(
    Plot*         plot_ptr,
    const Vector* x1_vp,
    const Vector* y1_vp,
    const Vector* x2_vp,
    const Vector* y2_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    double  x_min;
    double  x_max;
    double  temp_x1_min;
    double  temp_x1_max;
    double  temp_x2_min;
    double  temp_x2_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y1_min;
    double  temp_y1_max;
    double  temp_y2_min;
    double  temp_y2_max;
    double  temp_y_min;
    double  temp_y_max;
    int   num_segments;
    int   seg_count;
    char  line_type_str[ 256 ];


    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (x1_vp->length != x2_vp->length)
    {
        set_bug("X1 and X2 vectors have different lengths in pp_plot_segments.");
        return ERROR;
    }
    if (x1_vp->length != y1_vp->length)
    {
        set_bug("X1 and Y1 vectors have different lengths in pp_plot_segments.");
        return ERROR;
    }
    if (y1_vp->length != y2_vp->length)
    {
        set_bug("Y1 and Y2 vectors have different lengths in pp_plot_segments.");
        return ERROR;
    }

    num_segments = x1_vp->length;

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (seg_count=0; seg_count<num_segments; seg_count++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e\n",(x1_vp->elements)[seg_count],
                        (y1_vp->elements)[seg_count]));
        ERE(kjb_fprintf(temp_fp,"%e %e\n\n",(x2_vp->elements)[seg_count],
                        (y2_vp->elements)[seg_count]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x1_min = min_vector_element(x1_vp);
    temp_x1_max = max_vector_element(x1_vp);
    temp_x2_min = min_vector_element(x2_vp);
    temp_x2_max = max_vector_element(x2_vp);

    temp_x_min = MIN_OF(temp_x1_min, temp_x2_min);
    temp_x_max = MAX_OF(temp_x1_max, temp_x2_max);

    temp_x_min *= 0.9;
    temp_x_max *= 1.1;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y1_min = min_vector_element(y1_vp);
    temp_y1_max = max_vector_element(y1_vp);
    temp_y2_min = min_vector_element(y2_vp);
    temp_y2_max = max_vector_element(y2_vp);

    temp_y_min = MIN_OF(temp_y1_min, temp_y2_min);
    temp_y_max = MAX_OF(temp_y1_max, temp_y2_max);

    temp_y_min *= 0.9;
    temp_y_max *= 1.1;

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_cols
 *
 * Plots the columns of a matrix
 *
 * This routine plots the columns of a matrix as curves on the assumption that
 * the X-ordinates are eqaully spaced. The parameter "x_offset" is the first the
 * X-ordinate. The points are spaced "x_step" apart. If name_list is not NULL,
 * it is use to label the points. If line_types is not NULL, it further directs
 * the plotting using gnuplot() line types.
 *
 * Note:
 *     The line_types parameter is a bit gnuplot(1) centric.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_cols
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char**  name_list,
    const int*    line_types
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_num_str[ 256 ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int    i, j;
    double x_min;
    double x_max;
    double temp_x_min;
    double temp_x_max;
    int    line_type;
    char   line_type_str[ 256 ];
    Plot*  plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    temp_x_min = x_offset;
    temp_x_max = x_offset + x_step * mp->num_rows;

    if (plot_ptr->num_temp_files == 0)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 0)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    for (j=0; j < mp->num_cols; j++)
    {
        (plot_ptr->num_temp_files)++;

        ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                    plot_ptr->num_temp_files));

        NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

        for (i=0; i<mp->num_rows; i++)
        {
            ERE(kjb_fprintf(temp_fp,"%e %e\n",x_offset+i*x_step,
                            (mp->elements)[ i ][ j ]));
        }

        ERE(kjb_fclose(temp_fp));

        ERE(kjb_sprintf(temp_file_name, sizeof(temp_file_name), "%d",
                        plot_ptr->num_temp_files));

        BUFF_CAT(plot_command_buff, "\"");
        BUFF_CAT(plot_command_buff, temp_file_name);
        BUFF_CAT(plot_command_buff, "\" t \"");

        if ((name_list == NULL) || (name_list[ j ] == NULL))
        {
            ERE(kjb_sprintf(plot_num_str, sizeof(plot_num_str), "%d",
                            plot_ptr->num_temp_files));
            BUFF_CAT(plot_command_buff, plot_num_str);
        }
        else
        {
            BUFF_CAT(plot_command_buff, name_list[ j ]);
        }

        BUFF_CAT(plot_command_buff, "\" w lines ");

        if (line_types == NULL)
        {
            line_type = plot_ptr->num_temp_files;
        }
        else
        {
            line_type = line_types[ j ];
        }

        /* The "lt" is needed for newer gnuplots, likely breaks older ones.
         * Ideally we would know what version of gnuplot we are using and proceed
         * occurdingly, but we don't. For now, just record what worked before, but
         * does not work now. 
        */
#ifdef OLDER_GNUPLOTS
        ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d", line_type));
#else
        ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d", line_type));
#endif 

        BUFF_CAT(plot_command_buff, line_type_str);

        if (j == (mp->num_cols - 1))
        {
            BUFF_CAT(plot_command_buff, "\n");
        }
        else
        {
            BUFF_CAT(plot_command_buff, ",");
        }
    }

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_rows
 *
 * Plots the rows of a matrix
 *
 * This routine plots the rows of a matrix as curves on the assumption that the
 * X-ordinates are eqaully spaced. The parameter "x_offset" is the first the
 * X-ordinate. The points are spaced "x_step" apart. If name_list is not NULL,
 * it is use to label the points. If line_types is not NULL, it further directs
 * the plotting using gnuplot() line types.
 *
 * Note:
 *     The line_types parameter is a bit gnuplot(1) centric.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_rows
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char**  name_list,
    const int*    line_types
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_num_str[ 256 ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int    i, j;
    double x_min;
    double x_max;
    double temp_x_min;
    double temp_x_max;
    int    line_type;
    char   line_type_str[ 256 ];
    Plot*  plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    temp_x_min = x_offset;
    temp_x_max = x_offset + x_step * mp->num_cols;

    if (plot_ptr->num_temp_files == 0)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 0)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    for (i=0; i < mp->num_rows; i++)
    {
        (plot_ptr->num_temp_files) ++;

        ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                    plot_ptr->num_temp_files));

        NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id,
                                          temp_file_name));

        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(temp_fp,"%e %e\n",x_offset+j*x_step,
                            (mp->elements)[ i ][ j ]));
        }

        ERE(kjb_fclose(temp_fp));

        BUFF_CAT(plot_command_buff, "\"");
        BUFF_CAT(plot_command_buff, temp_file_name);
        BUFF_CAT(plot_command_buff, "\" t \"");

        if ((name_list == NULL) || (name_list[ i ] == NULL))
        {
            ERE(kjb_sprintf(plot_num_str, sizeof(plot_num_str), "%d",
                            plot_ptr->num_temp_files));
            BUFF_CAT(plot_command_buff, plot_num_str);
        }
        else
        {
            BUFF_CAT(plot_command_buff, name_list[ i ]);
        }

        BUFF_CAT(plot_command_buff, "\" w lines ");

        if (line_types == NULL)
        {
            line_type = plot_ptr->num_temp_files;
        }
        else
        {
            line_type = line_types[ i ];
        }

        /* The "lt" is needed for newer gnuplots, likely breaks older ones.
         * Ideally we would know what version of gnuplot we are using and proceed
         * occurdingly, but we don't. For now, just record what worked before, but
         * does not work now. 
        */
#ifdef OLDER_GNUPLOTS
        ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d", line_type));
#else
        ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d", line_type));
#endif 

        BUFF_CAT(plot_command_buff, line_type_str);

        if (i == (mp->num_rows - 1))
        {
            BUFF_CAT(plot_command_buff, "\n");
        }
        else
        {
            BUFF_CAT(plot_command_buff, ",");
        }
    }

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_multi_matrix_rows
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_multi_matrix_rows
(
    int           plot_id,
    const Matrix* mp,
    double        x_offset,
    double        x_step,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int    i, j;
    double x_min;
    double x_max;
    double temp_x_min;
    double temp_x_max;
    char   line_type_str[ 256 ];
    Plot*  plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i < mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(temp_fp, "%e %e\n", x_offset+j*x_step,
                            (mp->elements)[i][j]));
        }
        ERE(kjb_fprintf(temp_fp,"\n\n"));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = x_offset;
    temp_x_max = x_offset + x_step * mp->num_cols;

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "plot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(line_type_str, sizeof(line_type_str), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, line_type_str);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_values
 *
 * Plots the values of a matrix
 *
 * This routine plots the values stored in a mtrix as a 3D plot. The plot
 * handle, plot_id, MUST be a 3D plot. Sending a handle for a 2D plot is
 * currently treated as a bug.  For control over the ranges of X and Y, see
 * plot_matrix_values_2().
 *
 * If the matrix is NULL, then this is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting, 3D plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_values
(
    int           plot_id,
    const Matrix* mp
)
{

    if (mp == NULL)
    {
        return NO_ERROR;
    }

    return plot_matrix_values_2(plot_id, mp,
                                0.0, (double)(mp->num_rows - 1),
                                0.0, (double)(mp->num_cols - 1));

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_matrix_values_2
 *
 * Plots the values of a matrix
 *
 * This routine plots the values stored in a mtrix as a 3D plot. The plot
 * handle, plot_id, MUST be a 3D plot. Sending a handle for a 2D plot is
 * currently treated as a bug.  The ranges of X and Y must be specified.
 *
 * If the matrix is NULL, then this is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting, 3D plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_matrix_values_2
(
    int           plot_id,
    const Matrix* mp,
    double        x_min,
    double        x_max,
    double        y_min,
    double        y_max
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i, j;
    double  x_step;
    double  y_step;
    Plot* plot_ptr;


    if (mp == NULL)
    {
        return NO_ERROR;
    }

    NRE(plot_ptr = get_plot_ptr(plot_id));

    if (plot_ptr->dim != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((mp->num_rows < 2) || (mp->num_cols < 2))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    x_step = (x_max - x_min) / (mp->num_rows - 1);
    y_step = (y_max - y_min) / (mp->num_cols - 1);


    ERE(pp_plot_write(plot_ptr, "set parametric\n"));
    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set yrange [%f:%f]\n", y_min, y_max));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->num_temp_files = 1;

    BUFF_CPY(plot_command_buff, "splot '");

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id,
                                      temp_file_name));

    for (i=0; i < mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            ERE(kjb_fprintf(temp_fp,"%e %e %e\n",
                            x_min + (i * x_step), y_min + (j * y_step),
                            (mp->elements)[ i ][ j ]));
        }
        ERE(kjb_fprintf(temp_fp,"\n"));
    }

    ERE(kjb_fclose(temp_fp));

    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "' notitle with lines\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_update
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_update(int plot_id)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_update(plot_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_update
 * -----------------------------------------------------------------------------
 */

static int pp_plot_update(Plot* plot_ptr)
{
    const char* plot_command;


    if (plot_ptr->num_temp_files > 0)
    {
        plot_command = "replot\n";
        ERE(pp_plot_write(plot_ptr, plot_command));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_clear
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_clear(int plot_id)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_clear(plot_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_clear
 * -----------------------------------------------------------------------------
 */

static int pp_plot_clear(Plot* plot_ptr)
{
    int   result = NO_ERROR;


    if (pp_plot_clear_plot(plot_ptr) == ERROR)
    {
        result = ERROR;
    }

    if (pp_plot_clear_data(plot_ptr) == ERROR)
    {
        result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      pp_plot_clear_data
 * -----------------------------------------------------------------------------
 */

static int pp_plot_clear_data(Plot* plot_ptr)
{
    int   result;


    result = plot_remove_data_files(plot_ptr);

    if (    (plot_ptr->attached_data_ptr != NULL)
         && (plot_ptr->attached_data_disposal_fn != NULL)
       )
    {
        (*(plot_ptr->attached_data_disposal_fn))(plot_ptr->attached_data_ptr);
    }

    plot_ptr->attached_data_ptr = NULL;
    plot_ptr->attached_data_disposal_fn = NULL;

    plot_ptr->num_temp_files = 0;
    plot_ptr->x_min = 0.0;
    plot_ptr->x_max = 0.0;
    plot_ptr->y_min = 0.0;
    plot_ptr->y_max = 0.0;

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      pp_plot_clear_plot
 * -----------------------------------------------------------------------------
 */

static int pp_plot_clear_plot(Plot* plot_ptr)
{

    return pp_plot_write(plot_ptr, "set autoscale\nclear\n");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_close
 *
 * Closes a plot
 *
 * This routine closes a plot, removing associated temp files. Only the process
 * that created a plot can close it. If this is not the case, then we simply
 * ignore the close request. This makes it so that a plot_close_all() executed
 * by a child process works properly.
 *
 * Note:
 *     Currently we allow child processes to continue manipulating plots created
 *     by a parent, although this is not necessarily a good idea, and may be
 *     disabled in the future.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_close(int plot_id)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_close(plot_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_close
 * -----------------------------------------------------------------------------
 */

static int pp_plot_close(Plot* plot_ptr)
{
#ifdef UNIX
    IMPORT int  kjb_debug_level;
    const char* plot_command;
    int         plot_index;
    int         result;
    char        dir_buff[ MAX_FILE_NAME_SIZE ];


    if (plot_ptr->create_pid != MY_PID)
    {
        return NO_ERROR;
    }

    plot_command = "quit\n";

    /*
    // Can't return in case of error as it might be due to process having
    // disappeared. In this case we still need to be able to remove the files!
    */
    result = pp_plot_write(plot_ptr, plot_command);

    if ((result == ERROR) && (kjb_debug_level > 0))
    {
        kjb_print_error();
    }

    /*
    // Give it some time to exit, and then make sure it is dead.
    */
    nap(100);
    terminate_child_process(plot_ptr->plot_pid);

    result = pp_plot_clear_data(plot_ptr);

    if (get_plot_temp_file_dir(dir_buff,
                               sizeof(dir_buff),
                               plot_ptr->plot_id ) == ERROR )
    {
        result = ERROR;
    }
    else if (kjb_rmdir(dir_buff) == ERROR)
    {
        result = ERROR;
    }

    plot_index = plot_ptr->plot_id - 1;

    ASSERT(plot_ptr == fs_plot_info_array[ plot_index ]);

    fs_plot_info_array[ plot_index ] = NULL;

    close(plot_ptr->plot_read_des);
    close(plot_ptr->plot_write_des);

    kjb_free(plot_ptr);

    return result;

#else /* Case NOT UNIX follows. */

    set_error("Plotting is only implemented on UNIX.");
    return ERROR;

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_close_all
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

void plot_close_all(void)
{
#ifdef UNIX
    Plot*          plot_ptr;
    int            count;


    if (fs_plot_info_array != NULL)
    {
        for (count = 0; count < fs_max_num_plots; count++)
        {
            plot_ptr = fs_plot_info_array[ count ];

            if (plot_ptr != NULL)
            {
                EPE(pp_plot_close(plot_ptr));
            }
        }

        fs_max_plot_num = 0;

        kjb_free(fs_plot_info_array);
        fs_plot_info_array = NULL;
    }
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot_write
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot_write(int plot_id, const char* buff)
{
    Plot* plot_ptr;

    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot_write(plot_ptr, buff);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot_write
 * -----------------------------------------------------------------------------
 */

static int pp_plot_write(Plot* plot_ptr, const char* buff)
{
#ifdef UNIX
    size_t len;
    long   result;
    char   read_buff[ 50000 ];
    int    read_res;
    int    child_status;


    /*
    // Have to check that the plot process exists. This check is needed for
    // for every read due to ugliness that are beyond our control.
    */
    child_status = check_child(plot_ptr->plot_pid);

    if (child_status == PROCESS_IS_DEAD)
    {
#ifdef REALLY_TEST
        TEST_PSO(("\nPlot child process status is PROCESS_IS_DEAD\n"));
        TEST_PSO(("Output from execution of \"ps -eaf\" follows.\n\n"));
        TEST_PSO(("----------------------------------------------------------------------\n\n"));
        EPE(kjb_system("ps -eaf"));
        TEST_PSO(("\n----------------------------------------------------------------------\n\n"));
#endif
        set_error("Plot process (pid=%d) is no longer alive.",
                  plot_ptr->plot_pid);
        add_error("(If it never existed, this could be a program problem.)");
        return ERROR;
    }

    /*
    // Next swallow any leftover messages for us, especially ones that say that
    // the process has died.
    */
    read_buff[ 0 ] = '\0';

    /*CONSTCOND*/
    while ( TRUE )
    {
        ERE(read_res = BUFF_DGET_LINE(plot_ptr->plot_read_des, read_buff));

        if (read_res > 0)
        {
            if (    (STRCMP_EQ(read_buff, "Broken Pipe"))
                 || (HEAD_CMP_EQ(read_buff, "XIO:"))
               )
            {
                set_error("Failure writing to plot pipe (process %d).",
                          plot_ptr->plot_pid);
                add_error("It looks like that process is dead. ");
                cat_error("Killing it to make sure.");
                terminate_child_process(plot_ptr->plot_pid);
                return ERROR;
            }
            else
            {
                verbose_pso(1, read_buff);
                verbose_pso(1, "\n");
            }
        }
        else break;
    }

    /*
    // OK, the plot process seems to be alive, so continue with the write.
    */
    len = strlen(buff);

    verbose_pso(200, buff);

    /*
    sleep(2);
    term_beep(); 
    */

    result = safe_pipe_write(plot_ptr->plot_write_des, buff, len);

    ASSERT(len <= INT32_MAX);

    if (result != (long)len)
    {
        insert_error("Failure writing to plot pipe (process %d).",
                     plot_ptr->plot_pid);
        return ERROR;
    }

#ifdef TEST
    /*
    // Get any messages if they are available after 50 ms. to make debugging
    // easier.
    */

    nap(50);

    /*CONSTCOND*/
    while ( TRUE )
    {
        ERE(read_res = BUFF_DGET_LINE(plot_ptr->plot_read_des, read_buff));

        if (read_res > 0)
        {
            if (    (STRCMP_EQ(read_buff, "Broken Pipe"))
                 || (HEAD_CMP_EQ(read_buff, "XIO:"))
               )
            {
                set_error("Failure writing to plot pipe (process %d).",
                          plot_ptr->plot_pid);
                add_error("It looks like that process is dead. ");
                cat_error("Killing it to make sure.");
                terminate_child_process(plot_ptr->plot_pid);
                return ERROR;
            }
            else
            {
                verbose_pso(1, read_buff);
                verbose_pso(1, "\n");
            }
        }
        else break;
    }
#endif

    return NO_ERROR;

#else /* Case NOT UNIX follows. */

    set_error("Plotting is only implemented on UNIX.");
    return ERROR;

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      open_plot_temp_file
 * -----------------------------------------------------------------------------
 */

static FILE* open_plot_temp_file(int plot_id, const char* file_name)
{
    char file_path[ MAX_FILE_NAME_SIZE ];


    ERN(get_plot_temp_file_path(file_path, sizeof(file_path), plot_id,
                                file_name));

    return kjb_fopen(file_path, "w");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      get_plot_temp_file_path
 * -----------------------------------------------------------------------------
 */

static int get_plot_temp_file_path
(
    char*       file_path_buff,
    size_t      file_path_buff_size,
    int         plot_id,
    const char* file_name
)
{
    char dir_buff[ MAX_FILE_NAME_SIZE ];


    ERE(get_plot_temp_file_dir(dir_buff, sizeof(dir_buff), plot_id));

    ERE(kjb_sprintf(file_path_buff, file_path_buff_size, "%s/%s", dir_buff,
                    file_name));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      get_plot_temp_file_dir
 * -----------------------------------------------------------------------------
 */

static int get_plot_temp_file_dir
(
    char*  dir_buff,
    size_t dir_buff_size,
    int    plot_id
)
{
    char  user_id[ 256 ];
    char user_dir_buff[ MAX_FILE_NAME_SIZE ];


    ERE(get_user_id(user_id, sizeof(user_id)));

    ERE(kjb_sprintf(user_dir_buff, sizeof(user_dir_buff), "%s%s%s",
                    TEMP_DIR, DIR_STR, user_id));

    if (get_path_type(user_dir_buff) != PATH_IS_DIRECTORY)
    {
        ERE(kjb_mkdir(user_dir_buff));
    }

    ERE(kjb_sprintf(dir_buff, dir_buff_size, "%s%s%ld-%d-plots",
                    user_dir_buff, DIR_STR, (long)MY_PID, plot_id));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      get_plot_temp_file_name
 * -----------------------------------------------------------------------------
 */

static int get_plot_temp_file_name
(
    char*  file_name_buff,
    size_t file_name_buff_size,
    int    file_num
)
{


    return kjb_sprintf(file_name_buff, file_name_buff_size, "%d", file_num);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      plot_remove_data_files
 * -----------------------------------------------------------------------------
 */

static int plot_remove_data_files(Plot* plot_ptr)
{
    char  temp_file_path[ MAX_FILE_NAME_SIZE ];
    char  temp_file_name[ 100 ];
    int   i;
    int   temp_result;
    int   result = NO_ERROR;


    for (i=1; i <= plot_ptr->num_temp_files; i++)
    {
        EPE(temp_result = get_plot_temp_file_name(temp_file_name,
                                                  sizeof(temp_file_name), i));

        if (temp_result != ERROR)
        {
            EPE(temp_result = get_plot_temp_file_path(temp_file_path,
                                                      sizeof(temp_file_path),
                                                      plot_ptr->plot_id,
                                                      temp_file_name));
        }

        if (temp_result != ERROR)
        {
            EPE(temp_result = kjb_unlink(temp_file_path));
        }

        if (temp_result == ERROR) result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
/*
 * =============================================================================
 * STATIC                      free_display_plot_flag_stack
 * -----------------------------------------------------------------------------
 */

static void free_display_plot_flag_stack(void)
{
    /* Dont free fs_plot_info_array here, as it is free'd in plot_close_all.*/

    free_queue(&fs_display_plot_flag_stack_head, (Queue_element**)NULL,
               kjb_free);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * =============================================================================
 *                            plot3_points
 *
 * Plots points R^3
 *
 * This routine is another interface to plot points. The points are given by three
 * vectors, one for the X-ordinates, one for the Y-ordinates and one for the
 * Z-ordinates. If name is not NULL, it labels the point group.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot3_points
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
)
{
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot3_points(plot_ptr, x_vp, y_vp, z_vp, name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot3_points
 * -----------------------------------------------------------------------------
 */

static int pp_plot3_points
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    double  z_min;
    double  z_max;
    double  temp_z_min;
    double  temp_z_max;
    char  point_type[ 256 ];


    if (plot_ptr->dim != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (x_vp->length != y_vp->length || y_vp->length != z_vp->length)
    {
        set_bug("X and Y vectors have different lengths in plot_points.");
        return ERROR;
    }

    if (x_vp->length <= 0)
    {
        return NO_ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i<x_vp->length; i++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e %e\n",(x_vp->elements)[i],
                        (y_vp->elements)[i],
                        (z_vp->elements)[i]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * min_vector_element(x_vp);
    temp_x_max = 1.1 * max_vector_element(x_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * min_vector_element(y_vp);
    temp_y_max = 1.1 * max_vector_element(y_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set yrange [%f:%f]\n", y_min, y_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    temp_z_min = 0.9 * min_vector_element(z_vp);
    temp_z_max = 1.1 * max_vector_element(z_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        z_min = temp_z_min;
        z_max = temp_z_max;
    }
    else
    {
        z_min = MIN_OF(temp_z_min, plot_ptr->z_min);
        z_max = MAX_OF(temp_z_max, plot_ptr->z_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set zrange [%f:%f]\n", z_min, z_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->z_min = z_min;
    plot_ptr->z_max = z_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "splot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w points ");
    ERE(kjb_sprintf(point_type, sizeof(point_type), "%d",
                    plot_ptr->num_temp_files));
    BUFF_CAT(plot_command_buff, point_type);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            plot3_curve
 *
 * Plots a curve in R^3 by linking points
 *
 * This routine plots the curve implied by liking the points whose X-ordinates
 * are in the argument "x_vp", whose Y-ordinates are in the argument "y_vp" and
 * whose Z-ordinates are in "z_vp".
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: plotting
 *
 * -----------------------------------------------------------------------------
 */

int plot3_curve
(
    int           plot_id,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
)
{
    Plot* plot_ptr;


    NRE(plot_ptr = get_plot_ptr(plot_id));

    return pp_plot3_curve(plot_ptr, x_vp, y_vp, z_vp, name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     pp_plot3_curve
 * -----------------------------------------------------------------------------
 */

static int pp_plot3_curve
(
    Plot*         plot_ptr,
    const Vector* x_vp,
    const Vector* y_vp,
    const Vector* z_vp,
    const char*   name
)
{
    FILE* temp_fp;
    char  temp_file_name[ MAX_FILE_NAME_SIZE ];
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    int   i;
    double  x_min;
    double  x_max;
    double  temp_x_min;
    double  temp_x_max;
    double  y_min;
    double  y_max;
    double  temp_y_min;
    double  temp_y_max;
    double  z_min;
    double  z_max;
    double  temp_z_min;
    double  temp_z_max;
    char  point_type[ 256 ];


    if (plot_ptr->dim != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (x_vp->length != y_vp->length || y_vp->length != z_vp->length)
    {
        set_bug("X and Y vectors have different lengths in plot_points.");
        return ERROR;
    }

    if (x_vp->length <= 0)
    {
        return NO_ERROR;
    }

    (plot_ptr->num_temp_files)++;

    ERE(get_plot_temp_file_name(temp_file_name, sizeof(temp_file_name),
                                plot_ptr->num_temp_files));

    NRE(temp_fp = open_plot_temp_file(plot_ptr->plot_id, temp_file_name));

    for (i=0; i<x_vp->length; i++)
    {
        ERE(kjb_fprintf(temp_fp,"%e %e %e\n",(x_vp->elements)[i],
                        (y_vp->elements)[i],
                        (z_vp->elements)[i]));
    }

    ERE(kjb_fclose(temp_fp));

    temp_x_min = 0.9 * min_vector_element(x_vp);
    temp_x_max = 1.1 * max_vector_element(x_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        x_min = temp_x_min;
        x_max = temp_x_max;
    }
    else
    {
        x_min = MIN_OF(temp_x_min, plot_ptr->x_min);
        x_max = MAX_OF(temp_x_max, plot_ptr->x_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set xrange [%f:%f]\n", x_min, x_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->x_min = x_min;
    plot_ptr->x_max = x_max;

    temp_y_min = 0.9 * min_vector_element(y_vp);
    temp_y_max = 1.1 * max_vector_element(y_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        y_min = temp_y_min;
        y_max = temp_y_max;
    }
    else
    {
        y_min = MIN_OF(temp_y_min, plot_ptr->y_min);
        y_max = MAX_OF(temp_y_max, plot_ptr->y_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set yrange [%f:%f]\n", y_min, y_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->y_min = y_min;
    plot_ptr->y_max = y_max;

    temp_z_min = 0.9 * min_vector_element(z_vp);
    temp_z_max = 1.1 * max_vector_element(z_vp);

    if (plot_ptr->num_temp_files == 1)
    {
        z_min = temp_z_min;
        z_max = temp_z_max;
    }
    else
    {
        z_min = MIN_OF(temp_z_min, plot_ptr->z_min);
        z_max = MAX_OF(temp_z_max, plot_ptr->z_max);
    }

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "set zrange [%f:%f]\n", z_min, z_max));

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    plot_ptr->z_min = z_min;
    plot_ptr->z_max = z_max;

    if (plot_ptr->num_temp_files == 1)
    {
        BUFF_CPY(plot_command_buff, "splot ");
    }
    else
    {
        BUFF_CPY(plot_command_buff, "replot ");
    }

    BUFF_CAT(plot_command_buff, "\"");
    BUFF_CAT(plot_command_buff, temp_file_name);
    BUFF_CAT(plot_command_buff, "\" t \"");

    if (name != NULL)
    {
        char temp_buff [ 256 ];

        BUFF_CPY(temp_buff, name);
        char_for_char_translate(temp_buff, '"', '\'');
        BUFF_CAT(plot_command_buff, temp_buff);
    }

    BUFF_CAT(plot_command_buff, "\" w lines ");

    /* The "lt" is needed for newer gnuplots, likely breaks older ones.
     * Ideally we would know what version of gnuplot we are using and proceed
     * occurdingly, but we don't. For now, just record what worked before, but
     * does not work now. 
    */
#ifdef OLDER_GNUPLOTS
    ERE(kjb_sprintf(point_type, sizeof(point_type), "%d",
                    plot_ptr->num_temp_files));
#else
    ERE(kjb_sprintf(point_type, sizeof(point_type), "lt %d",
                    plot_ptr->num_temp_files));
#endif 

    BUFF_CAT(plot_command_buff, point_type);
    BUFF_CAT(plot_command_buff, "\n");

    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    ERE(pp_plot_update(plot_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int wait_for_plot(Plot* plot_ptr)
{
    char  plot_command_buff[ PLOT_COMMAND_BUFF_SIZE ];
    char  temp_name[ MAX_FILE_NAME_SIZE ];
    int   lock_wait_count = 0;


    BUFF_GET_TEMP_FILE_NAME(temp_name);

    ERE(kjb_sprintf(plot_command_buff, sizeof(plot_command_buff),
                    "!touch %s\n", temp_name));
    ERE(pp_plot_write(plot_ptr, plot_command_buff));

    nap(200);  /* Get off the CPU. */

    lock_wait_count = 0;

    while (    (lock_wait_count < 1000)
            && ( ! is_file(temp_name))
          )
    {
        if (lock_wait_count % 20 == 0)
        {
            int child_status;

            child_status = check_child(plot_ptr->plot_pid);

            if (child_status == PROCESS_IS_DEAD)
            {
                set_error("Plot process (pid=%d) is no longer alive.",
                          plot_ptr->plot_pid);
                add_error("(If it never existed, this could be a program problem.)");
                return ERROR;
            }
        }

        if (kjb_isatty(fileno(stdout)))
        {
            char mess[ 1000 ];

            ERE(kjb_sprintf(mess, sizeof(mess),
                            "Waiting for lock file %s (%04d)\r",
                            temp_name, lock_wait_count));
            term_puts(mess);
        }
        nap(500);
        lock_wait_count++;
    }

    if (is_file(temp_name))
    {
        EPE(kjb_unlink(temp_name));
    }
    else 
    {
        p_stderr("Giving up waiting on lockfile %s.\n",
                 temp_name);
        p_stderr("Likely some problem piping commands to gnuplot.\n");
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

