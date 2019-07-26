
/* $Id: cgi_draw.c 4723 2009-11-16 18:57:09Z kobus $ */

/*
  cgi-draw.c

  A library for creating and editing graphics, with an emphasis on
  cgi-based on-the-fly graphics!

  by Bill Kendrick
  New Breed Software

  kendrick@zippy.sonoam.edu
  http://zippy.sonoma.edu/kendrick/

  January 28, 1997 - July 16, 1997
*/


/*
 * Kobus: Hacked it up a bit over the years. This is sloppy code. Thorough
 * testing and further cleanup would be nice.
*/

#include "l/l_gen.h"   /* Kobus--added dir */

#ifdef DEF_OUT    /* Kobus -- Dangerous side effect! */
#    include <sys/time.h>
#endif 

#include <math.h>
#include "cgi_draw/cgi_draw.h"   /* Kobus--added dir */


#ifdef KJB_CPLUSPLUS
#    define new c_new
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Kobus, 05-0-22: Moved this stuff from cgi_draw.h. There is only one source file here!
*/



#define MAX_BITMAPS 16
#define MAX_FONTS 16

int cd_bitmapused[MAX_BITMAPS];
unsigned char * cd_map[MAX_BITMAPS];
int cd_bitmapwidth[MAX_BITMAPS];
int cd_bitmapheight[MAX_BITMAPS];

int cd_fontused[MAX_FONTS];
unsigned char cd_font[MAX_FONTS][96][100][100];
int cd_font_height[MAX_FONTS];
int cd_font_widths[MAX_FONTS][96];

#define CHAR_WRAP 1
#define WORD_WRAP 2
#define NEW -2
#define USED 123
#define UNUSED -123
#define ALL -999
#define CENTER -2
#define ASPECT -1


#define SPACE_CHAR 'o'


/* End Kobus. */




/* All-powerful global! */

int cd_inited=0;


/* cd_init - call to initialize cgi-draw
 * No arguments accepted
 * No return values */

void cd_init(void)
{
    int i;
#ifdef DEF_OUT    /* Kobus -- Dangerous side effect! */
    long z;
    struct timeval tp;
#endif


    /* Let the other functions know we've initialized: */

    cd_inited = 1;


    /* Mark all of the bitmaps and fonts as unused: */

    for (i = 0; i < MAX_BITMAPS; i++)
        cd_bitmapused[i] = UNUSED;

    for (i = 0; i < MAX_FONTS; i++)
        cd_fontused[i] = UNUSED;


#ifdef DEF_OUT    /* Kobus -- Dangerous side effect! */
    /* Initialize the random function: */

    gettimeofday(&tp, NULL);
    z = tp.tv_sec + tp.tv_usec;
    srand(z);
#endif
}


/* cd_rawassign - assigns a channel of a particular pixel in a bitmap with a
   certain value
 * Arguments: bitmap to affect, x and y location of pixel and rgb channel
 to change, and a value to assign
 * Returns 0 on success, -1 on error (unused bitmap, out of bounds, etc.) */

int cd_rawassign(BITMAP bitmap, int x, int y, int rgb, unsigned char value)
{
    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }


    /* Assing the channel of the bitmap the value, if we're within bounds: */

    if (x >= 0 && x < cd_bitmapwidth[bitmap] && y >= 0 &&
        y < cd_bitmapheight[bitmap])
    {
        cd_map[bitmap][(y * cd_bitmapwidth[bitmap] + x) * 3 + rgb] = value;
        return 0;
    }
    else
    {
        return -1;
    }
}


/* cd_rawget - returns the value of a channel of a particular pixel in a
   bitmap
 * Arguments: bitmap, x and y location and rgb channel you're interested in
 * Returns: unsigned char value of pixel/channel on success, -1 on error
 (unused bitmap, out of bounds, etc.)
 */

int cd_rawget(BITMAP bitmap, int x, int y, int rgb)
{
    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }


    /* Return value, or -1 on out of bounds: */

    if (x >= 0 && x < cd_bitmapwidth[bitmap] &&
        y >= 0 && y < cd_bitmapheight[bitmap])
        return cd_map[bitmap][(y * cd_bitmapwidth[bitmap] + x) * 3 + rgb];
    else
        return -1;
}


/* cd_newbitmap - creates a new, blank bitmap.
 * Arguments: the width and height of the new bitmap to create
 * Returns: the bitmap number of the new bitmap on success, -1 on error
 (no more available memory or space in the bitmaps table)
 */

BITMAP cd_newbitmap(int width, int height)
{
    int i, b, x, y, rgb;

    b = -1;

    if (cd_inited == 0)
    {
        return -1;
    }


    /* Find an unused bitmap slot to use: */

    for (i = 0; i < MAX_BITMAPS && b == -1; i++)
        if (cd_bitmapused[i] == UNUSED)
            b = i;


    /* Assign the width/height, create the bitmap in memory and blank it to
white: */

    if (b != -1)
    {
        cd_bitmapwidth[b] = width;
        cd_bitmapheight[b] = height;

        cd_map[b] = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);

        if (cd_map[b] != NULL)
        {
            cd_bitmapused[b] = USED;

            for (y = 0; y < height; y++)
                for (x = 0; x < width; x++)
                    for (rgb = 0; rgb < 3; rgb++)
                        cd_rawassign(b, x, y, rgb, 255);
        }
        else
        {
            return -1;
        }
    }

    return b;
}


/* cd_duplicatebitmap - copies part of a bitmap into a new bitmap.
 * Arguments: the source bitmap and the x1, y1 to x2, y2 area to
 create the new bitmap out of.
 * Returns: the bitmap number of the new bitmap on success, -1 on error
 (invalid source bitmap, no more available memory or space in the
 bitmaps table)
 */

