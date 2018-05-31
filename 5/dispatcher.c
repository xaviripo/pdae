#include "dispatcher.h"

#include "msp.h"
#include "lib_PAE2.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"
#include "states/MenuState.h"

#define DEFAULT_THR 40

RobotState *state_g;
StateContext ctx_g;
//thresholds
uint8_t thrleft_g, thrfront_g, thrright_g;

void set_state(RobotState *s) {
    state_g->exit();
    state_g = s;
    state_g->init();
    halLcdClearScreen(0);
}

uint8_t get_thr_left() {
    return ctx_g.thr_left;
}
uint8_t get_thr_front() {
    return ctx_g.thr_front;
}
uint8_t get_thr_right() {
    return ctx_g.thr_right;
}

void set_thr_left(uint8_t value) {
    ctx_g.thr_left = value;
}
void set_thr_front(uint8_t value) {
    ctx_g.thr_front = value;
}
void set_thr_right(uint8_t value) {
    ctx_g.thr_right = value;
}


void init(void) {

    // Initializations
    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer
    init_ucs_24MHz(); // Inicalizar UCS

    // Fase 0
    init_timers_0();
    init_controls_0();
    init_comm_0();

    // Fase 1
    init_timers_1();
    init_controls_1();
    init_comm_1();

    // Fase 2
    //Habilitamos las interrupciones a nivel global del micro.
    __enable_interrupt();

    // Pantalla
    halLcdInit();
    halLcdClearScreen(0);

    ctx_g.thr_front = DEFAULT_THR;
    ctx_g.thr_left = DEFAULT_THR;
    ctx_g.thr_right = DEFAULT_THR;
}


int main() {
    init();
    state_g = MenuState(&ctx_g);
    state_g->init();
    while (1) {
        state_g->update();
        if (state_g->screen_changed) {
            state_g->draw_screen();
        }
    }

    return 0;
}

/******************************************************************************/
// HANDLERS
/******************************************************************************/

void s2_pressed(void) {
    state_g->s2_pressed();
}

void s1_pressed(void) {
    state_g->s1_pressed();
}

void up_pressed(void) {
    state_g->up_pressed();
}

void down_pressed(void) {
    state_g->down_pressed();
}

void left_pressed(void) {
    state_g->left_pressed();
}

void right_pressed(void) {
    state_g->right_pressed();
}

void center_pressed(void) {
   state_g->down_pressed();
}
