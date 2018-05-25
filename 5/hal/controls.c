/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "controls.h"
#include "msp.h"



/******************************************************************************/
// INITS
/******************************************************************************/

void init_controls_0(void) {

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
    //P3REN |= BIT4 + BIT5; //Activar resistencia pullup
    P3IES &= ~BIT5;   // con transicion L->H
    P3IE |= BIT5;   //Interrupciones activadas en P3.5
    P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3

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

}

void init_controls_1(void) {

    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto

}



/******************************************************************************/
// HANDLERS
/******************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void) {
    //interrupcion del pulsador S2
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas

    s2_pressed();

    P3IE |= BIT5;   //interrupciones Joystick y S1 en port 5 reactivadas
}

void PORT4_IRQHandler(void)
{ //interrupci�n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D;   //interrupciones Joystick en port 4 desactivadas


    switch (flag)
    {
    case 0x0C: // P4.5 -> right
        right_pressed();
        break;
    case 0x10: // P4.7 -> left
        left_pressed();
        break;
    case 0x04: // P4.1 -> center
        center_pressed();
        break;
    }

    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P4IE |= 0xA2;   //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void) { //interrupci�n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas

    switch (flag)
    {
    case 0x0A: // P5.4 -> up
        up_pressed();
        break;
    case 0x0C: // P5.5 -> down
        down_pressed();
        break;
    case 0x04: // P5.1
        s1_pressed();
        break;
    }

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
