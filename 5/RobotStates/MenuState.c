#include "MenuState.h"

RobotState menuState_g; // singleton

void MenuState__init () {}
void MenuState__exit () {}

// update engine (se llama en el bucle principal)
void MenuState__update() {}

// screen
void MenuState__draw_screen () {}

// controls
void MenuState__s2_pressed () {}
void MenuState__s1_pressed () {}
void MenuState__up_pressed () {}
void MenuState__down_pressed () {}
void MenuState__left_pressed () {}
void MenuState__right_pressed () {}
void MenuState__center_pressed () {}

RobotState *MenuState() {
    menuState_g.init = &MenuState__init;
    menuState_g.exit = &MenuState__exit;
    menuState_g.update = &MenuState__update;
    menuState_g.draw_screen = &MenuState__draw_screen;
    menuState_g.s2_pressed = &MenuState__s2_pressed;
    menuState_g.s1_pressed = &MenuState__s1_pressed;
    menuState_g.up_pressed = &MenuState__up_pressed;
    menuState_g.down_pressed = &MenuState__down_pressed;
    menuState_g.left_pressed = &MenuState__left_pressed;
    menuState_g.right_pressed = &MenuState__right_pressed;
    menuState_g.left_pressed = &MenuState__left_pressed;
    menuState_g.center_pressed = &MenuState__center_pressed;
    return &menuState_g;
}
