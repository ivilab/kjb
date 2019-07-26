/**
 * @file
 * @brief implemenation of NED visualization function(s)
 * @author Andrew Predoehl
 */
/*
 * $Id: nedviz.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_std.h"
#include "i_cpp/i_pixel.h"
#include "i_cpp/i_filter.h"
#include "l_cpp/l_exception.h"
#include "topo_cpp/nedviz.h"

namespace
{
/*
 * The "tone it down, green!" factor:  each unit of green is incrementally
 * more salient than the same numerical unit of red or blue, on most computer
 * displays and to most human eyes.  So, we handicap green so it is less
 * dominant.  Red and blue channels in pixels range from 0 to almost 256, but
 * green ranges from 0 to almost 256*TIDG.  Factor TIDG is currently set to
 * 0.625, or five eighths.  So green ranges from 0 to almost 160.
 * Since 0.625 = 2**(-3) + 2**(-1) the multiplication can be done using
 * just two bit-shifts and addition, which I do below for old times' sake.
 */
const float TIDG = // 0.75; // "tone it down, green!"
                   0.625; // 12 sept 2014 -- 96 is still too rotten looking

// The following are for setting the intensity of the color channels of
// pixels in an 24-bit-deep color display (R, G, B each range 0 to 255).
const int FS = 0x100,             // full-scale (too big for a color channel)
          HS = FS>>1,             // half-scale
          FS_TIDG = (HS>>2) + HS, // alternate way to compute FS*TIDG
          HS_TIDG = FS_TIDG>>1;   // alternate way to compute HS*TIDG

const kjb::PixelRGBA RED  ( FS-6, 0,       0),
                     GREEN( 0,    FS_TIDG, 0);
}

namespace kjb
{

Image ned_visualize_grid(
    const Matrix& elev_o,
    const Matrix& elev_de_o,
    const Matrix& elev_dn_o,
    int decimation,
    int bar_length
)
{
    const int   rows = elev_o.get_num_rows(),
                cols = elev_o.get_num_cols();

    if (    elev_de_o.get_num_rows() != rows
        ||  elev_dn_o.get_num_rows() != rows
        ||  elev_de_o.get_num_cols() != cols
        ||  elev_dn_o.get_num_cols() != cols)
    {
        KJB_THROW_2(Dimension_mismatch, "Input sizes do not match");
    }

    if (decimation < 1)
    {
        KJB_THROW_2(Illegal_argument, "Decimation factor must be 1 or more");
    }

    const double    gradmax = std::max(max(elev_de_o), max(elev_dn_o)),
                    gradmin = std::min(min(elev_de_o), min(elev_dn_o)),
                    grad_amp = std::max(gradmax, -gradmin),
                    elevmax = max(elev_o),
                    elevmin = min(elev_o);

    if (elevmax == elevmin || gradmax == gradmin)
    {
        return Image(rows, cols, HS, HS_TIDG, HS);
    }

    const double    gnorm = (HS-1) / grad_amp,
                    enorm = (FS-1) / (elevmax - elevmin);

    const Matrix    GA(rows, cols, grad_amp),
                    ne = (elev_de_o + GA) * gnorm,
                    ns = (GA - elev_dn_o) * gnorm * TIDG,
                    nv = (elev_o - Matrix(rows, cols, elevmin)) * enorm;

    Image i1 = rgb_matrices_to_image(ne, ns, nv);

    if (decimation > 1)
    {
        Image i2(kjb::gauss_sample_image(i1, decimation, decimation));
        i1.swap(i2);
    }
    if (bar_length > 0)
    {
        i1.draw_aa_rectangle(1, 0, 1+bar_length, 0, RED);   // vertical
        i1.draw_aa_rectangle(0, 1, 0, 1+bar_length, GREEN); // horizontal
    }
    return i1;
}

}

