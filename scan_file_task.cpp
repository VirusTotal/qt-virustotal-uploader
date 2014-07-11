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
#include <jansson.h>

#include "VtFile.h"
#include "VtResponse.h"
#include "qvtfile.h"
#include "vt-log.h"

#include "scan_file_task.h"


ScanFileTask::ScanFileTask(QVtFile *vt_file)
{
  file = vt_file;
}

#define RESP_BUF_SIZE 255

static void progress_update(struct VtFile *vtfile, void *qfile)
{
  QVtFile::ProgessUpdateCallback(vtfile, (QVtFile *) qfile);
}


//Scan for a file
void ScanFileTask::ScanSmallFile(void)
{
  int ret;
  struct VtFile *api = VtFile_new();
  struct VtResponse *response;
  char *str = NULL;
  char buf[RESP_BUF_SIZE+1] = { 0, };
  int response_code;


  VtFile_setApiKey(api, file->GetApiKey().toStdString().c_str());
//  VtFile_setProgressCallback(api, QVtFile::ProgessUpdateCallback, (void *)file);
  VtFile_setProgressCallback(api, progress_update, (void *)file);

  ret = VtFile_scan(api, file->fileName().toStdString().c_str(), NULL);

  if (ret) {
    qDebug() << "Error opening fetching report " << QString(file->GetSha256().toHex()) << "  " << ret;
     file->SetState(kWaitForReport);
    emit LogMsg(VT_LOG_ERR, ret, "Error fetching report");
  } else {
    response = VtFile_getResponse(api);
    str = VtResponse_toJSONstr(response, VT_JSON_FLAG_INDENT);
    if (str) {
      qDebug() << "Scan Response: " << str;
      free(str);
    }
    VtResponse_getVerboseMsg(response, buf, RESP_BUF_SIZE);
    qDebug() << "Buff: " << buf;
    file->SetVerboseMsg(buf);

    ret = VtResponse_getResponseCode(response, &response_code);
    if (!ret) {
      qDebug() << "scan response code: " <<  response_code;

    } else {
      emit LogMsg(VT_LOG_ERR, ret, "Error fetching scan response code");
      goto free_response;
    }

    if (response_code == 1) {
      file->SetScanDate(QDateTime::currentDateTime());
      file->SetState(kWaitForReport);
      file->SetUploaded(true);
    } else {
      // FIXME  what other codes?
       goto free_response;
    }

    str = VtResponse_getString(response, "scan_id");
    if (str) {
      file->SetScanId(str);
      free(str);
    }

    str = VtResponse_getString(response, "permalink");
    if (str) {
      file->SetPermalink(str);
      free(str);
    }





    file->SetState(kReportFeteched);

    free_response:
    VtResponse_put(&response);
  }
  VtFile_put(&api);
}


void ScanFileTask::run(void)
{
  qint64 fsize= file->size();

  qDebug() << "Scanning:  " << file->fileName() << " size " << fsize;
  if (fsize < 64*1024*1024) {
    ScanSmallFile();
    return;
  }
  file->SetState(kErrorTooBig);
  qDebug() << "FixMe large file";

}

