#ifndef CALC_FILE_HASHES_TASK_H
#define CALC_FILE_HASHES_TASK_H

#include <QObject>
#include <QRunnable>
#include "qvtfile.h"

class CalcFileHashesTask :  public QObject, public QRunnable
{
  Q_OBJECT
  QVtFile *file;
public:
  CalcFileHashesTask(QVtFile  *file);

  void run(void);

signals:
  void LogMsg(int log_level, int err_code, QString Msg);

};

#endif // CALC_FILE_HASHES_TASK_H

