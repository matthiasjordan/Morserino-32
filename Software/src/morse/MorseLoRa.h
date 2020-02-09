#ifndef MORSELORA_H_
#define MORSELORA_H_

namespace MorseLoRa
{

    struct RawPacket {
        ~RawPacket() {
            delete payload;
        }
        int rssi;
        uint8_t* payload;
        uint16_t payloadLength;

        String payloadAsString() {
            String message;
            for (int i = 0; (i < payloadLength); i++) {
                message += (char) payload[i];
            }
            return message;
        }
    };


    void setup();
    void idle();

    void sendWithLora(const char loraTxBuffer[]);
    boolean loRaBuReady();
    void receive();
    RawPacket decodePacket();
}

#endif /* MORSELORA_H_ */
