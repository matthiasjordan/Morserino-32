#include "MorseModeTrx.h"

#include "MorseKeyer.h"
#include "decoder.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"

MorseModeTrx morseModeTrx;

//void MorseModeTrx::setup()
//{
//
//}

boolean MorseModeTrx::menuExec(String mode)
{
    MorseMachine::morseState = MorseMachine::morseTrx;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start CW Trx");
    MorseKeyer::clearPaddleLatches();
    MorseKeyer::keyTx = true;
    Decoder::startDecoder();
    return true;
}

boolean MorseModeTrx::loop()
{
    Serial.println("MorseModeTrx::loop()");
    if (MorseKeyer::doPaddleIambic())
    {
        return true;                                                        // we are busy keying and so need a very tight loop !
    }
    Decoder::doDecodeShow();
    Serial.println("MorseModeTrx::loop()");
    return false;
}

void MorseModeTrx::onPreferencesChanged()
{
    Serial.println("MorseModeTrx::onPrefsChanged()");
}

boolean MorseModeTrx::togglePause()
{
    return false;
}
