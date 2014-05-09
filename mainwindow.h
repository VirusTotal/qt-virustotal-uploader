#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QTimer>



#include "qvtfile.h"
#include "settings_dialog.h"
#include "tos_dialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

  void changed(const QMimeData *mimeData = 0);
  void dropped(const QMimeData *mimeData = 0);

private:
  Ui::MainWindow *ui;
  unsigned int req_per_minute_quota;
  QTimer *minute_timer;
  QTimer *state_timer;
  QVector<QVtFile *> file_vector;
  //QIcon icon;
  QMenuBar *menuBar;
  SettingsDialog *settings_dialog;
  ToSDialog *tos_dialog;
  long long int state_counter;
  int concurrent_uploads;

  void ReDrawScannerTable(void);
  void RunStateMachine(void);
  void AddDir(QString path);

private slots:
  void LogMsgRecv(int level, int code, QString Msg);
  void OnDropRecv(const QMimeData *data);
  void MinuteTimerSlot(void);
  void StateTimerSlot(void);
  void customMenuRequested(QPoint pos);
  void RemoveRowSlot(void);
  void ViewOnVTSlot(void);
  void RescanRowSlot(void);
  void DisplayPeferencesWindow();
  void AddFile(QString file_path);
  void AddAppBundle(QString bundle_path);
  void DisplayTosDialog(void);
  void DisplayFileDialog(void);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
  void closeEvent(QCloseEvent *event);
  bool event(QEvent *event);


};

#endif // MAINWINDOW_H
