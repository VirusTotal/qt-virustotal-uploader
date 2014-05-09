#include "vt_uploader_application.h"
#include <QFileOpenEvent>
#include <QDebug>

VTUploaderApplication::VTUploaderApplication(int & argc, char ** argv) : QApplication(argc, argv)
{
}


bool VTUploaderApplication::event(QEvent *event)
{
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug() << "File Open event";
      emit loadFile(static_cast<QFileOpenEvent *>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}
