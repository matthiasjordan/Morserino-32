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
#include "MorseModeEchoTrainer.h"
#include "MorsePlayerFile.h"

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

const MorseChar MorseText::morseChars[] = {//
        {"a", "12", ""},  //
        {"b", "2111", ""},  //
        {"c", "2121", ""}, //
        {"d", "211", ""}, //
        {"e", "1", ""}, //
        {"f", "1121", ""}, //
        {"g", "221", ""},  //
        {"h", "1111", ""},  //
        {"i", "11", ""},  //
        {"j", "1222", ""},  //
        {"k", "212", ""},  //
        {"l", "1211", ""},  //
        {"m", "22", ""},  //
        {"n", "21", ""},  //
        {"o", "222", ""},  //
        {"p", "1221", ""},  //
        {"q", "2212", ""},  //
        {"r", "121", ""},  //
        {"s", "111", ""},  //
        {"t", "2", ""},  //
        {"u", "112", ""},  //
        {"v", "1112", ""},  //
        {"w", "122", ""},  //
        {"x", "2112", ""},  //
        {"y", "2122", ""},  //
        {"z", "2211", ""},  //
        {"0", "22222", ""},  //
        {"1", "12222", ""},  //
        {"2", "11222", ""},  //
        {"3", "11122", ""},  //
        {"4", "11112", ""},  //
        {"5", "11111", ""},  //
        {"6", "21111", ""},  //
        {"7", "22111", ""},  //
        {"8", "22211", ""},  //
        {"9", "22221", ""},  //

        {".", "121212", ""},  //
        {",", "221122", ""},  //
        {":", "222111", ""},  //
        {"-", "211112", ""},  //
        {"/", "21121", ""},  //
        {"=", "21112", ""},  //
        {"?", "112211", ""},  //
        {"@", "122121", ""},  //

        {"ä", "1212", ""},  // ae
        {"ö", "2221", ""},  // oe
        {"ü", "1122", ""},  // ue

        {"+", "12121", "<ar>"},  //   (at the same time <ar> !)
        {"S", "12111", "<as>"},  //
        {"A", "21212", "<ka>"},  //
        {"N", "21221", "<kn>"},  //
        {"K", "111212", "<sk>"},   //
        {"E", "11121", "<ve>"},  //
        {"H", "2222", "<ch>"},   //
        {"X", "111222111", "<sos>"}, //
        {"R", "11111111", "<err>"}, //
        {"*", "", ""}
};

MorseText::Config config;

uint8_t repetitionsLeft = 0;
String lastGeneratedWord = "";
boolean nextWordIsEndSequence;
boolean repeatLast;
void (*MorseText::onGeneratorNewWord)(String);

void MorseText::start(GEN_TYPE genType)
{
    onGeneratorNewWord = &voidFunction;
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

void MorseText::setRepeatLast()
{
    repeatLast = true;
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
    else if (repeatLast)
    {
        repeatLast = false;
        result = lastGeneratedWord;
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
        MorseText::onGeneratorNewWord(result);
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
            Serial.println("internal::fRW " + String(MorsePreferences::prefs.randomLength));
            word = internal::getRandomChars(MorsePreferences::prefs.randomLength, MorsePreferences::prefs.randomOption);
            Serial.println("internal::fRW " + String(MorsePreferences::prefs.randomLength) + " -> " + word);
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
    Serial.println("internal::fRW " + String(MorsePreferences::prefs.randomLength) + " -> returning " + word);
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
        result += morseChars[random(s, e)].internal;
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
            call += String(morseChars[random(0, 26)].internal);
            call += morseChars[random(0, 26)].internal;
            l += 2;
            break;
        case 0:
            call += morseChars[random(0, 26)].internal;
            ++l;
            break;
        case 2:
            call += morseChars[random(0, 26)].internal;
            call += morseChars[random(26, 36)].internal;
            l = 2;
            break;
        case 3:
            call += morseChars[random(26, 36)].internal;
            call += morseChars[random(0, 26)].internal;
            l = 2;
            break;
    } // we have a prefix by now; l is its length
      // now generate a number
    call += morseChars[random(26, 36)].internal;
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
        call += morseChars[random(0, 26)].internal;
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

int MorseText::findChar(char c)
{
    String cStr = String(c);
    int pos = -1;
    for (int i = 0; morseChars[i].code != ""; i++)
    {
        if (morseChars[i].internal == cStr)
        {
            pos = i;
            break;
        }
    }
    return pos;
}


String MorseText::internalToProSigns(String &input)
{
    /// clean up clearText   -   S <as>,  - A <ka> - N <kn> - K <sk> - H ch;
    int i = 0;
    while (morseChars[i].prosign == "") {
        i += 1;
    }
    while (morseChars[i].code != "") {
        MorseChar m = morseChars[i];
        input.replace(m.internal, m.prosign);
        i += 1;
    }
    //Serial.println(input);
    return input;
}

String MorseText::proSignsToInternal(String &input)
{
    int i = 0;
    while (morseChars[i].prosign == "") {
        i += 1;
    }
    while (morseChars[i].prosign != "") {
        MorseChar m = morseChars[i];
        input.replace(m.prosign, m.internal);
        i += 1;
    }
    return input;
}

String MorseText::utf8umlaut(String s)
{ /// replace umtf umlauts with digraphs, and interpret pro signs, written e.g. as [kn] or <kn>
    s.replace("ä", "ae");
    s.replace("ö", "oe");
    s.replace("ü", "ue");
    s.replace("Ä", "ae");
    s.replace("Ö", "oe");
    s.replace("Ü", "ue");
    s.replace("ß", "ss");
    s.replace("[", "<");
    s.replace("]", ">");
    proSignsToInternal(s);
    return s;
}
