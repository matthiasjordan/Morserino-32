/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/

#include "MorseModeEchoTrainer.h"

#include "MorsePreferences.h"
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "MorseDisplay.h"
#include "MorseGenerator.h"
#include "MorsePlayerFile.h"
#include "MorseMachine.h"
#include "MorseMenu.h"
#include "decoder.h"
#include "MorseText.h"

MorseModeEchoTrainer morseModeEchoTrainer;

//String MorseModeEchoTrainer::echoResponse = "";
//MorseModeEchoTrainer::echoStates echoTrainerState;
//boolean MorseModeEchoTrainer::echoStop = false;                         // for maxSequence
//boolean MorseModeEchoTrainer::active = false;                           // flag for trainer mode
//String MorseModeEchoTrainer::echoTrainerWord;
//int MorseModeEchoTrainer::repeats = 0;

MorseModeEchoTrainer::Config metConfig;

boolean MorseModeEchoTrainer::menuExec(String mode)
{
    if (mode == "player")
    {
        MorsePlayerFile::openAndSkip();
    }
    MorseModeEchoTrainer::startEcho();
    return true;
}

void MorseModeEchoTrainer::startEcho()
{
    MorseMachine::morseState = MorseMachine::echoTrainer;
    MorseGenerator::setStart();

    Decoder::storeCharInResponse = [](String r)
    {   morseModeEchoTrainer.storeCharInResponse(r);};

    MorseText::proceed();
    MorseText::onGeneratorNewWord = [](String r)
    {   morseModeEchoTrainer.onGeneratorNewWord(r);};

    MorseModeEchoTrainer::echoStop = false;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0, MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn) ? "New Character:" : "Echo Trainer:");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start:       ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Press paddle ");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer
    MorseKeyer::keyTx = false;
    MorseModeEchoTrainer::onPreferencesChanged();

    metConfig.showFailedWord = !MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn);
    metConfig.generateStartSequence = false;
    MorseText::setGenerateStartSequence(metConfig.generateStartSequence);
    if (metConfig.generateStartSequence)
    {
        echoTrainerState = START_ECHO;
    }
    else
    {
        echoTrainerState = SEND_WORD;
    }
    MorseModeEchoTrainer::active = false;

    MorseKeyer::setup();
    MorseKeyer::onWordEnd = []()
    {
        return morseModeEchoTrainer.onKeyerWordEnd();
    };
    MorseKeyer::onWordEndDitDah = []()
    {
        morseModeEchoTrainer.onKeyerWordEndDitDah();
    };
    MorseKeyer::onWordEndNDitDah = []()
    {
        morseModeEchoTrainer.onKeyerWordEndNDitDah();
    };
}

boolean MorseModeEchoTrainer::onKeyerWordEnd()
{
    Serial.println("MMET::onKeyerWordEnd()");
    if (MorseModeEchoTrainer::isState(MorseModeEchoTrainer::COMPLETE_ANSWER))
    {       // change the state of the trainer at end of word
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::EVAL_ANSWER);
        return false;
    }
    return true;
}

void MorseModeEchoTrainer::onKeyerWordEndDitDah()
{
    Serial.println("MMET::onKeyerWordEndDitDah()");
    if (MorseMachine::isMode(MorseMachine::echoTrainer))
    {      // change the state of the trainer at end of word
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::COMPLETE_ANSWER);
    }
}

void MorseModeEchoTrainer::onKeyerWordEndNDitDah()
{
    Serial.println("MMET::onKeyerWordEndNDitDah()");
    if (MorseModeEchoTrainer::isState(MorseModeEchoTrainer::GET_ANSWER))
    {
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::EVAL_ANSWER);
    }
}

void MorseModeEchoTrainer::onPreferencesChanged()
{
    Serial.println("MorseET::oPC");

    metConfig.showPrompt = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);

    MorseText::setRepeatEach(MorsePreferences::prefs.echoRepeats);

    MorseGenerator::Config *generatorConfig = MorseGenerator::getConfig();
    Serial.println("MorseModeEchoTrainer start edis " + String(MorsePreferences::prefs.echoDisplay));
    generatorConfig->key = (MorsePreferences::prefs.echoDisplay != DISP_ONLY);
    generatorConfig->printDitDah = false;
    generatorConfig->printChar = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);
    generatorConfig->wordEndMethod = MorseGenerator::LF;
