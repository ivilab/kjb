
/* $Id: bttv_camera.c 21545 2017-07-23 21:57:31Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|
| * Justin Cappos            *
| * Camera code for project  *
| * Vision (577) Spring 2004 *
| * March 20th, 2004         *
|  and Kevin Wampler 2004...
|  and Kate Spriggs 2006
|                                                                              |
* =========================================================================== */

/*
 * This file and the corresponding .h file are example files for a kjb library
 * file.
 *
 * This file and the .h file should be renamed or copied to something sensible.
 * The two instances of the string MODULE_INCLUDED included in the .h file must
 * be changed to reflect the file name.
 *
 * Any file renaming. deletion, or changes to #include lines should be followed
 * by a "make depend" and a "make clean".
 *
 * The latest version of the build scripts should update the "_incl/h" file. You
 * may want to check that it reflects any .h adding or renaming.
 *
 */

/*Need this, otherwise there are problems with v4l2 errors*/
/*I am also pretty sure the includes should go in that order*/
/*todo: Check if the above two statements are true*/

#include "camera/camera_gen.h"
#include "camera/bttv_camera.h" 

#ifdef LINUX

#ifdef __STRICT_ANSI__
#define FOO__STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#include <asm/types.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>

/*This goes with the above*/

#ifdef FOO__STRICT_ANSI__
#define __STRICT_ANSI__
#undef FOO__STRICT_ANSI__
#endif

#include <linux/videodev.h>
/*for regular bw_byte_image*/

#ifdef KJB_HAVE_OPENGL
/*for displaying the images using opengl*/
#include <GL/glu.h>
#include <GL/glut.h>
#endif


#endif  /* End of case LINUX. */



/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* The size of the circular frame stream buffer */
#define FRAME_STREAM_LENGTH 60

#ifdef LINUX

/******************************* Local functions **************************/
static int internal_init_bttv_camera (Bttv_camera *camera);
static int internal_init_bttv_camera_info (Bttv_camera *camera);
static int free_bttv_camera_info (Bttv_camera_info *camera_struct);
static int do_bttv_channel_change (int channel, Bttv_camera *camera);

/* local functions for displaying images from the cameras */
static void display_cameras_opengl_CB(void);
static void reshape_cameras_opengl_CB(int width, int height);
static void key_cameras_opengl_CB(unsigned char key, int x, int y);

/* for ioctl errors */
static void add_ioctl_error(int errsv, const char * argument);

/******************************* VARIABLES **************************/

/* when no grabbers present, raise this so we don't try and free file descriptor
   that was never created*/
static int no_frame_grabbers = 0;

/* for ioctl errors, stores "errno"*/
static int errsv = 0;

#endif  /* End of case LINUX. */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            init_bttv_camera
 *
 * Inits a bttv camera
 *
 * This routine inits the internal structures needed to query the bttv camera
 * identified by camera_number, and sets the camera resolution to width and height.
 * The code looks for the camera device in 'device', usually being '/dev/videoN',
 * where N is an integer.  The device files should have the correct permissions set.
 * In general, it is better to initialize the cameras using init_bttv_camera_from_config_file
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Related:
 *    Bttv_camera, get_image_from_bttv_camera, display_cameras_opengl, init_bttv_camera_from_config_file
 *
 * Index:
 *     cameras 
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

