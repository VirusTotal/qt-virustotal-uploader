#ifndef VTFILE_H
#define VTFILE_H

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QDateTime>

enum QVtFileState {
  kNew = 0,
  kCheckingHash,
  kHashesCalculated,
  kCheckReport,
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
  int positive_scans;
  int total_scans;
  void InitCommon(void);


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

  QByteArray GetMd5();
  QByteArray GetSha1();
  QByteArray GetSha256();
  QString GetApiKey();

  int GetPositives();
  int GetTotalScans();
  QString GetVerboseMsg();
  QDateTime GetScanDate();
  QString GetPermalink();
  QString GetScanId();
  QDateTime GetStateChangeTime();

  void CalculateHashes(void);
  void CheckReport(void);
  void Scan();
  void ReScan();

signals:
  void LogMsg(int log_level, int err_code, QString Msg);

public slots:
  void RelayLogMsg(int log_level, int err_code, QString Msg);

};

#endif // VTFILE_H
