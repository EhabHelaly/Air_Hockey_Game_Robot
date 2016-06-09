#include <opencv2/flann.hpp>
#include "globals.h"
using namespace cvflann;
using namespace cv;

// Simulation parameters
Scalar _puck_hsv;
Scalar _push_hsv;
Scalar _push_hsv_;
Scalar _table_hsv;

Point _puck_c;
Point _push_c1;
Point _push_c2;

int   _vx1=200;
int   _vy1=0;

int _score_rob=0;
int _score_hum=0;
bool _collision=false;
// Editable parameters
int _VX_PUCK_MIN=1000;
int _VX_PUCK_MAX=1500;
int _VY_PUCK=750;
int _VX_PUSH=750;
int _VY_PUSH=750;

int _puck_r=25;
int _push_r1=15;
int _push_r2=30;

float _elasticity=0.95;

void simulate_table()
{
    if (data.game_mode>=2 )
    {
        image_cHSV=Mat(480,640,CV_8UC3,Scalar(0,0,240));
        line(image_cHSV,Point(TABLE_X_MIN-_puck_r,TABLE_Y_MIN-_puck_r),Point(TABLE_X_MAX+_puck_r,TABLE_Y_MIN-_puck_r),_table_hsv,1,LINE_AA);
        line(image_cHSV,Point(TABLE_X_MIN-_puck_r,TABLE_Y_MAX+_puck_r),Point(TABLE_X_MAX+_puck_r,TABLE_Y_MAX+_puck_r),_table_hsv,1,LINE_AA);
        line(image_cHSV,Point(TABLE_X_MIN-_puck_r,TABLE_Y_MIN-_puck_r),Point(TABLE_X_MIN-_puck_r,TABLE_Y_MAX+_puck_r),_table_hsv,1,LINE_AA);
        line(image_cHSV,Point(TABLE_X_MAX+_puck_r,TABLE_Y_MIN-_puck_r),Point(TABLE_X_MAX+_puck_r,TABLE_Y_MAX+_puck_r),_table_hsv,1,LINE_AA);
        line(image_cHSV,Point(TABLE_X_CENTER,TABLE_Y_MIN-_puck_r),Point(TABLE_X_CENTER,TABLE_Y_MAX+_puck_r),_table_hsv,1,LINE_AA);
        circle(image_cHSV,Point(TABLE_X_CENTER,TABLE_Y_CENTER),_puck_r*2,_table_hsv,1,LINE_AA);
    }
}
void simulate_attacks()
{
    if (_puck_c.x>TABLE_X_MAX || _puck_c.x<TABLE_X_MIN || ( _vx1<0 &&  _puck_c.x<TABLE_X_CENTER*rand_double() ))// random puck
    {
        _vx1=rand_int(_VX_PUCK_MAX,_VX_PUCK_MIN);
        _vy1=rand_int(_VY_PUCK,-1* _VY_PUCK);

        if (_puck_c.x>TABLE_X_MAX)
        {
            _puck_c.x=TABLE_X_MIN;
            _puck_c.y=rand_int(TABLE_Y_MAX-50,TABLE_Y_MIN+50);
            _score_hum++;
        }
        else
        {
            _puck_c.x=TABLE_X_MIN;
            _score_rob++;
        }
    }
}
void simulate_collision()
{
    float dx=_push_c2.x-_puck_c.x;
    float dy=_push_c2.y-_puck_c.y;
    float dr=pow(dx*dx+dy*dy,0.5);
    int dl=_puck_r+_push_r2;
    if (dr<dl)
    {
        if (!_collision)
        {
            _collision=true;
            if (_vx1<0)
            {
                _vx1+=velocity_x2;
            }
            else
                _vx1=(_vx1*(1-(2*dx/dr))+velocity_x2)*_elasticity;

            _vy1=(_vy1*(1-(2*dy/dr))+velocity_y2)*_elasticity;
        }
    }
    else
        _collision=false;
}
void simulate_puck()
{
    _puck_c.x+=_vx1*delta/1000;
    _puck_c.y+=_vy1*delta/1000;
    if (_puck_c.y>=TABLE_Y_MAX)
    {
        _puck_c.y=TABLE_Y_MAX*2-_puck_c.y;
        _vy1=-1*_vy1*_elasticity;
    }
    else if (_puck_c.y<=TABLE_Y_MIN)
    {
        _puck_c.y=TABLE_Y_MIN*2-_puck_c.y;
        _vy1=-1*_vy1*_elasticity;
    }
    circle(image_cHSV,_puck_c,_puck_r,_puck_hsv,-1);
}
void simulate_push()
{
    if (data.game_mode>=2 )
    {
        if (!_collision)
        {
            int x=_VX_PUSH*delta/1000;
            int y=_VY_PUSH*delta/1000;

            if (abs(_push_c1.x-predict_x)<x)
                _push_c1.x=predict_x;
            else if (_push_c1.x>predict_x)
                _push_c1.x=_push_c1.x-x;
            else if (_push_c1.x<predict_x)
                _push_c1.x=_push_c1.x+x;

            if (abs(_push_c1.y-predict_y)<y)
                _push_c1.y=predict_y;
            else if (_push_c1.y>predict_y)
                _push_c1.y=_push_c1.y-y;
            else if (_push_c1.y<predict_y)
                _push_c1.y=_push_c1.y+y;

            _push_c2.x=_push_c1.x+15;
            _push_c2.y=_push_c1.y;
        }
        circle(image_cHSV,_push_c2,_push_r2,_push_hsv_,-1);
        circle(image_cHSV,_push_c1,_push_r1,_push_hsv ,-1);
    }
}
void simulate() // declared and called in main.cpp
{
    simulate_table();
    simulate_attacks();
    simulate_puck();
    simulate_push();
    simulate_collision();

    cvtColor(image_cHSV,image_cBGR,CV_HSV2BGR);
    sprintf(image_text,"Score: %d:%d",_score_hum,_score_rob);
    putText(image_cBGR,image_text,Point(5,80),3,0.6,green,1,LINE_AA);
}
void simulate_initialization() // declared and called in intialization.cpp
{
    _puck_c=Point(TABLE_X_CENTER,CENTERY_POS);
    _push_c1=Point(DEFENCE_POS,CENTERY_POS);
    _push_c2=Point(DEFENCE_POS+15,CENTERY_POS);

    int h=0.5*puck_hsv_min.val[0]+0.5*puck_hsv_max.val[0];
    int s=0.3*puck_hsv_min.val[1]+0.7*puck_hsv_max.val[1];
    int v=0.3*puck_hsv_min.val[2]+0.7*puck_hsv_max.val[2];
    _puck_hsv=Scalar(h,s,v);

    h=0.5*push_hsv_min.val[0]+0.5*push_hsv_max.val[0];
    s=0.3*push_hsv_min.val[1]+0.7*push_hsv_max.val[1];
    v=0.3*push_hsv_min.val[2]+0.7*push_hsv_max.val[2];
    _push_hsv=Scalar(h,s,v);

    int h_min,h_max,h_mid1,h_mid2;
    int hh1=puck_hsv_min.val[0];
    int hh2=puck_hsv_max.val[0];
    int hh3=push_hsv_min.val[0];
    int hh4=push_hsv_max.val[0];
    if (hh2<hh3)
    {
        h_min=hh1;h_mid1=hh2;h_mid2=hh3-1;h_max=hh4;
    }
    else
    {
        h_min=hh3;h_mid1=hh4;h_mid2=hh1-1;h_max=hh2;
    }
    if (h_min>=h_mid2-h_mid1 && h_min>=179-h_max)
        h=(h_min-1)/2;
    else if (h_mid2-h_mid1>=179-h_max)
        h=(h_mid1+h_mid2+1)/2;
    else
        h=(180+h_max)/2;
    _push_hsv_=Scalar(h,200,200);
    _table_hsv=Scalar(h,250,180);
}
