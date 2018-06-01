#include "lib_PAE2.h"
#include "dispatcher.h"
#include "robot/sound.h"

#include "MusicState.h"
#include "MenuState.h"

RobotState musicState_g; // singleton
StateContext* ctx_g;

#define TITLE 0
#define NOPTIONS 4
#define MEGALOVANIA 0
#define SUPERMARIO 1
#define WIICHNL 2
#define PORTAL 3

#define PLAYINH 4

uint8_t pointer;
bool playing;
char *options[NOPTIONS];
char cadena[16];

void MusicState__init () {
    pointer = 0;
    playing = 0;

    options[MEGALOVANIA] = "+ MEGALOVANIA";
    options[SUPERMARIO]  = "  SUPERMARIO ";
    options[WIICHNL]     = "  WIICHANNEL ";
    options[PORTAL]      = "  PORTAL     ";

    musicState_g.screen_changed = 1;
}
void MusicState__exit () {

}

// update engine (se llama en el bucle principal)
void MusicState__update() {
    if (playing) {
        play_next_note();
    }
}

// screen
void MusicState__draw_screen () {
    //halLcdPrintLine("             ", TITLE, 0);
      options[MEGALOVANIA]   = (pointer==MEGALOVANIA)  ?"+ MEGALOVANIA":"  MEGALOVANIA";
      options[SUPERMARIO]     = (pointer==SUPERMARIO)    ?"+ SUPERMARIO ":"  SUPERMARIO ";
      options[WIICHNL] = (pointer==WIICHNL)? "+ WIICHANNEL ": "  WIICHANNEL ";
      options[PORTAL] = (pointer==PORTAL)? "+ PORTAL     ": "  PORTAL     ";

      halLcdPrintLine("MUSIC        ", TITLE, 0);
      halLcdPrintLine(options[MEGALOVANIA], MEGALOVANIA+3, 0);
      halLcdPrintLine(options[SUPERMARIO], SUPERMARIO+3, 0);
      halLcdPrintLine(options[WIICHNL], WIICHNL+3, 0);
      halLcdPrintLine(options[PORTAL], PORTAL+3, 0);

      halLcdPrintLine((playing)?"PLAYING":"PAUSED", WIICHNL, 0);

      musicState_g.screen_changed = 0;
}

// controls
void MusicState__s2_pressed () {
    // volver al menu principal
    set_state(MenuState(ctx_g));
    //ctx_g->set_state(MenuState(ctx_g));
}
void MusicState__s1_pressed () {
    if (playing) {
        playing = 0;

        Sound__enable(0);

        musicState_g.screen_changed = 1;
    }


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
    case PORTAL:
        play_portal();
        break;
    }
    playing=1;
    Sound__enable(0);
    musicState_g.screen_changed = 1;
}
void MusicState__up_pressed () {
    pointer = (pointer<1)?0:pointer-1;

    musicState_g.screen_changed = 1;
}
void MusicState__down_pressed () {
    pointer = (pointer+1>=NOPTIONS)?pointer:pointer+1;

    musicState_g.screen_changed = 1;
}
void MusicState__left_pressed () {}
void MusicState__right_pressed () {}
void MusicState__center_pressed () {

}

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
    musicState_g.center_pressed = &MusicState__center_pressed;

    return &musicState_g;
}
