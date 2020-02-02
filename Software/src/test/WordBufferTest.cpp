#include <stdio.h>
#include <string>

#include "TestSupport.h"

#include "WordBuffer.h"


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


