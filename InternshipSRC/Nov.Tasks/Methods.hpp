#ifndef METHODS_HPP
#define METHODS_HPP

#include <cv.h>
#include <highgui.h>
#include "AnnotatedFrame.hpp"

using namespace cv;

class Methods
{
public:
  float compute(const Mat &image, const AABBoxCollection &bboxes, int method);

protected:
  int compute1(const Mat &image, const AABBox &bbox);
  int compute2(const Mat &image, const AABBox &bbox);
};

#endif /* METHODS_HPP */
