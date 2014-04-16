#include <QObject>
#include <QThreadPool>
#include <QTime>
#include <QDebug>
#include <QSettings>
#include "qvtfile.h"
#include "calc_file_hashes_task.h"
#include "check_report_task.h"
#include "scan_file_task.h"
#include "rescan_file_task.h"
#include "vt-log.h"


QVtFile::QVtFile(QObject *parent) :
  QFile(parent)
{
  InitCommon();
}


QVtFile::QVtFile(const QString & name, QObject *parent) :
  QFile(name, parent)
{
  InitCommon();
}

QVtFile::~QVtFile()
{
}

void QVtFile::InitCommon(void)
{
  SetState(kNew);
  scan_id = "";
  verbose_msg = "";
  permalink = "";
  file_uploaded = false;
  positive_scans = 0;
  total_scans = 0;
}


enum QVtFileState QVtFile::GetState(void)
{
  return vt_file_state;
}


QString QVtFile::GetApiKey(void)
{
  QSettings settings;
  QString key;
  const char *default_key = "6d7880429b29ae9d1450f7c82cae7e1c18a630f44b584f8ad1df67659cc2a194";
  key = settings.value("api/key", default_key).toString();

  if (key.length() < 8)
    key = default_key;

  return key;
}

QString QVtFile::GetStateStr(void)
{
  switch (vt_file_state)
  {
    case kNew:
      return tr("New");
    case kCheckingHash:
       return tr("Checking Hash");
    case kHashesCalculated:
      return tr("Hashes Calculated");
    case kCheckReport:
      return tr("Checking Report");
    case kReportFeteched:
      return tr("Report Fetched");
    case KNoReportExists:
      return tr("No Report");
    case kRescan:
      return tr("Rescan");
    case kScan:
      return tr("Scanning");
    case kWaitForReport:
        return tr("Waiting");
    case kErrorTooBig:
      return tr("Too big");
    case kErrorAccess:
      return tr("Access Denied");
    case kStopped:
      return tr("Idle");
    case kError:
      return tr("Error");
    default:
     return  QString("QVtFile::GetStateStr Undefined state") + QString::number(vt_file_state);
  }
}

void QVtFile::SetState(enum QVtFileState state)
{
  qDebug() << "SetState OldSate " << vt_file_state << " " << GetStateStr();
  vt_file_state = state;
  last_state_change = QDateTime::currentDateTime();
  qDebug() << "SetState NewSate " << vt_file_state << " " << GetStateStr();

}

void QVtFile::CalculateHashes(void)
{
  CalcFileHashesTask *hash_task = new CalcFileHashesTask(this);

  vt_file_state = kCheckingHash;

  // QThreadPool takes ownership and deletes 'hash_task' automatically
  QThreadPool::globalInstance()->start(hash_task);
}

void QVtFile::SetMd5(const QByteArray &val)
{
  md5_hash = val;
}

void QVtFile::SetSha1(const QByteArray &val)
{
  sha1_hash = val;
}

void QVtFile::SetSha256(const QByteArray &val)
{
  sha256_hash = val;
}

void QVtFile::SetVerboseMsg(QString msg)
{
  verbose_msg = msg;
}

void QVtFile::SetDetections(int pos, int total)
{
  positive_scans = pos;
  total_scans = total;
}

void QVtFile::SetScanDate(QString date_str)
{
  qDebug() << "SetScanDate (string)  " << date_str;
  scan_date = QDateTime::fromString(date_str, "yyyy-MM-dd HH:mm:ss"); // "2013-09-13 09:39:08"
  qDebug() << "SetScanDate (string) " << scan_date.toString(Qt::DefaultLocaleShortDate);
}

void QVtFile::SetScanDate(QDateTime d)
{
  scan_date = d;
  qDebug() << "SetScanDate DatTime:" << scan_date.toString(Qt::DefaultLocaleShortDate);
}

void QVtFile::SetPermalink(QString link)
{
  permalink = link;
}

void QVtFile::SetScanId(QString id)
{
  scan_id = id;
}
void QVtFile::SetUploaded(bool val)
{
  file_uploaded = val;
}

QByteArray QVtFile::GetMd5()
{
  return md5_hash;
}

QByteArray QVtFile::GetSha1()
{
  return sha1_hash;
}

QByteArray QVtFile::GetSha256()
{
  return sha256_hash;
}

int QVtFile::GetPositives()
{
  return positive_scans;
}

int QVtFile::GetTotalScans()
{
  return total_scans;
}

QString QVtFile::GetVerboseMsg()
{
  return verbose_msg;
}

QDateTime QVtFile::GetScanDate()
{
  return scan_date;
}

QString QVtFile::GetPermalink()
{
  return permalink;
}

QString QVtFile::GetScanId()
{
  return scan_id;
}

bool QVtFile::GetUploaded()
{
  return file_uploaded;
}

QDateTime QVtFile::GetStateChangeTime()
{
  return last_state_change;
}

void QVtFile::RelayLogMsg(int log_level, int err_code, QString Msg)
{
  qDebug() << QTime::currentTime() << "Relay Relay_Log_Msg=" << Msg;

  emit LogMsg(log_level, err_code, Msg);
  qDebug() << QTime::currentTime() << "Relay Relay_Log_Msg= "  << log_level << " code" << err_code;
}

void QVtFile::CheckReport(void)
{
  CheckReportTask *task = new CheckReportTask(this);

  SetState(kCheckReport);
  emit LogMsg(VT_LOG_DEBUG, 0 , "CheckReport: " + this->fileName());
  QObject::connect(task, SIGNAL(LogMsg(int,int,QString)),this, SLOT(RelayLogMsg(int,int,QString)),  Qt::QueuedConnection);

  // QThreadPool takes ownership and deletes 'task' automatically
  QThreadPool::globalInstance()->start(task);
}

void QVtFile::Scan(void)
{
  ScanFileTask *task = new ScanFileTask(this);

  SetState(kScan);
  emit LogMsg(VT_LOG_DEBUG, 0 , "Scan file: " + this->fileName());
  QObject::connect(task, SIGNAL(LogMsg(int,int,QString)),this, SLOT(RelayLogMsg(int,int,QString)),  Qt::QueuedConnection);

  // QThreadPool takes ownership and deletes 'task' automatically
  QThreadPool::globalInstance()->start(task);
}

void QVtFile::ReScan(void)
{
  RescanFileTask *task = new RescanFileTask(this);

  SetState(kRescan);
  emit LogMsg(VT_LOG_DEBUG, 0 , "Scan file: " + this->fileName());
  QObject::connect(task, SIGNAL(LogMsg(int,int,QString)),this, SLOT(RelayLogMsg(int,int,QString)),  Qt::QueuedConnection);

  // QThreadPool takes ownership and deletes 'task' automatically
  QThreadPool::globalInstance()->start(task);
}

