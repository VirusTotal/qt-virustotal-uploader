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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

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
  QNetworkAccessManager *net_manager;
  long long int state_counter;
  int concurrent_uploads;

  void ReDrawScannerTable(void);
  void RunStateMachine(void);
  void ExportReport(QString file_name);

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
  void AddAppBundle(QString bundle_path);
  void DisplayTosDialog(void);
  void DisplayFileDialog(void);
  void DisplayExportDialog(void);
  void replyFinished(QNetworkReply*reply);

public slots:
  void AddFile(QString file_path);
  void AddDir(QString path);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
  void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
