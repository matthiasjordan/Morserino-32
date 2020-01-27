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

/////////////// READING and WRITING parameters from / into Non Volatile Storage, using ESP32 preferences
#include <LoRa.h>          // library for LoRa transceiver

#include "koch.h"
#include "abbrev.h"
#include "english_words.h"
#include "morsedefs.h"
#include "decoder.h"
#include "MorsePreferences.h"
#include "MorsePreferencesMenu.h"
#include "MorseDisplay.h"
#include "MorseUI.h"

using namespace MorsePreferences;

const String MorsePreferences::prefOption[] =
    {"Encoder Click", "Tone Pitch Hz", "External Pol.", "Paddle Polar.", "Keyer Mode   ", "CurtisB DahT%", "CurtisB DitT%", "AutoChar Spce",
            "Tone Shift   ", "InterWord Spc", "InterChar Spc", "Random Groups", "Length Rnd Gr", "Length Calls ", "Length Abbrev",
            "Length Words ", "CW Gen Displ ", "Each Word 2x ", "Echo Prompt  ", "Echo Repeats ", "Confrm. Tone ", "Key ext TX   ",
            "Send via LoRa", "Bandwidth    ", "Adaptv. Speed", "Koch Sequence", "Koch         ", "Latency      ", "Randomize File",
            "Time Out     ", "Quick Start  ", "LoRa Channel  ", "LoRa Band    ", "LoRa Frequ   ", "RECALLSnapshot", "STORE Snapshot",
            "Max # of Words",
            //
            "Sentinel"};

prefPos MorsePreferences::keyerOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posKeyTrainerMode, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::generatorOptions[] =
    {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomOption, posRandomLength, posCallLength,
            posAbbrevLength, posWordLength, posMaxSequence, posTrainerDisplay, posWordDoubler, posKeyTrainerMode, posLoraTrainerMode,
            posLoraSyncW, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::headOptions[] =
    {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomOption, posRandomLength, posCallLength,
            posAbbrevLength, posWordLength, posMaxSequence, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posTimeOut, posQuickStart,
            sentinel};
prefPos MorsePreferences::playerOptions[] =
    {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posMaxSequence, posTrainerDisplay, posRandomFile,
            posWordDoubler, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::echoPlayerOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posInterWordSpace, posInterCharSpace, posMaxSequence, posRandomFile, posEchoRepeats, posEchoDisplay,
            posEchoConf, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::echoTrainerOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption, posRandomLength, posCallLength, posAbbrevLength,
            posWordLength, posMaxSequence, posEchoRepeats, posEchoDisplay, posEchoConf, posSpeedAdapt, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::kochGenOptions[] =
    {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomLength, posAbbrevLength, posWordLength,
            posMaxSequence, posTrainerDisplay, posWordDoubler, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posKochSeq, posTimeOut,
            posQuickStart, sentinel};
prefPos MorsePreferences::kochEchoOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomLength, posAbbrevLength, posWordLength, posMaxSequence,
            posEchoRepeats, posEchoDisplay, posEchoConf, posSpeedAdapt, posKochSeq, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::loraTrxOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posTimeOut, posQuickStart, posLoraSyncW, sentinel};
prefPos MorsePreferences::extTrxOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posGoertzelBandwidth, posTimeOut, posQuickStart, sentinel};
prefPos MorsePreferences::decoderOptions[] =
    {posClicks, posPitch, posGoertzelBandwidth, posTimeOut, posQuickStart, sentinel};

prefPos MorsePreferences::allOptions[] =
    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
            posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption, posRandomLength, posCallLength, posAbbrevLength,
            posWordLength, posMaxSequence, posTrainerDisplay, posRandomFile, posWordDoubler, posEchoRepeats, posEchoDisplay, posEchoConf,
            posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posGoertzelBandwidth, posSpeedAdapt, posKochSeq, posTimeOut, posQuickStart,
            sentinel};