BITMAP cd_duplicatebitmap(BITMAP source, int x1, int y1, int x2, int y2)
{
    int x, y, rgb;
    BITMAP b;

    if (cd_inited == 0 || cd_bitmapused[source] != USED)
    {
        return -1;
    }

    CD_fixxy(source, &x1, &y1, &x2, &y2);


    /* Make a new bitmap in the shape of the peice we're copying: */

    b = cd_newbitmap(x2 - x1 + 1, y2 - y1 + 1);


    /* Copy the pixels of the source bitmap into the new bitmap:*/

    if (b != -1)
        for (y = y1; y <= y2; y++)
            for (x = x1; x <= x2; x++)
                for (rgb = 0; rgb < 3; rgb++)
                    cd_rawassign(b, x - x1, y - y1, rgb, cd_rawget(source, x, y, rgb));

    return b;
}


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
                   int dx, int dy, int opacity)
{
    int c;
    int x, y, rgb;

    if (cd_inited == 0 || cd_bitmapused[destination] != USED ||
        cd_bitmapused[source] != USED)
    {
        return -1;
    }

    CD_fixxy(source, &x1, &y1, &x2, &y2);

    if (opacity == 255)
    {
        for (y = y1; y <= y2; y++)
            for (x = x1; x <= x2; x++)
                for (rgb = 0; rgb < 3; rgb++)
                {
                    cd_rawassign(destination, x - x1 + dx, y - y1 + dy, rgb,
                                 cd_rawget(source, x, y, rgb));
                }
    }
    else if (opacity != 0)
    {
        for (y = y1; y <= y2; y++)
            for (x = x1; x <= x2; x++)
                for (rgb = 0; rgb < 3; rgb++)
                {
                    c = (cd_rawget(destination, x - x1 + dx, y - y1 + dy, rgb)) *
                        (255 - opacity);
                    c = c + cd_rawget(source, x, y, rgb) * opacity;

                    cd_rawassign(destination, x - x1 + dx, y - y1 + dy, rgb,
                                 c / 255);
                }
    }

    return 0;
}


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
                            int dx, int dy, int mx, int my)
{
    int x, y, rgb;
    int c, masko, sourceo, desto;

    if (cd_inited == 0 || cd_bitmapused[destination] != USED ||
        cd_bitmapused[source] != USED || cd_bitmapused[mask] != USED)
    {
        return -1;
    }

    CD_fixxy(source, &x1, &y1, &x2, &y2);

    for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
            for (rgb = 0; rgb < 3; rgb++)
            {
                masko = 255 - cd_rawget(mask, x - x1 + mx, y - y1 + my, rgb);
                desto = cd_rawget(destination, x - x1 + dx, y - y1 + dy, rgb);
                sourceo = cd_rawget(source, x, y, rgb);

                c = ((sourceo * masko) + (desto * (255 - masko))) / 255;

                cd_rawassign(destination, x - x1 + dx, y - y1 + dy, rgb, c);
            }

    return 0;
}


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
                          int deg, int __attribute__((unused)) dummy_opacity)
{
    BITMAP part, rotatedpart, mask, rotatedmask;

    if (cd_inited == 0 || cd_bitmapused[destination] != USED ||
        cd_bitmapused[source] != USED)
    {
        return -1;
    }

    CD_fixxy(source, &x1, &y1, &x2, &y2);

    part = cd_duplicatebitmap(source, x1, y1, x2, y2);
    if (part == -1)
    {
        return -1;
    }

    rotatedpart = cd_rotatebitmap(part, deg);
    cd_freebitmap(part);
    if (rotatedpart == -1)
    {
        return -1;
    }

    mask = cd_newbitmap(cd_bitmapwidth[part], cd_bitmapheight[part]);
    if (mask == -1)
    {
        return -1;
    }

    cd_invertbitmap(mask);
    rotatedmask = cd_rotatebitmap(mask, deg);
    cd_freebitmap(mask);
    if (rotatedmask == -1)
    {
        return -1;
    }

    cd_pastebitmapusingmask(destination, rotatedpart, rotatedmask,
                            0, 0,
                            cd_bitmapwidth[rotatedpart] - 1,
                            cd_bitmapheight[rotatedpart] - 1,
                            dx, dy, 0, 0);

    cd_freebitmap(rotatedpart);
    cd_freebitmap(rotatedmask);

    return 0;
}


/* cd_freebitmap - frees a used bitmap.
 * Arugments: bitmap to free.
 * Returns: 0 on success, -1 on error (invalid bitmap)
 */

int cd_freebitmap(BITMAP bitmap)
{
    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED)
    {
        return -1;
    }

    free(cd_map[bitmap]);
    cd_bitmapused[bitmap] = UNUSED;

    return 0;
}


/* cd_rect - draws a rectangle:
 * Arguments: bitmap to draw into, x1, y1 to x2, y2 area of the rectangle,
 width with which to draw the border, and the color/style of the
 border and the filled area.
 * Returns: 0 on success, -1 on failure (invalid bitmap, etc.)
 */

int cd_rect(BITMAP bitmap, int x1, int y1, int x2, int y2, int border_width,
            unsigned char * border_color, unsigned char * fill_color)
{
    int i;


    if (cd_inited == 0 || cd_bitmapused[bitmap] == UNUSED)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    /* Draw Filled Area: */

    for (i = y1; i <= y2; i++)
        cd_line(bitmap, x1, i, x2, i, fill_color);


    /* Draw Border: */

    for (i = 0; i < border_width; i++)
    {
        cd_line(bitmap, x1 + i, y1 + i, x2 - i, y1 + i, border_color); /* Top  */
        cd_line(bitmap, x2 - i, y1 + i, x2 - i, y2 - i, border_color); /* Rt   */
        cd_line(bitmap, x2 - i, y2 - i, x1 + i, y2 - i, border_color); /* Bot  */
        cd_line(bitmap, x1 + i, y2 - i, x1 + i, y1 + i, border_color); /* Left */
    }

    return 0;
}


/* cd_line: draws a line.
 * Arguments: bitmap to draw into, x1, y1 to x2, y2 points to draw the
 line between, and a color/style to draw with.
 * Returns: 0 on success, -1 on error (invalid bitmap)
 */

