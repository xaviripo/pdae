/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "timers.h"
#include "msp.h"



/******************************************************************************/
// GLOBALS
/******************************************************************************/

// Usado por el clock para contar el tiempo que ha pasado en ms.
time_t time_g = 0;



/******************************************************************************/
// INITS
/******************************************************************************/

void init_timers_0(void) {

    /****** Timer para el timeout en RX ******/
    // Retraso de los LEDs

    TA0CTL =
        TASSEL__SMCLK + // clock SMCLK
        MC__UP +        // modo UP
        ID__8;          // /8

    TA0CCTL0 =
        CCIE; // activar int clock

    // Seteamos la constante de tiempo m�ximo del contador
    // Queremos que la unidad b�sica sea 10^-3 segundos
    // Lo ponemos a 30 ya que f/1000/8 = 24*10^6/1000/8 = 3000
    TA0CCR0 = 3000;

    TA1CTL =
            TASSEL__ACLK + // clock ACLK
            MC__UP +       // modo UP
            ID__8;         // /8

    TA1CCTL0 =
            CCIE; // activar int clock

    // Valor inicial, que cambiará según el `tempo`
    TA1CCR0 = 1; // todo arreglar esto
}


void init_timers_1(void)
{
    // Timer para el timeout en RX
    NVIC->ICPR[0] |= BIT8; // Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT8; // y habilito las interrupciones del puerto

    // Timer musica
    NVIC->ICPR[0] |= BITA; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA; //y habilito las interrupciones del puerto
}



/******************************************************************************/
// INTERFAZ
/******************************************************************************/

/**
 * Activa o desactiva la interrupción del reloj
 * @param enable true = activar; false = desactivar
 */
void set_timer_interrupt(bool enable)
{
    if (enable) {
        TA0CCTL0 |=  CCIE;
    } else {
        TA0CCTL0 &= ~CCIE;
    }
}

/**
 * Resetea el timer a 0
 */
void reset_time(void) {
    time_g = 0;
}

/**
 * ¿Han pasado time milisegundos desde que se ha reseteado el contador?
 * @param time Tiempo en milisegundos que se quiere comprobar
 * @return true = ha pasado el tiempo; false = aún no ha pasado
 */
bool has_passed(time_t time)
{
    return time < time_g;
}


/**
 * Devuelve la constante de timeout en milisegundos
 */
uint8_t get_timeout(void) {
    return TIMEOUT;
}



/******************************************************************************/
// HANDLERS
/******************************************************************************/

// Clock
void TA0_0_IRQHandler(void) {
    set_timer_interrupt(0); // Desactivamos interrupciones

    time_g++; // Ha pasado 1ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    set_timer_interrupt(1);  // Reactivamos interrupciones
}

void TA1_0_IRQHandler(void) {
    set_timer_interrupt(0); // Desactivamos interrupciones

    

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    set_timer_interrupt(1);  // Reactivamos interrupciones
}
