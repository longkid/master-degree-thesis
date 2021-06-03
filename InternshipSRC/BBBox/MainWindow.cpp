#include "MainWindow.hpp"

#include <cassert>
#include <iostream>
#include <fstream>

#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QBoxLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QSizePolicy>

#include <FFMPEGObserver.hpp>
#define DEBUG 1

static const char *NAME="BBoxes GT GUI";
static const char *SUFFIX="_faces.txt";

static const int DEFAULT_IMAGE_WIDTH = 352;
static const int DEFAULT_IMAGE_HEIGHT = 288;

// 20110811: Lam Ho added
static const int TITLE_BAR_HEIGHT = 26;
QRect* availableScreen = new QRect();
int buttonHeight = 0;
int vboxLayoutSpacing = 0;
int vboxLayoutMargin = 0;

class Observer : public VFB::FFMPEGObserver
{
public:
  Observer(ImageArea *ia, size_t &currentIndex)
    : m_ia(ia), m_currentIndex(currentIndex), m_updateImage(true)
  {}

  virtual void update(VFB::FFMPEGFrame &);

  bool updateImage() const { return m_updateImage; }
  
  void setUpdateImage(bool onoff) { m_updateImage = onoff; }

protected:

  ImageArea *m_ia;
  size_t &m_currentIndex;
  bool m_updateImage;
};

void
Observer::update(VFB::FFMPEGFrame &f)
{
  if (m_updateImage) {
    VFB::shared_ptr<VFB::RGB32Frame> f_rgba(new VFB::RGB32Frame);
    f.convert(f_rgba);
  
    //QImage::QImage ( uchar * data, int width, int height, int bytesPerLine, Format format )

    QImage img(f_rgba->data(), f_rgba->width(), f_rgba->height(), f_rgba->linesize(), QImage::Format_RGB32);

    QSize imgSize = img.size();
    bool isFit = checkFitInsideScreen(*availableScreen, imgSize);

    QPixmap pixmap = QPixmap::fromImage(img);
    if (m_ia->hasSameSizePixmap(pixmap)) {
      // do not change the scaled ratio
    } else {
      if (isFit) {
	// do not resize pixmap => ratio = 1
	m_ia->setRatio(1);
      } else {
	// resize to fit
	m_ia->computeRatio(pixmap, availableScreen->width()/2, availableScreen->height()/2);
      }
    }
    m_ia->setPixmap(pixmap);
    QSize iaSize = m_ia->size();
    QSize pixmapSize = m_ia->pixmap()->size();
    if ((iaSize.width() > pixmapSize.width())
	|| (iaSize.height() > pixmapSize.height())) {
      m_ia->adjustSize();
    }
  }

  m_currentIndex = f.pictNumber();
  
}

bool
checkFitInsideScreen(const QRect& screen, const QSize &imageSize)
{
  // Compute MainWindow size after setting this pixmap
  int mwWidth = imageSize.width() + 2 * vboxLayoutMargin; // add the size of left & right margin
  int mwHeight = imageSize.height() + TITLE_BAR_HEIGHT
    + 4 * buttonHeight + 6 * vboxLayoutSpacing;
  
  // Check if main window is inside the specified screen or not
  QRect guiRect(screen.x(), screen.y(), mwWidth, mwHeight);
  return screen.contains(guiRect);
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent),
    //m_buf(NULL),
    m_observer(NULL),
    m_currentIndex(0),
    m_unnamed(true),
    m_decoderStarted(false)
{
  buildGUI();

  setCurrentBBoxesFile("");
}

MainWindow::~MainWindow()
{
  
  //delete [] m_buf;
}

