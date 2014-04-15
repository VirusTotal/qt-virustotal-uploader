#ifndef SCAN_FILE_TASK_H
#define SCAN_FILE_TASK_H

#include <QObject>
#include <QRunnable>

#include "qvtfile.h"

class ScanFileTask :public QObject, public QRunnable
{
  Q_OBJECT

private:
  QVtFile *file;

public:
  ScanFileTask(QVtFile *file);

  void ScanSmallFile(void);
  void run(void);

signals:
 void LogMsg(int log_level, int err_code, QString Msg);
};

#endif // SCAN_FILE_TASK_H
