#ifndef TESTSUPPORT_H
#define TESTSUPPORT_H

#include <stdio.h>
#include <vector>
#include <string>
#include "mock_arduino.h"

extern std::vector<const char*> failedTests;

const char* toString(bool b);
void assertEquals(const char* msg, const char *expected, String actual);
void assertEquals(const char* msg, String expected, String actual);
void assertEquals(const char* msg, std::string expected, std::string actual);
void assertEquals(const char* msg, const char *expected, const char* actual);
void assertEquals(const char* msg, int expected, int actual);
void assertTrue(const char* msg, bool actual);
void assertFalse(const char* msg, bool actual);

#endif
