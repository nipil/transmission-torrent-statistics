#-------------------------------------------------
#
# Project created by QtCreator 2013-11-23T17:27:04
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

# DEFINES += QT_NO_DEBUG QT_NO_DEBUG_OUTPUT NDEBUG

QMAKE_CXXFLAGS += -Wall

TARGET = transmission-torrent-statistics
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    tts.cpp \
    rpc.cpp \
    dbs.cpp \
    ttsig.cpp \
    web.cpp \
    options.cpp \
    logger.cpp

HEADERS += \
    tts.h \
    rpc.h \
    common.h \
    dbs.h \
    ttsig.h \
    web.h \
    options.h \
    logger.h

OTHER_FILES += \
    TODO \
    www/tts.js \
    www/index.html \
    www/tts.css

LIBS += -lqjson
