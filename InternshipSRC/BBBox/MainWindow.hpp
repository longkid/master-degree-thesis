#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "ImageArea.hpp"

#include "FFMPEGPlayer.hpp"
#include "FFMPEGObserver.hpp"


class QPushButton;
class QCheckBox;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:

  /**
   * @brief Constructor.
   * 
   * @Warning must be allocated on heap (as Qt::WA_DeleteOnClose attribute is set).
   */
  MainWindow(QWidget* parent = NULL);

  ~MainWindow();

protected slots:

  void open();
  void openBBoxesFile();
  bool saveBBoxes();
  bool saveAsBBoxes();
  

  void clearFrameBBoxes();
  void removeFrameSelectedBBox();
  void toggleKeepBBox(int state);

  void advanceToNextFrame();

protected:

  void closeEvent(QCloseEvent *event);


  void buildGUI();

  bool decodeImage();

  bool goToFrame(size_t f);

  int foundResultForFrame(int frameNb) const;


  bool maybeSaveBBoxes();
  void saveBBoxesFile(const QString &filename);
  void setCurrentBBoxesFile(const QString &filename);
  bool writeBBoxesFile(const QString &filename);

  virtual void keyPressEvent(QKeyEvent *event);

//   virtual void mouseMoveEvent( QMouseEvent *event);
    
//   virtual void mousePressEvent(QMouseEvent *event);
    
//   virtual void mouseReleaseEvent(QMouseEvent *event);
    
//   virtual void paintEvent(QPaintEvent *event);
    
protected:

  ImageArea *m_imageArea;
  QPushButton *m_clearFrameBBoxesButton;
  QPushButton *m_removeFrameSelectedBBoxButton;
  QCheckBox *m_keepBBoxesOnNextFrameCB;
  
  QPushButton *m_nextFrameButton;
  QLabel *m_frameLabel;


  struct FrameResult
  {
    FrameResult(int frame, const AABBoxCollection &col)
      : bboxes(col), frameNb(frame)
    {}


    AABBoxCollection bboxes;
    int frameNb;

  };

  typedef QList<FrameResult> AABBoxCollectionOfCollection;

  AABBoxCollectionOfCollection m_results;
  
  
  VFB::FFMPEGPlayer m_decoder;
  VFB::FFMPEGObserver *m_observer;
  std::string m_filename;

  //char *m_buf;
  QImage m_image;
  size_t m_currentIndex;

  QString m_prefix;
  QString m_currFile;
  bool m_unnamed;
  bool m_decoderStarted;
};

bool checkFitInsideScreen(const QRect& screen, const QSize &imageSize);

#endif /* ! MAINWINDOW_HPP */
