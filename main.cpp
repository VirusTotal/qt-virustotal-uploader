#include "mainwindow.h"
#include "vt_uploader_application.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  //QApplication a(argc, argv);
  VTUploaderApplication a(argc, argv);
  MainWindow w;


  QObject::connect(&a, SIGNAL(loadFile(QString)),
    &w, SLOT(AddFile(QString)),  Qt::QueuedConnection);

  w.show();

  return a.exec();
}
