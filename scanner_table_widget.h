#ifndef SCANNER_TABLE_WIDGET_H
#define SCANNER_TABLE_WIDGET_H

#include <QTableWidget>

class ScannerTableWidget : public QTableWidget
{
  Q_OBJECT
public:
  explicit ScannerTableWidget(QWidget *parent = 0);

signals:

  void changed(const QMimeData *mimeData = 0);
  void dropped(const QMimeData *mimeData = 0);
public slots:
  void clear();



protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);

};

#endif // SCANNER_TABLE_WIDGET_H
