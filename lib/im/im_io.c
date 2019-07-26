
/* $Id: im_io.c 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__

/*
   Copyright (c) 1994-2008 by Kobus Barnard (author).

   Personal and educational use of this code is granted, provided
   that this header is kept intact, and that the authorship is not
   misrepresented. Commercial use is not permitted.
*/


/*
// Note: Most of the code in this module will not work in the case
//       of non-contiguous arrays. However, leave it be (at least until the
//       the ImageMagick stuff has been moved to IM).
*/

#include "im/im_gen.h"

/*
// Kobus--moved this to im_gen.h to faciliate automatic determination of
// required archives.
//
#include "magick.h"
//
*/

#include "im/im_display.h" 

#include "im/im_io.h"

#include <sys/types.h>

#ifdef KJB_HAVE_X11
#    include <sys/wait.h>
#endif 


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_DISPLAYED_IMAGES  1000

static int num_fork_displayed_images = 0;
static int fork_display_process_pids[ MAX_NUM_DISPLAYED_IMAGES ];
static int im_client_name_set = FALSE;


/*
 * =============================================================================
 *
 * Routine to replace the ImageMagick warning message printing routine.
 *
 * -----------------------------------------------------------------------------
*/

void im_warning_handler(char* message, char* qualifier)
{


    if (qualifier == NULL)
    {
        set_error("ImageMagick says \"%s\".", message);
    }
    else
    {
        set_error("ImageMagick says \"%s (%s)\".", message, qualifier);
    }

}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef KJB_HAVE_X11

Byte_image* im_read_byte_image
(
    Byte_image*      ip,
    char*            file_name_and_sub_image,
    Image_file_info* image_file_info_ptr
)
{
    extern int   im_client_name_set;
    extern char* client_name;
    ImageInfo    image_info;
    Image*       im_image;
    Byte_pixel*  out_pos;
    char*        im_image_pos;
    int          num_rows, num_cols, i, j;
    char*        file_name_and_sub_image_pos;
    char         file_name[ MAX_FILE_NAME_SIZE ];
    char         sub_image[ MAX_FILE_NAME_SIZE ];
    char         expanded_file_name[ MAX_FILE_NAME_SIZE ];
    char         expanded_file_name_and_sub_image[ MAX_FILE_NAME_SIZE ];


    if (im_client_name_set)
    {
       /*
        * client_name is an ImageMagick global used for error reporting.
        * It is NOT initialized in the ImageMagick code (bad IMHO). Therefore,
        * we are obliged to ensure that it points to something (otherwise
        * suffer segmentation faults if there is an error, such as trying
        * to read an image from a mispelled file name). To avoid the crash
        * we will just initialize it here. It can be set to whatever one wants
        * the ImageMagick error prefix to be. The standard ImageMagick way
        * of doing it is as follows. (No double problem, but there is something
        * about setting a global pointer to something in the stack that bugs
        * me!).
        *
        * (To use client_name the same way as ImageMagick programsi, make the
        * following line the first line in main).
        *
        *           client_name=ClientName(*argv);
        */

        client_name = kjb_strdup("ImageMagick");
        im_client_name_set = TRUE;
    }

   /*
    * Let's be sure that if an error has been set, then
    * an ImageMagick routine set it (via im_warning_handler).
    */

    kjb_clear_error();

    /*
    ** Preempt ImageMagick printing of recoverable problems. Note that
    ** since all code uses the same routine, we do not need to reset the
    ** handler, or otherwise clean-up.
    **
    ** We could do a similar thing for the error handler for
    ** non-recoverable errors (SetErrorHandler), but since a call to exit
    ** is assumed by their use, we would have to handle the return
    ** (via a set_jmp baack to a routine in this file), so that ImageMagick
    ** code would not continue under conditions where it would normally
    ** have exited. Since we would need such a set_jmp for each affected
    ** ImageMagick routine, leave the default error handler in place for
    ** now.
    */
    SetWarningHandler(im_warning_handler);

    GetImageInfo(&image_info);

    if (file_name_and_sub_image == NULL)
    {
        strcpy(image_info.filename, "-");
    }
    else
    {
        file_name_and_sub_image_pos = file_name_and_sub_image;

        /* We get an error message here because ImageMagick undef's const in
        // magick.h if __STDC__ is not TRUE, but it could be defined as 0 and
        // we could still want const!
        */
        BUFF_GEN_GET_TOKEN(&file_name_and_sub_image_pos, file_name, "[");
        BUFF_CPY(sub_image, file_name_and_sub_image_pos);

        ERN(BUFF_REAL_PATH(file_name, expanded_file_name));

        BUFF_CPY(expanded_file_name_and_sub_image, expanded_file_name);
        BUFF_CAT(expanded_file_name_and_sub_image, sub_image);

        strcpy(image_info.filename, expanded_file_name_and_sub_image);
    }

    im_image = ReadImage(&image_info);

    if (im_image == NULL)
    {
        /*
         *  The call to the ImageMagick routine may have set the error on
         *  failure (since this has been arranged in certain circumstances),
         *  or it may still be unset due to the call to kjb_clear_error
         *  above. Insert_error covers both possibilities.
         */

        if (file_name_and_sub_image == NULL)
        {
            insert_error("ImageMagick is unable to read image from stdin.");
        }
        else
        {
            insert_error("ImageMagick is unable to read image from %s.",
                         file_name_and_sub_image);
        }

         return NULL;
     }

    if (image_file_info_ptr != NULL)
    {
        BUFF_CPY(image_file_info_ptr->magick, im_image->magick);
    }

    im_image->compression = NoCompression;
    im_image->class       = DirectClass;

    if ( ! RunlengthEncodeImage(im_image))
    {
        /*
         *  The call to the ImageMagick routine may have set the error on
         *  failure (since this has been arranged in certain circumstances),
         *  or it may still be unset due to the call to kjb_clear_error
         *  above. Insert_error covers both possibilities.
         */

        insert_error("ImageMagick can't \"encode\" image.");
        DestroyImage(im_image);
        return NULL;
    }

    num_rows = im_image->rows;
    num_cols = im_image->columns;

    if (ip != NULL)
    {
        if (    (ip->num_rows != num_rows)
             || (ip->num_cols != num_cols)
           )
        {
            free_byte_image(ip);
            ip = NULL;
        }
    }

    if (ip == NULL)
    {
        ip = create_byte_image(num_rows, num_cols);
    }

    if (ip == NULL)
    {
        DestroyImage(im_image);
        return NULL;
    }

    im_image_pos = (char*)im_image->packed_pixels;

    if ((im_image->packet_size != 3) && (im_image->packet_size != 4))
    {
        set_bug("Unexpected image format in im_read_byte_image.");
        free_byte_image(ip);
        DestroyImage(im_image);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        out_pos = ip->pixels[ i ];

        if (im_image->packet_size == 3)
        {
            for (j=0; j<num_cols; j++)
            {
                out_pos->r = *im_image_pos++;
                out_pos->g = *im_image_pos++;
                out_pos->b = *im_image_pos++;
                out_pos++;
            }
        }
        else if (im_image->packet_size == 4)
        {
            for (j=0; j<num_cols; j++)
            {
                out_pos->r = *im_image_pos++;
                out_pos->g = *im_image_pos++;
                out_pos->b = *im_image_pos++;
                out_pos++;
                im_image_pos++;
            }
        }
    }

    DestroyImage(im_image);

    return ip;
}

