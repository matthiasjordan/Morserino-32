/*
 * test.cpp
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#include <stdio.h>
#include <vector>
#include <string>
#include <cstring>

#include "TestSupport.h"

std::vector<const char*> failedTests;

const char* toString(bool b)
{
    return b ? "true" : "false";
}

void assertEquals(const char* msg, const char *expected, String actual)
{
    assertEquals(msg, expected, actual.c_str());
}

void assertEquals(const char* msg, String expected, String actual)
{
    assertEquals(msg, expected.c_str(), actual.c_str());
}

void assertEquals(const char* msg, std::string expected, std::string actual)
{
    assertEquals(msg, expected.c_str(), actual.c_str());
}

void assertEquals(const char* msg, const char* expected, const char* actual)
{
    if (strcmp(expected, actual) != 0)
    {
        printf("FAILED: %s - expected: %s, actual: %s\n", msg, expected, actual);
        failedTests.push_back(msg);
    }
}

void assertEquals(const char* msg, int expected, int actual)
{
    if (expected != actual)
    {
        printf("FAILED: %s - expected: %s, actual: %s\n", msg, String(expected).c_str(), String(actual).c_str());
        failedTests.push_back(msg);
    }
}

void assertTrue(const char* msg, bool actual)
{
    if (!actual)
    {
        printf("FAILED: %s - expected: true, actual: %s\n", msg, toString(actual));
        failedTests.push_back(msg);
    }
}

void assertFalse(const char* msg, bool actual)
{
    if (actual)
    {
        printf("FAILED: %s - expected: false, actual: %s\n", msg, toString(actual));
        failedTests.push_back(msg);
    }
}

