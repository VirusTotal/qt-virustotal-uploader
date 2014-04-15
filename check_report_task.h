#ifndef CHECK_REPORT_TASK_H
#define CHECK_REPORT_TASK_H

#include <QObject>
#include <QRunnable>

#include "qvtfile.h"

class CheckReportTask : public QObject, public QRunnable
{
  Q_OBJECT

private:
  QVtFile *file;

public:
  CheckReportTask(QVtFile  *file);
   void run(void);

signals:
  void LogMsg(int log_level, int err_code, QString Msg);

};

#endif // CHECK_REPORT_TASK_H
