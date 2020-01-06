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

#include "koch.h"
#include "MorsePreferences.h"
#include "abbrev.h"
#include "english_words.h"
#include "MorseGenerator.h"
#include "MorseEchoTrainer.h"

using namespace Koch;

const String morserinoKochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=K+SNAV@:";
const String lcwoKochChars = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x-K+ASNV@:";

boolean kochActive = false;                 // set to true when in Koch trainer mode

String Koch::kochChars;

void Koch::setup()
{
    if (MorsePreferences::prefs.lcwoKochSeq)
        kochChars = lcwoKochChars;
    else
        kochChars = morserinoKochChars;

    //// populate the array for abbreviations and words according to length and Koch filter
    createKochWords(MorsePreferences::prefs.wordLength, MorsePreferences::prefs.kochFilter);  //
    createKochAbbr(MorsePreferences::prefs.abbrevLength, MorsePreferences::prefs.kochFilter);
}

boolean Koch::menuExec(String mode)
{
    if (mode == "learn")
    {
        MorseEchoTrainer::startEcho();
    }
    else if (mode == "trainer")
    {
        Koch::setKochActive(true);
        MorseGenerator::startTrainer();
    }
    else if (mode == "echo")
    {
        Koch::setKochActive(true);
        MorseEchoTrainer::startEcho();
    }
    return true;
}

boolean Koch::isKochActive()
{
    return kochActive;
}

void Koch::setKochActive(boolean newActive)
{
    kochActive = newActive;
}

void Koch::updateKochChars(boolean lcwoKochSeq)
{
    kochChars = lcwoKochSeq ? lcwoKochChars : morserinoKochChars;
}

uint8_t Koch::wordIsKoch(String thisWord)
{
    uint8_t thisKoch = 0;
    uint8_t l = thisWord.length();
    for (int i = 0; i < l; ++i)
        thisKoch = _max(thisKoch, kochChars.indexOf(thisWord.charAt(i)) + 1);
    return thisKoch;
}

String Koch::getChar(uint8_t maxKochLevel)
{
    return (String) kochChars.charAt(maxKochLevel - 1);
}

String Koch::getRandomChars(int maxLength)
{
    String result;
    int endk = MorsePreferences::prefs.kochFilter;                        //              1   5    1    5    2    5    3    5    4    5    5
    for (int i = 0; i < maxLength; ++i)
    {
        if (random(2))                                    // in Koch mode, we generate the last third of the chars learned  a bit more often
            result = kochChars.charAt(random(2 * endk / 3, endk));
        else
            result = kochChars.charAt(random(endk));
    }
    return result;
}

String kochWords[EnglishWords::WORDS_NUMBER_OF_ELEMENTS];
int numberOfWords;

void Koch::createKochWords(uint8_t maxl, uint8_t koch)
{                  // this function creates an array of words that are compliant to Koch filter and max word length
    numberOfWords = 0;
    for (int i = EnglishWords::WORDS_POINTER[maxl]; i < EnglishWords::WORDS_NUMBER_OF_ELEMENTS; ++i)
    {     // do this for all words with max length maxl
        if (wordIsKoch(EnglishWords::words[i]) <= koch)
            kochWords[numberOfWords++] = EnglishWords::words[i];
    }
}

String Koch::getRandomWord()
{
    return kochWords[random(numberOfWords)];
}

String kochAbbr[Abbrev::ABBREV_NUMBER_OF_ELEMENTS];
int numberOfAbbr;

void Koch::createKochAbbr(uint8_t maxl, uint8_t koch)
{                  // this function creates an array of words that are compliant to Koch filter and max word length
    numberOfAbbr = 0;
    for (int i = Abbrev::ABBREV_POINTER[maxl]; i < Abbrev::ABBREV_NUMBER_OF_ELEMENTS; ++i)
    {     // do this for all words with max length maxl
        if (Koch::wordIsKoch(Abbrev::abbreviations[i]) <= koch)
            kochAbbr[numberOfAbbr++] = Abbrev::abbreviations[i];
    }
}

String Koch::getRandomAbbrev()
{
    return kochAbbr[random(numberOfAbbr)];
}

String Koch::filterNonKoch(String w)
{
    char c;
    String result = "";
    result.reserve(64);
    for (unsigned int i = 0; i < w.length(); ++i)
    {
        if (kochChars.indexOf(c = w.charAt(i)) != -1)
            result += c;
    }
    return result;
}
