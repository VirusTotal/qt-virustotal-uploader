#include <QThreadPool>
#include "vtfile.h"
#include "calc_file_hashes_task.h"

VtFile::VtFile(QObject *parent) :
  QFile(parent)
{
  InitCommon();
}


VtFile::VtFile(const QString & name, QObject *parent) :
  QFile(name, parent)
{
  InitCommon();
}

VtFile::~VtFile()
{
}

void VtFile::InitCommon(void)
{
  vt_file_state = kNew;
}


enum VtFileState VtFile::GetState(void)
{
  return vt_file_state;
}

QString VtFile::GetStateStr(void)
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
void VtFile::SetState(enum VtFileState state)
{
  vt_file_state = state;
}

void VtFile::CalculateHashes(void)
{
  CalcFileHashesTask *hash_task = new CalcFileHashesTask(this);

  vt_file_state = kCheckingHash;

  // QThreadPool takes ownership and deletes 'hash_task' automatically
  QThreadPool::globalInstance()->start(hash_task);
}

void VtFile::SetMd5(const QByteArray &val)
{
  md5_hash = val;
}

void VtFile::SetSha1(const QByteArray &val)
{
  sha1_hash = val;
}

void VtFile::SetSha256(const QByteArray &val)
{
  sha256_hash = val;
}

QByteArray VtFile::GetMd5()
{
  return md5_hash;
}

QByteArray VtFile::GetSha1()
{
  return sha1_hash;
}

QByteArray VtFile::GetSha256()
{
  return sha256_hash;
}

