#ifndef __XBOX_H__
#define __XBOX_H__

int open_controllers(int debug);
struct player_event get_event();

struct player_event {
    int player;
    int event;
    int data;
};

#define NO_EVENT        0

#define BUTTON_A        304
#define BUTTON_B        305
#define BUTTON_X        307
#define BUTTON_Y        308
#define BUTTON_START    315
#define BUTTON_SELECT   314
#define BUTTON_XBOX     316
#define BUTTON_LB       310
#define BUTTON_RB       311
#define BUTTON_LS       317
#define BUTTON_RS       318
#define BUTTON_DUP      706
#define BUTTON_DRIGHT   705
#define BUTTON_DDOWN    707
#define BUTTON_DLEFT    704

#define LEFT_X          0
#define LEFT_Y          1
#define RIGHT_X         3
#define RIGHT_Y         4
#define LEFT_TRIG       2
#define RIGHT_TRIG      5

#endif

