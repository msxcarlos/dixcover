#ifndef QTCAMERA_H
#define QTCAMERA_H

#include <QWidget>
#include <QCamera>
#include <QCameraImageCapture>


namespace Ui {
class QtCamera;
}

class QtCamera : public QWidget
{
    Q_OBJECT

public:
    explicit QtCamera(QWidget *parent = 0);
    ~QtCamera();

private slots:
    void setCamera(const QCameraInfo &cameraInfo);

    void startCamera();
    void stopCamera();

    void toggleLock();
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void configureCaptureSettings();
    void configureImageSettings();

    void displayCameraError();

    void updateCameraState(QCamera::State);
    void setExposureCompensation(int index);

    void processCapturedImage(int requestId, const QImage &img);
    void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

protected:


private:
    Ui::QtCamera *ui;

    QCamera *camera;
    QCameraImageCapture *imageCapture;

    QImageEncoderSettings imageSettings;
    bool isCapturingImage;
    bool applicationExiting;



};

#endif // QTCAMERA_H
