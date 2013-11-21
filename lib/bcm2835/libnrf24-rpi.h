#ifndef _LIBNRF_RPI_H
#define _LIBNRF_RPI_H
#include "libnrf24.h"
#include <stdint.h>
void platform_init();
uint8_t fast_shift(uint8_t);

void transmit_sync(uint8_t* pStart, uint8_t* pOut, uint8_t len);
void transfer_sync(uint8_t* pOut, uint8_t len);

void set_ce(pinmode val);
void set_csn(pinmode val);


#endif
