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
    set_obstacle_threshold(20);

    // Main loop
    uint8_t obstacle;
    uint16_t speed = 300;

    sensor_distance sd;

    /**
     * 0 -> buscando pared
     * 1 -> hemos detectado pared, rotar hasta que este perpendicular
     * 2 -> seguir pared a distancia fija
     * 3 -> la pared se ha cortado bruscamente, hay que volver a ella
     * 4 -> se encuentra con pared por delante
     */
    uint8_t state = 0;

    /**
     * which wall are we following
     * 0 -> left
     * 1 -> right
     */
    side_t wall;

    // Minimum measured distance on the right/left sensors
    uint8_t min_distance = 0;

    // Used in the loop
    uint8_t sensor;
    int16_t diff, speed_left, speed_right;

    // Initial state
    write_led(LEFT, 0);
    write_led(RIGHT, 0);
    rotate_left(FORWARD, speed);
    rotate_right(FORWARD, speed);

    rotate_left(FORWARD, 0);
    rotate_right(FORWARD, 0);
    while (stop_g);

    set_timer_interrupt(1);
    for (;;) {
        switch (state) {

        case 0:
            obstacle = read_obstacle();
            if (obstacle) {
                state = 1;
                rotate_right(FORWARD, 0);
                rotate_left(FORWARD, 0);
            }
            wall = obstacle & OBSTACLE_LEFT ? LEFT : RIGHT;
            break;

        case 1:
            sd = read_obstacle_distance();
            sensor = (wall == RIGHT ? sd.right : sd.left);

            if (sensor >= min_distance) {
                min_distance = (wall == RIGHT ? sd.right : sd.left);
                rotate_right(wall == RIGHT ? FORWARD : BACKWARD, speed);
                rotate_left(wall == RIGHT ? BACKWARD : FORWARD, speed);
            } else {
                state = 2;
            }
            break;

        case 2:
            sd = read_obstacle_distance();
            sensor = (wall == RIGHT ? sd.right : sd.left);

            if (sensor == 0) {
                state = 3;
                break;
            }

            if (sd.center > min_distance) {
                state = 4;
                break;
            }

            diff = sensor - min_distance;

            speed_left = speed - diff;
            if (speed_left < 0) speed_left = 0;
            if (speed_left > 1022) speed_left = 1022;

            speed_right = speed + diff;
            if (speed_right < 0) speed_right = 0;
            if (speed_right > 1022) speed_right = 1022;

            rotate_left(FORWARD, wall == RIGHT ? speed_left : speed_right);
            rotate_right(FORWARD, wall == RIGHT ? speed_right : speed_left);
            break;

        case 3:
            sd = read_obstacle_distance();
            sensor = (wall == RIGHT ? sd.right : sd.left);

            if (sensor != 0) {
                state = 2;
                break;
            }

            rotate_left(FORWARD, wall == RIGHT ? speed + 255 : 0);
            rotate_right(FORWARD, wall == RIGHT ? 0 : speed + 255);


            break;

        case 4:
            sd = read_obstacle_distance();
            sensor = (wall == RIGHT ? sd.right : sd.left);

            if (sd.center < min_distance) {
                state = 2;
                break;
            }

            rotate_left(wall == RIGHT ? BACKWARD : FORWARD, speed);
            rotate_right(wall == RIGHT ? FORWARD : BACKWARD, speed);

            break;

        default:
            break;

        }




//        sd = read_obstacle_distance();
//
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
//            if (sd.left < min_distance) {
//                min_distance = sd.left;
//                rotate_left(FORWARD, speed);
//                rotate_right(BACKWARD, speed);
//            }
//            break;
//
//        }

//        switch (obstacle) {
//
//        case 0:
//            rotate_left(FORWARD, speed);
//            rotate_right(FORWARD, speed);
//            break;
//
//        case OBSTACLE_LEFT:
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
