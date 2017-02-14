#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Local
#include "CameraConnectDialog.h"
#include "CameraView.h"
#include "Buffer.h"
#include "SharedImageBuffer.h"

#include <QMainWindow>
#include <QScrollBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionZoom_In_25_triggered();

    void on_actionZoom_Out_25_triggered();

    void on_actionNormal_Size_triggered();

    void on_actionFit_to_window_triggered();

    void on_actionAbout_triggered();

    void on_actionMetadata_Identification_triggered();

    void on_actionExit_triggered();

    void on_actionVideo_capture_triggered();

    void on_actionPhoto_capture_triggered();

    void updateCameraDevice(QAction *action);


private:
    Ui::MainWindow *ui;

    bool loadFile(const QString &fileName);
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    double scaleFactor;
    SharedImageBuffer *sharedImageBuffer;


};

#endif // MAINWINDOW_H
