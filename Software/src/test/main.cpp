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

    test_WordBuffer();
    test_TennisMachine();

    printf("Failed tests: %lu\n", failedTests.size());
    return failedTests.size();
}
