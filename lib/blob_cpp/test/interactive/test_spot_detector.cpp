#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"
#include "m_cpp/m_matrix.h"
#include "blob_cpp/blob_spot_detector.h"
#include "l_cpp/l_word_list.h"
#include "prob_cpp/prob_stat.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <boost/format.hpp>

using namespace std;
using namespace kjb;

Matrix image_std_dev(const vector<Image>& imgs, const Image& avg)
{
    Matrix sd(avg.get_num_rows(), avg.get_num_cols(), 0.0);
    for(int i = 0; i < imgs.size(); i++)
    {
        Matrix mat = imgs[i].to_grayscale_matrix() - avg.to_grayscale_matrix();
        for(int j = 0; j < mat.get_length(); j++)
        {
            mat(j) *= mat(j);
        }
        sd += mat;
    }
    sd /= (imgs.size() - 1);

    for(int j = 0; j < sd.get_length(); j++)
    {
        sd(j) = std::sqrt(sd(j));
    }

    return sd;
}

int main(int argc, char** argv)
{
    Word_list filenames("images/image_*.jpg");
    vector<Image> images(filenames.size());
    for(int i = 0; i < filenames.size(); i++)
    {
        images[i] = Image(string(filenames[i]));
    }

    Image mean_img = mean(images.begin(), images.end());
    Matrix std_dev_mat = image_std_dev(images, mean_img);
    Spot_detector detect_spots(mean_img.to_grayscale_matrix(), std_dev_mat, 20, 20, 300, 0.8);
    
    for(int i = 0; i < images.size(); i++)
    {
        Image& I = images[i];
        vector<Vector> spots = detect_spots(I);
        for(vector<Vector>::const_iterator vector_p = spots.begin(); vector_p != spots.end(); vector_p++)
        {
            const Vector& cur_spot = *vector_p;
            I.draw_circle(cur_spot[1], cur_spot[0], 6, 1, PixelRGBA(255.0f, 0.0f, 0.0f));
        }

        I.write(boost::str(boost::format("output/spotted_image_%02d.jpg") % i));
    }

    return EXIT_SUCCESS;
}

