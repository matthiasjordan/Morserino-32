#ifndef MORSELORA_H_
#define MORSELORA_H_

namespace MorseLoRa
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

    void setup();
    void idle();

    void cwForLora(int element);
    void sendWithLora();
    boolean loRaBuReady();
    Packet decodePacket();
    void receive();
}

#endif /* MORSELORA_H_ */
