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

#include "MorseEchoTrainer.h"
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

using namespace MorseEchoTrainer;

String MorseEchoTrainer::echoResponse = "";
echoStates echoTrainerState;
boolean MorseEchoTrainer::echoStop = false;                         // for maxSequence
boolean MorseEchoTrainer::active = false;                           // flag for trainer mode
String MorseEchoTrainer::echoTrainerWord;
int MorseEchoTrainer::repeats = 0;

MorseEchoTrainer::Config metConfig;

boolean MorseEchoTrainer::menuExec(String mode)
{
    if (mode == "player")
    {
        MorsePlayerFile::openAndSkip();
    }
    MorseEchoTrainer::startEcho();
    return true;
}

void MorseEchoTrainer::startEcho()
{
    MorseMachine::morseState = MorseMachine::echoTrainer;
    MorseGenerator::setup();

    MorseText::proceed();

    MorseEchoTrainer::echoStop = false;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0, MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn) ? "New Character:" : "Echo Trainer:");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start:       ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Press paddle ");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer
    MorseKeyer::keyTx = false;
    MorseEchoTrainer::onPreferencesChanged();

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
}

void MorseEchoTrainer::onPreferencesChanged()
{
    Serial.println("MorseET::oPC");

    metConfig.showPrompt = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);

    MorseText::setRepeatEach(MorsePreferences::prefs.echoRepeats);

    MorseGenerator::Config *generatorConfig = MorseGenerator::getConfig();
    Serial.println("MorseEchoTrainer start edis " + String(MorsePreferences::prefs.echoDisplay));
    generatorConfig->key = (MorsePreferences::prefs.echoDisplay != DISP_ONLY);
    generatorConfig->printDitDah = false;
    generatorConfig->printChar = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);
    generatorConfig->wordEndMethod = MorseGenerator::LF;
//    generatorConfig->printSpaceAfterWord = true;
    generatorConfig->timing = (MorsePreferences::prefs.echoDisplay == DISP_ONLY) ? MorseGenerator::quick : MorseGenerator::tx;
}

boolean MorseEchoTrainer::isState(echoStates state)
{
    return echoTrainerState == state;
}

void MorseEchoTrainer::setState(echoStates newState)
{
    echoTrainerState = newState;
}

echoStates MorseEchoTrainer::getState()
{
    return echoTrainerState;
}

void MorseEchoTrainer::storeCharInResponse(String symbol)
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
void MorseEchoTrainer::echoTrainerEval()
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
            changeSpeed(1);
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
            changeSpeed(-1);
    }
    echoResponse = "";
    MorseKeyer::clearPaddleLatches();
}   // end of function

void MorseEchoTrainer::changeSpeed(int t)
{
    MorsePreferences::prefs.wpm += t;
    MorsePreferences::prefs.wpm = constrain(MorsePreferences::prefs.wpm, 5, 60);
    MorseKeyer::updateTimings();
    MorseDisplay::displayCWspeed();                     // update display of CW speed
    MorsePreferences::charCounter = 0;                                    // reset character counter
}

/**
 * @return: -1: NOOB,
 */
