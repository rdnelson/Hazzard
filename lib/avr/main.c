/**
*Master controller firmware for the AVR ATMega microcontroller and the nRF24L01+
*Project Hazzard
*Hasith Vidanamadura, hasith@vidanamadura.net
*/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libspi.h"
#include "libnrf24.h"
#include "uart.h"

//#define DEBUG_MODE
//#ifdef DEBUG_MODE
#define PRINT_REG(reg) nrf_read_register(reg, buffer, 1);\
						uart_tx_string("Reading register " #reg ": 0x");\
						itoa(buffer[1], buffer+1, 16);\
						uart_tx_string(buffer + 1);\
						uart_tx_byte('\n');\
//#endif

//defined addresses for the radios/cars
uint8_t tx_address[5] = {0xDE,0xAD,0xBE,0xEF,0xD7};

//nRFnet address
uint8_t rx_address[5] = {0x0A,0xEF,0xBE,0xAD,0xDE};

//temporary 32-byte buffer for receiving datagrams
uint8_t buffer[32];

//command definitions
#define CMD_DRIVE 0x48
#define CMD_SETPULSE 0x49


#define FREQ_100HZ 39

void platform_init()
{
    /* set PORTB for output*/
    DDRD &= ~(_BV(PORTD2) | _BV(PORTD4) | _BV(PORTD3));
    //set pullups on PORTD2
    PORTD |= _BV(PORTD2);
    DDRD |= (1 << PORTD3 | 1 << PORTD4);
    DDRB |= _BV(PORTB1) | _BV(PORTB2);

    DDRD |= _BV(PORTD5) | _BV(PORTD6);

    // set PWM for 25% duty cycle @ 16bit

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1);
    // set none-inverting mode
    TCCR1B = 0;

    // set timer 1 prescale factor to 64
    TCCR1B |= _BV(CS11);
    TCCR1B |= _BV(CS10);

    // put timer 1 in 8-bit phase correct pwm mode
    TCCR1A |= _BV(WGM10);


    //set up the pulse train for 100Hz
    TCNT2 = 0;
    OCR2A = 39;
    //waveform generation mode, clear counter on top
    TCCR2A |= _BV(WGM21);
    //divide CLK by 1024, increments every 128 usec
    TCCR2B |=_BV(CS22) | _BV(CS21) | _BV(CS20);

    //enable Compare interrupt
    TIMSK2 |= _BV(OCIE2A);

    libspi_init();

    EIMSK |= _BV(INT0);  //Enable INT0
    EICRA |= _BV(ISC01); //Trigger on falling edge of INT0
    //enable IR_SIG output
    DDRD |= _BV(PORTD7);
    PORTD |= _BV(PORTD7);

}


/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_csn(pinmode mode)
{
    if (mode == HIGH)
        PORTD |= _BV(PORTD3);
    else
        PORTD &= ~(_BV(PORTD3));
}
/*
*Function for setting the CS pin
*@param val value to set the pin. 0 is LOW, non-zero is HIGH
*/
void set_ce(pinmode mode)
{
    if (mode == HIGH)
        PORTD |= _BV(PORTD4);
    else
        PORTD &= ~(_BV(PORTD4));
}


int main(void)
{
    uint8_t a;
    uint8_t i;

    unsigned char counter;




    nrf_init();


    nrf_config(12,4);
    nrf_set_address(1, tx_address);
    nrf_power_radio(MODE_RX);

    nrf_set_address(0, rx_address);
#ifdef DEBUG_MODE
    uart_init(115200);

    nrf_read_register(RX_ADDR_P1, buffer,5);
    uart_tx_string("RX_ADDR_P1:");
    for (i=0; i<5; i++)
        uart_tx_byte_hex(buffer[i]);

    uart_tx_string("\n");
    nrf_read_register(RX_ADDR_P0, buffer,5);
    uart_tx_string("RX_ADDR_P0:");
    for (i=0; i<5; i++)
        uart_tx_byte_hex(buffer[i]);
    uart_tx_string("\n");



    PRINT_REG(RF_CH);
    PRINT_REG(STATUS);
    PRINT_REG(EN_AA);
    PRINT_REG(FIFO_STATUS);
#endif
    sei();





    while (1);
}

