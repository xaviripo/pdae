/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "robot.h"
#include "hal/timers.h"
#include "hal/controls.h"
#include "hal/communication.h"
#include "msp.h"



/******************************************************************************/
// ENUMS
/******************************************************************************/

// Enum para la máquina de estados del movimiento del robot
typedef enum {
    STOP,                   // no hace nada
    FOLLOW,                 // se mantiene a distancia cte de la pared
    COLLISION,        // gira en sentido canónico (left wall -> cw)
    LOSS,             // gira en sentido no canónico (left wall -> ccw)
    DEAD_END          // retrocede en marcha atrás pq no puede girar
} state_t;



/******************************************************************************/
// GLOBALS
/******************************************************************************/

uint8_t stop_g = 1;

// Para almacenar el input de los sensores
sensor_distance sd;

uint8_t sensor_wall, sensor_front, sensor_wallnt;
bool obstacle_wall, obstacle_front, obstacle_wallnt;

// Main loop
uint8_t threshold = 20;

// Velocidad de crucero
uint16_t speed = 300;
int16_t diff = 0;
uint16_t speed_wall, speed_wallnt;

// Estado en el que se encuentra el robot
state_t state = STOP;

// Pared a reseguir (izda o dcha)
side_t wall;


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

void read_sensors(void) {
    sd = read_obstacle_distance();

    sensor_wall = (wall == LEFT ? sd.left : sd.right);
    sensor_front = sd.center;
    sensor_wallnt = (wall == LEFT ? sd.right : sd.left);

    obstacle_wall = sensor_wall > threshold;
    obstacle_front = sensor_front > threshold;
    obstacle_wallnt = sensor_wallnt > threshold;
}

void main(void)
{

    // Hacer que las cosas funcionen
    inits();

    bool front_seen;
    set_obstacle_threshold(threshold);

    // Initial state
    write_led(LEFT, 0);
    write_led(RIGHT, 0);
    rotate_left(FORWARD, 0);
    rotate_right(FORWARD, 0);

    for (;;) {

        // Leemos lo que hay fuera
        read_sensors();

        // Decidimos el estado mediante los inputs (máquina de Moore)
        if (stop_g) {
            state = STOP;
        } else if (obstacle_wall && obstacle_front && obstacle_wallnt) {
            // Wally wally wally no scapey
            state = DEAD_END;
        } else if (sensor_wall == 0) {
            // Loss
            state = LOSS;
        } else if (obstacle_wall && obstacle_front) {
            // Collision
            state = COLLISION;
        } else {
            // Follow
            state = FOLLOW;
        }

        // Actuamos según el estado
        switch (state) {

        case STOP: // no hace nada
            rotate_left(FORWARD, 0);
            rotate_right(FORWARD, 0);
            break;

        case FOLLOW: // se mantiene a distancia cte de la pared

            diff = sensor_wall - threshold;

            speed_wall = speed - diff*diff;
            if (speed_wall < SPEED_MIN) speed_wall = SPEED_MIN;
            if (speed_wall > SPEED_MAX) speed_wall = SPEED_MAX;

            speed_wallnt = speed + diff*diff;
            if (speed_wallnt < SPEED_MIN) speed_wallnt = SPEED_MIN;
            if (speed_wallnt > SPEED_MAX) speed_wallnt = SPEED_MAX;

            rotate_left(FORWARD, wall == RIGHT ? speed_wall : speed_wallnt);
            rotate_right(FORWARD, wall == RIGHT ? speed_wallnt : speed_wall);

            break;

        case COLLISION: // gira en sentido canónico (left wall -> cw)

            rotate_left(FORWARD, wall == RIGHT ? speed : 0); // TODO poner una velocidad chuli
            rotate_right(FORWARD, wall == RIGHT ? 0 : speed);

            break;

        case LOSS: // gira en sentido no canónico (left wall -> ccw)

            rotate_left(wall == RIGHT ? BACKWARD : FORWARD, speed);
            rotate_right(wall == RIGHT ? FORWARD : BACKWARD, speed);

            break;

        case DEAD_END: // callejón sin salida

            diff = sensor_wall - sensor_wallnt;

            speed_wall = speed - diff;
            if (speed_wall < SPEED_MIN) speed_wall = SPEED_MIN;
            if (speed_wall > SPEED_MAX) speed_wall = SPEED_MAX;

            speed_wallnt = speed + diff;
            if (speed_wallnt < SPEED_MIN) speed_wallnt = SPEED_MIN;
            if (speed_wallnt > SPEED_MAX) speed_wallnt = SPEED_MAX;

            while (obstacle_wall && obstacle_wallnt) {
                read_sensors();
                rotate_left(BACKWARD, wall == RIGHT ? speed_wallnt : speed_wall); // TODO comprobar si es wallnt : wall o al revés
                rotate_right(BACKWARD, wall == RIGHT ? speed_wall : speed_wallnt);
            }

            front_seen = 0;

            while (!obstacle_wallnt && !front_seen) {
                rotate_left(wall == RIGHT ? FORWARD : BACKWARD, speed);
                rotate_right(wall == RIGHT ? BACKWARD : FORWARD, speed);

                read_sensors();
                front_seen = front_seen || obstacle_front;
            }

            break;
        }

    }

}



/******************************************************************************/
// HANDLERS
/******************************************************************************/

void s1_pressed(void) {
    stop_g = !stop_g;
}
