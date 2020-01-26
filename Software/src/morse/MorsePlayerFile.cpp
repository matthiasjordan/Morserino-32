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
#include "MorseText.h"
#include "koch.h"

using namespace MorsePlayerFile;

namespace internal
{
    String cleanUpText(String w);
    void reopen();
}

const String playerFileName = "/player.txt";

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

    Serial.println("Initializing player file");

    //////////////////////// create file player.txt if it does not exist|
    const char * defaultFile =
            "This is just an initial dummy file for the player. Dies ist nur die anfänglich enthaltene Standarddatei für den Player.\n"
                    "Did you not upload your own file? Hast du keine eigene Datei hochgeladen?";

    if (!SPIFFS.exists(playerFileName))
    {                                    // file does not exist, therefor we create it from the text above
        File file = MorsePlayerFile::openForWriting();
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

File MorsePlayerFile::openForWriting()
{
    return SPIFFS.open(playerFileName, FILE_WRITE);
}

void MorsePlayerFile::openAndSkip()
{
    uint32_t wcount = 0;
    file = SPIFFS.open(playerFileName);                            // open file
    //skip MorsePreferences::prefs.fileWordPointer words, as they have been played before
    wcount = MorsePreferences::prefs.fileWordPointer;
    MorsePreferences::prefs.fileWordPointer = 0;
    MorsePlayerFile::skipWords(wcount);
}

String MorsePlayerFile::getWord()
{
    String result = "";
    byte c;
    boolean wordFinished = false;
    boolean reopened = false;

    while (!wordFinished && file.available())
    {
        c = file.read();
        if (c == 255)
        {
            if (!reopened)
            {
                // Some weird file - try to reopen;
                internal::reopen();
                reopened = true;
                continue;
            }
            else
            {
                break;
            }
        }

        if (!isSpace(c))
        {
            result += (char) c;
        }
        else if (result.length() > 0)
        {               // end of word
            ++MorsePreferences::prefs.fileWordPointer;
            wordFinished = true;
        }
    }

    if (!file.available())
    {
        internal::reopen();
    }
    Serial.println("file read " + result);
    result = internal::cleanUpText(result);
    Serial.println("file c/up " + result);

    return result;                                    // at eof
}

void internal::reopen()
{
    file.close();
    file = SPIFFS.open(playerFileName);
    MorsePreferences::prefs.fileWordPointer = 0;
    while (!file.available())
    {
        // not sure - wait?
    }
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
    w = MorseText::utf8umlaut(w);

    return Koch::filterNonKoch(w);
}

