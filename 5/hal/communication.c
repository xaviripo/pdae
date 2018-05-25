/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "communication.h"
#include "msp.h"
#include "timers.h"



/******************************************************************************/
// DEFINES
/******************************************************************************/

// Para comprobar si el buffer está ocupado
#define TXD2_READY (UCA2IFG & UCTXIFG)

// Máximo número de parámetros en TX
#define MAX_PARAMETER_LENGTH 16

// Instrucciones que acepta el motor y los sensores
#define READ_DATA 0x02
#define WRITE_DATA 0x03



/******************************************************************************/
// GLOBALS
/******************************************************************************/

// Se ha recibido un byte por UART?
bool has_byte_received_g = 0;

// Byte que se ha recibido por UART
uint8_t byte_received_g = 0;

/* Respuesta recibida por UART del motor/sensor.
 * Por alguna razón solo hemos conseguido recibir los datos correctamente
 * usando una variable global. */
RxPacket response_g;



/******************************************************************************/
// INITS
/******************************************************************************/

void init_comm_0(void)
{
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    /* UCSYNC=0 mode as�ncron
     * UCMODEx=0 seleccionem mode UART
     * UCSPB=0 nomes 1 stop bit
     * UC7BIT=0 8 bits de dades
     * UCMSB=0 bit de menys pes primer
     * UCPAR=x ja que no es fa servir bit de paritat
     * UCPEN=0 sense bit de paritat
     * Triem SMCLK (16MHz) com a font del clock BRCLK */
    UCA2CTLW0 |= UCSSEL__SMCLK;
    UCA2MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    /* Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
     * volem un baud rate de 500kb/s i fem sobre-mostreig de 16
     * el rellotge de la UART ha de ser de 8MHz (24MHz/3). */
    UCA2BRW = 3;
    // Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3; //I/O funci�: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3); //Configurem pin de selecci� del sentit de les dades Transmissi�/Recepeci�
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la l�nia de comunicacions s�rie
    UCA2IE |= UCRXIE; //Aix� nom�s s�ha d�activar quan tinguem la rutina de recepci�

}

void init_comm_1(void)
{

    NVIC->ICPR[0] |= 1<<0x12;
    NVIC->ISER[0] |= 1<<0x12;

}



/******************************************************************************/
// HELPERS para TX y RX
/******************************************************************************/

/**
 * Establece la dirección de la línea a RX
 */
void set_direction_rx(void)
{
    P3OUT &= ~BIT0;
}

/**
 * Establece la dirección de la línea a TX
 */
void set_direction_tx(void)
{
    P3OUT |= BIT0;
}

/**
 * Envía byte por la UCA2
 */
void tx_byte_uac2(uint8_t data)
{
    while(!TXD2_READY); // Esperamos a que est� listo el buffer de transmisi�n
    UCA2TXBUF = data;
}

/**
 * ¿Se ha recibido algún byte desde que se ha leído el último?
 * @return true = se ha recibido byte; false = no se ha recibido
 */
bool has_received_byte(void) {
    return has_byte_received_g;
}

/**
 * Lee el último byte recibidio y desactiva el flag de byte recbido.
 * @return Byte que se ha recibido
 */
uint8_t get_read_byte(void) {
    // Reseteamos el flag de alerta de byte
    has_byte_received_g = 0;
    // Devolvemos el byte recibido
    return byte_received_g;
}

