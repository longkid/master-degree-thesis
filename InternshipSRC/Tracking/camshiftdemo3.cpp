#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>
#include <cstdlib>
#include <string>

#include "BBoxFileReader.hpp"
#include "AnnotatedFrame.hpp"

using namespace cv;
using namespace std;

void usage(char* program_name)
{
  cout << "\nThis program tests CamShift tracking on a movie.\n"
    "Usage: " << program_name << " video textFile\n"
    "Arguments:\n"
    "- video: Path to the video file\n"
    "- textFile: Path to the file containing the tracking rectangle on first face\n";

  cout << "\n\nHot keys: \n"
    "\tESC - quit the program\n"
    "\tc - stop the tracking\n"
    "\tb - switch to/from backprojection view\n"
    "\tn - next frame\n" << endl;
}

Mat image;
BBoxFileReader bboxFR;
AnnotatedFrame aframe;
vector<Rect> faces;

bool backprojMode = false;
//bool selectObject = false;
int trackObject = 0;
//bool showHist = true;
//Point origin;
//Rect selection;
int vmin = 10, vmax = 256, smin = 30;

void readSelectionRect(char* bbox_name)
{
  std::string bboxFileName = bbox_name;
  if (bboxFR.open(bboxFileName)) {
    // Just parse the first line in the file
    if (bboxFR.hasMoreFrames()) {
      bboxFR >> aframe;
      AABBoxCollection bboxes = aframe.getBBoxes();
      for (int i = 0; i < bboxes.size(); i++) {
	AABBox bbox = bboxes[i];
	Rect face(bbox.xMin(), bbox.yMin(), bbox.width(), bbox.height());
	faces.push_back(face);
      }
      trackObject = -1; // Start to track faces
    }
  }
}

int main( int argc, char** argv )
{
  VideoCapture cap;
  Rect trackWindow;
  RotatedRect trackBox;
  int hsize = 16;
  float hranges[] = {0,180};
  const float* phranges = hranges;
  
  if (argc != 3) {
    cerr << "Insufficient arguments" << endl;
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  cap.open(argv[1]);
  if( !cap.isOpened() ) {
    cout << "***Could not initialize capturing...***\n";
    return EXIT_FAILURE;
  }
  readSelectionRect(argv[2]);
  
  //namedWindow( "Histogram", 0 );
  namedWindow( "CamShift Demo", 0 );
  //setMouseCallback( "CamShift Demo", onMouse, 0 );
  createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
  createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
  createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
  
  Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
  vector<Mat> face_hist;
  // Store the first frame number to start tracking
  int nbStartFrame = aframe.getFrameNum();
  //bool paused = false;
  
  for (int i = 0; ; i++) {
    cap >> frame; // Get a new frame
    if (frame.empty())
      break;

    cout << "Processing frame: " << i << endl;
    // Skip processing until the desired frame is encountered
    if (i < nbStartFrame)
      continue;
    
    frame.copyTo(image);
    
    cvtColor(image, hsv, CV_BGR2HSV);
    
    if (trackObject) {
      int _vmin = vmin, _vmax = vmax;
      
      inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
	      Scalar(180, 256, MAX(_vmin, _vmax)), mask);
      int ch[] = {0, 0};
      hue.create(hsv.size(), hsv.depth());
      mixChannels(&hsv, 1, &hue, 1, ch, 1);
      
      // Calculate the histogram for each face
      if (trackObject < 0) {
	for (int ii = 0; ii < faces.size(); ii++) {
	  Mat roi(hue, faces[ii]), maskroi(mask, faces[ii]);
	  calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	  normalize(hist, hist, 0, 255, CV_MINMAX);
	  // Store hist in face_hist
	  face_hist.push_back(hist);
	  
	  //trackWindow = selection;
	  faces[ii] &= Rect(0, 0, image.cols, image.rows);
	}
	trackObject = 1;
      }
      
      for (int ii = 0; ii < faces.size(); ii++) {
	calcBackProject(&hue, 1, 0, face_hist[ii], backproj, &phranges);
	backproj &= mask; // ???
	trackWindow = faces[ii];
	RotatedRect trackBox = CamShift(backproj, trackWindow,
					TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
	// Reallocate the position & size of track window
	if (trackWindow.area() <= 1) {
	  int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
	  trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
			     trackWindow.x + r, trackWindow.y + r) &
	    Rect(0, 0, cols, rows);
	}
	
	if( backprojMode )
	  cvtColor( backproj, image, CV_GRAY2BGR );
	// Draw an ellipse surrounding the face
	Rect boundRect = trackBox.boundingRect();
	rectangle( image, boundRect.tl(), boundRect.br(), Scalar(0,255,0) );
	ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
      }
    }
    
    /*if( selectObject && selection.width > 0 && selection.height > 0 )
      {
	Mat roi(image, selection);
	bitwise_not(roi, roi); // ???
	}*/
      
    // Display demo and histogram image
    imshow( "CamShift Demo", image );
    
    // Handle key input from the user
    char c = (char)waitKey(0);
    if( c == 27 )
      break;
    if (c == 'n')
      continue; // Process next frame
    switch (c) {
    case 'b':
      backprojMode = !backprojMode;
      break;
    case 'c':
      trackObject = 0;
      //histimg = Scalar::all(0);
      break;
    default:
      ;
    }
  }
  
  return EXIT_SUCCESS;
}
