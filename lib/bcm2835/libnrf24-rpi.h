#ifndef _LIBNRF_RPI_H
#define _LIBNRF_RPI_H
#include <stdint.h>
void libnrf_rpi_init();
uint8_t libnrf_spi_fast_shift();

void libnrf_spi_transmit_sync(uint8_t* pStart, uint8_t* pOut, uint8_t len);
void libnrf_spi_transfer_sync(uint8_t* pOut, uint8_t len);

void set_ce(uint8_t val);
void set_csn(uint8_t val);


#endif