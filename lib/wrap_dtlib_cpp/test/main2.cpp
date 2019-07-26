#include <wrap_dtlib_cpp/texture.h>
#include <wrap_dtlib_cpp/textonhisto.h>
#include <iostream>

using namespace DTLib;
using namespace kjb;
int main(int argc, char ** argv)
{
    if(argc != 4)
    {
        std::cout << "Usage: ./main2 <base_dir> <out_dir> <image_name>" << std::endl;
        return 1;
    }
    std::string img_name(argv[1]);
    img_name.append("/");
    img_name.append(argv[3]);
    img_name.append(".jpg");
    std::string ochi(argv[2]);
    ochi.append("/");
    ochi.append(argv[3]);
    std::string othi(ochi);
    ochi.append("_chi");
    othi.append("_thi");

    kjb::Image img(img_name.c_str());
    CImg<FloatCHistogramPtr> * m_pTextonHistoImg;
    CImg<FloatCHistogramPtr> * ColorHist;
    Compute_All_Histograms(img, &m_pTextonHistoImg, &ColorHist);
    std::cout << "Now write" << std::endl;
    WriteTextonHistoImg(othi.c_str(),m_pTextonHistoImg );
    WriteColorHistoImg(ochi.c_str(),ColorHist );

    zap(m_pTextonHistoImg);
    zap(ColorHist);
    return 0;
}