int init_bttv_camera(Bttv_camera **my_camera, int width, int height, char* camera_id, char* machine, char* device, int usd)
{
#ifdef LINUX
    if(my_camera == NULL)
    {
        add_error("Dbl pointer to camera structure in init_bttv_camera is NULL!");
        /*kjb_print_error();*/
        return ERROR;
    }
    if(*my_camera != NULL)
    {
        kjb_free(*my_camera);
    }
    (*my_camera) = (Bttv_camera *)kjb_malloc(sizeof(Bttv_camera));

    strcpy((*my_camera)->camera_id, camera_id);
    strcpy((*my_camera)->host_machine, machine);
    strcpy((*my_camera)->device, device);

    (*my_camera)->width = width;
    (*my_camera)->height = height;

    (*my_camera)->camera_is_upside_down = usd;
    (*my_camera)->position_vp = NULL;
    (*my_camera)->camera_info = NULL;

    if(internal_init_bttv_camera(*my_camera) == ERROR)
    {
        return ERROR;
    }
    return NO_ERROR;

#else  /* Case no LINUX follows. */

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            init_bttv_camera_from_config_file
 *
 * Inits a bttv camera and reads camera info from a config file
 *
 * This routine calls "init_bttv_camera" to init the camera, using the 
 * parameters found in "file_name".
 * If unable to read in file, the routine doesn't initialize the camera,
 * and ERROR is returned.
 *
 * Related:
 *    Bttv_camera, get_image_from_bttv_camera, display_cameras_opengl, init_bttv_camera
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

int init_bttv_camera_from_config_file(Bttv_camera **camera, int width, int height, const char* file_name)
{
#ifdef LINUX
    FILE * config_fp = NULL;
    char line[100];
    char *token = NULL, *token2 = NULL, *token3 = NULL;
    char camera_id[128], machine[128], device[128];
    int upside_down = 1;

    config_fp = kjb_fopen(file_name, "r");
    if(config_fp == NULL)
    {
        warn_pso("Unable to open config file %s for reading. Camera not initialized.\n", file_name);
        return ERROR;
    }

    while(BUFF_FGET_LINE(config_fp,line) != EOF)
    {
      /* skip lines with first or second character # - comments */
        if(line[0] == '#' || line[1] == '#')
        {
            continue;
        }

        kjb_free(token3);
        token3 = kjb_strdup(line);

        token = strtok(line, "=");
        if(token == NULL)
        {
            continue;
        }
        token2 = strtok(token, " ");

        if(kjb_strcmp(token2, "id") == 0)
        {
            token2 = strtok(token3, "=");
            token2 = strtok(NULL, " ");
            strcpy(camera_id, token2);
        }
        else if(kjb_strcmp(token2, "machine") == 0)
        {
            token2 = strtok(token3, "=");
            token2 = strtok(NULL, " ");
            strcpy(machine, token2);
        }
        else if(kjb_strcmp(token2, "device") == 0)
        {
            token2 = strtok(token3, "=");
            token2 = strtok(NULL, " ");
            strcpy(device, token2);
        }
        else if(kjb_strcmp(token2, "images-location") == 0)
        {
            /*waiting on input from group for this field*/
        }
        else if(kjb_strcmp(token2, "up-side-down") == 0)
        {
            token2 = strtok(token3, "=");
            token2 = strtok(NULL, " ");
            if(kjb_strcmp(token2, "1") == 0)
            {
                upside_down = 1;
            }
        }
    }
    kjb_free(token3);
    kjb_fclose(config_fp);

    if(init_bttv_camera(camera, width, height, camera_id, machine, device, upside_down) == ERROR)
    {
        return ERROR;
    }

    return NO_ERROR;
#else  /* Case no LINUX follows. */

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef LINUX

static int internal_init_bttv_camera(Bttv_camera *camera)
{
    Bttv_camera_info *my_camera_struct;
    int i;
    if(camera == NULL)
    {
        add_error("cannot init in internal_initIRCamera since camera passed is NULL!");
        /*kjb_print_error();*/
        return ERROR;
    }
    if(camera->camera_info != NULL)
    {
        free_bttv_camera_info(camera->camera_info);
    }
  
    camera->camera_info = (Bttv_camera_info *)kjb_malloc(sizeof(Bttv_camera_info));
    my_camera_struct = camera->camera_info;

    my_camera_struct->v_filedesc = 0;
    my_camera_struct->v_map = NULL;
    my_camera_struct->frame_stream = (char *)kjb_malloc(FRAME_STREAM_LENGTH * camera->width * camera->height);
    my_camera_struct->cur_buf = 1;
    my_camera_struct->v_capability = NULL;
    for(i = 0; i < CAMERA_CHANNELS; i++) my_camera_struct->v_channel[i] = NULL;

    my_camera_struct->v_mbuf = NULL;
    my_camera_struct->v_picture = NULL;
    for(i = 0; i < CAMERA_BUFFERS; i++) my_camera_struct->v_mmap[i] = NULL;


    /* some complicated v4linux2 initialization stuff...*/
    if(internal_init_bttv_camera_info(camera) == ERROR) return ERROR;

    return NO_ERROR;
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_image_from_bttv_camera
 *
 * Gets one image from a bttv camera
 *
 * This routine queries the bttv camera and puts the image in the structure 
 * "image". The user should call init_bttv_camera(..) before querying. 
 * The cameras are black and white, so the image is a black and white byte 
 * image. 
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Related:
 *    init_bttv_camera, Bw_byte_image, Bttv_camera, get_image_from_bttv_camera_2,
 *    get_images_from_all_bttv_cameras
 *
 * Index:
 *     cameras 
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

int get_image_from_bttv_camera (Bw_byte_image * image, Bttv_camera *camera)
{
#ifdef LINUX
    char *p;
    static int n=0;
    Bttv_camera_info *this_camera = NULL;
    /* temp image for rotation upside down
    Bw_byte_image *camera_image_p = NULL; */

    if(camera != NULL)
    {
        this_camera = camera->camera_info;
        if(image == NULL)
        {
            /*kjb_add_error("KJB_MALLOCING image \n");*/
            ERE(get_target_bw_byte_image(&image, camera->height, camera->width));
        }
        this_camera->cur_buf=(this_camera->cur_buf+1)%(CAMERA_BUFFERS+1);

        /*There is a bug if this_camera->cur_buf is 0 - a sigfault after the program exists, in 
        glXChannelRectSyncSGIX () from /usr/lib/libGL.so.1 when using gdb... not sure why
        but this fixes it.*/
        if(this_camera->cur_buf == 0) this_camera->cur_buf++;

        if( ioctl(this_camera->v_filedesc, VIDIOCSYNC, (char *)&n)<0) 
        { 
            errsv = errno;
            add_ioctl_error(errsv, "VIDIOCSYNC");
            return ERROR;
        }
      
        /* Set the pointer to the mmaped area and then copy into the image buffer */
        p = (char *)this_camera->v_map + this_camera->v_mbuf->offsets[n];
        /*copy the memory contents into the user's image*/
        if( p != NULL)
        {
            kjb_memcpy(image->pixels[0], p, camera->width * camera->height);
        }
        else
        {
            add_error("Pointer to mmapped area of camera is NULL in get_image_from_bttv_camera().\n");
            return ERROR;
        }
    }
    else
    {
        add_error("Camera object is null in get_image_from_bttv_camera() \n");
        return ERROR;
    }

    /* Request the capture of the next frame in this buffer */
    if(ioctl(this_camera->v_filedesc, VIDIOCMCAPTURE, this_camera->v_mmap[n])<0)
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCMCAPTURE");
        return ERROR;
    }

    /* Do we need to rotate this image 180 degrees? Not here. The process that
       needs to put them right-side-up should do it. */
    /*if(camera->camera_is_upside_down == 1)
    {
        get_target_bw_byte_image(&camera_image_p, camera->height, camera->width);

        rotate_bw_byte_image(&camera_image_p, image);
        kjb_copy_bw_byte_image(&image, camera_image_p);

        kjb_free_bw_byte_image(camera_image_p);
    }*/


    n = (n+1)%CAMERA_BUFFERS;

    return NO_ERROR;

#else  /* Case no LINUX follows. */

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 
} 

/*****************************************************************************/


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_image_from_bttv_camera_2
 *
 * Gets one camera image from a bttv camera recording the timestamp and camera 
 * number
 *
 * This routine queries the bttv camera and puts the image in the structure 
 * "image->data". The user should call init_bttv_camera(..) before querying. 
 * The cameras are black and white, so the image is a black and white byte 
 * image. 
 * The "image" structure will also get the time of day in image->image_time
 * and the camera number from camera->camera_number in image->camera_number
 *
 * Related:
 *    init_bttv_camera, Bw_byte_image, Bttv_camera, get_image_from_bttv_camera,
 *    get_images_from_all_bttv_cameras
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

int get_image_from_bttv_camera_2 (Camera_bw_byte_image * image, Bttv_camera *camera)
{
#ifdef LINUX

    if(image != NULL)
    {
        get_image_from_bttv_camera(image->data, camera);
        /*set the timestamp*/
        gettimeofday(&image->image_time, NULL);
        /*and the camera number*/
        strcpy(image->camera_id, camera->camera_id);
    }
    else
    {
        add_error("Get_image_from_bttv_camera_2 called with image = NULL");
        return ERROR;
    }
    return NO_ERROR;

#else  /* Case no LINUX follows. */

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 
}
/*****************************************************************************/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_images_from_all_bttv_cameras
 *
 * Gets one image from all installed bttv cameras
 *
 * This routine queries all installed bttv cameras and puts the image from each 
 * in the array "image". The user should call init_bttv_camera(..) for each 
 * camera before querying. 
 *
 * The cameras are black and white, so the image is a black and white byte 
 * image. 
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Related:
 *    init_bttv_camera, Bw_byte_image, Bttv_camera, get_image_from_bttv_camera,
 *    get_image_from_bttv_camera_2
 *
 * Index:
 *     cameras 
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

int get_images_from_all_bttv_cameras(Bw_byte_image *image[], Bttv_camera *camera[], int num_cameras)
{
#ifdef LINUX
    int i;
    for (i = 0; i < num_cameras; i++)
    {
        ERE(get_image_from_bttv_camera(image[i], camera[i]));
    }
    return NO_ERROR;
#else

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 
}

int get_images_from_all_bttv_cameras_2(Camera_bw_byte_image * image[], Bttv_camera *camera[], int num_cameras)
{
#ifdef LINUX
    int i;
    for (i = 0; i < num_cameras; i++)
    {
        ERE(get_image_from_bttv_camera_2(image[i], camera[i]));
    }
    return NO_ERROR;
#else

    set_error("Bttv camera support is only available on linux.");
    return ERROR;

#endif 
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef LINUX

/*not sure what this does...*/
static int do_bttv_channel_change(int channel, Bttv_camera *camera) 
{
  Bttv_camera_info *this_camera = NULL;
  if(camera == NULL)
    {
      add_error("Error: camera is NULL in static do_bttv_channel_change()\n");
      return ERROR;
    }
  this_camera = camera->camera_info;
  if(ioctl(this_camera->v_filedesc, VIDIOCSCHAN, this_camera->v_channel[channel])<0) 
    {
      errsv = errno;
      add_ioctl_error(errsv, "VIDIOCSCHAN");
      return ERROR;
    }
  if(this_camera->v_channel[channel] == NULL)
    {
      add_error("Error: ioctl(VIDIOCSCHAN) returned null in camera->v_channel[channel].\n");
      return ERROR;
    }
  return NO_ERROR;
}

#endif  /* End of case LINUX. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_bttv_camera(Bttv_camera *camera)
{
#ifdef LINUX
  if(camera != NULL)
    {
      free_bttv_camera_info(camera->camera_info);
      free_vector(camera->position_vp);
      kjb_free(camera);
    }
#endif  /* End of case LINUX. */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef LINUX

static int free_bttv_camera_info(Bttv_camera_info *camera_struct)
{
    int i;
    if(camera_struct != NULL)
    {
        kjb_free(camera_struct->frame_stream);
        kjb_free(camera_struct->v_capability);
        kjb_free(camera_struct->v_picture);
        for(i = 0; i < CAMERA_CHANNELS; i++)
        {
            kjb_free(camera_struct->v_channel[i]);
        }
        for(i = 0; i < CAMERA_BUFFERS; i++)
        {
            kjb_free(camera_struct->v_mmap[i]);
        }
        if(camera_struct->v_map != NULL)
        {
            if(camera_struct->v_mbuf != NULL)
            {
                if(munmap(camera_struct->v_map, camera_struct->v_mbuf->size) != 0) 
                {
                    add_error("Error: error unmapping device. munmap in static free_bttv_camera_info returned != 0.\n");
                }
                kjb_free(camera_struct->v_mbuf);
            }
            else
            {
                kjb_free(camera_struct->v_map);
            }
        }
        if(no_frame_grabbers == 0)
        {
            if(close(camera_struct->v_filedesc) != 0)
            {
                add_error("Error: erorr closing device: camera->v_filedesc failed in static free_bttv_camera_info.\n");
                if (errno == EBADF) add_error("EBADF");
                if (errno == EINTR) add_error("EINTR");
                if (errno == EIO) add_error("EIO");
          
            }
        }
        kjb_free(camera_struct);
    }
    return NO_ERROR;
}
#endif  /* End of case LINUX. */

#ifdef LINUX
static void add_ioctl_error(int errsv, const char * argument)
{
    add_error("ioctl with argument %s returned an error.\n", argument);
    if(errsv == EBADF) add_error("ioctl error: d is not a valid descriptor.\n");
    if(errsv ==  EFAULT) add_error("ioctl error: argp references an inaccessible memory area.\n");
    if(errsv ==  EINVAL) add_error("ioctl error: Request or argp is not valid.\n");
    if(errsv ==  ENOTTY) add_error("ioctl error: d is not associated with a character special device.\n");
    if(errsv ==  ENOTTY) add_error("ioctl error: The specified request does not apply to the kind of object that the descriptor d references.\n");

  }
#endif  /* End of case LINUX. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef LINUX
static int internal_init_bttv_camera_info(Bttv_camera *camera) 
{
    int n = 0;
    char cameradevstring[20];
    Bttv_camera_info *this_camera = NULL;

    if(camera == NULL)
    {
        add_error("camera parameter in internal_init_bttv_camera is NULL, cannot init\n");
        return ERROR;
    }
    this_camera = camera->camera_info;
    sprintf(cameradevstring,"/dev/%s", camera->device);
    if((this_camera->v_filedesc = open(cameradevstring, O_RDWR)) < 0) 
    {
        /*perror("open");*/

        add_error("Error opening file /dev/%s, will try to use /dev/my%s.\n", camera->device, camera->device);

        sprintf(cameradevstring,"/dev/my%s",camera->device);
        if((this_camera->v_filedesc = open(cameradevstring, O_RDWR)) < 0) 
        {
            add_error("Error opening file /dev/my%s.", camera->device);
            add_error("You don't seem to have any frame grabbers installed on your system,\nor you don't have the correct permissions to /dev/videoN or /dev/myvideoN.\n");
            no_frame_grabbers = 1;
            return ERROR;
        }
    }

    if (this_camera->v_capability == NULL)
    {
        this_camera->v_capability = (struct video_capability *)kjb_malloc(sizeof(struct video_capability));
        if(this_camera->v_capability == NULL)
        {
            add_error("this_camera->v_capability NULL after malloc in static internal_init_bttv_camera_info()\n");
            return ERROR;
        }
    }
    if(ioctl(this_camera->v_filedesc, VIDIOCGCAP, this_camera->v_capability)<0) 
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCGCAP");
        return ERROR;
    }

    if(camera->width > this_camera->v_capability->maxwidth) camera->width=this_camera->v_capability->maxwidth;
    if(camera->width < this_camera->v_capability->minwidth) camera->width=this_camera->v_capability->minwidth;
    if(camera->height > this_camera->v_capability->maxheight) camera->height=this_camera->v_capability->maxheight;
    if(camera->height < this_camera->v_capability->minheight) camera->height=this_camera->v_capability->minheight;

    for(n = 0 ; n < this_camera->v_capability->channels ; n++) 
    {
        if(this_camera->v_channel[n] == NULL)
        {
            this_camera->v_channel[n] = (struct video_channel *)kjb_malloc(sizeof(struct video_channel));
            if (this_camera->v_channel[n]  == NULL)
            {
                add_error("this_camera->v_channel[n] NULL after malloc in static internal_init_bttv_camera_info()\n");
                return ERROR;
            }
        }
        this_camera->v_channel[n]->channel = n;
        this_camera->v_channel[n]->norm = VIDEO_MODE_NTSC; /* 0:PAL 1:NTSC 2:SECAM */

        if(ioctl(this_camera->v_filedesc, VIDIOCSCHAN, this_camera->v_channel[n])<0) 
        {
            errsv = errno;
            add_ioctl_error(errsv, "VIDIOCSCHAN");
            return ERROR;
        }
    }

    if(this_camera->v_picture == NULL)
    {
        this_camera->v_picture = (struct video_picture *)kjb_malloc(sizeof(struct video_picture));
        if (this_camera->v_picture  == NULL)
        {
            add_error("this_camera->v_picture NULL after malloc in static internal_init_bttv_camera_info()\n");
            return ERROR;
        }
    }
    /* Set the attributes of the camera */
    this_camera->v_picture->brightness = 32767;
    this_camera->v_picture->hue        = 32767;
    this_camera->v_picture->colour     = 32767;
    this_camera->v_picture->contrast   = 32767;
    this_camera->v_picture->whiteness  = 32767;
    this_camera->v_picture->depth      = 8; /* color depth was 42, and it was complaining, so changed to 8 --kate*/ 
    this_camera->v_picture->palette    = VIDEO_PALETTE_GREY; /* palette = grey scale */

    if(ioctl(this_camera->v_filedesc, VIDIOCSPICT, this_camera->v_picture) < 0) 
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCSPICT");  
        return ERROR;
    }

    if(this_camera->v_mbuf == NULL)
    {
        this_camera->v_mbuf = (struct video_mbuf *)kjb_malloc(sizeof(struct video_mbuf));
        if (this_camera->v_mbuf  == NULL)
        {
            add_error("this_camera->v_mbuf NULL after malloc in static internal_init_bttv_camera_info()\n");
            return ERROR;
        }
    }

    if(ioctl(this_camera->v_filedesc, VIDIOCGMBUF, (this_camera->v_mbuf)) < 0) 
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCGMBUF");  
        return ERROR;
    }
    this_camera->v_map = (char *)mmap(0, this_camera->v_mbuf->size, PROT_READ|PROT_WRITE, MAP_SHARED, this_camera->v_filedesc, 0);
    if((char *)this_camera->v_map == MAP_FAILED)
    {
        perror("mmap");
        return ERROR;
    }

    /*make this loop to go for the camera buffers of the currently selected camera*/
    for (n=0; n<CAMERA_BUFFERS; n++) 
    {
        if(this_camera->v_mmap[n] == NULL)
        {
            this_camera->v_mmap[n] = (struct video_mmap *)kjb_malloc(sizeof(struct video_mmap));
            if (this_camera->v_mmap[n]  == NULL)
            {
                add_error("this_camera->v_mmap[n] NULL after malloc in static internal_init_bttv_camera_info()\n");
                return ERROR;
            }
        }
        this_camera->v_mmap[n]->frame  = n;
        this_camera->v_mmap[n]->width  = camera->width;
        this_camera->v_mmap[n]->height = camera->height;
        this_camera->v_mmap[n]->format = VIDEO_PALETTE_GREY;
        if(ioctl(this_camera->v_filedesc, VIDIOCMCAPTURE, (this_camera->v_mmap[n]))<0) 
        {
            errsv = errno;
            add_ioctl_error(errsv, "VIDIOCMCAPTURE");
            return ERROR;
        }
    }
    /*channel, camera_number*/
    do_bttv_channel_change(0,camera);
    n=0;
    if(ioctl(this_camera->v_filedesc, VIDIOCSYNC, &n)<0) 
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCSYNC");
        return ERROR;
    }
  
    /* Request the capture of the next frame in this buffer */
    if(ioctl(this_camera->v_filedesc, VIDIOCMCAPTURE, (this_camera->v_mmap[n]))<0) 
    {
        errsv = errno;
        add_ioctl_error(errsv, "VIDIOCMCAPTURE");      
        return ERROR;
    }
    return NO_ERROR;
}
#endif  /* End of case LINUX. */

