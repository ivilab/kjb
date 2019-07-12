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
    const double max_x = 1280.0;
    const double min_y = 0.0;
    const double max_y = 720.0;
    double dx, dy;

    for(size_t row = min_y; row < max_y; row++)
    {
        for(size_t col = min_x; col < max_x; col++)
        {
            dx = 100.0 * kjb_c::kjb_rand();
            dy = 100.0 * kjb_c::kjb_rand();
            Feature_pair pair(Vector((double)col, (double)row),
                              Vector((double)(col + dx), (double)(row + dy)));
            of_set.insert(pair);
        }
    }

    const double threshold = FLT_EPSILON;

    // Box parameters
    //const double center_x = 100.0 + 1000.0 * kjb_c::kjb_rand();
    //const double center_y = 100.0 + 1000.0 * kjb_c::kjb_rand();
    const double center_x = 600.0;
    const double center_y = 600.0;
    const double width = 400.0;
    const double height = 200.0;
    Bbox box(Vector(center_x, center_y), width, height);
    double area = box.get_width() * box.get_height();

    Vector old_flow = total_flow(of_set, box);
    old_flow /= area;

    const int unit = 2.0;
    // move right
    Bbox new_box(Vector(center_x + unit, center_y), width, height);
    kjb_c::init_cpu_time();
    Vector new_flow = total_flow(of_set, new_box);
    long cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    Vector new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move left
    new_box.set_center(Vector(center_x - unit, center_y));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0; 
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move UP
    new_box.set_center(Vector(center_x, center_y + unit));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // MOVE DOWN
    new_box.set_center(Vector(center_x, center_y - unit));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);


    /////////////////////////////////////////////////////////
    //          Create more complicatedd boxes             //
    /////////////////////////////////////////////////////////

    // move a bit more
    new_box.set_center(Vector(center_x + 4.0, center_y - 3.0));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //cout << vector_distance(new_flow, new_flow_est) << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // move a lot
    new_box.set_center(Vector(70.0, 70.0));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // change width and height
    new_box.set_center(Vector(center_x, center_y));
    new_box.set_height(new_box.get_height() * 0.8);
    new_box.set_width(new_box.get_width() * 1.2);
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    area = new_box.get_height() * new_box.get_width();
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    // change center (with width and height changed)
    new_box.set_center(Vector(center_x + 3.0, center_y + 7.0));
    kjb_c::init_cpu_time();
    new_flow = total_flow(of_set, new_box);
    cur_time = kjb_c::get_cpu_time();
    cout << cur_time/1000.0;
    new_flow /= area;
    kjb_c::init_cpu_time();
    new_flow_est = update_average_velocity(of_set, box, new_box, old_flow);
    cur_time = kjb_c::get_cpu_time();
    cout << " vs. " << cur_time/1000.0 << endl;
    //TEST_TRUE(vector_distance(new_flow, new_flow_est) < threshold);

    RETURN_VICTORIOUSLY();
}

