
/* $Id: cgi_draw.h 8780 2011-02-27 23:42:02Z predoehl $ */

#ifndef CGI_DRAW_INCLUDED    /* Kobus */
#define CGI_DRAW_INCLUDED


#include "l/l_sys_std.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  cgi_draw.h

  A library for creating and editing graphics, with an emphasis on
  cgi-based on-the-fly graphics!

  by Bill Kendrick
  New Breed Software

  kendrick@zippy.sonoam.edu
  http://zippy.sonoma.edu/kendrick/

  January 28, 1997 - July 16, 1997
*/


#define BITMAP int
#define FONT int


/* cd_init - call to initialize cgi-draw
 * No arguments accepted
 * No return values */

void cd_init(void);


/* cd_rawassign - assigns a channel of a particular pixel in a bitmap with a
   certain value
 * Arguments: bitmap to affect, x and y location of pixel and rgb channel
   to change, and a value to assign
 * Returns 0 on success, -1 on error (unused bitmap, out of bounds, etc.) */

int cd_rawassign(BITMAP bitmap, int x, int y, int rgb, unsigned char value);


/* cd_rawget - returns the value of a channel of a particular pixel in a
   bitmap
 * Arguments: bitmap, x and y location and rgb channel you're interested in
 * Returns: unsigned char value of pixel/channel on success, -1 on error
   (unused bitmap, out of bounds, etc.)
   */

int cd_rawget(BITMAP bitmap, int x, int y, int rgb);


/* cd_newbitmap - creates a new, blank bitmap.
 * Arguments: the width and height of the new bitmap to create
 * Returns: the bitmap number of the new bitmap on success, -1 on error
   (no more available memory or space in the bitmaps table)
   */

BITMAP cd_newbitmap(int width, int height);


/* cd_duplicatebitmap - copies part of a bitmap into a new bitmap.
 * Arguments: the source bitmap and the x1, y1 to x2, y2 area to
   create the new bitmap out of.
 * Returns: the bitmap number of the new bitmap on success, -1 on error
   (invalid source bitmap, no more available memory or space in the
   bitmaps table)
 */

BITMAP cd_duplicatebitmap(BITMAP source, int x1, int y1, int x2, int y2);


/* cd_pastebitmap - copies part of a bitmap into an existing bitmap.
 * Arguments: destination bitmap to be pasted into, source bitmap to
   copy from, x1, y1 to x2, y2 area of source from which to copy,
   dx, dy location within destination in which to copy and opacity with
   which to copy
 * Returns: 0 on success, -1 on error (invalid source or destination
   bitmap, out of bounds, etc.)
*/

int cd_pastebitmap(BITMAP destination, BITMAP source,
           int x1, int y1, int x2, int y2,
           int dx, int dy, int opacity);


/* cd_pastebitmapusingmask - copies part of a bitmap into an existing bitmap
   using yet another bitmap as a filter "mask"
 * Arguments: destination bitmap to be pasted into, source bitmap to
   copy from, mask bitmap to filter the source through,
   x1, y1 to x2, y2 area of source from which to copy,
   dx, dy location within destination in which to copy,
   and mx, my location within the mask to start using the mask.
 * Returns: 0 on success, -1 on error (invalid source or destination
   bitmap, out of bounds, etc.)
*/

int cd_pastebitmapusingmask(BITMAP destination, BITMAP source, BITMAP mask,
                int x1, int y1, int x2, int y2,
                int dx, int dy, int mx, int my);


/* cd_pastebitmaprotated - copies part of a bitmap into an existing bitmap,
   rotating the bitmap first.
 * Arguments: destination bitmap to be pasted into, source bitmap to
   copy from, mask bitmap to filter the source through,
   x1, y1 to x2, y2 area of source from which to copy,
   dx, dy location within destination in which to copy,
   degrees to rotate the source, and opacity with which to paste the source.
 * Returns: 0 on success, -1 on error (invalid source or destination
   bitmap, out of bounds, etc.)
   */

int cd_pastebitmaprotated(BITMAP destination, BITMAP source,
              int x1, int y1, int x2, int y2, int dx, int dy,
              int deg, int opacity);


