#ifndef GAUSSIANMAP_HPP
#define GAUSSIANMAP_HPP

#include "AnnotatedFrame.hpp"
#include <cv.h>
#include <math.h>
#include <cassert>

class GaussianMap
{
public:
  GaussianMap()
    : m_gaussianMat(),
      m_floatMat(),
      m_bboxMinSize(0)
  {

  }

  cv::Mat getMat() const
  {
    return m_gaussianMat;
  }

  int getBBoxMinSize() const
  {
    return m_bboxMinSize;
  }

  void setBBoxMinSize(int bboxMinSize)
  {
    m_bboxMinSize = bboxMinSize;
  }

  void set(const AABBoxCollection &bboxes, int width, int height);

  void setPixelValue(const AABBox &bbox, int width, int height);

protected:
  cv::Mat m_gaussianMat;
  cv::Mat m_floatMat;
  int m_bboxMinSize;

public:
  static int amplitude;
  static float scaledRatio;
};

#endif /* GAUSSIANMAP_HPP */