int cd_line(BITMAP bitmap, int x1, int y1, int x2, int y2,
            unsigned char * color)
{
    int z, i, steep, sx, sy, dx, dy, e;

    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED)
    {
        return -1;
    }


    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    /* Draw a simple vertical line: */

    if (x1 == x2)
    {
        if (y1 > y2)
        {
            z = y1;
            y1 = y2;
            y2 = z;
        }

        for (sy = y1; sy <= y2; sy++)
            cd_plot(bitmap, x1, sy, color);
    }
    else if (y1 == y2)
    {
        /* Draw a simple horizontal line: */

        if (x1 > x2)
        {
            z = x1;
            x1 = x2;
            x2 = z;
        }

        for (sx = x1; sx <= x2; sx++)
            cd_plot(bitmap, sx, y1, color);
    }
    else
    {
        /* Draw an angled line: */

        steep = 0;

        dx = abs(x2 - x1);
        sx = ((x2 - x1) > 0) ? 1 : -1;
        dy = abs(y2 - y1);
        sy = ((y2 - y1) > 0) ? 1 : -1;

        if (dy > dx)
        {
            steep = 1;

            z = x1;
            x1 = y1;
            y1 = z;

            z = dy;
            dy = dx;
            dx = z;

            z = sy;
            sy = sx;
            sx = z;
        }

        e = 2 * dy - dx;
        for (i = 0; i < dx; i++)
        {
            if (steep == 1)
                cd_plot(bitmap, y1, x1, color);
            else
                cd_plot(bitmap, x1, y1, color);

            while (e >= 0)
            {
                y1 = y1 + sy;
                e = e - 2 * dx;
            }

            x1 = x1 + sx;
            e = e + 2 * dy;
        }

        cd_plot(bitmap, x2, y2, color);
    }

    return 0;
}


/* cd_plot - Draws a dot.
 * Arguments: bitma to draw into, x, y point to draw at and color/style to
 draw with.
 * Returns: 0 on success, -1 on error
 */

int cd_plot(BITMAP bitmap, int x, int y, unsigned char * color)
{
    int c, rgb, old, new, z, xx, yy, thick, blur, i;


    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED)
    {
        return -1;
    }

    thick = color[6] - 1;
    blur = color[7];

    if (x - thick >= 0 && x + thick <= cd_bitmapwidth[bitmap] &&
        y - thick >= 0 && y + thick <= cd_bitmapheight[bitmap])
    {
        xx = x;
        yy = y;
        i = 0;

        for (xx = x - thick; xx <= x + thick; xx++)
        {
            i = i + 1;

            for (yy = y - thick; yy <= y + thick; yy++)
            {
                for (rgb = 0; rgb < 3; rgb++)
                {
                    if (blur != 0)
                        z = color[3 + rgb] / (blur + i);
                    else
                        z = color[3 + rgb];

                    if (z > 255)
                        z = 255;

                    if (z > 0 && z < 255)
                    {
                        old = cd_rawget(bitmap, xx, yy, rgb) * (255 - z);
                        new = color[rgb] * z;

                        c = (old + new) / 255;

                        cd_rawassign(bitmap, xx, yy, rgb, c);
                    }
                    else if (z == 255)
                        cd_rawassign(bitmap, xx, yy, rgb, color[rgb]);
                }
            }
        }
    }

    return 0;
}


/* cd_save - Saves the image as PPM format to the file stream.
 * Arguments: file stream to save into, bitmap to save and x1, y1 to x2, y2
 area to save.
 * Returns: 0 on success, -1 on error
 */

int cd_save(FILE * fi, BITMAP b, int x1, int y1, int x2, int y2)
{
    int x, y, rgb;


    if (cd_inited == 0 || cd_bitmapused[b] != USED || fi == NULL)
    {
        return -1;
    }

    CD_fixxy(b, &x1, &y1, &x2, &y2);

    if (x1 < 0 || x1 > cd_bitmapwidth[b] || y1 < 0 || y1 < cd_bitmapheight[b] ||
        x2 < 0 || x2 > cd_bitmapwidth[b] || y2 < 0 || y2 < cd_bitmapheight[b])
    {
        return -1;
    }

    /* Dump PPM header: */

    fprintf(fi, "P6\n");
    fprintf(fi, "%d %d\n", x2 - x1 + 1, y2 - y1 + 1);
    fprintf(fi, "255\n");


    /* Dump data: */

    for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
            for (rgb = 0; rgb < 3; rgb++)
                fputc(cd_rawget(b, x, y, rgb), fi);

    return 0;
}

/* cd_load - Loads a PPM format image.
 * Arguments: file stream to load from, bitmap to load into, x and y
 locations to start loading into, and opacity with which to add the
 data to the existing image.
 * Returns: 0 on success, -1 on error
 */

BITMAP cd_load(FILE * fi, BITMAP bitmap, int xx, int yy, int opacity)
{
    char temp[10240];
    int x, y, rgb, m, old, new, w, h, c;


    if (cd_inited == 0)
    {
        return -1;
    }

    if (bitmap != NEW)
        if (cd_bitmapused[bitmap] != USED)
        {
            return -1;
        }


    /* Load PPM header: */

    fscanf(fi, "%s %d %d %d", temp, &w, &h, &m);


    /* Make new bitmap if that's what they wanted: */

    if (bitmap == NEW)
        bitmap = cd_newbitmap(w + xx, h + yy);


    /* Check header: */

    if (w + xx > cd_bitmapwidth[bitmap] || w < 1 ||
        h + yy > cd_bitmapheight[bitmap] || h < 1 ||
        m > 255)
    {
        return -1;
    }


    fgetc(fi);


    /* Load data: */

    if (strcmp(temp, "P6") == 0)
    {
        if (opacity == 255)
        {
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++)
                    for (rgb = 0; rgb < 3; rgb++)
                        cd_rawassign(bitmap, x + xx, y + yy, rgb, fgetc(fi));
        }
        else if (opacity != 0)
        {
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++)
                    for (rgb = 0; rgb < 3; rgb++)
                    {
                        old = cd_rawget(bitmap, x + xx, y + yy, rgb) * (255 - opacity);
                        new = fgetc(fi) * opacity;

                        cd_rawassign(bitmap, x + xx, y + yy, rgb, (old + new) / 255);
                    }
        }
    }
    else if (strcmp(temp, "P5") == 0)
    {
        if (opacity == 255)
        {
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++)
                {
                    c = fgetc(fi);
                    for (rgb = 0; rgb < 3; rgb++)
                        cd_rawassign(bitmap, x + xx, y + yy, rgb, c);
                }
        }
        else if (opacity != 0)
        {
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++)
                {
                    c = fgetc(fi);

                    for (rgb = 0; rgb < 3; rgb++)
                    {
                        old = cd_rawget(bitmap, x + xx, y + yy, rgb) *
                            (255 - opacity);
                        new = c * opacity;

                        cd_rawassign(bitmap, x + xx, y + yy, rgb, (old + new) /
                                     255);
                    }
                }
        }
    }


    return bitmap;
}


