/*
 * test.cpp
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#include <stdio.h>
#include "WordBuffer.h"
#include <vector>
#include <string>

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

void test_WordBuffer_addChar()
{
    WordBuffer sut;
    sut.addChar(String("a"));
    sut.endWord();
    assertEquals("test_WordBuffer_addChar 1", "a", sut.get());

    sut.addChar("b");
    assertEquals("test_WordBuffer_addChar 2", "a b", sut.get());
}

void test_WordBuffer_addWord()
{
    WordBuffer sut;
    sut.addWord(String("abc"));
    assertEquals("test_WordBuffer_addWord 1: ", "abc", sut.get());

    sut.addWord("b");
    assertEquals("test_WordBuffer_addWord 2: ", "abc b", sut.get());

    sut.addWord("cd");
    assertEquals("test_WordBuffer_addWord 3: ", "abc b cd", sut.get());
}

void test_WordBuffer_getAndClear()
{
    WordBuffer sut;
    sut.addWord(String("abc"));
    assertEquals("test_WordBuffer_getAndClear 1: ", "abc", sut.get());

    String actual1 = sut.getAndClear();
    assertEquals("test_WordBuffer_getAndClear 2: ", "abc", actual1);

    String actual2 = sut.getAndClear();
    assertEquals("test_WordBuffer_getAndClear 3: ", "", actual2);
}

void test_WordBuffer_get()
{
    WordBuffer sut;
    sut.addWord(String("abc"));
    assertEquals("test_WordBuffer_get 1: ", "abc", sut.get());

    String actual1 = sut.get();
    assertEquals("test_WordBuffer_get 2: ", "abc", actual1);

    String actual2 = sut.get();
    assertEquals("test_WordBuffer_get 3: ", "abc", actual2);
}

void test_WordBuffer_equals()
{
    WordBuffer sut;
    sut.addWord(String("abc"));

    assertEquals("test_WordBuffer_equals 1: ", true, sut == "abc");

    String actual1 = sut.get();
    assertEquals("test_WordBuffer_equals 2: ", false, sut == "x");
}

int main()
{
    printf("Unit tests for Morselino\n\n");

    test_WordBuffer_addChar();
    test_WordBuffer_addWord();
    test_WordBuffer_getAndClear();
    test_WordBuffer_get();
    test_WordBuffer_equals();

    printf("Failed tests: %lu\n", failedTests.size());
    return 0;
}
