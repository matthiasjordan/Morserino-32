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

#include <Arduino.h>
#include "MorseText.h"
#include "MorsePreferences.h"
#include "koch.h"
#include "english_words.h"
#include "abbrev.h"
#include "MorsePlayerFile.h"
#include "MorseEchoTrainer.h"

using namespace MorseText;

namespace internal
{
    String getRandomChars(int maxLength, int option);
    String getRandomCall(int maxLength);
    String getRandomWord(int maxLength);
    String getRandomAbbrev(int maxLength);
    String getRandomCWChars(int option, int maxLength);

    String fetchRandomWord();
}

const String CWchars = "abcdefghijklmnopqrstuvwxyz0123456789.,:-/=?@+SANKVäöüH";

MorseText::Config config;

uint8_t repetitionsLeft = 0;
String lastGeneratedWord = "";
boolean nextWordIsEndSequence;

void MorseText::start(GEN_TYPE genType)
{
    config.generateStartSequence = true;
    config.generatorMode = genType;
    MorseText::onPreferencesChanged();
}

void MorseText::setGenerateStartSequence(boolean newValue)
{
    config.generateStartSequence = newValue;
}

void MorseText::setTextSource(GEN_TYPE genType)
{
    config.generatorMode = genType;
}

void MorseText::setNextWordIsEndSequence()
{
    nextWordIsEndSequence = true;
}

void MorseText::setRepeatEach(uint8_t n)
{
    Serial.println("MorseText::setRepEach " + String(n));
    config.repeatEach = n;
}

void MorseText::onPreferencesChanged()
{
    config.repeatEach = MorsePreferences::prefs.wordDoubler ? 2 : 1;
}

String MorseText::getCurrentWord()
{
    return lastGeneratedWord;
}

void MorseText::proceed()
{
    repetitionsLeft = 0;
}

String MorseText::generateWord()
{
    String result = "";

    if (config.generateStartSequence == true)
    {                                 /// do the initial sequence in trainer mode, too
        result = "vvvA";
        config.generateStartSequence = false;
        repetitionsLeft = 0;
        Serial.println("genWord(): generating 1 start sequence");
    }
    else if (nextWordIsEndSequence)
    {
        result = "+";
        nextWordIsEndSequence = false;
        Serial.println("genWord(): generating end sequence");
    }
    else if ((config.repeatEach == MorsePreferences::REPEAT_FOREVER) || repetitionsLeft)
    {
        Serial.println("genWord(): repeating last word - reps left: " + String(repetitionsLeft));
        result = lastGeneratedWord;
        if (repetitionsLeft > 0)
        {
            repetitionsLeft -= 1;
            Serial.println("genWord(): decreased reps left - now: " + String(repetitionsLeft));
        }
    }
    else
    {
        Serial.println("genWord(): generating new word");
        repetitionsLeft = config.repeatEach - 1;
        result = internal::fetchRandomWord();
        MorseEchoTrainer::onGeneratorNewWord(result);
    }       /// end if else - we either already had something in trainer mode, or we got a new word

    lastGeneratedWord = result;
    Serial.println("genWord(): returning " + result);
    return result;

}

String internal::fetchRandomWord()
{
    String word = "";

    switch (config.generatorMode)
    {
        case RANDOMS:
        {
            word = internal::getRandomChars(MorsePreferences::prefs.randomLength, MorsePreferences::prefs.randomOption);
            break;
        }
        case CALLS:
        {
            word = internal::getRandomCall(MorsePreferences::prefs.callLength);
            break;
        }
        case ABBREVS:
        {
            word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
            break;
        }
        case WORDS:
        {
            word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
            break;
        }
        case KOCH_LEARN:
        {
            word = Koch::getChar(MorsePreferences::prefs.kochFilter);
            break;
        }
        case MIXED:
        {
            switch (random(4))
            {
                case 0:
                    word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
                    break;
                case 1:
                    word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                    break;
                case 2:
                    word = internal::getRandomCall(MorsePreferences::prefs.callLength);
                    break;
                case 3:
                    word = internal::getRandomChars(1, OPT_PUNCTPRO); // just a single pro-sign or interpunct
                    break;
            }
            break;
        }
        case KOCH_MIXED:
        {
            switch (random(3))
            {
                case 0:
                    word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
                    break;
                case 1:
                    word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                    break;
                case 2:
                    word = internal::getRandomChars(MorsePreferences::prefs.randomLength, OPT_KOCH); // Koch option!
                    break;
            }
            break;
        }
        case PLAYER:
        {
            if (MorsePreferences::prefs.randomFile)
            {
                MorsePlayerFile::skipWords(random(MorsePreferences::prefs.randomFile + 1));
            }
            word = MorsePlayerFile::getWord();
            break;
        }
        case NA:
        {
            break;
        }
    } // end switch (generatorMode)
    return word;
}