/* cd_setstyle - Assigns color/style values to a color entry.
 * Arguments: Color entry buffer to store data into, red, green and blue
 color values for the color/style, red, green and blue channel opacities
 for the color/style and point thickness and point blur for the color/style.
 * Returns: 0 on success, -1 on error.
 */

int cd_setstyle(unsigned char * buf, int r, int g, int b,
                int ro, int go, int bo, int thick, int blur)
{
    if (r >= 0 && r <= 255 && g >=0 && g <= 255 && b >= 0 && g <= 255 &&
        ro >= 0 && ro <= 255 && go >= 0 && go <= 255 && bo >= 0 && go <= 255 &&
        thick >= 1 && blur >= 0 && cd_inited == 1)
    {
        buf[0] = r;
        buf[1] = g;
        buf[2] = b;

        buf[3] = ro;
        buf[4] = go;
        buf[5] = bo;

        buf[6] = thick;
        buf[7] = blur;

        return 0;
    }
    else
    {
        return -1;
    }
}


/* cd_loadfont - Loads a "Bill Font" (.bfnt) file.
 * Arguments: file stream from which to load the font data.
 * Returns: the font number of the loaded font on success, -1 on error
 (invalid file type, etc.).
 */

FONT cd_loadfont(FILE * fi)
{
    int c, x, y, f, i;
    char temp[512];


    if (cd_inited == 0)
    {
        return -1;
    }


    /* Find an unused font slot: */

    f = -1;
    for (i = 0; i < MAX_FONTS && f == -1; i++)
        if (cd_fontused[i] == UNUSED)
            f = i;


    /* Load the font: */

    if (f != -1)
    {
        cd_fontused[f] = USED;


        /* Load header: */

        fgets(temp, 512, fi);

        if (strcmp(temp, "bfnt - BillFont\n") != 0)
        {
            fclose(fi);
            {
                return -1;
            }
        }

        fgets(temp, 512, fi);  /* Skip "created from" line */
        fgets(temp, 512, fi);  /* Skip blank line */

        fgets(temp, 512, fi);  /* Read height line */
        cd_font_height[f] = atoi(temp);

        fgets(temp, 512, fi);  /* Skip blank line */


        /* Read each character: */

        for (c = 0; c < 94; c++)
        {
            /* Width: */

            cd_font_widths[f][c] = fgetc(fi);


            /* Data: */

            for (y = 0; y < cd_font_height[f]; y++)
                for (x = 0; x < cd_font_widths[f][c]; x++)
                    cd_font[f][c][y][x] = fgetc(fi);
        }
    }

    return f;
}


/* cd_drawtext - Draws text into a bitmap.
 * Arguments: bitmap to draw into, font to use for text generation,
 x, y position to start drawing from, a color/style to use for drawing,
 the text to draw, whether to add underline and/or italics to the text
 and how to wrap.
 * Returns width of text drawn on success, -1 on error.
 */

int cd_drawtext(BITMAP bitmap, FONT font, int x, int y, unsigned char * color,
                char * text, int __attribute__((unused)) dummy_underline, int italics, int wordwrap)
{
    unsigned char txtcolor[8];
    int xx, yy, i, c, z1, z2, z3, oldx, xstart, last_space;


    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED ||
        cd_fontused[font] != USED)
    {
        return -1;
    }


    if (wordwrap == WORD_WRAP)
    {
        last_space = -1;
        xx = x;

        for (i = 0; i < (int)strlen(text); i++)
        {
            c = text[i];

            if (c == ' ')
            {
                last_space = i;
                c = SPACE_CHAR;
            }

            if (c == '\n')
            {
                xx = x;
                last_space = -1;
            }
            else
            {
                xx = xx + cd_font_widths[font][c - 33];
                if (i < (int)strlen(text) - 1)
                {
                    if (xx + cd_font_widths[font][text[i] - 33] +
                        (cd_font_height[font] / 2 * italics) >=
                        cd_bitmapwidth[bitmap])
                    {
                        if (last_space != -1)
                        {
                            text[last_space] = '\n';
                            i = last_space - 1;
                        }
                    }
                }
            }
        }
    }


    oldx = x;

    for (i = 0; i < (int)strlen(text); i++)
    {
        c = text[i];

        if (c == '\n')
        {
            x = oldx;
            y = y + cd_font_height[font];

            if (y > cd_bitmapheight[bitmap])
                return x - oldx;
        }
        else if (c == ' ')
        {
            x = x + cd_font_widths[font][SPACE_CHAR - 33];
        }
        else
        {
            c = text[i] - 33;

            if (c >= 0)
            {
                for (yy = 0; yy < cd_font_height[font]; yy++)
                {
                    xstart = 0;
                    if (italics == 1)
                        xstart = (cd_font_height[font] - yy) / 2;

                    for (xx = 0; xx < cd_font_widths[font][c]; xx++)
                    {
                        /* Opacity is based on chosen text opacity AND
                           text mask: */

                        z1 = (((cd_font[font][c][yy][xx] * 255) / 255) *
                              color[3]) / 255;
                        z2 = (((cd_font[font][c][yy][xx] * 255) / 255) *
                              color[4]) / 255;
                        z3 = (((cd_font[font][c][yy][xx] * 255) / 255) *
                              color[5]) / 255;

                        cd_setstyle(txtcolor, color[0], color[1], color[2],
                                    z1, z2, z3, color[6], color[7]);

                        cd_plot(bitmap, xx + x + xstart, yy + y, txtcolor);
                    }
                }

                x = x + cd_font_widths[font][c];
            }
        }

        if (i < (int)strlen(text) - 1)
        {
            if (x + cd_font_widths[font][text[i] - 33] +
                (cd_font_height[font] / 2 * italics) >= cd_bitmapwidth[bitmap])
            {
                if (wordwrap == CHAR_WRAP)
                {
                    x = oldx;
                    y = y + cd_font_height[font];

                    if (y > cd_bitmapheight[bitmap])
                        return x - oldx;
                }
                else if (wordwrap == WORD_WRAP)
                {

                }
                else
                    return x - oldx;
            }
        }
    }

    return x - oldx;
}


