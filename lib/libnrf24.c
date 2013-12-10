#include "libnrf24.h"
//macro to simplify bit shifts. GCC will optimize this to a constant
#define _BV(c)	(1 << c)


/**
*Function to check if the radio is busy.
*Use for polling after each TX to check if the message is sent
*@return 0 if radio is idle, -1 if failed to TX last, and 1 if still transmitting
**/
uint8_t nrf_is_busy()
{

    uint8_t fifo_status;
    uint8_t status;
    //grab the FIFO register
    nrf_read_register(FIFO_STATUS, &fifo_status, 1);
    //check if the transmit data flag is set, or the MAX_RETRIES has been set
    if ((status = nrf_read_status()) & (_BV(TX_DS) | _BV(MAX_RT))) {
        //if (fifo_status & _BV(TX_FULL))
        //	return 1;
        //return the appropriate value
        return (status & _BV(MAX_RT) ? -1 : 0); /* false */

    }
    //MAX RT exceeded or transmit unsuccessful. Probably not done.
    return 1;

}

/**
*Get the size of the incoming data packet
*@return the size of the data packet in bytes. Flush the TX if it's greater than 32 bytes
**/
uint8_t nrf_get_payload_size(){
    uint8_t result;
    set_csn(0);
    fast_shift(I_R_RX_PL_WID);
    result = fast_shift(I_NOP);
    set_csn(1);
    return result;


}
/**
*Polling loop function to check if the radio has data available
*@return 0 if no data incoming, 1 if there's data in the buffers.
**/
uint8_t nrf_data_available()
{
    //grab the status byte
    uint8_t status = nrf_read_status();
    //check if there is data ready
    if (status & _BV(RX_DR))
        return 1;
    else {
        //check and make sure the FIFO is empty too
        nrf_read_register(FIFO_STATUS,&status, 1);
        return !(status & _BV(RX_EMPTY));
    }


}
/**
*Set the address for the transmitter/RX. Even in address prefix:suffix mode, the base address has to be set.
*@param tx mode to set the address for. 0 is the receiver, 1 is the transmitter.
*@param pStart pointer to start of data to send. Must be 5 bytes long.
*return None
**/
void nrf_set_address(uint8_t tx, uint8_t* pStart)
{

    if (tx) {
        nrf_write_register(RX_ADDR_P0,pStart,5);
        nrf_write_register(TX_ADDR,pStart,5);
    } else {
        nrf_write_register(RX_ADDR_P1,pStart,5);
    }


}
/**
*Receive a packet from the nRF radio
*@param pStart pointer to buffer to receive packet
*@param len length of packet to read.
*@return None
**/
void nrf_receive(uint8_t* pStart, uint8_t len)
{


    //stop the RTX module
    set_ce(0);
    //start a SPI transaction
    set_csn(0);
    //send read payload command
    fast_shift(I_R_RX_PAYLOAD);
    //grab the data
    transmit_sync(pStart, pStart, len);
    //finish the transaction
    set_csn(1);
    //ACK The transaction
    nrf_config_register(STATUS,(1<<RX_DR));
    //reenable RX
    set_ce(1);
}

/**
*Flush the internal buffer on the nRF radio
*@return None
*/
void nrf_flush_tx()
{
    set_csn(0);
    fast_shift(I_FLUSH_TX);
    set_csn(1);

}
/**
*Send a data packet to the configured TX address
*@param pStart pointer to start of data packet
*@param uSz size of packet to sernd
*@return None
*/
void nrf_send(uint8_t* pStart, uint8_t uSz)
{
    uint8_t status_var = 0x00;
    //power down the radio
    set_ce(0);
    //restart in TX mode
    nrf_power_radio(MODE_TX);
    /*do{
    nrf_read_register(FIFO_STATUS,&status_var,1);
     }
    while (!(status_var & _BV(TX_EMPTY)));
    */

    //start SPI transaction
    set_csn(0);
    //send a TX_PAYLOAD command
    fast_shift(I_W_TX_PAYLOAD);
    /* Write payload */
    transfer_sync(pStart, uSz);
    set_csn(1);
    //begin transmitting
    set_ce(1);

}


/**
*Set up the nrf library. This must be the first function called.
*@return None
*/
void nrf_init()
{
    //set up the platform
    platform_init();
    //set the SPI CS pin high
    set_csn(1);
    //set the CE pin LOW
    set_ce(0);

}

