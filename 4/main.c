#include <stdio.h>
#include <stdint.h>
#include "msp.h"
#include "lib_PAE2.h"

#define TXD2_READY (UCA2IFG & UCTXIFG)

#define MAX_PARAMETER_LENGTH 16

#define TIMEOUT TIMEOUT

typedef uint8_t bool;
typedef uint16_t time_t;

/**
 * GLOBALS
 */

time_t time_g = 0;
bool has_byte_received_g = 0;
uint8_t byte_received_g = 0;

/**
 * STRUCTS
 */

typedef struct RxPacket {
    int8_t status[MAX_PARAMETER_LENGTH];
    int8_t timeout;
} RxPacket;

/**
 * INITS
 */

void init_uart(void)
{
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; /**
                                 * UCSYNC=0 mode as�ncron
                                 * UCMODEx=0 seleccionem mode UART
                                 * UCSPB=0 nomes 1 stop bit
                                 * UC7BIT=0 8 bits de dades
                                 * UCMSB=0 bit de menys pes primer
                                 * UCPAR=x ja que no es fa servir bit de paritat
                                 * UCPEN=0 sense bit de paritat
                                 * Triem SMCLK (16MHz) com a font del clock BRCLK
                                 */
    UCA2MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3; /**
                  * Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                  * volem un baud rate de 500kb/s i fem sobre-mostreig de 16
                  * el rellotge de la UART ha de ser de 8MHz (24MHz/3).
                  */
    // Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3; //I/O funci�: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3); //Configurem pin de selecci� del sentit de les dades Transmissi�/Recepeci�
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la l�nia de comunicacions s�rie
    // UCA2IE |= UCRXIE; //Aix� nom�s s�ha d�activar quan tinguem la rutina de recepci�

    UCA2IE = UCRXIE; // TODO comprobar sie sta bien
}

void init_timers(void) {

    /****** Timer para el timeout en RX ******/
    // Retraso de los LEDs

    TA0CTL =
        TASSEL__SMCLK + // clock SMCLK
        MC__UP +        // modo UP
        ID__8;          // /8

    TA0CCTL0 =
        CCIE; // activar int clock

    // Seteamos la constante de tiempo m�ximo del contador
    // Queremos que la unidad b�sica sea 10^-5 segundos (como se pide en el c�digo incluido en la pr�ctica)
    // Lo ponemos a 30 ya que f/100000/8 = 24*10^6/100000/8 = 30
    TA0CCR0 = 30;

}

void init_interrupts(void)
{
    // Timer para el timeout en RX
    NVIC->ICPR[0] |= BIT8; // Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT8; // y habilito las interrupciones del puerto

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**
 * M�TODOS
 */

void set_direction_rx(void)
{
    P3OUT &= ~BIT0;
}

void set_direction_tx(void)
{
    P3OUT |= BIT0;
}

void tx_byte_uac2(uint8_t data)
{
    while(!TXD2_READY); // Esperamos a que est� listo el buffer de transmisi�n
    UCA2TXBUF = data;
}


void set_timer_interrupt(bool enable)
{
    if (enable) {
        TA0CCTL0 |=  CCIE;
    } else {
        TA0CCTL0 &= ~CCIE;
    }
}

void reset_time(void) {
    time_g = 0;
}

bool has_passed(time_t time)
{
    return time >= time_g;
}

bool has_received_byte(void) {
    return has_byte_received_g;
}


//TxPacket() 3 par�metres: ID del Dynamixel, Mida dels par�metres, Instruction byte. torna la mida del "Return packet"
uint8_t tx_instruction(uint8_t module_id, uint8_t parameter_length, uint8_t instruction, uint8_t parameters[16])
{
    uint8_t i, checksum, packet_length;
    uint8_t tx_buffer[32];

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


RxPacket rx_status(void)
{
    RxPacket response;
    uint8_t i, packet_length, checksum;
    time_t timeout = 0;
    bool received_byte;

    set_direction_rx(); //Ponemos la linea half duplex en Rx
    set_timer_interrupt(1);

    for(i = 0; i < 4; i++)
    {
        reset_time();
        received_byte = 0;
        while (!received_byte) // Se_ha_recibido_Byte())
        {
            received_byte = has_received_byte();
            timeout = has_passed(TIMEOUT); // tiempo en decenas de microsegundos
            if (timeout) break; //sale del while
        }
        //sale del for si ha habido Timeout
        if (timeout) break;
        //Si no, es que todo ha ido bien, y leemos un dato:
        response.status[i] = get_read_byte(); //Get_Byte_Leido_UART();
    }//fin del for

    // Continua llegint la resta de bytes del Status Packet
    if (!timeout)
    {
        response.timeout = 0;

        packet_length = response.status[3];

        for(i = 4; i < packet_length; i++) //packet_length; i++)
        {
            reset_time();
            received_byte = 0;
            while (!received_byte) // Se_ha_recibido_Byte())
            {
                received_byte = has_received_byte();
                timeout = has_passed(TIMEOUT); // tiempo en decenas de microsegundos
                if (timeout) break; //sale del while
            }
            //sale del for si ha habido Timeout
            if (timeout) break;
            //Si no, es que todo ha ido bien, y leemos un dato:
            response.status[i] = get_read_byte(); //Get_Byte_Leido_UART();
        }//fin del for
    }
    else
    {
        response.timeout = 1;
    }

    return response;

}




/**
 * MAIN
 */

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer
	init_ucs_24MHz(); // Inicalizar UCS
	init_timers();
	init_interrupts();

}


/**
 * HANDLERS
 */

void TA0_0_IRQHandler(void) {
    set_timer_interrupt(0); // Desactivamos interrupciones

    time_g++; // Ha pasado 1ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    set_timer_interrupt(1);  // Reactivamos interrupciones
}

// TODO no sabemos si este es el handler correcto para las interrupciones de recepcion de datos
void EUSCIA2_IRQHandler(void) {
    //
}
}
