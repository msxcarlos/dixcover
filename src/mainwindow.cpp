#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QImageReader>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCameraInfo>
#include "omens.h"
#include "qtcamera.h"

Q_DECLARE_METATYPE(QCameraInfo)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lblImage->setBackgroundRole(QPalette::Base);
    ui->lblImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lblImage->setScaledContents(true);
    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    // Create SharedImageBuffer object
    sharedImageBuffer = new SharedImageBuffer();

    //Camera devices:

    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
        if (cameraInfo == QCameraInfo::defaultCamera())
            videoDeviceAction->setChecked(true);

        ui->menuDevices->addAction(videoDeviceAction);
    }

    connect(videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    /*
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/home", tr("Image Files (*.png *.jpg *.bmp)"));
    if(!fileName.isEmpty()) {
        ui->scrollArea->setBackgroundRole(QPalette::Dark);
        ui->lblImage->clear();
        ui->lblImage->setGeometry(QRect(0, 0, 0, 0));
        ui->lblImage->setBackgroundRole(QPalette::Base);
        ui->lblImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        ui->lblImage->setScaledContents(true);
        ui->lblImage->setPixmap(QPixmap(fileName));
        ui->lblImage->adjustSize();
    }
    */

    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Open File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.first());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

bool MainWindow::loadFile(const QString &fileName)
{
    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        setWindowFilePath(QString());
        ui->lblImage->setPixmap(QPixmap());
        ui->lblImage->adjustSize();
        return false;
    }
    ui->lblImage->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    //printAct->setEnabled(true);
    ui->actionFit_to_window->setEnabled(true);
    updateActions();

    if (!ui->actionFit_to_window->isChecked()) {
        ui->lblImage->adjustSize();
        ui->scrollContents->resize(ui->lblImage->size());
    } else {
        //TODO???
    }


    setWindowFilePath(fileName);
    return true;
}

void MainWindow::updateActions()
{
    ui->actionZoom_In_25->setEnabled(!ui->actionFit_to_window->isChecked());
    ui->actionZoom_Out_25->setEnabled(!ui->actionFit_to_window->isChecked());
    ui->actionNormal_Size->setEnabled(!ui->actionFit_to_window->isChecked());
}

void MainWindow::scaleImage(double factor)
{
    Q_ASSERT(ui->lblImage->pixmap());
    scaleFactor *= factor;
    ui->lblImage->resize(scaleFactor * ui->lblImage->pixmap()->size());
    ui->scrollContents->resize(ui->lblImage->size());

    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), factor);

    ui->actionZoom_In_25->setEnabled(scaleFactor < 3.0);
    ui->actionZoom_Out_25->setEnabled(scaleFactor > 0.333);
}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::on_actionZoom_In_25_triggered()
{
    scaleImage(1.25);
}

void MainWindow::on_actionZoom_Out_25_triggered()
{
    scaleImage(0.8);
}

void MainWindow::on_actionNormal_Size_triggered()
{
    ui->lblImage->adjustSize();
    ui->scrollContents->resize(ui->lblImage->size());
    scaleFactor = 1.0;
}

void MainWindow::on_actionFit_to_window_triggered()
{
    bool fitToWindow = ui->actionFit_to_window->isChecked();
    ui->scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        on_actionNormal_Size_triggered();
    }
    updateActions();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About DiXCover"),
                tr("<p>The <b>DiXCover</b> application shows how to combine Text Detection "
                   "and NLP to display an image with metadata associated. </p> "
                   "<p>Author: <br> Carlos Morales</p>"));
}

void MainWindow::on_actionMetadata_Identification_triggered()
{
    Omens omen;
    ui->textBrowser->clear();
    ui->lblImage->setPixmap(omen.textDetection(ui->lblImage->pixmap()));
    ui->textBrowser->append(omen.getWordsDetection());
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionVideo_capture_triggered()
{

    // Show dialog
    CameraConnectDialog *cameraConnectDialog = new CameraConnectDialog(this, false);
    if(cameraConnectDialog->exec()==QDialog::Accepted)
    {
        // Save user-defined device number
        int deviceNumber = cameraConnectDialog->getDeviceNumber();
            // Create ImageBuffer with user-defined size
            Buffer<Mat> *imageBuffer = new Buffer<Mat>(cameraConnectDialog->getImageBufferSize());
            // Add created ImageBuffer to SharedImageBuffer object
            sharedImageBuffer->add(deviceNumber, imageBuffer, false);
            // Create CameraView
            CameraView* camera = new CameraView(ui->scrollContents, deviceNumber, sharedImageBuffer);

            //ui->scrollLayout->addWidget(camera);
            ui->scrollContents->resize(camera->size());

            // Check if stream synchronization is enabled
            //if(ui->actionSynchronizeStreams->isChecked())
            if(false)
            {
                // Prompt user
                int ret = QMessageBox::question(this, tr("qt-opencv-multithreaded"),
                                                tr("Stream synchronization is enabled.\n\n"
                                                   "Do you want to start processing?\n\n"
                                                   "Choose 'No' if you would like to open additional streams."),
                                                QMessageBox::Yes | QMessageBox::No,
                                                QMessageBox::Yes);
                // Start processing
                if(ret==QMessageBox::Yes)
                    sharedImageBuffer->setSyncEnabled(true);
                // Defer processing
                else
                    sharedImageBuffer->setSyncEnabled(false);
            }

            // Attempt to connect to camera
            if(camera->connectToCamera(cameraConnectDialog->getDropFrameCheckBoxState(),
                                           cameraConnectDialog->getCaptureThreadPrio(),
                                           cameraConnectDialog->getProcessingThreadPrio(),
                                           cameraConnectDialog->getEnableFrameProcessingCheckBoxState(),
                                           cameraConnectDialog->getResolutionWidth(),
                                           cameraConnectDialog->getResolutionHeight()))
            {
                // Prevent user from enabling/disabling stream synchronization after a camera has been connected
                // ui->actionSynchronizeStreams->setEnabled(false);
            }
            // Could not connect to camera
            else
            {
                // Display error message
                QMessageBox::warning(this,"ERROR:","Could not connect to camera. Please check device number.");
                // Remove from shared buffer
                sharedImageBuffer->removeByDeviceNumber(deviceNumber);
                // Explicitly delete ImageBuffer object
                delete imageBuffer;
            }
    }
    // Delete dialog
    delete cameraConnectDialog;


}

void MainWindow::on_actionPhoto_capture_triggered()
{
    // Create CameraView
    QtCamera* camera = new QtCamera(ui->scrollContents);

    //ui->scrollLayout->addWidget(camera);
    ui->scrollContents->resize(camera->size());
}

void MainWindow::updateCameraDevice(QAction *action)
{
    //setCamera(qvariant_cast<QCameraInfo>(action->data()));
}

