#include <iostream>
#include <cstdlib>
#include <string>
#include <cv.h>
#include <highgui.h>
#include <sstream>

#include "BBoxFileReader.hpp"
#include "AnnotatedFrame.hpp"
#include "GaussianMap.hpp"

void usage(char* program_name)
{
  std::cout << "\nThis program builds Gaussian Map from the result of annotating videos\n"
    "Usage: " << program_name << " textpath width height bboxMinSize\n"
    "Arguments:\n"
    "- textpath (string): Path to the result file\n"
    "- width (integer): The width of the annotated video\n"
    "- height (integer): The height of the annotated video\n"
    "- bboxMinSize (integer): The minimum size of bbox\n";
}

int
main(int argc, char* argv[]) {
  if (argc < 5) {
    std::cerr << "Insufficient arguments" << std::endl;
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  int imageWidth = atoi(argv[2]);
  int imageHeight = atoi(argv[3]);
  if (imageWidth == 0 || imageHeight == 0) {
    std::cerr << "Invalid argument: " << argv[2] << ", " << argv[3];
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  
  BBoxFileReader bboxFR;
  AnnotatedFrame aframe;
  GaussianMap gMap;
  std::string bboxFileName = argv[1];
  int bboxMinSize = atoi(argv[4]);
  if (bboxFR.open(bboxFileName)) {
    while(bboxFR.hasMoreFrames()) {
      bboxFR >> aframe;
      gMap.setBBoxMinSize(bboxMinSize);
      gMap.set(aframe.getBBoxes(), imageWidth, imageHeight);
      
      if (aframe.hasBBoxes()) {
	std::ostringstream oss;
	oss.width(5);
	oss.fill('0');
	oss << aframe.getFrameNum() << ".png";
	std::cout << "Filename= " << oss.str() << std::endl;
	cv::imwrite(oss.str(), gMap.getMat());
      }
    }
  }
  return EXIT_SUCCESS;
}
