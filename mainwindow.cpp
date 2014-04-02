#include <QMessageBox>
#include <QTime>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QThreadPool>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scanner_table_widget.h"
#include "vt-log.h"
#include "vtfile.h"

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


#define VT_UPLOADER_VERSION "0.1"

#define MAX_LOG_MSG_LINES 1000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  qDebug() << "Startup";
  ui->setupUi(this);
  setAcceptDrops(true);
  req_per_minute_quota = 0;

  QCoreApplication::setOrganizationName("VirusTotal");
  QCoreApplication::setOrganizationDomain("virustotal.com");
  QCoreApplication::setApplicationName("Q Uploader");


  emit __onLogMsgRecv(VT_LOG_DEBUG, 0 ,
      tr("VirusTotal Uplaoder") + " " + VT_UPLOADER_VERSION + " "
      + tr("Compiled:") + __DATE__ + " " + __TIME__);

  // Drag drop signals
  connect(ui->ScannerTableWidget_scan_table, SIGNAL(dropped(const QMimeData*)), this, SLOT(OnDropRecv(const QMimeData*)));
  connect(this, SIGNAL(dropped(const QMimeData*)), this, SLOT(OnDropRecv(const QMimeData*)));

  minute_timer = new QTimer(this);
  connect(minute_timer, SIGNAL(timeout()), this, SLOT(MinuteTimerSlot()));
  emit MinuteTimerSlot(); // initalize
  minute_timer->start(60000);

  state_timer = new QTimer(this);
  connect(state_timer, SIGNAL(timeout()), this, SLOT(StateTimerSlot()));
  state_timer->start(3000);
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


void MainWindow::__onLogMsgRecv(int log_level, int err_code, QString Msg)
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
      Level_Cell_Item->setText(QString("Unkown %1").arg(log_level));
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

void MainWindow::OnDropRecv(const QMimeData *mime_data)
{

  emit __onLogMsgRecv(VT_LOG_DEBUG, 0 , tr("Dropped ") + mime_data->text());
  QString file_path;
  int file_uri_pos = mime_data->text().indexOf("file://");

  if (file_uri_pos == -1) {
    // not found
    return;
  }
  file_path = mime_data->text().mid(strlen("file://"));

  file_vector.append(new VtFile(file_path, this));

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

void MainWindow::ReDrawScannerTable(void)
{
  QTableWidgetItem *Path_Cell_Item;
  int num_files = file_vector.count();

  ui->ScannerTableWidget_scan_table->clear();
  ui->ScannerTableWidget_scan_table ->setRowCount(num_files+1);


  qDebug() << "ReDraw..  Seperator:" << QDir::separator();

  for (int i = 0; i < num_files ; i++) {

    VtFile *file = file_vector.operator[](i);
    int last_slash = file->fileName().lastIndexOf(QDir::separator());


    // if not found it will be -1
    if (last_slash < 0)
      last_slash = 0;
    else
      last_slash+=1; // move off slash


//    ui->ScannerTableWidget_scan_table->insertRow(0);
    QString short_name = file->fileName().mid(last_slash);

    qDebug() << "Last_slash=" << last_slash  << " Short name: " << short_name;
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

  num_files = file_vector.count();

  for (int i = 0; i < num_files ; i++) {
    VtFile *file = file_vector.operator[](i);
    QString state_str = file->GetStateStr();
    enum VtFileState file_state = file->GetState();
    ui->ScannerTableWidget_scan_table->setItem(i, SCAN_TABLE_STATUS_COL, new QTableWidgetItem(state_str) );
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

         break;
       case kScan:
       case kErrorTooBig:
       case kErrorAccess:
       case kStopped:
         break;
         // do nothing
       default:
         qDebug() << "Undefined state" << file_state;
         break;
     }
  }
  ui->ScannerTableWidget_scan_table->resizeColumnsToContents();

}

void MainWindow::StateTimerSlot(void)
{
 qDebug() << "StateTimerSlot";
 RunStateMachine();

}

