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

#include <QSettings>
#include "tos_dialog.h"
#include "ui_tos_dialog.h"

ToSDialog::ToSDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ToSDialog)
{
  QSettings settings;

  ui->setupUi(this);

  ui->show_tos_checkBox->setChecked(settings.value("ui/show_tos", true).toBool());
  connect(ui->okButton, SIGNAL(clicked()), this, SLOT(OkClicked()));
  connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(CancelClicked()));

}

ToSDialog::~ToSDialog()
{
  delete ui;
}

void ToSDialog::OkClicked(void)
{
  QSettings settings;

  settings.setValue("ui/show_tos", ui->show_tos_checkBox->isChecked());
  this->close();
}

void ToSDialog::CancelClicked(void)
{
  QSettings settings;
  settings.setValue("ui/show_tos", false);
  emit ToSNotAccepted();
  this->close();
}

