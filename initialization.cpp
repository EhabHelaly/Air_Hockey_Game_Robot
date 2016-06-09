#include "globals.h"
#include <fstream>
using namespace cv;
/////////////////////////
//  Global Parameters  //
/////////////////////////
// object identification parameters
int PUCK_AREA_MIN=500;
int PUCK_AREA_MAX=2800;
int PUSH_AREA_MIN=200;
int PUSH_AREA_MAX=800;
float ROUNDNESS_FACTOR=0.7;
// table dimentions
int DEFENCE_POS=600;
int CENTERY_POS=240;
int TABLE_X_MIN=0;
int TABLE_X_MAX=640;
int TABLE_X_CENTER=320;
int TABLE_Y_MIN=35;
int TABLE_Y_MAX=445;
int TABLE_Y_CENTER=240;
float CAM_ROTATION=0.5;
// other
int SLOW_PUCK=200;
int MAX_SPEED=25000;
float PIX2STEP=1.28;
float PIX2MM=1.28;
float MM2STEP=1.28;
////////////////////////
////////////////////////

void simulate_initialization();
bool parse_color(int low1, int high1, int low2, int high2, int mx, bool flag)
{
    return low1>=0 && low1<=mx && high1>=0 && high1<=mx &&
           low2>=0 && low2<=mx && high2>=0 && high2<=mx &&
           low1<high1 && low2<high2 && (low1>high2 || low2>high1 || flag);
}
void load_configutation(int flag)
{
    if (!flag)
        return;
    ifstream conf("configuration.ini");

}
int parse_arguments(int* args)
{
    for (int i=0;i<5;i=i+2)
        if (!(parse_color(args[i],args[i+1],args[i+6],args[i+7],i==0? 179:255,i!=0)))
        {
            printf("Inappropriate color inputs\n");
            return 0;
        }
    puck_hsv_min=Scalar(args[0],args[2],args[ 4]);
    puck_hsv_max=Scalar(args[1],args[3],args[ 5]);
    push_hsv_min=Scalar(args[6],args[8],args[10]);
    push_hsv_max=Scalar(args[7],args[9],args[11]);


    load_configutation(args[12]);
    data.game_mode=args[13];
    data.camera=args[14]<0? 0:args[14];
    data.vfps=args[15]<30? 60:args[15];

    return 1;
}
int initialize_from_BAT_file(int argc, char** argv)
{
    printf("Initializing with passed arguments\n");
    int args[16];
    memset(&args,0,sizeof(int)*16);
    data.port=argv[1];
    for (int i=0;i++;i<argc&&i<16)
        args[i]=atoi(argv[i+2]);

    return parse_arguments(args);
}
int initialize_from_INI_file()
{
    printf("Initializing from INI file\n");
    ifstream ini("parameters.ini");
    if (!ini.is_open())
    {
        printf("Error: Failed to open file ""parameters.ini""\n");
        return 0;
    }
    int args[16];
    memset(&args,0,sizeof(int)*16);
    char line[64];
    ini.getline(line,64);sscanf(line,"puck   HSV color(min): %d,%d,%d",&args[0],&args[2],&args[ 4]);
    ini.getline(line,64);sscanf(line,"puck   HSV color(max): %d,%d,%d",&args[1],&args[3],&args[ 5]);
    ini.getline(line,64);sscanf(line,"pusher HSV color(min): %d,%d,%d",&args[6],&args[8],&args[10]);
    ini.getline(line,64);sscanf(line,"pusher HSV color(max): %d,%d,%d",&args[7],&args[9],&args[11]);
    ini.getline(line,64);sscanf(line,"serial port name: %s"           ,data.port.c_str());// parsed in serial.cpp
    ini.getline(line,64);sscanf(line,"load configuration(0): %d"      ,&args[12]);
    ini.getline(line,64);sscanf(line,"mode (0): %d"                   ,&args[13]);
    ini.getline(line,64);sscanf(line,"camera id(0): %d"               ,&args[14]);
    ini.getline(line,64);sscanf(line,"FPS(60): %d"                    ,&args[15]);
    ini.close();
    for (int i=0;i<16;i++)
        printf("%d ",args[i]);
    printf("\n");
    return parse_arguments(args);
}
int initialize_code_parameters(int argc,char** argv) // declared and called in main.cpp
{
    printf("_______________________________________________________________________________\n");
    printf("_____________________________                     _____________________________\n");
    printf("_____________________________   Air Hockey Game   _____________________________\n");
    printf("_______________________________________________________________________________\n");
    printf("Initializing...\nPlease Wait...\n");
    if (argc==1)
    {
        if (!initialize_from_INI_file())
            return 0;
    }
    else if (argc<14)
    {
        printf("Insufficient number of parameters.c_str() passed for program to initialize\n");
        if (!initialize_from_INI_file())
            return 0;
    }
    else
    {
        if (!initialize_from_BAT_file(argc,argv))
            return 0;
    }

    // game initialization
    puck_x_old=0;
    puck_y_old=0;
    push_x_old=0;
    push_y_old=0;
    predict_x_old=DEFENCE_POS;
    predict_y_old=CENTERY_POS;

    if (data.game_mode>0)
        simulate_initialization();

    return 1;
}
