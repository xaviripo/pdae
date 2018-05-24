#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

// Instrucciones que acepta el motor y los sensores
#define READ_DATA 0x02
#define WRITE_DATA 0x03

/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "../common.h"



/******************************************************************************/
// DEFINES
/******************************************************************************/

// Para comprobar si el buffer está ocupado
#define TXD2_READY (UCA2IFG & UCTXIFG)

// Máximo número de parámetros en TX
#define MAX_PARAMETER_LENGTH 16



/******************************************************************************/
// STRUCTS
/******************************************************************************/

typedef struct RxPacket {
    int8_t status[MAX_PARAMETER_LENGTH];
    bool timeout;
    bool checksum_correct;
} RxPacket;



/******************************************************************************/
// INTERFAZ
/******************************************************************************/

void init_comm_0(void);
void init_comm_1(void);



/**
 * Envía un paquete de instrucción a un módulo vía UART
 * @param module_id Id del módulo al que le enviamos el paquete
 * @param parameter_length Número de parámetros que contiene el paquete.
 *                         Tiene que ser el número de elementos en parameters.
 * @param instruction Instrucción que le enviamos al módulo.
 *                    Hay una constante definida para cada instrucción válida
 * @param parameters Array que contiene los parámetros a pasar.
 * @return 0 = error; n (>0) = número de bytes enviados
 */
uint8_t tx_instruction(uint8_t module_id, uint8_t parameter_length, uint8_t instruction, uint8_t parameters[MAX_PARAMETER_LENGTH]);

/**
 * Recibe un paquete de status del último módulo al que se le ha enviado algo
 * La función no devuelve nada, almacena el resultado en la variable global
 * response_g
 */
void rx_status(void);

/**
 * Devuelve el paquete global de status
 */
RxPacket get_status(void);



#endif /* COMMUNICATION_H_ */
