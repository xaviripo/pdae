#include "MusicState.h"
#include "MenuState.h"

#include "../lib_PAE2.h"

#include "../robot/sound.h"

RobotState musicState_g; // singleton
StateContext* ctx_g;

#define TITLE 0
#define NOPTIONS 3
#define MEGALOVANIA 0
#define SUPERMARIO 1
#define WIICHNL 2

#define PLAYINH 4

uint8_t pointer;
bool playing;
char *options[];
char cadena[16];

void MusicState__init () {
    pointer = 0;
    playing = 0;

    options[MEGALOVANIA] = "+ MEGALOVANIA";
    options[SUPERMARIO]  = "+ SUPERMARIO ";
    options[WIICHNL]     = "+ WIICHANNEL ";
}
void MusicState__exit () {}

// update engine (se llama en el bucle principal)
void MusicState__update() {
    if (playing) {
        play_next_note();
    }
}

// screen
void MusicState__draw_screen () {
    //halLcdPrintLine("             ", TITLE, 0);
      options[MEGALOVANIA][0]   = (pointer==MEGALOVANIA)  ?'+':' ';
      options[SUPERMARIO][0]     = (pointer==SUPERMARIO)    ?'+':' ';
      options[WIICHNL][0] = (pointer==WIICHNL)?'+':' ';

      halLcdPrintLine("MUSIC        ", TITLE, 0);
      halLcdPrintLine(options[MEGALOVANIA], MEGALOVANIA, 0);
      halLcdPrintLine(options[SUPERMARIO], SUPERMARIO, 0);
      halLcdPrintLine(options[WIICHNL], WIICHNL, 0);

      halLcdPrintLine((playing)?"PLAYING":"PAUSED", WIICHNL, 0);

      musicState_g.screen_changed = 0;
}

// controls
void MusicState__s2_pressed () {
    // volver al menu principal
    ctx_g->set_state(MenuState(ctx_g));
}
void MusicState__s1_pressed () {
    // seleccionar la cancion
    switch (pointer) {
    case MEGALOVANIA:
        play_megalo();
        break;
    case SUPERMARIO:
        play_smb();
        break;
    case WIICHNL:
        play_wii();
        break;
    }
}
void MusicState__up_pressed () {
    pointer = (pointer<1)?0:pointer-1;
}
void MusicState__down_pressed () {
    pointer = (pointer+1>=NOPTIONS)?pointer:pointer+1;
}
void MusicState__left_pressed () {}
void MusicState__right_pressed () {}
void MusicState__center_pressed () {
    playing = !playing;

    Sound__enable(playing);

    musicState_g.screen_changed = 1;
}

RobotState *MusicState(StateContext* ctx) {
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

    ctx_g = ctx;

    return &musicState_g;
}
