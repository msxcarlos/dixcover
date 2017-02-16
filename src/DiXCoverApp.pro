#-------------------------------------------------
#
# Project created by QtCreator 2015-06-12T14:39:09
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

TARGET = MiroApp
TEMPLATE = app


SOURCES += main.cpp\
        omens.cpp \
    parallelwebcam.cpp \
    SharedImageBuffer.cpp \
    ProcessingThread.cpp \
    MatToQImage.cpp \
    FrameLabel.cpp \
    CaptureThread.cpp \
    CameraView.cpp \
    CameraConnectDialog.cpp \
    ImageProcessingSettingsDialog.cpp \
    cvmatandqimage.cpp \
    mainwindow.cpp \
    qtcamera.cpp \
    imagesettings.cpp

HEADERS += mainwindow.h \
        omens.h \
    parallelwebcam.h \
    Structures.h \
    SharedImageBuffer.h \
    ProcessingThread.h \
    MatToQImage.h \
    FrameLabel.h \
    CaptureThread.h \
    Buffer.h \
    Config.h \
    CameraView.h \
    CameraConnectDialog.h \
    ImageProcessingSettingsDialog.h \
    cvmatandqimage.h \
    qtcamera.h \
    imagesettings.h

FORMS    += mainwindow.ui \
    CameraView.ui \
    CameraConnectDialog.ui \
    ImageProcessingSettingsDialog.ui \
    qtcamera.ui \
    imagesettings.ui

RESOURCES += \
    app.qrc

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_features2d -lopencv_videoio -lopencv_imgproc -lopencv_text
LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system -lboost_filesystem
