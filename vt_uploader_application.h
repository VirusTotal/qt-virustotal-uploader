#ifndef VT_UPLOADER_APPLICATION_H
#define VT_UPLOADER_APPLICATION_H

#include <QApplication>

class VTUploaderApplication : public QApplication
{
  Q_OBJECT
  protected:
    bool event(QEvent *);
  public:
    VTUploaderApplication(int & argc, char ** argv);
  signals:
    void loadFile(QString fileName);
};

#endif // VT_UPLOADER_APPLICATION_H
