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
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "MorseDisplay.h"
#include "MorseGenerator.h"
#include "MorsePlayerFile.h"
#include "MorseMachine.h"
#include "MorseMenu.h"
#include "decoder.h"
#include "MorseInput.h"

MorseModeEchoTrainer morseModeEchoTrainer;

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

    MorseInput::start([](String r)
    {
        MorseDisplay::printToScroll(FONT_OUTGOING, r);
        morseModeEchoTrainer.storeCharInResponse(r);
    }, []()
    {
        MorseDisplay::printToScroll(FONT_OUTGOING, " ");
        morseModeEchoTrainer.onKeyerWordEndNDitDah();
    });

    MorseText::proceed();
    MorseText::onGeneratorNewWord = [](String r)
    {   morseModeEchoTrainer.onGeneratorNewWord(r);};

    MorseDisplay::getKeyerModeSymbol = MorseDisplay::getKeyerModeSymbolWStraightKey;
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
    metConfig.speedUpAfter = 5;
    metConfig.speedDownAfter = 3;

    MorseText::getConfig()->generateStartSequence = metConfig.generateStartSequence;

    echoStop = false;
    active = false;
    repeats = 0;
    correctInARow = 0;
    wrongInARow = 0;

    if (metConfig.generateStartSequence)
    {
        echoTrainerState = START_ECHO;
    }
    else
    {
        echoTrainerState = SEND_WORD;
    }
}

boolean MorseModeEchoTrainer::onKeyerWordEnd()
{
    if (MorseModeEchoTrainer::isState(MorseModeEchoTrainer::COMPLETE_ANSWER))
    {       // change the state of the trainer at end of word
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::EVAL_ANSWER);
        return false;
    }
    return true;
}

void MorseModeEchoTrainer::onKeyerWordEndDitDah()
{
    MorseModeEchoTrainer::setState(MorseModeEchoTrainer::COMPLETE_ANSWER);
}

void MorseModeEchoTrainer::onKeyerWordEndNDitDah()
{
    if (MorseModeEchoTrainer::isState(MorseModeEchoTrainer::GET_ANSWER))
    {
        MorseModeEchoTrainer::setState(MorseModeEchoTrainer::EVAL_ANSWER);
    }
}

void MorseModeEchoTrainer::onPreferencesChanged()
{
    metConfig.showPrompt = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);

    MorseText::Config *texCon = MorseText::getConfig();
    texCon->repeatEach = MorsePreferences::prefs.echoRepeats;

    MorseGenerator::Config *generatorConfig = MorseGenerator::getConfig();
    generatorConfig->key = (MorsePreferences::prefs.echoDisplay != DISP_ONLY);
    generatorConfig->printDitDah = false;
    generatorConfig->printChar = (MorsePreferences::prefs.echoDisplay != CODE_ONLY);
    generatorConfig->wordEndMethod = MorseGenerator::flush;
    generatorConfig->timing = (MorsePreferences::prefs.echoDisplay == DISP_ONLY) ? MorseGenerator::quick : MorseGenerator::tx;

    generatorConfig->onFetchNewWord = []()
    {   morseModeEchoTrainer.onFetchNewWord();};
    generatorConfig->onLastWord = []()
    {   morseModeEchoTrainer.onLastWord();};
    generatorConfig->onGeneratorWordEnd = []()
    {   return morseModeEchoTrainer.onGeneratorWordEnd();};

    MorseInput::setStraightKeyFromPrefs();
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
    symbol = MorseText::proSignsToInternal(symbol);
    echoResponse.concat(symbol);
}

///////// evaluate the response in Echo Trainer Mode
void MorseModeEchoTrainer::echoTrainerEval()
{
    delay(MorseKeyer::interCharacterSpace / 2);
    if (echoResponse == echoTrainerWord)
    {
        correctInARow += 1;
        wrongInARow = 0;
        echoTrainerState = SEND_WORD;
        MorseDisplay::printToScroll(BOLD, "OK\n");
        if (MorsePreferences::prefs.echoConf)
        {
            MorseSound::soundSignalOK();
        }
        delay(MorseKeyer::interWordSpace);
        if (MorsePreferences::prefs.speedAdapt && (correctInARow >= metConfig.speedUpAfter))
        {
            MorseKeyer::changeSpeed(1);
            correctInARow = 0;
        }
        MorseText::proceed();
    }
    else
    {
        correctInARow = 0;
        wrongInARow += 1;
        echoTrainerState = REPEAT_WORD;
        if (!MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn) || echoResponse != "")
        {
            MorseDisplay::printToScroll(BOLD, "ERR\n");
            if (MorsePreferences::prefs.echoConf)
            {
                MorseSound::soundSignalERR();
            }
        }
        else
        {
            MorseDisplay::printToScroll(REGULAR, "\n");
        }

        delay(MorseKeyer::interWordSpace);
        if (MorsePreferences::prefs.speedAdapt && (wrongInARow >= metConfig.speedDownAfter))
        {
            MorseKeyer::changeSpeed(-1);
            wrongInARow = 0;
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
    MorseModeEchoTrainer::repeats = 0;
    MorseModeEchoTrainer::echoTrainerWord = newWord;
}

void MorseModeEchoTrainer::onFetchNewWord()
{
    Decoder::interWordTimerOff();

    if (MorseModeEchoTrainer::getState() == MorseModeEchoTrainer::REPEAT_WORD)
    {
        if (MorsePreferences::prefs.echoRepeats != MorsePreferences::REPEAT_FOREVER
                && MorseModeEchoTrainer::repeats >= MorsePreferences::prefs.echoRepeats)
        {
            if (metConfig.showFailedWord)
            {
                String result = MorseText::getCurrentWord();
                MorseDisplay::printToScroll(INVERSE_REGULAR, MorseText::internalToProSigns(result)); //// clean up first!
                MorseDisplay::printToScroll(REGULAR, "\n");
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
    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        MorseModeEchoTrainer::active = MorseGenerator::stopFlag = false;
        MorseGenerator::keyOut(false, true, 0, 0);
        MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
    }
    if (!MorseModeEchoTrainer::active && (MorseKeyer::leftKey || MorseKeyer::rightKey))
    {                       // touching a paddle starts  the generation of code
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
        switch (MorseModeEchoTrainer::getState())
        {
            case MorseModeEchoTrainer::START_ECHO:
            case MorseModeEchoTrainer::SEND_WORD:
            case MorseModeEchoTrainer::REPEAT_WORD:
                MorseModeEchoTrainer::echoResponse = "";
                MorseGenerator::generateCW();
                break;
            case MorseModeEchoTrainer::EVAL_ANSWER:
                MorseModeEchoTrainer::echoTrainerEval();
                break;
            case MorseModeEchoTrainer::COMPLETE_ANSWER:
            case MorseModeEchoTrainer::GET_ANSWER:
                if (MorseInput::doInput())
                {
                    return true;
                }
                break;
        }
    }
    return false;
}
