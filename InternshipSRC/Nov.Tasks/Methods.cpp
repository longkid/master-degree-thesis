#include "Methods.hpp"

#define METHOD_COB 1
#define METHOD_COP 2
#define METHOD_FOB 3

/*
  This method compute the value for each frame
*/
float
Methods::compute(const Mat &image, const AABBoxCollection &bboxes, int method)
{
  float ret = 0;
  int nbIntersectedBBox = 0; // used in method 1
  int nbCountedPixel = 0; // used in method 2
  float intersectedPercent = 0; // used in method 2

  for (int i = 0; i < bboxes.size(); i++) {
    AABBox bbox = bboxes[i];
    switch (method) {
    case METHOD_COB:
      nbIntersectedBBox += compute1(image, bbox);
      break;
    case METHOD_COP:
      nbCountedPixel = compute2(image, bbox);
      intersectedPercent += nbCountedPixel / (float)(bbox.width() * bbox.height());
      break;
    case METHOD_FOB:
      if (compute1(image, bbox) == 1)
	return 1;
      break;
    default:
      break;
    }
  }

  switch (method) {
  case METHOD_COB:
    ret = nbIntersectedBBox / (float)bboxes.size();
    break;
  case METHOD_COP:
    ret = intersectedPercent / (float)bboxes.size();
    break;
  default:
    break;
  }
    
  return ret;
}

/*
 * This method checks if a bounding box overlaps with saliency area.
 * Parameters:
 *   - image: input image. This can be an eye-tracker map, an error map,
 *     or a combination of two maps
 *   - bbox: coordinate of bounding box
 * Return value:
 *   1: the bbox overlaps with saliency area inside the input image
 *   0: otherwise
 */  
int
Methods::compute1(const Mat &image, const AABBox &bbox)
{
  for (int x = bbox.xMin(); x < bbox.xMax(); x++)
    for (int y = bbox.yMin(); y < bbox.yMax(); y++)
      if (image.at<unsigned char>(y, x) > 0)
	return 1;
  return 0;
}
  
/*
 * This method counts the number of overlapped pixels.
 * Parameters:
 *   - image: input image. This can be an eye-tracker map, an error map,
 *     or a combination of two maps
 *   - bbox: coordinate of bounding box
 * Return value:
 *   The number of overlapped pixels
 */
int
Methods::compute2(const Mat &image, const AABBox &bbox)
{
  int nbCountedPixel = 0;
  for (int x = bbox.xMin(); x < bbox.xMax(); x++)
    for (int y = bbox.yMin(); y < bbox.yMax(); y++)
      if (image.at<unsigned char>(y, x) > 0)
	nbCountedPixel++;
  return nbCountedPixel;
}
