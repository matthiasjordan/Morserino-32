/*
 * test.cpp
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#include <stdio.h>
#include <vector>
#include <string>

#include "TestSupport.h"

#include "TestSupportTest.h"

#include "WordBufferTest.h"
#include "TennisMachineTest.h"


int main()
{
    printf("Unit tests for Morselino\n\n");

    test_WordBuffer_addChar();
    test_WordBuffer_addWord();
    test_WordBuffer_getAndClear();
    test_WordBuffer_get();
    test_WordBuffer_equals();

    test_TennisMachine_1();

    printf("Failed tests: %lu\n", failedTests.size());
    return failedTests.size();
}
