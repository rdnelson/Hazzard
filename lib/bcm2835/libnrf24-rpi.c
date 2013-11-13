#include "libnrf24-rpi.h"
#include "libnrf24.h"

#include <stdio.h>
#include <stdlib.h>

#define CE_PIN RPI_GPIO_P1_26
uint8_t spi_buffer_in[32];
uint8_t spi_buffer_out[33];


int main(int argc, char** argv){

	libnrf_rpi_init();
	return 0;
}



void libnrf_rpi_init(){
	bcm2835_init();
	bcm2835_gpio_fsel(RPI_GPIO_P1_24,BCM2835_GPIO_FSEL_OUTP); //CSN
	bcm2835_gpio_fsel(RPI_GPIO_P1_26,BCM2835_GPIO_FSEL_OUTP); //CE
 	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); 
    bcm2835_spi_setClockSpeed(BCM2835_SPI_SPEED_8MHZ); 
    bcm2835_spi_begin(RPI_GPIO_P1_24);
    //wait for startup of SPI
    delay(5);
    libnrf_init(set_ce, set_csn, libnrf_spi_fast_shift, libnrf_spi_transmit_sync, libnrf_spi_transfer_sync);
    libnrf_read_status();
}
uint8_t libnrf_spi_fast_shift(uint8_t in){

	return bcm2835_spi_transfer(in);

}

void libnrf_spi_transmit_sync(uint8_t* pStart, uint8_t* pOut, uint8_t len){
	bcm2835_spi_transfernb(pStart, pOut, len);

}
void libnrf_spi_transfer_sync(uint8_t* pOut, uint8_t len){

	bcm2835_spi_writenb(pOut, len);


void set_ce(uint8_t val){
	if (val)
		  bcm2835_gpio_write(CE_PIN, HIGH);
	else
		  bcm2835_gpio_write(CE_PIN, HIGH);

}
//does nothing, because the BCM2835 library takes care of SPI 
void set_csn(uint8_t val){
	return
}
