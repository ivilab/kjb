
/* $Id: i_video.c 5831 2010-05-02 21:52:24Z ksimek $ */

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

#include "i/i_gen.h"
#include "i/i_video.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_VIDEO

#    include "i/i_valid.h"
#    include <setjmp.h>

#    define VIDEOGRAB_LOCKED_FIELDS   \
             (VGL_CROP | VGL_FILENAME | VGL_BLACK | VGL_AVERAGE | VGL_HUE |    \
              VGL_BRIGHTNESS | VGL_CONTRAST | VGL_SATURATION | VGL_FACTORY |   \
              VGL_SAVE_PRESET | VGL_RECALL | VGL_GARBAGE |                     \
              VGL_QUIT | VGL_VERBOSE |                                         \
              VGL_RETURN | VGL_DOUBLE_SIZE | VGL_HALF_SIZE | VGL_SAVE_FILE |   \
              VGL_SAVE_FLOAT | VGL_BLACK_RGB | VGL_BLACK_IMG | VGL_FORMAT |    \
              VGL_STDEV)

/*
// If any new videograb routine is used, make sure that in the file
// videograb_sub.c, the setting of Mike's
// verbose level is tied to my verbose level (take a look at the routines that
// are already used). Then make sure that after the call to the routine, that we
// do a kjb_fflush((FILE*)NULL);
*/

#endif

/* -------------------------------------------------------------------------- */

#define  MAX_NUM_CAPTURE_TRIES 5

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_VIDEO
    static int fs_video_active        = FALSE;
    static jmp_buf fs_before_video_routine_env;
#endif

static int fs_capture_frame_count = 1;

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_VIDEO
    static void stop_video_grabber_guts(void);
    static TRAP_FN_RETURN_TYPE video_grabber_atn_fn(TRAP_FN_ARGS);
#endif

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                               set_video_options
 *
 *
 *
 * -----------------------------------------------------------------------------
 */

/*
// We have a set routine expecially for video so that this file is not
// automatically linked whenever kjb_set is used. Thus if these set commands are
// needed, then set_video_options has to be explicitly called.
*/

