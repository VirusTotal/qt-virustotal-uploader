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

#ifndef ADD_DIR_TASK_H
#define ADD_DIR_TASK_H

#include <QObject>
#include <QRunnable>
#include <QFileInfo>


class AddDirTask :  public QObject, public QRunnable
{
  Q_OBJECT
  QString dir_path;
  bool only_executables;


  void ScanDir(QString path, unsigned int depth);
  void ScanPath(QString path, unsigned int depth);
  void ScanPath(QFileInfo file_info, unsigned int depth);


  public:
    AddDirTask(QString path);
    void run(void);

  signals:
    void LogMsg(int log_level, int err_code, QString Msg);
    void AddFile(QString path);
    void AddAppBundle(QString path);
};

#endif // ADD_DIR_TASK_H
