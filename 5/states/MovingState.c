#include <stdio.h>
#include <string.h>

#include "dispatcher.h"
#include "robot/robot.h"

#include "states/MovingState.h"
#include "states/MenuState.h"




#define TITLE 0
#define LINE_STATE 1
#define LINE_DIRS 2
#define LINE_SENSORS 3
#define LINE_OBSTACLE 4
#define LINE_THRESHOLD 5
#define LINE_SPEED 6

#define SPEED_INC 10

// Enum para la máquina de estados del movimiento del robot
typedef enum {
    STOP,                   // no hace nada
    FOLLOW,                 // se mantiene a distancia cte de la pared
    COLLISION,        // gira en sentido canónico (left wall -> cw)
    LOSS,             // gira en sentido no canónico (left wall -> ccw)
} state_t;

uint16_t speed_g = 300; // base speed
bool initialized = 0; // wheels initialized?

// Para almacenar el input de los sensores
sensor_distance_t sd_g;
bool stop_g; // flag for when the robot is paused
RobotState movingState_g; // singleton
bool exit = 0; // have to exit?

uint8_t sensor_wall, sensor_front, sensor_wallnt; // valor de los sensores
uint8_t threshold_wall, threshold_front, threshold_wallnt;
bool obstacle_wall, obstacle_front, obstacle_wallnt; // flag: true => hay obstaculo


// Pared a reseguir (izda o dcha)
side_t wall;
state_t prev_state;
char cadena[16]; // buffer used to write on screen;
char cadena_state[16];

void read_sensors(void) {
    sd_g = read_obstacle_distance();

    sensor_wall = (wall == LEFT ? sd_g.left : sd_g.right);
    sensor_front = sd_g.center;
    sensor_wallnt = (wall == LEFT ? sd_g.right : sd_g.left);

    obstacle_wall = sensor_wall > threshold_wall;
    obstacle_front = sensor_front > threshold_front;
    obstacle_wallnt = sensor_wallnt > threshold_wallnt;

    movingState_g.screen_changed = 1; // update values on screen
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

    if (s != prev_state) movingState_g.screen_changed = 1;
    // Actuamos según el estado
       switch (s) {

       case STOP: // no hace nada
           strcpy(cadena_state,"STOP           ");
           stop_movement();
           break;
       case FOLLOW: // se mantiene a distancia cte de la pared
           strcpy(cadena_state,"FOLLOW         ");

           diff = sensor_wall - threshold_wall;

           speed_wall = speed_g + diff*2;

           speed_wallnt = speed_g - diff*2;

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

           break;

       case COLLISION: // gira en sentido canónico (wall -> cw)
           halLcdPrintLine("COLLISION      ", LINE_STATE, NORMAL_TEXT);
           diff = sensor_front - threshold_front;
           speed_wall = speed_g + (diff>0?diff:-diff);

           rotate_left(FORWARD, wall == LEFT ? speed_g : 0);
           rotate_right(FORWARD, wall == LEFT ? 0 : speed_g);

           do { // Seguimos girando hasta que no se vea NADA delante
               read_sensors();
           } while (sensor_front != 0 && !obstacle_wall);

           break;

       case LOSS: // gira en sentido no canónico (walln't -> ccw)
           halLcdPrintLine("LOSS           ", LINE_STATE, NORMAL_TEXT);

           rotate_left(FORWARD, wall == LEFT ? 0 : speed_g);
           rotate_right(FORWARD, wall == LEFT ? speed_g : 0);

           break;
       }
}

void MovingState__init () {
    wall = LEFT; // arbitrario

    // set thresholds
    threshold_wall = get_thr_left();
    threshold_front = get_thr_front();
    threshold_wallnt = get_thr_right();

    set_robot_timer(1);

    movingState_g.screen_changed = 1;
}
void MovingState__exit () {
    // stop motors
    set_robot_timer(0);
    exit = 0;
    initialized = 0;
}

// update engine (se llama en el bucle principal)
void MovingState__update() {

    if (!initialized) {
        initialized = 1;
        init_wheels();
    }

    if (exit) {
        stop_movement();
        set_state(MenuState());
        return;
    }

    handle_state(calculate_state());
}

// screen
void MovingState__draw_screen () {
    // sensor values
    halLcdPrintLine("MOVING       ", TITLE, 0);

    sprintf(cadena, "S %03d %03d %03d", sd_g.left, sd_g.center, sd_g.right);
    halLcdPrintLine("  LFT CTR RGT", LINE_DIRS, NORMAL_TEXT);
    halLcdPrintLine(cadena, LINE_SENSORS, NORMAL_TEXT);

    if (wall == LEFT) {
        sprintf(cadena, "O %01d   %01d   %01d", sd_g.left > threshold_wall, sd_g.center > threshold_front, sd_g.right > threshold_wallnt);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    } else {
        sprintf(cadena, "O %01d   %01d   %01d", sd_g.left > threshold_wallnt, sd_g.center > threshold_front, sd_g.right > threshold_wall);
        halLcdPrintLine(cadena, LINE_OBSTACLE, NORMAL_TEXT);
    }

    sprintf(cadena, "Speed: %04d    ", speed_g);
    halLcdPrintLine(cadena, LINE_SPEED, NORMAL_TEXT);

    halLcdPrintLine(cadena_state, LINE_STATE, NORMAL_TEXT);

    movingState_g.screen_changed = 0;
}

// controls
void MovingState__s2_pressed () {
    // S2 -> RETURN TO MENU
    //ctx_g->set_state(MenuState(ctx_g));
    exit = 1;
}
void MovingState__s1_pressed () {
    // S1 -> PAUSE
    stop_g = !stop_g;


}

void MovingState__up_pressed () {
    speed_g += SPEED_INC;
    if (speed_g > SPEED_MAX) {
        speed_g = SPEED_MAX;
    }
    movingState_g.screen_changed = 1;
}

void MovingState__down_pressed () {
    if (speed_g < SPEED_INC) {
        speed_g = SPEED_MIN;
    } else {
        speed_g -= SPEED_INC;
    }
    movingState_g.screen_changed = 1;
}

void MovingState__left_pressed () {
    // Following LEFT wall
    wall = LEFT;
    threshold_wall   = get_thr_left();
    threshold_wallnt = get_thr_right();
    movingState_g.screen_changed = 1;
}
void MovingState__right_pressed () {
    // Following RIGHT wall
    wall = RIGHT;
    threshold_wallnt = get_thr_left();
    threshold_wall   = get_thr_right();
    movingState_g.screen_changed = 1;
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


