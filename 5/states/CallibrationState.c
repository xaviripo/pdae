#include <stdio.h>

#include "lib_PAE2.h"
#include "robot/robot.h"
#include "hal/timers.h"
#include "dispatcher.h"

#include "CallibrationState.h"
#include "MenuState.h"

#define L_TITLE 0
#define L_LEFT 3
#define L_FRONT 4
#define L_RIGHT 5

RobotState callibrationState_g; // singleton
sensor_distance_t sd_g;

uint8_t action = 0;

#define C_NONE 0
#define C_LEFT 1
#define C_CENTER 2
#define C_RIGHT 3

char cadena[16];

void callibrateSensors() {

}

void CallibrationState__init () {
    set_comm_timer_interrupt(1);
    callibrationState_g.screen_changed = 1;
}
void CallibrationState__exit () {}

// update engine (se llama en el bucle principal)
void CallibrationState__update() {
    switch (action) {
    case C_NONE:
        break;
    case C_LEFT:
        sd_g = read_obstacle_distance();

        set_thr_left(sd_g.left);
        callibrationState_g.screen_changed = 1;
        break;
    case C_RIGHT:
        sd_g = read_obstacle_distance();

        set_thr_right(sd_g.right);
        callibrationState_g.screen_changed = 1;
        break;
    case C_CENTER:
        sd_g = read_obstacle_distance();

        set_thr_front(sd_g.center);
        callibrationState_g.screen_changed = 1;
        break;
    default:
        break;
    }

    action = C_NONE;

}

// screen
void CallibrationState__draw_screen () {
    halLcdPrintLine("CALIBRATION     ", L_TITLE, 0);

    sprintf(cadena, "LEFT:  %3d      ", get_thr_left());
    halLcdPrintLine(cadena, L_LEFT, 0);

    sprintf(cadena, "FRONT: %3d      ", get_thr_front());
    halLcdPrintLine(cadena, L_FRONT, 0);

    sprintf(cadena, "RIGHT: %3d      ", get_thr_right());
    halLcdPrintLine(cadena, L_RIGHT, 0);
}

// controls
void CallibrationState__s2_pressed () {
    // S2 -> RETURN TO MENU
    set_state(MenuState());
    //ctx_g->set_state(MenuState(ctx_g));
}
void CallibrationState__s1_pressed () {}
void CallibrationState__up_pressed () {
    // calibrar front
    action = C_CENTER;}
void CallibrationState__down_pressed () {}
void CallibrationState__left_pressed () {
    // calibrar left
    action = C_LEFT;
}
void CallibrationState__right_pressed () {
    // calibrar right
    action = C_RIGHT;
}
void CallibrationState__center_pressed () {}

RobotState *CallibrationState() {
    callibrationState_g.init = &CallibrationState__init;
    callibrationState_g.exit = &CallibrationState__exit;
    callibrationState_g.update = &CallibrationState__update;
    callibrationState_g.draw_screen = &CallibrationState__draw_screen;
    callibrationState_g.s2_pressed = &CallibrationState__s2_pressed;
    callibrationState_g.s1_pressed = &CallibrationState__s1_pressed;
    callibrationState_g.up_pressed = &CallibrationState__up_pressed;
    callibrationState_g.down_pressed = &CallibrationState__down_pressed;
    callibrationState_g.left_pressed = &CallibrationState__left_pressed;
    callibrationState_g.right_pressed = &CallibrationState__right_pressed;
    callibrationState_g.center_pressed = &CallibrationState__center_pressed;

    return &callibrationState_g;
}
