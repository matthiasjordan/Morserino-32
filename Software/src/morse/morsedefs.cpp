#include <Arduino.h>
#include "morsedefs.h"

void voidFunction()
{
    // Well, nothing.
    MORSELOGLN("voidFunction");
}

void voidFunction(String s) {
    MORSELOGLN("voidFunction String");
}


unsigned long uLongFunctionMinus1()
{
    MORSELOGLN("uLongFunctionMinus1");
    return -1;
}


boolean booleanFunctionFalse()
{
    MORSELOGLN("booleanFunctionFalse");
    return false;
}
