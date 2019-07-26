
/* $Id: g3_geom_view.c 4727 2009-11-16 20:53:54Z kobus $ */

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

/*
 * Note: This stuff is very OBSOLETE. 
*/

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "g3/g3_geom_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define INC_TARGET_STR_CPY(x,y) \
          {const char *t; t = y; while (*t != '\0'){*x = *t; x++; t++;} *x='\0';}

/* -------------------------------------------------------------------------- */

static int fs_geom_view_num = 0;
static int fs_max_num_geom_views = DEFAULT_MAX_NUM_GEOM_VIEWS;
static Geom_view **fs_geom_view_info_array = NULL;

/* -------------------------------------------------------------------------- */

#ifdef UNIX
static Geom_view* geom_view_open_guts(int);
#endif 

/* -------------------------------------------------------------------------- */

int set_max_num_geom_views(int max_num_geom_views)
{


    if (fs_geom_view_num > 0)
    {
        set_bug("Can't set max number of geom_views after one is open.");
        return ERROR;
    }

    fs_max_num_geom_views = max_num_geom_views;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Geom_view* geom_view_open(void)
{
#ifdef UNIX
    Geom_view* geom_view_ptr;
    int        i;


    if (fs_max_num_geom_views < 1)
    {
        set_bug("Max number of geom_views is not positive.");
        return NULL;
    }

    if (fs_geom_view_num == 0)
    {
        NRN(fs_geom_view_info_array = N_TYPE_MALLOC(Geom_view*,
                                                   fs_max_num_geom_views));

        for (i=0; i<fs_max_num_geom_views; i++)
        {
            fs_geom_view_info_array[ i ] = NULL;
        }
    }

    if (fs_geom_view_num < fs_max_num_geom_views)
    {
        NRN(geom_view_ptr = geom_view_open_guts( fs_geom_view_num ));
        fs_geom_view_info_array[ fs_geom_view_num ] = geom_view_ptr;
    }

    else
    {
        geom_view_ptr = fs_geom_view_info_array[
                                      fs_geom_view_num % fs_max_num_geom_views ];

        if (geom_view_ptr == NULL)
        {
            NRN(geom_view_ptr = geom_view_open_guts(
                                      fs_geom_view_num % fs_max_num_geom_views));

            fs_geom_view_info_array[ fs_geom_view_num % fs_max_num_geom_views] =
                                                              geom_view_ptr;
        }
        else
        {
            EPE(geom_view_clear_data(geom_view_ptr));
        }
    }

    fs_geom_view_num++;

    ERN(geom_view_dummy(geom_view_ptr));

    return geom_view_ptr;

#else /* Case NOT UNIX follows. */

    set_error("Support for geomview is obsolete, and ever only worked on UNIX.");

    return NULL;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static Geom_view* geom_view_open_guts(int geom_view_num)
{
    static int first_time = TRUE;
    Geom_view *geom_view_ptr;
    char user_id[ 200 ];
    char pipe_name[ 200 ];
    int geom_view_pipe[2];
    int geom_view_write_des;
    int geom_view_pid;
    char command_buff[ 200 ];
    int x1, x2, y1, y2, required_y1, required_y2;


    if (pipe(geom_view_pipe) == EOF)
    {
        kjb_fprintf(stderr,
                    "Creation of pipe for a geomview process failed.\n");
        return NULL;
    }

    if (first_time)
    {
        add_cleanup_function(geom_view_close_all);
        first_time = FALSE;
    }

    geom_view_pid = kjb_fork();

    if (geom_view_pid < 0)
    {
        set_error("Fork of geomview process failed.%S");
        return NULL;
    }

    if (IS_CHILD(geom_view_pid))
    {
        char error_file[ 200 ];
        char system_command[ 200 ];


        ERN(get_user_id(user_id, sizeof(user_id)));

        close(geom_view_pipe[WRITE_END]);

        dup2(geom_view_pipe[READ_END],fileno(stdin));

        sprintf(error_file, "/tmp/%s-%ld-%ld-geomview.error", user_id,
                (long)MY_PID, (long)(geom_view_num+1));

        NRN(kjb_freopen(error_file, "w", stderr));

        sprintf(pipe_name,"%s-%ld-%ld",user_id, (long)MY_PID,
                (long)(geom_view_num + 1));

        sprintf(system_command, "togeomview -c %s" , pipe_name);

        kjb_exec(system_command);

        /*NOTREACHED*/

        return NULL;   /* Keep error checkers happy. */
    }
    else
    {
        if (signal(SIGPIPE, geom_view_sigpipe_fn) == SIG_ERR)
        {
            kjb_fprintf(stderr,
                        "Failed to setup pipe signal to geomview process\n");
            return NULL;
        }

        NRN(geom_view_ptr = TYPE_MALLOC(Geom_view));

        geom_view_write_des = geom_view_pipe[ WRITE_END ];

        geom_view_ptr->write_des = geom_view_write_des;

        x1 = 620;
        x2 = 1070;

        /*
           There seems to be a bug in geomview's window command that
           requires a hack here.
        */
        required_y1 = 20 + geom_view_num * 100;
        required_y2 = 470 + geom_view_num * 100;

        y1 = 870 - required_y2;
        y2 = 870 - required_y1;

        sprintf(command_buff,
                "(window default window { position %ld %ld %ld %ld } ) \n",
                (long)x1, (long)x2, (long)y1, (long)y2);

        ERN(geom_view_write(geom_view_ptr, command_buff));

        sprintf(command_buff,
                "(window allcams { position %ld %ld %ld %ld } ) \n",
                (long)x1, (long)x2, (long)y1, (long)y2);

        ERN(geom_view_write(geom_view_ptr, command_buff));

        x1 = 500;
        x2 = 600;
        y1 = 20 + geom_view_num * 100;
        y2 = 470 + geom_view_num * 100;

        sprintf(command_buff,
                "(ui-panel tools on { position %ld %ld %ld %ld } ) \n",
                (long)x1, (long)x2, (long)y1, (long)y2);

        ERN(geom_view_write(geom_view_ptr, command_buff));

        x1 = 100;
        x2 = 500;

        sprintf(command_buff,
                "(ui-panel geomview on { position %ld %ld %ld %ld } ) \n",
                (long)x1, (long)x2, (long)y1, (long)y2);

        ERN(geom_view_write(geom_view_ptr, command_buff));

        geom_view_ptr->id = geom_view_num + 1;
        geom_view_ptr->app_field = NULL;
        geom_view_ptr->pid = geom_view_pid;
        geom_view_ptr->coloured_object_count = 0;
        geom_view_ptr->object_list_head = NULL;
        geom_view_ptr->label = NULL;

        return geom_view_ptr;
    }
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Geom_view* get_geom_view_info_ptr(int geom_view_num)
{

    if (geom_view_num == NEW_GEOM_VIEW)
    {
        return geom_view_open();
    }
    else if (geom_view_num == CURRENT_GEOM_VIEW)
    {
        return fs_geom_view_info_array[(fs_geom_view_num-1) %
                                                           fs_max_num_geom_views];
    }
    else if (geom_view_num <= 0)
    {
        set_bug("Non-positive argument passed to get_geom_view_ptr.");
        return NULL;
    }
    else if (geom_view_num > fs_geom_view_num)
    {
        set_bug("Request for info ptr for geom view %d, but there are only %d.",
                geom_view_num, fs_geom_view_num);
        return NULL;
    }
    else
    {
        return fs_geom_view_info_array[(geom_view_num-1)% fs_max_num_geom_views];
    }
    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_geometry
(
    Geom_view*  geom_view_ptr,
    const char* geometry,
    const char* label,
    double      red,
    double      green,
    double      blue
)
{


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_geometry.");
        return ERROR;
    }

    if (geom_view_ptr->object_list_head != NULL)
    {
        free_queue(&(geom_view_ptr->object_list_head), NULL, kjb_free);
    }

    if (geom_view_ptr->label != NULL)
    {
        kjb_free(geom_view_ptr->label);
        geom_view_ptr->label = NULL;
    }

    geom_view_ptr->coloured_object_count = 0;

    ERE(geom_view_add_geometry(geom_view_ptr, geometry, label, red, green,blue));

    ERE(geom_view_display(geom_view_ptr));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_add_geometry
(
    Geom_view*  geom_view_ptr,
    const char* geometry,
    const char* label,
    double      red,
    double      green,
    double      blue
)
{
    char *new_geometry;
    int result;


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_add_geometry.");
        return ERROR;
    }

    if (geom_view_ptr->label == NULL)
    {
        geom_view_ptr->label = kjb_strdup(label);
    }
    else
    {
        if (! STRCMP_EQ(label, geom_view_ptr->label))
        {
            set_bug("Label does not match previous in geom_view_add_geometry.");
            return ERROR;
        }
    }


    NRE(new_geometry = get_facet_list(geometry, red, green, blue));

    result = geom_view_add_object(geom_view_ptr,new_geometry);

    kjb_free(new_geometry);

    if (result != ERROR)
    {
        (geom_view_ptr->coloured_object_count)++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_dummy(const Geom_view* geom_view_ptr)
{


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_dummy.");
        return ERROR;
    }

    ERE(geom_view_write(geom_view_ptr, "(new-geometry dummy "));

    ERE(geom_view_write(geom_view_ptr,
                        "{appearance {+edge +transparent -evert linewidth 2} LIST  \n"));

    ERE(geom_view_write(geom_view_ptr, "{ VECT 1 2 0\n"));
    ERE(geom_view_write(geom_view_ptr, "2 0 \n"));
    ERE(geom_view_write(geom_view_ptr, "0 0 0 0 0 0 \n"));
    ERE(geom_view_write(geom_view_ptr, "} \n }\n"));

    ERE(geom_view_write(geom_view_ptr, ")\n"));

    ERE(geom_view_write(geom_view_ptr, "(bbox-draw dummy no)\n"));


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_add_axis(Geom_view* geom_view_ptr, double length)
{
    char axis_geometry [ 200 ];
    char axis_vector[ 200 ];


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_add_axis.");
        return ERROR;
    }

    BUFF_CPY(axis_geometry, "{ VECT 3 6 0\n");
    BUFF_CAT(axis_geometry, "2 2 2 0 0 0 \n");

    sprintf(axis_vector, "0 0 0 %f 0 0 \n", length);
    BUFF_CAT(axis_geometry, axis_vector);


    sprintf(axis_vector, "0 0 0 0 %f 0 \n", length);
    BUFF_CAT(axis_geometry, axis_vector);

    sprintf(axis_vector, "0 0 0 0 0 %f \n", length);
    BUFF_CAT(axis_geometry, axis_vector);

    BUFF_CAT(axis_geometry, "}\n");

    ERE(geom_view_add_object(geom_view_ptr, axis_geometry));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_add_object(Geom_view* geom_view_ptr, const char* geometry)
{
    char *new_geometry;


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_add_object.");
        return ERROR;
    }

    NRE(new_geometry = kjb_strdup(geometry));

    ERE(insert_into_queue(&(geom_view_ptr->object_list_head),
                          (Queue_element**)NULL, new_geometry));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_display(const Geom_view* geom_view_ptr)
{
    Queue_element *cur_elem;


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_display.");
        return ERROR;
    }

    if ( geom_view_ptr->label == NULL)
    {
        set_bug("No name for geometry in geom_view_display.");
        return ERROR;
    }


    ERE(geom_view_write(geom_view_ptr, "(geometry "));

    ERE(geom_view_write(geom_view_ptr, geom_view_ptr->label));

    ERE(geom_view_write(geom_view_ptr, " { "));

    ERE(geom_view_write(geom_view_ptr,
                        "appearance { +edge -evert linewidth 3 \n"));

    ERE(geom_view_write(geom_view_ptr,
                        "material {  }"));

    ERE(geom_view_write(geom_view_ptr, "}"));

    ERE(geom_view_write(geom_view_ptr, "LIST \n"));

    cur_elem = geom_view_ptr->object_list_head;

    while (cur_elem != NULL)
    {
        ERE(geom_view_write(geom_view_ptr,(char*)(cur_elem->contents)));
        cur_elem = cur_elem->next;
    }

    ERE(geom_view_write(geom_view_ptr, "}\n"));

    ERE(geom_view_write(geom_view_ptr, ")\n"));

    ERE(geom_view_write(geom_view_ptr, "(bbox-draw "));
    ERE(geom_view_write(geom_view_ptr, geom_view_ptr->label));
    ERE(geom_view_write(geom_view_ptr, " no)\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_clear_data(const Geom_view* geom_view_ptr)
{


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_clear_data.");
        return ERROR;
    }

    ERE(geom_view_write(geom_view_ptr, "(delete allgeoms)\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_close(Geom_view* geom_view_ptr)
{
    int geom_view_index;


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_close.");
        return ERROR;
    }

    ERE(geom_view_write(geom_view_ptr, "(exit)\n"));

    geom_view_index = ((geom_view_ptr->id) - 1) % fs_max_num_geom_views;
    fs_geom_view_info_array[ geom_view_index ] = NULL;

    kjb_free(geom_view_ptr);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void geom_view_close_all(void)
{
    Geom_view *geom_view_ptr;
    int count;


    if (fs_geom_view_info_array == NULL) return;

    for (count = 0; count < fs_max_num_geom_views; count++)
    {
        geom_view_ptr = fs_geom_view_info_array[ count ];

        if (geom_view_ptr != NULL)
        {
            EPE(geom_view_close(geom_view_ptr));
        }
    }

    fs_geom_view_num = 0;

    kjb_free(fs_geom_view_info_array);
    fs_geom_view_info_array = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

char* get_facet_list
(
    const char* geometry,
    double      red,
    double      green,
    double      blue
)
{
    char  colour_str[ 200 ];
    char  line [ 10000 ];
    char* line_pos;
    char  word [ 200 ];
    char* new_geometry;
    int   max_new_geometry_size;
    const char* geometry_pos;
    char*       new_geometry_pos;
    int         num_facet_points;
    int         num_lines;
    int         j, k;
    int         done;


    kjb_sprintf(colour_str, sizeof(colour_str), "%5.3f %5.3f %5.3f 0.5", red, green, blue);

    ERN(num_lines = string_count_real_lines(geometry));

    /*
       Heavily over-estimated, as strlen(geometry) should be enough.
       However, make it so that it is impossilbe to exceed.
    */
    max_new_geometry_size = strlen(geometry) + (num_lines*strlen(colour_str)) +
                                                                  ROOM_FOR_NULL;

    NRN(new_geometry = STR_MALLOC(max_new_geometry_size));

    geometry_pos = geometry;
    new_geometry_pos = new_geometry;

    BUFF_SGET_REAL_LINE(&geometry_pos, line);

    done = FALSE;

    while ( ! done)
    {
        ERN(BUFF_SGET_REAL_LINE(&geometry_pos, line));

        line_pos = line;

        trim_beg(&line_pos);

        if (*line_pos == '}')
        {
            done = TRUE;
        }
        else
        {
            INC_TARGET_STR_CPY(new_geometry_pos, line);
            INC_TARGET_STR_CPY(new_geometry_pos, "\n");

            for (k=0; k<3; k++)
            {
                if ( ! BUFF_GET_TOKEN_OK(&line_pos, word))
                {
                    set_bug("Unexpected end of line in get_facet_list.");
                    return NULL;
                }
            }

            ERN(ss1pi(word, &num_facet_points));

            for (j=0; j<num_facet_points; j++)
            {
                ERN(BUFF_SGET_REAL_LINE(&geometry_pos, line));
                INC_TARGET_STR_CPY(new_geometry_pos, line);
                INC_TARGET_STR_CPY(new_geometry_pos, "\n");
            }

            ERN(BUFF_SGET_REAL_LINE(&geometry_pos, line));

            line_pos = line;

            for (k=0; k<1+num_facet_points; k++)
            {
                if ( ! BUFF_GET_TOKEN_OK(&line_pos, word))
                {
                    set_bug("Unexpected end of line in get_facet_list.");
                    return NULL;
                }

                INC_TARGET_STR_CPY(new_geometry_pos, word);
                INC_TARGET_STR_CPY(new_geometry_pos, " ");
            }

            INC_TARGET_STR_CPY(new_geometry_pos, colour_str);
            INC_TARGET_STR_CPY(new_geometry_pos, " }\n");
        }
    }

    return new_geometry;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int geom_view_write(const Geom_view* geom_view_ptr, const char* buff)
{
    size_t len;


    if (geom_view_ptr == NULL)
    {
        set_bug("Null pointer passed to geom_view_write.");
        return ERROR;
    }

    len = strlen(buff);

    if (kjb_write(geom_view_ptr->write_des, buff, len) == ERROR)
    {
        insert_error("Failure writing %d bytes to geom_view pipe.", len);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
TRAP_FN_RETURN_TYPE geom_view_sigpipe_fn(int __attribute__((unused)) dummy_sig)
{


    kjb_fprintf(stderr, "Geomview has died.\n");

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

