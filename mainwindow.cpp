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

#include <QMessageBox>
#include <QTime>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QThreadPool>
#include <QLabel>
#include <QTextEdit>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings_dialog.h"
#include "scanner_table_widget.h"
#include "vt-log.h"
#include "qvtfile.h"
#include "add_dir_task.h"

#define LOG_TIME_COL 0
#define LOG_CODE_COL 1
#define LOG_LEVEL_COL 2
#define LOG_MSG_COL 3

#define SCAN_TABLE_FILE_COL   0
#define SCAN_TABLE_STATUS_COL 1
#define SCAN_TABLE_DETECTIONS_COL 2
#define SCAN_TABLE_DATE_COL 3
#define SCAN_TABLE_LINK_COL 4
#define SCAN_TABLE_MESSAGE_COL 5
#define SCAN_TABLE_SHA1_COL 6
#define SCAN_TABLE_SHA256_COL 7
#define SCAN_TABLE_MD5_COL 8
#define SCAN_TABLE_FULLPATH_COL 9


#define VT_UPLOADER_VERSION "1.3"

#define MAX_LOG_MSG_LINES 1000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  qDebug() << "Startup";
  ui->setupUi(this);
  setAcceptDrops(true);
  req_per_minute_quota = 0;
  settings_dialog = NULL;
  tos_dialog = NULL;
  state_counter = 0;

  QCoreApplication::setOrganizationName("VirusTotal");
  QCoreApplication::setOrganizationDomain("virustotal.com");
  QCoreApplication::setApplicationName("VirusTotal Uploader");


  emit LogMsgRecv(VT_LOG_DEBUG, 0 ,
      tr("VirusTotal Uploader") + " " + VT_UPLOADER_VERSION + " "
      + tr("Compiled: ") + __DATE__ + " " + __TIME__);

  // Drag drop signals
  connect(ui->ScannerTableWidget_scan_table, SIGNAL(dropped(const QMimeData*)), this, SLOT(OnDropRecv(const QMimeData*)));
  connect(this, SIGNAL(dropped(const QMimeData*)), this, SLOT(OnDropRecv(const QMimeData*)));

  connect(this, SIGNAL(dropped(const QMimeData*)), this, SLOT(OnDropRecv(const QMimeData*)));

  minute_timer = new QTimer(this);
  connect(minute_timer, SIGNAL(timeout()), this, SLOT(MinuteTimerSlot()));
  emit MinuteTimerSlot(); // initalize
  minute_timer->start(60000);

  state_timer = new QTimer(this);
  connect(state_timer, SIGNAL(timeout()), this, SLOT(StateTimerSlot()));
  state_timer->setSingleShot(true); // make single shot so it can't overun
  state_timer->start(400);

  ui->ScannerTableWidget_scan_table->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->ScannerTableWidget_scan_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  connect(ui->ScannerTableWidget_scan_table, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(customMenuRequested(QPoint)));


  ui->ScannerTableWidget_scan_table->horizontalHeader()->setSectionResizeMode(
       SCAN_TABLE_FULLPATH_COL, QHeaderView::Stretch);
  ui->tableWidget_log_msg->horizontalHeader()->setSectionResizeMode(
       LOG_MSG_COL, QHeaderView::Stretch);

  this->setWindowIcon(QIcon("vtlogo-sigma.png"));


  QAction *action_preferences = new QAction(tr("Preferences"), this);
  connect(action_preferences, SIGNAL(triggered()), this, SLOT(DisplayPeferencesWindow()));
  this->ui->menuFile->addAction(action_preferences);

  QAction *action_file_select = new QAction(tr("Select Files"), this);
  connect(action_file_select, SIGNAL(triggered()), this, SLOT(DisplayFileDialog()));
  this->ui->menuFile->addAction(action_file_select);


  QAction *action_export_select = new QAction(tr("Export Report"), this);
  connect(action_export_select, SIGNAL(triggered()), this, SLOT(DisplayExportDialog()));
  this->ui->menuFile->addAction(action_export_select);


  QAction *tos = new QAction(tr("VirusTotal Terms"), this);
  connect(tos, SIGNAL(triggered()), this, SLOT(DisplayTosDialog()));
  this->ui->menuHelp->addAction(tos);

  net_manager = new QNetworkAccessManager(this);
  connect(net_manager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(replyFinished(QNetworkReply*)));
  net_manager->get(QNetworkRequest(QUrl("https://www.virustotal.com/static/bin/osx/mac_uploader.json")));
}

