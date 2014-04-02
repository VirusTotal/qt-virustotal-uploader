#-------------------------------------------------
#
# Project created by QtCreator 2014-03-21T14:03:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-vt-uploader
TEMPLATE = app



unix {
  INCLUDEPATH += /usr/local/include/ $$(HOME)/local/include
}
win32 {
  INCLUDEPATH += "C:\Work\include"
}


SOURCES += main.cpp\
        mainwindow.cpp \
    scanner_table_widget.cpp \
    vtfile.cpp \
    calc_file_hashes_task.cpp \
    check_report_task.cpp

HEADERS  += mainwindow.h vt-log.h \
    scanner_table_widget.h \
    vtfile.h \
    calc_file_hashes_task.h \
    check_report_task.h

FORMS    += mainwindow.ui
