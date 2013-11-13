#include <avr/io.h>
#include <util/delay.h>
#include "libspi.h"
#include "libnrf24.h"
#include "uart.h"

uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t buffer[10];
void __jMain(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
void __jMain(void)
{   
    asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );
    asm volatile ( "clr __zero_reg__" );        // gcc expcets this to be zero.  
    asm volatile ( "rjmp main");                // start main()
} 


void set_csn(uint8_t val){
	if (val)
		PORTD |= _BV(PORTD3);
	else
		PORTD &= ~(_BV(PORTD3));
}

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
  nrf_config(2,4);
  nrf_set_address(1, tx_address);

  nrf_set_address(0, rx_address);
  uart_init(115200);

nrf_power_radio(0);
uart_tx_string("Initialization complete! Status register is: ");
  nrf_read_register(RX_ADDR_P1, buffer,5);
  uart_tx_string("RX_ADDR: \n");
  for (i=0;i<5;i++)
          uart_tx_byte_hex(buffer[i]);
  nrf_read_register(RX_ADDR_P0, buffer,5);
  uart_tx_string("RX_ADDR: \n");
  for (i=0;i<5;i++)
          uart_tx_byte_hex(buffer[i]);
uart_tx_byte_hex(nrf_read_status());



  while (1)
    {
      //uart_tx_byte_hex(nrf_read_status());
      if (nrf_data_available()){
        nrf_receive(buffer);
        uart_tx_string("received payload: \n");
        for (i=0;i<4;i++)
          uart_tx_byte_hex(buffer[i]);

      }
    }
 
  return 1;
}


