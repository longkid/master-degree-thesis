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
#define METHOD_NB 2 // Number of used methods
#define WIDTH 1920
#define HEIGHT 1080

using namespace cv;
using namespace std;

void
help(char* program_name)
{
  std::cout << "\nUsage: " << program_name << " errorMapDir eyeTrackerMapDir textPath outputFile\n";
}

int
main( int argc, char** argv )
{
  if (argc != 5) {
    cerr << "Insufficient arguments" << endl;
    help(argv[0]);
    return EXIT_FAILURE;
  }

  // Write the computed values
  ofstream outfile;
  outfile.open(argv[4], ios_base::app | ios_base::out);

  // Read index file to get image filename
  ifstream infile;
  string imgDir(argv[1]);
  imgDir += "/to_eval/error_maps/";
  string indexFile(imgDir);
  indexFile += "index.txt";
  infile.open(indexFile.c_str());
  if (!infile) {
    cerr << "Unable to open file: " << indexFile << endl;
    return false;
  }

  string bboxFileName = argv[3];
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
      // Get error map file name & path
      string imgFileName;
      getline(infile, imgFileName);
      string imgFilePath(imgDir);
      imgFilePath += imgFileName;
      if (!infile.good() || (frameNum > MAX_FRAME))
	break;

      if (aframe.hasBBoxes()) {
	Mat errorMap = imread(imgFilePath.c_str(), 0);
	Mat resized_errorMap;
	resize(errorMap, resized_errorMap, Size(WIDTH, HEIGHT), 0, 0, INTER_LINEAR);

	// Read eye tracker map
	ostringstream oss;
	oss << argv[2] << "/frame" << frameNum << "-ET.png";
	Mat eyeTrackerMap = imread(oss.str(), 0);
	
	AABBoxCollection bboxes = aframe.getBBoxes();
	Mat A = resized_errorMap.mul(eyeTrackerMap);

	for (int i = 0; i < METHOD_NB; i++) {
	  mean[i] += computingObj.compute(A, bboxes, i+1);
	}
	nbProcessedFrame++;
      }
    }
    // Compute the mean of all computed values
    for (int i = 0; i < METHOD_NB; i++) {
      mean[i] /= (float)nbProcessedFrame;
    }

    string imgDir(argv[1]);
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