MainWindow::~MainWindow()
{

  minute_timer->stop();
  state_timer->stop();

  delete minute_timer;
  delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();
  emit changed(event->mimeData());
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
  event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent * event)
{
  QString text = event->mimeData()->text();

  qDebug() << "MainWindow::dropEvent " << text;
  event->acceptProposedAction();
  emit dropped(event->mimeData());
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event) {
  event->accept();
}

//void QWidget::closeEvent ( QCloseEvent * event )   [virtual protected]
void MainWindow::closeEvent(QCloseEvent *event) {
  // do some data saves or something else
  qDebug() << "MainWindow::closeEvent ";
  QVtFile::CancelOperations();
  QThreadPool::globalInstance()->waitForDone();
  event->accept();
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();

  int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  qDebug() << "Update StatusCode:" << QVariant(status_code).toString();
  if (status_code == 200) {
    QString reply_str = QString::fromUtf8(bytes.data(), bytes.size());
    qDebug() << "Update JSON:" << reply_str << endl;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply_str.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QString latest_ver_str = jsonObject["latest_ver"].toString();
    qDebug() << "Latest version:" << latest_ver_str << endl;

#if defined(TARGET_OS_X) || defined(__APPLE__ )
    if (latest_ver_str != QString(VT_UPLOADER_VERSION)) {
      QMessageBox msgBox;
      msgBox.setTextFormat(Qt::RichText);
      msgBox.setText(
        "You have version: " VT_UPLOADER_VERSION " The latest version is: "
        + latest_ver_str +
        "<br>A new version of the VirusTotal Uploader is avaiable at: <br>"
        "<a href=\"https://www.virustotal.com/en/documentation/desktop-applications/mac-osx-uploader\">"
        "https://www.virustotal.com/en/documentation/desktop-applications/mac-osx-uploader</a>");
      msgBox.exec();
    }
#endif
  }

}


void MainWindow::LogMsgRecv(int log_level, int err_code, QString Msg)
{

  QTableWidgetItem *Time_Cell_Item;
  QTableWidgetItem *Code_Cell_Item;
  QTableWidgetItem *Level_Cell_Item;
  QTableWidgetItem *Msg_Cell_Item;
  int count;

  if (log_level < VT_LOG_ERR && ui->checkBox_popUpErrors->isChecked())
    QMessageBox::critical(this, "Critial Error", Msg);

  ui->tableWidget_log_msg->insertRow(0);


  Time_Cell_Item  = new QTableWidgetItem(QTime::currentTime().toString());
  Code_Cell_Item  = new QTableWidgetItem(QString("%1").arg(err_code));

  Level_Cell_Item = new QTableWidgetItem();

  if (err_code == 204) {
    req_per_minute_quota = 0;
  }

  switch (log_level)
  {
    case VT_LOG_EMERG:   // system is unusable
      Level_Cell_Item->setText("Emergency");
      break;
    case VT_LOG_ALERT:   // action must be taken immediately
      Level_Cell_Item->setText("Alert");
      break;
    case VT_LOG_CRIT:    // critical conditions
      Level_Cell_Item->setText("Critical");
      break;
    case VT_LOG_ERR:     // error conditions
      Level_Cell_Item->setText("Error");
      break;
    case VT_LOG_WARNING: // warning conditions
      Level_Cell_Item->setText("Warning");
      break;
    case VT_LOG_NOTICE:  // normal but significant condition
      Level_Cell_Item->setText("Notice");
      break;
    case VT_LOG_INFO:    // informational
      Level_Cell_Item->setText("Info");
      break;
    case VT_LOG_DEBUG:   // debug-level messages
      Level_Cell_Item->setText("Debug");
      break;
    default:
      Level_Cell_Item->setText(QString("Unknown %1").arg(log_level));
      break;
  }

  Msg_Cell_Item   = new QTableWidgetItem(Msg);

  ui->tableWidget_log_msg->setItem(0, LOG_TIME_COL, Time_Cell_Item );
  ui->tableWidget_log_msg->setItem(0, LOG_CODE_COL, Code_Cell_Item);
  ui->tableWidget_log_msg->setItem(0, LOG_LEVEL_COL, Level_Cell_Item);
  ui->tableWidget_log_msg->setItem(0, LOG_MSG_COL, Msg_Cell_Item);

  count = ui->tableWidget_log_msg->rowCount();
  if (count >= MAX_LOG_MSG_LINES) {
    ui->tableWidget_log_msg->removeRow(count-1);
  }

  ui->tableWidget_log_msg->resizeColumnsToContents();

}
void MainWindow::AddFile(QString file_path)
{
  QVtFile *file = NULL;

  for (int i = 0; i < file_vector.length(); i++) {
//    qDebug() << "Checking: " << file_vector[i]->fileName();
    if (file_vector[i]->fileName() == file_path)
      return;
  }
//  qDebug() << "Adding:" << file_path;
  file = new QVtFile(file_path, this);
  connect(file, SIGNAL(LogMsg(int,int,QString)),this, SLOT(LogMsgRecv(int,int,QString)));
  file_vector.append(file);
  ReDrawScannerTable();
}

