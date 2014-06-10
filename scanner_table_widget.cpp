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
#include <QtGui>
#include <QDebug>

#include "scanner_table_widget.h"


ScannerTableWidget::ScannerTableWidget(QWidget *parent) : QTableWidget(parent)
{

  qDebug() << "ScannerTableWidget";

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setDragDropMode(QAbstractItemView::DropOnly);
  setAcceptDrops(true);
}

void ScannerTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();
  emit changed(event->mimeData());
}

void ScannerTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
  event->acceptProposedAction();
}

void ScannerTableWidget::dropEvent(QDropEvent * event)
{
  QString text = event->mimeData()->text();

  qDebug() << "dropEvent " << text;
  event->acceptProposedAction();
  emit dropped(event->mimeData());
}

void ScannerTableWidget::dragLeaveEvent(QDragLeaveEvent *event) {
  event->accept();
}

void ScannerTableWidget::clear() {
  emit changed();
}

