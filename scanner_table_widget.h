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
