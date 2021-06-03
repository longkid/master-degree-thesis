#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <string>
//#include <fstream>
#include <sstream>
#include <cv.h>

#include "BBoxFileReader.hpp"
#include "AnnotatedFrame.hpp"
#include "GaussianMap.hpp"

/*
  Change value of OUTPUT to create desired output
  OUTPUT = 0: produce Gaussian map
  OUTPUT = 1: produce frame with face-bounding boxes
  OUTPUT = 2: produce BBoxes file
*/
#define OUTPUT 2

using namespace std;
using namespace cv;

void help(const char* argv)
{
  cout << "\nThis program demonstrates the cascade recognizer. Now you can use Haar or LBP features.\n"
    "This classifier can recognize many ~rigid objects, it's most known use is for faces.\n"
    "Usage:\n"
       << argv << " [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
    "   [--scale=<image scale greater or equal to 1, try 1.3 for example>\n"
    "   [filename|camera_index]\n"
#if OUTPUT==0
    "   width height bboxMinSize\n\n"
#endif
    "see facedetect.cmd for one call:\n"
    "./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye.xml\" --scale=1.3 \n"
    "Hit any key to quit.\n"
    "Using OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw(int frameNumber, Mat& img, CascadeClassifier& cascade, double scale);

String cascadeName = "haarcascade_frontalface_alt.xml";
//String nestedCascadeName = "../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";

#if OUTPUT==0
int videoWidth = 1920;
int videoHeight = 1080;
int bboxMinSize = 50;
#elif OUTPUT==2
ofstream bboxesFile;
#endif

int main( int argc, const char** argv )
{
  CvCapture* capture = 0;
  Mat frame, frameCopy, image;
  const String scaleOpt = "--scale=";
  size_t scaleOptLen = scaleOpt.length();
  const String cascadeOpt = "--cascade=";
  size_t cascadeOptLen = cascadeOpt.length();
  String inputName;
  
  help(argv[0]);
  
  CascadeClassifier cascade, nestedCascade;
  double scale = 1;
  
  for( int i = 1; i < argc; i++ )
    {
      cout << "Processing " << i << " " <<  argv[i] << endl;
      if( cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
	  cascadeName.assign( argv[i] + cascadeOptLen );
	  cout << "  from which we have cascadeName= " << cascadeName << endl;
        }
      else if( scaleOpt.compare( 0, scaleOptLen, argv[i], scaleOptLen ) == 0 )
        {
	  if( !sscanf( argv[i] + scaleOpt.length(), "%lf", &scale ) || scale < 1 )
	    scale = 1;
	  cout << " from which we read scale = " << scale << endl;
        }
      else if( argv[i][0] == '-' )
        {
	  cerr << "WARNING: Unknown option %s" << argv[i] << endl;
        }
      else
	inputName.assign( argv[i] );
    }
  
  if( !cascade.load( cascadeName ) )
    {
      cerr << "ERROR: Could not load classifier cascade" << endl;
      cerr << "Usage: facedetect [--cascade=<cascade_path>]\n"
	"   [--nested-cascade[=nested_cascade_path]]\n"
	"   [--scale[=<image scale>\n"
	"   [filename|camera_index]\n" << endl ;
      return -1;
    }
  
  if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
      capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
      int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
      if(!capture) cout << "Capture from CAM " <<  c << " didn't work" << endl;
    }
  else if( inputName.size() )
    {
      image = imread( inputName, 1 );
      if( image.empty() )
        {
	  capture = cvCaptureFromAVI( inputName.c_str() );
	  if(!capture) cout << "Capture from AVI didn't work" << endl;
        }
    }
  
  //cvNamedWindow( "result", 1 );
  
  if( capture )
    {
      cout << "In capture ..." << endl;
      // 20110927: LH added
      int frameNumber = 0;
#if OUTPUT==0
      videoWidth = atoi(argv[argc - 3]);
      videoHeight = atoi(argv[argc - 2]);
      bboxMinSize = atoi(argv[argc - 1]);
#elif OUTPUT==2
      // Write the BBoxes file
      string outputname(inputName);
      outputname += ".txt";
      bboxesFile.open(outputname.c_str(), ios_base::app | ios_base::out);
#endif

      for(;;)
        {
	  IplImage* iplImg = cvQueryFrame( capture );
	  frame = iplImg;
	  if( frame.empty() )
	    break;
	  if( iplImg->origin == IPL_ORIGIN_TL )
	    frame.copyTo( frameCopy );
	  else
	    flip( frame, frameCopy, 0 );
	  
	  detectAndDraw( frameNumber, frameCopy, cascade, scale );
	  frameNumber++;
	  
	  if( waitKey( 10 ) >= 0 ) {
	    goto _cleanup_;
	  }
        }
      
      waitKey(0);
      
    _cleanup_:
      cvReleaseCapture( &capture );
    }
  
  //cvDestroyWindow("result");
#if OUTPUT==2
  bboxesFile.close();
#endif

  return 0;
}

