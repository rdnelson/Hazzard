#define _BV(a)	(1 << a)
#include "libnrf24-rpi.h"
#include "libnrf24.h"
#include "bcm2835.h"
#include "string.h"
#include "stdio.h"

#define CE_PIN		RPI_GPIO_P1_26
#define CSN_PIN		RPI_GPIO_P1_24

//buffers for SPI transactions
uint8_t spi_buffer_in[33];
uint8_t spi_buffer_out[33];




/**
*Platform initialization for the bcm2835 (Raspberry Pi). Sets up GPIO and SPI
*@return None
**/
void platform_init(){
	//initialize the bcm2835 library
	bcm2835_init();
	//set the CE pin GPIO
	bcm2835_gpio_fsel(RPI_GPIO_P1_26,BCM2835_GPIO_FSEL_OUTP); //CE
	//configure the SPI to MSB first mode
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	//set the SPI interface to mode0
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); 
	//set the device speed to 8MHz
	bcm2835_spi_setClockSpeed(BCM2835_SPI_SPEED_8MHZ); 
	//disable the built in SPI toggle, because we'll be doing it on our own
 	bcm2835_spi_begin(BCM2835_SPI_CS_NONE);
 	//reconfigure SPI_CS as a GPIO so we can can access it
	bcm2835_gpio_fsel(RPI_GPIO_P1_24,BCM2835_GPIO_FSEL_OUTP); //CS

}
/**
*Execute a single byte SPI transfer
*@param b byte to shift out
*@return value of response from SPI slave
*/
uint8_t fast_shift(uint8_t in){
	uint8_t a;
	a =  bcm2835_spi_transfer(in);

	return a;
}
/**
*shift a buffer into SPI and get the shifted out contents back
*@param pIn pointer to byte array to transmit
*@param pOut pointer to byte buffer to receive from. Must be len bytes long.
*@param len Size of the transaction in bytes
*@return None
**/
void transmit_sync(uint8_t* pStart, uint8_t* pOut, uint8_t len){
	bcm2835_spi_transfernb((char*)pStart, (char*)spi_buffer_in, len);
	memcpy(pOut,spi_buffer_in, len);

}

/**
*Synchronously transfer out a buffer onto the SPI bus. Use when you don't care about the return value[s]
*@param pStart pointer to byte array to transfer
*@param len length of byte array to transfer
*@return None.
*/
void transfer_sync(uint8_t* pOut, uint8_t len){
	
	bcm2835_spi_writenb((char*)pOut, len);
}

/**
*Set nRF CE pin
*@param val value to set CE pin to.
*@return None.
**/
void set_ce(pinmode val){
	if (val == HIGH)
		  bcm2835_gpio_write(CE_PIN, HIGH);
	else
		  bcm2835_gpio_write(CE_PIN, LOW);

}
/**
*Set SPI CS pin
*@param val value to set CS pin to.
*@return None.
**/
void set_csn(pinmode val){
	if (val == HIGH)
		  bcm2835_gpio_write(CSN_PIN, HIGH);
	else
		  bcm2835_gpio_write(CSN_PIN, LOW);

}
