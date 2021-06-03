#ifndef IMAGEAREA_HPP
#define IMAGEAREA_HPP

#include <QLabel>

#include "AABBox.hpp"

typedef QList<AABBox> AABBoxCollection;

class ImageArea : public QLabel
{
  Q_OBJECT

public:
  ImageArea(QWidget *parent = NULL);

  void setPixmap(const QPixmap &p);

  void getBBoxes(AABBoxCollection &bboxes) const;

  void setBBoxes(const AABBoxCollection &bboxes);

  int getNbBBoxes() const
  {
    return m_bboxes.size();
  }

  float getRatio() const
  {
    return m_scaledRatio;
  }

  void setRatio(float ratio)
  {
    m_scaledRatio = ratio;
  }

  bool hasSameSizePixmap(const QPixmap &p);

  void computeRatio(const QPixmap &originalPixmap, const QPixmap &scaledPixmap);

  void computeRatio(const QPixmap &pixmap, int width, int height);

public slots:
  
  void clearBBoxes();
  void removeSelectedBBox();
  int intersectedBBox(int x, int y) const;


protected:
  
  virtual void mouseMoveEvent( QMouseEvent *event);
    
  virtual void mousePressEvent(QMouseEvent *event);
    
  virtual void mouseReleaseEvent(QMouseEvent *event);
    
  virtual void paintEvent(QPaintEvent *event);

  virtual void resizeEvent(QResizeEvent *event);

  void correctBBox(int &x0, int &y0, int &x1, int &y1);

  void toOriginalBBox(AABBox &originalBBox, const AABBox &scaledBBox, float ratio) const;

  void toScaledBBox(AABBox &scaledBBox, const AABBox &originalBBox, float ratio) const;
  
  void scalePixmap(int width, int height);

protected:
  AABBoxCollection m_bboxes;
  int m_selectionIndex;
  
  QPoint m_startPos;
  bool m_drawing;
  bool m_moving;
  QPixmap m_originalPixmap; // 20110718: Lam added
  float m_scaledRatio; // 20110721: Lam added
};

#endif /* ! IMAGEAREA_HPP */
