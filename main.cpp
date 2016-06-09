#include <ctime>
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>
#include "globals.h"
using namespace cv;

DWORD time_start,time_old,time_new;
int frames=1;
float fps;

////////////////////////
/// Global Variables ///
////////////////////////
char image_text[50];
Data data;
// positions
int puck_x,puck_y;
int push_x,push_y;
int predict_x,predict_y;
// old positions
int puck_x_old,puck_y_old;
int push_x_old,push_y_old;
int predict_x_old,predict_y_old;
// velocities slope and time delta
float velocity_x,velocity_y;
float velocity_x2,velocity_y2;
float slope;
int delta;
int time_impact;

// images
Mat image_cBGR(480,640,CV_8UC3);
Mat image_cHSV(480,640,CV_8UC3);
Mat image_puck(480,640,CV_8UC1);
Mat image_push(480,640,CV_8UC1);

// colors
Scalar puck_hsv_min;
Scalar puck_hsv_max;
Scalar push_hsv_min;
Scalar push_hsv_max;
Scalar green (30,255,30);
Scalar blue  (255,150,100);
Scalar orange(0,150,255);
Scalar purple(250,100,200);
///////////////////////////

// Function Declarations
void simulate();
int initialize_code_parameters(int argc, char** argv);
// Function Definitions
void detect_objects(Mat source,Scalar color_min,Scalar color_max,Mat dest)
{
    inRange(source,color_min,color_max,dest);
    // filter image
    erode (dest,dest,Mat());
    dilate(dest,dest,Mat());
}
void identify_object(Mat source,int &pos_x,int &pos_y,int area_min,int area_max)
{
    vector<vector<Point> > contours0;
    vector<vector<Point> > contours1;
    vector<Vec4i> hierarchy;
    pos_x=0;
    pos_y=0;
    findContours(source,contours0,hierarchy,RETR_TREE, CHAIN_APPROX_SIMPLE);// mode,method
    // approximate
    contours1.resize(contours0.size());
    for( size_t k = 0; k < contours0.size(); k++ )
        approxPolyDP(Mat(contours0[k]), contours1[k], 3, true);
    float contour=0;
    while (contours1.size()>0 && contour>=0)
    {
        float radius,a1,a2;
        Point2f center;

        minEnclosingCircle(contours1[contour],center,radius);
        a1=contourArea(contours1[contour]);
        a2=3.1415*radius*radius;
        if (a1/a2>ROUNDNESS_FACTOR && a2>area_min && a2<area_max) // checks for roundness,size
        {
            // object identified, center holds the object position
            circle(image_cBGR,center,radius,orange, 1,LINE_AA);
            circle(image_cBGR,center,     1,orange,-1,LINE_AA);
            pos_x=center.x;
            pos_y=center.y;

            return;
        }
        contour=hierarchy[contour][0];
    }

}
void predict_path()
{
        velocity_x2=(push_x-push_x_old)*1000/delta;
        velocity_y2=(push_y-push_y_old)*1000/delta;

    if (puck_y==0 || puck_y_old==0) // lost track of puck || 1st recognition (not enough to predict path)
    {
        predict_x=predict_x_old;
        predict_y=predict_y_old;
        sprintf(image_text,"Mode: lost track or 1st recognition");
    }
    else
    {
        velocity_x=(puck_x-puck_x_old)*1000/delta;
        velocity_y=(puck_y-puck_y_old)*1000/delta;
        if (velocity_x<SLOW_PUCK) // receding or slow
        {
            if (abs(velocity_x)<SLOW_PUCK &&puck_x>(TABLE_X_CENTER+TABLE_X_MAX)/2) // in range
            {
                predict_x=puck_x;
                predict_y=puck_y;
                sprintf(image_text,"Mode: slow");
            }
            else // receding or not in range yet
            {
                predict_x=DEFENCE_POS;
                predict_y=CENTERY_POS;
                sprintf(image_text,"Mode: receding or not in range");

            }
            putText(image_cBGR,image_text,Point(150,60),3,0.6,blue,1,LINE_AA);
            return;
        }
        slope=velocity_y/velocity_x;
        predict_x=DEFENCE_POS;
        predict_y=slope*(predict_x-puck_x)+puck_y;
        if (predict_y>TABLE_Y_MAX||predict_y<TABLE_Y_MIN) // bounce
        {
            int bounce_x,bounce_y;

            if (predict_y>TABLE_Y_MAX)
                bounce_y=TABLE_Y_MAX;
            else
                bounce_y=TABLE_Y_MIN;
            bounce_x=(bounce_y-puck_y)/slope+puck_x;
            predict_y=slope*(bounce_x-predict_x)+bounce_y;
            if (predict_y>TABLE_Y_MAX||predict_y<TABLE_Y_MIN) // bounce again
            {
                predict_x=predict_x_old;
                predict_y=predict_y_old;
                sprintf(image_text,"Mode: bounce more than 1 time");
            }
            else
            {
                sprintf(image_text,"Mode: bounce");
                line(image_cBGR,Point(puck_x,puck_y),Point(bounce_x,bounce_y),green,1,LINE_AA);
                line(image_cBGR,Point(predict_x,predict_y),Point(bounce_x,bounce_y),green,1,LINE_AA);
            }
        }
        else // direct hit
        {
            sprintf(image_text,"Mode: direct hit");
            line(image_cBGR,Point(puck_x,puck_y),Point(predict_x,predict_y),green,1,LINE_AA);
        }

    }
    predict_x=(predict_x+predict_x_old)/2;
    predict_y=(predict_y+predict_y_old)/2;
    putText(image_cBGR,image_text,Point(150,60),3,0.6,blue,1,LINE_AA);
}
void put_data_on_frame()
{
    fps=(1000.0f*frames)/(time_new-time_start);
    sprintf(image_text,"Time: %ld",(time_new-time_start));
    putText(image_cBGR,image_text,Point(5,20),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Frame: %.d",frames);
    putText(image_cBGR,image_text,Point(5,40),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Fps: %.2f",fps);
    putText(image_cBGR,image_text,Point(5,60),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Puck: %d,%d",puck_x,puck_y);
    putText(image_cBGR,image_text,Point(150,20),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Push: %d,%d",push_x,push_y);
    putText(image_cBGR,image_text,Point(150,40),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Vx: %.0f",velocity_x);
    putText(image_cBGR,image_text,Point(350,20),3,0.6,blue,1,LINE_AA);
    sprintf(image_text,"Vy: %.0f",velocity_y);
    putText(image_cBGR,image_text,Point(350,40),3,0.6,blue,1,LINE_AA);

}
int main(int argc,char* argv[])
{
    // parameters initialization
    if (!initialize_code_parameters(argc,argv))
    {
        MessageBox(NULL,"Initialization Failed !","AHV Game Error",NULL);
        return -1;
    }
    printf("Initialization Completed\nStarting Game\n");

    // video initialization
    time_t ctime=time(0);
    tm *date=localtime(&ctime);
    struct stat dir;
    if (stat("Videos",&dir))
        mkdir("Videos");
    sprintf(image_text,"Videos/Game_%d_%d_%d_%d_%d.avi",date->tm_year+1900,date->tm_mon+1,date->tm_mday,date->tm_hour,date->tm_min);
    VideoWriter video(image_text,CV_FOURCC('P','I','M','1'),data.vfps,Size(640,480),true);

    // camera initialization
    VideoCapture cam(data.camera);
    if (!cam.isOpened())
    {
        MessageBox(NULL,"Failed to open camera, please check that the desired camera is fully functioning","AHV Game Error",NULL);
        return -1;
    }
    cam.set(CAP_PROP_FRAME_WIDTH,640);
    cam.set(CAP_PROP_FRAME_HEIGHT,480);
    cam.set(CAP_PROP_FPS,60);

    // create widows
    namedWindow("camera",0);

    // start game
    time_start=GetTickCount();
    time_old =time_start;
    waitKey(data.vfps);
    while (1)
    {
        // get camera frame, convert it to HSV color space
        cam>>image_cBGR;
        if (image_cBGR.empty())
        {
            printf("Failed to obtain camera frame\n");
            return-1;
        }
        time_new=GetTickCount(); // frame capturing time
        delta=time_new-time_old;
        cvtColor(image_cBGR,image_cHSV,CV_BGR2HSV);

        //* simulate (toggle comment if not needed)
        if (data.game_mode>0)
            simulate(); //*/


        // thresh images
        detect_objects(image_cHSV,puck_hsv_min,puck_hsv_max,image_puck);
        detect_objects(image_cHSV,push_hsv_min,push_hsv_max,image_push);

        // center calculation
        identify_object(image_puck,puck_x,puck_y,PUCK_AREA_MIN,PUCK_AREA_MAX);
        identify_object(image_push,push_x,push_y,PUSH_AREA_MIN,PUSH_AREA_MAX);

        // predict the the puck path, determine the pusher's current mode and target position
        predict_path();

        // add frame info to the frame
        put_data_on_frame();

        // update video, show images
        video<<image_cBGR;
        imshow("camera",image_cBGR);

        // breaks on pressing escape
        if (waitKey(10)==27)
            break;

        // adjust variables for the next frame
        puck_x_old=puck_x;
        puck_y_old=puck_y;
        push_x_old=push_x;
        push_y_old=push_y;
        predict_x_old=predict_x;
        predict_y_old=predict_y;
        time_old=time_new;
        frames++;
    }
    cvDestroyAllWindows();
    MessageBox(NULL,"Game Successfully terminated","AHV Game",NULL);
    return 0;
}
