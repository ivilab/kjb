/**
 * @file objectDetection2.cpp
 * @author A. Huaman ( based in the classic facedetect.cpp in samples/c )
 * @brief A simplified version of facedetect.cpp, show how to load a cascade classifier and how to find objects (Face + eyes) in a video stream - Using LBP here
 */
#include <wrap_opencv_cpp/cv_object_detect.h>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace kjb;
using namespace kjb::opencv;

/** Function Headers */
//void detectAndDisplay( Mat frame );

/** Global variables */
//String face_cascade_name = "/opt/local/share/opencv/haarcascades/haarcascade_frontalface_default.xml";
#ifdef KJB_HAVE_OPENCV
CV_cascade_classifier face_cascade;
/**
 * @function main
 */
int main( int argc, const char** argv )
{
   //CvCapture* capture;
   if(argc != 3)
   {
        std::cout<<"Usuage: "<<argv[0] << " image-file classifier-file \n";
        return EXIT_SUCCESS;
   }
   const char* img_fname = argv[1]; 
   std::string face_cascade_name = argv[2];
   //Mat frame = cvLoadImageM(img_fname);
   Image img(img_fname);

   //-- 1. Load the cascade
   if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading face\n"); return -1; };
   vector<Axis_aligned_rectangle_2d> faces;
   face_cascade.detect_multiscale( img, faces, 1.1, 2, 0, 80, 80);

   for( int i = 0; i < faces.size(); i++ )
   {
       //-- Draw the face
       faces[i].draw(img, 255.0, 0.0, 0.0, 2.0); 
   } 
   img.write("out.jpg");
  return EXIT_SUCCESS;
}
#endif
/**
 * @function detectAndDisplay
 */
//void detectAndDisplay( const Image& img)
//{
   //std::vector<Rect> faces;
   //Mat frame_gray;

   //cvtColor( frame, frame_gray, CV_BGR2GRAY );
   //equalizeHist( frame_gray, frame_gray );

   //-- Detect faces
   //-- Show what you got
//}
