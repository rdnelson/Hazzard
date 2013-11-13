#ifndef _HAZZARD_LIBSPI_H__
#define _HAZZARD_LIBSPI_H__

void libspi_init();
inline uint8_t libspi_tx_blocking(uint8_t b);
void libspi_tx_buffer(uint8_t* pStart, uint8_t len);
void libspi_shift_buffer(uint8_t* pIn, uint8_t* pOut, uint8_t len);



#endif