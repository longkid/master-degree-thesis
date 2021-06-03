#include <cv.h>
#include <highgui.h>

#include <iostream>
#include <ctype.h>
#include <cstdlib>
#include <string>
#include <sstream>

#include "BBoxFileReader.hpp"
#include "AnnotatedFrame.hpp"
#include "Methods.hpp"

#define MAX_FRAME 243
#define METHOD_NB 3 // Number of used methods

using namespace cv;
using namespace std;

void
help(char* program_name)
{
  std::cout << "\nUsage: " << program_name << " imgDir textPath outputFile\n";
}

int
main( int argc, char** argv )
{
  if (argc != 4) {
    cerr << "Insufficient arguments" << endl;
    help(argv[0]);
    return EXIT_FAILURE;
  }

  // Write the computed values
  ofstream outfile;
  outfile.open(argv[3], ios_base::app | ios_base::out);

  std::string bboxFileName = argv[2];
  BBoxFileReader bboxFR;
  AnnotatedFrame aframe;
  int nbProcessedFrame = 0;
  float mean[METHOD_NB];
  Methods computingObj = Methods();

  for (int i = 0; i < METHOD_NB; i++) {
    mean[i] = 0;
  }
  if (bboxFR.open(bboxFileName)) {
    while(bboxFR.hasMoreFrames()) {
      bboxFR >> aframe;
      int frameNum = aframe.getFrameNum();
      if (frameNum > MAX_FRAME)
	break;
      if (aframe.hasBBoxes()) {
	ostringstream oss;
	oss << argv[1] << "/frame" << frameNum << "-ET.png";
	Mat image = imread(oss.str(), 0);
	AABBoxCollection bboxes = aframe.getBBoxes();

	for (int i = 0; i < METHOD_NB; i++) {
	  mean[i] += computingObj.compute(image, bboxes, i+1);
	}
	nbProcessedFrame++;
      }
    }
    // Compute the mean of all computed values
    for (int i = 0; i < METHOD_NB; i++) {
      mean[i] /= (float)nbProcessedFrame;
    }

    std::string imgDir(argv[1]);
    int found = imgDir.find("SRC");
    if (found != string::npos) {
      outfile << imgDir.substr(found) << " ";
      for (int i = 0; i < METHOD_NB; i++) {
	outfile << mean[i] << " ";
      }
      outfile << "\n";
    }
    outfile.flush();
  }
  outfile.close();
  
  return EXIT_SUCCESS;
}
