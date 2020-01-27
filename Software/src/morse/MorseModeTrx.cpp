#include "MorseModeTrx.h"

#include "MorseKeyer.h"
#include "decoder.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"

MorseModeTrx morseModeTrx;

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
    if (MorseKeyer::doPaddleIambic())
    {
        return true;                                                        // we are busy keying and so need a very tight loop !
    }
    Decoder::doDecodeShow();
    return false;
}

void MorseModeTrx::onPreferencesChanged()
{
}

boolean MorseModeTrx::togglePause()
{
    return false;
}