String internal::getRandomCWChars(int option, int maxLength)
{
    int s = 0, e = 50;
    switch (option)
    {
        case OPT_NUM:
        case OPT_NUMPUNCT:
        case OPT_NUMPUNCTPRO:
            s = 26;
            break;
        case OPT_PUNCT:
        case OPT_PUNCTPRO:
            s = 36;
            break;
        case OPT_PRO:
            s = 44;
            break;
        default:
            s = 0;
            break;
    }
    switch (option)
    {
        case OPT_ALPHA:
            e = 26;
            break;
        case OPT_ALNUM:
        case OPT_NUM:
            e = 36;
            break;
        case OPT_ALNUMPUNCT:
        case OPT_NUMPUNCT:
        case OPT_PUNCT:
            e = 45;
            break;
        default:
            e = 50;
            break;
    }
    String result = "";
    for (int i = 0; i < maxLength; ++i)
    {
        result += CWchars.charAt(random(s, e));
    }

    return result;
}

// we use substrings as char pool for trainer mode
// SANK will be replaced by <as>, <ka>, <kn> and <sk>
// Options:
//    0: a9?<> = CWchars; all of them; same as Koch 45+
//    1: a = CWchars.substring(0,26);
//    2: 9 = CWchars.substring(26,36);
//    3: ? = CWchars.substring(36,45);
//    4: <> = CWchars.substring(44,50);
//    5: a9 = CWchars.substring(0,36);
//    6: 9? = CWchars.substring(26,45);
//    7: ?<> = CWchars.substring(36,50);
//    8: a9? = CWchars.substring(0,45);
//    9: 9?<> = CWchars.substring(26,50);

//  {OPT_ALL, OPT_ALPHA, OPT_NUM, OPT_PUNCT, OPT_PRO, OPT_ALNUM, OPT_NUMPUNCT, OPT_PUNCTPRO, OPT_ALNUMPUNCT, OPT_NUMPUNCTPRO}

String internal::getRandomChars(int maxLength, int option)
{             /// random char string, eg. group of 5, 9 differing character pools; maxLength = 1-6
    String result;

    if (maxLength > 6)
    {                                        // we use a random length!
        maxLength = random(2, maxLength - 3);                     // maxLength is max 10, so random upper limit is 7, means max 6 chars...
    }

    if (Koch::isKochActive())
    {
        result = Koch::getRandomChars(maxLength);
    }
    else
    {
        result = internal::getRandomCWChars(option, maxLength);
    }
    return result;
}

String internal::getRandomCall(int maxLength)
{            // random call-sign like pattern, maxLength = 3 - 6, 0 returns any length
    const byte prefixType[] = {1, 0, 1, 2, 3, 1};         // 0 = a, 1 = aa, 2 = a9, 3 = 9a
    byte prefix;
    String call = "";
    unsigned int l = 0;
    //int s, e;

    if (maxLength == 1 || maxLength == 2)
        maxLength = 3;
    if (maxLength > 6)
        maxLength = 6;

    if (maxLength == 3)
        prefix = 0;
    else
        prefix = prefixType[random(0, 6)];           // what type of prefix?
    switch (prefix)
    {
        case 1:
            call += CWchars.charAt(random(0, 26));
            ++l;
        case 0:
            call += CWchars.charAt(random(0, 26));
            ++l;
            break;
        case 2:
            call += CWchars.charAt(random(0, 26));
            call += CWchars.charAt(random(26, 36));
            l = 2;
            break;
        case 3:
            call += CWchars.charAt(random(26, 36));
            call += CWchars.charAt(random(0, 26));
            l = 2;
            break;
    } // we have a prefix by now; l is its length
      // now generate a number
    call += CWchars.charAt(random(26, 36));
    ++l;
    // generate a suffix, 1 2 or 3 chars long - we re-use prefix for the suffix length
    if (maxLength == 3)
        prefix = 1;
    else if (maxLength == 0)
    {
        prefix = random(1, 4);
        prefix = (prefix == 2 ? prefix : random(1, 4)); // increase the likelihood for suffixes of length 2
    }
    else
    {
        //prefix = random(1,_min(maxLength-l+1, 4));     // suffix not longer than 3 chars!
        prefix = _min(maxLength - l, 3);                 // we try to always give the desired length, but never more than 3 suffix chars
    }
    while (prefix--)
    {
        call += CWchars.charAt(random(0, 26));
        ++l;
    } // now we have the suffix as well
      // are we /p or /m? - we do this only in rare cases - 1 out of 9, and only when maxLength = 0, or maxLength-l >= 2
    if (maxLength == 0) //|| maxLength - l >= 2)
        if (!random(0, 8))
        {
            call += "/";
            call += (random(0, 2) ? "m" : "p");
        }
    // we have a complete call sign!
    return call;
}

String internal::getRandomWord(int maxLength)
{        //// give me a random English word, max maxLength chars long (1-5) - 0 returns any length
    if (maxLength > 5)
        maxLength = 0;
    if (Koch::isKochActive())
        return Koch::getRandomWord();
    else
        return EnglishWords::getRandomWord(maxLength);
}

String internal::getRandomAbbrev(int maxLength)
{        //// give me a random CW abbreviation , max maxLength chars long (1-5) - 0 returns any length
    if (maxLength > 5)
        maxLength = 0;
    if (Koch::isKochActive())
        return Koch::getRandomAbbrev();
    else
        return Abbrev::getRandomAbbrev(maxLength);
}
