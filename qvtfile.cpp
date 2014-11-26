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

#include <QObject>
#include <QThreadPool>
#include <QTime>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QTemporaryDir>
#include "qvtfile.h"
#include "calc_file_hashes_task.h"
#include "check_report_task.h"
#include "scan_file_task.h"
#include "rescan_file_task.h"
#include "create_app_zip_task.h"
#include "vt-log.h"
#include "VtFile.h"



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
  if (is_bundle)
    this->remove();  // delete and cleanup ourself

  if (bundle_tmp_dir)
    delete (bundle_tmp_dir);
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
  is_bundle = false;
  bundle_tmp_dir = NULL;
  bundle_path = "";
  progress = 0.0;
  scan_date = QDateTime::currentDateTime();
}


enum QVtFileState QVtFile::GetState(void)
{
  return vt_file_state;
}

void QVtFile::CreateBundleZip()
{
  bundle_tmp_dir = new QTemporaryDir();
  QString zip_name;
  QFileInfo finfo(bundle_path);
  CreateAppZipTask *task = new CreateAppZipTask(this);

  if (bundle_tmp_dir->isValid()) {
    if (finfo.bundleName().length() > 1)
      zip_name =  bundle_tmp_dir->path() + "/" + finfo.bundleName() + ".app.zip";
    else {
      int last_slash = finfo.filePath().lastIndexOf("/");
      qDebug() << "last_slash = " << last_slash;
      qDebug() << "path = " << finfo.filePath();
      if (last_slash > 0) {
         zip_name = bundle_tmp_dir->path() + "/" + finfo.filePath().mid(last_slash) + ".zip";
         qDebug() << "CreateBundleZip zip_name: " <<  zip_name;
      } else {
         zip_name = bundle_tmp_dir->path() + "/OSXApplication.app.zip";
      }
    }

  } else {
    zip_name = QDir::tempPath() + "/" + finfo.bundleName() + ".app.zip";
    emit LogMsg(VT_LOG_DEBUG, 0 , "Tmpdir not valid  using  " + zip_name);
  }

  this->setFileName(zip_name);

  QObject::connect(task, SIGNAL(LogMsg(int,int,QString)),
      this, SLOT(RelayLogMsg(int,int,QString)),  Qt::QueuedConnection);

  QThreadPool::globalInstance()->start(task);

}

void QVtFile::SetBundlePath(QString path)
{
  bundle_path = path;
  is_bundle = true;
  qDebug() << "QVtFile::SetBundlePath " << path;


  SetState(kCreateAppZip);

  CreateBundleZip();

}
QString QVtFile::GetBundlePath()
{
  return bundle_path;
}

QString QVtFile::GetApiKey(void)
{
  QSettings settings;
  QString key;

  // build string avoiding someone dumping the whole thing with strings
  QString default_key = "";
  default_key += "70f8a";
  default_key += "7ffa6";
  default_key += "68547";
  default_key += "1afc8";
  default_key += "5d9202";
  default_key += "468f72";
  default_key += "77ae";
  default_key += "20fa7f7";
  default_key += "816";
  default_key += "3aa22b4c8822f18a5a";

  key = settings.value("api/key", default_key).toString();

  if (key.length() < 8) {
    key = default_key;
  }

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
    case kCreateAppZip:
      return tr("Zipping App");
    case kReportFeteched:
      return tr("Report Fetched");
    case KNoReportExists:
      return tr("No Report");
    case kRescan:
      return tr("Rescan");
    case kScan:
      return tr("Uploading");
    case kWaitForReport:
        return tr("Waiting");
    case kErrorTooBig:
      return tr("Too big");
    case kErrorAccess:
      return tr("Access Denied");
    case kStopped:
      return tr("Done");
    case kError:
      return tr("Error");
    default:
     return  QString("QVtFile::GetStateStr Undefined state: ") + QString::number(vt_file_state) + " "+ this->fileName();
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
int QVtFile::GetReportAgeDays()
{
  QDateTime now = QDateTime::currentDateTime();
  qint64 age;

  age = scan_date.msecsTo(now);
  age /= 1000; // seconds
  age /= 86400; // days
  return age;
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

void QVtFile::SetProgress(float val)
{
  progress = val;
}

float QVtFile::GetProgress(void)
{
  return progress;
}

static bool cancel_operations = false;
void QVtFile::CancelOperations(void)
{
  cancel_operations = true;
}

void QVtFile::ProgessUpdateCallback(struct VtFile *vtfile, QVtFile *qfile)
{
  int64_t dltotal = 0;
  int64_t dlnow = 0;
  int64_t ul_total = 0;
  int64_t ul_now = 0;
  float progress = 0.0;
  qDebug() << "ProgessUpdateCallback";
  VtFile_getProgress(vtfile, &dltotal, &dlnow, &ul_total, &ul_now);

  // avoid div by 0
  if (ul_total > 0 && qfile->size() > 0) {
    //progress = (ul_now / ul_total) * 100.0;
    progress =  ( (float) ul_now / (float) qfile->size()) * 100.0;
  }

  // fix rounding so a user won´t see 100.1.  curl wil include the headers in what was uploaded
  if (progress > 100.0)
    progress = 100.0;

  qDebug() << "ProgessUpdateCallback ul_now= " << ul_now
      << " ul_total=" << ul_total
      << " progress= " << progress;
  qfile->SetProgress(progress);

  if (cancel_operations) {
    VtFile_cancelOperation(vtfile);
  }

}


