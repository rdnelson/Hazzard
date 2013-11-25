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
  DDRD |= (1 << PORTD3 | 1 << PORTD4 | _BV(PORTD5) | _BV(PORTD6));


  PORTD |= _BV(PORTD3);
  _delay_ms(500);

  PORTD &= ~_BV(PORTD3);
  _delay_ms(1);



  PORTD |= _BV(PORTD4);
  _delay_ms(500);

  PORTD &= ~_BV(PORTD4);
  _delay_ms(1);

  PORTD |= _BV(PORTD5);
  _delay_ms(500);

  PORTD &= ~_BV(PORTD5);
  _delay_ms(1);

  PORTD |= _BV(PORTD6);
  _delay_ms(500);

  PORTD &= ~_BV(PORTD6);
  _delay_ms(1);

}


