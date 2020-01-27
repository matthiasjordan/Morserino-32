#include "MorseGenerator.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "MorseMenu.h"
#include "MorseModeHeadCopying.h"
#include "MorsePlayerFile.h"

MorseModeHeadCopying morseModeHeadCopying;

MorseModeHeadCopying::AutoStopState autoStopState;

void MorseModeHeadCopying::setup()
{

}

boolean MorseModeHeadCopying::menuExec(String mode)
{
    startTrainer();

    MorseGenerator::Config *genCon = MorseGenerator::getConfig();
    genCon->effectiveTrainerDisplay = DISPLAY_BY_CHAR;
    genCon->wordEndMethod = MorseModeHeadCopying::wordEndMethod;
    genCon->onGeneratorWordEnd = []()
    {   return morseModeHeadCopying.onGeneratorWordEnd();};

    if (mode == "a")
    {
    }
    else if (mode == "player")
    {
        MorsePlayerFile::openAndSkip();
    }

    autoStopState = off;
    MorseDisplay::getConfig()->autoFlush = false;

    return true;
}

void MorseModeHeadCopying::startTrainer()
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

unsigned long MorseModeHeadCopying::onGeneratorWordEnd()
{
    autoStopState = stop1;
    return -1;
}

boolean MorseModeHeadCopying::loop()
{
    boolean activeOld = MorseModeHeadCopying::active;

    if ((autoStopState == MorseModeHeadCopying::stop1) || MorseKeyer::leftKey || MorseKeyer::rightKey)
    {                                    // touching a paddle starts and stops the generation of code

        if (MorseKeyer::leftKey)
        {
            MorseText::setRepeatLast();
            MorseDisplay::clearScrollBuffer();
            MorseGenerator::getConfig()->wordEndMethod = MorseGenerator::nothing;
            autoStopState = off;
        }
        else
        {
            MorseGenerator::getConfig()->wordEndMethod = MorseModeHeadCopying::wordEndMethod;
        }

        // for debouncing:
        while (MorseKeyer::checkPaddles())
            ;                                                           // wait until paddles are released

        MorseModeHeadCopying::active = (autoStopState == MorseModeHeadCopying::off);
        switch (autoStopState)
        {
            case MorseModeHeadCopying::off:
            {
                break;
                //
            }
            case MorseModeHeadCopying::stop1:
            {
                autoStopState = MorseModeHeadCopying::stop2;
                break;
            }
            case MorseModeHeadCopying::stop2:
            {
                MorseDisplay::flushScroll();
                autoStopState = MorseModeHeadCopying::off;
                break;
            }
        }

        //delay(100);
    } /// end squeeze

    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        active = MorseGenerator::stopFlag = false;
    }

    if (activeOld != active)
    {
        if (!active)
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
    if (active)
    {
        MorseGenerator::generateCW();
    }

    return false;
}

void MorseModeHeadCopying::onPreferencesChanged()
{

}

boolean MorseModeHeadCopying::togglePause()
{
    return false;
}
