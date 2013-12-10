/**
*Super lightweight UART transmitter library
*Used primarily for debugging purposes
**/
#ifndef _UART_H
#define _UART_H


void uart_tx_byte(uint8_t pByte);
void uart_tx_string(uint8_t* pStart);
void uart_init(uint32_t baudrate);
void uart_tx_byte_hex(uint8_t byteval);



#endif
