/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "msp.h"
#include "robot.h"
#include "hal/timers.h"
#include "hal/communication.h"

/******************************************************************************/
// DEFINES
/******************************************************************************/

// Usado para leer el output de los sensores
#define OBSTACLE_LEFT 0x01
#define OBSTACLE_CENTER 0x02
#define OBSTACLE_RIGHT 0x04

// Instrucciones que acepta el motor y los sensores
#define READ_DATA 0x02
#define WRITE_DATA 0x03

/******************************************************************************/
// GLOBALES
/******************************************************************************/

sensor_distance_t cached_sd_g;

/******************************************************************************/
// READ y WRITE
/******************************************************************************/

/**
 * Escribe datos en un módulo y comprueba que se haga correctamente.
 * La respuesta del módulo se puede leer en response_g
 * @param id Id del módulo en el que escribimos los datos
 * @param parameter_length Número de bytes a escribir.
 *                         Tiene que ser el número de elementos en parameters.
 * @param parameters Array de bytes a escribir
 * @return true = se ha escrito correctamente; false = ha habido algún problema
 */
bool write(uint8_t id, uint8_t parameter_length, uint8_t parameters[MAX_PARAMETER_LENGTH]) {

    bool correct;

    correct = (tx_instruction(id, parameter_length, WRITE_DATA, parameters) != 0);

    rx_status();

    if (get_status().timeout || !get_status().checksum_correct) {
        return 0;
    }

    return correct;

}

/**
 * Lee datos de un módulo y comprueba que se haga correctamente.
 * La respuesta del módulo se puede leer en response_g
 * @param id Id del módulo del que leeremos los datos
 * @param initial_position Posición inicial a leer del módulo
 * @param amount Número de bytes a leer incluyendo el inicial
 * @return true = se ha leído correctamente; false = ha habido algún problema
 */
bool read(uint8_t id, uint8_t initial_position, uint8_t amount) {

    bool correct;
    uint8_t parameters[] = {initial_position, amount};

    correct = (tx_instruction(id, 2, READ_DATA, parameters) != 0);

    rx_status();

    if (get_status().timeout || !get_status().checksum_correct) {
        return 0;
    }

    return correct;
}



/******************************************************************************/
// API del ROBOT
/******************************************************************************/

/*
 * Activa o desactiva un LED
 * @param side Lado del robot en el que se encuentra el LED
 * @param on Activo o inactivo
 * @return true = se ha hecho el cambio correctamente; false = algo ha ido mal
 */
bool write_led(bool side, bool on) {

    uint8_t parameter_length = 2;
    uint8_t parameters[] = {0x19, (on?1:0)};
    uint8_t id = side ? 2 : 3;

    return write(id, parameter_length, parameters);

}

/**
 * Rotar una rueda del robot
 * @param wheel_id Id del motor cuya rueda queremos rotar
 * @param direction Dirección en la que rotar la rueda, BACKWARD o FORWARD
 * @param speed Velocidad. Entero entre 0 y 1022, ambos incluidos.
 *              0 = parado; 1022 = máxima velocidad
 * @return true = ha ido bien; false = ha ido mal
 */
bool rotate_wheel(uint8_t wheel_id, bool direction, uint16_t speed) {

    if (speed > 1022) {
        return 0;
    }

    // Dynamixel uses 0 for unlimited power
    // 1 stopped
    // 2-1023 linear speed
    speed ++;
    uint8_t parameters[5];

    //////////////////////////////////////////////////////////////////////////////

    // CW and CCW angle limits to 0 so no max angle


    parameters[0] = 0x06;
    parameters[1] = 0;
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;

    if(!write(wheel_id, 5, parameters)) {
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////

    parameters[0] = 0x20;
    parameters[1] = speed & 0xff; // Cogemos los 8 bits inferiores
    parameters[2] = (speed >> 8); // Cogemos los 8 bits superiores
    parameters[2] |= (direction ? 1 : 0) * BIT2; // Cambiamos el bit 10 a 1 o 0 seg�n direcci�n

    if(!write(wheel_id, 3, parameters)) {
        return 0;
    }

    return 1;

}

/**
 * Rota la rueda izquierda del robot
 * @param direction Dirección en la que rotar la rueda, BACKWARD o FORWARD
 * @param speed Velocidad. Ver rotate_wheel para ver el dominio.
 */
bool rotate_left(bool direction, uint16_t speed) {
    return rotate_wheel(0x03, !direction, speed);
}

/**
 * Rota la rueda derecha del robot
 * @param direction Dirección en la que rotar la rueda, BACKWARD o FORWARD
 * @param speed Velocidad. Ver rotate_wheel para ver el dominio.
 */
bool rotate_right(bool direction, uint16_t speed) {
    return rotate_wheel(0x02, direction, speed);
}

bool turn(rot_t direction, uint16_t speed) {
    switch (direction) {
    case CLOCKWISE:
        rotate_left(FORWARD, speed);
        rotate_left(BACKWARD, speed);
        break;
    case COUNTER_CLOCKWISE:
        rotate_left(BACKWARD, speed);
        rotate_left(FORWARD, speed);
        break;
    default:
        break;
    }
    return 1;
}

bool stop_movement() {
    return rotate_left(FORWARD,0) && rotate_right(FORWARD,0);
}

/**
 * Lee si hay obstáculos enfrente del robot.
 * @return Uno o varios de los siguientes:
 *         OBSTACLE_LEFT Obstáculo a la izquierda
 *         OBSTACLE_CENTER Obstáculo justo enfrente
 *         OBSTACLE_RIGHT Obstáculo a la derecha
 *         Por ejemplo, si hay obstáculos a izda y dcha, se devolverá:
 *         (OBSTACLE LEFT | OBSTACLE_RIGHT)
 */
uint8_t read_obstacle(void) {

    // Ha ido mal
    if(!read(100, 0x20, 1)) {
        return -1;
    }

    return get_status().status[5];

}

sensor_distance_t read_obstacle_distance(void) {
    // Ha ido mal
    if(!read(100, 0x1A, 3)) {
        return cached_sd_g; // todo comprobar si es que ha ido mal!
    }

    cached_sd_g.left = get_status().status[5];
    cached_sd_g.center = get_status().status[6];
    cached_sd_g.right = get_status().status[7];
    return cached_sd_g;
}

/*
 * Configura el parámetro de distancia a la que se detectan obstáculos.
 * @param threshold Valor entre 0 y 255
 *                        0 = detección a proximidad mínima
 *                        255 = detección a proximidad máxima
 */
void set_obstacle_threshold(uint8_t threshold) {
    uint8_t parameters[] = {0x14, threshold};
    write(100, 2, parameters);
}

/**
 * Lee el contador de sonidos ("palmadas")
 * @return El número de palmadas detectadas en total en los últimos 800ms
 */
uint8_t read_claps(void) {
    read(100, 0x25, 1);
    return get_status().status[5];
}

void set_robot_timer(bool flag) {
    set_comm_timer_interrupt(flag);
}


