/**
*Simple ACK and test program for the AVR ATMega microcontroller and the nRF24L01+
*
*Hasith Vidanamadura, hasith@vidanamadura.net
*/
#include <avr/io.h>
#include <util/delay.h>

#include "libspi.h"
#include "libnrf24.h"
#include "uart.h"

uint8_t tx_address[5] = {0xDE,0xAD,0xBE,0xEF,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xC3,0xE2,0xE7};
uint8_t buffer[32];


/*
*preamble section for barebones runtime
*/
void __jMain(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
void __jMain(void)
{   
    asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );
    asm volatile ( "clr __zero_reg__" );        // gcc expcets this to be zero.  
    asm volatile ( "rjmp main");                // start main()
} 

/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_csn(uint8_t val){
	if (val)
		PORTD |= _BV(PORTD3);
	else
		PORTD &= ~(_BV(PORTD3));
}
/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_ce(uint8_t val){
	if (val)
		PORTD |= _BV(PORTD4);
	else
		PORTD &= ~(_BV(PORTD4));
}


int main(void){
uint8_t a;
uint8_t i;

unsigned char counter;


  /* set PORTB for output*/
  DDRD &= ~(_BV(PORTD2) | _BV(PORTD4) | _BV(PORTD3));
  DDRD |= _BV(PORTD2);
  DDRD |= (1 << PORTD3 | 1 << PORTD4);


nrf_init(set_ce, set_csn, libspi_tx_blocking, libspi_shift_buffer, libspi_tx_buffer);
libspi_init();

  nrf_config(12,4);
  nrf_set_address(1, tx_address);

  nrf_set_address(0, rx_address);
  uart_init(115200);

nrf_power_radio(MODE_RX);

  nrf_read_register(RX_ADDR_P1, buffer,5);
  uart_tx_string("RX_ADDR_P1:");
  for (i=0;i<5;i++)
          uart_tx_byte_hex(buffer[i]);

uart_tx_string("\n");
  nrf_read_register(RX_ADDR_P0, buffer,5);
  uart_tx_string("RX_ADDR_P0:");
  for (i=0;i<5;i++)
          uart_tx_byte_hex(buffer[i]);
uart_tx_string("\n");



uart_tx_string("Reading register RF_CH:");
  nrf_read_register(RF_CH, buffer,1);
uart_tx_byte_hex(buffer[0]);



uart_tx_string("Reading register STATUS:");
  nrf_read_register(STATUS, buffer,1);
uart_tx_byte_hex(buffer[0]);
uart_tx_string("\n");


uart_tx_string("Reading register CONFIG:");
  nrf_read_register(CONFIG, buffer,1);
uart_tx_byte_hex(buffer[0]);
uart_tx_string("\n");

uart_tx_string("Reading register FIFO_STATUS:");
  nrf_read_register(FIFO_STATUS, buffer,1);
uart_tx_byte_hex(buffer[0]);
uart_tx_string("\n");

uart_tx_string("Reading register EN_AA_STATUS:");
  nrf_read_register(EN_AA, buffer,1);
uart_tx_byte_hex(buffer[0]);
uart_tx_string("\n");



uart_tx_string("Reading register EN_RXADDR_STATUS:");
  nrf_read_register(EN_RXADDR, buffer,1);
uart_tx_byte_hex(buffer[0]);
uart_tx_string("\n");







  while (1)
    {
      //uart_tx_byte_hex(nrf_read_status());
      if (nrf_data_available()){
        uint8_t sz = nrf_get_payload_size();
        uart_tx_string("size:");
        uart_tx_byte_hex(sz);

        if (sz < 32){
        nrf_receive(buffer, sz);
        nrf_config_register(STATUS, 0x70);
        uart_tx_string("received payload: ");
        for (i=0;i<sz;i++)
          uart_tx_byte_hex(buffer[i]);
      uart_tx_string("\n");

      }
    }

    }
 
  return 1;
}


