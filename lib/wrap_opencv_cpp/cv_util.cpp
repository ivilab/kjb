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

/* $Id: cv_util.cpp 21172 2017-01-30 09:21:24Z kobus $ */

/* Kobus: Added Jan 13, 2017. */
#include <iostream>

#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <wrap_opencv_cpp/cv_util.h>


//#ifdef KJB_HAVE_OPENCV
/* Does not compile */ 
//#warning "Disabling OPENCV in cv_object_detec.cpp  because it does not compile"
//#undef KJB_HAVE_OPENCV
//#endif


namespace kjb
{
namespace opencv
{

#ifdef KJB_HAVE_OPENCV

/*cv::Ptr<IplImage> to_opencv(const Image& image)
{
    cv::Ptr<IplImage> iplImage = cvCreateImage(cvSize(image.get_num_cols(), 
                                                      image.get_num_rows()),
                                                      IPL_DEPTH_32F, 3);
    for(int i = 0; i < image.get_num_rows(); i++)
    {
        for(int j = 0; j < image.get_num_cols(); j++)
        {
            ((float *)(iplImage->imageData + i*iplImage->widthStep))
                [j*iplImage->nChannels + 0] = (image(i, j).b); // B

            ((float *)(iplImage->imageData + i*iplImage->widthStep))
                [j*iplImage->nChannels + 1] = (image(i, j).g); // G

            ((float *)(iplImage->imageData + i*iplImage->widthStep))
                [j*iplImage->nChannels + 2] = (image(i, j).r); // R

        }
    }

    // For test
    //cvSaveImage("tmp_cv.jpg", iplImage);  

    return iplImage;
}
*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

cv::Ptr<IplImage> to_opencv_gray(const Image& image)
{

    const char file_name[] = "temp_image.jpg";

    cv::Ptr<IplImage> iplImage;
    try
    {
        image.write(file_name);
        iplImage = cvLoadImage(file_name, CV_LOAD_IMAGE_GRAYSCALE);

        iplImage->origin = IPL_ORIGIN_TL;
        remove(file_name);

        return iplImage;

    }
    catch(kjb::IO_error kio)
    {
        kio.print(std::cout);
    }
    catch(cv::Exception ce)
    {
        std::cout<<ce.what()<<std::endl;
    }
    
    return iplImage;

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

cv::Ptr<IplImage> to_opencv_gray(const Matrix& matrix)
{

    // Does not work for opencv optical flow  
    cv::Ptr<IplImage> iplImage_32f = 
        cvCreateImage(cvSize(matrix.get_num_cols(), matrix.get_num_rows()), 
                      IPL_DEPTH_32F, 1);
    for(int i = 0; i < matrix.get_num_rows(); i++)
    {
        for(int j = 0; j < matrix.get_num_cols(); j++)
        {
            ((float *)(iplImage_32f->imageData 
                + i*iplImage_32f->widthStep))[j] = matrix(i, j); 
        }
    }

    cv::Ptr<IplImage> iplImage_8u = 
        cvCreateImage(cvSize(matrix.get_num_cols(), matrix.get_num_rows()), 
                      IPL_DEPTH_8U, 1);
    cvConvertScale(iplImage_32f, iplImage_8u, 1.0);
    // For test
    //cvNormalize(iplImage_8u, iplImage_8u, 0, 255, cv::CV_MINMAX);
    //cvSaveImage("tmp_cv_grey_32f.jpg", iplImage_32f);  
    //cvSaveImage("tmp_cv_grey_8u.jpg", iplImage_8u);  

    return iplImage_8u;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

cv::Mat to_opencv(const Image& img)
{
    int num_rows = img.get_num_rows();
    int num_cols = img.get_num_cols();
    cv::Mat cv_mat(num_rows, num_cols, CV_32FC3);
    for(int r = 0; r < num_rows; r++)
    {
        for(int c = 0; c < num_cols; c++)
        {
            cv_mat.at<cv::Vec3f>(r, c)[0] = img(r, c).b;
            cv_mat.at<cv::Vec3f>(r, c)[1] = img(r, c).g;
            cv_mat.at<cv::Vec3f>(r, c)[2] = img(r, c).r;
        }
    }
    return cv_mat;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

cv::Mat to_opencv(const Matrix& matrix)
{
    cv::Mat cv_matrix(matrix.get_num_rows(), matrix.get_num_cols(), cv::DataType<float>::type);
    for(int i = 0; i < cv_matrix.rows; i++)
    {
        for(int j = 0; j < cv_matrix.cols; j++)
        {
            cv_matrix.at<float>(i, j) = matrix(i, j);
        }
    }
    return cv_matrix;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix opencv_to_kjb(const cv::Mat& cv_mat)
{
    Matrix matrix(cv_mat.rows, cv_mat.cols);
    for(int i = 0; i < cv_mat.rows; i++)
    {
        for(int j = 0; j < cv_mat.cols; j++)
        {
            matrix(i, j) = cv_mat.at<float>(i, j);
        }
    }
    return matrix;
}

#endif
} //namespace opencv
} //namespace kjb
