// throwaway code

#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_cpp_incl.h>

#include <gr_cpp/gr_2D_bounding_box.h>

#include <iostream>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <i/i_float.h>

#include <psi_cpp/psi_human_box.h>

using namespace kjb::psi;

void draw_boxes(kjb::Image& img, const std::vector<kjb::Bounding_Box2D>& boxes)
{
    for(size_t i = 0; i < boxes.size(); i++)
    {
        boxes[i].draw(img, 0., 0., 255.);
    }
}

void print_usage(const char* cmd_name)
{
    std::cout << "Usage: " << cmd_name << " image_fname boxes_fname out_fname" << std::endl;
}

int main(int argc, char* argv[])
{
    using std::string;
    using kjb::Image;
    using std::vector;
    // image filename, boxes filename
    // output image with boxes
    //
    if(argc != 4)
    {
        print_usage(argv[0]);
        exit(1);
    }

    string img_fname(argv[1]);
    string box_fname(argv[2]);
    string out_fname(argv[3]);

    vector<kjb::Bounding_Box2D> boxes;

    Image img_in(img_fname);
    std::ifstream ifs(box_fname.c_str());
    std::vector<Human_boxes> actor_boxes = parse_human_boxes(ifs);
    Image img_out = img_in;

    for(size_t actor = 0; actor < actor_boxes.size(); actor++)
    {
        draw_boxes(img_out, actor_boxes[actor]);
    }

    img_out.write(out_fname);

    return 0;
}


