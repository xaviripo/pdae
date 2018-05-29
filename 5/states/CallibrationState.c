#include "CallibrationState.h"

RobotState callibrationState_g; // singleton

void callibrateSensors() {

}

void CallibrationState__init () {}
void CallibrationState__exit () {}

// update engine (se llama en el bucle principal)
void CallibrationState__update() {}

// screen
void CallibrationState__draw_screen () {}

// controls
void CallibrationState__s2_pressed () {}
void CallibrationState__s1_pressed () {}
void CallibrationState__up_pressed () {}
void CallibrationState__down_pressed () {}
void CallibrationState__left_pressed () {}
void CallibrationState__right_pressed () {}
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
    callibrationState_g.left_pressed = &CallibrationState__left_pressed;
    callibrationState_g.center_pressed = &CallibrationState__center_pressed;
    return &callibrationState_g;
}
