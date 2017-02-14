#include <QString>
#include <QtTest>
#include "../src/omens.h"
#include "../src/parallelwebcam.h"
#include <opencv2/opencv.hpp>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>


using namespace std;
using namespace cv;


class TestDiXCover : public QObject
{
    Q_OBJECT

public:
    TestDiXCover();

private Q_SLOTS:
    void testOpencv();
    void testTextDetection();
    void testWebCamTextDetection();
    void testQImage();
};

TestDiXCover::TestDiXCover()
{
}

void TestDiXCover::testOpencv()
{
    QSKIP("This test skip...");
    Mat image;
      image = imread( "../../DiXCover/resources/scenetext01.jpg", 1 );

      if( !image.data )
        {
          printf( "No image data \n" );
        }

      namedWindow( "Display Image", WINDOW_AUTOSIZE );
      imshow( "Display Image", image );

      waitKey(0);

    QVERIFY2(true, "Failure");
}

void TestDiXCover::testTextDetection()
{
    QSKIP("This test skip...");
    QImage img = QImage("../../DiXCover/resources/time20100412.png");
    Omens o;
    o.textDetection(img);
    QVERIFY2(true, "Failure");
}

void TestDiXCover::testWebCamTextDetection()
{
    ParallelWebCam webcam;
    webcam.textDetection();
    QVERIFY2(true, "Failure");
}

void TestDiXCover::testQImage()
{
    QSKIP("This test skip...");

    char  arg0[] = "programName";
        char  arg1[] = "arg";
        char  arg2[] = "another arg";
        char* argv[] = { &arg0[0], &arg1[0], &arg2[0], NULL };
        int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    QApplication a(argc, &argv[0]);

    QImage myImage;
    myImage.load("test.png");

    QLabel myLabel;
    myLabel.setPixmap(QPixmap::fromImage(myImage));

    myLabel.show();

    a.exec();
}


QTEST_APPLESS_MAIN(TestDiXCover)

#include "tst_dixcovertest.moc"