/* cd_rotatebitmap - Rotates an existing bitmap, creating a new bitmap.
 * Arguments: bitmap to base the new, rotated bitmap on, and the number of
 degrees to rotate the bitmap.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_rotatebitmap(BITMAP bitmap, int deg)
{
    int c, x, y, rgb, w, h, nw, nh, xo, yo, nxo, nyo;
    double nx, ny;
    float rad, sin_r, cos_r;
    BITMAP n;

    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0 )
    {
        return -1;
    }

    w = cd_bitmapwidth[bitmap];
    h = cd_bitmapheight[bitmap];

    if (deg == 0)
    {
        n = cd_duplicatebitmap(bitmap, 0, 0, w - 1, h - 1);
    }
    else if (deg == 180)
    {
        n = cd_newbitmap(w, h);

        if (n != -1)
        {
            for (x = 0; x < w; x++)
                for (y = 0; y < h; y++)
                    for (rgb = 0; rgb < 3; rgb++)
                    {
                        cd_rawassign(n, w - x - 1, h - y - 1, rgb,
                                     cd_rawget(bitmap, x, y, rgb));
                    }
        }
    }
    else if (deg == 90 || deg == 270)
    {
        n = cd_newbitmap(h, w);

        if (n != -1)
        {
            if (deg == 270)
            {
                for (y = 0; y < h; y++)
                    for (x = 0; x < w; x++)
                        for (rgb = 0; rgb < 3; rgb++)
                        {
                            cd_rawassign(n, y, w - x - 1, rgb,
                                         cd_rawget(bitmap, w - x - 1, h - y - 1,
                                                   rgb));
                        }
            }
            else if (deg == 90)
            {
                for (y = 0; y < h; y++)
                    for (x = 0; x < w; x++)
                        for (rgb = 0; rgb < 3; rgb++)
                        {
                            cd_rawassign(n, h - y - 1, x, rgb,
                                         cd_rawget(bitmap, w - x - 1, h - y - 1, rgb));
                        }
            }
        }
    }
    else
    {
        rad = - (3.14159 * deg) / 180;

        xo = w / 2;
        yo = h / 2;

        cos_r = cos(rad);
        sin_r = sin(rad);

        /*
         * Kobus: Patched this a bit. I am not sure if this is what is meant.
         *
         * How it was:
         *     nw = (abs(w * cos_r) + abs(h * sin_r));
         *     nh = (abs(w * sin_r) + abs(h * cos_r));
         */

        UNTESTED_CODE();

        nw = (int)(0.5 + (fabs((double)(w * cos_r)) + fabs((double)(h * sin_r))));
        nh = (int)(0.5 + (fabs((double)(w * sin_r)) + fabs((double)(h * cos_r))));

        nxo = nw / 2;
        nyo = nh / 2;

        n = cd_newbitmap(nw, nh);


        if (n != -1)
        {
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++)
                    for (rgb = 0; rgb < 3; rgb++)
                    {
                        c = cd_rawget(bitmap, x, y, rgb);

                        nx = (x - xo) * cos_r - (y - yo) * sin_r + nxo;
                        ny = (x - xo) * sin_r + (y - yo) * cos_r + nyo;

                        /* c = (c + c + c + cd_rawget(n, nx, ny, rgb)) / 4; */

                        /*
                         * Kobus: 05/01/22: Did a round and a cast when the compiler noticed
                         * that we are casting from double to int, but I am only assuming that
                         * this is correct!
                         *
                         * How it was:
                         *    cd_rawassign(n, nx, ny, rgb, c);
                         *    cd_rawassign(n, nx + 1, ny, rgb, c);
                         *    cd_rawassign(n, nx, ny + 1, rgb, c);
                         *    cd_rawassign(n, nx + 1, ny + 1, rgb, c);
                         */
                        UNTESTED_CODE();

                        cd_rawassign(n, (int)(0.5 + nx), (int)(0.5 + ny), rgb, c);
                        cd_rawassign(n, (int)(nx + 1.5), (int)(0.5 + ny), rgb, c);
                        cd_rawassign(n, (int)(0.5 + nx), (int)(ny + 1.5), rgb, c);
                        cd_rawassign(n, (int)(nx + 1.5), (int)(ny + 1.5), rgb, c);
                    }
        }
    }

    return n;
}


/* cd_invertbitmap: inverts every pixel in a bitmap.
 * Arguments: a bitmap to invert
 * Returns: 0 on success, -1 on error
 */

int cd_invertbitmap(BITMAP bitmap)
{
    int x, y, rgb;

    for (y = 0; y < cd_bitmapheight[bitmap]; y++)
    {
        for (x = 0; x < cd_bitmapwidth[bitmap]; x++)
        {
            for (rgb = 0; rgb < 3; rgb++)
            {
                cd_rawassign(bitmap, x, y, rgb, 255 - cd_rawget(bitmap, x, y, rgb));
            }
        }
    }

    return 0;  /* Kobus: 05-01-22. */
}