/* cd_freebitmap - frees a used bitmap.
 * Arugments: bitmap to free.
 * Returns: 0 on success, -1 on error (invalid bitmap)
 */

int cd_freebitmap(BITMAP bitmap);


/* cd_rect - draws a rectangle:
 * Arguments: bitmap to draw into, x1, y1 to x2, y2 area of the rectangle,
   width with which to draw the border, and the color/style of the
   border and the filled area.
 * Returns: 0 on success, -1 on failure (invalid bitmap, etc.)
 */

int cd_rect(BITMAP bitmap, int x1, int y1, int x2, int y2, int border_width,
         unsigned char * border_color, unsigned char * fill_color);


/* cd_line: draws a line.
 * Arguments: bitmap to draw into, x1, y1 to x2, y2 points to draw the
   line between, and a color/style to draw with.
 * Returns: 0 on success, -1 on error (invalid bitmap)
 */

int cd_line(BITMAP bitmap, int x1, int y1, int x2, int y2,
        unsigned char * color);


/* cd_plot - Draws a dot.
  * Arguments: bitma to draw into, x, y point to draw at and color/style to
    draw with.
  * Returns: 0 on success, -1 on error
  */

int cd_plot(BITMAP bitmap, int x, int y, unsigned char * color);


/* cd_save - Saves the image as PPM format to the file stream.
 * Arguments: file stream to save into, bitmap to save and x1, y1 to x2, y2
   area to save.
 * Returns: 0 on success, -1 on error
 */

int cd_save(FILE * fi, BITMAP b, int x1, int y1, int x2, int y2);


/* cd_load - Loads a PPM format image.
 * Arguments: file stream to load from, bitmap to load into, x and y
   locations to start loading into, and opacity with which to add the
   data to the existing image.
 * Returns: 0 on success, -1 on error
 */

BITMAP cd_load(FILE * fi, BITMAP bitmap, int xx, int yy, int opacity);


/* cd_setstyle - Assigns color/style values to a color entry.
 * Arguments: Color entry buffer to store data into, red, green and blue
   color values for the color/style, red, green and blue channel opacities
   for the color/style and point thickness and point blur for the color/style.
 * Returns: 0 on success, -1 on error.
 */

int cd_setstyle(unsigned char * buf, int r, int g, int b,
         int ro, int go, int bo, int thick, int blur);


/* cd_style - returns a temporary pointer to a set of color and style info.
   sent to it.
 * Arguments: Red, green and blue color values for the color/style,
   red, green and blue channel opacities for the color/style and
   point thickness and point blur for the color/style.
 * Returns: a (temporary) color/style buffer
 */

unsigned char * cd_style(int r, int g, int b, int ro, int go, int bo,
             int thick, int blur);


/* cd_loadfont - Loads a "Bill Font" (.bfnt) file.
 * Arguments: file stream from which to load the font data.
 * Returns: the font number of the loaded font on success, -1 on error
   (invalid file type, etc.).
 */

FONT cd_loadfont(FILE * fi);


/* cd_drawtext - Draws text into a bitmap.
 * Arguments: bitmap to draw into, font to use for text generation,
   x, y position to start drawing from, a color/style to use for drawing,
   the text to draw, whether to add underline and/or italics to the text
   and how to wrap.
 * Returns width of text drawn on success, -1 on error.
 */

int cd_drawtext(BITMAP bitmap, FONT font, int x, int y, unsigned char * color,
        char * text, int underline, int italics, int wordwrap);


/* cd_rotatebitmap - Rotates an existing bitmap, creating a new bitmap.
 * Arguments: bitmap to base the new, rotated bitmap on, and the number of
   degrees to rotate the bitmap.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_rotatebitmap(BITMAP bitmap, int deg);


/* cd_invertbitmap: inverts every pixel in a bitmap.
 * Arguments: a bitmap to invert
 * Returns: 0 on success, -1 on error
 */

int cd_invertbitmap(BITMAP bitmap);


