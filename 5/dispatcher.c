#include "dispatcher.h"

#include "msp.h"
#include "lib_PAE2.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"

RobotState *state_g;
//thresholds
uint8_t thrleft_g, thrfront_g, thrright_g;

void setState(RobotState *s) {
    state_g->exit();
    state_g = s;
    state_g->init();
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
}

void set_thresholds(uint8_t left, uint8_t front, uint8_t right) {
    thrleft_g = left;
    thrfront_g = front;
    thrright_g = right;
}

uint8_t get_thr_left() {
    return thrleft_g;
}

uint8_t get_thr_front() {
    return thrfront_g;
}

uint8_t get_thr_right() {
    return thrright_g;
}


int main() {
    while (1) {
        state_g->update();
        state_g->draw_screen();
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
