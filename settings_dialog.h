#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QButtonGroup>

#define DIR_ONLY_EXE_KEY "dir/only_exe"
#define RESCAN_DAYS_KEY "rescan_days"
#define API_KEY "api/key"
#define API_RATE_KEY "api/rate"

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
