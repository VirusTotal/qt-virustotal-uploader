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
#include "rescan_file_task.h"
#include "VtFile.h"
#include "VtResponse.h"
#include "qvtfile.h"
#include "vt-log.h"

RescanFileTask::RescanFileTask(QVtFile *f)
{
  file = f;
}


#define RESP_BUF_SIZE 255
void RescanFileTask::run(void)
{
  int ret;
  struct VtFile *api = VtFile_new();
  struct VtResponse *response;
  char *str = NULL;
  char buf[RESP_BUF_SIZE+1] = { 0, };
  int response_code;


//  connect(this, SIGNAL(LogMsg(int,int,QString)),file, SLOT(RelayLogMsg(int,int,QString)));
  VtFile_setApiKey(api, file->GetApiKey().toStdString().c_str());

  ret = VtFile_rescanHash(api,
      QString(file->GetSha256().toHex()).toStdString().c_str(),
      0, 0, 0, NULL, false);

  if (ret) {
     qDebug() << "Error opening fetching report " << QString(file->GetSha256().toHex()) << "  " << ret;
     file->SetState(kWaitForReport);
    emit LogMsg(VT_LOG_ERR, ret, "Error fetching report");
  } else {
    response = VtFile_getResponse(api);
    str = VtResponse_toJSONstr(response, VT_JSON_FLAG_INDENT);
    if (str) {
      qDebug() << "report Response: " << str;
      free(str);
    }
    VtResponse_getVerboseMsg(response, buf, RESP_BUF_SIZE);
    qDebug() << "Buff: " << buf;
    file->SetVerboseMsg(buf);

    ret = VtResponse_getResponseCode(response, &response_code);
    if (!ret) {
      qDebug() << "report response code: " <<  response_code;
      if (response_code == 0) {
        file->SetState(kError);
        goto free_response;
      }
    }

    str = VtResponse_getString(response, "permalink");
    if (str) {
      file->SetPermalink(str);
      free(str);
    }

    str = VtResponse_getString(response, "scan_id");
    if (str) {
      file->SetScanId(str);
      free(str);
    }

    file->SetState(kWaitForReport);

    free_response:
    VtResponse_put(&response);
  }
  VtFile_put(&api);
}
