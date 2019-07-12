#include <edge_cpp/feature_histogram.h>
#include <wrap_dtlib_cpp/texture.h>
#include <i_cpp/i_image.h>

#include <ostream>
#include <iostream>

using namespace kjb;
using namespace DTLib; 
using namespace std;
int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cout << "Usuage: " << argv[0] << " image-fp\n";
        return EXIT_SUCCESS;
    }
    string image_fp = argv[1];
    Image image(image_fp);

    CImg<FloatCHistogramPtr>* texton_histo = NULL;
    CImg<FloatCHistogramPtr>* color_histo = NULL;
    Compute_All_Histograms(image, &texton_histo, &color_histo);

    string thist_fp("./output/text_hist.his");
    string chist_fp("./output/color_hist.his");

    WriteColorHistoImg(chist_fp, color_histo);
    WriteColorHistoImg(thist_fp, texton_histo);

    Feature_histogram histo(*color_histo, 5, image.get_num_rows(), image.get_num_cols(), 10);

    std::cout << "Num darts:" << histo.get_num_darts() << std::endl;
    Image img2;
    histo.draw_darts(img2);
    img2.write("./output/darts.tiff");
    zap(color_histo);
    zap(texton_histo);
    
    return EXIT_SUCCESS;
}