void detectAndDraw(int frameNumber, Mat& img, CascadeClassifier& cascade, double scale)
{
  int i = 0;
  double t = 0;
  vector<Rect> faces;
  Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
  
  cvtColor( img, gray, CV_BGR2GRAY );
  resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
  equalizeHist( smallImg, smallImg );
  
  t = (double)cvGetTickCount();
  cascade.detectMultiScale( smallImg, faces,
			    1.1, 2, 0
			    //|CV_HAAR_FIND_BIGGEST_OBJECT
			    //|CV_HAAR_DO_ROUGH_SEARCH
			    |CV_HAAR_SCALE_IMAGE
			    ,
			    Size(30, 30) );
  t = (double)cvGetTickCount() - t;
  printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
  //cout << "FrameNumber = " << frameNumber << ", nbBBoxes = " << faces.size() << "\n";
  
  /*for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
    Mat smallImgROI;
    //vector<Rect> nestedObjects;
    Point center;
    Scalar color = colors[i%8];
    int radius;
    center.x = cvRound((r->x + r->width*0.5)*scale);
    center.y = cvRound((r->y + r->height*0.5)*scale);
    radius = cvRound((r->width + r->height)*0.25*scale);
    }*/

#if OUTPUT==0
  BBoxFileReader bboxFR;
  std::string bboxFileName = argv[1];
  AABBoxCollection bboxes;
#endif

#if OUTPUT==2
  bboxesFile << frameNumber << " " << faces.size() << " ";
#endif

  for (int i = 0; i < faces.size(); i++) {
    int xMin = int (faces[i].x * scale + 0.5);
    int yMin = int (faces[i].y * scale + 0.5);
    int width = int (faces[i].width * scale + 0.5);
    int height = int (faces[i].height * scale + 0.5);
#if OUTPUT==0
    AABBox bbox(xMin, yMin, width, height);
    bboxes.push_back(bbox);
#elif OUTPUT==1
    cv::Rect rect(xMin, yMin, width, height);
    cv::rectangle(img, rect, cv::Scalar(255,0,0), 3);
#elif OUTPUT==2
    bboxesFile << xMin << " " << yMin << " " << width << " " << height << " ";
#endif
  }

#if OUTPUT==0
  AnnotatedFrame aframe(frameNumber, bboxes);
  GaussianMap gMap;
  gMap.setBBoxMinSize(bboxMinSize);
  gMap.set(aframe.getBBoxes(), videoWidth, videoHeight);
  
  if (aframe.hasBBoxes()) {
    std::ostringstream oss;
    oss.width(5);
    oss.fill('0');
    oss << aframe.getFrameNum() << ".jpg";
    cv::imwrite(oss.str(), gMap.getMat());
  }
#elif OUTPUT==1  
  std::ostringstream oss;
  oss.width(5);
  oss.fill('0');
  oss << frameNumber << ".jpg";
  cv::imwrite(oss.str(), img);
#elif OUTPUT==2
  bboxesFile << "\n";
  bboxesFile.flush();
#endif
}
