#include <iostream>
#include <blob_cpp/blob_spot_detector.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    if(argc != 6)
    {
        cout << "Usage: detect_spots image-file min-spot-size max-spot-size min-brightness similarity" << endl;
        return EXIT_SUCCESS;
    }

    string image_file = argv[1];
    Image I(image_file);
    Spot_detector detect_spots(I.get_num_rows(), I.get_num_cols(), 0.0, 10.0,
                               boost::lexical_cast<int>(argv[4]), boost::lexical_cast<int>(argv[2]),
                               boost::lexical_cast<int>(argv[3]), boost::lexical_cast<double>(argv[5]));

    detect_spots(I);
    const vector<vector<pair<int, int> > >& spots = detect_spots.get_spot_coordinates();
    for(size_t sp = 0; sp < spots.size(); sp++)
    {
        for(size_t px = 0; px < spots[sp].size(); px++)
        {
            I(spots[sp][px].second, spots[sp][px].first, Image::RED) = 255.0;
            I(spots[sp][px].second, spots[sp][px].first, Image::GREEN) = 0.0;
            I(spots[sp][px].second, spots[sp][px].first, Image::BLUE) = 0.0;
        }
    }

    string filename = string(image_file).substr(string(image_file).rfind("/") + 1);
    I.write("output/spotted_fill_" + filename);

    return EXIT_SUCCESS;
}

