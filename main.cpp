#include <QCoreApplication>
#include "recognition.h"
#include <cv.h>
#include <highgui.h>
using namespace cv;
using namespace std;



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    recognition tmp;
    tmp.init();

    return a.exec();
}
