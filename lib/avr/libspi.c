//global AVR defines header
#include <avr/io.h>

#include "libspi.h"
//pull in the UART if we want to debug SPI transactions
#ifdef DEBUG_MODE
	#include "uart.h"
#endif
/**
*Defines for SPI pins on the AVR
*/
#define PIN_SCK PORTB5
#define PIN_MOSI PORTB3
#define PIN_MISO PORTB4
#define DDR_SPI DDRB


/**
*Setup the SPI module on the AVR
**/
void libspi_init(){

	//clear SPI bits
	DDR_SPI &= ~(_BV(PIN_SCK) | _BV(PIN_MOSI) | _BV(PIN_MISO) | _BV(PORTB2)); 

	//SCK and MOSI are outputs. SS on the Atmel SPI bus is set to an output.
	DDR_SPI |= _BV(PIN_SCK) | _BV(PIN_MOSI) | _BV(PORTB2);
	

	//prescaler is now FCLK/4 = 4MHz for 16MHz, 2MHz for 8MHz FCLK
	SPCR = (1 << MSTR | 1 << SPE);

	//double speed, SPI clock is now 4MHz for 8MHZ master clock and 8MHz for 16MHZ master clock.
	SPSR |= _BV(SPI2X);

}

/**
*shift a buffer into SPI and get the shifted out contents back
*@param pIn pointer to byte array to transmit
*@param pOut pointer to byte buffer to receive from. Must be len bytes long.
*@param len Size of the transaction in bytes
*@return None
**/
void transmit_sync(uint8_t* pIn, uint8_t* pOut, uint8_t len)
{
	uint8_t i;
	for (i =0 ;i < len; i++){
		SPDR = pIn[i];
    	while((SPSR & (1<<SPIF))==0);
    	pOut[i] = SPDR;
	}

}

/**
*Synchronously transfer out a buffer onto the SPI bus. Use when you don't care about the return value[s]
*@param pStart pointer to byte array to transfer
*@param len length of byte array to transfer
*@return None.
*/
void transfer_sync(uint8_t* pStart, uint8_t len){
	uint8_t i;
	for (i = 0; i < len; i++){
		SPDR = pStart[i];
	 	while ((SPSR & _BV(SPIF)) == 0);
	}
}

/**
*Execute a single byte SPI transfer
*@param b byte to shift out
*@return value of response from SPI slave
*/
uint8_t fast_shift(uint8_t b){
    SPDR = b;
 	while ((SPSR & _BV(SPIF)) == 0);
    return SPDR;

}
