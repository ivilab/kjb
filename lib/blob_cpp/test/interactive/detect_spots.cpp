#include <iostream>
#include "blob_cpp/blob_spot_detector.h"
#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"
#include "m_cpp/m_vector.h"
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

    Image I(argv[1]);
    Spot_detector detect_spots(I.get_num_rows(), I.get_num_cols(), 0.0, 20.0,
                               boost::lexical_cast<int>(argv[4]), boost::lexical_cast<int>(argv[2]),
                               boost::lexical_cast<int>(argv[3]), boost::lexical_cast<double>(argv[5]));

    const vector<Vector>& spots = detect_spots(I);
    for(vector<Vector>::const_iterator vector_p = spots.begin(); vector_p != spots.end(); vector_p++)
    {
        const Vector& cur_spot = *vector_p;
        cout << "Found spot at (row,col) = (" << cur_spot[1] << ", " << cur_spot[0] << ")\n";
        I.draw_circle(cur_spot[1], cur_spot[0], 6, 1, PixelRGBA(255.0f, 0.0f, 0.0f));
    }

    string filename = string(argv[1]).substr(string(argv[1]).rfind("/") + 1);
    I.write("output/spotted_" + filename);

    return EXIT_SUCCESS;
}

