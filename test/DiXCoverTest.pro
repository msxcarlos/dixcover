#-------------------------------------------------
#
# Project created by QtCreator 2015-08-17T19:36:12
#
#-------------------------------------------------

QT       += core gui testlib

QT       -=

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tst_dixcovertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += ../src/omens.h \
        ../src/cvmatandqimage.h \
        ../src/parallelwebcam.h

SOURCES += tst_dixcovertest.cpp \
        ../src/omens.cpp \
        ../src/cvmatandqimage.cpp \
        ../src/parallelwebcam.cpp


DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_features2d -lopencv_videoio -lopencv_imgproc -lopencv_text
LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system -lboost_filesystem


