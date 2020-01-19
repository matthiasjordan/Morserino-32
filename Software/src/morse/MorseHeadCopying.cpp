#include "MorseHeadCopying.h"
#include "MorseGenerator.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "MorseMenu.h"
#include "MorsePlayerFile.h"

using namespace MorseHeadCopying;

namespace internal
{
    void setupHeadCopying();
    void startTrainer();
}

namespace MorseHeadCopying
{
    boolean active = false;
    const MorseGenerator::WordEndMethod wordEndMethod = MorseGenerator::LF;
}

MorseHeadCopying::AutoStopState autoStopState;

void MorseHeadCopying::setup()
{

}


boolean MorseHeadCopying::menuExec(String mode)
{
    internal::startTrainer();
    internal::setupHeadCopying();

    if (mode == "a")
    {
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
    }
    else if (mode == "player")
    {
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
        MorsePlayerFile::openAndSkip();
    }

    autoStopState = off;
    MorseDisplay::getConfig()->autoFlush = false;

    return true;
}

void internal::startTrainer()
{
    MorseGenerator::setStart();
    MorseMachine::morseState = MorseMachine::headCopying;
    MorseGenerator::setup();
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0, "Head Copying  ");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start/Stop:   ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Paddle | BLACK");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::clearScroll();      // clear the buffer
}

void internal::setupHeadCopying()
{
    MorseGenerator::Config *genCon = MorseGenerator::getConfig();
    genCon->effectiveTrainerDisplay = DISPLAY_BY_CHAR;
    genCon->wordEndMethod = MorseHeadCopying::wordEndMethod;

}

void MorseHeadCopying::onGeneratorWordEnd()
{
    autoStopState = stop1;
}

boolean MorseHeadCopying::loop()
{
    boolean activeOld = MorseHeadCopying::active;

    if ((autoStopState == MorseHeadCopying::stop1) || MorseKeyer::leftKey || MorseKeyer::rightKey)
    {                                    // touching a paddle starts and stops the generation of code

        if (MorseKeyer::leftKey)
        {
            Serial.println("MG::loop() 3 replast");
            MorseText::setRepeatLast();
            MorseDisplay::clearScrollBuffer();
            MorseGenerator::getConfig()->wordEndMethod = MorseGenerator::nothing;
        }
        else {
            MorseGenerator::getConfig()->wordEndMethod = MorseHeadCopying::wordEndMethod;
        }

        // for debouncing:
        while (MorseKeyer::checkPaddles())
            ;                                                           // wait until paddles are released

        MorseHeadCopying::active = (autoStopState == MorseHeadCopying::off);
        switch (autoStopState)
        {
            case MorseHeadCopying::off:
            {
                break;
                //
            }
            case MorseHeadCopying::stop1:
            {
                autoStopState = MorseHeadCopying::stop2;
                break;
            }
            case MorseHeadCopying::stop2:
            {
                MorseDisplay::flushScroll();
                autoStopState = MorseHeadCopying::off;
                break;
            }
        }

        //delay(100);
    } /// end squeeze

    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        MorseHeadCopying::active = MorseGenerator::stopFlag = false;
    }

    if (activeOld != MorseHeadCopying::active)
    {
        if (!MorseHeadCopying::active)
        {
            MorseGenerator::keyOut(false, true, 0, 0);
            MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
        }
        else
        {
            //cleanStartSettings();
            MorseGenerator::generatorState = MorseGenerator::KEY_UP;
            MorseGenerator::genTimer = millis() - 1; // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc...
            MorseDisplay::displayTopLine();
        }
    }
    if (MorseHeadCopying::active)
    {
        MorseGenerator::generateCW();
    }

    return false;
}
