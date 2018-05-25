/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include <math.h>

#include "robot.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"
#include "hal/sound.h"
#include "msp.h"
#include "lib_PAE2.h"



/******************************************************************************/
// ENUMS
/******************************************************************************/

// Enum para la m√°quina de estados del movimiento del robot
typedef enum {
    STOP,                   // no hace nada
    CALLIBRATE,             // calibrara el robot
    FOLLOW,                 // se mantiene a distancia cte de la pared
    COLLISION,        // gira en sentido can√≥nico (left wall -> cw)
    LOSS,             // gira en sentido no can√≥nico (left wall -> ccw)
    DEAD_END          // retrocede en marcha atr√°s pq no puede girar
} state_t;

#define LINE_STATE 0
#define LINE_SPEED 2
#define LINE_OBSTACLE 3
#define LINE_THRESHOLD 4
#define LINE_WALL 5
#define LINE_DEBUG 6

#define BUFFER_SIZE 4

/******************************************************************************/
// GLOBALS
/******************************************************************************/
uint8_t stop_g = 1;
uint8_t callibrate_g = 0;

// Para almacenar el input de los sensores
sensor_distance sd;

uint8_t sensor_wall, sensor_front, sensor_wallnt;
bool obstacle_wall, obstacle_front, obstacle_wallnt;

// Main loop
uint8_t threshold_wall = 40;
uint8_t threshold_wallnt = 40;
uint8_t threshold_front = 40;

// Velocidad de crucero
uint16_t speed = 300;
int16_t diff = 0;
int32_t speed_wall, speed_wallnt;

// Estado en el que se encuentra el robot
state_t state = STOP;

// Pared a reseguir (izda o dcha)
side_t wall = LEFT;

// global to test melodies
int i = 0;

char cadena[16];


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

    // Pantalla
    halLcdInit();

}

void read_sensors(void) {

    sd = read_obstacle_distance();

    sensor_wall = (wall == LEFT ? sd.left : sd.right);
    sensor_front = sd.center;
    sensor_wallnt = (wall == LEFT ? sd.right : sd.left);

    obstacle_wall = sensor_wall > threshold_wall;
    obstacle_front = sensor_front > threshold_front;
    obstacle_wallnt = sensor_wallnt > threshold_wallnt;

    halLcdPrintLine("  LFT CTR RGT", LINE_SPEED-1, NORMAL_TEXT);

    sprintf(cadena, "V %03d %03d %03d", sd.left, sd.center, sd.right);
    halLcdPrintLine(cadena, LINE_SPEED, NORMAL_TEXT);

    if (wall == LEFT) {
        sprintf(cadena, "T %03d %03d %03d", threshold_wall, threshold_front, threshold_wallnt);
        halLcdPrintLine(cadena, LINE_THRESHOLD, NORMAL_TEXT);
        sprintf(cadena, "O %01d   %01d   %01d", sd.left > threshold_wall, sd.center > threshold_front, sd.right > threshold_wallnt);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    } else {
        sprintf(cadena, "T %03d %03d %03d", threshold_wallnt, threshold_front, threshold_wall);
        halLcdPrintLine(cadena, LINE_THRESHOLD, NORMAL_TEXT);
        sprintf(cadena, "O %01d   %01d   %01d", sd.left > threshold_wallnt, sd.center > threshold_front, sd.right > threshold_wall);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    }

}

void callibrate_sensors(void) {
    sd = read_obstacle_distance();

    if (wall == LEFT) { // Est· m·s cerca del muro izdo
        threshold_wall = sd.left;
        threshold_front = sd.center;
        threshold_wallnt = sd.right;
    } else {
        threshold_wall = sd.right;
        threshold_front = sd.center;
        threshold_wallnt = sd.left;
    }

    sprintf(cadena, "W %c", threshold_wall, wall==LEFT ? 'L' : 'R');
    halLcdPrintLine(cadena, LINE_WALL, NORMAL_TEXT);

}

bool rotate_wall(bool direction, uint16_t speed) {
    return wall == LEFT ? rotate_left(direction, speed) : rotate_right(direction, speed);
}

bool rotate_wallnt(bool direction, uint16_t speed) {
    return wall == LEFT ? rotate_right(direction, speed) : rotate_left(direction, speed);
}