void MainWindow::AddAppBundle(QString file_path)
{
  QVtFile *file = NULL;
  QFileInfo file_info(file_path);
  QString zip_name = QDir::tempPath() + "/" + file_info.bundleName() + ".app.zip";
  qDebug() << "AddAppBundle  zip_name: " << zip_name;

  file = new QVtFile(this);
  connect(file, SIGNAL(LogMsg(int,int,QString)),this, SLOT(LogMsgRecv(int,int,QString)));
  file->SetBundlePath(file_path);
  file_vector.append(file);
  ReDrawScannerTable();
}


void MainWindow::AddDir(QString path)
{
  AddDirTask *task = new AddDirTask(path);

  qDebug() << "AddDir : " << path;

  QObject::connect(task, SIGNAL(LogMsg(int,int,QString)),
    this, SLOT(LogMsgRecv(int,int,QString)),  Qt::QueuedConnection);

  QObject::connect(task, SIGNAL(AddFile(QString)),
    this, SLOT(AddFile(QString)),  Qt::QueuedConnection);

  QObject::connect(task, SIGNAL(AddAppBundle(QString)),
    this, SLOT(AddAppBundle(QString)), Qt::QueuedConnection);

  // QThreadPool takes ownership and deletes 'task' automatically
  QThreadPool::globalInstance()->start(task);
}

void MainWindow::OnDropRecv(const QMimeData *mime_data)
{

  emit LogMsgRecv(VT_LOG_DEBUG, 0 , tr("Dropped ") + mime_data->text());
  QString file_path;
  QFileInfo file_info;

  // check for our needed mime type, here a file or a list of files
  if (mime_data->hasUrls())
  {
   //  QStringList pathList;
    QList<QUrl> urlList = mime_data->urls();

    qDebug() << "urlList.size():"  << urlList.size();
    // extract the local paths of the files
    for (int i = 0; i < urlList.size() && i < 1024; ++i)
    {
      QUrl url = urlList.at(i);
      file_path = url.path();
      qDebug() << "urlList.at(i):"  << urlList.at(i);
      qDebug() << "url "  << " : "  << file_path;
      //pathList.append(urlList.at(i).toLocalFile());
      if (!url.isLocalFile()) {
        // not local
        emit LogMsgRecv(VT_LOG_INFO,
          0 , tr("non file::// types not supported.  Please request this feature. ")
          + mime_data->text());
        return;
      }
      file_info.setFile(file_path);

      if (!file_info.isReadable()) {
        emit LogMsgRecv(VT_LOG_ERR, 0 , tr("File not readable.") + file_path);
        continue;
      }
      if (file_info.isFile()) {
        emit AddFile(file_path);
      } else if (file_info.isDir()) {
        emit AddDir(file_path);
      } else {
        emit LogMsgRecv(VT_LOG_ERR, 0 , tr("Unknown file type ") + mime_data->text());
      }
    }

  }

  ReDrawScannerTable();
}

void MainWindow::MinuteTimerSlot(void)
{
  qDebug() << "MinuteTimerSlot";
  QSettings settings;
  int minute_quota = settings.value("quota/perMinute", 4).toInt();

  qDebug() << "Settings quota = " << minute_quota;
  req_per_minute_quota = minute_quota;
}

void MainWindow::RemoveRowSlot(void)
{
  QObject* obj = sender();
  QAction *action = qobject_cast<QAction *>(obj);
  int row = action->data().toInt();

  qDebug() << "Remove row: " << row;

  if (ui->ScannerTableWidget_scan_table->rowCount() > row)
    ui->ScannerTableWidget_scan_table->removeRow(row);
  else
    emit LogMsgRecv(VT_LOG_ERR, 0 , tr("Invalid Row ") + row);

  if (file_vector.count() > row)
    file_vector.remove(row);
  else
    emit LogMsgRecv(VT_LOG_ERR, 0 , tr("Invalid Row ") + row);
}

