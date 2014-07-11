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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QButtonGroup>

#define DIR_ONLY_EXE_KEY "dir/only_exe"
#define RESCAN_DAYS_KEY "rescan_days"
#define API_KEY "api/key"
#define API_RATE_KEY "api/rate"


#define DEFAULT_RESCAN_DAYS 7

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = 0);
  ~SettingsDialog();

private:
  Ui::SettingsDialog *ui;
  QButtonGroup *rescan_group;
  QButtonGroup *dir_exe_group;

public slots:
  void SaveSettings(void);
  void CancelSettings(void);
};

#endif // SETTINGS_DIALOG_H