unsigned long MorseEchoTrainer::onGeneratorWordEnd()
{

    Serial.println("MET::onGenWordEnd MET::s: " + String(MorseEchoTrainer::getState()));

    if (!MorseMachine::isMode(MorseMachine::echoTrainer))
    {
        return -1;
    }

    unsigned long delta = 0;

    switch (MorseEchoTrainer::getState())
    {
        case MorseEchoTrainer::START_ECHO:
        {
            MorseEchoTrainer::setState(MorseEchoTrainer::SEND_WORD);
            delta = MorseKeyer::interCharacterSpace + (MorsePreferences::prefs.promptPause * MorseKeyer::interWordSpace);
            break;
        }
        case MorseEchoTrainer::REPEAT_WORD:
            // fall through
        case MorseEchoTrainer::SEND_WORD:
        {
            if (MorseEchoTrainer::echoStop)
            {
                break;
            }
            else
            {
                MorseEchoTrainer::setState(MorseEchoTrainer::GET_ANSWER);
                if (metConfig.showPrompt)
                {
                    MorseDisplay::printToScroll(REGULAR, " ");
                    MorseDisplay::printToScroll(INVERSE_REGULAR, ">");    /// add a blank after the word on the display
                }
                ++MorseEchoTrainer::repeats;
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

void MorseEchoTrainer::onGeneratorNewWord(String newWord)
{
    Serial.println("MET::onGNW");
    MorseEchoTrainer::repeats = 0;
    MorseEchoTrainer::echoTrainerWord = newWord;
}

void MorseEchoTrainer::onFetchNewWord()
{
    Decoder::interWordTimerOff();

    Serial.println("MET::oFNW() 1 - " + String(MorseEchoTrainer::getState()));
    if (MorseEchoTrainer::getState() == MorseEchoTrainer::REPEAT_WORD)
    {
        Serial.println("MET::oFNW() 2 " + String(MorseEchoTrainer::repeats));
        if (MorsePreferences::prefs.echoRepeats != MorsePreferences::REPEAT_FOREVER
                && MorseEchoTrainer::repeats >= MorsePreferences::prefs.echoRepeats)
        {
            Serial.println("MET::oFNW() 3");
            if (metConfig.showFailedWord)
            {
                Serial.println("MET::oFNW() 4");
                String result = MorseText::getCurrentWord();
                MorseDisplay::printToScroll(INVERSE_REGULAR, MorseDisplay::cleanUpProSigns(result)); //// clean up first!
                MorseDisplay::printToScroll(REGULAR, " ");
            }
        }
    }
}

void MorseEchoTrainer::onLastWord()
{
    MorseEchoTrainer::echoStop = true;
    if (MorseEchoTrainer::isState(MorseEchoTrainer::REPEAT_WORD))
    {
        MorseEchoTrainer::setState(MorseEchoTrainer::SEND_WORD);
    }

}

boolean MorseEchoTrainer::loop()
{
//Serial.println("MorseEchoTrainer loop() 1");
    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        Serial.println("MorseEchoTrainer loop() 2 stopFlag");
        MorseEchoTrainer::active = MorseGenerator::stopFlag = false;
        MorseGenerator::keyOut(false, true, 0, 0);
        MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
    }
    if (!MorseEchoTrainer::active && (MorseKeyer::leftKey || MorseKeyer::rightKey))
    {                       // touching a paddle starts  the generation of code
        Serial.println("MorseEchoTrainer loop() 3 paddle hit");
        // for debouncing:
        while (MorseKeyer::checkPaddles())
        {
            ;                                                           // wait until paddles are released
        }
        MorseEchoTrainer::active = !MorseEchoTrainer::active;

        MorseMenu::cleanStartSettings();
    } /// end touch to start
    if (MorseEchoTrainer::active)
    {
//        Serial.println("MorseEchoTrainer loop() 4 active");
        switch (MorseEchoTrainer::getState())
        {
            case MorseEchoTrainer::START_ECHO:
            case MorseEchoTrainer::SEND_WORD:
            case MorseEchoTrainer::REPEAT_WORD:
//                Serial.println("MorseEchoTrainer loop() 5 send");
                MorseEchoTrainer::echoResponse = "";
                MorseGenerator::generateCW();
                break;
            case MorseEchoTrainer::EVAL_ANSWER:
                Serial.println("MorseEchoTrainer loop() 6 eval");
                MorseEchoTrainer::echoTrainerEval();
                break;
            case MorseEchoTrainer::COMPLETE_ANSWER:
            case MorseEchoTrainer::GET_ANSWER:
//                Serial.println("MorseEchoTrainer loop() 7 get answer");
                if (MorseKeyer::doPaddleIambic())
                    return true;                             // we are busy keying and so need a very tight loop !
                break;
        }
    }
    return false;
}
