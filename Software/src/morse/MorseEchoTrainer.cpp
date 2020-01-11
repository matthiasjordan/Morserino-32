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

using namespace MorseEchoTrainer;

String MorseEchoTrainer::echoResponse = "";
echoStates echoTrainerState = START_ECHO;
boolean MorseEchoTrainer::echoStop = false;                         // for maxSequence
boolean MorseEchoTrainer::active = false;                           // flag for trainer mode
String MorseEchoTrainer::echoTrainerWord;
int MorseEchoTrainer::repeats = 0;

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
    MorseEchoTrainer::echoStop = false;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0,
            MorseGenerator::generatorMode == MorseGenerator::KOCH_LEARN ? "New Character:" : "Echo Trainer:");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start:       ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Press paddle ");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer
    MorseKeyer::keyTx = false;
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
    }
    else
    {
        echoTrainerState = REPEAT_WORD;
        if (MorseGenerator::generatorMode != MorseGenerator::KOCH_LEARN || echoResponse != "")
        {
            MorseDisplay::printToScroll(BOLD, "ERR");
            if (MorsePreferences::prefs.echoConf)
            {
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
                if (MorsePreferences::prefs.echoDisplay != CODE_ONLY)
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


void MorseEchoTrainer::onGeneratorNewWord(String newWord) {
    MorseEchoTrainer::repeats = 0;
    MorseEchoTrainer::echoTrainerWord = newWord;
}
