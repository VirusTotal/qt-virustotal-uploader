#include <QThreadPool>
#include "vtfile.h"
#include "calc_file_hashes_task.h"

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
  vt_file_state = kNew;
}


enum QVtFileState QVtFile::GetState(void)
{
  return vt_file_state;
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
    case kScan:
      return tr("Scanning");
    case kErrorTooBig:
      return tr("Too big");
    case kErrorAccess:
      return tr("Access Denied");
    case kStopped:
      return tr("Idle");
    default:
     return  QString("Undefined state") + QString::number(vt_file_state);
  }
}
void QVtFile::SetState(enum QVtFileState state)
{
  vt_file_state = state;
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

void QVtFile::CheckReport(void)
{

}

