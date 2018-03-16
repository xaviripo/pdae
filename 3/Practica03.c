/******************************
 *
 * Practica_02_PAE Programaciï¿½ de Ports
 * i prï¿½ctica de les instruccions de control de flux:
 * "do ... while", "switch ... case", "if" i "for"
 * UB, 02/2017.
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432

#define LINEA_PAE 1
#define LINEA_ESTADO 2
#define LINEA_RETARD 3
#define LINEA_HORA 5
#define LINEA_HORA_SELEC 6
#define LINEA_ALARMA 7
#define LINEA_ALARMA_SELEC 8
#define LINEA_ALARMA_MSG 9

char saludo[16] = " PRACTICA 2 PAE"; //max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
uint32_t retraso = 500000;
uint16_t ms_elapsed = 0;
uint32_t time_seconds = 0; // 0-86399
uint8_t time_running = 1; // Bool que indica si el reloj está pausado o no
uint16_t alarm_minutes = 0; // 0-1439

// control leds P7

const uint32_t INC_RETRASO = 10;
const uint32_t MAX_RETRASO = 5000;
const uint32_t MIN_RETRASO = 10;


/**************************************************************************
 * INICIALIZACIï¿½N DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_interrupciones()
{
    // Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
    // accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
    // Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto
    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    // Timers
    NVIC->ICPR[0] |= BIT8; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT8; //y habilito las interrupciones del puerto

    // Timers
    NVIC->ICPR[0] |= BITA; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA; //y habilito las interrupciones del puerto

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACIï¿½N DE LA PANTALLA LCD.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

/**************************************************************************
 * BORRAR LINEA
 *
 * Datos de entrada: Linea, indica la linea a borrar
 *
 * Sin datos de salida
 *
 **************************************************************************/
