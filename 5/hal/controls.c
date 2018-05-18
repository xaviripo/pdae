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

}

void init_controls_1(void) {

    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

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



//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void) { //interrupci�n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas

    s1_pressed();

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
