#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QTimer>


#include "vtfile.h"

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
  QVector<VtFile *> file_vector;

  void ReDrawScannerTable(void);
  void RunStateMachine(void);

private slots:
  void __onLogMsgRecv(int level, int code, QString Msg);
  void OnDropRecv(const QMimeData *data);
  void MinuteTimerSlot(void);
  void StateTimerSlot(void);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);

};

#endif // MAINWINDOW_H
