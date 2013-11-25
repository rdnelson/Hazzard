/**
*bcm_test [num_packets <=255]
*Author: Hasith Vidanamadura, Carleton University
*Description: Test program for driving. Assumes payloads are ACKed by receiver.
*/

#include "libnrf24-rpi.h"
#include "libnrf24.h"
#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include  "string.h"
#include "curses.h"
uint8_t data_array[32];
uint8_t rx_address[5] = {0xDE,0xAD,0xBE,0xEF,0xD7};
uint8_t tx_address[5] = {0xE7,0xE7,0xC3,0xE2,0xE7};
#define FALLOFF_CONSTANT 0.25

#define RF_CHANNEL 12
#define PAYLOAD_SIZE 4

int main(int argc, char** argv){
	uint8_t a;
	uint8_t b;
	uint8_t c;
	int16_t val = 0;
	WINDOW *menu_win;
	uint8_t plos_count = 0;
	uint8_t packet_count = argc > 1 ? atoi(argv[1]) : 100;	
	
	//initialize nRF library for the Raspberry Pi
	nrf_init();

	initscr();
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
	nodelay(stdscr, true);	
	keypad(stdscr, TRUE);
	mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	//print init message and STATUS
	mvprintw(1,0,"Initialized nRF module\n");
	mvprintw(2,0,"STATUS: %02x\n",nrf_read_status());
	

	//set and verify RF channel
	mvprintw(3,0,"Setting RF channel 12 and payload size 4:\n");
	nrf_config(RF_CHANNEL, PAYLOAD_SIZE);

	nrf_read_register(RF_CH, &a, 1);
	mvprintw(4,0,"RF_CH: %02x\n",a);
	

	//turn on radio and set transmitter and receiver addresses
	nrf_power_radio(MODE_TX);
	nrf_set_address(1, tx_address);
	nrf_set_address(0, rx_address);
	
	//read back the receiver address
	nrf_read_register(RX_ADDR_P1, data_array,5);
	mvprintw(5,0,"RX_ADDR_P1 is:");
	for (a=0;a<5;a++)
		mvprintw(6,0,"%02X", data_array[a]);


/*
	//read back the receiver address
	nrf_read_register(RX_ADDR_P0, data_array,5);
	printf("RX_ADDR_P0 is:");
	for (a=0;a<5;a++)
		printf("%02X", data_array[a]);
	printf("\n");

	nrf_read_register(CONFIG, &a, 1);
	printf("CONFIG: %02x\n",a);


	nrf_read_register(EN_AA, &a, 1);
	printf("EN_AA: %02x\n",a);
	nrf_read_register(RF_SETUP, &a, 1);
	printf("RF_SETUP: %02x\n",a);
	nrf_read_register(FIFO_STATUS, &a, 1);
	printf("FIFO_STATUS: %02x\n",a);
	nrf_read_register(SETUP_AW, &a, 1);
	printf("SETUP_AW: %02x\n",a);
	//read back the transmitter address
	nrf_read_register(TX_ADDR, data_array,5);
	printf("TX_ADDR is:");
	for (a=0;a<5;a++)
		printf("%02X", data_array[a]);
	printf("\n");

	*/

	//start ping test program with payload
	mvprintw(7,0,"============Send Begin, %d times=============", packet_count);
	clrtoeol();
	refresh();
	data_array[0] = 0x48;
	
	
	data_array[1] = 10;
	data_array[2] = 0x00;
	data_array[3] = 0x00;
	
	while(1){

	usleep(30000);
	c = getch();

	switch(c){
		case (0x03):
			val = val + 10;
			if (val > 255) val = 255;
			break;
		case (0x02):
			val = val - 10;
			if (val < -255) val = -255;
			break;
		default:
	val = val * (1 - FALLOFF_CONSTANT);

		break;
	}

	mvprintw(10,0, "Throttle value:      ", val);

	mvprintw(10,0, "Throttle value: %03d", val);
	data_array[1]  = (uint8_t)val;
	data_array[2]  = (uint8_t)(val >> 8);

	nrf_send(data_array,4);
	//busy wait until transmission complete
	while((b = nrf_is_busy()) == 1);
	//if transmit failed, retry once
	if (b == 255){


	mvprintw(8,0,"Error: MAX_RT exceeded!\n");
	//read OBSERVE_TX into memory
	nrf_read_register(OBSERVE_TX,&b,1);
	
	//flush the transmitter pipe
	nrf_flush_tx();
	//print OBSERVE_TX
	mvprintw(9,0,"OBSERVE_TX: %02x\n",b);
	//retry once, if it fails, we have no recourse
	mvprintw(10,0,"retrying!\n");
	nrf_send(data_array,4);
	while((b = nrf_is_busy()) == 1);
	plos_count++;
	}
	//clear any interrupts that happened
	if (nrf_read_status() & 0x70){
	mvprintw(8,0,"ACK\n");	
	nrf_config_register(STATUS, 0x70);	
	}
	clrtoeol();
	refresh();	
	}

	endwin();

       mvprintw(1,0,"Done sending! %d lost packets\n", plos_count);
}

