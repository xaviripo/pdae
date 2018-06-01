#ifndef ROBOT_H_
#define ROBOT_H_

/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "common.h"
#include "lib_PAE2.h"



/******************************************************************************/
// DEFINES
/******************************************************************************/

// Usado para leer el output de los sensores
#define OBSTACLE_LEFT 0x01
#define OBSTACLE_CENTER 0x02
#define OBSTACLE_RIGHT 0x04

// Rango de velocidades
#define SPEED_MIN 0
#define SPEED_MAX 1022


/******************************************************************************/
// TYPEDEFS
/******************************************************************************/

// Lado del robot, usado para identificar el motor;
// se considera "delante" la dirección FORWARD
typedef enum {LEFT, RIGHT} side_t;

// Sentido de rotaci�n, viendo el robot en plano cenital;
typedef enum {CLOCKWISE, COUNTER_CLOCKWISE} rot_t;

// Dirección en la que se puede mover el robot;
// FORWARD es hacia donde apunta el sensor
typedef enum {BACKWARD, FORWARD} direction_t;



/******************************************************************************/
// STRUCTS
/******************************************************************************/

typedef struct sensor_distance {
    uint8_t left;
    uint8_t center;
    uint8_t right;
} sensor_distance_t;



/******************************************************************************/
// API del ROBOT
/******************************************************************************/

/*
 * Activa o desactiva un LED
 * @param side Lado del robot en el que se encuentra el LED
 * @param on Activo o inactivo
 * @return true = se ha hecho el cambio correctamente; false = algo ha ido mal
 */
bool write_led(bool side, bool on);
/**
 * Rota la rueda izquierda del robot
 * @param direction Dirección en la que rotar la rueda, BACKWARD o FORWARD
 * @param speed Velocidad. Ver rotate_wheel para ver el dominio.
 */
bool rotate_left(bool direction, uint16_t speed);

/**
 * Rota la rueda derecha del robot
 * @param direction Dirección en la que rotar la rueda, BACKWARD o FORWARD
 * @param speed Velocidad. Ver rotate_wheel para ver el dominio.
 */
bool rotate_right(bool direction, uint16_t speed);

/**
 * Rota el robot sobre s� mismo
 * @param direction Direcci�n. RIGHT = cw visto desde arriba, LEFT = ccw visto desde arriba
 */
bool turn(rot_t direction, uint16_t speed);

/**
 * Lee si hay obstáculos enfrente del robot.
 * @return Uno o varios de los siguientes:
 *         OBSTACLE_LEFT Obstáculo a la izquierda
 *         OBSTACLE_CENTER Obstáculo justo enfrente
 *         OBSTACLE_RIGHT Obstáculo a la derecha
 *         Por ejemplo, si hay obstáculos a izda y dcha, se devolverá:
 *         (OBSTACLE LEFT | OBSTACLE_RIGHT)
 */
uint8_t read_obstacle(void);

sensor_distance_t read_obstacle_distance(void);

/*
 * Configura el parámetro de distancia a la que se detectan obstáculos.
 * @param threshold Valor entre 0 y 255
 *                        0 = detección a proximidad mínima
 *                        255 = detección a proximidad máxima
 */
void set_obstacle_threshold(uint8_t threshold);

/**
 * Lee el contador de sonidos ("palmadas")
 * @return El número de palmadas detectadas en total en los últimos 800ms
 */
uint8_t read_claps(void);

/**
 * Encapsular activaci�n/desactivaci�n de timer de sensores
 * @param flag the flag
 */
void set_robot_timer(bool flag);

/**
 * Stop the wheels
 * @return if the function worked correctly
 */
bool stop_movement();

#endif /* ROBOT_H_ */
