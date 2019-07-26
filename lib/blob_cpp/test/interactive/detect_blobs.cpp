#include <iostream>
#include "blob_cpp/blob_detector.h"
#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"
#include <set>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    kjb_c::kjb_l_set("page", "off");
    if(argc != 4)
    {
        cout << "Usage: detect_blobs image_file min_blob_size max_blob_size" << endl;
        return EXIT_SUCCESS;
    }

    Image I(argv[1]);
    Blob_detector detect_blobs(boost::lexical_cast<int>(argv[2]), boost::lexical_cast<int>(argv[3]));

    const set<Blob>& blobs = detect_blobs(I);
    for(set<Blob>::const_iterator blob_p = blobs.begin(); blob_p != blobs.end(); blob_p++)
    {
        //cout << blob_p->size << ", " << blob_p->row << ", " << blob_p->col << endl;
        cout << "Found blob at (row,col) = (" << blob_p->row << ", " << blob_p->col << ") of size " << blob_p->size << ".\n";
        I.draw_circle(blob_p->row, blob_p->col, blob_p->size / 2, 1, PixelRGBA(255.0f, 0.0f, 0.0f));
    }

    string filename = string(argv[1]).substr(string(argv[1]).rfind("/") + 1);
    I.write("output/blobbed_" + filename);

    return EXIT_SUCCESS;
}

