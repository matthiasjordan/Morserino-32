#include "MorseTrx.h"
#include "MorseKeyer.h"
#include "decoder.h"

boolean MorseTrx::loop()
{
    if (MorseKeyer::doPaddleIambic())
    {
        return true;                                                        // we are busy keying and so need a very tight loop !
    }
    Decoder::doDecodeShow();
    return false;
}

