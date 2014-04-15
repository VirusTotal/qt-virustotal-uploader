#ifndef RESCAN_FILE_TASK_H
#define RESCAN_FILE_TASK_H

#include <QObject>
#include <QRunnable>

#include "qvtfile.h"

class RescanFileTask : public QObject, public QRunnable
{
  Q_OBJECT;
  private:
    QVtFile *file;

  public:
    RescanFileTask(QVtFile *file);
    void run(void);

  signals:
    void LogMsg(int log_level, int err_code, QString Msg);
};

#endif // RESCAN_FILE_TASK_H
