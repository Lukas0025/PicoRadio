#include <Arduino.h>

#pragma once

#define PICO_RADIO_SAFTY
//#define PICO_RADIO_SYNCWORD 0x34
//#define PICO_RADIO_SEND_IQ_INVERT
//#define PICO_RADIO_RECV_IQ_INVERT
//#define PICO_RADIO_PREAMBLE_LEN 0x08
//#define PICO_RADIO_RX_TIMEOUT 255

// define needed configuration if not set
#ifndef PICO_RADIO_SYNCWORD
    #define PICO_RADIO_SYNCWORD 0x34
#endif

#ifndef PICO_RADIO_PREAMBLE_LEN
    #define PICO_RADIO_PREAMBLE_LEN 0x08
#endif

#ifndef PICO_RADIO_RX_TIMEOUT
    #define PICO_RADIO_RX_TIMEOUT 255
#endif

// errors
#define PICORADIO_ERROR_RX_TIMEOUT    -1
#define PICORADIO_ERROR_CRC           -2
#define PICORADIO_ERROR_UNKNOWN       -3

// standart tables indexes freq
#define F868100KHZ 0x00
#define F868300KHZ 0x01
#define F868500KHZ 0x02
#define F867100KHZ 0x03
#define F867300KHZ 0x04
#define F867500KHZ 0x05
#define F867700KHZ 0x06
#define F867900KHZ 0x07

// standart tables indexes datarates
#define SF7BW125  0x00
#define SF8BW125  0x01
#define SF9BW125  0x02
#define SF10BW125 0x03
#define SF11BW125 0x04
#define SF12BW125 0x05

namespace PicoRadio {

    class module {
        public:
            /**
             * value containding last SNR
             */
            int8_t lastSnr;

            /**
             * Inicialize radio module
             */
            virtual void init() = 0;
            
            /**
             * Send data using radio module
             * @param packet pointer to data to send
             * @param length length of data to send
             */
            virtual void send(uint8_t *packet, uint8_t lenght) = 0;

            /**
             * receive packet using lora radio
             * @param packet    poiter to array where save received packet
             * @param maxLength maximal length of received packet (size of allocated array)
             * @return length of packet (positive) or error status of receive (negative)
             */
            virtual int8_t receive(uint8_t *packet, uint8_t maxLength) = 0;

            /**
             * Set data rate and freqvency of radiomodule
             * @param datarate index of data rate to use in datarate table
             * @param freq     index of freq to use in freq table
             */
            void setChannel(uint8_t freq, uint8_t datarate);

            /**
             * Set freqvecy and datarates tables
             * @param freq pointer to freqvecy table
             * @param rates pointer to rates table
             */
            void setTables(uint8_t* freqs, uint8_t* rates);

            /**
             * Descructor of module
             */
            virtual ~module() {};

        protected:
            #ifdef PICO_RADIO_SAFTY
                uint8_t* CurDr     = NULL;
                uint8_t* CurFreq   = NULL;

                uint8_t* freqTable = NULL;
                uint8_t* drTable   = NULL;
            #endif

            #ifndef PICO_RADIO_SAFTY
                uint8_t* CurDr;
                uint8_t* CurFreq;

                uint8_t* freqTable;
                uint8_t* drTable;
            #endif
    };
}