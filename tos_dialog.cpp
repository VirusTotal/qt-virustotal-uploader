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

