#-------------------------------------------------
#
# Project created by QtCreator 2014-03-21T14:03:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-vt-uploader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    scanner_table_widget.cpp

HEADERS  += mainwindow.h vt-log.h \
    scanner_table_widget.h

FORMS    += mainwindow.ui
