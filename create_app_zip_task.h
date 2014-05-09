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
