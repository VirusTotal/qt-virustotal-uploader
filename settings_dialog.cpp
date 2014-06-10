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
#include <QSettings>

#include "settings_dialog.h"
#include "ui_settings_dialog.h"


SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingsDialog)
{
  QSettings settings;
  int rescan_days;
  bool only_exec;
  ui->setupUi(this);
  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(SaveSettings()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(CancelSettings()));

  rescan_group = new QButtonGroup(this);
  dir_exe_group = new QButtonGroup(this);

  rescan_group->addButton(ui->rescan_0_radioButton);
  rescan_group->addButton(ui->rescan_7_radioButton);
  rescan_group->addButton(ui->rescan_30_radioButton);

  rescan_days = settings.value(RESCAN_DAYS_KEY, DEFAULT_RESCAN_DAYS).toInt();
  switch (rescan_days) {
    case 0:
      ui->rescan_0_radioButton->setChecked(true);
      break;
    case 30:
      ui->rescan_30_radioButton->setChecked(true);
      break;
    default:
    case 7:
      ui->rescan_7_radioButton->setChecked(true);
      break;
  }

  dir_exe_group->addButton(ui->dir_all_files_radioButton);
  dir_exe_group->addButton(ui->dir_only_exe_radioButton);
  only_exec = settings.value(DIR_ONLY_EXE_KEY,  true).toBool();
  if (only_exec) {
    ui->dir_only_exe_radioButton->setChecked(true);
  } else {
    ui->dir_all_files_radioButton->setChecked(true);
  }

  ui->max_req_rate_spinBox->setValue(settings.value(API_RATE_KEY, 4).toInt());
  ui->apikey_lineEdit->setText(settings.value(API_KEY, "").toString());
}

SettingsDialog::~SettingsDialog()
{
  delete dir_exe_group;
  delete rescan_group;
  delete ui;
}



void SettingsDialog::SaveSettings(void)
{
  bool only_exec = true;
  int rescan_days = DEFAULT_RESCAN_DAYS;
  QSettings settings;

  qDebug() << "Save Settings";
  if (ui->dir_all_files_radioButton->isChecked())
  {
    only_exec = false;
  }

  settings.setValue(DIR_ONLY_EXE_KEY, only_exec);

  if (ui->rescan_0_radioButton->isChecked())
    rescan_days = 0;
  else if (ui->rescan_7_radioButton->isChecked())
    rescan_days = 7;
  else if (ui->rescan_30_radioButton->isChecked())
    rescan_days = 30;

  settings.setValue(RESCAN_DAYS_KEY, rescan_days);

  settings.setValue(API_KEY, ui->apikey_lineEdit->text());
  settings.setValue(API_RATE_KEY, ui->max_req_rate_spinBox->value());

}

void SettingsDialog::CancelSettings(void)
{
  qDebug() << "Cancel Settings";
}
