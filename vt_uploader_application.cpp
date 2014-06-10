/*
Copyright 2014 VirusTotal S.L. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
