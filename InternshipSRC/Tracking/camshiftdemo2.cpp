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

//Mat image;
Mat image[2];
vector<Rect> faces;

//bool backprojMode = false;
//bool selectObject = false;
//int trackObject = 0;
bool showHist = true;
//Point origin;
//Rect selection;
int vmin = 10, vmax = 256, smin = 30;

void readSelectionRect(char* bbox_name)
{
  BBoxFileReader bboxFR;
  AnnotatedFrame aframe;
  std::string bboxFileName = bbox_name;
  if (bboxFR.open(bboxFileName)) {
    while(bboxFR.hasMoreFrames()) {
      bboxFR >> aframe;
      AABBoxCollection bboxes = aframe.getBBoxes();
      for (int i = 0; i < bboxes.size(); i++) {
	AABBox bbox = bboxes[i];
	Rect face(bbox.xMin(), bbox.yMin(), bbox.width(), bbox.height());
	faces.push_back(face);
      }
    }
  }
}

void usage(char* program_name)
{
  std::cout << "\nThis program tests CamShift tracking on two images.\n"
    "Usage: " << program_name << " image1 image2 textFile\n"
    "Arguments:\n"
    "- image1: Path to the first image file\n"
    "- image2: Path to the second image file\n"
    "- textFile: Path to the file containing the tracking rectangle on image1\n";
}

int main( int argc, char** argv )
{
  namedWindow("Image 1", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  namedWindow("Image 2", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  namedWindow("backproj", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);

  Rect trackWindow;
  //vector<Rect> trackWindows;
  RotatedRect trackBox;
  int hsize = 16;
  float hranges[] = {0,180};
  const float* phranges = hranges;
  
  if (argc != 4) {
    cerr << "Insufficient arguments" << endl;
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  
  /*namedWindow( "Histogram", 0 );
  namedWindow( "CamShift Demo", 0 );
  setMouseCallback( "CamShift Demo", onMouse, 0 );
  createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
  createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
  createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );*/
  //namedWindow("Image 1", CV_WINDOW_AUTOSIZE);
  //namedWindow("Image 2", CV_WINDOW_AUTOSIZE);

  readSelectionRect(argv[3]);
  
  Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
  vector<Mat> face_hist;
  //bool paused = false;
  
  for (int index = 0; index < 2; index++) {
    image[index] = imread(argv[index + 1], 1);
    cvtColor(image[index], hsv, CV_BGR2HSV);
      
    int _vmin = vmin, _vmax = vmax;
      
    inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
	    Scalar(180, 256, MAX(_vmin, _vmax)), mask);
    int ch[] = {0, 0};
    hue.create(hsv.size(), hsv.depth());
    mixChannels(&hsv, 1, &hue, 1, ch, 1);
    
    for (int ii = 0; ii < faces.size(); ii++) {  
      if( index == 0 ) {
	//Mat roi(hue, selection), maskroi(mask, selection);
	Mat roi(hue, faces[ii]), maskroi(mask, faces[ii]);
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	normalize(hist, hist, 0, 255, CV_MINMAX);
	
	// Store hist in face_hist
	face_hist.push_back(hist);
	
	//trackWindow = selection;
	faces[ii] &= Rect(0, 0, image[index].cols, image[index].rows);
	/*trackObject = 1;
	
	// The remaining statements in this block are used to draw the histogram image
	histimg = Scalar::all(0);
	int binW = histimg.cols / hsize;
	Mat buf(1, hsize, CV_8UC3);
	for( int i = 0; i < hsize; i++ )
	  buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
	cvtColor(buf, buf, CV_HSV2BGR);
	
	for( int i = 0; i < hsize; i++ ) {
	  int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
	  rectangle( histimg, Point(i*binW,histimg.rows),
		     Point((i+1)*binW,histimg.rows - val),
		     Scalar(buf.at<Vec3b>(i)), -1, 8 );
		     }*/
      }
      
      calcBackProject(&hue, 1, 0, face_hist[ii], backproj, &phranges);
      backproj &= mask; // ???
      
      trackWindow = faces[ii];
      RotatedRect trackBox = CamShift(backproj, trackWindow,
				      TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
      // Reallocate the position & size of track window
      if( trackWindow.area() <= 1 ) {
	int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
	trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
			   trackWindow.x + r, trackWindow.y + r) &
	  Rect(0, 0, cols, rows);
      }
      
      // Draw an ellipse surrounding the face
      Rect boundRect = trackBox.boundingRect();
      rectangle( image[index], boundRect.tl(), boundRect.br(), Scalar(0,255,0) );
      ellipse( image[index], trackBox, Scalar(0,0,255), 3, CV_AA );
      
      /*if( selectObject && selection.width > 0 && selection.height > 0 ) {
	Mat roi(image, selection);
	bitwise_not(roi, roi); // ???
	}
      if (index == 0 && faces[ii].width > 0 && faces[ii].height > 0) {
	Mat roi(image[index], faces[ii]);
	bitwise_not(roi, roi);
      }*/
    }
  }
  
      
  // Display demo and histogram image
  //imshow( "CamShift Demo", image );
  //imshow( "Histogram", histimg );
  imshow("Image 1", image[0]);
  imshow("Image 2", image[1]);
  imshow("backproj", backproj);

  imwrite("outIMG_1.jpeg", image[0]);
  imwrite("outIMG_2.jpeg", image[1]);
  waitKey(0);

  /*// Handle key input from the user
      char c = (char)waitKey(10);
      if( c == 27 )
	break;
      switch(c)
        {
        case 'b':
	  backprojMode = !backprojMode;
	  break;
        case 'c':
	  trackObject = 0;
	  histimg = Scalar::all(0);
	  break;
        case 'h':
	  showHist = !showHist;
	  if( !showHist )
	    destroyWindow( "Histogram" );
	  else
	    namedWindow( "Histogram", 1 );
	  break;
        case 'p':
	  paused = !paused;
	  break;
        default:
	  ;
        }
	}*/
  
  return EXIT_SUCCESS;
}
