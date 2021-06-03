/*#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"*/
#include <cv.h>
#include <highgui.h>

#include <iostream>
#include <ctype.h>
#include <cstdlib>
#include <string>
#include <sstream>

#include "BBoxFileReader.hpp"
#include "AnnotatedFrame.hpp"

#define MAX_FACE 10
#define THRESHOLD 0.15

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
    "\tOther key - skip to next frame\n" << endl;
}

BBoxFileReader bboxFR;
AnnotatedFrame startFrame;
vector<Rect> faces;

Mat image;
bool backprojMode = false;
//bool selectObject = false;
int trackObject = 0;
//bool showHist = true;
//Point origin;
//Rect selection;
int vmin = 10, vmax = 256, smin = 30;

void openBBoxesFile(char* bbox_name)
{
  std::string bboxFileName = bbox_name;
  bboxFR.open(bboxFileName);
}

void readFaceRect(int currentFrame)
{
  // Just parse the first line in the file
  if (bboxFR.hasMoreFrames()) {
    AnnotatedFrame tmp_frame;
    do {
      bboxFR >> tmp_frame;
    } while (tmp_frame.getFrameNum() < currentFrame);

    if (tmp_frame.getBBoxesNum() > 0) {
      faces.clear();
      AABBoxCollection bboxes = tmp_frame.getBBoxes();
      for (int i = 0; i < bboxes.size(); i++) {
	AABBox bbox = bboxes[i];
	Rect face(bbox.xMin(), bbox.yMin(), bbox.width(), bbox.height());
	faces.push_back(face);
	printf("Rect %d: (%d, %d, %d, %d)\t", i, face.x, face.y, face.width, face.height); 
      }
      startFrame = tmp_frame;
      trackObject = -1; // Start to track faces
    }
  }
}

