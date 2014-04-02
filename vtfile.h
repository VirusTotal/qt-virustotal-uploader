#ifndef VTFILE_H
#define VTFILE_H

#include <QFile>
#include <QByteArray>
#include <QString>

enum QVtFileState {
  kNew = 0,
  kCheckingHash,
  kHashesCalculated,
  kCheckReport,
  kScan,
  kWaitForReport,
  kStopped,
  kErrorTooBig,
  kErrorAccess,
};

class QVtFile : public QFile
{
  Q_OBJECT
  enum QVtFileState vt_file_state;
  QByteArray md5_hash;
  QByteArray sha1_hash;
  QByteArray sha256_hash;
  void InitCommon(void);


public:
  QVtFile(QObject *parent = 0);
  QVtFile( const QString & name );
  QVtFile(const QString & name, QObject *parent);
  ~QVtFile();

  enum QVtFileState GetState(void);
  QString GetStateStr(void);
  void SetState(enum QVtFileState state);
  void CalculateHashes(void);
  void SetMd5(const QByteArray &val);
  void SetSha1(const QByteArray &val);
  void SetSha256(const QByteArray &val);

  QByteArray GetMd5();
  QByteArray GetSha1();
  QByteArray GetSha256();

  void CheckReport(void);

signals:

public slots:

};

#endif // VTFILE_H
