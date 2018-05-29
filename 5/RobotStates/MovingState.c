#include "MovingState.h"

#include "MenuState.h"
#include <stdio.h>
#include "robot.h"
#include "dispatcher.h"

#define LINE_STATE 0
#define LINE_SPEED 2
#define LINE_OBSTACLE 3
#define LINE_THRESHOLD 4
#define LINE_WALL 5
#define LINE_DEBUG 6

#define SPEED 300 // base speed

// Enum para la máquina de estados del movimiento del robot
typedef enum {
    STOP,                   // no hace nada
    FOLLOW,                 // se mantiene a distancia cte de la pared
    COLLISION,        // gira en sentido canónico (left wall -> cw)
    LOSS,             // gira en sentido no canónico (left wall -> ccw)
} state_t;

// Para almacenar el input de los sensores
sensor_distance_t sd_g;
bool stop_g; // flag for when the robot is paused
RobotState movingState_g; // singleton

uint8_t sensor_wall, sensor_front, sensor_wallnt; // valor de los sensores
uint8_t threshold_wall, threshold_front, threshold_wallnt;
bool obstacle_wall, obstacle_front, obstacle_wallnt; // flag: true => hay obstaculo


// Pared a reseguir (izda o dcha)
side_t wall;
char cadena[16]; // buffer used to write on screen;

void read_sensors(void) {
    sd_g = read_obstacle_distance();

    sensor_wall = (wall == LEFT ? sd_g.left : sd_g.right);
    sensor_front = sd_g.center;
    sensor_wallnt = (wall == LEFT ? sd_g.right : sd_g.left);

    obstacle_wall = sensor_wall > threshold_wall;
    obstacle_front = sensor_front > threshold_front;
    obstacle_wallnt = sensor_wallnt > threshold_wallnt;
}

bool rotate_wall(bool direction, uint16_t speed) {
    return wall == LEFT ? rotate_left(direction, speed) : rotate_right(direction, speed);
}

bool rotate_wallnt(bool direction, uint16_t speed) {
    return wall == LEFT ? rotate_right(direction, speed) : rotate_left(direction, speed);
}

state_t calculate_state() {
    if (stop_g) {
       return STOP;
    }
    read_sensors();

    if (obstacle_wall && obstacle_front && obstacle_wallnt) {
       return COLLISION;
       //state = DEAD_END;
    } else if (!obstacle_wall && !obstacle_front) {
        return LOSS;
    } else if (obstacle_wall && !obstacle_front) {
       return FOLLOW;
    } else {
       return COLLISION;
    }
}

void handle_state(state_t s) {
    int16_t diff;
    int32_t speed_wall, speed_wallnt;


    // Actuamos según el estado
       switch (s) {

       case STOP: // no hace nada
           halLcdPrintLine("STOP           ", LINE_STATE, NORMAL_TEXT);
           rotate_left(FORWARD, 0);
           rotate_right(FORWARD, 0);
           break;
       case FOLLOW: // se mantiene a distancia cte de la pared
           halLcdPrintLine("FOLLOW         ", LINE_STATE, NORMAL_TEXT);

           diff = sensor_wall - threshold_wall;

           speed_wall = SPEED + diff*2;

           speed_wallnt = SPEED - diff*2;

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

       case COLLISION: // gira en sentido canónico (wall -> cw)
           halLcdPrintLine("COLLISION      ", LINE_STATE, NORMAL_TEXT);
           diff = sensor_front - threshold_front;
           speed_wall = SPEED + (diff>0?diff:-diff);

           rotate_left(FORWARD, wall == LEFT ? SPEED : 0);
           rotate_right(FORWARD, wall == LEFT ? 0 : SPEED);

           do { // Seguimos girando hasta que no se vea NADA delante
               read_sensors();
           } while (sensor_front != 0 && !obstacle_wall);

           break;

       case LOSS: // gira en sentido no canónico (walln't -> ccw)
           halLcdPrintLine("LOSS           ", LINE_STATE, NORMAL_TEXT);

           rotate_left(FORWARD, wall == LEFT ? 0 : SPEED);
           rotate_right(FORWARD, wall == LEFT ? SPEED : 0);

           break;
       }
}

void MovingState__init () {
    wall = LEFT; // arbitrario
    handle_state(FOLLOW);

    // set thresholds
    threshold_wall = get_thr_left();
    threshold_front = get_thr_front();
    threshold_wallnt = get_thr_right();
}
void MovingState__exit () {}

// update engine (se llama en el bucle principal)
void MovingState__update() {
    handle_state(calculate_state());
}

// screen
void MovingState__draw_screen () {
    // sensor values
    sprintf(cadena, "V %03d %03d %03d", sd_g.left, sd_g.center, sd_g.right);
    halLcdPrintLine("  LFT CTR RGT", LINE_SPEED-1, NORMAL_TEXT);
    halLcdPrintLine(cadena, LINE_SPEED, NORMAL_TEXT);

    if (wall == LEFT) {
        sprintf(cadena, "O %01d   %01d   %01d", sd_g.left > threshold_wall, sd_g.center > threshold_front, sd_g.right > threshold_wallnt);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    } else {
        sprintf(cadena, "O %01d   %01d   %01d", sd_g.left > threshold_wallnt, sd_g.center > threshold_front, sd_g.right > threshold_wall);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    }
}

// controls
void MovingState__s2_pressed () {
    // S2 -> RETURN TO MENU
    setState(MenuState());
}
void MovingState__s1_pressed () {
    // S1 -> PAUSE
    stop_g = !stop_g;

}
void MovingState__up_pressed () {}
void MovingState__down_pressed () {}

void MovingState__left_pressed () {
    // Following LEFT wall
    wall = LEFT;
    threshold_wall   = get_thr_left();
    threshold_wallnt = get_thr_right();
}
void MovingState__right_pressed () {
    // Following RIGHT wall
    wall = RIGHT;
    threshold_wallnt = get_thr_left();
    threshold_wall   = get_thr_right();
}
void MovingState__center_pressed () {}

RobotState *MovingState() {
    movingState_g.init = &MovingState__init;
    movingState_g.exit = &MovingState__exit;
    movingState_g.update = &MovingState__update;
    movingState_g.draw_screen = &MovingState__draw_screen;
    movingState_g.s2_pressed = &MovingState__s2_pressed;
    movingState_g.s1_pressed = &MovingState__s1_pressed;
    movingState_g.up_pressed = &MovingState__up_pressed;
    movingState_g.down_pressed = &MovingState__down_pressed;
    movingState_g.left_pressed = &MovingState__left_pressed;
    movingState_g.right_pressed = &MovingState__right_pressed;
    movingState_g.left_pressed = &MovingState__left_pressed;
    movingState_g.center_pressed = &MovingState__center_pressed;
    return &movingState_g;
}


