#ifndef MORSELORACW_H_
#define MORSELORACW_H_

#include <Arduino.h>
#include "MorseLoRa.h"

namespace MorseLoRaCW
{
    typedef struct
    {
            uint8_t protocolVersion;
            int rssi;
            int rxWpm;
            String payload;
            boolean valid = true;

            String toString() {
                return String((valid ? "ok" : "err")) + String(" ") + String(protocolVersion) + " " + String(rssi) + " " + String(rxWpm) + " : " + payload;
            }
    } Packet;

    char* getTxBuffer();
    void cwForLora(int element);
    Packet decodePacket();
}

#endif /* MORSELORA_H_ */
