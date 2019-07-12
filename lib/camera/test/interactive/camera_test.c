
/* $Id: camera_test.c 21491 2017-07-20 13:19:02Z kobus $ */

#include "l/l_init.h"
#include "camera/camera_bw_byte_image.h"
#include "camera/camera_bw_byte_image_io.h"
#include "camera/bttv_camera.h"


/*to format time from gettimeofday..*/
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>



/*example of how to get all four images at the same time, mostly*/
static Bttv_camera *all_cameras[MAXIMUM_NUMBER_OF_CAMERAS];
static Camera_bw_byte_image *all_images[MAXIMUM_NUMBER_OF_CAMERAS];

int main(int argc, char **argv)
{

#ifndef PROGRAMMER_IS_kobus
    /* FIXME */
    /* This is broken, but Kobus needs to test compiling! */

    int i,j, display_images = 1;
    char temp_string[40];
    int my_width = 320;
    int my_height = 240;

    DIR * my_directory;
    char image_save_path [40];
    mode_t file_mode = 0;

    kjb_init();

    /* Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    /* init */
    for (i = 0; i < MAXIMUM_NUMBER_OF_CAMERAS; i++)
    {
        all_cameras[i] = NULL;
        /*if you just want a bbtv_camera, use the next line; if you want to read from a
          config file, use init_bttv_camera_from_config_file(). */
        /*if(init_bttv_camera(&all_cameras[i], i, my_width, my_height) == ERROR)*/
        char curfile[1024];
        sprintf(curfile, "cam%d_info.txt", i);
        if(init_bttv_camera_from_config_file(&all_cameras[i], /*i,*/ my_width, my_height, 
                                             curfile)== ERROR)
        {
            kjb_print_error();
            for(j = 0; j <= i; j++)
            {
                free_bttv_camera(all_cameras[j]);
            }
            kjb_exit(EXIT_FAILURE);
        }

        /*init image structures*/
        all_images[i] = NULL;
        if(get_target_camera_bw_byte_image(&all_images[i], my_height, my_width) == ERROR)
        {
            kjb_print_error();
            kjb_exit(EXIT_FAILURE);
        }
    }


    /*if you are interested in the positions of the cameras,
      you would want to have called init_bttv_camera_from_config_file(..)
      so that the positions can be read from the config file.*/
    for(i= 0; i < MAXIMUM_NUMBER_OF_CAMERAS; i++)
    {
        if(all_cameras[i]->position_vp != NULL)
        {
            if(all_cameras[i]->position_vp->elements[0] != -1 &&
               all_cameras[i]->position_vp->elements[1] != -1 &&
               all_cameras[i]->position_vp->elements[2] != -1)
            {
                printf("Camera %d position at: [%f, %f, %f].\n", i, 
                       all_cameras[i]->position_vp->elements[0],
                       all_cameras[i]->position_vp->elements[1],
                       all_cameras[i]->position_vp->elements[2]);
            }
            else
            {
                printf("Camera %d position unknown.\n", i);
            }
        }
    }


    if(display_images)
    {
        display_cameras_opengl(argc, argv, my_width, my_height,
                               /*all_images,*/ all_cameras, MAXIMUM_NUMBER_OF_CAMERAS );
        for(i = 0; i < MAXIMUM_NUMBER_OF_CAMERAS; i++)
        {
            free_bttv_camera(all_cameras[i]);
            kjb_free_camera_bw_byte_image(all_images[i]);
        }

        return EXIT_FAILURE;

    }
    else
    {
        /* query for images */
        /* uncomment next if getting images from one camera at a time
           if(get_images_from_all_bttv_cameras_2(all_images, all_cameras) == ERROR)
           {
           kjb_print_error();
           kjb_exit(EXIT_FAILURE);
           }
           */

        /*create a directory to write sample images in*/ 
        sprintf(image_save_path, "images");
        my_directory = opendir(image_save_path);
        if(my_directory == NULL)
        {
            /*create the directory*/
            file_mode = (file_mode | S_IRUSR) | (S_IXUSR | S_IWUSR);
            if(mkdir(image_save_path, file_mode) == -1)
            {
                printf("error creating directory %s. exiting \n", image_save_path);
                kjb_exit(EXIT_FAILURE);
            }
        }


        for(i = 0; i < 3; i++)
        {
            printf("about to get image number %d \n", i);
            for(j = 0; j < MAXIMUM_NUMBER_OF_CAMERAS; j++)
            {
                /* comment next if getting all cameras at once */
                /* get_image_from_bttv_camera_2 gets the timestamp, too*/
                if(get_image_from_bttv_camera_2(all_images[j], all_cameras[j]) == ERROR)
                {
                    kjb_print_error();
                    kjb_exit(EXIT_FAILURE);
                }

                /*
                to get the time in a pretty format, you can use 
                localtime_r and asctime_r
                */
                printf("image taken at: %ld seconds since the Epoch\n", all_images[j]->image_time.tv_sec);
                sprintf(temp_string, "images/test-%d%05d.jpg", j, i);
                if(write_camera_bw_byte_image(all_images[j], temp_string) == ERROR)
                {
                    kjb_print_error();
                    kjb_exit(EXIT_FAILURE);
                }
            }
        }
    }

    /* clean up */
    for(i = 0; i < MAXIMUM_NUMBER_OF_CAMERAS; i++)
    {
        free_bttv_camera(all_cameras[i]);
        kjb_free_camera_bw_byte_image(all_images[i]);
    }

#endif  /* End disabled so Kobus can compile. */

    return 0;
}