prefPos MorsePreferences::noOptions[] = {};

Preferences pref;               // use the Preferences library for storing and retrieving objects

/// variables for managing snapshots
uint8_t MorsePreferences::memories[8];
uint8_t MorsePreferences::memCounter;
uint8_t MorsePreferences::memPtr = 0;

MorsePrefs MorsePreferences::prefs;

prefPos *MorsePreferences::currentOptions = MorsePreferences::allOptions;

unsigned long MorsePreferences::charCounter = 25; // we use this to count characters after changing speed - after n characters we decide to write the config into NVS

MorsePrefs MorsePreferences::readPreferences(String repository)
{
    MorsePrefs p;
    unsigned int l = 15;
    char repName[l];
    uint8_t temp;
    uint32_t tempInt;

    boolean atStart = false;

    if (repository == "morserino")
        atStart = true;

    repository.toCharArray(repName, l);
    // MORSELOGLN("Reading from repository: " + String(repName));
    // read preferences from non-volatile storage
    // if version cannot be read, we have a new ESP32 and need to write the preferences first

    if (atStart)
        pref.begin(repName, false);                // open namespace in read/write mode
    else
        pref.begin(repName, true);                 // read only in all other cases

    /// new code for reading preferences values - we check if we have a value, and if yes, we use it; if no, we use and write a default value

    if (atStart)
    {
        if ((temp = pref.getUChar("version_major")) != p.version_major)
            pref.putUChar("version_major", p.version_major);
        if ((temp = pref.getUChar("version_minor")) != p.version_minor)
            pref.putUChar("version_minor", p.version_minor);
    }

    if ((temp = pref.getUChar("sidetoneFreq")))
        p.sidetoneFreq = temp;
    else if (atStart)
        pref.putUChar("sidetoneFreq", p.sidetoneFreq);

    if ((temp = pref.getUChar("wpm")))
        p.wpm = temp;
    else if (atStart)
        pref.putUChar("wpm", p.wpm);

    if ((temp = pref.getUChar("sidetoneVolume", 255)) != 255)
        p.sidetoneVolume = temp;
    else if (atStart)
        pref.putUChar("sidetoneVolume", p.sidetoneVolume);

    if ((temp = pref.getUChar("keyermode")))
        p.keyermode = temp;
    else if (atStart)
        pref.putUChar("keyermode", p.keyermode);

    if ((temp = pref.getUChar("farnsworthMode")))
        p.interCharSpace = temp;
    else if (atStart)
        pref.putUChar("farnsworthMode", p.interCharSpace);

    if ((temp = pref.getUChar("ACSlength", 255)) != 255)
        p.ACSlength = temp;
    else if (atStart)
        pref.putUChar("ACSlength", p.ACSlength);

    if ((temp = pref.getUChar("keyTrainerMode", 255)) != 255)
        p.keyTrainerMode = temp;
    else if (atStart)
        pref.putUChar("keyTrainerMode", p.keyTrainerMode);

    if ((temp = pref.getUChar("randomLength")))
        p.randomLength = temp;
    else if (atStart)
        pref.putUChar("randomLength", p.randomLength);

    if ((temp = pref.getUChar("randomOption", 255)) != 255)
        p.randomOption = temp;
    else if (atStart)
        pref.putUChar("randomOption", p.randomOption);

    if ((temp = pref.getUChar("callLength", 255)) != 255)
        p.callLength = temp;
    else if (atStart)
        pref.putUChar("callLength", p.callLength);

    if ((temp = pref.getUChar("abbrevLength", 255)) != 255)
        p.abbrevLength = temp;
    else if (atStart)
        pref.putUChar("abbrevLength", p.abbrevLength);

    if ((temp = pref.getUChar("wordLength", 255)) != 255)
        p.wordLength = temp;
    else if (atStart)
        pref.putUChar("wordLength", p.wordLength);

    if ((temp = pref.getUChar("trainerDisplay", 255)) != 255)
        p.trainerDisplay = temp;
    else if (atStart)
        pref.putUChar("trainerDisplay", p.trainerDisplay);

    if ((temp = pref.getUChar("echoDisplay", 255)) != 255)
        p.echoDisplay = temp;
    else if (atStart)
        pref.putUChar("echoDisplay", p.echoDisplay);

    if ((temp = pref.getUChar("curtisBTiming", 255)) != 255)
        p.curtisBTiming = temp;
    else if (atStart)
        pref.putUChar("curtisBTiming", p.curtisBTiming);

    if ((temp = pref.getUChar("curtisBDotT", 255)) != 255)
        p.curtisBDotTiming = temp;
    else if (atStart)
        pref.putUChar("curtisBDotT", p.curtisBDotTiming);

    if ((temp = pref.getUChar("interWordSpace")))
        p.interWordSpace = temp;
    else if (atStart)
        pref.putUChar("interWordSpace", p.interWordSpace);

    if ((temp = pref.getUChar("echoRepeats", 255)) != 255)
        p.echoRepeats = temp;
    else if (atStart)
        pref.putUChar("echoRepeats", p.echoRepeats);

    if ((temp = pref.getUChar("echoToneShift", 255)) != 255)
        p.echoToneShift = temp;
    else if (atStart)
        pref.putUChar("echoToneShift", p.echoToneShift);

    if (atStart)
    {
        if ((temp = pref.getUChar("kochFilter")))
            p.kochFilter = temp;
        else
            pref.putUChar("kochFilter", p.kochFilter);
    }

    if ((temp = pref.getUChar("loraTrainerMode")))
        p.loraTrainerMode = temp;
    else if (atStart)
        pref.putUChar("loraTrainerMode", p.loraTrainerMode);

    if ((temp = pref.getUChar("goertzelBW")))
        p.goertzelBandwidth = temp;
    else if (atStart)
        pref.putUChar("goertzelBW", p.goertzelBandwidth);

    if ((temp = pref.getUChar("latency")))
        p.latency = temp;
    else if (atStart)
        pref.putUChar("latency", p.latency);

    if ((temp = pref.getUChar("randomFile")))
        p.randomFile = temp;

    if ((temp = pref.getUChar("lastExecuted")))
        p.menuPtr = temp;
    //MORSELOGLN("read: p.menuPtr = " + String(p.menuPtr));

    if ((temp = pref.getUChar("timeOut")))
        p.timeOut = temp;
    else if (atStart)
        pref.putUChar("timeOut", p.timeOut);

    if ((temp = pref.getUChar("loraSyncW")))
        p.loraSyncW = temp;
    else if (atStart)
        pref.putUChar("loraSyncW", p.loraSyncW);

    if ((temp = pref.getUChar("maxSequence", p.maxSequence)))
        p.maxSequence = temp;

    p.didah = pref.getBool("didah", true);
    p.useExtPaddle = pref.getBool("useExtPaddle");
    p.encoderClicks = pref.getBool("encoderClicks", true);
    p.echoConf = pref.getBool("echoConf", true);
    p.wordDoubler = pref.getBool("wordDoubler");
    p.speedAdapt = pref.getBool("speedAdapt");

    p.wlanSSID = pref.getString("wlanSSID");
    p.wlanPassword = pref.getString("wlanPassword");
    p.fileWordPointer = pref.getUInt("fileWordPtr");
    p.lcwoKochSeq = pref.getBool("lcwoKochSeq");
    p.quickStart = pref.getBool("quickStart");

    if (atStart)
    {
        if ((temp = pref.getUChar("loraBand")))
            p.loraBand = temp;
        else
            p.loraBand = 0;

        if ((tempInt = pref.getUInt("loraQRG")))
        {
            p.loraQRG = tempInt;
        }
        else
            p.loraQRG = QRG433;

        if ((temp = pref.getUChar("snapShots")))
        {
            p.snapShots = temp;
            updateMemory(temp);
        }  // end: we have snapshots
    }  // endif atStart
    pref.end();

    prefs = p;
    return p;
}

