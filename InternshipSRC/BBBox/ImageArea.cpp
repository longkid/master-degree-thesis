#include "ImageArea.hpp"

#include <cassert>
#include <iostream>

#include <QMouseEvent>
#include <QPainter>
#include <stdio.h>

#include <QApplication>
#include <QDesktopWidget>

ImageArea::ImageArea(QWidget *parent)
  : QLabel(parent),
    m_selectionIndex(0),
    m_drawing(false),
    m_moving(false),
    m_scaledRatio(1) // 20110809: Lam Ho added
{

}

void
ImageArea::setPixmap(const QPixmap &p)
{
  m_originalPixmap = p;
  // Compute scaled dimension to scale pixmap
  int scaledWidth = int( m_originalPixmap.width() / m_scaledRatio + 0.5 );
  int scaledHeight = int( m_originalPixmap.height() / m_scaledRatio + 0.5 );
  scalePixmap(scaledWidth, scaledHeight);
  //QLabel::setPixmap(p);
}

void
ImageArea::resizeEvent(QResizeEvent *event)
{
  float oldRatio = getRatio();
  scalePixmap(width(), height());
  float newRatio = getRatio();
  for (int i = 0; i < m_bboxes.size(); ++i) {
    AABBox originalBBox;
    AABBox newScaledBBox;
    toOriginalBBox(originalBBox, m_bboxes.at(i), oldRatio);
    toScaledBBox(newScaledBBox, originalBBox, newRatio);
    m_bboxes.replace(i, newScaledBBox);
  }
  update();
}

void
ImageArea::getBBoxes(AABBoxCollection &bboxes) const
{
  for (int i = 0; i < m_bboxes.size(); ++i) {
    AABBox originalBBox;
    toOriginalBBox(originalBBox, m_bboxes.at(i), getRatio());
    bboxes.append(originalBBox);
  }
  //bboxes = m_bboxes;
}

void
ImageArea::setBBoxes(const AABBoxCollection &bboxes)
{
  m_bboxes.clear();
  for (int i = 0; i < bboxes.size(); ++i) {
    AABBox scaledBBox;
    toScaledBBox(scaledBBox, bboxes.at(i), getRatio());
    m_bboxes.append(scaledBBox);
  }
  //m_bboxes = bboxes;
  m_selectionIndex = m_bboxes.size() - 1;

  update();
}

bool
ImageArea::hasSameSizePixmap(const QPixmap &p)
{
  return m_originalPixmap.size() == p.size();
}

void
ImageArea::computeRatio(const QPixmap &originalPixmap, const QPixmap &scaledPixmap)
{
  m_scaledRatio = (float) originalPixmap.width() / scaledPixmap.width();
}

void
ImageArea::computeRatio(const QPixmap &pixmap, int width, int height)
{
  QPixmap scaledPixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  computeRatio(pixmap, scaledPixmap);
}

void
ImageArea::toOriginalBBox(AABBox &originalBBox, const AABBox &scaledBBox, float ratio) const
{
  const int xMin = int( ratio * scaledBBox.xMin() + 0.5 );
  const int yMin = int( ratio * scaledBBox.yMin() + 0.5 );
  const int width = int( ratio * scaledBBox.width() + 0.5 );
  const int height = int( ratio * scaledBBox.height() + 0.5 );
  originalBBox.set(xMin, yMin, width, height);
}

void
ImageArea::toScaledBBox(AABBox &scaledBBox, const AABBox &originalBBox, float ratio) const
{
  assert(ratio != 0);
  toOriginalBBox(scaledBBox, originalBBox, 1 / ratio);
}

void
ImageArea::scalePixmap(int width, int height)
{
  QPixmap scaledPixmap = m_originalPixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  computeRatio(m_originalPixmap, scaledPixmap);
  QLabel::setPixmap(scaledPixmap);
}

void
ImageArea::clearBBoxes()
{
  m_bboxes.clear();
  m_selectionIndex = 0;

  update();
}

void
ImageArea::removeSelectedBBox()
{
  if (m_selectionIndex < m_bboxes.size()) {
    m_bboxes.removeAt(m_selectionIndex);
    m_selectionIndex = m_bboxes.size()-1;

    update();
  }
}

/**
 * Seek if there is bbox under point (x, y).
 *
 * Return index of found bbox if any 
 * or m_bboxes.size() if none contains this point.
 */
int
ImageArea::intersectedBBox(int x, int y) const
{
  const int nbBBoxes = m_bboxes.size();
  for (int i=0; i<nbBBoxes; ++i) {
    const AABBox &bb = m_bboxes.at(i);
    if (bb.contains(x, y)) {
      return i;
    }
  }
  return nbBBoxes;
}


void
ImageArea::correctBBox(int &x0, int &y0, int &x1, int &y1)
{
  static const int MIN_DIM = 6;

  assert(pixmap()->width()>=MIN_DIM);
  assert(pixmap()->height()>=MIN_DIM);

  if (x0<0) {
    x0 = 0;
    if (x1 <= x0)
      x1 = x0 + MIN_DIM;
  }

  if (y0<0) {
    y0 = 0;
    if (y1 <= y0)
      y1 = y0 + MIN_DIM;
  }
    
  if (x0 >= pixmap()->width()) {
    x0 = pixmap()->width()-MIN_DIM;
    x1 = pixmap()->width();
  }
  if (y0 >= pixmap()->height()) {
    y0 = pixmap()->height()-MIN_DIM;
    y1 = pixmap()->height();
  }
    
  if (x1 > pixmap()->width()) {
    x1 = pixmap()->width();
  }
  if (y1 > pixmap()->height()) {
    y1 = pixmap()->height();
  }


  if (x0 == x1)
    x1 = x0 + MIN_DIM;
  if (y0 == y1)
    y1 = y0 + MIN_DIM;
    
  assert(x0>=0);
  assert(y0>=0);
  assert(x0<x1);
  assert(y0<y1);
  assert(x1<=pixmap()->width());
  assert(y1<=pixmap()->height());
}

