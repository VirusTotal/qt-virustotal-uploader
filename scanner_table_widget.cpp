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