//    generatorConfig->printSpaceAfterWord = true;
    generatorConfig->timing = (MorsePreferences::prefs.echoDisplay == DISP_ONLY) ? MorseGenerator::quick : MorseGenerator::tx;

    generatorConfig->onFetchNewWord = []()
    {   morseModeEchoTrainer.onFetchNewWord();};
    generatorConfig->onLastWord = []()
    {   morseModeEchoTrainer.onLastWord();};
    generatorConfig->onGeneratorWordEnd = []()
    {   return morseModeEchoTrainer.onGeneratorWordEnd();};
}

boolean MorseModeEchoTrainer::togglePause()
{
    active = !active;
    return !active;
}

boolean MorseModeEchoTrainer::isState(echoStates state)
{
    return echoTrainerState == state;
}

void MorseModeEchoTrainer::setState(echoStates newState)
{
    echoTrainerState = newState;
}

MorseModeEchoTrainer::echoStates MorseModeEchoTrainer::getState()
{
    return echoTrainerState;
}

void MorseModeEchoTrainer::storeCharInResponse(String symbol)
{
    symbol.replace("<as>", "S");
    symbol.replace("<ka>", "A");
    symbol.replace("<kn>", "N");
    symbol.replace("<sk>", "K");
    symbol.replace("<ve>", "V");
    symbol.replace("<ch>", "H");
    echoResponse.concat(symbol);
}

///////// evaluate the response in Echo Trainer Mode
void MorseModeEchoTrainer::echoTrainerEval()
{
    delay(MorseKeyer::interCharacterSpace / 2);
    if (echoResponse == echoTrainerWord)
    {
        echoTrainerState = SEND_WORD;
        MorseDisplay::printToScroll(BOLD, "OK");
        if (MorsePreferences::prefs.echoConf)
        {
            MorseSound::pwmTone(440, MorsePreferences::prefs.sidetoneVolume, false);
            delay(97);
            MorseSound::pwmNoTone();
            MorseSound::pwmTone(587, MorsePreferences::prefs.sidetoneVolume, false);
            delay(193);
            MorseSound::pwmNoTone();
        }
        delay(MorseKeyer::interWordSpace);
        if (MorsePreferences::prefs.speedAdapt)
        {
            MorseKeyer::changeSpeed(1);
        }
        MorseText::proceed();
    }
    else
    {
        Serial.println("MET::eTE() 5 " + echoResponse);
        echoTrainerState = REPEAT_WORD;
        if (!MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn) || echoResponse != "")
        {
            Serial.println("MET::eTE() 6");
            MorseDisplay::printToScroll(BOLD, "ERR");
            if (MorsePreferences::prefs.echoConf)
            {
                Serial.println("MET::eTE() 7");
                MorseSound::pwmTone(311, MorsePreferences::prefs.sidetoneVolume, false);
                delay(193);
                MorseSound::pwmNoTone();
            }
        }
        delay(MorseKeyer::interWordSpace);
        if (MorsePreferences::prefs.speedAdapt)
        {
            MorseKeyer::changeSpeed(-1);
        }
    }
    echoResponse = "";
    MorseKeyer::clearPaddleLatches();
}   // end of function

/**
 * @return: -1: NOOB,
 */