void borrar(uint8_t Linea)
{
    halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

/**************************************************************************
 * ESCRIBIR LINEA
 *
 * Datos de entrada: Linea, indica la linea del LCD donde escribir
 * 					 String, la cadena de caracteres que vamos a escribir
 *
 * Sin datos de salida
 *
 **************************************************************************/
void escribir(char String[], uint8_t Linea)

{
    halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

/**************************************************************************
 * INICIALIZACIï¿½N DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_botons(void)
{
    //Configuramos botones y leds
    //***************************

    //Leds RGB del MK II:
    P2DIR |= BIT4 | BIT6;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
    P5DIR |= BIT6;  //Pin P5.6 (B)como salida Led (RGB)
    P2OUT &= ~(BIT4 | BIT6 );  //Inicializamos Led RGB a 0 (apagados)
    P5OUT &= ~BIT6; //Inicializamos Led RGB a 0 (apagados)

    //Boton S1 del MK II:
    P5SEL0 &= ~BIT1;   //Pin P5.1 como I/O digital,
    P5SEL1 &= ~BIT1;   //Pin P5.1 como I/O digital,
    P5DIR &= ~BIT1; //Pin P5.1 como entrada
    P5IES &= ~BIT1;   // con transicion L->H
    P5IE |= BIT1;     //Interrupciones activadas en P5.1,
    P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 5
    //P5REN: Ya hay una resistencia de pullup en la placa MK II

    //Boton S2 del MK II:
    P3SEL0 &= ~BIT5;   //Pin P3.5 como I/O digital,
    P3SEL1 &= ~BIT5;   //Pin P3.5 como I/O digital,
    P3DIR &= ~BIT5; //Pin P3.5 como entrada
    P3REN |= BIT4 + BIT5; //Activar resistencia pullup
    P3IES &= ~BIT5;   // con transicion L->H
    P3IE |= BIT5;   //Interrupciones activadas en P3.5
    P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3
    //P3REN: Ya hay una resistencia de pullup en la placa MK II

    //Configuramos los GPIOs del joystick del MK II:
    P4DIR &= ~(BIT1 + BIT5 + BIT7 ); //Pines P4.1, 4.5 y 4.7 como entrades,
    P4SEL0 &= ~(BIT1 + BIT5 + BIT7 ); //Pines P4.1, 4.5 y 4.7 como I/O digitales,
    P4SEL1 &= ~(BIT1 + BIT5 + BIT7 );
    P4REN |= BIT1 + BIT5 + BIT7;  //con resistencia activada
    P4OUT |= BIT1 + BIT5 + BIT7;  // de pull-up
    P4IE |= BIT1 + BIT5 + BIT7;  //Interrupciones activadas en P4.1, 4.5 y 4.7,
    P4IES &= ~(BIT1 + BIT5 + BIT7 ); //las interrupciones se generaran con transicion L->H
    P4IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4

    P5DIR &= ~(BIT4 + BIT5 ); //Pines P5.4 y 5.5 como entrades,
    P5SEL0 &= ~(BIT4 + BIT5 ); //Pines P5.4 y 5.5 como I/O digitales,
    P5SEL1 &= ~(BIT4 + BIT5 );
    P5REN |= BIT4 + BIT5; //Activar resistencia pullup
    P5OUT |= BIT4 + BIT5; //
    P5IE |= BIT4 + BIT5;  //Interrupciones activadas en 5.4 y 5.5,
    P5IES &= ~(BIT4 + BIT5 ); //las interrupciones se generaran con transicion L->H
    P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4
    // - Ya hay una resistencia de pullup en la placa MK II
}

/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void delay_t(uint32_t temps)
{
    volatile uint32_t i = temps;
    do
    {
        i--;
    }
    while (i > 0);
}

/*****************************************************************************
 * CONFIGURACIï¿½N DEL PUERTO 7. A REALIZAR POR EL ALUMNO
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void config_P7_LEDS(void)
{
    P7DIR |= 0xFF; // Todos de salida
    P7SEL0 &= 0x00; // Todos GPIO
    P7SEL1 &= 0x00;
    P7OUT &= 0x00;  // Todos apagados
}

void init_timer(void) {

    /****** Timer 0 ******/
    // Retraso de los LEDs

    TA0CTL =
            TASSEL__SMCLK + // clock SMCLK
            MC__UP +       // modo UP
            ID__8;         // /8

    TA0CCTL0 =
            CCIE; // activar int clock

    // Seteamos la constante de tiempo máximo del contador
    // Lo ponemos a 3000 ya que f/1000/8 = 24*10^6/1000/8 = 3000
    TA0CCR0 = 3000;

    /****** Timer 1 ******/
    // hh:mm:ss y alarma

    TA1CTL =
            TASSEL__ACLK + // clock SMCLK
            MC__UP +       // modo UP
            ID__8;         // /8

    TA1CCTL0 =
            CCIE; // activar int clock

    // Seteamos la constante de tiempo máximo del contador
    // Lo ponemos a 4096 ya que f/8 = 2^15/8 = 4096
    TA1CCR0 = 4096;

}

/* Codigo control de los leds P7 */
void update_leds(uint8_t izquierdaderecha, uint8_t* led_actual) {
    
    uint8_t temp;

    // mover segun sentido
    if (izquierdaderecha)
    {   // sentido ->
        /**
         * Se prueba el siguiente. Si se sale, se
         * vuelve a empezar
         **/
        temp = *led_actual << 0x01;
        *led_actual = temp ? temp : 0x01;
    } else {
        // sentido <-
        /**
         * Se prueba el siguiente. Si se sale, se
         * vuelve a empezar
         **/
        temp = *led_actual >> 0x01;
        *led_actual = temp ? temp : 0x80;
    }

    P7OUT = *led_actual;
}


void inc_seconds(uint32_t *time) {
    uint32_t secs = *time%60;
    *time -= secs;
    secs++;
    secs %= 60;
    *time += secs;
}

void inc_minutes(uint32_t *time) {
    uint32_t mins = (*time/60)%60;
    *time -= mins*60;
    mins++;
    mins %= 60;
    *time += mins*60;
}

void inc_hours(uint32_t *time) {
    uint32_t hrs = (*time/3600)%24;
    *time -= hrs*3600;
    hrs++;
    hrs %= 24;
    *time += hrs*3600;
}

void dec_seconds(uint32_t *time) {
    uint32_t secs = *time%60;
    *time -= secs;
    secs = secs == 0 ? 59 : secs-1;
    secs %= 60;
    *time += secs;
}

void dec_minutes(uint32_t *time) {
    uint32_t mins = (*time/60)%60;
    *time -= mins*60;
    mins = mins == 0 ? 59 : mins-1;
    mins %= 60;
    *time += mins*60;
}

void dec_hours(uint32_t *time) {
    uint32_t hrs = (*time/3600)%24;
    *time -= hrs*3600;
    hrs = hrs == 0 ? 23 : hrs-1;
    hrs %= 24;
    *time += hrs*3600;
}

void inc_minutes_alarm(uint16_t *time) {
    uint16_t mins = *time%60;
    *time -= mins;
    mins++;
    mins %= 60;
    *time += mins;
}

void inc_hours_alarm(uint32_t *time) {
    uint32_t hrs = (*time/60)%60;
    *time -= hrs*60;
    hrs++;
    hrs %= 60;
    *time += hrs*60;
}

void manage_states(uint8_t estado, uint8_t *izquierdaderecha, uint16_t *retraso_leds, uint8_t *selected_field) {

    /**
     * ESTADO DE *selected_field:
     * 0 -> segundos de la hora
     * 1 -> minutos de la hora
     * 2 -> horas de la hora
     * 3 -> minutos de la alarma
     * 4 -> horas de la alarma
     */

    uint32_t retraso_temp;
    uint32_t alarm_seconds;

    switch (estado)
    {
    case 1: // S1
        // leds RGB encendidos
        P2OUT |= BIT4 | BIT6;
        P5OUT |= BIT6;

        switch (*selected_field) {
        case 3: // m de alarma
            *selected_field = 1;
            break;
        case 4: // h de alarma
            *selected_field = 2;
            break;
        }

        break;
    case 2: // S2
        // leds RGB apagados
        P2OUT &= ~(BIT4 | BIT6 );
        P5OUT &= ~BIT6;

        switch (*selected_field) {
        case 0: // s de hora
        case 1: // m de hora
            *selected_field = 3;
            break;
        case 2: // h de hora
            *selected_field = 4;
            break;
        }

        break;
    case 3: // Left
        // leds RGB encendidos
        P2OUT |= BIT4 | BIT6;
        P5OUT |= BIT6;
        *izquierdaderecha = 0;
        P7OUT = 0x00;

        switch (*selected_field) {
        case 0: // s de hora
            *selected_field = 1;
            break;
        case 1: // m de hora
            *selected_field = 2;
            break;
        case 3: // m de alarma
            *selected_field = 4;
            break;
        }

        break;
    case 4: // Right
        // leds RG encendidos y B apagado
        P2OUT |= BIT4 | BIT6;
        P5OUT &= ~BIT6;

        switch (*selected_field) {
        case 1: // m de hora
            *selected_field = 0;
            break;
        case 2: // h de hora
            *selected_field = 1;
            break;
        case 4: // h de alarma
            *selected_field = 3;
            break;
        }

        // direcion ->
        *izquierdaderecha = 1;
        P7OUT = 0x00;
        break;
    case 5: // Up
        // leds RB encendidos y G apagado
        P2OUT |= BIT6;
        P2OUT &= ~BIT4;
        P5OUT |= BIT6;

        switch (*selected_field) {
        case 0: // s de hora
            if (!time_running) inc_seconds(&time_seconds);
            break;
        case 1: // m de hora
            if (!time_running) inc_minutes(&time_seconds);
            break;
        case 2: // h de hora
            if (!time_running) inc_hours(&time_seconds);
            break;
        case 3: // m de alarma
            alarm_seconds = alarm_minutes * 60;
            inc_minutes(&alarm_seconds);
            alarm_minutes = alarm_seconds / 60;
            break;
        case 4: // h de alarma
            alarm_seconds = alarm_minutes * 60;
            inc_hours(&alarm_seconds);
            alarm_minutes = alarm_seconds / 60;
            break;
        }

        /**
         * nota: si el retraso es demasiado grande
         * (overflow), se pone al valor maximo.
         **/
        retraso_temp = *retraso_leds + INC_RETRASO;
        *retraso_leds =
                (retraso_temp < *retraso_leds) || // Controlamos el overflow
                (retraso_temp > MAX_RETRASO) ? // Controlamos el valor mínimo permitido
                        MAX_RETRASO : retraso_temp;
        break;
    case 6: // Down
        // leds GB encendidos y R apagado
        P2OUT &= ~BIT6;
        P2OUT |= BIT4;
        P5OUT |= BIT6;

        switch (*selected_field) {
        case 0: // s de hora
            if (!time_running) dec_seconds(&time_seconds);
            break;
        case 1: // m de hora
            if (!time_running) dec_minutes(&time_seconds);
            break;
        case 2: // h de hora
            if (!time_running) dec_hours(&time_seconds);
            break;
        case 3: // m de alarma
            alarm_seconds = alarm_minutes * 60;
            dec_minutes(&alarm_seconds);
            alarm_minutes = alarm_seconds / 60;
            break;
        case 4: // h de alarma
            alarm_seconds = alarm_minutes * 60;
            dec_hours(&alarm_seconds);
            alarm_minutes = alarm_seconds / 60;
            break;
        }

        /**
         * nota: si el retraso es demasiado pequeÃ±o
         * (underflow), se pone a 1.
         **/
        retraso_temp = *retraso_leds - INC_RETRASO;
        *retraso_leds =
                (retraso_temp > *retraso_leds) || // Controlamos el underflow
                (retraso_temp < MIN_RETRASO) ? // Controlamos el valor mínimo permitido
                        MIN_RETRASO : retraso_temp;
        break;
    case 7: // Center
        // inversion de los leds
        P2OUT ^= BIT4 | BIT6;
        P5OUT ^= BIT6;

        time_running = !time_running;

        if (!time_running) {
            TA1CCTL0 &= ~CCIE; // Desactivamos interrupciones
        } else {
            TA1CCTL0 |=  CCIE; // Reactivamos interrupciones
        }

        break;
    }
}

void main(void)
{
    uint16_t retraso_leds;
    uint8_t izquierdaderecha;
    uint8_t led_actual;
    uint32_t prev_seconds;
    uint8_t selected_field; // 0 -> Hora; 1 -> Alarma

    WDTCTL = WDTPW + WDTHOLD;       	// Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_timer();
    init_interrupciones(); //Configurar y activar las interrupciones de los botones
    init_LCD();			    // Inicializamos la pantalla
    config_P7_LEDS();       // Configuramos los LEDs del P7

    halLcdPrintLine(saludo, LINEA_PAE, INVERT_TEXT); //escribimos saludo en la primera linea

    // init variables
    ms_elapsed = 0;
    izquierdaderecha = 1;
    retraso_leds = 50;
    led_actual = 0x00;
    selected_field = 2;
    time_running = 1;

    prev_seconds = 86401; // Para que sea distinto a cualquier tiempo inicial

    //Bucle principal (infinito):
    do
    {

        if (estado_anterior != estado) // Dependiendo del valor del estado se encenderï¿½ un LED u otro.
        {

            sprintf(cadena, " estado %d", estado); // Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena, LINEA_ESTADO);          // Escribimos la cadena al LCD
            estado_anterior = estado; // Actualizamos el valor de estado_anterior, para que no estï¿½ siempre escribiendo.

            // Escribir el delay por pantalla
            halLcdClearLine(LINEA_RETARD);
            sprintf(cadena, " retard %d", retraso_leds);
            escribir(cadena, LINEA_RETARD);

            manage_states(estado, &izquierdaderecha, &retraso_leds, &selected_field);

            // Escribir la alarma por pantalla
            sprintf(cadena, " alarma %02d:%02d", alarm_minutes / 60, alarm_minutes % 60);
            escribir(cadena, LINEA_ALARMA);

            // Subrallar la alarma si corresponde
            switch (selected_field) {
            case 0: // s de hora
                sprintf(cadena, "            ##");
                break;
            case 1: // m de hora
                sprintf(cadena, "         ##   ");
                break;
            case 2: // h de hora
                sprintf(cadena, "      ##      ");
                break;
            case 3: // m de alarma
                sprintf(cadena, "           ##");
                break;
            case 4: // h de alarma
                sprintf(cadena, "        ##   ");
                break;
            default:
                break;
            }

            escribir(cadena, selected_field <= 2 ? LINEA_HORA_SELEC : LINEA_ALARMA_SELEC);
            borrar(selected_field <= 2 ? LINEA_ALARMA_SELEC : LINEA_HORA_SELEC);

        }
        
        if (prev_seconds != time_seconds) {
            // Escribir la hora por pantalla
            sprintf(cadena, " hora %02d:%02d:%02d", time_seconds / 3600, (time_seconds % 3600) / 60, time_seconds % 60);
            escribir(cadena, LINEA_HORA);

            // Riiiing
            if ((uint16_t)((time_seconds/60)%60) == alarm_minutes) {
                sprintf(cadena, "Riiing");
                escribir(cadena, LINEA_ALARMA_MSG);
            } else {
                borrar(LINEA_ALARMA_MSG);
            }

            prev_seconds = time_seconds;
        }

        // contar si ha pasado suficiente tiempo
        if (retraso_leds <= ms_elapsed)
        {
            ms_elapsed = 0; // se resetea el contador
            update_leds(izquierdaderecha, &led_actual);
        }

    }
    while (1); //Condicion para que el bucle sea infinito
}


/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectarï¿½ quï¿½ botï¿½n se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualizar el valor de la variable global estado
 *
 **************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void)
{ //interrupcion del pulsador S2
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    switch (flag)
    {

    case 0x0C: // P3.5
        estado = 2;
        break;
    }

    P3IE |= BIT5;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void)
{ //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D; 	//interrupciones Joystick en port 4 desactivadas
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO BLOQUE switch ... case
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    switch (flag)
    {
    case 0x0C: // P4.5
        estado = 4;
        break;
    case 0x10: // P4.7
        estado = 3;
        break;
    case 0x04: // P4.1
        estado = 7;
        break;
    }

    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P4IE |= 0xA2; 	//interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void)
{ //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO BLOQUE switch ... case
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    switch (flag)
    {
    case 0x0A: // P5.4
        estado = 5;
        break;
    case 0x0C: // P5.5
        estado = 6;
        break;
    case 0x04: // P5.1
        estado = 1;
        break;
    }

    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}

void TA0_0_IRQHandler(void) {
    TA0CCTL0 &= ~CCIE; // Desactivamos interrupciones

    ms_elapsed++; // Ha pasado 1ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    TA0CCTL0 |=  CCIE;  // Reactivamos interrupciones
}

void TA1_0_IRQHandler(void) {
    TA1CCTL0 &= ~CCIE; // Desactivamos interrupciones

    time_seconds++; // Ha pasado 1s
    time_seconds %= 86400; // Un día = 86400s

    TA1CCTL0 &= ~CCIFG; // Limpiar flag
    TA1CCTL0 |=  CCIE;  // Reactivamos interrupciones
}
