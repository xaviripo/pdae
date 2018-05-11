/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "robot.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"
#include "msp.h"



/******************************************************************************/
// GLOBALS
/******************************************************************************/

uint8_t stop_g = 1;



/******************************************************************************/
// HELPERS
/******************************************************************************/

time_t delay_backwards(uint8_t hits, time_t base_time, time_t multiplier_time) {
    return base_time + hits*hits*multiplier_time;
}

/******************************************************************************/
// MAIN
/******************************************************************************/

void inits(void) {

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

}

void main(void)
{

    // Hacer que las cosas funcionen
    inits();

    // Settings
    set_obstacle_threshold(255);

    // Main loop
    uint8_t obstacle;
    uint8_t obstacle_memory = 0;
    uint8_t speed = 200;
    time_t time_rotate = 350;
    time_t base_time = 200;
    time_t multiplier_time = 20;

    // Initial state
    write_led(LEFT, 0);
    write_led(RIGHT, 0);
    rotate_left(FORWARD, speed);
    rotate_right(FORWARD, speed);

    // N�mero de hits en el sensor del centro seguidos
    uint8_t forward_hits = 0;

    sensor_distance sd;

    set_timer_interrupt(1);
    for (;;) {

        if (stop_g) {
            rotate_left(FORWARD, 0);
            rotate_right(FORWARD, 0);
            continue;
        }

        sd = read_obstacle_distance();

        write_led(RIGHT, sd.right > sd.center ? 1 : 0);
        write_led(LEFT, sd.left > sd.center ? 1 : 0);

//        obstacle = read_obstacle();
//
//        switch (obstacle) {
//
//        case 0:
//            rotate_left(FORWARD, speed);
//            rotate_right(FORWARD, speed);
//            break;
//
//        case OBSTACLE_LEFT:
//
//            forward_hits = 0;
//
//            // Si tiene obs a la izda
//            if (obstacle_memory & OBSTACLE_LEFT) {
//
//                rotate_left(FORWARD, speed);
//                rotate_right(BACKWARD, speed);
//
//                reset_time();
//                while (!has_passed(time_rotate));
//
//                obstacle_memory &= ~OBSTACLE_LEFT;
//            }
//
//            rotate_left(FORWARD, speed);
//            rotate_right(FORWARD, speed);
//
//            break;
//
//        case OBSTACLE_RIGHT:
//
//            forward_hits = 0;
//
//            // Si tiene obs a la dcha
//            if (obstacle_memory & OBSTACLE_RIGHT) {
//
//                rotate_left(BACKWARD, speed);
//                rotate_right(FORWARD, speed);
//
//                reset_time();
//                while (!has_passed(time_rotate));
//                obstacle_memory &= ~OBSTACLE_RIGHT;
//            }
//
//            rotate_left(FORWARD, speed);
//            rotate_right(FORWARD, speed);
//
//            break;
//
//        case OBSTACLE_LEFT | OBSTACLE_RIGHT:
//
//            forward_hits = 0;
//
//            rotate_left(FORWARD, speed);
//            rotate_right(FORWARD, speed);
//            obstacle_memory = 0;
//            break;
//
//        case OBSTACLE_CENTER:
//            forward_hits++;
//
//            rotate_left(BACKWARD, speed);
//            rotate_right(BACKWARD, speed);
//
//            reset_time();
//            while (!has_passed(delay_backwards(forward_hits, base_time, multiplier_time)));
//
//            rotate_left(FORWARD, speed);
//            rotate_right(BACKWARD, speed);
//
//            break;
//
//        case OBSTACLE_CENTER | OBSTACLE_LEFT:
//            forward_hits++;
//
//            rotate_left(BACKWARD, speed);
//            rotate_right(BACKWARD, speed);
//
//            reset_time();
//            while (!has_passed(delay_backwards(forward_hits, base_time, multiplier_time)));
//
//            rotate_left(FORWARD, speed);
//            rotate_right(BACKWARD, speed);
//
//            obstacle_memory |= OBSTACLE_LEFT;
//
//            break;
//
//        case OBSTACLE_CENTER | OBSTACLE_RIGHT:
//            forward_hits++;
//
//            rotate_left(BACKWARD, speed);
//            rotate_right(BACKWARD, speed);
//
//
//            reset_time();
//            while (!has_passed(delay_backwards(forward_hits, base_time, multiplier_time)));
//
//            rotate_left(BACKWARD, speed);
//            rotate_right(FORWARD, speed);
//
//            obstacle_memory |= OBSTACLE_RIGHT;
//
//            break;
//
//        case OBSTACLE_LEFT | OBSTACLE_CENTER | OBSTACLE_RIGHT:
//            rotate_left(FORWARD, 0);
//            rotate_right(FORWARD, 0);
//            break;
//
//        default:
//            // Nunca deber�a entrar aqu�
//            rotate_left(FORWARD, 0);
//            rotate_right(FORWARD, 0);
//            break;
//
//        }

    }

}



/******************************************************************************/
// HANDLERS
/******************************************************************************/

void s1_pressed(void) {
    stop_g = !stop_g;
}
