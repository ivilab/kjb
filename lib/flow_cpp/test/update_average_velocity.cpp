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

    // Create sytheticc features
    Flow_feature_set of_set;
    const double min_x = 0.0;
    const double max_x = 300.0;
    const double min_y = 0.0;
    const double max_y = 300.0;
    double dx, dy;

    for(size_t row = min_y; row < max_y; row++)
    {
        for(size_t col = min_x; col < max_x; col = col++)
        {
            dy = 100.0 * kjb_c::kjb_rand();
            dx = 100.0 * kjb_c::kjb_rand();
            Feature_pair pair(Vector((double)col, (double)row),
                              Vector((double)(col + dx), (double)(row + dy)));
            of_set.insert(pair);
        }
    }

    const double threshold = FLT_EPSILON;
    const double center_x = 50;
    const double center_y = 50;
    const double width = 20;
    const double height = 20;
    int unit = 1.0;

    Bbox box(Vector(center_x, center_y), width, height);
    double area = box.get_width() * box.get_height();
    Vector old_flow = total_flow(of_set, box);
    old_flow /= area;

    // move right
    Bbox new_box(Vector(center_x + unit, center_y), width, height);
    Vector new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    Vector new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move left
    new_box.set_center(Vector(center_x - unit, center_y));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move UP
    new_box.set_center(Vector(center_x, center_y + unit));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // MOVE DOWN
    new_box.set_center(Vector(center_x, center_y - unit));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    /////////////////////////////////////////////////////////
    //          Create more complicatedd boxes             //
    /////////////////////////////////////////////////////////

    // move a bit more
    new_box.set_center(Vector(center_x + 4.0, center_y - 3.0));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move a lot
    new_box.set_center(Vector(70.0, 70.0));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // change width and height
    new_box.set_center(Vector(center_x, center_y));
    new_box.set_height(new_box.get_height() * 0.8);
    new_box.set_width(new_box.get_width() * 1.2);
    new_flow = total_flow(of_set, new_box);
    area = new_box.get_height() * new_box.get_width();
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // change center (with width and height changed)
    new_box.set_center(Vector(center_x + 3.0, center_y + 7.0));
    new_flow = total_flow(of_set, new_box);
    new_flow /= area;
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    RETURN_VICTORIOUSLY();
}

