#include <stdio.h>
#include <stdint.h>
#include "msp.h"
#include "lib_PAE2.h"

#define TXD2_READY (UCA2IFG & UCTXIFG)

#define MAX_PARAMETER_LENGTH 16
#define READ_DATA 0x02
#define WRITE_DATA 0x03
#define TIMEOUT 1000

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
    bool timeout;
    bool checksum_correct;
} RxPacket;

/**
 * INITS
 */

void init_uart(void)
{
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; /**
                                 * UCSYNC=0 mode asíncron
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
    P3SEL0 |= BIT2 | BIT3; //I/O funció: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3); //Configurem pin de selecció del sentit de les dades Transmissió/Recepeció
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la línia de comunicacions sèrie
    UCA2IE |= UCRXIE; //Això només s’ha d’activar quan tinguem la rutina de recepció

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

    // Seteamos la constante de tiempo máximo del contador
    // Queremos que la unidad básica sea 10^-3 segundos
    // Lo ponemos a 30 ya que f/1000/8 = 24*10^6/1000/8 = 3000
    TA0CCR0 = 3000;

}

void init_interrupts(void)
{
    // Timer para el timeout en RX
    NVIC->ICPR[0] |= BIT8; // Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT8; // y habilito las interrupciones del puerto

    NVIC->ICPR[0] |= 1<<0x12;
    NVIC->ISER[0] |= 1<<0x12;

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**
 * MÉTODOS
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
    while(!TXD2_READY); // Esperamos a que esté listo el buffer de transmisión
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
    return time < time_g;
}

bool has_received_byte(void) {
    return has_byte_received_g;
}

uint8_t get_read_byte(void) {
    // Reseteamos el flag de alerta de byte
    has_byte_received_g = 0;
    // Devolvemos el byte recibido
    return byte_received_g;
}


//TxPacket() 3 paràmetres: ID del Dynamixel, Mida dels paràmetres, Instruction byte. torna la mida del "Return packet"
// Retorna 0 si hi ha error
uint8_t tx_instruction(uint8_t module_id, uint8_t parameter_length, uint8_t instruction, uint8_t parameters[16])
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
    tx_buffer[2] = module_id; //ID del mòdul al que volem enviar el missatge
    tx_buffer[3] = parameter_length + 2; //Length(Parameter,Instruction,Checksum)
    tx_buffer[4] = instruction; //Instrucció que enviem al Mòdul
    for(i = 0; i < parameter_length; i++) //Comencem a generar la trama que hem d’enviar
    {
        tx_buffer[i+5] = parameters[i];
    }

    // Calcular checksum
    checksum = 0;
    packet_length = parameter_length+4+2;
    for(i = 2; i < packet_length-1; i++) //Càlcul del checksum
    {
        checksum += tx_buffer[i];
    }
    tx_buffer[i] = ~checksum; //Escriu el Checksum (complement a 1)

    // Enviar los datos
    for(i = 0; i < packet_length; i++) //Aquest bucle és el que envia la trama al Mòdul Robot
    {
        tx_byte_uac2(tx_buffer[i]);
    }

    int dummy=1;
    while(UCA2STATW & UCBUSY){ //Espera fins que s’ha transmès el últim byte
        dummy++;
    }
    set_direction_rx(); //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta

    return packet_length;

}


RxPacket rx_status(void) {

    RxPacket response;
    uint8_t i, packet_length, checksum;
    bool timeout = 0;
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
            timeout = has_passed(TIMEOUT); // tiempo en ms
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

        for(i = 4; i < packet_length + 4; i++) //packet_length; i++)
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
        }

        // Comprobar checksum sea correcto
        // Atención! A pesar de que la documentación pone que hay que sumar "instruction",
        // lo que hay que sumar es "error"
        checksum = 0;
        for (i = 2; i < packet_length + 3; i++) {
            checksum += response.status[i];
        }

        response.checksum_correct = (checksum + response.status[packet_length+3] + 1) == 0;

    }
    else
    {
        response.timeout = 1;
    }

    return response;

}

/*
 * Returns true if succesful
 */
bool set_led(bool on) {

    uint8_t parameters[2] = {0x19, (on?1:0)};
    bool correct;
    RxPacket response;

    correct = (tx_instruction(2, 2, WRITE_DATA, parameters) != 0);

    response = rx_status();

    if (response.timeout || !response.checksum_correct) {
        return 0;
    }

    return 1;

}

// wheel_id {2, 3}
// direction {0, 1}
// speed [0, 1022]
bool rotate_wheel(uint8_t wheel_id, bool direction, uint16_t speed) {

    if (speed > 1022) {
        return 0;
    }

    // Dynamixel uses 0 for unlimited power
    // 1 stopped
    // 2-1023 linear speed
    speed ++;

    uint8_t parameters[5];
    bool correct;
    RxPacket response;

    //////////////////////////////////////////////////////////////////////////////

    // CW and CCW angle limits to 0 so no max angle
    parameters[0] = 0x06;
    parameters[1] = 0;
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;

    // Do the thing(TM)
    correct = (tx_instruction(wheel_id, 5, WRITE_DATA, parameters) != 0);

    response = rx_status();

    if (response.timeout || !response.checksum_correct) {
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////

    parameters[0] = 0x20;
    parameters[1] = speed & 0xff; // Cogemos los 8 bits inferiores
    parameters[2] = (speed >> 8); // Cogemos los 8 bits superiores
    parameters[2] |= (direction ? 1 : 0) * BIT2; // Cambiamos el bit 10 a 1 o 0 según dirección

    correct = (tx_instruction(wheel_id, 3, WRITE_DATA, parameters) != 0);

    response = rx_status();

    if (response.timeout || !response.checksum_correct) {
        return 0;
    }

    return correct;

}

// direction: true -> robot-forward, false -> robot-backward
bool rotate_left(bool direction, uint16_t speed) {
    return rotate_wheel(0x03, direction, speed);
}

// direction: true -> robot-forward, false -> robot-backward
bool rotate_right(bool direction, uint16_t speed) {
    return rotate_wheel(0x02, ~direction, speed);
}

uint8_t has_obstacle(void) {

    uint8_t parameters[1] = {0x20};
    bool correct;
    RxPacket response;

    correct = (tx_instruction(100, 1, READ_DATA, parameters) != 0);

    response = rx_status();

    if (response.timeout || !response.checksum_correct) {
        return -1;
    }

    return response.status[5];

}




/**
 * MAIN
 */

void main(void)
{

    uint8_t ho;
    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer
	init_ucs_24MHz(); // Inicalizar UCS
	init_timers();
	init_uart();
	init_interrupts();
	while (1) {
	    ho = has_obstacle();
	    if (ho) {
	        set_led(1);
	    } else {
	        set_led(0);
	    }
	}
}


/**
 * HANDLERS
 */

// Clock
void TA0_0_IRQHandler(void) {
    set_timer_interrupt(0); // Desactivamos interrupciones

    time_g++; // Ha pasado 1ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    set_timer_interrupt(1);  // Reactivamos interrupciones
}

// UART
void EUSCIA2_IRQHandler(void) {
    UCA2IE &= ~UCRXIE;
    byte_received_g = UCA2RXBUF;
    has_byte_received_g = 1;
    UCA2IE |= UCRXIE;
}