void MainWindow::ViewOnVTSlot(void)
{
  QObject* obj = sender();
  QAction *action = qobject_cast<QAction *>(obj);
  int row = action->data().toInt();

  if (row >= file_vector.count()) {
    emit LogMsgRecv(VT_LOG_ERR, 0 , tr("Invalid Row to view on VT") + row);
    return;
  }

  QString sha256str = ui->ScannerTableWidget_scan_table->item(row,SCAN_TABLE_SHA256_COL)->text();
  if (sha256str.length() < 10) {
    emit LogMsgRecv(VT_LOG_WARNING, 0 , tr("SHA not ready") + row);
    return;
  }
  QDesktopServices::openUrl("https://www.virustotal.com/latest-scan/"+sha256str);
}

void MainWindow::RescanRowSlot(void)
{
  QObject* obj = sender();
  QAction *action = qobject_cast<QAction *>(obj);
  int row = action->data().toInt();
  QVtFile *file = NULL;

  qDebug() << "Rescan row: " << row;

  if (ui->ScannerTableWidget_scan_table->rowCount() > row
  && file_vector.count() > row) {
    file = file_vector[row];
    file->ReScan();
  } else
    emit LogMsgRecv(VT_LOG_ERR, 0 , tr("Invalid Row ") + row);

}

void MainWindow::DisplayPeferencesWindow(void)
{
  qDebug() << "MainWindow::DisplayPeferencesWindow " << settings_dialog;

  if (!settings_dialog) {
    settings_dialog = new SettingsDialog(this);
  }

  settings_dialog->show();
}

void MainWindow::DisplayTosDialog(void)
{
  if (!tos_dialog) {
    tos_dialog = new ToSDialog(this);
    connect(tos_dialog, SIGNAL(ToSNotAccepted()), this, SLOT(close()));
  }

  tos_dialog->setModal(true);
  tos_dialog->show();
}

void MainWindow::DisplayFileDialog(void)
{
  QStringList files = QFileDialog::getOpenFileNames(this,
    tr("Select one or more files to open"));

  for (int i= 0; i < files.length(); i++) {
    QString filename = files[i];
    qDebug() << "DisplayFileDialog File:" << filename;
    AddFile(filename);
  }

}

void MainWindow::ExportReport(QString file_name)
{
  QFile file( file_name );
  if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
  {
      QTextStream stream( &file );
      for (int i = 0; i < file_vector.count(); i++) {
        QVtFile *file = file_vector.operator[](i);

        stream << "\"" << file->fileName() << "\",";
        if (file->GetTotalScans() > 1) {
          // If we have results show them.
          stream << "\"" << file->GetPositives() << "\",";
          stream << "\"" << file->GetTotalScans() << "\",";
        } else {
          // leave empty
          stream << "\"\",\"\",";
        }
        stream << "\"" << file->GetScanDate().toString(Qt::DefaultLocaleShortDate) << "\",";
        stream << "\"" << file->GetPermalink() << "\",";
        stream << "\"" << QString(file->GetMd5().toHex()) << "\",";
        stream << "\"" << QString(file->GetSha1().toHex()) << "\",";
        stream << "\"" << QString(file->GetSha256().toHex()) << "\"" << endl;

      }
      file.close();
  } else {
    QMessageBox msgBox;
    msgBox.setText(
      "Error opening file for writing: " + file_name);
    msgBox.exec();
  }
}

void MainWindow::DisplayExportDialog(void)
{

  if (file_vector.count() > 0) {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save Report"),
          "vt-report.csv", // dir
          tr("CSV (*.csv)"));
    qDebug() << "DisplayExportDialog File:" << file_name;
    ExportReport(file_name);
  } else {
    QMessageBox msgBox;
    msgBox.setText(
      "You have to scan a file before you can export the report!");
    msgBox.exec();

  }



}