/* Global variables needed to display cameras using openGL. */
static Bttv_camera** all_cameras;
static Camera_bw_byte_image* all_images[MAXIMUM_NUMBER_OF_CAMERAS] = {NULL, NULL, NULL, NULL};
static int number_of_cameras;


 /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            display_cameras_opengl
 *
 * Displays images from all cameras
 *
 * Uses OpenGL with glut to display images from all cameras. Uses callbacks.
 * 
 *
 * Returns:
 *    On error, this routine returns ERROR, otherwise, NO_ERROR.
 *
 * Related:
 *    Bttv_camera, init_bttv_camera, get_image_from_bttv_camera
 *
 * Index:
 *     cameras 
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
*/

void display_cameras_opengl(int argc, char **argv, int my_width, int my_height, Bttv_camera *all_cameras_in[MAXIMUM_NUMBER_OF_CAMERAS], int num_cams)
{
#ifdef LINUX
#ifdef KJB_HAVE_OPENGL
    int i;

    all_cameras = all_cameras_in;
    number_of_cameras = num_cams;
    for(i = 0; i < number_of_cameras; i++)
    {
        if(get_target_camera_bw_byte_image(&all_images[i], my_height, my_width) == ERROR)
        {
            kjb_print_error();
            kjb_exit(EXIT_FAILURE);
        }
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(2*my_width, 2*my_height);

    glutInitWindowPosition(100, 100);
    glutCreateWindow("Cameras");
    
    /*----------Set basic view properties and callbacks----------*/
    glutDisplayFunc(display_cameras_opengl_CB);
    glutIdleFunc(display_cameras_opengl_CB);
    glutReshapeFunc(reshape_cameras_opengl_CB);
    glutKeyboardFunc(key_cameras_opengl_CB);

    glutMainLoop();

#else
    add_error("display_cameras_opengl(..) uses OPENGL and it doesn't seem you have OPENGL on your system.\n ");
    kjb_print_error();
#endif

#endif    
}

#ifdef LINUX
#ifdef KJB_HAVE_OPENGL
static void display_cameras_opengl_CB(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    if(get_images_from_all_bttv_cameras_2(all_images, all_cameras, number_of_cameras) == ERROR)
    {
        kjb_print_error();
        kjb_exit(EXIT_FAILURE);
    }

    /* We need to flip the images, that is why we use glPixelZoom */
    if(all_images[0] != NULL)
    {
        glRasterPos2i(0,1);
        glPixelZoom(-1.0,-1.0);
        glDrawPixels(all_images[0]->data->num_cols, all_images[0]->data->num_rows, 
            GL_LUMINANCE, GL_UNSIGNED_BYTE, *all_images[0]->data->pixels);
    }

    if(all_images[1] != NULL)
    {
        glRasterPos2i(1,1);
        glPixelZoom(-1.0, -1.0);
        glDrawPixels(all_images[1]->data->num_cols, all_images[1]->data->num_rows, 
            GL_LUMINANCE, GL_UNSIGNED_BYTE, *all_images[1]->data->pixels);
    }
    if(all_images[2] != NULL)
    {
        glRasterPos2i(0,0);
        glPixelZoom(-1.0, -1.0);
        glDrawPixels(all_images[2]->data->num_cols, all_images[2]->data->num_rows, 
            GL_LUMINANCE, GL_UNSIGNED_BYTE, *all_images[2]->data->pixels);
    }  
    if(all_images[3] != NULL)
    {
        glRasterPos2i(1,0);
        glPixelZoom(-1.0, -1.0);
        glDrawPixels(all_images[3]->data->num_cols, all_images[3]->data->num_rows, 
            GL_LUMINANCE, GL_UNSIGNED_BYTE, *all_images[3]->data->pixels);
    }

    glFlush();
    glutSwapBuffers();
}
#endif /*case KJB_HAVE_OPENGL*/
#endif  /* End of case LINUX. */

#ifdef LINUX
#ifdef KJB_HAVE_OPENGL
static void reshape_cameras_opengl_CB(int width, int height)
{
    glViewport(0,0, (GLsizei)width, (GLsizei)height);
    glLoadIdentity();
    gluOrtho2D(0,0,width+1,height+1);  
}
#endif /*case KJB_HAVE_OPENGL*/
#endif  /* End of case LINUX. */

#ifdef LINUX
#ifdef KJB_HAVE_OPENGL
static void key_cameras_opengl_CB(unsigned char key, int x, int y)
{
    int i;
    switch(key)
    {
        case 'q':
        case 'Q':
            for(i = 0; i < MAXIMUM_NUMBER_OF_CAMERAS; i++)
            {
                free_bttv_camera(all_cameras[i]);
                kjb_free_camera_bw_byte_image(all_images[i]);
            }
            exit(0);
            break;
        default:
            break;
    }
}
#endif /*case KJB_HAVE_OPENGL*/
#endif  /* End of case LINUX. */


#ifdef __cplusplus
}
#endif


