#include <wrap_dtlib_cpp/texture.h>
#include <wrap_dtlib_cpp/textonhisto.h>
#include <iostream>

using namespace DTLib;
using namespace kjb;
int main(int argc, char ** argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: ./main2 <base_dir> <image_name>" << std::endl;
        return 1;
    }
    std::string img_name(argv[1]);
    img_name.append("/");
    img_name.append(argv[2]);
    img_name.append(".jpg");

    kjb::Image img(img_name.c_str());

    if(kjb_c::is_black_and_white(img.c_ptr()))
    {
        std::cout << "Black and white" << std::endl;
    }
    else
    {
        std::cout << "Colour" << std::endl;
    }

    return 0;
}