unsigned long MorseModeEchoTrainer::onGeneratorWordEnd()
{

    Serial.println("MET::onGenWordEnd MET::s: " + String(MorseModeEchoTrainer::getState()));

    if (!MorseMachine::isMode(MorseMachine::echoTrainer))
    {
        return -1;
    }

    unsigned long delta = 0;

    switch (MorseModeEchoTrainer::getState())
    {
        case MorseModeEchoTrainer::START_ECHO:
        {
            MorseModeEchoTrainer::setState(MorseModeEchoTrainer::SEND_WORD);
            delta = MorseKeyer::interCharacterSpace + (MorsePreferences::prefs.promptPause * MorseKeyer::interWordSpace);
            break;
        }
        case MorseModeEchoTrainer::REPEAT_WORD:
            // fall through
        case MorseModeEchoTrainer::SEND_WORD:
        {
            if (MorseModeEchoTrainer::echoStop)
            {
                break;
            }
            else
            {
                MorseModeEchoTrainer::setState(MorseModeEchoTrainer::GET_ANSWER);
                if (metConfig.showPrompt)
                {
                    MorseDisplay::printToScroll(REGULAR, " ");
                    MorseDisplay::printToScroll(INVERSE_REGULAR, ">");    /// add a blank after the word on the display
                }
                ++MorseModeEchoTrainer::repeats;
                delta = MorsePreferences::prefs.responsePause * MorseKeyer::interWordSpace;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return delta;

}

void MorseModeEchoTrainer::onGeneratorNewWord(String newWord)
{
    Serial.println("MET::onGNW");
    MorseModeEchoTrainer::repeats = 0;
    MorseModeEchoTrainer::echoTrainerWord = newWord;
}

void MorseModeEchoTrainer::onFetchNewWord()
{
    Decoder::interWordTimerOff();

    Serial.println("MET::oFNW() 1 - " + String(MorseModeEchoTrainer::getState()));
    if (MorseModeEchoTrainer::getState() == MorseModeEchoTrainer::REPEAT_WORD)
    {
        Serial.println("MET::oFNW() 2 " + String(MorseModeEchoTrainer::repeats));
        if (MorsePreferences::prefs.echoRepeats != MorsePreferences::REPEAT_FOREVER
                && MorseModeEchoTrainer::repeats >= MorsePreferences::prefs.echoRepeats)
        {
            Serial.println("MET::oFNW() 3");
            if (metConfig.showFailedWord)
            {
                Serial.println("MET::oFNW() 4");
                String result = MorseText::getCurrentWord();
                MorseDisplay::printToScroll(INVERSE_REGULAR, MorseText::cleanUpProSigns(result)); //// clean up first!
                MorseDisplay::printToScroll(REGULAR, " ");
            }
        }
    }
}

void MorseModeEchoTrainer::onLastWord()
{
    MorseModeEchoTrainer::echoStop = true;
    if (MorseModeEchoTrainer::isState(MorseModeEchoTrainer::REPEAT_WORD))
    {
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::SEND_WORD);
    }

}

boolean MorseModeEchoTrainer::loop()
{
//Serial.println("MorseModeEchoTrainer loop() 1");
    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        Serial.println("MorseModeEchoTrainer loop() 2 stopFlag");
        MorseModeEchoTrainer::active = MorseGenerator::stopFlag = false;
        MorseGenerator::keyOut(false, true, 0, 0);
        MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
    }
    if (!MorseModeEchoTrainer::active && (MorseKeyer::leftKey || MorseKeyer::rightKey))
    {                       // touching a paddle starts  the generation of code
        Serial.println("MorseModeEchoTrainer loop() 3 paddle hit");
        // for debouncing:
        while (MorseKeyer::checkPaddles())
        {
            ;                                                           // wait until paddles are released
        }
        MorseModeEchoTrainer::active = !MorseModeEchoTrainer::active;

        MorseMenu::cleanStartSettings();
    } /// end touch to start
    if (MorseModeEchoTrainer::active)
    {
//        Serial.println("MorseModeEchoTrainer loop() 4 active");
        switch (MorseModeEchoTrainer::getState())
        {
            case MorseModeEchoTrainer::START_ECHO:
            case MorseModeEchoTrainer::SEND_WORD:
            case MorseModeEchoTrainer::REPEAT_WORD:
//                Serial.println("MorseModeEchoTrainer loop() 5 send");
                MorseModeEchoTrainer::echoResponse = "";
                MorseGenerator::generateCW();
                break;
            case MorseModeEchoTrainer::EVAL_ANSWER:
                Serial.println("MorseModeEchoTrainer loop() 6 eval");
                MorseModeEchoTrainer::echoTrainerEval();
                break;
            case MorseModeEchoTrainer::COMPLETE_ANSWER:
            case MorseModeEchoTrainer::GET_ANSWER:
//                Serial.println("MorseModeEchoTrainer loop() 7 get answer");
                if (MorseKeyer::doPaddleIambic())
                    return true;                             // we are busy keying and so need a very tight loop !
                break;
        }
    }
    return false;
}
