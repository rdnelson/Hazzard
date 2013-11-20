/**
*bcm_test [num_packets <=255]
*Author: Hasith Vidanamadura, Carleton University
*Description: Test program for nRF transmitter. Assumes payloads are ACKed by receiver.
*/

#include "libnrf24-rpi.h"
#include "libnrf24.h"
#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include  "string.h"
uint8_t data_array[32];
uint8_t rx_address[5] = {0xDE,0xAD,0xBE,0xEF,0xD7};
uint8_t tx_address[5] = {0xE7,0xE7,0xC3,0xE2,0xE7};
#define RF_CHANNEL 12
#define PAYLOAD_SIZE 4

int main(int argc, char** argv){
	uint8_t a;
	uint8_t b;
	uint8_t plos_count = 0;
	uint8_t packet_count = argc > 1 ? atoi(argv[1]) : 100;	
	
	//initialize nRF library for the Raspberry Pi
	libnrf_rpi_init();

	//print init message and STATUS
	printf("Initialized nRF module\n");
	printf("STATUS: %02x\n",nrf_read_status());
	

	//set and verify RF channel
	printf("Setting RF channel 12 and payload size 4:\n");
	nrf_config(RF_CHANNEL, PAYLOAD_SIZE);

	nrf_read_register(RF_CH, &a, 1);
	printf("RF_CH: %02x\n",a);
	

	//turn on radio and set transmitter and receiver addresses
	nrf_power_radio(MODE_TX);
	nrf_set_address(1, tx_address);
	nrf_set_address(0, rx_address);
	
	//read back the receiver address
	nrf_read_register(RX_ADDR_P1, data_array,5);
	printf("RX_ADDR_P1 is:");
	for (a=0;a<5;a++)
		printf("%02X", data_array[a]);
	printf("\n");
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


	//start ping test program with payload
	printf("============Send Begin, %d times=============", packet_count);
	memcpy(data_array, "Hi!", 4);
	
	for(a=0;a<packet_count;a++){
	data_array[3] = a;
	nrf_send(data_array,4);
	//busy wait until transmission complete
	while((b = nrf_is_busy()) == 1);
	//if transmit failed, retry once
	if (b == 255){


	printf("Error: MAX_RT exceeded!\n");
	//read OBSERVE_TX into memory
	nrf_read_register(OBSERVE_TX,&b,1);
	
	//flush the transmitter pipe
	nrf_flush_tx();
	//print OBSERVE_TX
	printf("OBSERVE_TX: %02x\n",b);
	//retry once, if it fails, we have no recourse
	printf("retrying!\n");
	nrf_send(data_array,4);
	while((b = nrf_is_busy()) == 1);
	plos_count++;
	}
	//clear any interrupts that happened
	if (nrf_read_status() & 0x70){
	printf("ACK\n");	
	nrf_config_register(STATUS, 0x70);	
	}
	
	}

	memcpy(data_array, "this is longer!",15);
	nrf_send(data_array,15);


	printf("Done sending! %d lost packets\n", plos_count);
}

