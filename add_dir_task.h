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
  void AddAppBundle(QString path);

  public:
    AddDirTask(QString path);
    void run(void);

  signals:
    void LogMsg(int log_level, int err_code, QString Msg);
    void AddFile(QString path);
};

#endif // ADD_DIR_TASK_H