void MorsePreferences::writePreferences(String repository)
{
    MorsePrefs p = prefs;
    unsigned int l = 15;
    char repName[l];

    boolean morserino = false;

    if (repository == "morserino")
        morserino = true;
//MORSELOGLN("Writing to repository: " + repository);
    repository.toCharArray(repName, l);

    pref.begin(repName, false);                // open namespace in read/write mode

    if (p.sidetoneFreq != pref.getUChar("sidetoneFreq"))
        pref.putUChar("sidetoneFreq", p.sidetoneFreq);
    if (p.didah != pref.getBool("didah"))
        pref.putBool("didah", p.didah);
    if (p.keyermode != pref.getUChar("keyermode"))
        pref.putUChar("keyermode", p.keyermode);
    if (p.interCharSpace != pref.getUChar("farnsworthMode"))
        pref.putUChar("farnsworthMode", p.interCharSpace);
    if (p.useExtPaddle != pref.getBool("useExtPaddle"))
        pref.putBool("useExtPaddle", p.useExtPaddle);
    if (p.ACSlength != pref.getUChar("ACSlength"))
        pref.putUChar("ACSlength", p.ACSlength);
    if (p.keyTrainerMode != pref.getUChar("keyTrainerMode"))
        pref.putUChar("keyTrainerMode", p.keyTrainerMode);
    if (p.encoderClicks != pref.getBool("encoderClicks"))
        pref.putBool("encoderClicks", p.encoderClicks);
    if (p.randomLength != pref.getUChar("randomLength"))
        pref.putUChar("randomLength", p.randomLength);
    if (p.randomOption != pref.getUChar("randomOption"))
        pref.putUChar("randomOption", p.randomOption);
    if (p.callLength != pref.getUChar("callLength"))
        pref.putUChar("callLength", p.callLength);
    if (p.abbrevLength != pref.getUChar("abbrevLength"))
    {
        pref.putUChar("abbrevLength", p.abbrevLength);
        if (morserino)
            Koch::createKochAbbr(p.abbrevLength, p.kochFilter); // update the abbrev array!
    }
    if (p.wordLength != pref.getUChar("wordLength"))
    {
        pref.putUChar("wordLength", p.wordLength);
        if (morserino)
            Koch::createKochWords(p.wordLength, p.kochFilter);  // update the word array!
    }
    if (p.trainerDisplay != pref.getUChar("trainerDisplay"))
        pref.putUChar("trainerDisplay", p.trainerDisplay);
    if (p.echoDisplay != pref.getUChar("echoDisplay"))
        pref.putUChar("echoDisplay", p.echoDisplay);
    if (p.curtisBTiming != pref.getUChar("curtisBTiming"))
        pref.putUChar("curtisBTiming", p.curtisBTiming);
    if (p.curtisBDotTiming != pref.getUChar("curtisBDotT"))
        pref.putUChar("curtisBDotT", p.curtisBDotTiming);
    if (p.interWordSpace != pref.getUChar("interWordSpace"))
        pref.putUChar("interWordSpace", p.interWordSpace);
    if (p.echoRepeats != pref.getUChar("echoRepeats"))
        pref.putUChar("echoRepeats", p.echoRepeats);
    if (p.echoToneShift != pref.getUChar("echoToneShift"))
        pref.putUChar("echoToneShift", p.echoToneShift);
    if (p.echoConf != pref.getBool("echoConf"))
        pref.putBool("echoConf", p.echoConf);
    if (morserino)
    {
        if (p.kochFilter != pref.getUChar("kochFilter"))
        {
            pref.putUChar("kochFilter", p.kochFilter);
            Koch::createKochWords(p.wordLength, p.kochFilter);  // update the arrays!
            Koch::createKochAbbr(p.abbrevLength, p.kochFilter);
        }
    }
    if (p.lcwoKochSeq != pref.getBool("lcwoKochSeq"))
    {
        pref.putBool("lcwoKochSeq", p.lcwoKochSeq);
        if (morserino)
        {
            Koch::updateKochChars(p.lcwoKochSeq);
            Koch::createKochWords(p.wordLength, p.kochFilter);  // update the arrays!
            Koch::createKochAbbr(p.abbrevLength, p.kochFilter);
        }
    }
    if (p.wordDoubler != pref.getBool("wordDoubler"))
        pref.putBool("wordDoubler", p.wordDoubler);
    if (p.speedAdapt != pref.getBool("speedAdapt"))
        pref.putBool("speedAdapt", p.speedAdapt);
    if (p.loraTrainerMode != pref.getUChar("loraTrainerMode"))
        pref.putUChar("loraTrainerMode", p.loraTrainerMode);
    if (p.goertzelBandwidth != pref.getUChar("goertzelBW"))
    {
        pref.putUChar("goertzelBW", p.goertzelBandwidth);
        if (morserino)
            Decoder::setupGoertzel();
    }
    if (p.loraSyncW != pref.getUChar("loraSyncW"))
    {
        pref.putUChar("loraSyncW", p.loraSyncW);
        if (morserino)
            LoRa.setSyncWord(p.loraSyncW);
    }
    if (p.maxSequence != pref.getUChar("maxSequence"))
        pref.putUChar("maxSequence", p.maxSequence);

    if (p.latency != pref.getUChar("latency"))
        pref.putUChar("latency", p.latency);
    if (p.randomFile != pref.getUChar("randomFile"))
        pref.putUChar("randomFile", p.randomFile);
    if (p.timeOut != pref.getUChar("timeOut"))
        pref.putUChar("timeOut", p.timeOut);
    if (p.quickStart != pref.getBool("quickStart"))
        pref.putBool("quickStart", p.quickStart);

    if (p.snapShots != pref.getUChar("snapShots"))
        pref.putUChar("snapShots", p.snapShots);

    if (!morserino)
    {
        pref.putUChar("lastExecuted", p.menuPtr);   // store last executed command in snapshots
        //MORSELOGLN("write: last executed: " + String(p.menuPtr));
    }

    pref.end();
}