void MainWindow::customMenuRequested(QPoint pos){
    QModelIndex index=ui->ScannerTableWidget_scan_table->indexAt(pos);
    int row = index.row();
    //QString sha256str;
    QAction *rescan_action = new QAction("ReScan", this);
    QAction *remove_action = new QAction("Remove from list", this);
    QAction *view_action = new QAction("View on VT", this);



    rescan_action->setData(row);
    remove_action->setData(row);
    view_action->setData(row);
    connect(rescan_action, SIGNAL(triggered()), this, SLOT(RescanRowSlot()));
    connect(remove_action, SIGNAL(triggered()), this, SLOT(RemoveRowSlot()));
    connect(view_action, SIGNAL(triggered()), this, SLOT(ViewOnVTSlot()));

//    qDebug() << "sha;" << sha256str;
    QMenu *menu=new QMenu(this);
    menu->addAction(rescan_action);
    menu->addAction(remove_action);
    menu->addAction(view_action);

    menu->popup(ui->ScannerTableWidget_scan_table->viewport()->mapToGlobal(pos));
    qDebug() << "index=" << index;
}

void MainWindow::ReDrawScannerTable(void)
{
  QTableWidgetItem *Path_Cell_Item;
  int num_files = file_vector.count();

  ui->ScannerTableWidget_scan_table->clear();
  ui->ScannerTableWidget_scan_table ->setRowCount(num_files+1);

  for (int i = 0; i < num_files ; i++) {

    QVtFile *file = file_vector.operator[](i);
    int last_slash = file->fileName().lastIndexOf(QDir::separator());


    // if not found it will be -1
    if (last_slash < 0)
      last_slash = 0;
    else
      last_slash+=1; // move off slash


//    ui->ScannerTableWidget_scan_table->insertRow(0);
    QString short_name = file->fileName().mid(last_slash);

//    qDebug() << "Last_slash=" << last_slash  << " Short name: " << short_name;
    Path_Cell_Item  = new QTableWidgetItem(short_name);

    ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_FILE_COL, Path_Cell_Item );
    QString state_str = file->GetStateStr();
    ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL, new QTableWidgetItem(state_str) );
    ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_FULLPATH_COL, new QTableWidgetItem(file->fileName()) );
    ui->ScannerTableWidget_scan_table->resizeColumnsToContents();


  }
}

