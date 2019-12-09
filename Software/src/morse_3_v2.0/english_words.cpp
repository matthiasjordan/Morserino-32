/*
 * english_words.cpp
 *
 *  Created on: 07.12.2019
 *      Author: mj
 */

#include <Arduino.h>

#include "morsedefs.h"
#include "koch.h"
#include "english_words.h"

using namespace EnglishWords;

String kochWords[EnglishWords::WORDS_NUMBER_OF_ELEMENTS];
int numberOfWords;

void EnglishWords::createKochWords(uint8_t maxl, uint8_t koch)
{                  // this function creates an array of words that are compliant to Koch filter and max word length
    numberOfWords = 0;
    for (int i = WORDS_POINTER[maxl]; i < WORDS_NUMBER_OF_ELEMENTS; ++i)
    {     // do this for all words with max length maxl
        if (Koch::wordIsKoch(words[i]) <= koch)
            kochWords[numberOfWords++] = words[i];
    }
}

