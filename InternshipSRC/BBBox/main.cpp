#include <QApplication>
#include "MainWindow.hpp"


int
main(int argc, char* argv[])
{  
  QApplication app(argc, argv);

  MainWindow *window = new MainWindow; //Must be allocated on heap (if Qt::WA_DeleteOnClose attribute is set) !!! 
  window->show();

  return app.exec();
}