void
ImageArea::mouseMoveEvent(QMouseEvent *event)
{
  QPoint currPos = event->pos();

  if (m_drawing) {

    assert(! m_bboxes.isEmpty());

    AABBox &bbox = m_bboxes.back();
    
    int x0 = m_startPos.x();
    int x1 = currPos.x();
    if (x1 < x0)
      std::swap(x0, x1);

    int y0 = m_startPos.y();
    int y1 = currPos.y();
    if (y1 < y0)
      std::swap(y0, y1);

    correctBBox(x0, y0, x1, y1);

    bbox.set(x0, y0, x1-x0, y1-y0);

    update();
  }
  else if (m_moving) {

    const int tx = currPos.x() - m_startPos.x();
    const int ty = currPos.y() - m_startPos.y();

    if (abs(tx)>0 || abs(ty)>0) {
    
      assert(! m_bboxes.isEmpty());
    
      AABBox &bbox = m_bboxes[m_selectionIndex];
    
      int nx0 = bbox.xMin()+tx;
      int ny0 = bbox.yMin()+ty;
      
      int nw = bbox.width();
      int nh = bbox.height();
      
      int nx1 = nx0 + nw;
      int ny1 = ny0 + nh;

      m_startPos = currPos;

      correctBBox(nx0, ny0, nx1, ny1);

      nw = nx1-nx0;
      nh = ny1-ny0;

      assert(nx0>=0);
      assert(nx1>=0);
      assert(nw>=0);
      assert(nh>=0);
      assert(nx0+nw<=pixmap()->width());
      assert(ny0+nh<=pixmap()->height());

      bbox.set(nx0, ny0, nw, nh);

      update();
    }
    
  }
}


void
ImageArea::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {

    assert(! m_drawing);
    assert(! m_moving);

    const int intersectionIndex = intersectedBBox(event->pos().x(), event->pos().y());
    if (intersectionIndex != m_bboxes.size()) {
      //one bbox selected 

      m_selectionIndex = intersectionIndex;
      update();

      m_startPos = event->pos();
      m_moving = true;

    }
    else {
      m_startPos = event->pos();
      m_drawing = true;
      m_bboxes.push_back(AABBox(m_startPos.x(), m_startPos.y()));
      m_selectionIndex = m_bboxes.size()-1;
    }

  }
  //std::cerr<<"ImageArea::mousePressEvent x0="<<m_startPos.x()<<" y0="<<m_startPos.y()<<std::endl;

}

void
ImageArea::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->button() == Qt::LeftButton) {

    assert(! m_bboxes.isEmpty());

    assert(m_drawing || m_moving);

    m_drawing = false;
    m_moving = false;

    assert(! m_bboxes.empty());
    assert(m_selectionIndex < m_bboxes.size());
    const AABBox &bbox = m_bboxes.at(m_selectionIndex);
    if ((bbox.xMin() == bbox.xMax())
	|| (bbox.yMin() == bbox.yMax())) {
      if (m_selectionIndex != m_bboxes.size()-1) {
	std::swap(m_bboxes[m_selectionIndex], m_bboxes[m_bboxes.size()-1]);
      }
      m_bboxes.pop_back();
      m_selectionIndex = m_bboxes.size()-1;
    }
    else {
      assert(bbox.xMin() >= 0);
      assert(bbox.yMin() >= 0);
      assert(bbox.xMin() < bbox.xMax());
      assert(bbox.yMin() < bbox.yMax());
      assert(bbox.xMax() <= pixmap()->width());
      assert(bbox.yMax() <= pixmap()->height());
    }
    /*
      std::cerr<<"ImageArea::mouseReleaseEvent   m_bboxes.size()="<< m_bboxes.size()<<std::endl;
      for (int i=0; i<m_bboxes.size(); ++i) {
      const AABBox &bbox = m_bboxes.at(i);
      std::cerr<<" x0="<<bbox.xMin()<<" y0="<<bbox.yMin()<<" w="<<bbox.width()<<" h="<<bbox.height()<<std::endl;
      }
    */
  }
}

void
ImageArea::paintEvent(QPaintEvent *event)
{
  QLabel::paintEvent(event);
  
  const int nbBBoxes = m_bboxes.size();
  if (nbBBoxes > 0) {
    QPainter painter(this);
    painter.setPen(QPen(Qt::white));

    for (int i=0; i<nbBBoxes; ++i) {
      const AABBox &bbox = m_bboxes.at(i);
      painter.drawRect(bbox.xMin(), bbox.yMin(), bbox.width(), bbox.height());
    }

    painter.setPen(QPen(Qt::yellow));
    const AABBox &bbox = m_bboxes.at(m_selectionIndex);
    painter.drawRect(bbox.xMin(), bbox.yMin(), bbox.width(), bbox.height());
    
  }

  //std::cerr<<"ImageArea::paintEvent"<<std::endl;

}
