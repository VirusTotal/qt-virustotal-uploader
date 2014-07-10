#-------------------------------------------------
#
# Project created by QtCreator 2014-03-21T14:03:35
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "VirusTotalUploader"
TEMPLATE = app

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
QMAKE_INFO_PLIST += Info.plist
CONFIG += static

unix {
  INCLUDEPATH += /usr/local/include/ $$(HOME)/local/include/
  LIBS += -L/usr/local/lib -L$$(HOME)/local/lib -lcvtapi -lcurl -ljansson -lz
  QMAKE_CXXFLAGS+= -g3
}
win32 {
  INCLUDEPATH += "C:\Work\include"
}

QMAKE_CXXFLAGS+= -DNOCRYPT
QMAKE_CFLAGS+= -DNOCRYPT




ICON = vticons.icns

SOURCES += main.cpp\
        mainwindow.cpp \
    scanner_table_widget.cpp \
    qvtfile.cpp \
    calc_file_hashes_task.cpp \
    check_report_task.cpp \
    settings_dialog.cpp \
    scan_file_task.cpp \
    rescan_file_task.cpp \
    add_dir_task.cpp \
    tos_dialog.cpp \
    ioapi.c \
    zip.c \
    create_app_zip_task.cpp \
    vt_uploader_application.cpp


HEADERS  += mainwindow.h vt-log.h \
    scanner_table_widget.h \
    qvtfile.h \
    calc_file_hashes_task.h \
    check_report_task.h \
    settings_dialog.h \
    scan_file_task.h \
    rescan_file_task.h \
    add_dir_task.h \
    tos_dialog.h \
    ioapi.h \
    zip.h \
    create_app_zip_task.h \
    vt_uploader_application.h

FORMS    += mainwindow.ui \
    settings_dialog.ui \
    tos_dialog.ui
