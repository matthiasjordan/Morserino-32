#include <Arduino.h>
#include "morsedefs.h"

void voidFunction()
{
    // Well, nothing.
    Serial.println("voidFunction");
}

void voidFunction(String s) {
    Serial.println("voidFunction String");
}


unsigned long uLongFunctionMinus1()
{
    Serial.println("uLongFunctionMinus1");
    return -1;
}


boolean booleanFunctionFalse()
{
    Serial.println("booleanFunctionFalse");
    return false;
}
