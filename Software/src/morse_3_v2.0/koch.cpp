#include "koch.h"
#include "MorsePreferences.h"
#include "abbrev.h"
#include "english_words.h"

using namespace Koch;

const String morserinoKochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=K+SNAV@:";
const String lcwoKochChars = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x-K+ASNV@:";


void Koch::setup() {
    if (MorsePreferences::prefs.lcwoKochSeq) kochChars = lcwoKochChars;
    else kochChars = morserinoKochChars;


    //// populate the array for abbreviations and words according to length and Koch filter
    EnglishWords::createKochWords(MorsePreferences::prefs.wordLength, MorsePreferences::prefs.kochFilter) ;  //
    Abbrev::createKochAbbr(MorsePreferences::prefs.abbrevLength, MorsePreferences::prefs.kochFilter);
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

