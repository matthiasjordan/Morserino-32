/*
 * MorseInput.cpp
 *
 *  Created on: 13.02.2020
 *      Author: mj
 */

#include "MorseInput.h"
#include "decoder.h"
#include "MorseKeyer.h"

using namespace MorseInput;

boolean MorseInput::useStraightKey = false;
//void (*MorseInput::onCharacter)(String) = [](String s)
//{};
//void (*MorseInput::onWordEnd)() = []()
//{};

void MorseInput::start(void (*onCharacter)(String), void (*onWordEnd)())
{
    Decoder::startDecoder();
    Decoder::onCharacter = onCharacter;
    Decoder::onWordEnd = onWordEnd;

    MorseKeyer::setup();
    MorseKeyer::onCharacter = onCharacter;
    MorseKeyer::onWordEnd = onWordEnd;
    MorseKeyer::clearPaddleLatches();
}

boolean MorseInput::doInput()
{
    boolean busy = false;
    if (useStraightKey) {
        busy = Decoder::doDecodeShow();
    }
    else {
        busy = MorseKeyer::doPaddleIambic();
    }
    return busy;
}

