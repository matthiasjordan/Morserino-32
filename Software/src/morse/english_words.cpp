#include <Arduino.h>

#include "morsedefs.h"
#include "koch.h"
#include "english_words.h"

using namespace EnglishWords;

String EnglishWords::getRandomWord(int maxLength)
{
    return words[random(WORDS_POINTER[maxLength], WORDS_NUMBER_OF_ELEMENTS)];
}
