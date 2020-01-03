#include "abbrev.h"
#include "koch.h"

using namespace Abbrev;

String Abbrev::getRandomAbbrev(int maxLength)
{
    return abbreviations[random(ABBREV_POINTER[maxLength], ABBREV_NUMBER_OF_ELEMENTS)];
}

