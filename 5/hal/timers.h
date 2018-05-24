#ifndef TIMERS_H_
#define TIMERS_H_

/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "../common.h"



/******************************************************************************/
// DEFINES
/******************************************************************************/

// Tiempo de timeout al leer datos UART
#define TIMEOUT 1000



/******************************************************************************/
// INTERFAZ
/******************************************************************************/

void init_timers_0();
void init_timers_1();

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


/**
 * Devuelve la constante de timeout en milisegundos
 */
uint8_t get_timeout(void);

extern void on_music_tick();

#endif /* TIMERS_H_ */
