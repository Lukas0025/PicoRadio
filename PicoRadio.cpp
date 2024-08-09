#include "PicoRadio.h"

namespace PicoRadio {

    void module::setChannel(uint8_t freq, uint8_t datarate) {
        this->CurDr   = &this->drTable[datarate * 3];
        this->CurFreq = &this->freqTable[freq * 3]; 
    }

    void module::setTables(uint8_t* freqs, uint8_t* rates) {
        this->drTable   = rates;
        this->freqTable = freqs;   
    }

}