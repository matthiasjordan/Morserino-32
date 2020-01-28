/*
 * decoder.h
 *
 *  Created on: 07.12.2019
 *      Author: mj
 */

#ifndef DECODER_H_
#define DECODER_H_

#include <Arduino.h>
#include "morsedefs.h"

namespace Decoder
{

    // morse code decoder

    struct linklist
    {
            const char* symb;
            const uint8_t dit;
            const uint8_t dah;
    };

    extern const struct linklist CWtree[71];

    extern boolean filteredState;
    extern boolean filteredStateBefore;
    extern void (*onCharacter)(String);

    /// state machine for decoding CW
    enum DECODER_STATES
    {
        LOW_, HIGH_, INTERELEMENT_, INTERCHAR_
    };

    extern DECODER_STATES decoderState;

    extern unsigned long ditAvg, dahAvg;     /// average values of dit and dah lengths to decode as dit or dah and to adapt to speed change

    extern byte treeptr;                          // pointer used to navigate within the linked list representing the dichotomic tree

    extern unsigned long acsTimer;            // timer to use for automatic character spacing (ACS)
    extern boolean speedChanged;

    extern unsigned long interWordTimer;      // timer to detect interword spaces
    extern int goertzel_n;   //// you can use:         152, 304, 456 or 608 - thats the max buffer reserved in checktone()


    void startDecoder();
    void doDecodeShow();
    void setupGoertzel();
    void drawInputStatus(boolean on);
    void interWordTimerOff();
    String CWwordToClearText(String cwword);
    String displayMorse();
    uint8_t getDecodedWpm();

}

#endif /* DECODER_H_ */
