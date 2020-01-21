#ifndef MORSELORA_H_
#define MORSELORA_H_


namespace MorseLoRa
{

    void setup();
    boolean menuExec(String mode);
    void idle();
    boolean loop();

    void cwForLora(int element);
    void sendWithLora();
    boolean loRaBuReady();
    uint8_t decodePacket(int* rssi, int* wpm, String* cwword);
    void receive();
}

#endif /* MORSELORA_H_ */
