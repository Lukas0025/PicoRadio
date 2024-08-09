#include "RFM95.h"
#include <tinySPI.h>
#include <picoGpio.h>

namespace PicoRadio {

  // default tables
  #ifdef PICO_RADIO_SAFTY
    uint8_t RFM95_FTABLE[] = RFM95_STANDART_FREQ_TABLE;
    uint8_t RFM95_DTABLE[] = RFM95_STANDART_DR_TABLE;
  #endif

  RFM95::RFM95(uint8_t nssPin) {
    this->nssPin = nssPin;
  }

  void RFM95::init() {
    // set pinmode output
    PICO_OUTPUT(this->nssPin);
    
    // NSS for starting and stopping communication with the RFM95 module
    PICO_HIGH(this->nssPin);

    //Switch RFM to sleep
    this->write(RFM_REG_OP_MODE, 0x00);

    //Set RFM in LoRa mode
    this->write(RFM_REG_OP_MODE, 0x80);

    //Set RFM in Standby mode wait on mode ready
    this->write(RFM_REG_OP_MODE, 0x81);

    PICO_DELAY(10);

    //PA pin (maximal power)
    this->write(RFM_REG_PA_CONFIG, 0xFF);

    //Preamble length set
    this->write(RFM_REG_PREAMBLE_MSB, 0x00);
    this->write(RFM_REG_PREAMBLE_LSB, PICO_RADIO_PREAMBLE_LEN);

    //Set LoRa sync word
    this->write(RFM_REG_SYNC_WORD, PICO_RADIO_SYNCWORD);

    // Errata Note - 2.3 Receiver Spurious Reception
    uint8_t detect_optimize = this->read(RFM_REG_DETECT_OPTIMIZE);
    this->write(RFM_REG_DETECT_OPTIMIZE, (detect_optimize & 0x78) | 0x03);
    this->write(RFM_REG_IF_FREQ_1, 0x00);
    this->write(RFM_REG_IF_FREQ_2, 0x40);

    //Set FIFO pointers
    this->write(RFM_REG_FIFO_TX_BASE_ADDR, 0x80);
    this->write(RFM_REG_FIFO_RX_BASE_ADDR, 0x00);

    //Switch RFM to sleep
    this->write(RFM_REG_OP_MODE, 0x00);

    #ifdef PICO_RADIO_SAFTY
      //check if tables is set if not define it
      if (this->freqTable == NULL || this->drTable == NULL) {
        this->freqTable = RFM95_FTABLE;
        this->drTable   = RFM95_DTABLE;
      }

      //set current preq and datarate to zero index
      if (this->CurFreq == NULL || this->CurDr == NULL) {
        this->setChannel(0, 0);
      }
    #endif
  }

  void RFM95::write(uint8_t addr, uint8_t data) {
    //Set NSS pin Low to start communication
    PICO_LOW(this->nssPin);

    //Send Addres with MSB 1 to make it a write command
    SPI.transfer(addr | 0x80);
    //Send Data
    SPI.transfer(data);

    //Set NSS pin High to end communication
    PICO_HIGH(this->nssPin);
  }

  unsigned char RFM95::read(unsigned char addr) {
    uint8_t data;

    //Set NSS pin low to start SPI communication
    PICO_LOW(this->nssPin);

    //Send Address
    SPI.transfer(addr);

    //Send 0x00 to be able to receive the answer from the RFM
    data = SPI.transfer(0x00);

    //Set NSS high to end communication
    PICO_HIGH(this->nssPin);

    //Return received data
    return data;
  }

  void RFM95::send(uint8_t* packet, uint8_t length) {
    uint8_t i;

    // check all needed arrays
    #ifdef PICO_RADIO_SAFTY
      if (
        this->CurFreq == NULL ||
        this->CurDr   == NULL || 
        packet        == NULL ||
        length        >  64
      ) {
        return;
      }
    #endif

    //Set RFM in Standby mode wait on mode ready
    this->write(RFM_REG_OP_MODE, 0x81);

    PICO_DELAY(10);

    //Set carrier frequency
    this->write(RFM_REG_FR_MSB, this->CurFreq[0]);
    this->write(RFM_REG_FR_MID, this->CurFreq[1]);
    this->write(RFM_REG_FR_LSB, this->CurFreq[2]);

    //Set SF and BW
    this->write(RFM_REG_MODEM_CONFIG_2, this->CurDr[0]); //SF 
    this->write(RFM_REG_MODEM_CONFIG_1, this->CurDr[1]); //BW
    this->write(RFM_REG_MODEM_CONFIG_3, this->CurDr[2]); 

    #ifdef PICO_RADIO_SEND_IQ_INVERT
      // Invert IQ
      this->write(RFM_REG_INVERT_IQ,   0x66);
      this->write(RFM_REG_INVERT_IQ_2, 0x19);
    #endif

    #ifndef PICO_RADIO_SEND_IQ_INVERT
      //Set IQ to normal values
      this->write(RFM_REG_INVERT_IQ,   0x27);
      this->write(RFM_REG_INVERT_IQ_2, 0x1D);
    #endif

    //Set payload length to the right length
    this->write(RFM_REG_PAYLOAD_LENGTH, length);

    // set fifo poiter to start of TX
    this->write(RFM_REG_FIFO_ADDR_PTR, 0x80);

    //Write Payload to FiFo
    for (i = 0; i < length; i++) {
      this->write(RFM_REG_FIFO, packet[i]);
    }

    //Switch RFM to Tx
    this->write(RFM_REG_OP_MODE, 0x83);

    // ACTIVE WAIT UNTIL END
    for (
      i = 0;
      (i < 2000) && ((this->read(RFM_REG_IRQ_FLAGS) & RFM_STATUS_TX_DONE) != RFM_STATUS_TX_DONE);
      i++
    ) { // max 16s
      
      if (i >= 1500) { // transmission timeout 12s
        this->init(); // reset radio
        break;
      }

      PICO_DELAY(1); // wait 8ms
    }

    //Clear interrupt
    this->write(RFM_REG_IRQ_FLAGS, 0xFF);

    //Switch RFM to sleep
    this->write(RFM_REG_OP_MODE, 0x00);
  }

