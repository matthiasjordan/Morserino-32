#ifndef MORSELORACW_H_
#define MORSELORACW_H_

#include <Arduino.h>
#include "MorseLoRa.h"

namespace MorseLoRaCW
{
    typedef struct
    {
            int rssi;
            uint8_t header;
            int rxWpm;
            String payload;
            boolean valid = true;

            uint8_t protocolVersion() { return header >> 6; };
            String toString() {
                return String((valid ? "ok" : "err")) + String(" ") + String(protocolVersion()) + " " + String(rssi) + " " + String(rxWpm) + " : " + payload;
            }
    } Packet;

    char* getTxBuffer();
    void cwForLora(int element);
    Packet decodePacket();
}

#endif /* MORSELORA_H_ */
