#-------------------------------------------------
#
# Project created by QtCreator 2014-03-21T14:03:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-vt-uploader
TEMPLATE = app

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

unix {
  INCLUDEPATH += /usr/local/include/ $$(HOME)/local/include/
  LIBS += -L/usr/local/lib -L$$(HOME)/local/lib -lcvtapi -lcurl -ljansson
  QMAKE_CXXFLAGS+= -g3
}
win32 {
  INCLUDEPATH += "C:\Work\include"
}

ICON = vticons.icns

SOURCES += main.cpp\
        mainwindow.cpp \
    scanner_table_widget.cpp \
    qvtfile.cpp \
    calc_file_hashes_task.cpp \
    check_report_task.cpp \
    settings_dialog.cpp \
    scan_file_task.cpp \
    rescan_file_task.cpp

HEADERS  += mainwindow.h vt-log.h \
    scanner_table_widget.h \
    qvtfile.h \
    calc_file_hashes_task.h \
    check_report_task.h \
    settings_dialog.h \
    scan_file_task.h \
    rescan_file_task.h

FORMS    += mainwindow.ui \
    settings_dialog.ui
