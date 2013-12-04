#ifndef _LIBNRF24_H
#define _LIBNRF24_H
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


typedef enum NRF_MODE{
	MODE_RX,
	MODE_TX,
} nrf_mode_t;

typedef enum {R =0, W=1} io_mode_t;
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

extern void set_ce(pinmode mode);
extern void set_csn(pinmode mode);

extern void transmit_sync(uint8_t* pStart, uint8_t* pIn, uint8_t len);
extern void transfer_sync(uint8_t* pStart, uint8_t len);
extern uint8_t fast_shift(uint8_t value);
extern void platform_init();

#endif
