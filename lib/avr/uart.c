#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "uart.h"
//for handling multiple transmit queues.
static uint8_t* pCurrent[4];
static uint8_t* queue_current;
static uint8_t* queue_in;

//lookup table for fast binary->hex conversions
const char LUT[16] = "0123456789ABCDEF";


/**
*Sets up the UART on the AVR
*@param baudrate data TX rate to set the device to
*@return None
*/
void uart_init(uint32_t baudrate){

    //Pulled from AVR reference code
    uint8_t use_u2x = 1;
    uint16_t baud_setting = 0;

try_again:
 
    if (use_u2x) {
        UCSR0A = 1 << U2X0;
        baud_setting = (F_CPU / 4 / baudrate - 1) / 2;
    }
    else {
    
        UCSR0A = 0;
        baud_setting = (F_CPU / 8 / baudrate - 1) / 2;
    }
 
    if ((baud_setting > 4095) && use_u2x){
        use_u2x = 0;
        goto try_again;
    }

    UBRR0H = baud_setting >> 8;
    UBRR0L = (uint8_t) baud_setting;
    UCSR0B |= (1<< RXEN0) | (1<<TXEN0)/* |(1 <<RXCIE0)*/ | (0   << UDRIE0);
    sei(); //reeenable global interrupts


}

/**
*ISR to do asynch receive. NOT IMPLEMENTED IN CURRENT REVISION
**/
ISR(USART_RX_vect){
/*	if (*queue_current == NULL){
		queue_current++;
		if (queue_current == pCurrent + 3 || queue_current==)
			 UCSR0B &= ~(1 << UDRIE0);
	}
	else
		if (*queue_current != 0)
			UDR0 = queue_current++;
*/
}
/**
*Synchronously transfer out a string (or byte array)
*@param pStart pointer to null-terminated string
*@return None
**/
void uart_tx_string(uint8_t* pStart){
    //iterate through the string and increment the
    //pointer on every access
	while(*pStart != 0){
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = *pStart++;
	}
}

/**
*Fast inline single byte transmitter
*@param byteval byte to transmit
*@return None
**/
inline void uart_tx_byte(uint8_t byteval){

    //busy wait for the transmitter to be idle
	while (!(UCSR0A & (1 << UDRE0)));
    //send the byte
	UDR0 = byteval;

}

/**
*Fast helper function to send a byte as hex i.e: "FA"
*@param byteval bytevalue to send as hex
*@return None
*/
void uart_tx_byte_hex(uint8_t byteval){
    //transmit the high nibble first
    uart_tx_byte(LUT[(byteval >> 4) & 0x0F]);
	//and then the low nibble
    uart_tx_byte(LUT[byteval & 0x0F]);


}