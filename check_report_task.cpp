#include <QDebug>
#include <jansson.h>
#include "check_report_task.h"
#include "VtFile.h"
#include "VtResponse.h"
#include "qvtfile.h"
#include "vt-log.h"

CheckReportTask::CheckReportTask(QVtFile *vt_file)
{
  file = vt_file;
}

#define RESP_BUF_SIZE 255
void CheckReportTask::run(void)
{
  int ret;
  struct VtFile *api = VtFile_new();
  struct VtResponse *response;
  char *str = NULL;
  char buf[RESP_BUF_SIZE+1] = { 0, };
  int response_code;
  int total;
  int positives;

//  connect(this, SIGNAL(LogMsg(int,int,QString)),file, SLOT(RelayLogMsg(int,int,QString)));
  VtFile_setApiKey(api, file->GetApiKey().toStdString().c_str());

  ret = VtFile_report(api, QString(file->GetSha256().toHex()).toStdString().c_str());

  if (ret) {
    qDebug() << "Error opening fetching report " << QString(file->GetSha256().toHex()) << "  " << ret;
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
        file->SetState(KNoReportExists);
        goto free_response;
      }
    }

    ret = VtResponse_getIntValue(response, "total", &total);
    if (ret) {
      emit LogMsg(VT_LOG_ERR, ret, "Error finding 'total' in report");
      goto free_response;
    }
    qDebug() << " total " << total;

    ret = VtResponse_getIntValue(response, "positives", &positives);
    if (ret) {
      emit LogMsg(VT_LOG_ERR, ret, "Error finding 'positives' in report");
      goto free_response;
    }
    qDebug() << "positives: " << positives;
    file->SetDetections(positives, total);

    str = VtResponse_getString(response, "scan_date");
    if (str) {
      file->SetScanDate(str);
      free(str);
    }

    str = VtResponse_getString(response, "permalink");
    if (str) {
      file->SetPermalink(str);
      free(str);
    }
    str = VtResponse_getString(response, "scan_id");
    if (str) {

      // if no scan ID set
      if (file->GetScanId().length() < 5) {
        file->SetScanId(str);
      }
      if (file->GetScanId() == str) {
        qDebug() << "Current report fetched";
        file->SetState(kReportFeteched);
      } else {
        qDebug() << "Wait for updated report";
        file->SetState(kWaitForReport);
      }
      free(str);
    }

    free_response:
    VtResponse_put(&response);
  }
  VtFile_put(&api);
}