boolean MorsePreferences::recallSnapshot()
{         // return true if we selected a real recall, false when it was cancelled
    String snapname;
    String text;

    memPtr = 0;
    MorsePreferencesMenu::displayKeyerPreferencesMenu(posSnapRecall);
    if (!MorsePreferencesMenu::adjustKeyerPreference(posSnapRecall))
    {
        //MORSELOGLN("recall memPtr: " + String(memPtr));
        text = "Snap " + String(memories[memPtr] + 1) + " RECALLD";
        if (memCounter)
        {
            if (memPtr != memCounter)
            {
                snapname = "snap" + String(memories[memPtr]);
                //MORSELOGLN("recall snapname: " + snapname);
                readPreferences(snapname);
                MorseDisplay::printOnScroll(2, BOLD, 0, text);
                //MORSELOGLN("after recall - p.menuPtr: " + String(p.menuPtr));
                delay(1000);
                return true;
            }
            return false;
        }
    }
    return false;

}

boolean MorsePreferences::storeSnapshot(uint8_t menu)
{        // return true if we selected a real store, false when it was cancelled
    String snapname;
    uint8_t mask = 1;
    String text;

    memPtr = 0;
    MorsePreferencesMenu::displayKeyerPreferencesMenu(posSnapStore);
    MorsePreferencesMenu::adjustKeyerPreference(posSnapStore);
    MorseUI::volButton.Update();
    //MORSELOGLN("store memPtr: " + String(memPtr));
    if (memPtr != 8)
    {
        MorsePreferences::prefs.menuPtr = menu;     // also store last menu selection
        //MORSELOGLN("menu: " + String(p.menuPtr));
        text = "Snap " + String(memPtr + 1) + " STORED ";
        snapname = "snap" + String(memPtr);
        //MORSELOGLN("store snapname: " + snapname);
        //MORSELOGLN("store: p.menuPtr = " + String(p.menuPtr));
        writePreferences(snapname);
        /// insert the correct bit into p.snapShots & update memory variables
        mask = mask << memPtr;
        prefs.snapShots = MorsePreferences::prefs.snapShots | mask;
        //MORSELOGLN("store p.snapShots: " + String(p.snapShots));
        MorseDisplay::printOnScroll(2, BOLD, 0, text);
        updateMemory(MorsePreferences::prefs.snapShots);
        delay(1000);
        return true;
    }
    return false;
}

