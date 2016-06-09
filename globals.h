#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <opencv2/opencv.hpp>
using namespace std;

struct Data{
    string port;
    int camera;
    int vfps;
    int game_mode; // 0:Game(robot vs human), 1:Simulation(robot vs virtual puck), 2:Simulation(virtual puck and robot)
};
extern struct Data data;
extern char image_text[50];
// positions
extern int puck_x,puck_y;
extern int push_x,push_y;
extern int predict_x,predict_y;
// old positions
extern int puck_x_old,puck_y_old;
extern int push_x_old,push_y_old;
extern int predict_x_old,predict_y_old;
// velocities slope and time delta
extern float velocity_x,velocity_y;
extern float velocity_x2,velocity_y2;
extern float slope;
extern int delta;
extern int time_impact;

// images
extern cv::Mat image_cBGR;
extern cv::Mat image_cHSV;
extern cv::Mat image_puck;
extern cv::Mat image_push;

// colors
extern cv::Scalar puck_hsv_min;
extern cv::Scalar puck_hsv_max;
extern cv::Scalar push_hsv_min;
extern cv::Scalar push_hsv_max;
extern cv::Scalar green ;
extern cv::Scalar blue  ;
extern cv::Scalar orange;
extern cv::Scalar purple;

//////////////////////////////////////////
// definitions
extern int PUCK_AREA_MIN;
extern int PUCK_AREA_MAX;
extern int PUSH_AREA_MIN;
extern int PUSH_AREA_MAX;

extern float ROUNDNESS_FACTOR;

extern int DEFENCE_POS;
extern int CENTERY_POS;

extern int TABLE_X_MIN;
extern int TABLE_X_MAX;
extern int TABLE_X_CENTER;
extern int TABLE_Y_MIN;
extern int TABLE_Y_MAX;
extern int TABLE_Y_CENTER;

extern float PIX2STEP;
extern float PIX2MM;
extern float MM2STEP;

extern float CAM_ROTATION;
extern int MAX_SPEED;
extern int SLOW_PUCK;
/////////////////////////////////////


#endif // GLOBALS_H_INCLUDED