  /**
   * Function for receiving a packet using the RFM
   *
   * @param packet Pointer to RX packet array.
   * @param packet_max_length Maximum number of bytes to read from RX packet.
   * @param channel The frequency table channel index.
   * @param dri The data rate table index.
   * @param rx_tickstamp Listen until rx_tickstamp elapsed.
   * @return The packet length or an error code.
   */
  int8_t RFM95::receive(uint8_t *packet, uint8_t maxLen) {
    // check all needed arrays
    #ifdef PICO_RADIO_SAFTY
      if (
        this->CurFreq == NULL ||
        this->CurDr   == NULL || 
        packet        == NULL ||
        maxLen        >  64
      ) {
        return;
      }
    #endif

    // Switch RFM to standby
    this->write(RFM_REG_OP_MODE, 0x81);

    // Set SPI pointer to start of Rx part in FiFo
    this->write(RFM_REG_FIFO_ADDR_PTR, 0x00);

    //Set carrier frequency
    this->write(RFM_REG_FR_MSB, this->CurFreq[0]);
    this->write(RFM_REG_FR_MID, this->CurFreq[1]);
    this->write(RFM_REG_FR_LSB, this->CurFreq[2]);

    //Set SF and BW
    this->write(RFM_REG_MODEM_CONFIG_2, this->CurDr[0]); //SF 
    this->write(RFM_REG_MODEM_CONFIG_1, this->CurDr[1]); //BW
    this->write(RFM_REG_MODEM_CONFIG_3, this->CurDr[2]); 

    #ifdef PICO_RADIO_RECV_IQ_INVERT
      // Invert IQ
      this->write(RFM_REG_INVERT_IQ,   0x66);
      this->write(RFM_REG_INVERT_IQ_2, 0x19);
    #endif
    
    #ifndef PICO_RADIO_RECV_IQ_INVERT
      //Set IQ to normal values
      this->write(RFM_REG_INVERT_IQ,   0x27);
      this->write(RFM_REG_INVERT_IQ_2, 0x1D);
    #endif

    // Rx timeout - x symbols
    this->write(RFM_REG_SYMB_TIMEOUT_LSB, PICO_RADIO_RX_TIMEOUT);

    // Clear interrupts
    write(RFM_REG_IRQ_FLAGS, 0xFF);

    // Switch RFM to Rx
    write(RFM_REG_OP_MODE, 0x86);

    // Wait for RxDone or RxTimeout
    uint8_t irqFlags = 0;
    for (uint16_t i = 0; (i < 2000) && !(irqFlags & 0xC0); i++) { // max 16s

      irqFlags = this->read(RFM_REG_IRQ_FLAGS);
      
      if (i >= 1500) { // transmission timeout 12s
        this->init();  // reset radio
        return PICORADIO_ERROR_RX_TIMEOUT;
      }

      delay(1); // wait 8ms
    }


    uint8_t packetLength = this->read(RFM_REG_RX_NB_BYTES);

    packetLength = (maxLen < packetLength) ? maxLen : packetLength;

    write(RFM_REG_FIFO_ADDR_PTR, this->read(RFM_REG_FIFO_RX_CURRENT_ADDR));
    
    for (uint8_t i = 0; i < packetLength; i++) {
      packet[i] = this->read(RFM_REG_FIFO);
    }

    // SNR
    this->lastSnr = (int8_t) this->read(RFM_REG_PKT_SNR_VALUE) / 4;

    // Clear interrupts
    write(RFM_REG_IRQ_FLAGS, 0xFF);

    // Switch RFM to sleep
    write(RFM_REG_OP_MODE, 0x00);

    switch (irqFlags & 0xC0) {
      case RFM_STATUS_RX_TIMEOUT:
        return PICORADIO_ERROR_RX_TIMEOUT;
      case RFM_STATUS_RX_DONE_CRC_ERROR:
        return PICORADIO_ERROR_CRC;
      case RFM_STATUS_RX_DONE:
        return packetLength;
    }

    return PICORADIO_ERROR_UNKNOWN;
  }
}