void main(void)
{

    // Hacer que las cosas funcionen
    inits();

    // unused
    //set_obstacle_threshold(threshold);

    // Initial state
    write_led(LEFT, 0);
    write_led(RIGHT, 0);
    rotate_left(FORWARD, 0);
    rotate_right(FORWARD, 0);
    halLcdClearScreen(0/*?*/);

    for (;;) {

        // Leemos lo que hay fuera
        read_sensors();

        if (callibrate_g) {
            state = CALLIBRATE;
        } else if (stop_g) {
            state = STOP;
        } else if (obstacle_wall && obstacle_front && obstacle_wallnt) {
            state = DEAD_END;
        } else if (!obstacle_wall && !obstacle_front) {
            state = LOSS;
        } else if (obstacle_wall && !obstacle_front) {
            state = FOLLOW;
        } else {
            state = COLLISION;
        }

        // Actuamos seg√∫n el estado
        switch (state) {

        case STOP: // no hace nada
            halLcdPrintLine("STOP           ", LINE_STATE, NORMAL_TEXT);
            rotate_left(FORWARD, 0);
            rotate_right(FORWARD, 0);
            break;

        case CALLIBRATE:
            callibrate_sensors();
            callibrate_g = 0;
            break;

        case FOLLOW: // se mantiene a distancia cte de la pared
            halLcdPrintLine("FOLLOW         ", LINE_STATE, NORMAL_TEXT);

            uint16_t speed_tmp;

            diff = sensor_wall - threshold_wall;

            speed_wall = speed + diff;

            speed_wallnt = speed - diff;

            if (speed_wall < 0) {
                rotate_wall(BACKWARD, -speed_wall);
            } else {
                rotate_wall(FORWARD, speed_wall);
            }

            if (speed_wallnt < 0) {
                rotate_wallnt(BACKWARD, -speed_wallnt);
            } else {
                rotate_wallnt(FORWARD, speed_wallnt);
            }
            //rotate_left(FORWARD, 0);
            //rotate_right(FORWARD, 0);

            sprintf(cadena, "diff: %03d", diff);
            halLcdPrintLine(cadena, LINE_DEBUG, NORMAL_TEXT);

            break;

        case COLLISION: // gira en sentido can√≥nico (wall -> cw)
            halLcdPrintLine("COLLISION      ", LINE_STATE, NORMAL_TEXT);

//            diff = sensor_front - threshold_front;
//
//            speed_wall = speed + (diff>0?diff:-diff);


            rotate_left(FORWARD, wall == LEFT ? speed : 0);
            rotate_right(FORWARD, wall == LEFT ? 0 : speed);

            do { // Seguimos girando hasta que no se vea NADA delante
                read_sensors();
            } while (sensor_front != 0 && !obstacle_wall);

            break;

        case LOSS: // gira en sentido no can√≥nico (walln't -> ccw)
            halLcdPrintLine("LOSS           ", LINE_STATE, NORMAL_TEXT);

            rotate_left(FORWARD, wall == LEFT ? 0 : speed);
            rotate_right(FORWARD, wall == LEFT ? speed : 0);

            break;

        case DEAD_END: // callej√≥n sin salida
            halLcdPrintLine("DEAD_END 1     ", LINE_STATE, NORMAL_TEXT);

            Note n;
            n.duration = 5;
            n.pitch = SCALE*3 + MI;

            set_sec_timer_interrupt(1);
            reset_sec_time();
            play_note(n);
            uint8_t counter = 0;
            do { // Tira hacia atr·s "ajustando" la direcciÛn para no chocar contra los muros
                if (has_passed_sec(1000)) {
                    reset_sec_time();
                    stop_sound();
                    play_note(n);
                }
                read_sensors();

                diff = sensor_wall - sensor_wallnt;

                speed_wall = speed - diff;
                if (speed_wall < SPEED_MIN) speed_wall = SPEED_MIN;
                if (speed_wall > SPEED_MAX) speed_wall = SPEED_MAX;

                speed_wallnt = speed + diff;
                if (speed_wallnt < SPEED_MIN) speed_wallnt = SPEED_MIN;
                if (speed_wallnt > SPEED_MAX) speed_wallnt = SPEED_MAX;

                rotate_left(BACKWARD, wall == RIGHT ? speed_wall : speed_wallnt); // TODO comprobar si es wallnt : wall o al rev√©s
                rotate_right(BACKWARD, wall == RIGHT ? speed_wallnt : speed_wall);

                if (!(obstacle_wall || obstacle_wallnt)) counter++;

            } while (counter < 1);

            rotate_left(wall == LEFT ? FORWARD : BACKWARD, speed);
            rotate_right(wall == LEFT ? BACKWARD : FORWARD, speed);

            halLcdPrintLine("DEAD_END 2     ", LINE_STATE, NORMAL_TEXT);

            reset_sec_time();
            play_note(n);
            do { // Giramos en sentido canÛnico hasta estar "paralelo" al deadend
                if (has_passed_sec(1000)) {
                    reset_sec_time();
                    stop_sound();
                    play_note(n);
                }
                read_sensors();
            } while (!obstacle_wall);

            halLcdPrintLine("DEAD_END 3     ", LINE_STATE, NORMAL_TEXT);

            reset_sec_time();
            play_note(n);
            counter = 0;
            do { // Seguimos girando para compensar que a˙n no estÈ paralelo
                if (has_passed_sec(1000)) {
                    reset_sec_time();
                    stop_sound();
                    play_note(n);
                    counter++;
                }
                read_sensors();
            } while (obstacle_wall || counter < 1);


            break;
        }

    }

}



/******************************************************************************/
// HANDLERS
/******************************************************************************/

void s2_pressed(void) {
    callibrate_g = 1;
}

void s1_pressed(void) {
    stop_g = !stop_g;
}

void up_pressed(void) {

}

void down_pressed(void) {

}

void left_pressed(void) {
    wall = RIGHT; // god left
}

void right_pressed(void) {
    wall = LEFT;
}

void center_pressed(void) {}
