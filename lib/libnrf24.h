#ifndef _LIBNRF24_H
#define _LIBNRF24_H
/**
*LibNRF24: portable library for interfacing with the nRF24L01+ series of modules
*@author Hasith Vidanamadura
*This library simplifies the task of configuring and transmitting data over an nRF link.
*It is designed to be portable and interfaces with six external functions for setting up the necessary IO,
*setting pins for SPI and radio functions, and for the SPI interface. The code is portable to any architecture  
*supported by GCC.
*Usage:
*The function calls should follow an order:
* - nrf_init(), followed by nrf_config(channel, payload-size);
* - nrf_set_address() for both TX and RX modes
* - nrf_set_address_suffix if using devices on same "subnet"
* - nrf_send()
*Addresses:
*The currently accepted convention in a five-byte address is as follows
* <4 byte prefix><single byte suffix> where the prefix determines the subnet, and the suffix the hostname.
*/

#include <stdint.h>
#include <stdlib.h>
/* Memory Map */
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

/* Bit Mnemonics */
#define RF_DR_LOW 5


/* configuratio nregister */
#define MASK_RX_DR 6
#define MASK_TX_DS 5
#define MASK_MAX_RT 4
#define EN_CRC 3
#define CRCO 2
#define PWR_UP 1
#define PRIM_RX 0


/* enable auto acknowledgment */
#define ENAA_P5 5
#define ENAA_P4 4
#define ENAA_P3 3
#define ENAA_P2 2
#define ENAA_P1 1
#define ENAA_P0 0

/* enable rx addresses */
#define ERX_P5 5
#define ERX_P4 4
#define ERX_P3 3
#define ERX_P2 2
#define ERX_P1 1
#define ERX_P0 0

/* setup of address width */
#define AW 0 /* 2 bits */

/* setup of auto re-transmission */
#define ARD 4 /* 4 bits */
#define ARC 0 /* 4 bits */

/* RF setup register */
#define PLL_LOCK 4
#define RF_DR 3
#define RF_PWR 1 /* 2 bits */

/* general status register */
#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4
#define RX_P_NO 1 /* 3 bits */
#define TX_FULL 0

/* transmit observe register */
#define PLOS_CNT 4 /* 4 bits */
#define ARC_CNT 0 /* 4 bits */

/* fifo status */
#define TX_REUSE 6
#define FIFO_FULL 5
#define TX_EMPTY 4
#define RX_FULL 1
#define RX_EMPTY 0

/* dynamic length */
#define DPL_P0 0
#define DPL_P1 1
#define DPL_P2 2
#define DPL_P3 3
#define DPL_P4 4
#define DPL_P5 5

#define EN_DPL 2

/* SPI instructions */
#define I_R_REGISTER    0x00
#define I_W_REGISTER    0x20
#define I_REGISTER_MASK 0x1F
#define I_R_RX_PAYLOAD  0x61
#define I_W_TX_PAYLOAD  0xA0
#define I_W_TX_PAYLOAD_NOACK  0xB0

#define I_FLUSH_TX      0xE1
#define I_FLUSH_RX      0xE2
#define I_REUSE_TX_PL   0xE3
#define I_NOP           0xFF
#define I_R_RX_PL_WID	0x60 

#define NRF_CHANNEL 3

/**
*Typedef for NRF transmitter mode
*
*/
typedef enum NRF_MODE{
	MODE_RX,
	MODE_TX,
} nrf_mode_t;

/**
*IO mode for functions (read|write)
**/
typedef enum {R =0, W=1} io_mode_t;
/**
*Typedef for pins
**/
typedef enum {LOW =0, HIGH=1} pinmode;

void nrf_init();
void nrf_config(uint8_t channel, uint8_t payload_sz);
void nrf_config_register(uint8_t reg, uint8_t value);
uint8_t nrf_read_status();
void nrf_read_register(uint8_t reg, uint8_t* pStart, uint8_t len);
void nrf_write_register(uint8_t reg, uint8_t* pStart, uint8_t len);
void nrf_power_up();
void nrf_power_down();
void nrf_flush_tx();

uint8_t nrf_data_available();
uint8_t nrf_get_payload_size();

void nrf_set_address(uint8_t tx, uint8_t* pStart);
void nrf_set_address_suffix(uint8_t suffix);


void nrf_send(uint8_t* pStart, uint8_t uSz);
void nrf_power_radio(nrf_mode_t tx);
void nrf_receive(uint8_t* pStart, uint8_t len);
uint8_t nrf_is_busy();


/**
*Platform provided function to set the CE pin.
*The CE pin is a pin to enable the radio, and must follow a certain sequence.
*The platform must support setting or unsetting a GPIO pin when this function is called.
*@param mode mode to set the pin to, HIGH or LOW.
*@return None.
*/
extern void set_ce(pinmode mode);
/**
*Platform provided function to set the SPI Chip Select pin.
*The SS/CSN pin is a pin to signal a SPI data transfer and the end, and is used by the nRF hardware to determine a transaction.
*The platform must support setting or unsetting a GPIO pin when this function is called.
*@param mode mode to set the pin to, HIGH or LOW.
*@return None.
*/
extern void set_csn(pinmode mode);


/**
*Platform provided function to synchrnously exchange data with an SPI slave.
*Transmit_sync() transfers len bytes to the nRF radio, while receiving len bytes back.
*The platform must NOT toggle the CS pin, set_csn() will be called appropriately.
*@param pIn pointer to byte array to transmit
*@param pOut pointer to byte buffer to receive from. Must be len bytes long.
*@param len Size of the transaction in bytes
*@return None
*/
extern void transmit_sync(uint8_t* pStart, uint8_t* pIn, uint8_t len);

/**
*Platform provided function to write data to a SPI slave and ignore received.
*Transfer_sync() transfers len bytes to the nRF radio and discards the received data from the slave.
*The platform must NOT toggle the CS pin, set_csn() will be called appropriately.
*@param pStart pointer to byte array to transmit
*@param len Size of the transaction in bytes
*@return None
*/
extern void transfer_sync(uint8_t* pStart, uint8_t len);

/**
*Platform provided function to write  a single data byte to a SPI slave and receive a byte back.
*fast_shift() transfers 1 byte to the nRF radio and returns the received data from the slave.
*The platform must NOT toggle the CS pin, set_csn() will be called appropriately.
*@param value value to send to SPI slave.
*@return The response from the SPI slave.
*/
extern uint8_t fast_shift(uint8_t value);

/**
*Platform provided function to initialize the GPIO and SPI subsystems required for the nRF to function.
*Here, the platform must set up SPI clock speeds for the slave, Mode 0 communication, and the GPIOs for SPI CS and CE pins.
*libNRF will call this function as part of its setup routine
*@return None
*/
extern void platform_init();

#endif