void MorsePreferences::updateMemory(uint8_t temp)
{
    memCounter = 0;                 // create an array that contains the snapshots (memories) that are in use
    for (int i = 0; i < 8; ++i)
    {
        if (temp & 1)
        {     // mask rightmost bit
            memories[memCounter] = i;
            ++memCounter;
        }
        temp = temp >> 1;   // shift one position to the right
    }
}

void MorsePreferences::clearMemory(uint8_t ptr)
{
    String text = "Snap " + String(memories[ptr] + 1) + " CLEARED";

    prefs.snapShots &= ~(1 << memories[ptr]);     // clear the bit
    MorseDisplay::printOnScroll(2, BOLD, 0, text);
    updateMemory(prefs.snapShots);
    delay(1000);
}

void MorsePreferences::writeLoRaPrefs(uint8_t loraBand, uint32_t loraQRG)
{
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putUChar("loraBand", loraBand);
    pref.putUInt("loraQRG", loraQRG);
    pref.end();
}

void MorsePreferences::fireCharSeen(boolean wpmOnly)
{
    ++charCounter;                         // count this character

    // if we have seen 12 chars since changing speed, we write the config to Preferences
    if (charCounter == 12)
    {
        pref.begin("morserino", false);             // open the namespace as read/write
        pref.putUChar("wpm", MorsePreferences::prefs.wpm);
        if (!wpmOnly)
        {
            pref.putUChar("tLeft", MorsePreferences::prefs.tLeft);
            pref.putUChar("tRight", MorsePreferences::prefs.tRight);
        }
        pref.end();
    }

}

