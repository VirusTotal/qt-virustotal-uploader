#ifndef VTFILE_H
#define VTFILE_H

#include <QFile>
#include <QByteArray>
#include <QString>

enum VtFileState {
  kNew = 0,
  kCheckingHash,
  kHashesCalculated,
  kScan,
  kStopped,
  kErrorTooBig,
  kErrorAccess,
};

class VtFile : public QFile
{
  Q_OBJECT
  enum VtFileState vt_file_state;
  QByteArray md5_hash;
  QByteArray sha1_hash;
  QByteArray sha256_hash;
  void InitCommon(void);


public:
  VtFile(QObject *parent = 0);
  VtFile( const QString & name );
  VtFile(const QString & name, QObject *parent);
  ~VtFile();

  enum VtFileState GetState(void);
  QString GetStateStr(void);
  void SetState(enum VtFileState state);
  void CalculateHashes(void);
  void SetMd5(const QByteArray &val);
  void SetSha1(const QByteArray &val);
  void SetSha256(const QByteArray &val);

  QByteArray GetMd5();
  QByteArray GetSha1();
  QByteArray GetSha256();

signals:

public slots:

};

#endif // VTFILE_H
