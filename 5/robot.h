/*
 * robot.h
 *
 *  Created on: 4 maig 2018
 *      Author: mat.aules
 */

#ifndef ROBOT_H_
#define ROBOT_H_

/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "msp.h"
#include "lib_PAE2.h"



/******************************************************************************/
// DEFINES
/******************************************************************************/

// Máximo número de parámetros en TX
#define MAX_PARAMETER_LENGTH 16

// Usado para leer el output de los sensores
#define OBSTACLE_LEFT 0x01
#define OBSTACLE_CENTER 0x02
#define OBSTACLE_RIGHT 0x04


/******************************************************************************/
// TYPEDEFS
/******************************************************************************/

// Lado del robot, usado para identificar el motor;
// se considera "delante" la dirección FORWARD
typedef enum {RIGHT, LEFT} side_t;

// Dirección en la que se puede mover el robot;
// FORWARD es hacia donde apunta el sensor
typedef enum {BACKWARD, FORWARD} direction_t;

typedef uint8_t bool;
typedef uint16_t time_t;



/******************************************************************************/
// STRUCTS
/******************************************************************************/

typedef struct RxPacket {
    int8_t status[MAX_PARAMETER_LENGTH];
    bool timeout;
    bool checksum_correct;
} RxPacket;


/******************************************************************************/
// INITS
/******************************************************************************/

void init_uart(void);

void init_timers(void);

void init_interrupts(void);

/******************************************************************************/
// HELPERS para TX y RX
/******************************************************************************/

//////////////// Reloj

/**
 * Activa o desactiva la interrupción del reloj
 * @param enable true = activar; false = desactivar
 */
void set_timer_interrupt(bool enable);

/**
 * Resetea el timer a 0
 */
void reset_time(void);
/**
 * ¿Han pasado time milisegundos desde que se ha reseteado el contador?
 * @param time Tiempo en milisegundos que se quiere comprobar
 * @return true = ha pasado el tiempo; false = aún no ha pasado
 */
bool has_passed(time_t time);


//////////////// Envío y recepción de bytes por la UART

/**
 * Establece la dirección de la línea a RX
 */
void set_direction_rx(void);

/**
 * Establece la dirección de la línea a TX
 */
void set_direction_tx(void);

/**
 * Envía byte por la UCA2
 */
void tx_byte_uac2(uint8_t data);

/**
 * ¿Se ha recibido algún byte desde que se ha leído el último?
 * @return true = se ha recibido byte; false = no se ha recibido
 */
bool has_received_byte(void);

/**
 * Lee el último byte recibidio y desactiva el flag de byte recbido.
 * @return Byte que se ha recibido
 */
uint8_t get_read_byte(void);


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
 * Lee si hay obstáculos enfrente del robot.
 * @return Uno o varios de los siguientes:
 *         OBSTACLE_LEFT Obstáculo a la izquierda
 *         OBSTACLE_CENTER Obstáculo justo enfrente
 *         OBSTACLE_RIGHT Obstáculo a la derecha
 *         Por ejemplo, si hay obstáculos a izda y dcha, se devolverá:
 *         (OBSTACLE LEFT | OBSTACLE_RIGHT)
 */
uint8_t read_obstacle(void);
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

#endif /* ROBOT_H_ */
