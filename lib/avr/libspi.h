#ifndef _HAZZARD_LIBSPI_H__
#define _HAZZARD_LIBSPI_H__

void libspi_init();
inline uint8_t  fast_shift(uint8_t b);
void transfer_sync(uint8_t* pStart, uint8_t len);
void transmit_sync(uint8_t* pIn, uint8_t* pOut, uint8_t len);



#endif
