#define _BV(a)	(1 << a)
#include "libnrf24-rpi.h"
#include "libnrf24.h"
#include "bcm2835.h"
#include "string.h"
#include "stdio.h"
#define CE_PIN		RPI_GPIO_P1_26
#define CSN_PIN	RPI_GPIO_P1_24
uint8_t spi_buffer_in[33];
uint8_t spi_buffer_out[33];





void platform_init(){
	bcm2835_init();

	bcm2835_gpio_fsel(RPI_GPIO_P1_26,BCM2835_GPIO_FSEL_OUTP); //CE
 	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); 

    bcm2835_spi_setClockSpeed(BCM2835_SPI_SPEED_8MHZ); 
    bcm2835_spi_begin(BCM2835_SPI_CS_NONE);
   bcm2835_gpio_fsel(RPI_GPIO_P1_24,BCM2835_GPIO_FSEL_OUTP); //CS

}
uint8_t fast_shift(uint8_t in){
	uint8_t a;
	a =  bcm2835_spi_transfer(in);

	return a;
}

void transmit_sync(uint8_t* pStart, uint8_t* pOut, uint8_t len){
	bcm2835_spi_transfernb((char*)pStart, (char*)spi_buffer_in, len);
	memcpy(pOut,spi_buffer_in, len);

}
void transfer_sync(uint8_t* pOut, uint8_t len){
	
	bcm2835_spi_writenb((char*)pOut, len);
}
void set_ce(pinmode val){
	if (val == HIGH)
		  bcm2835_gpio_write(CE_PIN, HIGH);
	else
		  bcm2835_gpio_write(CE_PIN, LOW);

}
//does nothing, because the BCM2835 library takes care of SPI 
void set_csn(pinmode val){
	if (val == HIGH)
		  bcm2835_gpio_write(CSN_PIN, HIGH);
	else
		  bcm2835_gpio_write(CSN_PIN, LOW);

}
