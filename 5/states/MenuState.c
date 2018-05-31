#include "MenuState.h"

#include "MusicState.h"
#include "MovingState.h"
#include "CallibrationState.h"

#include "../dispatcher.h"
#include "../common.h"
#include "../lib_PAE2.h"

#define TITLE 0
#define NOPTIONS 3
#define CALIBRAR 0
#define MUSICA  1
#define MOVIMIENTO 2

uint8_t pointer; // apunta a una opción
RobotState menuState_g; // singleton

char *options[NOPTIONS];

void MenuState__init () {
    pointer = 0;
    options[CALIBRAR] = "+ CALIBRAR   ";
    options[MUSICA] = "  MUSICA     ";
    options[MOVIMIENTO] = "  MOVIMIENTO ";
    menuState_g.screen_changed = 1;
}
void MenuState__exit () {}

// update engine (se llama en el bucle principal)
void MenuState__update() {}

// screen
void MenuState__draw_screen () {
  //halLcdPrintLine("             ", TITLE, 0);
    options[CALIBRAR]   = (pointer==CALIBRAR)  ? "+ CALIBRAR   ": "  CALIBRAR   ";
    options[MUSICA]     = (pointer==MUSICA)    ?  "+ MUSICA     ": "  MUSICA     ";
    options[MOVIMIENTO] = (pointer==MOVIMIENTO)?"+ MOVIMIENTO ": "  MOVIMIENTO ";

    halLcdPrintLine("MENU         ", TITLE, 0);
    halLcdPrintLine(options[CALIBRAR], CALIBRAR+3, 0);
    halLcdPrintLine(options[MUSICA], MUSICA+3, 0);
    halLcdPrintLine(options[MOVIMIENTO], MOVIMIENTO+3, 0);

    menuState_g.screen_changed = 0;
}

// controls
void MenuState__s2_pressed () {}
void MenuState__s1_pressed () {
    RobotState *s = &menuState_g;
    switch (pointer) {
    case CALIBRAR:
        s = (CallibrationState());
        break;
    case MOVIMIENTO:
        s = (MovingState());
        break;
    case MUSICA:
        s = (MusicState());
        break;
    }
    set_state(s);
    //ctx_g->set_state(s);
}
void MenuState__up_pressed () {
    pointer = (pointer<1)?0:pointer-1;

    menuState_g.screen_changed = 1;
}
void MenuState__down_pressed () {
    pointer = (pointer+1>=NOPTIONS)?pointer:pointer+1;

    menuState_g.screen_changed = 1;
}
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
