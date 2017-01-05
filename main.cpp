#include <QCoreApplication>
#include "Object.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

// valores iniciales de los valores mínimos y máximos del filtro HSV.
// estos serán cambiados usando trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

// ancho y altura de captura predeterminados
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

// número máximo de objetos a detectar en el marco
const int MAX_NUM_OBJECTS=50;

// área mínima y máxima del objeto
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

// nombres que aparecerán en la parte superior de cada ventana

const string windowName = "Imagen Original";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

// Lo siguiente es para el detector de borde canny
Mat dst, detected_edges;
Mat src, src_gray;
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
const char* window_name = "Edge Map";

void on_trackbar( int, void* )
{
    // Esta función se llama cuando
    // se cambia la posición de la barra de seguimiento
}

string intToString(int number){
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void createTrackbars(){
    //Creamos la ventana para trackbars
    namedWindow(trackbarWindowName,0);

    //Crear memoria para almacenar el nombre de la barra de seguimiento en la ventana
    char TrackbarName[50];
    sprintf( TrackbarName, "H_MIN", H_MIN);
    sprintf( TrackbarName, "H_MAX", H_MAX);
    sprintf( TrackbarName, "S_MIN", S_MIN);
    sprintf( TrackbarName, "S_MAX", S_MAX);
    sprintf( TrackbarName, "V_MIN", V_MIN);
    sprintf( TrackbarName, "V_MAX", V_MAX);

    // Creamos trackbars e insertamos en la ventana
    // Los 3 parámetros son: la dirección de la variable que está cambiando cuando se mueve la barra de desplazamiento (eg.H_LOW),
    // El valor máximo que puede mover la barra de seguimiento (por ejemplo, H_HIGH),
    // Y la función que se llama cada vez que se mueve la barra de seguimiento (por ejemplo, on_trackbar)
    //                                  ---->    ---->     ---->
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );
}

void drawObject(vector<Object> theObjects,Mat &frame, Mat &temp, vector< vector<Point> > contours, vector<Vec4i> hierarchy){

    for(int i =0; i<theObjects.size(); i++){

    cv::drawContours(frame,contours,i,theObjects.at(i).getColor(),3,8,hierarchy);
    cv::circle(frame,cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()),5,theObjects.at(i).getColor());
    cv::putText(frame,intToString(theObjects.at(i).getXPos())+ " , " + intToString(theObjects.at(i).getYPos()),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()+20),1,1,theObjects.at(i).getColor());
    cv::putText(frame,theObjects.at(i).getType(),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()-20),1,2,theObjects.at(i).getColor());
    }
}

void drawObject(vector<Object> theObjects,Mat &frame){

    for(int i =0; i<theObjects.size(); i++){

    cv::circle(frame,cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()),10,cv::Scalar(0,0,255));
    cv::putText(frame,intToString(theObjects.at(i).getXPos())+ " , " + intToString(theObjects.at(i).getYPos()),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()+20),1,1,Scalar(0,255,0));
    cv::putText(frame,theObjects.at(i).getType(),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()-30),1,2,theObjects.at(i).getColor());
    }
}

void morphOps(Mat &thresh){

    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

    erode(thresh,thresh,erodeElement);
    erode(thresh,thresh,erodeElement);

    dilate(thresh,thresh,dilateElement);
    dilate(thresh,thresh,dilateElement);
}

void trackFilteredObject(Mat threshold,Mat HSV, Mat &cameraFeed)
{
    vector <Object> objects;
    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS)
        {
            for (int index = 0; index >= 0; index = hierarchy[index][0])
            {
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA)
                {
                    Object object;

                    object.setXPos(moment.m10/area);
                    object.setYPos(moment.m01/area);

                    objects.push_back(object);

                    objectFound = true;

                }
                else objectFound = false;
            }
            //let user know you found an object
            if(objectFound ==true)
            {
                //draw object location on screen
                drawObject(objects,cameraFeed);
            }
        }
        else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
    }
}

void trackFilteredObject(Object theObject,Mat threshold,Mat HSV, Mat &cameraFeed){

    vector <Object> objects;
    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {

                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;

        //if the area is less than 20 px by 20px then it is probably just noise
        //if the area is the same as the 3/2 of the image size, probably just a bad filter
        //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA){

                    Object object;

                    object.setXPos(moment.m10/area);
                    object.setYPos(moment.m01/area);
                    object.setType(theObject.getType());
                    object.setColor(theObject.getColor());

                    objects.push_back(object);

                    objectFound = true;

                }else objectFound = false;
            }
            //let user know you found an object
            if(objectFound ==true){
                //draw object location on screen
                drawObject(objects,cameraFeed,temp,contours,hierarchy);}

        }else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
    }
}

int main(int argc, char* argv[])
{
    //if we would like to calibrate our filter values, set to true.
    bool calibrationMode = true;

    //Matrix to store each frame of the webcam feed
    Mat cameraFeed;
    Mat threshold;
    Mat HSV;

    if(calibrationMode){
        //create slider bars for HSV filtering
        createTrackbars();
    }
    //video capture object to acquire webcam feed
    VideoCapture capture;
    //open capture object at location zero (default location for webcam)
    capture.open(1);
    //set height and width of capture frame
    capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    //start an infinite loop where webcam feed is copied to cameraFeed matrix
    //all of our operations will be performed within this loop
    waitKey(1000);
    while(1){
        //store image to matrix
        capture.read(cameraFeed);

        src = cameraFeed;

        if( !src.data )
        { return -1; }

        //convert frame from BGR to HSV colorspace
        cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);

        if(calibrationMode==true){

        //need to find the appropriate color range values
        // calibrationMode must be false

        //if in calibration mode, we track objects based on the HSV slider values.
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
            morphOps(threshold);
            imshow(windowName2,threshold);

        //the folowing for canny edge detec
            /// Create a matrix of the same type and size as src (for dst)
            dst.create( src.size(), src.type() );
            /// Convert the image to grayscale
            cvtColor( src, src_gray, CV_BGR2GRAY );
            /// Create a window
            namedWindow( window_name, CV_WINDOW_AUTOSIZE );
            /// Create a Trackbar for user to enter threshold
            createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold);
            /// Show the image
            trackFilteredObject(threshold,HSV,cameraFeed);
        }
        else{
            //create some temp fruit objects so that
            //we can use their member functions/information
            Object blue("blue"), yellow("yellow"), red("red"), green("green");

            //first find blue objects
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,blue.getHSVmin(),blue.getHSVmax(),threshold);
            morphOps(threshold);
            trackFilteredObject(blue,threshold,HSV,cameraFeed);
            //then yellows
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,yellow.getHSVmin(),yellow.getHSVmax(),threshold);
            morphOps(threshold);
            trackFilteredObject(yellow,threshold,HSV,cameraFeed);
            //then reds
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,red.getHSVmin(),red.getHSVmax(),threshold);
            morphOps(threshold);
            trackFilteredObject(red,threshold,HSV,cameraFeed);
            //then greens
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,green.getHSVmin(),green.getHSVmax(),threshold);
            morphOps(threshold);
            trackFilteredObject(green,threshold,HSV,cameraFeed);

        }
        //show frames
        //imshow(windowName2,threshold);

        imshow(windowName,cameraFeed);

        waitKey(30);
    }
    return 0;
}