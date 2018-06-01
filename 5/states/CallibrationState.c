#include <stdio.h>

#include "lib_PAE2.h"
#include "robot/robot.h"
#include "hal/timers.h"
#include "dispatcher.h"

#include "CallibrationState.h"
#include "MenuState.h"

#define TITLE 0
#define LEFT 3
#define FRONT 4
#define RIGHT 5

RobotState callibrationState_g; // singleton
sensor_distance_t sd_g;

uint8_t action = 0;

#define NONE 0
#define LEFT 1
#define CENTER 2
#define RIGHT 3

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
    case NONE:
        break;
    case LEFT:
        sd_g = read_obstacle_distance();

        set_thr_left(sd_g.left);
        callibrationState_g.screen_changed = 1;
        break;
    case RIGHT:
        sd_g = read_obstacle_distance();

        set_thr_right(sd_g.right);
        callibrationState_g.screen_changed = 1;
        break;
    case CENTER:
        sd_g = read_obstacle_distance();

        set_thr_front(sd_g.center);
        callibrationState_g.screen_changed = 1;
        break;
    default:
        break;
    }

    action = NONE;

}

// screen
void CallibrationState__draw_screen () {
    halLcdPrintLine("CALIBRATION     ", TITLE, 0);

    sprintf(cadena, "LEFT:  %3d      ", get_thr_left());
    halLcdPrintLine(cadena, LEFT, 0);

    sprintf(cadena, "FRONT: %3d      ", get_thr_front());
    halLcdPrintLine(cadena, FRONT, 0);

    sprintf(cadena, "RIGHT: %3d      ", get_thr_right());
    halLcdPrintLine(cadena, RIGHT, 0);
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
    action = CENTER;}
void CallibrationState__down_pressed () {}
void CallibrationState__left_pressed () {
    // calibrar left
    action = LEFT;
}
void CallibrationState__right_pressed () {
    // calibrar right
    action = RIGHT;
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
