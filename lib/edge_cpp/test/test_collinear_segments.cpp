#include <vector>
#include <algorithm>

#include <edge_cpp/line_segment.h>
#include <edge_cpp/collinear_segment_chain.h>
#include <l_cpp/l_test.h>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    const double m = -6.0;
    const double b = 1.0;
    const int num_segments = 5;

    vector<Line_segment> segments;

    for(size_t i = 0; i < num_segments; i++)
    {
        double x = i;
        Vector start = Vector().set(x, m * x + b);

        x = i + 4/5.0;
        Vector end = Vector().set(x, m * x + b);

        segments.push_back(Line_segment(start, end));
    }

    random_shuffle(segments.begin(), segments.end());
    Collinear_segment_chain chain(segments);

    const double first_x = 0.0;
    const double first_y = m * first_x + b;
    TEST_TRUE(fabs(chain.get_start_x() - first_x) < FLT_EPSILON);
    TEST_TRUE(fabs(chain.get_start_y() - first_y) < FLT_EPSILON);

    const double last_x = num_segments - 1 + 4/5.0;
    const double last_y = m * last_x + b;
    TEST_TRUE(fabs(chain.get_end_x() - last_x) < FLT_EPSILON);
    TEST_TRUE(fabs(chain.get_end_y() - last_y) < FLT_EPSILON);

    RETURN_VICTORIOUSLY();
}