/**
*Configures the radio to use a certain channel and default payload size
*@param channel channel to set the radio
*@param payload_sz size of default payload.
*@return None
*/
void nrf_config(uint8_t channel, uint8_t payload_sz)
{
    //Most of these values are from the datasheet.
    nrf_config_register(RF_CH, channel); //set the correct RF channel
    nrf_config_register(RX_PW_P0, 0x00); // Auto-ACK pipe
    nrf_config_register(RX_PW_P1, payload_sz); // Data payload pipe
    nrf_config_register(RX_PW_P2, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P3, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P4, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P5, 0x00); // Pipe not used


    //set 1Mbps data rate, and full power
    nrf_config_register(RF_SETUP, (0<<RF_DR)|((0x03)<<RF_PWR));

    //enable auto-ack on pipes 0 and 1
    nrf_config_register(EN_AA,_BV(ENAA_P0) | _BV(ENAA_P1));
    //15 retries, 250us delay between each
    nrf_config_register(SETUP_RETR,(0x05<<ARD)|(0x0F<<ARC));
    //enable P0 and Pi for receiving data
    nrf_config_register(EN_RXADDR,_BV(ERX_P0)|_BV(ERX_P1));
    //setup the correct address sizes, 5 bytes.
    nrf_config_register(SETUP_AW, (0x03 << AW));
    //enable dynamic payload sizes, to have un-ACKed payloads.
    nrf_config_register(DYNPD,_BV(DPL_P0) | _BV(DPL_P1));

    //enable dynamic payloads in system
    nrf_config_register(FEATURE, _BV(EN_DPL));

    // CRC enable, 1 byte CRC length
    nrf_config_register(CONFIG, (1 << EN_CRC));
}

/**
*Power up the radio in either transmit or receive mode
*@param tx mode to enable the radio in. Valid values are MODE_TX and MODE_RX
*@return None
**/
void nrf_power_radio(nrf_mode_t tx)
{


    //if TX mode is enabled, clear all interrupts, and set the device in TX mode
    if (tx == MODE_TX) {
        nrf_config_register(STATUS, _BV(RX_DR)|  _BV(TX_DS)| _BV(MAX_RT));
        nrf_config_register(CONFIG, _BV(EN_CRC)| _BV(PWR_UP));
    }

    else {
        //flush the RX buffers
        set_csn(0);
        fast_shift(I_FLUSH_RX);
        set_csn(1);

        //power down the radio
        set_ce(0);
        //enable RX fnctionality, and clear pending interrupts
        nrf_config_register(CONFIG,_BV(EN_CRC)|_BV(PWR_UP)|_BV(PRIM_RX));
        nrf_config_register(STATUS,_BV(RX_DR) |_BV(TX_DS) |_BV(MAX_RT));
        
        //flush the RX buffer
        set_csn(0);
        fast_shift(I_FLUSH_RX);
        set_csn(1);

        //flush the TX buffer
        set_csn(0);
        fast_shift(I_FLUSH_TX);
        set_csn(1);
        //reenable radio module
        set_ce(1);
    }

}


/**
*Power up the radio (as in, exit sleep mode)
*@return None
**/
void nrf_power_up()
{
    //write to the CONFIG register
    nrf_config_register(CONFIG, _BV(PWR_UP) | _BV(PRIM_RX));
}


/**
*Power down the radio (as in, exit sleep mode)
*@return None
**/
void nrf_power_down()
{
    nrf_config_register(CONFIG, _BV(EN_CRC));


}
/**
*Configure a single byte register on the nRF radio.
*@param reg register to read.
*@param value value to write to register
*@return None
**/
void nrf_config_register(uint8_t reg, uint8_t value)
{

    //start SPI transaction
    set_csn(0);
    //shift the W_REGISTER command ORd with the register number
    fast_shift(I_W_REGISTER | ( 0x1f & reg));
    //shift the value to write
    fast_shift(value);
    //end transaction
    set_csn(1);
}
/**
*Reads a multi-byte register on the nRF radio.
*@param reg register value to start read from.
*@param pStart pointer to buffer to read the data into
*@param len number of bytes to read
*@return None
**/
void nrf_read_register(uint8_t reg, uint8_t* pStart, uint8_t len)
{
    //start transaction
    set_csn(0);
    //shift the R_register command ORd with the register address
    fast_shift(I_R_REGISTER | (0x1f & reg ));
    //start reading len bytes
    transmit_sync(pStart,pStart,len);
    //end transaction
    set_csn(1);
}

/**
*Writes a multi-byte register on the nRF radio.
*@param reg register value to start write.
*@param pStart pointer to buffer to read the data from
*@param len number of bytes to write
*@return None
**/
void nrf_write_register(uint8_t reg, uint8_t* pStart, uint8_t len)
{
    //start transaction
    set_csn(0);
    //write register command ORd with register address
    fast_shift(I_W_REGISTER | (0x1f & reg ));

    //start writing to device
    transfer_sync(pStart,len);
    //end transaction
    set_csn(1);
}

/**
*Reads the nRF status byte
*@retun STATUS register value
**/
uint8_t nrf_read_status()
{

    uint8_t status = 0;
    //start SPI transaction
    set_csn(0);
    //shift a NOP to get the status byte
    status = fast_shift(I_NOP);
    //end transaction
    set_csn(1);

    return status;
}

/**
*Set an address suffix.
*@param suffix address suffix to set
*@return None
**/
void nrf_set_address_suffix(uint8_t suffix){
	//disable the TX
    set_ce(0);
    //write the addresses
    nrf_write_register(RX_ADDR_P0,&suffix,1);
    nrf_write_register(TX_ADDR,&suffix,1);
    //enable the TX
    set_ce(1);
}


