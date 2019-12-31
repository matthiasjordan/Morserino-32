#ifndef MORSELORA_H_
#define MORSELORA_H_

namespace MorseLoRa {

        void setup();
        void idle();

        void cwForLora (int element);
        void sendWithLora();
        boolean loRaBuReady();
        uint8_t decodePacket(int* rssi, int* wpm, String* cwword);

}


#endif /* MORSELORA_H_ */
