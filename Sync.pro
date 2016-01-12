#-------------------------------------------------
#
# Project created by QtCreator 2015-06-08T07:55:31
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sync
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    threadcopie.cpp \
    threaddiriterator.cpp

HEADERS  += mainwindow.h \
    threadcopie.h \
    threaddiriterator.h

FORMS    += mainwindow.ui

RESOURCES += \
    application.qrc

DISTFILES += \
    myapp.rc

RC_FILE = myapp.rc
