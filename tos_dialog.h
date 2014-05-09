#ifndef TOS_DIALOG_H
#define TOS_DIALOG_H

#include <QDialog>

namespace Ui {
  class ToSDialog;
}

class ToSDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ToSDialog(QWidget *parent = 0);
    ~ToSDialog();

  private:
    Ui::ToSDialog *ui;

  signals:
    void ToSNotAccepted();

  private slots:
    void OkClicked(void);
    void CancelClicked(void);
};

#endif // TOS_DIALOG_H