/* cd_resizebitmap: Makes a new resized bitmap out of an existing bitmap
 * Arguments: a bitmap to base the new, resized bitmap on and the new bitmap's
 width and height.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_resizebitmap(BITMAP bitmap, int w, int h)
{
    int x, y, rgb, ox, oy, ow, oh;
    BITMAP n;

    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    ow = cd_bitmapwidth[bitmap];
    oh = cd_bitmapheight[bitmap];

    if (h == ASPECT)
        h = (oh * w / ow);

    if (w == ASPECT)
        w = (ow * h / oh);

    n = cd_newbitmap(w, h);
    if (n != -1)
    {
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
            {
                ox = (x * ow) / w;
                oy = (y * oh) / h;

                for (rgb = 0; rgb < 3; rgb++)
                {
                    cd_rawassign(n, x, y, rgb, cd_rawget(bitmap, ox, oy, rgb));
                }
            }
    }

    return n;
}


/* cd_resamplebitmap: Makes a new resized bitmap out of an existing bitmap
   and averages original pixels together when shrinking an image
 * Arguments: a bitmap to base the new, resized bitmap on and the new bitmap's
 width and height.
 * Returns: the bitmap number of the new bitmap on success, -1 on error.
 */

BITMAP cd_resamplebitmap(BITMAP bitmap, int w, int h)
{
    int x, y, rgb, ox, oy, ow, oh, z, c;
    BITMAP n;

    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    ow = cd_bitmapwidth[bitmap];
    oh = cd_bitmapheight[bitmap];

    if (h == ASPECT)
        h = (oh * w / ow);

    if (w == ASPECT)
        w = (ow * h / oh);

    n = cd_newbitmap(w, h);
    if (n != -1)
    {
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
            {
                for (rgb = 0; rgb < 3; rgb++)
                {
                    z = 0;
                    c = 0;

                    for (oy = (y * oh) / h; oy < ((y + 1) * oh) / h; oy++)
                        for (ox = (x * ow) / w; ox < ((x + 1) * ow) / w; ox++)
                        {
                            z = z + 1;
                            c = c + cd_rawget(bitmap, ox, oy, rgb);
                        }

                    c = c / z;
                    cd_rawassign(n, x, y, rgb, c);
                }
            }
    }

    return n;
}


/* CD_fixxy: cgi-draw routine to convert (x1,y1)-(x2,y2) to (0,0)-(w,h)
   if the values are "ALL": */

int CD_fixxy(BITMAP bitmap, int * x1, int * y1, int * x2, int * y2)
{
    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    if (*x1 == ALL)
        *x1 = 0;
    if (*y1 == ALL)
        *y1 = 0;

    if (*x2 == ALL)
        *x2 = cd_bitmapwidth[bitmap] - 1;
    if (*y2 == ALL)
        *y2 = cd_bitmapheight[bitmap] - 1;

    return 0;
}


/* cd_greyscalebitmap: Converts a portion of a bitmap to greyscale.
 * Arguments: a bitmap to turn grey, the x1, y1 to x2, y2 area to make grey.
 * Returns: 0 on success, -1 on error.
 */

int cd_greyscalebitmap(BITMAP bitmap, int x1, int y1, int x2, int y2)
{
    int x, y, r, g, b, c;


    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            r = cd_rawget(bitmap, x, y, 0);
            g = cd_rawget(bitmap, x, y, 1);
            b = cd_rawget(bitmap, x, y, 2);

            c = (56 * g + 33 * r + 11 * b) / 100;

            cd_rawassign(bitmap, x, y, 0, c);
            cd_rawassign(bitmap, x, y, 1, c);
            cd_rawassign(bitmap, x, y, 2, c);
        }
    }

    return 0;  /* Kobus: 05-01-22. */
}


int sign(int n)
{
    if (n < 0)
        return -1;
    else if (n > 0)
        return 1;
    else
        return 0;
}


/* cd_lumadjustbitmap: Brightens or darkens part of a bitmap.
 * Arguments: a bitmap to adjust, the x1, y1 to x2, y2 area to affect,
 and the brightness (0 = no change, > 0 = brighter, < 0 = darker) to
 set it to.
 * Returns: 0 on success, -1 on error.
 */

int cd_lumadjustbitmap
(
 BITMAP bitmap,
 int    x1,
 int    y1,
 int    x2,
 int    y2,
 int    brightness
)
{
    int x, y, rgb, c;


    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    for (y = y1 ; y <= y2; y++)
        for (x = x1; x <= x2; x++)
            for (rgb = 0; rgb < 3; rgb++)
            {
                c = cd_rawget(bitmap, x, y, rgb) + brightness;

                if (c < 0)
                    c = 0;
                else if (c > 255)
                    c = 255;

                cd_rawassign(bitmap, x, y, rgb, c);
            }

    return 0;
}


/* cd_satadjustbitmap: Increases or decreases color saturation.
 * Arguments: a bitmap to adjust, the x1, y1 to x2, y2 area to affect,
 and the adjustment (0 = no change, > 0 = more colorful,
 < 0 = less colorful) to make.
 * Returns: 0 on success, -1 on error.
 */

int cd_satadjustbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2,
                       int adjustment)
{
    int x, y, r, g, b, r2, g2, b2;


    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    for (y = y1 ; y <= y2; y++)
        for (x = x1; x <= x2; x++)
        {
            r = cd_rawget(bitmap, x, y, 0);
            g = cd_rawget(bitmap, x, y, 1);
            b = cd_rawget(bitmap, x, y, 2);

            r2 = CD_satpick(r, g, b, adjustment);
            g2 = CD_satpick(g, r, b, adjustment);
            b2 = CD_satpick(b, r, g, adjustment);

            cd_rawassign(bitmap, x, y, 0, r2);
            cd_rawassign(bitmap, x, y, 1, g2);
            cd_rawassign(bitmap, x, y, 2, b2);
        }

    return 0;
}


/* CD_satpick: returns adjusted channel intensity of first channel
   based on second and third channels:
 */

int CD_satpick(int c1, int c2, int c3, int adj)
{
    int cavg;

    cavg = (c1 + c2 + c3) / 3;

    if (c1 > cavg)
        c1 = c1 + ((cavg - c1) * (-adj)) / 255;
    else
        c1 = c1 - ((c1 - cavg) * (-adj)) / 255;

    if (c1 < 0)
        c1 = 0;
    else if (c1 > 255)
        c1 = 255;

    return c1;
}


