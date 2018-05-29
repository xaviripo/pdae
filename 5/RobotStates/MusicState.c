#include "MusicState.h"

RobotState musicState_g; // singleton

void MusicState__init () {}
void MusicState__exit () {}

// update engine (se llama en el bucle principal)
void MusicState__update() {}

// screen
void MusicState__draw_screen () {}

// controls
void MusicState__s2_pressed () {}
void MusicState__s1_pressed () {}
void MusicState__up_pressed () {}
void MusicState__down_pressed () {}
void MusicState__left_pressed () {}
void MusicState__right_pressed () {}
void MusicState__center_pressed () {}

RobotState *MusicState() {
    musicState_g.init = &MusicState__init;
    musicState_g.exit = &MusicState__exit;
    musicState_g.update = &MusicState__update;
    musicState_g.draw_screen = &MusicState__draw_screen;
    musicState_g.s2_pressed = &MusicState__s2_pressed;
    musicState_g.s1_pressed = &MusicState__s1_pressed;
    musicState_g.up_pressed = &MusicState__up_pressed;
    musicState_g.down_pressed = &MusicState__down_pressed;
    musicState_g.left_pressed = &MusicState__left_pressed;
    musicState_g.right_pressed = &MusicState__right_pressed;
    musicState_g.left_pressed = &MusicState__left_pressed;
    musicState_g.center_pressed = &MusicState__center_pressed;
    return &musicState_g;
}
