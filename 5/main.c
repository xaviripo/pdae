/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include <math.h>

#include "robot.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"
#include "msp.h"
#include "lib_PAE2.h"



/******************************************************************************/
// ENUMS
/******************************************************************************/

// Enum para la mÃ¡quina de estados del movimiento del robot
typedef enum {
    STOP,                   // no hace nada
    CALLIBRATE,             // calibrara el robot
    FOLLOW,                 // se mantiene a distancia cte de la pared
    COLLISION,        // gira en sentido canÃ³nico (left wall -> cw)
    LOSS,             // gira en sentido no canÃ³nico (left wall -> ccw)
    DEAD_END          // retrocede en marcha atrÃ¡s pq no puede girar
} state_t;

#define LINE_STATE 0
#define LINE_SPEED 2
#define LINE_OBSTACLE 3
#define LINE_THRESHOLD 4
#define LINE_DEBUG 5

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
uint8_t threshold = 40;

// Velocidad de crucero
uint16_t speed = 500;
int16_t diff = 0;
int32_t speed_wall, speed_wallnt;

// Estado en el que se encuentra el robot
state_t state = STOP;

// Pared a reseguir (izda o dcha)
side_t wall = LEFT;

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

    obstacle_wall = sensor_wall > threshold;
    obstacle_front = sensor_front > threshold;
    obstacle_wallnt = sensor_wallnt > threshold;

    halLcdPrintLine("LFT CTR RGT", LINE_SPEED-1, NORMAL_TEXT);

    sprintf(cadena, "%03d %03d %03d", sd.left, sd.center, sd.right);
    halLcdPrintLine(cadena, LINE_SPEED, NORMAL_TEXT);

    sprintf(cadena, "%01d   %01d   %01d", sd.left > threshold, sd.center > threshold, sd.right > threshold);
    halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);

}

void callibrate_sensors(void) {
    sd = read_obstacle_distance();

    if (sd.left > sd.right) { // Está más cerca del muro izdo
        wall = LEFT;
        threshold = sd.left;
    } else {
        wall = RIGHT;
        threshold = sd.right;
    }

    sprintf(cadena, "Threshold: %03d", threshold);
    halLcdPrintLine(cadena, LINE_THRESHOLD, NORMAL_TEXT);

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

    set_obstacle_threshold(threshold);

    // Initial state
    write_led(LEFT, 0);
    write_led(RIGHT, 0);
    rotate_left(FORWARD, 0);
    rotate_right(FORWARD, 0);

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

        // Actuamos segÃºn el estado
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

            diff = sensor_wall - threshold;

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

        case COLLISION: // gira en sentido canÃ³nico (wall -> cw)
            halLcdPrintLine("COLLISION      ", LINE_STATE, NORMAL_TEXT);

            rotate_left(FORWARD, wall == LEFT ? speed : 0);
            rotate_right(FORWARD, wall == LEFT ? 0 : speed);

            do { // Seguimos girando hasta que no se vea NADA delante
                read_sensors();
            } while (sensor_front != 0 && !obstacle_wall);

            break;

        case LOSS: // gira en sentido no canÃ³nico (walln't -> ccw)
            halLcdPrintLine("LOSS           ", LINE_STATE, NORMAL_TEXT);

            rotate_left(FORWARD, wall == LEFT ? 0 : speed);
            rotate_right(FORWARD, wall == LEFT ? speed : 0);

            break;

        case DEAD_END: // callejÃ³n sin salida
            halLcdPrintLine("DEAD_END       ", LINE_STATE, NORMAL_TEXT);

            diff = sensor_wall - sensor_wallnt;

            speed_wall = speed - diff;
            if (speed_wall < SPEED_MIN) speed_wall = SPEED_MIN;
            if (speed_wall > SPEED_MAX) speed_wall = SPEED_MAX;

            speed_wallnt = speed + diff;
            if (speed_wallnt < SPEED_MIN) speed_wallnt = SPEED_MIN;
            if (speed_wallnt > SPEED_MAX) speed_wallnt = SPEED_MAX;

            rotate_left(BACKWARD, wall == RIGHT ? speed_wall : speed_wallnt); // TODO comprobar si es wallnt : wall o al revÃ©s
            rotate_right(BACKWARD, wall == RIGHT ? speed_wallnt : speed_wall);

            do { // Tira hacia atrás "ajustando" la dirección para no chocar contra los muros
                read_sensors();
            } while (obstacle_wall && obstacle_wallnt);

            rotate_left(wall == LEFT ? FORWARD : BACKWARD, speed);
            rotate_right(wall == LEFT ? BACKWARD : FORWARD, speed);

            do { // Giramos en sentido canónico hasta estar "paralelo" al deadend
                read_sensors();
            } while (!obstacle_wall);

            do { // Seguimos girando para compensar que aún no esté paralelo
                read_sensors();
            } while (obstacle_wall);

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
