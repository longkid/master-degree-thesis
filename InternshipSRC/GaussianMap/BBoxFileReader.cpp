#include "BBoxFileReader.hpp"
#include <iostream>
#include <sstream>

bool
BBoxFileReader::open(const std::string &filePath)
{
  m_bboxFile.open(filePath.data());
  if (!m_bboxFile) {
    std::cerr << "Unable to open file" << std::endl;
    return false;
  }

  return true;
}

BBoxFileReader & operator>>(BBoxFileReader &fr, AnnotatedFrame &af)
{
  std::string line;
  getline(fr.m_bboxFile, line);
  // Parse tokens from line
  std::istringstream iss(line);

  int frameNumber = 0;
  int nbBBoxes = 0;
  iss >> frameNumber >> nbBBoxes;
  AABBoxCollection bboxes;
  af = AnnotatedFrame(frameNumber, bboxes);
  if (nbBBoxes != 0) {
    for (int i = 0; i < nbBBoxes; i++) {
      int xMin = 0;
      int yMin = 0;
      int width = 0;
      int height = 0;
      iss >> xMin >> yMin >> width >> height;
      AABBox bbox(xMin, yMin, width, height);
      bboxes.push_back(bbox);
    }
    af.setBBoxes(bboxes);
  }
  std::cout << "frameNumber = " << frameNumber << ", nbBBoxes = " << nbBBoxes << std::endl;

  return fr;
}

bool
BBoxFileReader::hasMoreFrames()
{
  return !m_bboxFile.eof() && m_bboxFile.good();
}
