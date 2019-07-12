#include <l/l_sys_io.h>
#include <flow_cpp/flow_feature_set.h>
#include <flow_cpp/flow_visualizer.h>
#include <gr_cpp/gr_2D_bounding_box.h>

#include <string>
#include <vector>

using namespace kjb; 
using namespace std;

int main(int argc, char** argv)
{
    string flow_fp = "input/01_of.txt";
    string image_fp = "input/01.jpg";
    Image img(image_fp);
    Flow_feature_set ffs = read_flow_features(flow_fp);

    Axis_aligned_rectangle_2d box(Vector(300.0, 400.0), 100.0, 200.0);
    vector<Feature_pair> fv = look_up_features(ffs, box);

    const double percentile = 1.0;
    vector<Vector> vfv = valid_flow(fv, percentile); 
    Vector avf = average_flow(vfv);

    draw_features(img, fv, avf);

    kjb_c::kjb_mkdir("output/");
    img.write("output/01_flow.jpg");
    

    return EXIT_SUCCESS;
}
