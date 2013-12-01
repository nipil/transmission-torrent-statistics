#-------------------------------------------------
#
# Project created by QtCreator 2013-11-23T17:27:04
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = transmission-torrent-statistics
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    tts.cpp \
    rpc.cpp \
    dbs.cpp

HEADERS += \
    tts.h \
    rpc.h \
    common.h \
    dbs.h

OTHER_FILES += \
    TODO

LIBS += -lqjson