#else

Byte_image* im_read_byte_image
(
    Byte_image*      dummy_ip,
    char*            dummy_file_name_and_sub_image,
    Image_file_info* dummy_image_file_info_ptr
)
{
    set_error("Routine im_read_byte_image() needs to be compiled with X11.");
    return NULL;
}

#endif   /* KJB_HAVE_X11  */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef KJB_HAVE_X11

int im_write_byte_image
(
    Byte_image*      ip,
    char*            file_name,
    Image_file_info* image_file_info_ptr
)
{
    extern int im_client_name_set;
    extern char *client_name;
    ImageInfo image_info;
    Image *im_image;
    int num_rows, num_cols;
    int status;
    int packet_size;
    long num_packets;
    long num_bytes;


    if (im_client_name_set)
    {
        /*
        ** client_name is an ImageMagick global used for error reporting.
        ** It is NOT initialized in the ImageMagick code (bad IMHO). Therefore,
        ** we are obliged to ensure that it points to something (otherwise
        ** suffer segmentation faults if there is an error, such as trying
        ** to read an image from a mispelled file name). To avoid the crash
        ** we will just initialize it here. It can be set to whatever one wants
        ** the ImageMagick error prefix to be. The standard ImageMagick way
        ** of doing it is as follows. (No double problem, but there is something
        ** about setting a global pointer to something in the stack that bugs
        ** me!).
        **
        ** (To use client_name the same way as ImageMagick programsi, make the
        ** following line the first line in main).
        **
        **           client_name=ClientName(*argv);
        */

        client_name = kjb_strdup("ImageMagick");
        im_client_name_set = TRUE;
    }
   /*
    * Let's be sure that if an error has been set,
    * then an ImageMagick routine set it (via im_warning_handler).
    */

    kjb_clear_error();

    /*
    ** Preempt ImageMagick printing of recoverable problems. Note that since
    ** all code uses the same warning handler routine, we do not need to reset
    ** the handler, or otherwise clean-up.
    **
    ** We could do a similar thing for the error handler for non-recoverable
    ** errors (SetErrorHandler), but since a call to exit is assumed by their
    ** use, we would have to handle the return (via a set_jmp baack to a
    ** routine in this file), so that ImageMagick code would not continue under
    ** conditions where it would normally have exited. Since we would need such
    ** a set_jmp for each affected ImageMagick routine, leave the default error
    ** handler in place for now.
    */

    SetWarningHandler(im_warning_handler);

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    GetImageInfo(&image_info);

    if (file_name == NULL)
    {
        strcpy(image_info.filename, "-");
    }
    else
    {
        strcpy(image_info.filename, file_name);
    }

    if (image_file_info_ptr != NULL)
    {
        strcpy(image_info.magick, image_file_info_ptr->magick);
    }
    else
    {
        strcpy(image_info.magick, "MIFF");
    }

    im_image = AllocateImage(&image_info);

    if (im_image == NULL)
    {
        /*
         *  The call to the ImageMagick routine may have set the error on
         *  failure (since this has been arranged in certain circumstances),
         *  or it may still be unset due to the call to kjb_clear_error
         *  above. Insert_error covers both possibilities.
         */

        insert_error("Unable to allocate ImageMagick image.");
        return ERROR;
    }

    im_image->columns = (unsigned int) num_cols;
    im_image->rows    = (unsigned int) num_rows;

    packet_size = 3;
    im_image->packet_size = packet_size;

    num_packets = ((long)(im_image->columns)) * ((long)(im_image->rows));
    im_image->packets = num_packets;

    num_bytes = num_packets * packet_size;

    im_image->packed_pixels = BYTE_MALLOC(num_bytes);

    if (im_image->packed_pixels == NULL)
    {
        DestroyImages(im_image);
        return ERROR;
    }

    kjb_memcpy((char*)im_image->packed_pixels, ip->pixels[ 0 ], num_bytes);

    im_image->class = DirectClass;
    im_image->compression = NoCompression;

    status = RunlengthDecodeImage(im_image);

    if (status == False)
    {
        /*
         *  The call to the ImageMagick routine may have set the error on
         *  failure (since this has been arranged in certain circumstances),
         *  or it may still be unset due to the call to kjb_clear_error
         *  above. Insert_error covers both possibilities.
         */

        insert_error("ImageMagick can't \"decode\" image.");

        /*
         *   For easier detection of memory leaks, deallocate all that has
         *   been allocated in this module, even though DestroyImages
         *   will free them (if they are not null).
         */

        kjb_free(im_image->packed_pixels);
        im_image->packed_pixels = NULL;
        DestroyImages(im_image);

        return ERROR;
    }

    status = WriteImage(&image_info, im_image);

    if (status == False)
    {
        /*
         *  The call to the ImageMagick routine may have set the error on
         *  failure (since this has been arranged in certain circumstances),
         *  or it may still be unset due to the call to kjb_clear_error
         *  above. Insert_error covers both possibilities.
         */

        insert_error("Unable to write image.");

        /*
         *   For easier detection of memory leaks, deallocate all that has
         *   been allocated in this module, even though DestroyImages
         *   will free them (if they are not null.
         */

        kjb_free(im_image->packed_pixels);
        im_image->packed_pixels = NULL;
        DestroyImages(im_image);

        return ERROR;
    }

    /*
       For easier detection of memory leaks, deallocate all that has
       been allocated in this module, even though DestroyImages
       will free them (if they are not null.
    */
    kjb_free(im_image->packed_pixels);
    im_image->packed_pixels = NULL;
    DestroyImages(im_image);

    return NO_ERROR;
}

