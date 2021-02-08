#-------------------------------------------------
#
# Project created by QtCreator 2020-12-24T09:07:18
#
#-------------------------------------------------

QT       += core gui charts sql


greaterThan(QT_MAJOR_VERSION, 4): QT += core gui charts sql

TARGET = SDI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#win32:QMAKE_LFLAGS += -shared
#CONFIG += c++11 console
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$PWD/ffmpeg/include
INCLUDEPATH += $$PWD/SDL2/include
INCLUDEPATH += $$PWD/portaudio/include
INCLUDEPATH += $$PWD/player/
INCLUDEPATH += $$PWD/player/impl

LIBS += $$PWD/ffmpeg/lib/avformat.lib   \
        $$PWD/ffmpeg/lib/avcodec.lib    \
        $$PWD/ffmpeg/lib/avdevice.lib   \
        $$PWD/ffmpeg/lib/avfilter.lib   \
        $$PWD/ffmpeg/lib/avutil.lib     \
        $$PWD/ffmpeg/lib/postproc.lib   \
        $$PWD/ffmpeg/lib/swresample.lib \
        $$PWD/ffmpeg/lib/swscale.lib    \
        $$PWD/SDL2/lib/x86/SDL2.lib     \
        $$PWD/portaudio/lib/portaudio_x86.lib

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        codeeditor.cpp \
    findfilesthread.cpp \
    dbmanager.cpp \
    player/impl/avdecodecontext.cpp \
    player/impl/avpacketqueue.cpp\
    player/avdecoder.cpp \
    player/impl/avdecodecontroller.cpp \
    player/glwidget.cpp \
    player/qplayerslider.cpp


HEADERS += \
    mainwindow.h \
    widget_util.h \
    codeeditor.h \
    findfilesthread.h \
    dbmanager.h \
    model.h \
    player/impl/avdecodecontext.h \
    player/impl/avpacketqueue.h \
    player/avdecoder.h \
    player/impl/avdecodecontroller.h \
    player/glwidget.h \
    player/qplayerslider.h \
    player/impl/avdecodetime.h



FORMS += \
        mainwindow.ui

RESOURCES     = application.qrc


