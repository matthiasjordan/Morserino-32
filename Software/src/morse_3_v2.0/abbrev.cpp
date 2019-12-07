/*
 * abbrev.cpp
 *
 *  Created on: 07.12.2019
 *      Author: mj
 */

#include "abbrev.h"
#include "koch.h"


using namespace Abbrev;




String kochAbbr[Abbrev::ABBREV_NUMBER_OF_ELEMENTS];
int numberOfAbbr;




void Abbrev::createKochAbbr(uint8_t maxl, uint8_t koch) {                  // this function creates an array of words that are compliant to Koch filter and max word length
  numberOfAbbr = 0;
  for (int i = ABBREV_POINTER[maxl]; i< ABBREV_NUMBER_OF_ELEMENTS; ++i) {     // do this for all words with max length maxl
      if (Koch::wordIsKoch(abbreviations[i]) <= koch)
          kochAbbr[numberOfAbbr++] = abbreviations[i];
  }
}