int set_video_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_int;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);


    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "capture-frame-count"))
         || (match_pattern(lc_option, "cfc"))
       )
    {
        if (value[ 0 ] == '\0')
        {
            ERE(pso("Capture frame count (cfc) is %d.\n", fs_capture_frame_count));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("capture-frame-count = %d\n", fs_capture_frame_count));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            ERE(set_capture_frame_count(temp_int));
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             set_capture_frame_count
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

/*
// We have to export this one because programs such as multi_capture use it. It
// is possible to get arround this, but since the video set routines are
// regarded as special (see comment for kjb_video_set()), it is easier to simply
// export this routine.
*/

int set_capture_frame_count(int new_value)
{
    if (new_value < 1)
    {
        set_error("Capture frame count (\"capture-frame-count\" must ");
        set_error("be greater than zero.");
        return ERROR;
    }

    fs_capture_frame_count = new_value;

#ifdef KJB_HAVE_VIDEO
    if (fs_video_active)
    {
        struct msg_arg_struct msg_arg;
        int    send_args_res;

        if (init_args_vgrab(&msg_arg) == NULL)
        {
            set_error("Read of videograbber window failed.");
            return ERROR;
        }

        msg_arg.avgframecount = fs_capture_frame_count;

        send_args_res = send_args_vgrab(&msg_arg);

        kjb_fflush((FILE*)NULL);  /* Always flush after a videograb routine. */

        if (send_args_res == -1)
        {
            set_error("Modification of videograbber window failed.");
            return ERROR;
        }

    }
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_capture_frame_count(void)
{
    return fs_capture_frame_count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int start_video_grabber(void)
{
#ifdef KJB_HAVE_VIDEO
    const int              argc                      = 1;
    char                   zeroth_program_arg[ 100 ];
    char*                  argv[ 1 ];
    struct msg_arg_struct  msg_arg;
    struct msg_arg_struct* init_args_res;
    int                    dflags                    = NO_RESIZE;
    int                    send_args_res;


    BUFF_GET_PROGRAM_NAME(zeroth_program_arg);
    argv[ 0 ] = zeroth_program_arg;

    init_vgrab(argc, argv, dflags);
    init_args_res = init_args_vgrab(&msg_arg);
    kjb_fflush((FILE*)NULL); /* Always flush after a videograb routine. */

    if (init_args_res == NULL)
    {
        set_error("Initialization of videograbber window failed");
        return ERROR;
    }

    msg_arg.locked = VIDEOGRAB_LOCKED_FIELDS;

    msg_arg.uflags = MSG_BLACK_OFF | MSG_GARBAGE_ON | MSG_FORMAT_FLT |
                         MSG_STDEV_OFF;

    msg_arg.avgframecount = fs_capture_frame_count;

    send_args_res = send_args_vgrab(&msg_arg);

    kjb_fflush((FILE*)NULL); /* Always flush after a videograb routine. */

    if (send_args_res == -1)
    {
        set_error("Startup of videograbber failed.");
        return ERROR;
    }

    if ( ! fs_video_active )
    {
        add_cleanup_function(stop_video_grabber_guts);
    }

    fs_video_active = TRUE;

    return NO_ERROR;
#else
    set_error("Video is not supported on this platform.");
    return ERROR;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_VIDEO
/*ARGSUSED*/
int capture_image
(
    KJB_image**   ipp,
    Image_window* __attribute__((unused)) dummy_image_window_ptr
)
{
    /*
    // FIX
    //
    // Either this should be static or freed !!!!
    // To free an XImage use XDestroyImage().
    // The null pointer to the Ximage means allocated.
    // The routines will recycle, so perhaps static makes most sense?
    */
    struct xistruct returned_image = { NULL, NULL, NULL, 0, 0, 0 };
    int          chk;
    int          locked;
    int          uflags;
    int          dflags;
    int          delay     = 0;
    int          immediate = 1;
    int          killchild = 0;
    Pixel*      out_pos;
    int               num_rows;
    int               num_cols;
    int               i;
    int               j;
    struct ret_image* returned_image_ptr;
    const int         argc                      = 1;
    char              zeroth_program_arg[ 100 ];
    char*             argv[ 1 ];
    static int        count;


    count = 0;

    ERE(start_video_grabber());

    BUFF_GET_PROGRAM_NAME(zeroth_program_arg);
    argv[ 0 ] = zeroth_program_arg;

    uflags = MSG_BLACK_OFF | MSG_GARBAGE_ON | MSG_FORMAT_FLT | MSG_STDEV_OFF;

    dflags = NO_RESIZE;

    locked = VIDEOGRAB_LOCKED_FIELDS;

    /*CONSTCOND*/
    while ( TRUE )
    {
        int res;

        /*
        // Use RESTART_AFTER_SIGNAL for easier debugging. Since a wet run has a
        // longjmp, whether or not we restart is irrelavent.
        */
        set_atn_trap(video_grabber_atn_fn, RESTART_AFTER_SIGNAL);

        if ((res = sigsetjmp(fs_before_video_routine_env, 1)) != 0)
        {
            unset_atn_trap();
            kjb_fflush((FILE*)NULL); /*Always flush after a videograb routine.*/

            if (res != INTERRUPTED)
            {
                set_bug("Unexpected return from sigsetjmp.");
                /* Don't return ERROR here. It will happend soon enough! */
            }

            terminate_video_grabber();
            set_error("Image capture interrupted.");
            add_error("Videograbber was explicitly killed.");
            return ERROR;
        }

        kjb_clear_error();

        chk = get_video_image( argc, argv, &returned_image,
                               fs_capture_frame_count,
                               (char*)NULL, /* Ask for  default outfile */
                               delay, uflags, dflags, locked,
                               immediate, killchild,
                               (char*)NULL /*blackfile-name*/
                             );

        kjb_fflush((FILE*)NULL);  /* Always flush after a videograb routine. */
        unset_atn_trap();

        if (chk == -1)
        {
            insert_error("Image capture failed.");
            return ERROR;
        }

        returned_image_ptr = returned_image.im;

        if (    (returned_image_ptr == NULL)
             || (returned_image_ptr->current != RI_FLOAT_CURRENT)
           )
        {
            set_bug("Problem with get_video_image.");
            return ERROR;
        }

        num_rows = returned_image_ptr->Rows;
        num_cols = returned_image_ptr->Cols;

        if (    (num_rows == STRIPPED_VIDEO_NUM_ROWS)
             && (num_cols == STRIPPED_VIDEO_NUM_COLS)
           )
        {
            break;
        }
        else if (count >= MAX_NUM_CAPTURE_TRIES)
        {
            set_error("Unexpected image size captured.");
            return ERROR;
        }
        else
        {
            verbose_pso(1, "Unexpected image size captured. Trying again.\n");
            count++;
        }
    }

    verbose_pso(5, "Image dimensions are %d rows by %d columns.\n",
                num_rows, num_cols);

    if (get_target_image(ipp, num_rows, num_cols) == ERROR)
    {
        free_ret_image(&(returned_image.im));
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        out_pos = (*ipp)->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {

            out_pos->r = (returned_image_ptr->fimdata)[ 0 ][ i ][ j ];
            out_pos->g = (returned_image_ptr->fimdata)[ 1 ][ i ][ j ];
            out_pos->b = (returned_image_ptr->fimdata)[ 2 ][ i ][ j ];

            out_pos->extra.invalid.pixel = VALID_PIXEL;
            out_pos->extra.invalid.r = VALID_PIXEL;
            out_pos->extra.invalid.g = VALID_PIXEL;
            out_pos->extra.invalid.b = VALID_PIXEL;

            out_pos++;
        }
    }

    free_ret_image(&(returned_image.im));
    kjb_fflush((FILE*)NULL);  /* Always flush after a videograb routine. */

    ERE(mark_clipped_pixels(*ipp));
    ERE(remove_camera_offset_from_image(*ipp));
    ERE(mark_dark_pixels(*ipp));

    return NO_ERROR;
}

#else  /* Case !KJB_HAVE_VIDEO follows. */

/*ARGSUSED*/
int capture_image
(
    KJB_image**   __attribute__((unused)) dummy_ipp,
    Image_window* __attribute__((unused)) dummy_image_window_ptr
)
{

    set_error("Video is not supported on this platform.");
    return ERROR;
}

#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int stop_video_grabber(void)
{
#ifdef KJB_HAVE_VIDEO

    stop_video_grabber_guts();
    return NO_ERROR;
#else
    set_error("Video is not supported on this platform.");
    return ERROR;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_VIDEO

static void stop_video_grabber_guts(void)
{
    if (fs_video_active)
    {
        kill_vgrab();
        kjb_fflush((FILE*)NULL);  /* Always flush after a videograb routine. */
        fs_video_active = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_VIDEO

/*ARGSUSED*/ /* Usually have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE video_grabber_atn_fn(TRAP_FN_DUMMY_ARGS)
{
    if (yes_no_query("Kill videograbber? (Y for yes, N for No) "))
    {
        siglongjmp(fs_before_video_routine_env, INTERRUPTED);
        /*NOTREACHED*/
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */



#ifdef __cplusplus
}
#endif