void MainWindow::RunStateMachine(void)
{
  int num_files;
  QString link_url;
  QString detections_str;
  qint64 state_change_msec;
  const int wait_delay = 63000;
  QSettings settings;
  int rescan_days = settings.value(RESCAN_DAYS_KEY, DEFAULT_RESCAN_DAYS).toInt();

  if(state_counter == 0) {
    // things to run 1st time only

    if (settings.value("ui/show_tos", true).toBool()) {
      qDebug() << "show_tos " << settings.value("ui/show_tos", true).toBool();
      DisplayTosDialog();
    }
  }

  num_files = file_vector.count();

  concurrent_uploads  = 0;
  for (int i = 0; i < num_files ; i++) {
    QVtFile *file = file_vector.operator[](i);
    enum QVtFileState file_state = file->GetState();

    if (file_state == kScan)
      // count uploading files, so we don't have too many parallel
      concurrent_uploads++;
  }

  for (int i = 0; i < num_files ; i++) {
    QVtFile *file = file_vector.operator[](i);

    if (file->GetState() != KNoReportExists)
      continue;

    // do this pass before the other states below so we can prioritize uploads

    if (file->GetUploaded()) {
      // already uploaed  wait more
      qDebug() << "No Report..already uploaded,  wait more";
      file->SetState(kWaitForReport);
    } else {
      qDebug() << "No Report..Scan file quota=" << req_per_minute_quota
               << " concurrent_uploads=" << concurrent_uploads;
      if (req_per_minute_quota && concurrent_uploads < 3) {
        req_per_minute_quota--;
        file->Scan();
        concurrent_uploads++;
      }
    }
  }
  for (int i = 0; i < num_files ; i++) {
    QVtFile *file = file_vector.operator[](i);
    QString state_str = file->GetStateStr();
    enum QVtFileState file_state = file->GetState();
    int age_days = 0;

    state_change_msec = QDateTime::currentDateTime().toMSecsSinceEpoch()
                        - file->GetStateChangeTime().toMSecsSinceEpoch();

    ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL,
                                               new QTableWidgetItem(state_str) );
    switch (file_state)
    {

      case kNew:
       file->CalculateHashes();
       break;
      case kCheckingHash:
        break;
      case kHashesCalculated:

       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_SHA1_COL,
         new QTableWidgetItem(QString(file->GetSha1().toHex())));
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_SHA256_COL,
         new QTableWidgetItem(QString(file->GetSha256().toHex())));
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_MD5_COL,
         new QTableWidgetItem(QString(file->GetMd5().toHex())) );

       // if we have quota
       if (req_per_minute_quota) {
         req_per_minute_quota--;
         file->CheckReport();
       }
       break;
      case kCheckReport:
       break;
      case kCreateAppZip:
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL,
          new QTableWidgetItem(state_str + " "
          + QString::number(file->GetProgress(), 'f', 2) + "%") );
       break;
      case kReportFeteched:

       QLabel *link_label;

       // if finished scanning
       if (file->GetTotalScans() > 20) {
         QLabel *detections_label;
         if (file->GetPositives() == 0) {
           detections_str = "<span style='color:green'>";
         } else if (file->GetPositives() >= 5) {
           detections_str = "<span style='color:red'>";
         } else {
           detections_str = "<span style='color:orange'>";
         }
         detections_str += QString::number(file->GetPositives()) + " / "
             + QString::number(file->GetTotalScans()) + "</span>";

         detections_label = new QLabel(detections_str);
         detections_label->setTextFormat(Qt::RichText);
         ui->ScannerTableWidget_scan_table->setCellWidget(i, SCAN_TABLE_DETECTIONS_COL, detections_label);

         age_days = file->GetReportAgeDays();
         if (rescan_days  // rescan enabled
             && age_days > rescan_days  && req_per_minute_quota) {
           req_per_minute_quota--;
           file->ReScan();
         } else {
           file->SetState(kStopped);
         }
       } else {
         file->SetState(kWaitForReport);
       }
       /*
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_DETECTIONS_COL,
         new QTableWidgetItem(
           QString::number(file->GetPositives()) + " / " +
           QString::number(file->GetTotalScans())));
      */
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_MESSAGE_COL,
         new QTableWidgetItem(QString(file->GetVerboseMsg())) );


       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_DATE_COL,
         new QTableWidgetItem(file->GetScanDate().toString(Qt::DefaultLocaleShortDate)) );

      //         ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_LINK_COL,
         //new QTableWidgetItem( file->GetPermalink()));

      link_url = "<a href='" + file->GetPermalink()+ "'>" + file->GetPermalink() + "</a>";
      link_label= new QLabel(link_url);
      link_label->setTextFormat(Qt::RichText);
      link_label->setOpenExternalLinks(true);

      ui->ScannerTableWidget_scan_table->setCellWidget(i, SCAN_TABLE_LINK_COL, link_label);


       break;
      case KNoReportExists: // 5

       if (file->GetUploaded()) {
         // already uploaed  wait more
         qDebug() << "No Report..already uploaded,  wait more";
         file->SetState(kWaitForReport);
       } else {
         qDebug() << "No Report..Scan file quota=" << req_per_minute_quota
                  << " concurrent_uploads=" << concurrent_uploads;
         if (req_per_minute_quota && concurrent_uploads < 3) {
           req_per_minute_quota--;
           file->Scan();
           concurrent_uploads++;
         }
       }
       break;
      case kWaitForReport:


       // if we have quota
       if (state_change_msec < wait_delay) {
         ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL,
           new QTableWidgetItem("Wait "
           + QString::number((wait_delay -state_change_msec)/1000)));

       } else if (req_per_minute_quota > 0) {
         req_per_minute_quota--;
         file->CheckReport();
       }
       break;
      case kRescan:
        break;
      case kScan:
       ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL,
         new QTableWidgetItem("Uploading: "
            + QString::number(file->GetProgress(), 'f', 1) + "%") );
       break;
      case kErrorTooBig:
      case kErrorAccess:
       break;
      case kStopped:

      case kError:
       break;
       // do nothing
      default:
       qDebug() << "Undefined state" << file_state;
       break;
     }
  }
  ui->ScannerTableWidget_scan_table->resizeColumnsToContents();
  ui->ScannerTableWidget_scan_table->horizontalHeader()->setSectionResizeMode(
       SCAN_TABLE_FULLPATH_COL, QHeaderView::Stretch);
  ui->statusBar->showMessage("Quota Remaining:"
    + QString::number(req_per_minute_quota) + " Request(s)/Minute");
}

void MainWindow::StateTimerSlot(void)
{
  qDebug() << "StateTimerSlot";
  RunStateMachine();
  state_counter++;

  state_timer->start(400); // rearm

}


