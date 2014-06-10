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

#ifndef CREATE_APP_ZIP_TASK_H
#define CREATE_APP_ZIP_TASK_H

#include <QObject>
#include <QRunnable>
#include "qvtfile.h"

class CreateAppZipTask : public QObject, public QRunnable
{
  Q_OBJECT;

  QVtFile  *vtfile;
  zipFile zf; // for App bundles
  qint64 total_size;
  qint64 processed_size;

  void OpenZip(QString filename);
  void SumDir(QString path, unsigned int depth);
  void SumSizePath(QFileInfo file_info, unsigned int depth);
  void AddZipDir(QString path, unsigned int depth);
  void AddZipPath(QFileInfo file_info, unsigned int depth);

  public:
    CreateAppZipTask(QVtFile  *file);
     void run(void);

  signals:
    void LogMsg(int log_level, int err_code, QString Msg);
};

#endif // CREATE_APP_ZIP_TASK_H