/*Timer 2 compare interrupt
*Fires on OC2A interrupt
**/
ISR(TIMER2_COMPA_vect  )
{
    //flip pin every time OC2A is hit
    PORTD ^= _BV(PORTD7);
}


/**
*Sets PWM on forward channel
*@param val 0-255 value to set PWM channel
*@return None
*/
inline void set_pwm_fwd(uint8_t val)
{

    //if PWM is 0, stop output on pin compare
    if (val == 0) {
        TCCR1A &=  ~_BV(COM1A1);
        PORTB  &= ~_BV(PORTB1);
    } else {

        TCCR1A |= _BV(COM1A1);
        OCR1A =  val; // else, set duty cycle
    }
}

/**
*Sets PWM on reverse channel
*@param val 0-255 value to set PWM channel
*@return None
*/
inline void set_pwm_rev(uint8_t val)
{

    //if PWM is 0, stop output on pin compare
    if (val == 0) {
        TCCR1A &=  ~_BV(COM1B1);
        PORTB  &= ~_BV(PORTB2);
    } else {

        TCCR1A |= _BV(COM1B1);
        OCR1B = val; // else, set duty cyclr

    }
}

/**
*Command handler for incoming data
*@param pCmd pointer to command in memory
*/
void command_handler(uint8_t* pCmd)
{

    int8_t turn_val;
    int16_t drive_offset;
    switch(*pCmd){
    
    	case (CMD_DRIVE):

	//calculate throttle and turn information.
        drive_offset =   *(pCmd+1) | *(pCmd+2) << 8;
	turn_val = *(pCmd+3);
        
#ifdef DEBUG_MODE
	//if in debug mode, print out received packet on UART
        uart_tx_string("<INFO>drive command received: velocity = ");
        itoa(drive_offset, &buffer[6], 10);
        uart_tx_string(&buffer[6]);

        uart_tx_string(" turn = ");
        itoa(turn_val, &buffer[6], 10);
        uart_tx_string(&buffer[6]);
        uart_tx_byte('\n');

#endif


        //if stop is received, shut off the PWM generators
        if (drive_offset == 0) {

            set_pwm_fwd(0);
            set_pwm_rev(0);

        } else if (drive_offset > 0) {

            //turn off the reverse PWM channel, set forward PWM
            set_pwm_rev(0);
            set_pwm_fwd(drive_offset);

        } else {
            //turn off the forward PWM, set reverse PWM
            set_pwm_fwd(0);
            set_pwm_rev(-drive_offset);


        }

        //turn handler. If (-), turn right
        if (turn_val < 0) {
            PORTD |= _BV(PORTD5);
            PORTD &= ~_BV(PORTD6);
        } else if (turn_val == 0) {
            //if turn == 0, turn off both output lines
            PORTD &= ~_BV(PORTD5);
            PORTD &= ~_BV(PORTD6);
        }


        else {
            //if turn is positive, go left
            PORTD  |= _BV(PORTD6);
            PORTD &= ~_BV(PORTD5);
        }
	break;
	case (CMD_SETPULSE):
	OCR2A = *(pCmd +1)  * FREQ_100HZ;
	


    }
}


/**
*Interupt handler for INT0, connected to the nRF radio module
*Fires when a data packet is ready
*/
ISR(INT0_vect)
{
    uint8_t i;
    uint8_t sz;
    while (nrf_data_available()) {
        sz = nrf_get_payload_size();
#ifdef DEBUG_MODE
        uart_tx_string("size:");
        uart_tx_byte_hex(sz);
        uart_tx_byte(' ');
#endif
        if (sz < 32) {
            nrf_receive(buffer, sz);
            //ACK the packet received, so the radio can clear the FIFO
            nrf_config_register(STATUS, 0x70);
#ifdef DEBUG_MODE
            uart_tx_string("received payload: ");
            for (i=0; i<sz; i++)
                uart_tx_byte_hex(buffer[i]);
            uart_tx_string("\n");
#endif
            //Call the command handler
            command_handler(buffer);
        }
    }
}