#else

/*ARGSUSED*/
int im_write_byte_image
(
    Byte_image*      dummy_ip,
    char*            dummy_file_name,
    Image_file_info* dummy_image_file_info_ptr
)
{
    set_error("Routine im_write_byte_image() needs to be compiled with X11.");
    return ERROR;
}

#endif   /* KJB_HAVE_X11  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef KJB_HAVE_X11

/*ARGSUSED*/
IM_displayed_image* im_display_byte_image
(
    Byte_image* ip,
    char*       dummy_title,
    int         pipe_needed
)
{
    extern int            num_fork_displayed_images;
    extern int            fork_display_process_pids[ MAX_NUM_DISPLAYED_IMAGES ];
    ImageInfo             image_info;
    Image                *im_image;
    int                   num_rows, num_cols;
    int                   status;
    int                   packet_size;
    long                  num_packets;
    long                  num_bytes;
    int                   display_pid;
    int                   im_pipe[2];
    IM_displayed_image *displayed_image_ptr;


    if (num_fork_displayed_images >= MAX_NUM_DISPLAYED_IMAGES)
    {
        set_error("Too many images to display.");
        return NULL;
    }

    if (pipe_needed)
    {
        if (pipe(im_pipe) == EOF)
        {
            set_error("Creation of pipe to ImageMagick process failed.");
            return NULL;
        }
    }
    else
    {
        im_pipe[ READ_END ] = NOT_SET;
        im_pipe[ WRITE_END ] = NOT_SET;
    }

    display_pid = kjb_fork();

    if (display_pid < 0)
    {
        return NULL;
    }
    else if (IS_PARENT(display_pid))
    {
        NRN(displayed_image_ptr = TYPE_MALLOC(IM_displayed_image));

        displayed_image_ptr->pid = display_pid;

        if (pipe_needed)
        {
            close(im_pipe[WRITE_END]);
            displayed_image_ptr->read_des = im_pipe[ READ_END ];
        }
        else
        {
            displayed_image_ptr->read_des = NOT_SET;
        }

        fork_display_process_pids[ num_fork_displayed_images ] = display_pid;
        num_fork_displayed_images++;

        return displayed_image_ptr;
    }

    if (pipe_needed)
    {
        close(im_pipe[READ_END]);
        dup2(im_pipe[WRITE_END], fileno(stdout));
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    GetImageInfo(&image_info);

    /* Default, possibly to be overridden. */
    strcpy(image_info.filename, "dummy");

    strcpy(image_info.magick, "MIFF");

    im_image = AllocateImage(&image_info);

    if (im_image == NULL)
    {
        kjb_fprintf(stderr, "Unable to allocate ImageMagick image.\n");
        kjb_print_error();
        kjb_exit( EXIT_FAILURE );
    }

    im_image->columns = (unsigned int) num_cols;
    im_image->rows    = (unsigned int) num_rows;

    packet_size = 3;
    im_image->packet_size=packet_size;

    num_packets = im_image->columns*im_image->rows;
    im_image->packets = num_packets;

    num_bytes = num_packets*packet_size;

    im_image->packed_pixels = BYTE_MALLOC(num_bytes);

    if (im_image->packed_pixels == NULL)
    {
        kjb_print_error();
        DestroyImages(im_image);
        kjb_exit( EXIT_FAILURE );
    }

    kjb_memcpy(im_image->packed_pixels, ip->pixels[ 0 ], num_bytes);

    im_image->class = DirectClass;
    im_image->compression = NoCompression;

    status = RunlengthDecodeImage(im_image);

    if (status == False)
    {
        kjb_fprintf(stderr, "Unable to decode ImageMagick image.\n");
        kjb_print_error();

        DestroyImages(im_image);

        kjb_exit( EXIT_FAILURE );
    }

    im_display(im_image, &image_info);

    DestroyImages(im_image);

    kjb_exit ( EXIT_SUCCESS );

    /*NOTREACHED*/

#ifdef __GNUC__
    return NULL;
#endif
}

#else

/*ARGSUSED*/
IM_displayed_image* im_display_byte_image
(
    Byte_image* dummy_ip,
    char*       dummy_title,
    int         dummy_pipe_needed
)
{
    set_error("Routine im_display_byte_image() needs to be compiled with X11.");
    return NULL;
}

#endif   /* KJB_HAVE_X11  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef KJB_HAVE_X11

int im_fork_close(IM_displayed_image* displayed_image_ptr)
{
    int pid;

    pid = displayed_image_ptr->pid;

    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);

    kjb_free(displayed_image_ptr);

    return NO_ERROR;
}

#else

/*ARGSUSED*/
int im_fork_close(IM_displayed_image* dummy_displayed_image_ptr)
{
    set_error("Routine im_fork_close() needs to be compiled with X11.");
    return ERROR;
}

#endif   /* KJB_HAVE_X11  */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef KJB_HAVE_X11

int im_fork_close_all(void)
{
    extern int num_fork_displayed_images;
    extern int fork_display_process_pids[ MAX_NUM_DISPLAYED_IMAGES ];
    int i;


    for (i=0; i<num_fork_displayed_images; i++)
    {
        kill(fork_display_process_pids[i], SIGTERM);
        waitpid(fork_display_process_pids[i], NULL, 0);
    }

    return NO_ERROR;
}

#else

/*ARGSUSED*/
int im_fork_close_all(void)
{
    set_error("Routine im_fork_close_all() needs to be compiled with X11.");
    return ERROR;
}

#endif   /* KJB_HAVE_X11  */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

#endif   /* #ifndef __C2MAN__ */

