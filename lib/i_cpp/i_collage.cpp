/**
 * @file
 * @brief wrapper for kjb_c function to make a collage
 */
/*
 * $Id: i_collage.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "i_cpp/i_collage.h"
#include "i_cpp/i_image.h"
#include "l_cpp/l_exception.h"

#include <vector>

namespace {

size_t positive_product(int h, int v)
{
    if (h < 1 || v <1)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Collage sizes must be positive");
    }
    return h*v;
}

}

namespace kjb {

Image make_collage(
    Image const* const* input_images,
    int num_horizontal,
    int num_vertical
)
{
    const size_t count = positive_product(num_horizontal, num_vertical);

    kjb_c::KJB_image* output = 0;
    std::vector<const kjb_c::KJB_image*> i2(count);
    for (size_t iii = 0; iii < count; ++iii)
    {
        i2.at(iii) = input_images[iii] -> c_ptr();
    }

    ETX(kjb_c::make_image_collage(&output, num_horizontal, num_vertical,
                                                    & i2.front()));
    return Image(output);
}


Image make_collage(
    Image const* input_img_array,
    int num_horizontal,
    int num_vertical
)
{
    const size_t count = positive_product(num_horizontal, num_vertical);

    std::vector< const kjb::Image* > in(count, 0);
    for (size_t iii = 0; iii < count; ++iii)
    {
        in.at(iii) = input_img_array + iii;
    }
    return make_collage(& in.front(), num_horizontal, num_vertical);
}

}

