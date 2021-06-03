#include "GaussianMap.hpp"

using namespace cv;
#define FACTOR 3

typedef float ELT_TYPE;

int GaussianMap::amplitude = 255;

float GaussianMap::scaledRatio = 1.2;

void
GaussianMap::set(const AABBoxCollection &bboxes, int width, int height)
{
  if (!bboxes.empty()) {
    // Algorithm goes here
    m_gaussianMat = Mat(height, width, CV_8UC1);
    m_floatMat = Mat::zeros(height, width, CV_32FC1);
    for (int i = 0; i < bboxes.size(); i++) {
      AABBox bbox = bboxes[i];
      if (bbox.width() < m_bboxMinSize) {
	bbox.set(bbox.xMin(), bbox.yMin(), m_bboxMinSize, bbox.height());
      }
      if (bbox.height() < m_bboxMinSize) {
	bbox.set(bbox.xMin(), bbox.yMin(), bbox.width(), m_bboxMinSize);
      }
      setPixelValue(bbox, width, height);
    }
    // Get max of m1
    ELT_TYPE maxMat = 0;
    for(int row = 0; row < m_floatMat.rows; row++)
      for(int col = 0; col < m_floatMat.cols; col++)
	if (maxMat < m_floatMat.at<ELT_TYPE>(row, col))
	  maxMat = m_floatMat.at<ELT_TYPE>(row, col);
    // Normalize
    assert(maxMat != 0);
    m_floatMat.convertTo(m_gaussianMat, CV_8UC1, 255/maxMat);
  }
}

void
GaussianMap::setPixelValue(const AABBox &bbox, int width, int height)
{
  int centerX = 0;
  int centerY = 0;
  centerX = bbox.xMin() + bbox.width() / 2;
  centerY = bbox.yMin() + bbox.height() / 2;
  float sigmaX = GaussianMap::scaledRatio * bbox.width() / 2;
  float sigmaY = GaussianMap::scaledRatio * bbox.height() / 2;
  // Compute the coordinate of the bbox covering 2D Gaussian function
  int xMin = std::max(centerX - (int)sigmaX*FACTOR, 0);
  int xMax = std::min(centerX + (int)sigmaX*FACTOR, width - 1);
  int yMin = std::max(centerY - (int)sigmaY*FACTOR, 0);
  int yMax = std::min(centerY + (int)sigmaY*FACTOR, height - 1);
  for (int y = yMin; y <= yMax; y++) {
    for (int x = xMin; x <= xMax; x++) {
      float power = 0.5 * (pow(x - centerX, 2)/(sigmaX*sigmaX) + pow(y - centerY, 2)/(sigmaY*sigmaY));
      ELT_TYPE f = GaussianMap::amplitude * exp(-1 * power);
      m_floatMat.at<ELT_TYPE>(y, x) += f;
    }
  }
}
