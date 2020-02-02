#ifndef TESTSUPPORT_H
#define TESTSUPPORT_H

#include <stdio.h>
#include <vector>
#include <string>

extern std::vector<const char*> failedTests;

const char* toString(bool b);
void assertEquals(const char* msg, std::string expected, std::string actual);
void assertEquals(const char* msg, bool expected, bool actual);
void assertTrue(const char* msg, bool actual);


#endif
