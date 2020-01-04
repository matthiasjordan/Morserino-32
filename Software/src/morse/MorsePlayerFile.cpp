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
#include "FS.h"
#include "SPIFFS.h"
#include "MorsePlayerFile.h"
#include "MorsePreferences.h"
#include "koch.h"

using namespace MorsePlayerFile;

namespace internal
{
    String cleanUpText(String w);
    String utf8umlaut(String s);

}

File file;

void MorsePlayerFile::setup()
{
    ///////////////////////// mount (or create) SPIFFS file system
#define FORMAT_SPIFFS_IF_FAILED true

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {     ///// if SPIFFS cannot be mounted, it does not exist. So create  (format) it, and mount it
        //Serial.println("SPIFFS Mount Failed");
        return;
    }
    //////////////////////// create file player.txt if it does not exist|
    const char * defaultFile =
            "This is just an initial dummy file for the player. Dies ist nur die anfänglich enthaltene Standarddatei für den Player.\n"
                    "Did you not upload your own file? Hast du keine eigene Datei hochgeladen?";

    if (!SPIFFS.exists("/player.txt"))
    {                                    // file does not exist, therefor we create it from the text above
        File file = SPIFFS.open("/player.txt", FILE_WRITE);
        if (!file)
        {
            Serial.println("- failed to open file for writing");
            return;
        }
        if (file.print(defaultFile))
        {
            ;
        }
        else
        {
            Serial.println("- write failed");
        }
        file.close();
    }

}

void MorsePlayerFile::openAndSkip()
{
    uint32_t wcount = 0;
    file = SPIFFS.open("/player.txt");                            // open file
    //skip MorsePreferences::prefs.fileWordPointer words, as they have been played before
    wcount = MorsePreferences::prefs.fileWordPointer;
    MorsePreferences::prefs.fileWordPointer = 0;
    MorsePlayerFile::skipWords(wcount);
}

String MorsePlayerFile::getWord()
{
    String result = "";
    byte c;

    while (file.available())
    {
        c = file.read();
        //Serial.println((int) c);
        if (!isSpace(c))
            result += (char) c;
        else if (result.length() > 0)
        {               // end of word
            ++MorsePreferences::prefs.fileWordPointer;
            //Serial.println("word: " + result);
            return result;
        }
    }
    file.close();
    file = SPIFFS.open("/player.txt");
    MorsePreferences::prefs.fileWordPointer = 0;
    while (!file.available())
        ;
    result = internal::cleanUpText(result);
    return result;                                    // at eof
}

void MorsePlayerFile::skipWords(uint32_t count)
{             /// just skip count words in open file fn
    while (count > 0)
    {
        getWord();
        --count;
    }
}

String internal::cleanUpText(String w)
{                        // all to lower case, and convert umlauts
    w.toLowerCase();
    w = utf8umlaut(w);

    return Koch::filterNonKoch(w);
}

String internal::utf8umlaut(String s)
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
    s.replace("<ar>", "+");
    s.replace("<bt>", "=");
    s.replace("<as>", "S");
    s.replace("<ka>", "K");
    s.replace("<kn>", "N");
    s.replace("<sk>", "K");
    s.replace("<ve>", "V");
    return s;
}