/******************************************************************************/
// INTERFAZ
/******************************************************************************/

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
uint8_t tx_instruction(uint8_t module_id, uint8_t parameter_length, uint8_t instruction, uint8_t parameters[MAX_PARAMETER_LENGTH])
{

    uint8_t i, checksum, packet_length;
    uint8_t tx_buffer[32];

    // IMPUESTO POR LOS PROFESORES: prohibido escribir en los registros 0x00, 0x01, ..., 0x05
    // Check initial writing position is >= 0x06
    // also check that initial position + nr of bytes we write isn't > than last registrer, in case it wraps around
    if (instruction != READ_DATA && parameters[0] < 0x06 && parameters[0]+parameter_length-2 <= 0x31) {
        return 0;
    }

    // Check we don't try to write

    set_direction_tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)

    // Rellenar el paquete de datos
    tx_buffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    tx_buffer[1] = 0xff;
    tx_buffer[2] = module_id; //ID del m�dul al que volem enviar el missatge
    tx_buffer[3] = parameter_length + 2; //Length(Parameter,Instruction,Checksum)
    tx_buffer[4] = instruction; //Instrucci� que enviem al M�dul
    for(i = 0; i < parameter_length; i++) //Comencem a generar la trama que hem d�enviar
    {
        tx_buffer[i+5] = parameters[i];
    }

    // Calcular checksum
    checksum = 0;
    packet_length = parameter_length+4+2;
    for(i = 2; i < packet_length-1; i++) //C�lcul del checksum
    {
        checksum += tx_buffer[i];
    }
    tx_buffer[i] = ~checksum; //Escriu el Checksum (complement a 1)

    // Enviar los datos
    for(i = 0; i < packet_length; i++) //Aquest bucle �s el que envia la trama al M�dul Robot
    {
        tx_byte_uac2(tx_buffer[i]);
    }

    while(UCA2STATW & UCBUSY); //Espera fins que s�ha transm�s el �ltim byte

    set_direction_rx(); //Posem la l�nia de dades en Rx perqu� el m�dul Dynamixel envia resposta

    return packet_length;

}

/**
 * Recibe un paquete de status del último módulo al que se le ha enviado algo
 * La función no devuelve nada, almacena el resultado en la variable global
 * response_g
 */
void rx_status(void) {
    uint8_t i, packet_length, checksum;
    bool timeout = 0;
    bool received_byte;

    set_direction_rx(); //Ponemos la linea half duplex en Rx
    set_comm_timer_interrupt(1);

    response_g.status[3] = 0x00;

    for(i = 0; i < 4; i++)
    {
        reset_comm_time();
        received_byte = 0;
        while (!received_byte) // Se_ha_recibido_Byte())
        {
            received_byte = has_received_byte();
            timeout = has_passed_comm(TIMEOUT); // tiempo en ms
            if (timeout) break; //sale del while
        }
        //sale del for si ha habido Timeout
        if (timeout) break;
        //Si no, es que todo ha ido bien, y leemos un dato:
        response_g.status[i] = get_read_byte(); //Get_Byte_Leido_UART();
    }//fin del for

    // Continua llegint la resta de bytes del Status Packet
    if (!timeout)
    {
        response_g.timeout = 0;

        packet_length = response_g.status[3];

        for(i = 4; i < packet_length + 4; i++) //packet_length; i++)
        {
            reset_comm_time();
            received_byte = 0;
            while (!received_byte) // Se_ha_recibido_Byte())
            {
                received_byte = has_received_byte();
                timeout = has_passed_comm(TIMEOUT); // tiempo en decenas de microsegundos
                if (timeout) break; //sale del while
            }
            //sale del for si ha habido Timeout
            if (timeout) break;
            //Si no, es que todo ha ido bien, y leemos un dato:
            response_g.status[i] = get_read_byte(); //Get_Byte_Leido_UART();
        }

        // Comprobar checksum sea correcto
        // Atenci�n! A pesar de que la documentaci�n pone que hay que sumar "instruction",
        // lo que hay que sumar es "error"
        checksum = response_g.status[packet_length+3] + 1;
        for (i = 2; i < packet_length + 3; i++) {
            checksum += response_g.status[i];
        }

        response_g.checksum_correct = checksum == 0;

    }
    else
    {
        response_g.timeout = 1;
    }
}

RxPacket get_status(void) {
    return response_g;
}



/******************************************************************************/
// INTERRUPT HANDLERS
/******************************************************************************/

// UART
void EUSCIA2_IRQHandler(void) {
    UCA2IE &= ~UCRXIE;
    byte_received_g = UCA2RXBUF;
    has_byte_received_g = 1;
    UCA2IE |= UCRXIE;
}