void MorsePreferences::writeWordPointer()
{
    if ((prefs.fileWordPointer != pref.getUInt("fileWordPtr")))
    {   // update word pointer if necessary (if we ran player before)
        pref.begin("morserino", false);              // open the namespace as read/write
        pref.putUInt("fileWordPtr", prefs.fileWordPointer);
        pref.end();
    }
}

void MorsePreferences::writeVolume()
{
    pref.begin("morserino", false);                     // open the namespace as read/write
    if (pref.getUChar("sidetoneVolume") != MorsePreferences::prefs.sidetoneVolume)
        pref.putUChar("sidetoneVolume", MorsePreferences::prefs.sidetoneVolume);  // store the last volume, if it has changed
    pref.end();
}

void MorsePreferences::writeLastExecuted(uint8_t menuPtr)
{
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putUChar("lastExecuted", menuPtr);   // store last executed command
    pref.end();                                 // close namespace
}

void MorsePreferences::writeWifiInfo(String SSID, String passwd)
{
    MorsePreferences::prefs.wlanSSID = SSID;
    MorsePreferences::prefs.wlanPassword = passwd;
    //MORSELOGLN("SSID: " + MorsePreferences::prefs.wlanSSID + " Password: " + MorsePreferences::prefs.wlanPassword);
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putString("wlanSSID", MorsePreferences::prefs.wlanSSID);
    pref.putString("wlanPassword", MorsePreferences::prefs.wlanPassword);
    pref.end();

}

