#include <Arduino.h>
#include "SPIFFS.h"
#include "MorsePlayerFile.h"
#include "MorsePreferences.h"


using namespace MorsePlayerFile;


File file;

String MorsePlayerFile::getWord() {
  String result = "";
  byte c;

  while (file.available()) {
      c=file.read();
      //Serial.println((int) c);
      if (!isSpace(c))
        result += (char) c;
      else if (result.length() > 0)    {               // end of word
        ++MorsePreferences::prefs.fileWordPointer;
        //Serial.println("word: " + result);
        return result;
      }
    }
    file.close(); file = SPIFFS.open("/player.txt");
    MorsePreferences::prefs.fileWordPointer = 0;
    while (!file.available())
      ;
    return result;                                    // at eof
}



void MorsePlayerFile::skipWords(uint32_t count) {             /// just skip count words in open file fn
  while (count > 0) {
    getWord();
    --count;
  }
}

