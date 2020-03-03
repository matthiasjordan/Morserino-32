/*
 * MorseInput.cpp
 *
 *  Created on: 13.02.2020
 *      Author: mj
 */

#include "MorseInput.h"
#include "decoder.h"
#include "MorseKeyer.h"
#include "MorsePreferences.h"

using namespace MorseInput;

void MorseInput::start(void (*onCharacter)(String), void (*onWordEnd)())
{
    Decoder::startDecoder();
    Decoder::onCharacter = onCharacter;
    Decoder::onWordEnd = onWordEnd;

    MorseKeyer::setup();
    MorseKeyer::onCharacter = onCharacter;
    MorseKeyer::onWordEnd = onWordEnd;
    MorseKeyer::clearPaddleLatches();
    setStraightKeyFromPrefs();
}

boolean MorseInput::doInput()
{
    boolean busy = false;

    if (MorsePreferences::prefs.useStraightKey)
    {
        busy = Decoder::doDecodeShow();
    }
    else
    {
        busy = MorseKeyer::doPaddleIambic();
    }
    return busy;
}

void MorseInput::setStraightKeyFromPrefs()
{
    Decoder::config.straightKeyInput = MorsePreferences::prefs.useStraightKey;
}
