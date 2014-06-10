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

#ifndef VTFILE_H
#define VTFILE_H

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QTemporaryDir>

#include "zip.h"

enum QVtFileState {
  kNew = 0,
  kCheckingHash,
  kHashesCalculated,
  kCheckReport,
  kCreateAppZip,
  kReportFeteched,
  KNoReportExists,
  kRescan,
  kScan,
  kWaitForReport, // scan queued sent need to wait
  kStopped,
  kErrorTooBig,
  kErrorAccess,
  kError, // generic error occured
};

class QVtFile : public QFile
{
  Q_OBJECT

private:
  enum QVtFileState vt_file_state;
  QByteArray md5_hash;
  QByteArray sha1_hash;
  QByteArray sha256_hash;
  QString verbose_msg;
  QDateTime scan_date;
  QDateTime last_state_change;
  QString permalink;
  QString scan_id;
  bool file_uploaded;
  int positive_scans;
  int total_scans;
  float progress;

  bool is_bundle;
  int num_files_in_bundle;
  QTemporaryDir *bundle_tmp_dir;
  QString bundle_path;
  void InitCommon(void);
  void CreateBundleZip();



public:
  QVtFile(QObject *parent = 0);
  QVtFile( const QString & name );
  QVtFile(const QString & name, QObject *parent);
  ~QVtFile();

  enum QVtFileState GetState(void);
  QString GetStateStr(void);
  void SetState(enum QVtFileState state);

  void SetMd5(const QByteArray &val);
  void SetSha1(const QByteArray &val);
  void SetSha256(const QByteArray &val);
  void SetVerboseMsg(QString Msg);
  void SetDetections(int positive, int total);
  void SetScanDate(QString date_time);
  void SetScanDate(QDateTime d);
  void SetPermalink(QString link);
  void SetScanId(QString id);
  void SetUploaded(bool val=true);
  void SetBundlePath(QString);
  void SetProgress(float value);


  QByteArray GetMd5();
  QByteArray GetSha1();
  QByteArray GetSha256();
  QString GetApiKey();
  QString GetBundlePath();

  int GetPositives();
  int GetTotalScans();
  QString GetVerboseMsg();
  QDateTime GetScanDate();
  int GetReportAgeDays();
  QString GetPermalink();
  QString GetScanId();
  QDateTime GetStateChangeTime();
  bool GetUploaded();
  float GetProgress();

  void CalculateHashes(void);
  void CheckReport(void);
  void Scan();
  void ReScan();
  static void CancelOperations();

  // Callback from C library
  static void ProgessUpdateCallback(struct VtFile *vtfile, QVtFile *qfile);

signals:
  void LogMsg(int log_level, int err_code, QString Msg);

public slots:
  void RelayLogMsg(int log_level, int err_code, QString Msg);

};


#endif // VTFILE_H