void
MainWindow::buildGUI()
{
  QPushButton *openButton = new QPushButton(tr("Open movie..."), this);
  QPushButton *openBBoxesFileButton = new QPushButton(tr("Open BBoxes file..."), this);
  QPushButton *saveBBoxesFileButton = new QPushButton(tr("Save BBoxes file..."), this);
  QPushButton *saveAsBBoxesFileButton = new QPushButton(tr("Save As BBoxes file..."), this);
  
  m_imageArea = new ImageArea(this);

  QPixmap p(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT);
  p.fill(Qt::black);
  m_imageArea->setPixmap(p);
  // 20110729: Lam Ho added
  // This command solve the issue which the content of image area does not
  // expand to the whole available size of this area
  m_imageArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  m_clearFrameBBoxesButton = new QPushButton(tr("Remove all BBoxes"), this);
  m_removeFrameSelectedBBoxButton = new QPushButton(tr("Remove selected BBox"), this);

  m_keepBBoxesOnNextFrameCB = new QCheckBox(tr("Keep BBoxes on next frame"), this);
  m_keepBBoxesOnNextFrameCB->setCheckState(Qt::Checked); //default is checked


  m_nextFrameButton = new QPushButton(tr("Next frame"), this);
  m_nextFrameButton->setEnabled(false);

  m_frameLabel = new QLabel(tr("0"), this);
  
  QHBoxLayout *h1 = new QHBoxLayout;
  h1->addWidget(openButton);
  h1->addWidget(openBBoxesFileButton);
  QHBoxLayout *h2 = new QHBoxLayout;
  h2->addWidget(saveBBoxesFileButton);
  h2->addWidget(saveAsBBoxesFileButton);

  QHBoxLayout *h3 = new QHBoxLayout;
  QVBoxLayout *v1 = new QVBoxLayout;
  v1->addWidget(m_removeFrameSelectedBBoxButton);
  v1->addWidget(m_clearFrameBBoxesButton);
  QHBoxLayout *v21 = new QHBoxLayout;
  v21->addWidget(m_nextFrameButton);
  v21->addWidget(m_frameLabel);

  QVBoxLayout *v2 = new QVBoxLayout;
  v2->addLayout(v21);
  v2->addWidget(m_keepBBoxesOnNextFrameCB);

  h3->addLayout(v1);
  h3->addLayout(v2);


  QVBoxLayout *vL = new QVBoxLayout;
  vL->addLayout(h1);
  vL->addLayout(h2);
  vL->addWidget(m_imageArea);
  vL->addLayout(h3);

  QWidget *w = new QWidget(this);
  w->setLayout(vL);
  setCentralWidget(w);

  // 20110811: Lam Ho added
  // These size is used to calculate the size of main window
  // in order to show the pixmap inside the desktop screen
  QDesktopWidget* desktop = QApplication::desktop();
  QRect rect = desktop->availableGeometry();
  availableScreen = new QRect(rect.topLeft(), rect.size());
  buttonHeight = openButton->height();
  vboxLayoutSpacing = vL->spacing();
  vboxLayoutMargin = vL->margin();
  // Move the main window to the top-left corner of the available screen
  // in order to check exactly if the window is inside the screen
  move(availableScreen->x(), availableScreen->y());

  connect(openButton, SIGNAL(clicked()),
	  this, SLOT(open()));
  connect(openBBoxesFileButton, SIGNAL(clicked()),
	  this, SLOT(openBBoxesFile()));
  connect(saveBBoxesFileButton, SIGNAL(clicked()),
	  this, SLOT(saveBBoxes()));
  connect(saveAsBBoxesFileButton, SIGNAL(clicked()),
	  this, SLOT(saveAsBBoxes()));
  
  connect(m_clearFrameBBoxesButton, SIGNAL(clicked()),
	  this, SLOT(clearFrameBBoxes()));
  connect(m_removeFrameSelectedBBoxButton, SIGNAL(clicked()),
	  this, SLOT(removeFrameSelectedBBox()));

  connect(m_keepBBoxesOnNextFrameCB, SIGNAL(stateChanged(int)),
	  this, SLOT(toggleKeepBBox(int)));

  connect(m_nextFrameButton, SIGNAL(clicked()),
	  this, SLOT(advanceToNextFrame()));
}

