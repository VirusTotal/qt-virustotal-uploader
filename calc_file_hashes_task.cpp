#include <QDebug>
#include <QThread>
#include <QByteArray>
#include <QCryptographicHash>

#include "calc_file_hashes_task.h"

CalcFileHashesTask::CalcFileHashesTask(VtFile *vt_file)
{
  file = vt_file;
}


void CalcFileHashesTask::run()
{
  QByteArray buffer;
  const int buff_size = 65536;
  QCryptographicHash md5_hash(QCryptographicHash::Md5);
  QCryptographicHash sha1_hash(QCryptographicHash::Sha1);
  QCryptographicHash sha256_hash(QCryptographicHash::Sha256);
  qDebug() << "CalcFileHashesTask::run thread " << QThread::currentThread();

  buffer.resize(buff_size);

  if  (!file) {
    return;
  }

  if(!file->open(QIODevice::ReadOnly)) {
      // error opening
     qDebug() << "CalcFileHashesTask Error opening file " << file->error();
    file->SetState(kErrorAccess);
    return;
  }


  QCryptographicHash cryptoHash(QCryptographicHash::Md5);
  while(!file->atEnd()){
    buffer = file->read(buff_size);
    md5_hash.addData(buffer);
    sha1_hash.addData(buffer);
    sha256_hash.addData(buffer);
  }

  file->SetMd5(md5_hash.result());
  file->SetSha1(sha1_hash.result());
  file->SetSha256(sha256_hash.result());

  file->close();
  file->SetState(kHashesCalculated); // mark done
}
