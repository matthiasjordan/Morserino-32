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

std::vector<const char*> failedTests;

const char* toString(bool b)
{
    return b ? "true" : "false";
}

void assertEquals(const char* msg, std::string expected, std::string actual)
{
    if (expected != actual)
    {
        printf("FAILED: %s - expected: %s, actual: %s\n", msg, expected.c_str(), actual.c_str());
        failedTests.push_back(msg);
    }
}

void assertEquals(const char* msg, bool expected, bool actual)
{
    if (expected != actual)
    {
        printf("FAILED: %s - expected: %s, actual: %s\n", msg, toString(expected), toString(actual));
        failedTests.push_back(msg);
    }
}

void assertTrue(const char* msg, bool actual)
{
    if (!actual)
    {
        printf("FAILED: %s - expected: true, actual: false\n", msg);
        failedTests.push_back(msg);
    }
}

