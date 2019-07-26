#include <flow_cpp/flow_dense.h>
#include <m_cpp/m_matrix.h>
#include <l/l_sys_io.h>
#include <string>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    string x_flow_fp("input/x_flows.txt.gz");
    string y_flow_fp("input/y_flows.txt.gz");

    Matrix x_flows(x_flow_fp);
    Matrix y_flows(y_flow_fp);
    Matrix mag = flow_magnitude(x_flows, y_flows);

    double max_val = max(mag);
    mag *= 255.0/max_val; 
    Image mag_img(mag);

    kjb_c::kjb_mkdir("output/");
    mag.write("output/mag.txt");
    mag_img.write("output/mag.jpg");


}