/* cd_flipbitmap: Mirrors part of an image vertically. (Top becomes bottom).
 * Arguments: a bitmap to flip and the x1, y1 to x2, y2 area to flip.
 * Returns: 0 on success, -1 on error.
 */

int cd_flipbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2)
{
    int x, y, rgb, c, yy;

    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    for (y = y1; y <= y1 + ((y2 - y1) / 2); y++)
    {
        yy = y2 - (y - y1);

        for (x = x1; x <= x2; x++)
            for (rgb = 0; rgb < 3; rgb++)
            {
                c = cd_rawget(bitmap, x, y, rgb);
                cd_rawassign(bitmap, x, y, rgb, cd_rawget(bitmap, x, yy, rgb));
                cd_rawassign(bitmap, x, yy, rgb, c);
            }
    }

    return 0;
}


/* cd_mirrorbitmap: Mirrors part of an image horizontaly. (Left becomes right).
 * Arguments: a bitmap to mirror and the x1, y1 to x2, y2 area to mirror.
 * Returns: 0 on success, -1 on error.
 */

int cd_mirrorbitmap(BITMAP bitmap, int x1, int y1, int x2, int y2)
{
    int x, y, rgb, c, xx;

    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);

    for (x = x1; x <= x1 + ((x2 - x1) / 2); x++)
    {
        xx = x2 - (x - x1);

        for (y = y1; y <= y2; y++)
            for (rgb = 0; rgb < 3; rgb++)
            {
                c = cd_rawget(bitmap, x, y, rgb);
                cd_rawassign(bitmap, x, y, rgb, cd_rawget(bitmap, xx, y, rgb));
                cd_rawassign(bitmap, xx, y, rgb, c);
            }
    }

    return 0;
}


/* cd_enlargebitmap: Enlarges the size of a bitmap, leaving existing contents
   unaltered.
 * Arguments: a bitmap to enlarge, the width and height to change it to,
 and the location to place the previous contents within the new image.
 * Returns: bitmap number of new image on success, -1 on error.
 */

BITMAP cd_enlargebitmap(BITMAP bitmap, int width, int height, int x1, int y1)
{
    BITMAP newbitmap;


    if (cd_inited == 0 || cd_bitmapused[bitmap] == 0)
    {
        return -1;
    }

    if (width < cd_bitmapwidth[bitmap] || height > cd_bitmapheight[bitmap])
    {
        return -1;
    }

    if (x1 == CENTER)
        x1 = (width + cd_bitmapwidth[bitmap]) / 2;

    if (y1 == CENTER)
        y1 = (height + cd_bitmapheight[bitmap]) / 2;

    newbitmap = cd_newbitmap(width, height);

    if (newbitmap == -1)
    {
        return -1;
    }

    cd_pastebitmap(newbitmap, bitmap, ALL, ALL, ALL, ALL, x1, y1, 255);

    return newbitmap;
}


/* cd_inbounds: tells whether a location is within the bounds of a bitmap.
 * Arguments: bitmap you're testing for and x,y position
 * Returns: 0 if it's out of bounds, 1 if it's in bounds
 */

int cd_inbounds(BITMAP bitmap, int x, int y)
{
    if (x < 0 || y < 0 || x >= cd_bitmapwidth[bitmap] ||
        y >= cd_bitmapheight[bitmap])
        return 0;
    else
        return 1;
}


#ifdef DEF_OUT /* Kobus: We don't need this. */
int CD_randnum(int mod)
{
    int v;

    v = rand();

    return v % mod;
}
#endif


/* cd_trimbitmap: trims a bitmap by removing any exterior area of the
   image that's the same color, +/- a threshold.
 * Arguments: a bitmap to trim, the r, g and b color values of the
 exterior color to be considered excess, and a threshold value for
 that color.
 * Returns: a new bitmap on success, -1 on error
 */

BITMAP cd_trimbitmap(BITMAP bitmap, int r, int g, int b, int threshold)
{
    int left, top, right, bottom, x, y, found;

    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED)
    {
        return -1;
    }


    left = 0;
    top = 0;
    right = cd_bitmapwidth[bitmap] - 1;
    bottom = cd_bitmapheight[bitmap] - 1;


    /* Find top: */

    found = 0;
    for (y = top; y <= bottom && found == 0; y++)
        for (x = left; x <= right && found == 0; x++)
        {
            if (cd_rawget(bitmap, x, y, 0) < r - threshold ||
                cd_rawget(bitmap, x, y, 0) > r + threshold ||
                cd_rawget(bitmap, x, y, 1) < g - threshold ||
                cd_rawget(bitmap, x, y, 1) > g + threshold ||
                cd_rawget(bitmap, x, y, 2) < b - threshold ||
                cd_rawget(bitmap, x, y, 2) > b + threshold)
            {
                top = y;
                found = 1;
            }
        }


    /* Find bottom: */

    found = 0;
    for (y = bottom; y >= top && found == 0; y--)
        for (x = left; x <= right && found == 0; x++)
        {
            if (cd_rawget(bitmap, x, y, 0) < r - threshold ||
                cd_rawget(bitmap, x, y, 0) > r + threshold ||
                cd_rawget(bitmap, x, y, 1) < g - threshold ||
                cd_rawget(bitmap, x, y, 1) > g + threshold ||
                cd_rawget(bitmap, x, y, 2) < b - threshold ||
                cd_rawget(bitmap, x, y, 2) > b + threshold)
            {
                bottom = y;
                found = 1;
            }
        }


    /* Find left: */

    found = 0;
    for (x = left; x <= right && found == 0; x++)
        for (y = top; y <= bottom && found == 0; y++)
        {
            if (cd_rawget(bitmap, x, y, 0) < r - threshold ||
                cd_rawget(bitmap, x, y, 0) > r + threshold ||
                cd_rawget(bitmap, x, y, 1) < g - threshold ||
                cd_rawget(bitmap, x, y, 1) > g + threshold ||
                cd_rawget(bitmap, x, y, 2) < b - threshold ||
                cd_rawget(bitmap, x, y, 2) > b + threshold)
            {
                left = x;
                found = 1;
            }
        }


    /* Find right: */

    found = 0;
    for (x = right; x >= left && found == 0; x--)
        for (y = top; y <= bottom; y++)
        {
            if (cd_rawget(bitmap, x, y, 0) < r - threshold ||
                cd_rawget(bitmap, x, y, 0) > r + threshold ||
                cd_rawget(bitmap, x, y, 1) < g - threshold ||
                cd_rawget(bitmap, x, y, 1) > g + threshold ||
                cd_rawget(bitmap, x, y, 2) < b - threshold ||
                cd_rawget(bitmap, x, y, 2) > b + threshold)
            {
                right = x;
                found = 1;
            }
        }

    return cd_duplicatebitmap(bitmap, left, top, right, bottom);
}


