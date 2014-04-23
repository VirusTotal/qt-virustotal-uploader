#include <QDebug>
#include <QDir>
#include <QMimeDatabase>
#include <QSettings>

#include "add_dir_task.h"
#include "vt-log.h"

AddDirTask::AddDirTask(QString path)
{
  QSettings settings;
  dir_path = path;

  only_executables = settings.value("only_executables", true).toBool();
}

void AddDirTask::ScanDir(QString path, unsigned int depth)
{
  QDir dir(path);
  QFileInfoList file_info_list;
  int length;


  if (depth > 16)
    return;  // avoid too much recursion

  file_info_list = dir.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
  length = file_info_list.length();
  for (int i = 0; i < length; i++) {
    QFileInfo file_info = file_info_list[i];
    QString file_path = file_info.canonicalFilePath();

    // If the file does not exist, canonicalFilePath() returns an empty string.
    if (file_path.isEmpty())
      continue;

    qDebug() <<  path << " file "<< i << "/" << length << " = " << file_path;
    ScanPath(file_info, depth);

  }
}

void AddDirTask::AddAppBundle(QString path)
{
  emit LogMsg(VT_LOG_NOTICE, 0, "FixME OSX Bundle: " + path);
}

void AddDirTask::ScanPath(QFileInfo file_info, unsigned int depth)
{
  QString file_path = file_info.canonicalFilePath();
  QMimeDatabase mime_db;

  // If the file does not exist, canonicalFilePath() returns an empty string.
  if (file_path.isEmpty())
    return;

  if (file_info.isBundle()) {
    qDebug() <<  "OSX app bunndle " << file_info.path();
    AddAppBundle(file_path);
    return;
  } else if (file_info.isDir()) {
    ScanDir(file_path, depth+1);
  } else if (file_info.isFile()) {
    QMimeType mime_type = mime_db.mimeTypeForFile(file_info);
    qDebug() << file_path << " mime type: " << mime_type.name();

    if (!only_executables) {
      emit AddFile(file_path);
    } else {
      if (mime_type.name().contains("application/") || file_info.isExecutable())
         emit AddFile(file_path);
    }
  } else {
    emit LogMsg(VT_LOG_NOTICE, 0, "Unknown file type: " + file_path);
  }
}

void AddDirTask::ScanPath(QString path, unsigned int depth)
{
  ScanPath(QFileInfo(path), depth);
}

void AddDirTask::run()
{
  qDebug() << "Path: " << dir_path;
  ScanPath(dir_path, 0);
}