void
normalizeHistogram(cv::Mat &h)
{
  assert(h.type() == CV_32F && h.cols==1);
  double sumH = cv::sum(h)[0];
  if (sumH != 0.0) {
    const double inv_sumH = 1.0/sumH;
    h *= inv_sumH;
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

  openBBoxesFile(argv[2]);
  //readSelectionRect(argv[2]);

  // Write the return values of applying compareHist function
  char* fileName;
  
  fileName = strtok(argv[1], ".");
  strcat(fileName, "_");
  strcat(fileName, "CompareHist");
  strcat(fileName, ".txt");
  ofstream outfile(fileName);
  //outfile << CV_COMP_CORREL << " " << CV_COMP_CHISQR << " " << CV_COMP_INTERSECT << " " << CV_COMP_BHATTACHARYYA << "\n";

  //namedWindow( "Histogram", 0 );
  namedWindow( "CamShift Demo", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
  //setMouseCallback( "CamShift Demo", onMouse, 0 );
  createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
  createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
  createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
  
  Mat frame, hsv, hue, mask, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
  // Store the temporary histogram for each face
  MatND hist;
  vector<MatND> first_face_hist;
  vector<MatND> new_face_hist;
  // Store the histogram-compare value of current frame and first frame
  vector<double> histDelta(MAX_FACE);
  // Store the first frame number to start tracking
  int nbStartFrame = 0;
  //bool paused = false;
  
  for (int frameNum = 0; ; frameNum++) {
    cap >> frame; // Get a new frame
    if (frame.empty())
      break;

    cout << "Processing frame: " << frameNum << endl;
    outfile << frameNum << " ";
    //outfile << "Frame " << i << ":\n";

    // Skip processing until the desired frame is encountered
    /*if (i < nbStartFrame)
      continue;*/
    // Read bbox information to find the new tracked faces
    if (trackObject == 0) {
      readFaceRect(frameNum);
      if (trackObject < 0) {
	nbStartFrame = startFrame.getFrameNum();
	histDelta.clear();
      }
    }

    frame.copyTo(image);
    
    cvtColor(image, hsv, CV_BGR2HSV);
    if (trackObject) {
      int _vmin = vmin, _vmax = vmax;
      
      inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
	      Scalar(180, 256, MAX(_vmin, _vmax)), mask);
      int ch[] = {0, 0};
      hue.create(hsv.size(), hsv.depth());
      mixChannels(&hsv, 1, &hue, 1, ch, 1);
      
      if (trackObject < 0) {
	for (int ii = 0; ii < faces.size(); ii++) {
	  // Calculate the histogram for each face
	  faces[ii] &= Rect(0, 0, image.cols, image.rows);
	  Mat roi(hue, faces[ii]), maskroi(mask, faces[ii]);
	  calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	  normalize(hist, hist, 0, 255, CV_MINMAX);
	  first_face_hist.push_back(hist.clone());
	}
	trackObject = 1;
      }
      
      // Clear the content to store the histograms for new frame
      new_face_hist.clear();

      for (int ii = 0; ii < faces.size(); ii++) {
	// Compute the back project
	calcBackProject(&hue, 1, 0, first_face_hist[ii], backproj, &phranges);
	backproj &= mask;
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
	
	// Calculate the histogram for new bounding rectangle of face
	Rect boundRect = trackBox.boundingRect();
	Mat roi(hue, boundRect), maskroi(mask, boundRect);
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	normalize(hist, hist, 0, 255, CV_MINMAX);
	new_face_hist.push_back(hist.clone());

	// Compare the new histogram with the original histogram
	cout << "Face " << ii << ": ";
	//outfile << "\tFace " << ii << ": ";

	Mat h1 = first_face_hist[ii].clone();
	Mat h2 = new_face_hist[ii].clone();
	normalizeHistogram(h1);
	normalizeHistogram(h2);
	
	/*double sumH = 0;
	for (int i=0; i<h1.rows; ++i) {
	  cout<<"h1["<<i<<"] = "<<h1.at<float>(i, 0)<<endl;
	  cout<<"h2["<<i<<"] = "<<h2.at<float>(i, 0)<<endl;
	  sumH += h1.at<float>(i, 0);
	}
	cout<<"sumH="<<sumH<<endl;
	*/
	
	int compare_method = CV_COMP_INTERSECT;
	//for (int compare_method = 0; compare_method < 4; compare_method++) {
	//double first_new = compareHist(first_face_hist[ii], new_face_hist[ii], compare_method);
	double first_new = compareHist(h1, h2, compare_method);
	cout << "Method " << compare_method << ": " << first_new << ", ";
	cout << endl;

	if (frameNum > nbStartFrame) {
	  double d = fabs(first_new - histDelta[ii]);
	  if (d <= THRESHOLD)
	    histDelta[ii] = first_new;
	  else
	    trackObject = 0; // Stop tracking
	} else {
	  histDelta[ii] = first_new;
	}
	//outfile << first_new << " ";
	//}
	
	if (trackObject) {
	  // Draw an ellipse surrounding the face
	  rectangle( image, boundRect.tl(), boundRect.br(), Scalar(0,255,0) );
	  ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
	} else {
	  rectangle( image, boundRect.tl(), boundRect.br(), Scalar(255,0,0) );
	  ellipse( image, trackBox, Scalar(255,0,0), 3, CV_AA );
	}
      }
    }
    
    if (trackObject)
      outfile << "1\n";
    else
      outfile << "0\n";
    outfile.flush();

    /*if( selectObject && selection.width > 0 && selection.height > 0 )
      {
	Mat roi(image, selection);
	bitwise_not(roi, roi); // ???
	}*/
      
    // Display demo and histogram image
    imshow( "CamShift Demo", image );
    ostringstream oss;
    oss << "Frame: " << frameNum << ", trackObject: " << trackObject;
    displayOverlay("CamShift Demo", oss.str(), 5000);
    
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
  outfile.close();
  
  return EXIT_SUCCESS;
}
