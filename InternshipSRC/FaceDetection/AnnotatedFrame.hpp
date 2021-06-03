#ifndef ANNOTATEDFRAME_HPP
#define ANNOTATEDFRAME_HPP

#include "AABBox.hpp"
#include <vector>

typedef std::vector<AABBox> AABBoxCollection;

class AnnotatedFrame
{
public:
  AnnotatedFrame();

  AnnotatedFrame(int frameNum, const AABBoxCollection &bboxes);

  int getFrameNum() const
  {
    return m_frameNum;
  }

  void setFrameNum(int frameNum)
  {
    m_frameNum = frameNum;
  }

  const AABBoxCollection &getBBoxes() const
  {
   return m_bboxes;
  }

  void setBBoxes(const AABBoxCollection &bboxes)
  {
    m_bboxes = bboxes;
  }

  bool hasBBoxes()
  {
    return !m_bboxes.empty();
  }

protected:
  int m_frameNum;
  AABBoxCollection m_bboxes;
};

#endif /* ANNOTATEDFRAME_HPP */
