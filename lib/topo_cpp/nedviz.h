/**
 * @file
 * @brief interface for NED visualization functionality
 * @author Andrew Predoehl
 */
/*
 * $Id: nedviz.h 17606 2014-09-26 01:09:51Z predoehl $
 */

#ifndef NEDVIZ_H_UOFARIZONA_VISION
#define NEDVIZ_H_UOFARIZONA_VISION

#include <m_cpp/m_matrix.h>
#include <i_cpp/i_image.h>

namespace kjb
{

/**
 * @brief show NED grid output in visual form
 * @param elev_o        Grid of elevation values
 * @param elev_de_o     Grid of east-west elevation gradient values
 * @param elev_dn_o     Grid of north-south elevation gradient values
 * @param decimation    Optional decimation factor, to shrink output size
 * @param bar_length    Optional length of key color bars in top left
 * @return Image showing elevation, which looks sort of like a relief map.
 * @throws KJB_error if the input matrices are not all the same size
 *
 * The first 3 parameters must all be the same size and are expected to be the
 * outputs of an elevation grid such as a ned13_grid.  The decimation factor
 * can shrink the output image, which might be convenient.  For example, if
 * decimation==3 then the output will have only one-third the number of rows
 * and columns as elev_o.
 *
 * The bar_length parameter defaults to 0, which means "do not draw bars."
 * If bar_length is positive, it sets the length, in pixels, of lines drawn at
 * the top left corner, which remind the viewer of which color indicates
 * which gradient:  red shows incline uphill going west to east, green shows
 * incline uphill going north to south.  The latter is arguably backwards, but
 * our human visual habits expect illumination from "above," and so these
 * visualizations look like there is green light shining from "above," north,
 * and also red light shining from the left (west) edge, like a sunset.
 *
 * There is another tweak:  on many display media, each unit of green is
 * incrementally more salient than the same unit of red.  In fact it is kind of
 * obnoxious.  So, the green channel is diminished from the value it would
 * have in a perfectly equitable world.
 * Thus, a low flat area will probably look brown, not olive.  I call the
 * less-than-unity-gain the "tone it down, green" factor.
 *
 * The visualization normalizes all the inputs, so on very flat terrain, the
 * output visualization might be dominated by kriging artifacts or noise.
 */
Image ned_visualize_grid(
    const Matrix& elev_o,
    const Matrix& elev_de_o,
    const Matrix& elev_dn_o,
    int decimation = 1,
    int bar_length = 0
);

}
#endif
