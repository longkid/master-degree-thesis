#include "AnnotatedFrame.hpp"

AnnotatedFrame::AnnotatedFrame()
  : m_frameNum(0),
    m_bboxes()
{
  
}

AnnotatedFrame::AnnotatedFrame(int frameNum, const AABBoxCollection &bboxes)
  : m_frameNum(frameNum),
    m_bboxes(bboxes)
{
  
}
