#ifndef MORSELORA_H_
#define MORSELORA_H_

namespace MorseLoRa
{

    struct RawPacket {
        ~RawPacket() {
            delete payload;
        }
        uint8_t header;
        int rssi;
        uint8_t* payload;
        uint16_t payloadLength;

        uint8_t protocolVersion() { return header >> 6;};
    };


    void setup();
    void idle();

    void sendWithLora(const char loraTxBuffer[]);
    boolean loRaBuReady();
    void receive();
    RawPacket decodePacket();
}

#endif /* MORSELORA_H_ */
