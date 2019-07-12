#include <m_cpp/m_vector.h>
#include <flow_cpp/flow_feature_set.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <l_cpp/l_test.h>
#include <iostream>

using namespace kjb;
using namespace std;

typedef Axis_aligned_rectangle_2d Bbox;

int main(int argc, char** argv)
{
    kjb_c::kjb_init();
    Flow_feature_set of_set;
    Flow_feature_set sparse_of_set;
    const double min_x = 0.0;
    const double max_x = 100.0;
    const double min_y = 0.0;
    const double max_y = 100.0;
    const double center_x = 50.0;
    const double center_y = 50.0;
    const double width = 40.0;
    const double height = 40.0;
    const double f_x = 2.0;
    const double f_y = 2.0;
    double left = center_x - width/2.0;
    double right = center_x + width/2.0;
    double bottom = center_y - height/2.0;
    double top = center_y + height/2.0;
    size_t left_px = left;
    // size_t right_px = std::ceil(right);
    size_t right_px = ceil(right);
    size_t bottom_px = bottom;
    // size_t top_px = std::ceil(top);
    size_t top_px = ceil(top);
    double dx, dy; 
    for(size_t row = min_y; row < max_y; row = row + 1)
    {
        if(row >= bottom_px && row < top_px)
        {
            dy = f_y;
        }
        else
        {
            dy = kjb_c::kjb_rand();
        }

        for(size_t col = min_x; col < max_x; col = col + 1)
        {
            if(col >= left_px && col < right_px)
            {
                dx = f_x; 
            }
            else
            {
                dx = kjb_c::kjb_rand();
            }
            Feature_pair pair(Vector((double)col, (double)row), 
                              Vector((double)(col + dx), (double)(row + dy)));
            of_set.insert(pair);
        }
    }

    Bbox box(Vector(center_x, center_y), width, height);
    double area = box.get_width() * box.get_height();
    const double thresh = FLT_EPSILON;
    Vector f(f_x, f_y);
    Vector tf = total_flow(of_set, box);
    tf /= area; 
    TEST_TRUE(fabs(vector_distance(tf, f) < thresh));

    RETURN_VICTORIOUSLY();

}