void
MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open movie"), "", tr("Movie files (*.mpg *.mpeg *.mp2 *.mp4);; All files (*.*)"));
  if (!fileName.isEmpty()) {
 
    //const int prevWidth = m_decoder.getWidth();
    //const int prevHeight = m_decoder.getHeight();
    

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const std::string s = fileName.toStdString();
    int res = m_decoder.open(s);
    //    m_decoder.open(fileName.toStdString());
    QApplication::restoreOverrideCursor();
    
    if (res != 0) {

      QMessageBox::critical(this, tr("Open movie"), tr("Movie has not been loaded."));
      m_decoderStarted = false;
    }
    else {

      m_decoderStarted = true;
      m_filename = s;
      
      delete m_observer;
      m_observer = new Observer(m_imageArea, m_currentIndex);
      m_decoder.registerObserver(m_observer);

      QFileInfo fi(fileName);
      m_prefix = fi.completeBaseName();
      if (m_unnamed) {
	setCurrentBBoxesFile(""); //change default name
      }

      //m_decoder.setOutputFormat(Mpeg2Decoder::RGB32);
      //m_decoder.setOutputFrequency(Mpeg2Decoder::OUTPUT_I);
      
      m_nextFrameButton->setEnabled(true);

      if (m_results.size() > 0) {

	//maybeSave ???

	bool imageOk = true;

	const int ret = QMessageBox::question(this, tr("Clear current results"), tr("Do you want to clear current results ?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
	if (ret == QMessageBox::Yes) {
	  m_results.clear();
	  m_imageArea->clearBBoxes();
	
	  setCurrentBBoxesFile("");
	  imageOk = decodeImage();
	}
	else {

	  const int ret2 = QMessageBox::question(this, tr("Go to last frame"), tr("Do you want to go to last processed frame ?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
	
	  QApplication::setOverrideCursor(Qt::WaitCursor);

	  if (ret2 == QMessageBox::Yes) {
	    const FrameResult &r = m_results.back();
	  
	    imageOk = goToFrame(r.frameNb);
	    
	    assert(m_currentIndex >= r.frameNb);
	  
	    m_imageArea->setBBoxes(r.bboxes);
	  
	  }
	  else {
	  
	    m_keepBBoxesOnNextFrameCB->setCheckState(Qt::Unchecked);


	    imageOk = decodeImage();
	    const int resultIndex = foundResultForFrame(m_currentIndex);	
	    if (resultIndex != m_results.size() && m_results.at(resultIndex).frameNb==0) {
	      m_imageArea->setBBoxes(m_results.at(resultIndex).bboxes);
	    }

	  }
      
	  QApplication::restoreOverrideCursor();

	}

	if (! imageOk) {
	  QMessageBox::critical(this, tr("Decode movie"), tr("Unable to decode image of movie."));
	}
	

      }


    }
    
  }   
}


bool
MainWindow::maybeSaveBBoxes()
{
  if (isWindowModified()) {
    const int ret = QMessageBox::warning(this, tr("Save"), 
					 tr("Do you want to save your changes?"),
					 QMessageBox::Yes | QMessageBox::Default,
					 QMessageBox::No,
					 QMessageBox::Cancel | QMessageBox::Escape);
    if (ret == QMessageBox::Yes)
      return saveBBoxes();
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}

bool
MainWindow::saveBBoxes()
{
  if (m_unnamed) {
    return saveAsBBoxes();
  }
  else {
    saveBBoxesFile(m_currFile);
    return true;
  }
}

void
MainWindow::saveBBoxesFile(const QString &fileName)
{
  const bool writeOk = writeBBoxesFile(fileName);
  if (writeOk) {
    setCurrentBBoxesFile(fileName);
  }  
}

bool
MainWindow::saveAsBBoxes()
{
 const QString fileName = QFileDialog::getSaveFileName(this, tr("Save BBoxes file"),
						       m_currFile, tr("Text Files (*.txt);; All Files (*.*)"));

  if (fileName.isEmpty())
    return false;

  saveBBoxesFile(fileName);

  return true;
}

QString
strippedName(const QString &fullfileName)
{
  return QFileInfo(fullfileName).fileName();
}


void
MainWindow::setCurrentBBoxesFile(const QString &filename)
{
  static int docNumber = 1;

  if (filename.isEmpty()) {
    m_unnamed = true;
    if (m_prefix.isEmpty()) {
      QFileInfo fi;
      do {
	m_currFile = tr("document%1.txt").arg(docNumber++);	
	fi.setFile(m_currFile);
      }
      while (fi.exists());
    }
    else {
      QFileInfo fi;
      do {
	m_currFile = m_prefix+QString("_")+QString::number(docNumber++)+QString(SUFFIX);
 	fi.setFile(m_currFile);
     }
      while (fi.exists());
    }
  }
  else {
    m_currFile = filename;
    m_unnamed = false;
  }

  setWindowModified(false);
  setWindowTitle(tr("%1[*] - %2").arg(strippedName(m_currFile)).arg(tr(NAME)));
}



void
MainWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSaveBBoxes()) {
    event->accept();
  }
  else {
    event->ignore();
  }
}


void
MainWindow::openBBoxesFile()
{
  if (maybeSaveBBoxes()) {

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open BBoxes file"), "", tr("BBoxes files (*.txt);; All files (*.*)"));
    if (!fileName.isEmpty()) {
 
      std::ifstream inputFile(fileName.toAscii().constData());
      if (! inputFile) {
	QMessageBox::critical(this, tr("Open BBoxes file"), tr("Unable to open input file: %1").arg(fileName));
      
	return;
      }

      QApplication::setOverrideCursor(Qt::WaitCursor);
    
      m_results.clear();

      size_t line = 1;
      while (! inputFile.eof() && inputFile.good()) {

	int frameNb;
	inputFile >> frameNb;
	if (inputFile.eof() || ! inputFile.good())
	  break;
      
	if (! m_results.isEmpty()) {
	  if (m_results.back().frameNb >= frameNb) {
	    QMessageBox::warning(this, tr("Open BBoxes file"), tr("File: %1\nError: line: %2: frame number inferior to previous frame number").arg(fileName).arg(line));
	    break;
	  }
	}

	int nbBBoxes;
	inputFile >> nbBBoxes;
	AABBoxCollection bboxes;
	for (int i=0; i<nbBBoxes; ++i) {
	  int x0, y0, w, h;
	  inputFile >> x0;
	  inputFile >> y0;
	  inputFile >> w;
	  inputFile >> h;
	  AABBox bb(x0, y0, w, h);
	  bboxes.push_back(bb);
	}
	FrameResult r(frameNb, bboxes);
	m_results.push_back(r);
      
	++line;
      }
    
      QApplication::restoreOverrideCursor();

      setCurrentBBoxesFile(fileName);

      if (m_decoderStarted)
	m_nextFrameButton->setEnabled(true);
    
      if (m_results.size() > 0 && m_decoderStarted) {
	const int ret = QMessageBox::question(this, tr("Go to last frame"), tr("Do you want to go to last processed frame ?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (ret == QMessageBox::Yes) {
	  const FrameResult &r = m_results.back();
	
	  if (r.frameNb != m_currentIndex)  {
	    if (r.frameNb<m_currentIndex) {
	      m_decoder.close();
	      int res = m_decoder.open(m_filename);
	      assert(res == 0);
	    }

	    bool imageOk = goToFrame(r.frameNb);

	  }

	  if (m_currentIndex == r.frameNb)
	    m_imageArea->setBBoxes(r.bboxes);
	
	}
	else {
	
	  m_keepBBoxesOnNextFrameCB->setCheckState(Qt::Unchecked);

	  const int resultIndex = foundResultForFrame(m_currentIndex);	
	  if (resultIndex != m_results.size() && m_results.at(resultIndex).frameNb==0) {
	    m_imageArea->setBBoxes(m_results.at(resultIndex).bboxes);
	  }

	}
      
	QApplication::restoreOverrideCursor();
      
      } 

    
    }

  }
}

bool
MainWindow::writeBBoxesFile(const QString &fileName)
{
  std::ofstream outputFile(fileName.toAscii().constData());
  if (! outputFile) {
    QMessageBox::critical(this, tr("Save BBoxes file"), tr("Unable to open ouput file: %1").arg(fileName));
    
    return false;
  }
  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  
  const int nbFrames = m_results.size();
  
  for (int i=0; i<nbFrames; ++i) {
    
    const FrameResult &r = m_results.at(i);
    const int frameNb = r.frameNb;
    const AABBoxCollection &bboxes = r.bboxes;
    const int nbBBoxes = bboxes.size();
    
    outputFile<<frameNb<<" "<<nbBBoxes;
    for (int j=0; j<nbBBoxes; ++j) {
      const AABBox &bb = bboxes.at(j);
      outputFile<<" "<<bb.xMin()<<" "<<bb.yMin()<<" "<<bb.width()<<" "<<bb.height();
    }
    outputFile<<"\n";
    
  }
  
  QApplication::restoreOverrideCursor();
  
  return true;
}

void
MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
  case Qt::Key_N:
    advanceToNextFrame();
    break;
  case Qt::Key_S:
    saveBBoxes();
    break;
  case Qt::Key_R:
    removeFrameSelectedBBox();
    break;
  default:
    QWidget::keyPressEvent(event);
  }
}

void
MainWindow::clearFrameBBoxes()
{
  m_imageArea->clearBBoxes();
}

void
MainWindow::removeFrameSelectedBBox()
{
  m_imageArea->removeSelectedBBox();
}

void
MainWindow::toggleKeepBBox(int state)
{

}

/**
 * Return index in m_results of result with frameNb equal or immediatly superior to frameNb.
 * Return m_results.size() if no such result is found.
 *
 * Suppose that m_results is sorted in increasing order acoording to frameNb.
 */
int
MainWindow::foundResultForFrame(int frameNb) const
{
  const int nbResults = m_results.size();
  for (int i=0; i<nbResults; ++i) {
    if (m_results.at(i).frameNb>=frameNb)
      return i;
  }
  return nbResults;
}


void
MainWindow::advanceToNextFrame()
{
  //- get bbox for current frame
  AABBoxCollection bboxes;
  m_imageArea->getBBoxes(bboxes);

  FrameResult result(m_currentIndex, bboxes);

  const int resultIndex = foundResultForFrame(m_currentIndex);
  if (resultIndex < m_results.size()) {
    
    //std::cerr<<"advance: foundResultForFrame("<<m_currentIndex<<")="<<resultIndex<<" < m_results.size()="<<m_results.size()<<std::endl;
    //std::cerr<<"  m_results.at("<<resultIndex<<").frameNb="<<m_results.at(resultIndex).frameNb<<" =?= "<<m_currentIndex<<std::endl;

    if (m_results.at(resultIndex).frameNb == m_currentIndex)
      m_results[resultIndex] = result;
    else {
      //insert
      m_results.insert(resultIndex, result);
    }
  }
  else {
    m_results.push_back(result);
  }

  //- get and display next frames
  const bool nextOk = decodeImage(); //update m_currentIndex
      
  if (nextOk) {

    setWindowModified(true);

    //- set bboxes of previous frame on next frame if necessary
#if 0
    if (m_keepBBoxesOnNextFrameCB->checkState() == Qt::Checked) {
      m_imageArea->setBBoxes(bboxes);
    }
    else {
#else
    if (m_keepBBoxesOnNextFrameCB->checkState() != Qt::Checked) {      
#endif 
      m_imageArea->clearBBoxes();
    

      const int resultIndex2 = foundResultForFrame(m_currentIndex);
      //TODO:OPTIM: we search from 0, but we could search from resultIndex

      /*
      std::cerr<<"foundResultForFrame("<<m_currentIndex<<") = "<<resultIndex2<<" on m_results.size()="<<m_results.size()<<std::endl;
      if (resultIndex2 < m_results.size())
	std::cerr<<"  m_results.at("<<resultIndex2<<").frameNb="<<m_results.at(resultIndex2).frameNb<<" nb bboxes="<<m_results.at(resultIndex2).bboxes.size()<<std::endl;
      */

      if (resultIndex2 < m_results.size() 
	  && m_results.at(resultIndex2).frameNb == m_currentIndex) {
	m_imageArea->setBBoxes(m_results.at(resultIndex2).bboxes);

	//std::cerr<<"set bbox in results: nb bboxes="<<m_results.at(resultIndex2).bboxes.size()<<std::endl;
      }
      

    }
  }

}

#if 0
void
MainWindow::mouseMoveEvent(QMouseEvent *event)
{
  std::cerr<<"mouseMoveEvent"<<std::endl;
  QMainWindow::mouseMoveEvent(event);
}

void
MainWindow::mousePressEvent(QMouseEvent *event)
{
  std::cerr<<"mousePressEvent"<<std::endl;
  QMainWindow::mousePressEvent(event);

}

void
MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
  std::cerr<<"mouseReleaseEvent"<<std::endl;
  QMainWindow::mouseReleaseEvent(event);

}

void
MainWindow::paintEvent(QPaintEvent *event)
{
  std::cerr<<"paintEvent"<<std::endl;
  QMainWindow::paintEvent(event);

}
#endif //0


bool
MainWindow::goToFrame(size_t frameNb)
{
  assert(frameNb > m_currentIndex);

  const bool ui = reinterpret_cast<Observer *>(m_observer)->updateImage();
  
  bool imageOk = true;
  const size_t frameNbm1 = frameNb - 1;
  reinterpret_cast<Observer *>(m_observer)->setUpdateImage(false);
  
  while (m_currentIndex < frameNbm1 && imageOk) {
    imageOk = decodeImage();
  }
  
  reinterpret_cast<Observer *>(m_observer)->setUpdateImage(ui);
  while (m_currentIndex < frameNb && imageOk) {
    imageOk = decodeImage();
  }

  return imageOk;
}      



bool
MainWindow::decodeImage()
{
  bool decodeOk = m_decoder.decodeNextFrame();
  if (decodeOk) {

    m_frameLabel->setText(tr("%1").arg(m_currentIndex));

    return true;
  }
  else {
    std::cerr<<"unable to decode next image"<<std::endl;

    m_nextFrameButton->setEnabled(false);
  }

  return false;
}
