#include <Arduino.h>
#include "SPIFFS.h"
#include "MorsePlayerFile.h"
#include "MorsePreferences.h"
#include "koch.h"


using namespace MorsePlayerFile;


namespace MorsePlayerFile::internal {
    String cleanUpText(String w);
    String utf8umlaut(String s);

}

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
    result = internal::cleanUpText(result);
    return result;                                    // at eof
}



void MorsePlayerFile::skipWords(uint32_t count) {             /// just skip count words in open file fn
  while (count > 0) {
    getWord();
    --count;
  }
}


String MorsePlayerFile::internal::cleanUpText(String w) {                        // all to lower case, and convert umlauts
  w.toLowerCase();
  w = utf8umlaut(w);

  return Koch::filterNonKoch(w);
}


String MorsePlayerFile::internal::utf8umlaut(String s) { /// replace umtf umlauts with digraphs, and interpret pro signs, written e.g. as [kn] or <kn>
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

