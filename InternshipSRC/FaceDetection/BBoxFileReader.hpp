#ifndef BBOXFILEREADER_HPP
#define BBOXFILEREADER_HPP

#include <fstream>
#include <string>
#include "AnnotatedFrame.hpp"

class BBoxFileReader
{
public:
  bool open(const std::string &filePath);

  friend BBoxFileReader &operator >> (BBoxFileReader &fr, AnnotatedFrame &af);

  bool hasMoreFrames();

protected:
  std::ifstream m_bboxFile;
};

#endif /* BBOXFILEREADER_HPP */
