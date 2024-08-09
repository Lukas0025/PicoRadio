#include "PicoRadio.h"
#include "Arduino.h"

#pragma once

/**
 * Begin of strandart tables definition
 */
#define RFM95_STANDART_FREQ_TABLE { \
  0xD9, 0x06, 0x8B,  /* Channel 0 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B */ \
  0xD9, 0x13, 0x58,  /* Channel 1 868.300 MHz / 61.035 Hz = 14226264 = 0xD91358 */ \
  0xD9, 0x20, 0x24,  /* Channel 2 868.500 MHz / 61.035 Hz = 14229540 = 0xD92024 */ \
  0xD8, 0xC6, 0x8B,  /* Channel 3 867.100 MHz / 61.035 Hz = 14206603 = 0xD8C68B */ \
  0xD8, 0xD3, 0x58,  /* Channel 4 867.300 MHz / 61.035 Hz = 14209880 = 0xD8D358 */ \
  0xD8, 0xE0, 0x24,  /* Channel 5 867.500 MHz / 61.035 Hz = 14213156 = 0xD8E024 */ \
  0xD8, 0xEC, 0xF1,  /* Channel 6 867.700 MHz / 61.035 Hz = 14216433 = 0xD8ECF1 */ \
  0xD8, 0xF9, 0xBE   /* Channel 7 867.900 MHz / 61.035 Hz = 14219710 = 0xD8F9BE */ \
}

#define RFM95_STANDART_DR_TABLE { \
  0x74, 0x72, 0x04,  /* SF7BW125 */  \
  0x84, 0x72, 0x04,  /* SF8BW125 */  \
  0x94, 0x72, 0x04,  /* SF9BW125 */  \
  0xA4, 0x72, 0x04,  /* SF10BW125 */ \
  0xB4, 0x72, 0x0C,  /* SF11BW125 */ \
  0xC4, 0x72, 0x0C   /* SF12BW125 */ \
}

// RFM registers
#define RFM_REG_FIFO                    0x00
#define RFM_REG_OP_MODE                 0x01
#define RFM_REG_FR_MSB                  0x06
#define RFM_REG_FR_MID                  0x07
#define RFM_REG_FR_LSB                  0x08
#define RFM_REG_PA_CONFIG               0x09
#define RFM_REG_FIFO_ADDR_PTR           0x0D
#define RFM_REG_FIFO_TX_BASE_ADDR       0x0E
#define RFM_REG_FIFO_RX_BASE_ADDR       0x0F
#define RFM_REG_FIFO_RX_CURRENT_ADDR    0x10
#define RFM_REG_IRQ_FLAGS_MASK          0x11
#define RFM_REG_IRQ_FLAGS               0x12
#define RFM_REG_RX_NB_BYTES             0x13
#define RFM_REG_PKT_SNR_VALUE           0x19
#define RFM_REG_PKT_RSSI_VALUE          0x1A
#define RFM_REG_MODEM_CONFIG_1          0x1D
#define RFM_REG_MODEM_CONFIG_2          0x1E
#define RFM_REG_SYMB_TIMEOUT_LSB        0x1F
#define RFM_REG_PREAMBLE_MSB            0x20
#define RFM_REG_PREAMBLE_LSB            0x21
#define RFM_REG_PAYLOAD_LENGTH          0x22
#define RFM_REG_MODEM_CONFIG_3          0x26

// Only in SX1276 datasheet
#define RFM_REG_IF_FREQ_2               0x2F
#define RFM_REG_IF_FREQ_1               0x30
#define RFM_REG_DETECT_OPTIMIZE         0x31
#define RFM_REG_INVERT_IQ               0x33
#define RFM_REG_SYNC_WORD               0x39
#define RFM_REG_INVERT_IQ_2             0x3B

// RFM status
#define RFM_STATUS_TX_DONE              0x08
#define RFM_STATUS_RX_DONE              0x40
#define RFM_STATUS_RX_DONE_CRC_ERROR    0x60
#define RFM_STATUS_RX_TIMEOUT           0x80

namespace PicoRadio {
  class RFM95 : public module {
    public:
      
      RFM95(uint8_t nssPin);
      
      /**
       * Inicialize radio module
       */
      void init();
            
      /**
       * Send data using radio module
       * @param packet pointer to data to send
       * @param length length of data to send
       */
      void send(uint8_t *packet, uint8_t lenght);

      /**
       * receive packet using lora radio
       * @param packet    poiter to array where save received packet
       * @param maxLength maximal length of received packet (size of allocated array)
       * @return length of packet (positive) or error status of receive (negative)
       */
      int8_t receive(uint8_t *packet, uint8_t maxLength);

    private:

      /**
       * Write data to RFM register using SPI bus
       * @param address address to write
       * @param data    data to write on address
       * @warning MSB in addr is set in function to 1 to be write command (addr & 0x80) == 1
       */
      void write(unsigned char address, unsigned char data);

      /**
       * Read data from RFM register using SPI bus
       * @param address address from read
       * @return data on register
       * @warning MSB in addr must be 0 to be read command (addr & 0x80) == 0
       */
      uint8_t read(unsigned char address);
      
      int nssPin;
  };
}