/* cd_quantizebitmap: Quantizes the colors in a bitmap, reducing the
   number of unique colors found in the bitmap.
 * Arguments: a bitmap to quantize, the x1,y1 to x2,y2 area to quantize,
 and how many colors to reduce the bitmap to.
 * Returns: 0 on success, -1 on error
 */

int cd_quantizebitmap
(
 BITMAP bitmap,
 int    x1,
 int    y1,
 int    x2,
 int    y2,
 double colors,
 int    dither
)
{
    unsigned char r, g, b, r1, g1, b1;
    int x, y, num_colors, found, i, changed, rgb, difference, ofound = -1,
    odifference;
    double min, z;
    unsigned char color_table[32767][3];
    double used[32767];


    if (cd_inited == 0 || cd_bitmapused[bitmap] != USED || colors > 32767)
    {
        return -1;
    }

    CD_fixxy(bitmap, &x1, &y1, &x2, &y2);


    /* Collect the unique colors in this bitmap: */

    num_colors = 0;

    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            /* Get the color of a pixel: */

            r = cd_rawget(bitmap, x, y, 0);
            g = cd_rawget(bitmap, x, y, 1);
            b = cd_rawget(bitmap, x, y, 2);


            /* See if this color has been used before: */

            found = -1;

            for (i = 0; i < num_colors; i++)
            {
                r1 = color_table[i][0];
                g1 = color_table[i][1];
                b1 = color_table[i][2];

                if ((r - r1) * (r - r1) + (g - g1) * (g - g1) +
                    (b - b1) * (b - b1) < 100)
                    found = i;
            }

            if (found == -1)
            {
                /* Add this color to the collection table: */

                color_table[num_colors][0] = r;
                color_table[num_colors][1] = g;
                color_table[num_colors][2] = b;
                used[num_colors] = 1;

                num_colors = num_colors + 1;


                /* More colors than we care to deal with? */

                if (num_colors >= colors)
                {
                    num_colors = num_colors - 1;


                    /* Find the least used color: */

                    found = -1;
                    min = (x2 - x1 + 1) * (y2 - y1 + 1) + 1;

                    for (i = 0; i < num_colors; i++)
                    {
                        if (used[i] < min)
                        {
                            r1 = color_table[i][0];
                            g1 = color_table[i][1];
                            b1 = color_table[i][2];

                            if ((r - r1) * (r - r1) + (g - g1) * (g - g1) +
                                (b - b1) * (b - b1) > 100)
                            {
                                found = i;
                                min = used[i];
                            }
                        }
                    }


                    color_table[found][0] = r;
                    color_table[found][1] = g;
                    color_table[found][2] = b;
                    used[found] = 1;
                }
            }
            else
            {
                /* Increment how many times this color's been used: */

                used[found] = used[found] + 1;
            }
        }
    }


    /* Sort the colors by how much they're used: */

    do
    {
        changed = 0;

        for (i = 0; i < num_colors - 1; i++)
        {
            if (used[i] < used[i + 1])
            {
                z = used[i];
                used[i] = used[i + 1];
                used[i + 1] = z;

                for (rgb = 0; rgb < 3; rgb++)
                {
                    z = color_table[i][rgb];
                    color_table[i][rgb] = color_table[i + 1][rgb];
                    color_table[i + 1][rgb] = (unsigned char)z;
                }

                changed = 1;
            }
        }
    }
    while (changed == 1);


    /* Apply the most used colors onto the image: */

    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            /* Get the current pixel value: */

            r = cd_rawget(bitmap, x, y, 0);
            g = cd_rawget(bitmap, x, y, 1);
            b = cd_rawget(bitmap, x, y, 2);


            /* Find the closest color to that: */

            found = -1;
            difference = 255 * 3 + 1;
            odifference = difference;

            for (i = 0; i < colors; i++)
            {
                z = (abs(color_table[i][0] - r) +
                     abs(color_table[i][1] - g) +
                     abs(color_table[i][2] - b));

                if (z < difference)
                {
                    if (odifference - difference < 10)
                        ofound = found;
                    found = i;

                    odifference = difference;
                    difference = (int)z;
                }
            }


            /* If we're dithering, and we're on an odd pixel, and we found a
               semi-close color last time, and the current color isn't an exact
               match, then pick the previously-picked most-close match... */

            if (dither == 1 && ((y + x) % 2) == 1 && ofound != -1
                && difference != 0)
                found = ofound;


            /* Assing the pixel to the looked-up closest-matching color: */

            for (rgb = 0; rgb < 3; rgb++)
                cd_rawassign(bitmap, x, y, rgb, color_table[found][rgb]);
        }
    }

    return 0;  /* Kobus: 05-01-22. */
}


/*
 * Kobus, 05-01-22: The globals are a mess. Making the statics, and hacking up
 * functions to give the values out. Slows things down, but likely better to
 * provide higher level functionality from this file.
 */

int cd_get_font_height(int font)
{
    return cd_font_height[font];
}


int cd_get_bitmap_width(int bitmap)
{
    return cd_bitmapwidth[bitmap];
}


unsigned char cd_get_map_value(int bitmap, int index)
{
    return cd_map[ bitmap ][ index];
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

