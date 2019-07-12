
/* $Id: axis_camera.c 4723 2009-11-16 18:57:09Z kobus $ */

#include "m/m_incl.h"
#include "i/i_float.h"
#include "i/i_bw_byte.h"
#include "camera/camera_bw_byte_image.h"
#include "camera/axis_camera.h"
#include "camera/base64.h"

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Kobus: This file is a good start, but it is not very system independent, nor
 * does it conform to the KJB library conventions. 
*/
#ifndef SUN5 


static void save_data_to_file ( unsigned char * data, int len, char * filename )
{
  FILE *f;

  f = fopen(filename, "w");
  fwrite(data, 1, len, f);
  fclose(f);
}

static int connect_to_host ( const char * host )
{
  struct sockaddr_in sin;
  struct hostent *ph;
  int s;

  sin.sin_family = PF_INET;
  sin.sin_port = htons(80);
  sin.sin_addr.s_addr = inet_addr(host);

  if (sin.sin_addr.s_addr == INADDR_NONE)
  {
      if (!(ph = gethostbyname(host)))
    return 0;

      memcpy(&sin.sin_addr, ph->h_addr, ph->h_length);
  }

  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
  {
      close(s);
      return 0;
  }
  
  return s;
}

/* returns number of bytes downloaded */
static int mem_download_file
(
    const char * host, 
    const char * path, 
    const char * user, 
    const char * password,
    unsigned char ** data 
)
{
    char buffer[8192], auth[64], *ptr, *ptr2;
    int s, len, ret, clen = 0;

    /* Resolve hostname and connect */
     
    if (!(s = connect_to_host(host)))
        return 0;

    sprintf(buffer, "%s:%s", user, password);
    len = sizeof(auth);
    base64_encode((unsigned char *)auth, &len, (unsigned char *)buffer, strlen(buffer));
    auth[len] = '\0';
    len = 0;

    /* Request file and start receiving data */

    sprintf(buffer, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\nAuthorization: Basic %s\r\n\r\n", path, host, auth);
    send(s, buffer, (signed)strlen(buffer), 0);
    *data = NULL;

    while ((ret = recv(s, buffer, sizeof(buffer), 0)) > 0)
    {
        if (len == 0)
        {
          /* Make sure the request succeeded */

            if (!(ptr = strstr(buffer, "200 OK")))
            {
                close(s);
                return 0;
            }

            /* Grab the data length to check against after the server stops
               sending data as a check */

            if (!(ptr = strstr(buffer, "Content-Length: ")) || !(ptr2 = strstr(ptr, "\r\n")))
            {
                close(s);
                return 0;
            }
            
            ptr2[0] = '\0';
            clen = atoi(ptr + 16);
            ptr2[0] = '\r';

            /* Get rid of the HTTP header and start saving the binary data */

            if (!(ptr = strstr(buffer, "\r\n\r\n")))
            {
                close(s);
                return 0;
            }

            len = ret - (ptr - buffer + 4);

            if (!(*data = (unsigned char *)malloc(len)))
            {
                close(s);
                return 0;
            }

            memcpy(*data, ptr + 4, len);
        }
        else
        {
            len += ret;

            if (!(*data = (unsigned char *)realloc(*data, len)))
            {
                close(s);
                return 0;
            }

            memcpy(*data + len - ret, buffer, ret);     
        }
    }

    close(s);

    if (len != clen)
    {
        if (*data)
        {
            free(*data);
            *data = NULL;
        }

        return 0;
    }

    return len;
}

/*
/axis-cgi/com/ptz.cgi?camera=1&ircutfilter=on
/axis-cgi/com/ptz.cgi?camera=1&backlight=off
/axis-cgi/com/ptz.cgi?camera=1&autoiris=on
/axis-cgi/com/ptz.cgi?camera=1&autofocus=off
/axis-cgi/com/ptz.cgi?camera=1&riris=-250 iris close
/axis-cgi/com/ptz.cgi?camera=1&riris=250 iris open

/axis-cgi/com/ptz.cgi?camera=1&rzoom=1000 zoom -1000 to 1000

/incl/specialzoomcmd.shtml?camNo=1&zoombar=178&coord=177,0 0 to 177
/axis-cgi/com/ptz.cgi?camera=1&zoombar=178,horisontal&barcoord=177,10
/axis-cgi/com/ptz.cgi?camera=1&move=right


/axis-cgi/com/ptz.cgi?camera=1&panbar=194,horisontal&barcoord=194,0
/axis-cgi/com/ptz.cgi?camera=1&tiltbar=194,vertical&barcoord=0,194
/axis-cgi/com/ptz.cgi?camera=1&move=down
 */

int set_zoom ( Axis_camera * camera, int zoom ) /* 0 to 1000 */
{
    unsigned char *data = NULL;
    char path[128];
    int len;

    sprintf(path, "/axis-cgi/com/ptz.cgi?camera=1&zoombar=1000,horisontal&barcoord=%d,0", zoom);
    len = mem_download_file(camera->hostname, path, camera->username, camera->password, &data);

    if (len == 0)
    return ERROR;

    free(data);
    return NO_ERROR;
}

int set_pan ( Axis_camera * camera, int pan ) /* 0 to 1000 */
{
  unsigned char *data = NULL;
  char path[128];
  int len;

  sprintf(path, "/axis-cgi/com/ptz.cgi?camera=1&panbar=1000,horisontal&barcoord=%d,0", pan);
  len = mem_download_file(camera->hostname, path, camera->username, camera->password, &data);

  if (len == 0)
      return ERROR;

  free(data);
  return NO_ERROR;
}


int set_tilt ( Axis_camera * camera, int tilt ) /* 0 to 1000 */
{
  unsigned char *data = NULL;
  char path[128];
  int len;

  sprintf(path, "/axis-cgi/com/ptz.cgi?camera=1&tiltbar=1000,horisontal&barcoord=%d,0", tilt);
  len = mem_download_file(camera->hostname, path, camera->username, camera->password, &data);

  if (len == 0)
      return ERROR;

  free(data);
  return NO_ERROR;
}

int get_image_from_axis_camera ( Camera_bw_byte_image ** image, Axis_camera * camera )
{
    KJB_image *kjb_image = NULL;
    unsigned char *data = NULL;
    char filename[64] = "temp.jpg";
    struct timeval tv;
    int len;

    len = mem_download_file(camera->hostname, "/axis-cgi/jpg/image.cgi", camera->username, camera->password, &data);    

    if (len == 0)
    {
    set_error("unable to get camera image");
    return ERROR;
    }

    gettimeofday(&tv, NULL);
    save_data_to_file(data, len, filename);    

    if (kjb_read_image_2(&kjb_image, filename) != NO_ERROR)
    {
    kjb_free(data);
    set_error("unable to read image data");
    return ERROR;
    }

    get_target_camera_bw_byte_image(image, kjb_image->num_rows, kjb_image->num_cols);
    (*image)->image_time = tv;

    kjb_image_to_bw_byte_image(&(*image)->data, kjb_image);
    kjb_free_image(kjb_image);
    free(data);

    return NO_ERROR;
}

#endif 

#ifdef __cplusplus
}
#endif
