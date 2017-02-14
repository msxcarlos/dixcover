#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGuiApplication::setApplicationDisplayName(MainWindow::tr("Badak Labs"));
    MainWindow w;
    w.show();

    return a.exec();
}
