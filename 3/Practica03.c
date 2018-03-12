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

char saludo[16] = " PRACTICA 2 PAE"; //max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
uint32_t retraso = 500000;
uint32_t ms_elapsed = 0;

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
    halLcdPrintLine(String, Linea, INVERT_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
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

    // TODO pasar a máscaras/constantes/(structs?)
    TA0CTL =
            TASSEL__ACLK + // clock ACLK
            MC__UP +       // modo UP
            ID__1;         // /1

    TA0CCTL0 =
            CCIE; // activar int clock

    // Seteamos la constante de tiempo máximo del contador
    // Lo ponemos a 32 ya que f/1000 = 32.768 (que no es int)
    TA0CCR0 = 32;

    /****** Timer 1 ******/
    // hh:mm:ss y alarma

    /*
     * Introducir alarma desde el robot o desde el código?
     *
     * */
}

/* Codigo control de los leds P7 */
void update_leds(uint8_t izquierdaderecha, uint8_t led_actual) {
    
    // mover segun sentido
    if (izquierdaderecha)
    {   // sentido ->
        /**
         * Se prueba el siguiente. Si se sale, se
         * vuelve a empezar
         **/
        temp = led_actual << 0x01;
        led_actual = temp ? temp : 0x01;
    }
    else
    {
        // sentido <-
        /**
         * Se prueba el siguiente. Si se sale, se
         * vuelve a empezar
         **/
        temp = led_actual >> 0x01;
        led_actual = temp ? temp : 0x80;
    }
    P7OUT = led_actual;
}

void main(void)
{
    uint32_t retraso_leds;
    uint8_t izquierdaderecha;
    uint8_t temp;
    uint8_t led_actual;
    uint32_t retraso_temp;

    WDTCTL = WDTPW + WDTHOLD;       	// Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_16MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_timer();
    init_interrupciones(); //Configurar y activar las interrupciones de los botones
    init_LCD();			    // Inicializamos la pantalla
    config_P7_LEDS();       // Configuramos los LEDs del P7

    halLcdPrintLine(saludo, linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++; //Aumentamos el valor de linea y con ello pasamos a la linea siguiente

    // init variables
    ms_elapsed = 0;
    izquierdaderecha = 1;
    retraso_leds = 50;
    led_actual = 0x00;

    //Bucle principal (infinito):
    do
    {
        if (estado_anterior != estado) // Dependiendo del valor del estado se encenderï¿½ un LED u otro.
        {
            sprintf(cadena, " estado %d", estado); // Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena, linea);          // Escribimos la cadena al LCD
            estado_anterior = estado; // Actualizamos el valor de estado_anterior, para que no estï¿½ siempre escribiendo.

            // Escribir el delay por pantalla
            halLcdClearLine(linea+1);
            sprintf(cadena, " retard %d", retraso_leds);
            escribir(cadena, linea+1);

            /**********************************************************+
             A RELLENAR POR EL ALUMNO BLOQUE switch ... case
             Para gestionar las acciones:
             Boton S1, estado = 1
             Boton S2, estado = 2
             Joystick left, estado = 3
             Joystick right, estado = 4
             Joystick up, estado = 5
             Joystick down, estado = 6
             Joystick center, estado = 7
             ***********************************************************/

            switch (estado)
            {
            case 1:
                // leds RGB encendidos
                P2OUT |= BIT4 | BIT6;
                P5OUT |= BIT6;
                break;
            case 2:
                // leds RGB apagados
                P2OUT &= ~(BIT4 | BIT6 );
                P5OUT &= ~BIT6;
                break;
            case 3:
                // leds RGB encendidos
                P2OUT |= BIT4 | BIT6;
                P5OUT |= BIT6;
                izquierdaderecha = 0;
                P7OUT = 0x00;

                break;
            case 4:
                // leds RG encendidos y B apagado
                P2OUT |= BIT4 | BIT6;
                P5OUT &= ~BIT6;

                // direcion ->
                izquierdaderecha = 1;
                P7OUT = 0x00;
                break;
            case 5:
                // leds RB encendidos y G apagado
                P2OUT |= BIT6;
                P2OUT &= ~BIT4;
                P5OUT |= BIT6;

                /**
                 * nota: si el retraso es demasiado grande
                 * (overflow), se pone al valor maximo.
                 **/
                retraso_temp = retraso_leds + INC_RETRASO;
                retraso_leds =
                        (retraso_temp < retraso_leds) || // Controlamos el overflow
                        (retraso_temp > MAX_RETRASO) ? // Controlamos el valor mínimo permitido
                                MAX_RETRASO : retraso_temp;
                break;
            case 6:
                // leds GB encendidos y R apagado
                P2OUT &= ~BIT6;
                P2OUT |= BIT4;
                P5OUT |= BIT6;

                /**
                 * nota: si el retraso es demasiado pequeÃ±o
                 * (underflow), se pone a 1.
                 **/
                retraso_temp = retraso_leds - INC_RETRASO;
                retraso_leds =
                        (retraso_temp > retraso_leds) || // Controlamos el underflow
                        (retraso_temp < MIN_RETRASO) ? // Controlamos el valor mínimo permitido
                                MIN_RETRASO : retraso_temp;
                break;
            case 7:
                // inversion de los leds
                P2OUT ^= BIT4 | BIT6;
                P5OUT ^= BIT6;
                break;
            }
        }
        
        // contar si ha pasado suficiente tiempo
        if (retraso_leds <= ms_elapsed)
        {
            ms_elapsed = 0; // se resetea el contador
            update_leds(izquierdaderecha, led_actual)
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

    ms_elapsed++; // Ha pasado 1 ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    TA0CCTL0 |=  CCIE;  // Reactivamos interrupciones
}
