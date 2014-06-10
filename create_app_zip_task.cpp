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

#include <QDebug>
#include "create_app_zip_task.h"
#include "vt-log.h"
#include "zlib.h"
#include "zip.h"

#ifdef _WIN32
#elif defined(unix) || defined(__APPLE__)
#include <sys/stat.h>
#endif

#define MAXFILENAME (512)

CreateAppZipTask::CreateAppZipTask(QVtFile *f)
{
  vtfile = f;
  total_size  = 0;
  processed_size = 0;
}


void CreateAppZipTask::OpenZip(QString filename)
{
    #ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    zf = zipOpen2_64(filename.toStdString().c_str(),0,NULL,&ffunc);
    #else
    zf = zipOpen64(filename.toStdString().c_str(), 0);
    #endif

}


void CreateAppZipTask::SumDir(QString path, unsigned int depth)
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
    SumSizePath(file_info, depth);

  }
}


#ifdef _WIN32
uLong filetime(f, tmzip, dt)
    char *f;                /* name of file to get info on */
    tm_zip *tmzip;             /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#elif defined(unix) || defined(__APPLE__)
uLong filetime(char *f, tm_zip *tmzip, uLong *dt)
            /* name of file to get info on */
       /* return value: access, modific. and creation times */
      /* dostime */
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;
  *dt = 0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(char *f, tm_zip *tmzip, uLong dt)
                 /* name of file to get info on */
            /* return value: access, modific. and creation times */
      /* dostime */
{
    return 0;
}
#endif


#define BUF_SIZE 65536
void CreateAppZipTask::AddZipPath(QFileInfo file_info, unsigned int depth)
{
	// Add file to zip
  FILE * fin = NULL;
  int size_read = 0;
  QString file_path = "";
  unsigned char buf[BUF_SIZE];
  const char *savefilenameinzip;
  zip_fileinfo zi;
  unsigned long crcFile=0;
  int zip64 = 0;
  int err = 0;
  qDebug() << "file_info =" << file_info.path();

  file_path = file_info.canonicalFilePath();
  // If the file does not exist, canonicalFilePath() returns an empty string.
  if (file_path.isEmpty())
    return;

  if (file_info.isDir()) {
    AddZipDir(file_path, depth+1);
    return;
  }

  char* filenameinzip = strdup(file_path.toStdString().c_str());
  
  zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
  zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
  zi.dosDate = 0;
  zi.internal_fa = 0;
  zi.external_fa = 0;

  filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

  if ( (savefilenameinzip = strstr(filenameinzip, ".app/")) ) {
    // find the Directory of the .app in the path
    while (savefilenameinzip > filenameinzip && *savefilenameinzip != '/')
      savefilenameinzip--;

  } else {
    savefilenameinzip = filenameinzip;
  }

  /* The path name saved, should not include a leading slash. */
  /*if it did, windows/xp and dynazip couldn't read the zip file. */

//  printf("filenameinzip=%s\n", filenameinzip);
  while( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
  {
     savefilenameinzip++;
  }

 // qDebug() << "Add file1 " << file_path;
 // emit LogMsg(VT_LOG_DEBUG, 0 , "Add file " + QString(savefilenameinzip));
 // printf("savefilenameinzip=%s\n", savefilenameinzip);


   /**/
  err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                   NULL,0,NULL,0,NULL /* comment*/,
                    Z_DEFLATED,
                   9, // compress level
                   0,
                   /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                   NULL,crcFile, zip64);

  if (err != ZIP_OK)
      emit LogMsg(VT_LOG_ERR, 0 , "error opening " + QString(filenameinzip) + " in zipfile");
  else
  {
      fin = fopen(filenameinzip,"r");
      if (fin==NULL)
      {
          err=ZIP_ERRNO;
          printf("error in opening %s for reading\n",filenameinzip);
      }
  }

  if (err == ZIP_OK) {
    do
    {
      err = ZIP_OK;
      size_read = (int)fread(buf,1,BUF_SIZE,fin);
      if (size_read < BUF_SIZE)
          if (feof(fin)==0)
      {
          printf("error in reading %s\n",filenameinzip);
          err = ZIP_ERRNO;
      }

      if (size_read>0)
      {


          err = zipWriteInFileInZip (zf,buf,size_read);
          if (err<0)
          {
              printf("error in writing %s in the zipfile\n",
                               filenameinzip);
          }
          processed_size += size_read;

      }
    } while ((err == ZIP_OK) && (size_read>0));
  }

  if (fin)
      fclose(fin);

  if (err<0)
      err=ZIP_ERRNO;
  else
  {
      err = zipCloseFileInZip(zf);
      if (err!=ZIP_OK)
          printf("error in closing %s in the zipfile\n",
                      filenameinzip);
  }
  free(filenameinzip);
}


void CreateAppZipTask::AddZipDir(QString path, unsigned int depth)
{
  QDir dir(path);
  QFileInfoList file_info_list;
  int length;


  if (depth >10)
    return;  // avoid too much recursion

  file_info_list = dir.entryInfoList(QDir::AllDirs
  | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable);
  length = file_info_list.length();
  
  // 1st go through and get all dirs
  for (int i = 0; i < length; i++) {
    QFileInfo file_info = file_info_list[i];
    QString file_path = file_info.canonicalFilePath();
    if (file_path.isEmpty())
      continue;
    
    if (file_info.isDir()) {
      AddZipDir(file_path, depth+1);
    }
  }

  
  for (int i = 0; i < length; i++) {
    QFileInfo file_info = file_info_list[i];
    QString file_path = file_info.canonicalFilePath();

    if (file_info.isDir()) {
      continue;
    }
    
    // If the file does not exist, canonicalFilePath() returns an empty string.
    if (file_path.isEmpty())
      continue;

    qDebug() <<  path << " file "<< i << "/" << length << " = " << file_path
    << " depth= " << depth  << " total_size= " << total_size;
    AddZipPath(file_info, depth);
    vtfile->SetProgress( ((float) i / (float) length) * 100.0);

  }
}

void CreateAppZipTask::SumSizePath(QFileInfo file_info, unsigned int depth)
{
    QString file_path = file_info.canonicalFilePath();

    // If the file does not exist, canonicalFilePath() returns an empty string.
    if (file_path.isEmpty())
      return;

    if (file_info.isDir()) {
      SumDir(file_path, depth+1);
    } else
      total_size += file_info.size();
}

void CreateAppZipTask::run(void)
{
  SumSizePath(vtfile->GetBundlePath(), 0);
  OpenZip(vtfile->fileName());
  AddZipPath(vtfile->GetBundlePath(), 0);

  zipClose(zf,NULL);
  vtfile->SetState(kNew);
}
