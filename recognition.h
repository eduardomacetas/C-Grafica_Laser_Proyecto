#ifndef RECOGNITION_H;
#define RECOGNITION_H
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

class recognition
{
public:
    int iLowH, iHighH;
    int iLowS, iHighS;
    int iLowV, iHighV;
    double hi;
    double wi;

public:
    recognition();
    void init();
    void trackBar();
    void recog();

    //
    void saveXYZ(const char*filename, const Mat& mat);
    //

};

#endif // RECOGNITION_H
