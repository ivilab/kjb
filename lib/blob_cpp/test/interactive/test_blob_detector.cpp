#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"
#include "blob_cpp/blob_detector.h"
#include "l_cpp/l_word_list.h"

#include <iostream>
#include <vector>
#include <set>
#include <boost/format.hpp>

#include <boost/progress.hpp>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    Word_list filenames("images/image_*.jpg");
    vector<Image> images(filenames.size());
    for(int i = 0; i < filenames.size(); i++)
    {
        images[i] = Image(filenames[i]);
    }

    Blob_detector detect_blobs(4, 25);
    
    boost::progress_display progress(images.size());
    for(int i = 0; i < images.size(); i++)
    {
        Image& I = images[i];
        set<Blob> blobs = detect_blobs(I);
        for(set<Blob>::const_iterator blob_p = blobs.begin(); blob_p != blobs.end(); blob_p++)
        {
            I.draw_circle(blob_p->row, blob_p->col, blob_p->size / 2, 1, PixelRGBA(255.0f, 0.0f, 0.0f));
        }

        I.write(boost::str(boost::format("output/blobbed_image_%02d.jpg") % i));

        ++progress;
    }

    return EXIT_SUCCESS;
}

