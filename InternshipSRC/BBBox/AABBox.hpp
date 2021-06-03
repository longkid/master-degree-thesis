#ifndef AABBOX_HPP
#define AABBOX_HPP

#include <algorithm> //std::max
#include <cassert>

class AABBox
{
public:
  AABBox(int xMin=0, int yMin=0, int width=0, int height=0)
    : m_xMin(xMin), m_yMin(yMin), m_width(width), m_height(height)
  {

  }

  void set(int xMin, int yMin, int width, int height)
  {
    m_xMin = xMin;
    m_yMin = yMin;
    m_width = width;
    m_height = height;
  }

  int xMin() const
  {
    return m_xMin;
  }
      
  int yMin() const
  {
    return m_yMin;
  }

  /**
   * @warning outside bbox.
   */
  int xMax() const
  {
    return m_xMin+m_width;
  }
      
  /**
   * @warning outside bbox.
   */
  int yMax() const
  {
    return m_yMin+m_height;
  }

  int width() const
  {
    return m_width;
  }

  int height() const
  {
    return m_height;
  }


  bool valid() const
  {
    return ((width() > 0) && (height() > 0));
  }


  int area() const
  {
    return m_width*m_height;
  }

  bool operator==(const AABBox &b) const
  {
    return ((m_xMin == b.m_xMin) && (m_yMin == b.m_yMin)
	    && (m_width == b.m_width) && (m_height == b.m_height));
  }

  bool operator!=(const AABBox &b) const
  {
    return ((m_xMin != b.m_xMin) || (m_yMin != b.m_yMin)
	    || (m_width != b.m_width) || (m_height != b.m_height));
  }



  bool contains(int x, int y) const
  {
    return ((x>=xMin()) && (x<=xMax()) && (y>=yMin()) && (y<=yMax()));
  }

  bool intersects(const AABBox &b) const
  {
    assert(this->valid());
    assert(b.valid());

    return ((b.xMin()<xMax()) && (b.xMax()>xMin())
	    && (b.yMin()<yMax()) && (b.yMax()>yMin()));

    //warning: not <= because xMax/yMax are outside bbox !
  }

  /**
   * @warning valid only if intersects(@a b) is true
   */
  AABBox intersecte(const AABBox &b) const
  {
    assert(intersects(b));
    const int nxMin = std::max(xMin(), b.xMin());
    int nxMax = std::min(xMax(), b.xMax());
    assert(nxMin <= nxMax);
    //nxMax = std::max(nxMin, nxMax);
    const int nyMin = std::max(yMin(), b.yMin());
    int nyMax = std::min(yMax(), b.yMax());
    assert(nyMin <= nyMax);
    //nyMax = std::max(nyMin, nyMax);
    
    return AABBox(nxMin, nyMin, nxMax-nxMin, nyMax-nyMin);
  }


  AABBox unite(const AABBox &b) const
  {
    assert(b.valid());

    const int a_xMax = m_xMin+m_width;
    const int a_yMax = m_yMin+m_height;
    const int b_xMax = b.m_xMin+b.m_width;
    const int b_yMax = b.m_yMin+b.m_height;

    const int xMin = std::min(m_xMin, b.m_xMin);
    const int yMin = std::min(m_yMin, b.m_yMin);

    const int width = std::max(a_xMax, b_xMax) - xMin;
    const int height = std::max(a_yMax, b_yMax) - yMin;

    return AABBox(xMin, yMin, width, height);
  }



  void
  scale(float scaleFactor)
  {
    assert(valid());

    const int halfWidth = m_width * 0.5;
    const int halfHeight = m_height * 0.5;

    const float cx = m_xMin + halfWidth;
    const float cy = m_yMin + halfHeight;

    const float scaledHalfWidth = halfWidth * scaleFactor;
    const float scaledHalfHeight = halfHeight * scaleFactor;
    
    int nxMin = (int)(cx - scaledHalfWidth);
    int nxMax = (int)(cx + scaledHalfWidth);
    int nyMin = (int)(cy - scaledHalfHeight);
    int nyMax = (int)(cy + scaledHalfHeight);
    if (nxMin < 0)
      nxMin = 0;
    if (nyMin < 0)
      nyMin = 0;
    if (nxMax <= nxMin) 
      nxMax = nxMin + 1;
    if (nyMax <= nyMin)
      nyMax = nyMin + 1;

    m_xMin = nxMin;
    m_yMin = nyMin; 
    m_width = nxMax - nxMin;
    m_height = nyMax - nyMin;
    
    assert(valid());
  }

  
  
protected:
  int m_xMin;
  int m_yMin;
  int m_width;
  int m_height;

};

#endif /* ! AABBOX_HPP */
