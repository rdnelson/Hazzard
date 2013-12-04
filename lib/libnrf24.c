#include "libnrf24.h"

#define _BV(c)	(1 << c)


uint8_t nrf_is_busy()
{

    uint8_t fifo_status;
    uint8_t status;
    nrf_read_register(FIFO_STATUS, &fifo_status, 1);
    if ((status = nrf_read_status()) & (_BV(TX_DS) | _BV(MAX_RT))) {
        //if (fifo_status & _BV(TX_FULL))
        //	return 1;
        return (status & _BV(MAX_RT) ? -1 : 0); /* false */

    }
    //MAX RT exceeded or transmit unsuccessful. Probably not done.
    return 1;

}

uint8_t nrf_get_payload_size(){
    uint8_t result;
    set_csn(0);
    fast_shift(I_R_RX_PL_WID);
    result = fast_shift(I_NOP);
    set_csn(1);
    return result;


}
uint8_t nrf_data_available()
{

    uint8_t status = nrf_read_status();

    if (status & _BV(RX_DR))
        return 1;
    else {
        nrf_read_register(FIFO_STATUS,&status, 1);
        return !(status & _BV(RX_EMPTY));
    }


}

void nrf_set_address(uint8_t tx, uint8_t* pStart)
{

    if (tx) {
        nrf_write_register(RX_ADDR_P0,pStart,5);
        nrf_write_register(TX_ADDR,pStart,5);
    } else {
        nrf_write_register(RX_ADDR_P1,pStart,5);
    }


}
void nrf_receive(uint8_t* pStart, uint8_t len)
{


    set_ce(0);
    set_csn(0);

    fast_shift(I_R_RX_PAYLOAD);

    transmit_sync(pStart, pStart, len);

    set_csn(1);

    nrf_config_register(STATUS,(1<<RX_DR));
    set_ce(1);
}

void nrf_flush_tx()
{
    set_csn(0);
    fast_shift(I_FLUSH_TX);
    set_csn(1);

}
void nrf_send(uint8_t* pStart, uint8_t uSz)
{
    uint8_t status_var = 0x00;
    set_ce(0);
    nrf_power_radio(MODE_TX);
    /*do{
    nrf_read_register(FIFO_STATUS,&status_var,1);
     }
    while (!(status_var & _BV(TX_EMPTY)));
    */

    set_csn(0);
    fast_shift(I_W_TX_PAYLOAD);
    /* Write payload */
    transfer_sync(pStart, uSz);
    set_csn(1);
    set_ce(1);

}



void nrf_init()
{
    platform_init();
    set_csn(1);
    set_ce(0);

}

void nrf_config(uint8_t channel, uint8_t payload_sz)
{
    nrf_config_register(RF_CH, channel);
    nrf_config_register(RX_PW_P0, 0x00); // Auto-ACK pipe ...
    nrf_config_register(RX_PW_P1, payload_sz); // Data payload pipe
    nrf_config_register(RX_PW_P2, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P3, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P4, 0x00); // Pipe not used
    nrf_config_register(RX_PW_P5, 0x00); // Pipe not used

    nrf_config_register(RF_SETUP, (0<<RF_DR)|((0x03)<<RF_PWR));


    nrf_config_register(EN_AA,_BV(ENAA_P0) | _BV(ENAA_P1));
    nrf_config_register(SETUP_RETR,(0x05<<ARD)|(0x0F<<ARC));
    nrf_config_register(EN_RXADDR,_BV(ERX_P0)|_BV(ERX_P1));
    nrf_config_register(SETUP_AW, (0x03 << AW));
    nrf_config_register(DYNPD,_BV(DPL_P0) | _BV(DPL_P1));
    nrf_config_register(FEATURE, _BV(EN_DPL));

    // CRC enable, 1 byte CRC length
    nrf_config_register(CONFIG, (1 << EN_CRC));
}


void nrf_power_radio(nrf_mode_t tx)
{


    if (tx == MODE_TX) {
        nrf_config_register(STATUS, _BV(RX_DR)|  _BV(TX_DS)| _BV(MAX_RT));
        nrf_config_register(CONFIG, _BV(EN_CRC)| _BV(PWR_UP));
    }

    else {
        set_csn(0);
        fast_shift(I_FLUSH_RX);
        set_csn(1);


        set_ce(0);
        nrf_config_register(CONFIG,_BV(EN_CRC)|_BV(PWR_UP)|_BV(PRIM_RX));
        nrf_config_register(STATUS,_BV(RX_DR) |_BV(TX_DS) |_BV(MAX_RT));
        
        set_csn(0);
        fast_shift(I_FLUSH_RX);
        set_csn(1);

        set_csn(0);
        fast_shift(I_FLUSH_TX);
        set_csn(1);
        set_ce(1);
    }

}



void nrf_power_up()
{

    nrf_config_register(CONFIG, _BV(PWR_UP) | _BV(PRIM_RX));
}

void nrf_power_down()
{
    nrf_config_register(CONFIG, _BV(EN_CRC));


}
void nrf_config_register(uint8_t reg, uint8_t value)
{

    set_csn(0);
    fast_shift(I_W_REGISTER | ( 0x1f & reg));
    fast_shift(value);
    set_csn(1);
}

void nrf_read_register(uint8_t reg, uint8_t* pStart, uint8_t len)
{
    set_csn(0);
    fast_shift(I_R_REGISTER | (0x1f & reg ));
    transmit_sync(pStart,pStart,len);
    set_csn(1);
}


void nrf_write_register(uint8_t reg, uint8_t* pStart, uint8_t len)
{
    set_csn(0);
    fast_shift(I_W_REGISTER | (0x1f & reg ));
    transfer_sync(pStart,len);
    set_csn(1);
}

uint8_t nrf_read_status()
{
    uint8_t status = 0;
    set_csn(0);
    status = fast_shift(I_NOP);
    set_csn(1);

    return status;
}

void nrf_set_address_suffix(uint8_t suffix){
	set_ce(0);
    nrf_write_register(RX_ADDR_P0,&suffix,1);
    nrf_write_register(TX_ADDR,&suffix,1);
    set_ce(1);
}


