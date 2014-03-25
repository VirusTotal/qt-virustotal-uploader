#include <QMessageBox>
#include <QTime>
#include <QDebug>
#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scanner_table_widget.h"
#include "vt-log.h"

#define LOG_TIME_COL 0
#define LOG_CODE_COL 1
#define LOG_LEVEL_COL 2
#define LOG_MSG_COL 3
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
}

MainWindow::~MainWindow()
{
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
}

void MainWindow::MinuteTimerSlot(void)
{
  qDebug() << "MinuteTimerSlot";
  QSettings settings;
  int minute_quota = settings.value("quota/perMinute", 4).toInt();

  qDebug() << "Settings quota = " << minute_quota;
  req_per_minute_quota = minute_quota;
}

void MainWindow::StateTimerSlot(void)
{
  qDebug() << "MinuteTimerSlot";
}