/* cd_resizebitmap: Makes a new resized bitmap out of an existing bitmap
 * Arguments: a bitmap to base the new, resized bitmap on and the new bitmap's
   width and height.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_resizebitmap(BITMAP bitmap, int w, int h);


/* cd_resamplebitmap: Makes a new resized bitmap out of an existing bitmap
   and averages original pixels together when shrinking an image
 * Arguments: a bitmap to base the new, resized bitmap on and the new bitmap's
   width and height.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_resamplebitmap(BITMAP bitmap, int w, int h);


/* CD_fixxy: cgi-draw routine to convert (x1,y1)-(x2,y2) to (0,0)-(w,h)
   if the values are "ALL": */

int CD_fixxy(BITMAP bitmap, int * x1, int * y1, int * x2, int * y2);


/* cd_greyscalebitmap: Converts a portion of a bitmap to greyscale.
 * Arguments: a bitmap to turn grey, the x1, y1 to x2, y2 area to make grey.
 * Returns: 0 on success, -1 on error.
 */

int cd_greyscalebitmap(BITMAP bitmap, int x1, int y1, int x2, int y2);


int sign(int n);


/* cd_lumadjustbitmap: Brightens or darkens part of a bitmap.
 * Arguments: a bitmap to adjust, the x1, y1 to x2, y2 area to affect,
   and the brightness (0 = no change, > 0 = brighter, < 0 = darker) to
   set it to.
 * Returns: 0 on success, -1 on error.
 */

int cd_lumadjustbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2,
               int brightness);


/* cd_satadjustbitmap: Increases or decreases color saturation.
 * Arguments: a bitmap to adjust, the x1, y1 to x2, y2 area to affect,
   and the adjustment (0 = no change, > 0 = more colorful,
   < 0 = less colorful) to make.
 * Returns: 0 on success, -1 on error.
 */

int cd_satadjustbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2,
               int adjustment);


/* CD_satpick: returns adjusted channel intensity of first channel
   based on second and third channels:
   */

int CD_satpick(int c1, int c2, int c3, int adj);


/* cd_flipbitmap: Mirrors part of an image vertically. (Top becomes bottom).
 * Arguments: a bitmap to flip and the x1, y1 to x2, y2 area to flip.
 * Returns: 0 on success, -1 on error.
 */

int cd_flipbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2);


/* cd_mirrorbitmap: Mirrors part of an image horizontaly. (Left becomes right).
 * Arguments: a bitmap to mirror and the x1, y1 to x2, y2 area to mirror.
 * Returns: 0 on success, -1 on error.
 */

int cd_mirrorbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2);


/* cd_enlargebitmap: Enlarges the size of a bitmap, leaving existing contents
   unaltered.
 * Arguments: a bitmap to enlarge, the width and height to change it to,
   and the location to place the previous contents within the new image.
 * Returns: bitmap number of new image on success, -1 on error.
 */

BITMAP cd_enlargebitmap(BITMAP bitmap, int width, int height, int x1, int y1);


/* cd_inbounds: tells whether a location is within the bounds of a bitmap.
 * Arguments: bitmap you're testing for and x,y position
 * Returns: NO if it's out of bounds, YES if it's in bounds
 */

int cd_inbounds(BITMAP bitmap, int x, int y);


#ifdef DEF_OUT /* Kobus: We don't need this. */
int CD_randnum(int mod);
#endif


/* cd_trimbitmap: trims a bitmap by removing any exterior area of the
   image that's the same color, +/- a threshold.
 * Arguments: a bitmap to trim, the r, g and b color values of the
   exterior color to be considered excess, and a threshold value for
   that color.
 * Returns: a new bitmap on success, -1 on error
 */

BITMAP cd_trimbitmap(BITMAP bitmap, int r, int g, int b, int threshold);


/* cd_quantizebitmap: Quantizes the colors in a bitmap, reducing the
   number of unique colors found in the bitmap.
 * Arguments: a bitmap to quantize, the x1,y1 to x2,y2 area to quantize,
   and how many colors to reduce the bitmap to.
 * Returns: 0 on success, -1 on error
 */

int cd_quantizebitmap(BITMAP bitmap, int x1, int y1, int x2, int y2,
              double colors, int dither);

int           cd_get_font_height (int font);
unsigned char cd_get_map_value   (int bitmap, int index);
int           cd_get_bitmap_width(int bitmap);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif   /* Kobus */



