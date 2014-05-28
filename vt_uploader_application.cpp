#include "vt_uploader_application.h"
#include <QFileOpenEvent>
#include <QFileInfo>
#include <QDebug>

VTUploaderApplication::VTUploaderApplication(int & argc, char ** argv) : QApplication(argc, argv)
{
}


bool VTUploaderApplication::event(QEvent *event)
{
  QFileInfo file_info;

  switch (event->type()) {
    case QEvent::FileOpen:
      file_info.setFile(static_cast<QFileOpenEvent *>(event)->file());
      qDebug() << "File Open event";

      if (file_info.isFile()) {
        emit loadFile(static_cast<QFileOpenEvent *>(event)->file());
      } else if (file_info.isDir()) {
        emit loadDir(static_cast<QFileOpenEvent *>(event)->file());
      }

      return true;
    default:
      return QApplication::event(event);
  }
}
