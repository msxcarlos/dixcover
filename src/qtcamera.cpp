#include "qtcamera.h"
#include "ui_qtcamera.h"
#include "imagesettings.h"


#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QTimer>


QtCamera::QtCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QtCamera),
    camera(0),
    imageCapture(0),
    isCapturingImage(false),
    applicationExiting(false)
{
    ui->setupUi(this);

    setCamera(QCameraInfo::defaultCamera());

}

QtCamera::~QtCamera()
{
    delete ui;
    delete imageCapture;
    delete camera;

}

void QtCamera::setCamera(const QCameraInfo &cameraInfo)
{
    delete imageCapture;
    delete camera;

    camera = new QCamera(cameraInfo);

    connect(camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

    imageCapture = new QCameraImageCapture(camera);

    connect(ui->sldExposureCompensation, SIGNAL(valueChanged(int)), SLOT(setExposureCompensation(int)));

    camera->setViewfinder(ui->viewfinder);

    updateCameraState(camera->state());
    updateLockStatus(camera->lockStatus(), QCamera::UserRequest);

    connect(imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    connect(imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
            SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    connect(camera, SIGNAL(lockStatusChanged(QCamera::LockStatus,QCamera::LockChangeReason)),
            this, SLOT(updateLockStatus(QCamera::LockStatus,QCamera::LockChangeReason)));

    camera->start();
}

void QtCamera::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    // Display captured image for 4 seconds.
    displayCapturedImage();
    QTimer::singleShot(4000, this, SLOT(displayViewfinder()));
}

void QtCamera::configureCaptureSettings()
{
    switch (camera->captureMode()) {
    case QCamera::CaptureStillImage:
        configureImageSettings();
        break;
    default:
        break;
    }
}

void QtCamera::configureImageSettings()
{
    ImageSettings settingsDialog(imageCapture);

    settingsDialog.setImageSettings(imageSettings);

    if (settingsDialog.exec()) {
        imageSettings = settingsDialog.imageSettings();
        imageCapture->setEncodingSettings(imageSettings);
    }
}

void QtCamera::toggleLock()
{
    switch (camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        camera->unlock();
        break;
    case QCamera::Unlocked:
        camera->searchAndLock();
    }
}

void QtCamera::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
    QColor indicationColor = Qt::black;

    switch (status) {
    case QCamera::Searching:
        indicationColor = Qt::yellow;
        ui->btnLock->setText(tr("Focusing..."));
        break;
    case QCamera::Locked:
        indicationColor = Qt::darkGreen;
        ui->btnLock->setText(tr("Unlock"));
        break;
    case QCamera::Unlocked:
        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
        ui->btnLock->setText(tr("Focus"));
        if (reason == QCamera::LockFailed)
            QMessageBox::warning(this, tr("Image Capture Error"), tr("Focus Failed"));
    }

    QPalette palette = ui->btnLock->palette();
    palette.setColor(QPalette::ButtonText, indicationColor);
    ui->btnLock->setPalette(palette);
}

void QtCamera::takeImage()
{
    isCapturingImage = true;
    imageCapture->capture();
}

void QtCamera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    isCapturingImage = false;
}

void QtCamera::startCamera()
{
    camera->start();
}

void QtCamera::stopCamera()
{
    camera->stop();
}

void QtCamera::updateCameraState(QCamera::State state)
{
    switch (state) {
    case QCamera::ActiveState:
        ui->btnStartCamera->setEnabled(false);
        //ui->actionStopCamera->setEnabled(true);
        ui->btnSettings->setEnabled(true);
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        ui->btnStartCamera->setEnabled(true);
        //ui->actionStopCamera->setEnabled(false);
        ui->btnSettings->setEnabled(false);
    }
}

void QtCamera::setExposureCompensation(int index)
{
    camera->exposure()->setExposureCompensation(index*0.5);
}

void QtCamera::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera error"), camera->errorString());
}

void QtCamera::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void QtCamera::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void QtCamera::readyForCapture(bool ready)
{
    ui->btnTakeImage->setEnabled(ready);
}

void QtCamera::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);
    Q_UNUSED(fileName);

    isCapturingImage = false;
    if (applicationExiting)
        close();
}

