/**
*Simple ACK and test program for the AVR ATMega microcontroller and the nRF24L01+
*
*Hasith Vidanamadura, hasith@vidanamadura.net
*/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libspi.h"
#include "libnrf24.h"
#include "uart.h"
#define PRINT_REG(reg) nrf_read_register(reg, buffer, 1);\
						uart_tx_string("Reading register " #reg ": 0x");\
						itoa(buffer[1], buffer+1, 16);\
						uart_tx_string(buffer + 1);\
						uart_tx_byte('\n');\
						

uint8_t tx_address[5] = {0xDE,0xAD,0xBE,0xEF,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xC3,0xE2,0xE7};
uint8_t buffer[32];


#define CMD_DRIVE 0x48

void platform_init(){
  /* set PORTB for output*/
  DDRD &= ~(_BV(PORTD2) | _BV(PORTD4) | _BV(PORTD3));
  //set pullups on PORTD2
  PORTD |= _BV(PORTD2);
  DDRD |= (1 << PORTD3 | 1 << PORTD4);
  DDRB |= _BV(PORTB1) | _BV(PORTB2);
  
    // set PWM for 25% duty cycle @ 16bit

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1);
    // set none-inverting mode

    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM12);
    // set Fast PWM mode using 8-bit PWM as TOP
    
    TCCR1B |= (1 << CS10);
  
  libspi_init();

    EIMSK |= _BV(INT0);  //Enable INT0
    EICRA |= _BV(ISC01); //Trigger on falling edge of INT0


}


/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_csn(pinmode mode){
	if (mode == HIGH)
		PORTD |= _BV(PORTD3);
	else
		PORTD &= ~(_BV(PORTD3));
}
/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_ce(pinmode mode){
	if (mode == HIGH)
		PORTD |= _BV(PORTD4);
	else
		PORTD &= ~(_BV(PORTD4));
}


int main(void){
uint8_t a;
uint8_t i;

unsigned char counter;




nrf_init();


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



PRINT_REG(RF_CH);
PRINT_REG(STATUS);
PRINT_REG(EN_AA);
PRINT_REG(FIFO_STATUS);

 sei();





  while (1);
}
inline void set_pwm_fwd(uint8_t val){
	if (val == 0)
	{
		TCCR1A |=  ~_BV(COM1A1);
		PORTB  |= ~_BV(PORTB1);
	}
	else{
	
                  TCCR1A |= _BV(COM1A1);
                  OCR1A = 255 - val; // set pwm duty

	
	}
} 

inline void set_pwm_rev(uint8_t val){
	if (val == 0)
	{
		TCCR1A |=  ~_BV(COM1B1);
		PORTB  |= ~_BV(PORTB2);
	}
	else{
	
                  TCCR1A |= _BV(COM1B1);
                  OCR1B = 255 - val; // set pwm duty

	
	}
} 
void command_handler(uint8_t* pCmd){

	if (*pCmd == CMD_DRIVE){
		uart_tx_string("drive command received");
		int16_t drive_offset =   *(pCmd+1) | *(pCmd+2) << 8;
		itoa(drive_offset, &buffer[6], 10);
		uart_tx_string(&buffer[6]);
		int8_t turn_val = *(pCmd+3);	
		if (drive_offset == 0){
		
			set_pwm_fwd(0);
			//set_pwm_rev(0);
		
		} else if (drive_offset > 0){
			//set_pwm_rev(0);		
			set_pwm_fwd(drive_offset);
		
		} else
		{
	//		set_pwm_fwd(0);		
	//		set_pwm_rev(-drive_offset);
		
		
		}
		
	}
}

ISR(INT0_vect){
	uint8_t i;
       
        uint8_t sz;
        while (nrf_data_available()){
        sz = nrf_get_payload_size();
        uart_tx_string("size:");
        uart_tx_byte_hex(sz);
	uart_tx_byte(' ');
        if (sz < 32){
        nrf_receive(buffer, sz);
        nrf_config_register(STATUS, 0x70);
        uart_tx_string("received payload: ");
        for (i=0;i<sz;i++)
          uart_tx_byte_hex(buffer[i]);
      uart_tx_string("\n");
      command_handler(buffer);
	}
	}
}
