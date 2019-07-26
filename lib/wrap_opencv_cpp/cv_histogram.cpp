/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: cv_histogram.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>

#include <wrap_opencv_cpp/cv_util.h>
#include <wrap_opencv_cpp/cv_histogram.h>
#include <fstream>


using namespace kjb;
using namespace kjb::opencv;

Matrix calculate_histogram
(
    const Matrix src,
    int num_bins,
    float lower_bound,
    float upper_bound
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;
    Mat cv_src = to_opencv(src);
    
    float range[] = {lower_bound, upper_bound};
    const float* ranges = { range };
    bool uniform = true;
    bool accumulate = false;
    int channels[] = {0};
    const int nimages = 1;
    const int dim = 1;
    Mat hist;
    calcHist(&cv_src, nimages, channels, Mat(), hist, dim, &num_bins, &ranges, 
                              uniform, accumulate);
    // Normalize the histogram
    normalize(hist, hist, 0, 1, NORM_L1);


    Matrix histogram(num_bins, 1);
    for(int i = 0; i < num_bins; i++)
    {
        histogram(i, 0) = hist.at<float>(i);
    }

    return histogram;
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::opencv::compare_histograms
(
    const Matrix& h1, 
    const Matrix& h2, 
    int method
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;
    int cv_method; 
    switch(method)
    {
        case CORRELATION: cv_method = CV_COMP_CORREL;
            break;
        case CHI_SQUARE: cv_method = CV_COMP_CHISQR;
            break;
        case INTERSECTION: cv_method = CV_COMP_INTERSECT;
            break;
        case BHATTACHARYYA: cv_method = CV_COMP_BHATTACHARYYA;
            break;
        default: KJB_THROW(Not_implemented);
    }

    Mat cv_h1 = to_opencv(h1);
    Mat cv_h2 = to_opencv(h2);
    return compareHist(cv_h1, cv_h2, cv_method);
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::opencv::calculate_rg_histogram
(
     const Image& img, 
     const Axis_aligned_rectangle_2d& box,
     int red_bins,
     int green_bins
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;

    // convert to cv::Mat
    cv::Mat src(img.get_num_rows(), img.get_num_cols(), CV_32FC3);
    float max_red = 0.0f;
    float min_red = FLT_MAX;
    float max_green = 0.0f;
    float min_green = FLT_MAX;
    // Convert to r, g, S space
    for(int r = 0; r < src.rows; r++)
    {
        for(int c = 0; c < src.cols; c++)
        {
            float red = img(r, c).r;
            float green = img(r, c).g;
            float blue = img(r, c).b;
            float S = red + green + blue;
            if( S > 0.0)
            {
                double dr = red / S;
                double dg = green / S;
                src.at<Vec3f>(r, c)[0] = dr;
                src.at<Vec3f>(r, c)[1] = dg;
                src.at<Vec3f>(r, c)[2] = S;
                /*if(dr > max_red)
                {
                    max_red = dr; 
                }
                if(dg > max_green)
                {
                    max_green = dg;
                }
                if(dr < min_red)
                {
                    min_red = dr; 
                }
                if(dg < min_green)
                {
                    min_green = dg; 
                }
                */
            }
            else
            {
                src.at<Vec3f>(r, c)[0] = 0.0f;
                src.at<Vec3f>(r, c)[1] = 0.0f;
                src.at<Vec3f>(r, c)[2] = 0.0f;
            }
        }
    }

    //imwrite("src.jpg" , src);
  
    //std::cout << max_red << " " << max_green <<  std::endl;
    //std::cout << " showing source \n";

    //float r_ranges[] = { 0.0, max_r};
    //float g_ranges[] = { 0.0, max_green};
    float r_ranges[] = {0.0f, 1.0f};
    float g_ranges[] = {0.0f, 1.0f};
    const float* ranges[] = { r_ranges, g_ranges};
    int hist_size[] = { red_bins, green_bins};
    int channels[] = { 0, 1};

    const kjb::Vector& center = box.get_center();
    double width = box.get_width();
    double height = box.get_height();
    size_t min_c = std::max(0, (int)std::floor(center[0] - width/2.0));
    size_t min_r = std::max(0, (int)std::floor(center[1] - height/2.0));
    size_t max_c = std::min(src.cols - 1, 
                            (int)std::ceil(center[0] + width/2.0));
    size_t max_r = std::min(src.rows - 1, 
                            (int)std::ceil(center[1] + height/2.0));

    Mat mask(src.rows, src.cols, CV_8UC1, Scalar(0));
    for(size_t r = min_r; r < max_r; r++)
    {
        for(size_t c = min_c; c < max_c; c++)
        {
            mask.at<char>(r, c) = 255;
        }
    }

    const int num_images = 1;
    const int dims = 2;
    bool uniform = true;
    bool accumulate = false;
       
    Mat hist(red_bins, green_bins, cv::DataType<float>::type); 
    calcHist(&src, num_images, channels, mask, hist, dims, hist_size, 
            ranges, uniform, accumulate);
    double sum = 0.0;
    for (size_t r = 0; r < hist.rows; r++)
    {
        for(size_t c = 0; c < hist.cols; c++)
        {
            sum += hist.at<float>(r, c);
        }
    }

    for (size_t r = 0; r < hist.rows; r++)
    {
        for(size_t c = 0; c < hist.cols; c++)
        {
            hist.at<float>(r, c) = hist.at<float>(r, c) / sum;
        }
    }

    return opencv_to_kjb(hist);
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::opencv::calculate_hs_histogram
(
     const Image& img, 
     const Axis_aligned_rectangle_2d& box,
     int hue_bins,
     int saturation_bins
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;

    // convert to cv::Mat
    Mat src = to_opencv(img);
    Mat hsv;
    cvtColor(src, hsv, CV_BGR2HSV);

    float h_ranges[] = {0.0, 256.0};
    float s_ranges[] = {0.0, 180.0};
    const float* ranges[] = { h_ranges, s_ranges};
    int hist_size[] = { hue_bins, saturation_bins};
    int channels[] = { 0, 1};

    const kjb::Vector& center = box.get_center();
    double width = box.get_width();
    double height = box.get_height();
    size_t min_c = std::max(0, (int)std::floor(center[0] - width/2.0));
    size_t min_r = std::max(0, (int)std::floor(center[1] - height/2.0));
    size_t max_c = std::min(src.cols - 1, 
                            (int)std::ceil(center[0] + width/2.0));
    size_t max_r = std::min(src.rows - 1, 
                            (int)std::ceil(center[1] + height/2.0));

    Mat mask(src.rows, src.cols, CV_8UC1, Scalar(0));
    for(size_t r = min_r; r < max_r; r++)
    {
        for(size_t c = min_c; c < max_c; c++)
        {
            mask.at<char>(r, c) = 255;
        }
    }

    const int num_images = 1;
    const int dims = 2;
    bool uniform = true;
    bool accumulate = false;
        
    Mat hist; 
    calcHist(&hsv, num_images, channels, mask, hist, dims, hist_size, 
            ranges, uniform, accumulate);
    // normalize
    double s = norm(hist, NORM_L1);
    double sum = 0.0;
    for (size_t r = 0; r < hist.rows; r++)
    {
        for(size_t c = 0; c < hist.cols; c++)
        {
            sum += hist.at<float>(r, c);
        }
    }

    for (size_t r = 0; r < hist.rows; r++)
    {
        for(size_t c = 0; c < hist.cols; c++)
        {
            hist.at<float>(r, c) = hist.at<float>(r, c) / sum;
        }
    }
    
    // Convert to kjb::Matrix 
    std::ofstream ofs("hist.txt");
    ofs << hist;
    std::ofstream ofsk("hist_kjb.txt");
    ofsk << opencv_to_kjb(hist);
    return opencv_to_kjb(hist);
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Matrix> kjb::opencv::calculate_rg_histograms
(
     const Image& img, 
     const std::vector<Axis_aligned_rectangle_2d> & boxes,
     int red_bins,
     int green_bins
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;
    std::vector<Matrix> histos; 

    // convert to cv::Mat

    cv::Mat src(img.get_num_rows(), img.get_num_cols(), CV_32FC3);
    float max_red = 0.0f;
    float max_green = 0.0f;
    // Convert to r, g, S space
    for(int r = 0; r < src.rows; r++)
    {
        for(int c = 0; c < src.cols; c++)
        {
            float red = img(r, c).r;
            float green = img(r, c).g;
            float blue = img(r, c).b;
            float S = red + green + blue;
            if( S > 0.0)
            {
                src.at<Vec3f>(r, c)[0] = red/S;
                src.at<Vec3f>(r, c)[1] = green/S;
                src.at<Vec3f>(r, c)[2] = S;
                if( red/S > max_red)
                {
                    max_red = red / S; 
                }
                if( green/S > max_green)
                {
                    max_green = green / S;
                }
            }
            else
            {
                src.at<Vec3f>(r, c)[0] = 0.0f;
                src.at<Vec3f>(r, c)[1] = 0.0f;
                src.at<Vec3f>(r, c)[2] = 0.0f;
            }
        }
    }

    float r_ranges[] = { 0.0, max_red};
    float g_ranges[] = { 0.0, max_green};
    const float* ranges[] = { r_ranges, g_ranges};
    int hist_size[] = { red_bins, green_bins};
    int channels[] = { 0, 1};

    for(size_t i = 0; i < boxes.size(); i++)
    {
        const kjb::Vector& center = boxes[i].get_center();
        double width = boxes[i].get_width();
        double height = boxes[i].get_height();
        size_t min_c = std::max(0, (int)std::floor(center[0] - width/2.0));
        size_t min_r = std::max(0, (int)std::floor(center[1] - height/2.0));
        size_t max_c = std::min(src.cols - 1, 
                                (int)std::ceil(center[0] + width/2.0));
        size_t max_r = std::min(src.rows - 1, 
                                (int)std::ceil(center[1] + height/2.0));

        Mat mask(src.rows, src.cols, CV_8UC1, Scalar(0));
        for(size_t r = min_r; r < max_r; r++)
        {
            for(size_t c = min_c; c < max_c; c++)
            {
                mask.at<char>(r, c) = 255;
            }
        }
        //boost::format fmt("mask-%d.jpg");
        //imwrite((fmt % i).str(), mask);

        const int num_images = 1;
        const int dims = 2;
        bool uniform = true;
        bool accumulate = false;
            
        Mat hist; 
        calcHist(&src, num_images, channels, mask, hist, dims, hist_size, 
                ranges, uniform, accumulate);
        double sum = 0.0;
        for (size_t r = 0; r < hist.rows; r++)
        {
            for(size_t c = 0; c < hist.cols; c++)
            {
                sum += hist.at<float>(r, c);
            }
        }

        for (size_t r = 0; r < hist.rows; r++)
        {
            for(size_t c = 0; c < hist.cols; c++)
            {
                hist.at<float>(r, c) = hist.at<float>(r, c) / sum;
            }
        }

        histos.push_back(opencv_to_kjb(hist));
    }
    return histos;
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}

std::vector<Matrix> kjb::opencv::calculate_hs_histograms
(
     const Image& img, 
     const std::vector<Axis_aligned_rectangle_2d> & boxes,
     int hue_bins,
     int saturation_bins
)
{
#ifdef KJB_HAVE_OPENCV
    using namespace cv;
    std::vector<Matrix> histos; 

    // convert to cv::Mat
    Mat src = to_opencv(img);
    Mat hsv;
    cvtColor(src, hsv, CV_BGR2HSV);

    float h_ranges[] = { 0.0, 256.0};
    float s_ranges[] = { 0.0, 180.0};
    const float* ranges[] = { h_ranges, s_ranges};
    int hist_size[] = { hue_bins, saturation_bins};
    int channels[] = { 0, 1};

    for(size_t i = 0; i < boxes.size(); i++)
    {
        const kjb::Vector& center = boxes[i].get_center();
        double width = boxes[i].get_width();
        double height = boxes[i].get_height();
        size_t min_c = std::max(0, (int)std::floor(center[0] - width/2.0));
        size_t min_r = std::max(0, (int)std::floor(center[1] - height/2.0));
        size_t max_c = std::min(src.cols - 1, 
                                (int)std::ceil(center[0] + width/2.0));
        size_t max_r = std::min(src.rows - 1, 
                                (int)std::ceil(center[1] + height/2.0));

        Mat mask(src.rows, src.cols, CV_8UC1, Scalar(0));
        for(size_t r = min_r; r < max_r; r++)
        {
            for(size_t c = min_c; c < max_c; c++)
            {
                mask.at<char>(r, c) = 255;
            }
        }

        const int num_images = 1;
        const int dims = 2;
        bool uniform = true;
        bool accumulate = false;
            
        Mat hist; 
        calcHist(&hsv, num_images, channels, mask, hist, dims, hist_size, 
                ranges, uniform, accumulate);
        // normalize
        double s = norm(hist, NORM_L1);
        double sum = 0.0;
        for (size_t r = 0; r < hist.rows; r++)
        {
            for(size_t c = 0; c < hist.cols; c++)
            {
                sum += hist.at<float>(r, c);
            }
        }

        for (size_t r = 0; r < hist.rows; r++)
        {
            for(size_t c = 0; c < hist.cols; c++)
            {
                hist.at<float>(r, c) = hist.at<float>(r, c) / sum;
            }
        }
        
        // Convert to kjb::Matrix 
        std::ofstream ofs("hist.txt");
        ofs << hist;
        std::ofstream ofsk("hist_kjb.txt");
        ofsk << opencv_to_kjb(hist);
        histos.push_back(opencv_to_kjb(hist));
    }
    return histos;
#else //KJB_HAVE_OPENCV
    KJB_THROW_2(kjb::Missing_dependency, "opencv");
#endif
}
