#include "recognition.h"
#include "object.h"

using namespace std;
using namespace cv;

//
static const int width = 640;
static const int height = 480;
//

recognition::recognition()
{
    iLowH  = iLowS = iLowV = 0;
    iHighH = 500;
    iHighS = iHighV = 500;
}


void recognition::init(){
    trackBar();
    recog();
}

void recognition::trackBar(){
    namedWindow("Control", CV_WINDOW_AUTOSIZE);
    cvCreateTrackbar("LowH","Control", &iLowH, 500);
    cvCreateTrackbar("HighH","Control", &iHighH,500);

    cvCreateTrackbar("LowS","Control", &iLowS, 500);
    cvCreateTrackbar("HighS","Control", &iHighS,500);

    cvCreateTrackbar("LowV","Control", &iLowV, 500);
    cvCreateTrackbar("HighV","Control", &iHighV,500);
}



void recognition::recog(){

    VideoCapture cap(1);
    if( !cap.isOpened()){
        cout<<"no se pudo abrir a camara"<<endl;
        return;
    }

    while(true){
        Mat imgOriginal;
        bool bSuccess = cap.read(imgOriginal);

        if(bSuccess){
            cout<<"No puede leer frame de el video stream"<<endl;
        }

        Mat imgHSV;
        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
        Mat imgThresholded;


        //editando
        CvPoint mid_bottom, mid_top;
        mid_bottom.x = width/2;
        mid_bottom.y = 0;
        mid_top.x = width/2;
        mid_top.y = height;
        IplImage frame_ipl = imgOriginal;
        cvLine(&frame_ipl, mid_bottom, mid_top, CV_RGB(0,255,0), 2);
        wi = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        cout<<wi<<endl;//640
        hi = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        cout<<hi<<endl;//480
        //editando

        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV),Scalar(iHighH,iHighS,iHighV), imgThresholded);
        dilate(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        erode(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        dilate(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        erode(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

        //_editando
        Mat Salida = imgThresholded.clone() ;

        //cvtColor(imgThresholded, Salida, CV_RGB2GRAY);

        Canny(imgThresholded,Salida,100,200);
        vector< vector<Point> > contours;
        findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        Mat mask = Mat::zeros(imgThresholded.rows, imgThresholded.cols, CV_8UC1);
        drawContours(mask, contours, -1, Scalar(255), CV_FILLED);

        imshow("original", Salida);
        imshow("mask", mask);
        imshow("canny", imgThresholded);
        //imshow("cropped", crop);

        //_editando

        imshow("thresholder image", imgThresholded);
        imshow("original", imgOriginal);

        if(waitKey(30) == 27){
            cout<<"esc key is pressed by user"<<endl;
            break;
        }
    }



}

