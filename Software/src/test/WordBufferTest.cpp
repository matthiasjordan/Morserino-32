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
    assertEquals("test_WordBuffer_addWord 1", "abc", sut.get());

    sut.addWord("b");
    assertEquals("test_WordBuffer_addWord 2", "abc b", sut.get());

    sut.addWord("cd");
    assertEquals("test_WordBuffer_addWord 3", "abc b cd", sut.get());
}

void test_WordBuffer_getAndClear()
{
    WordBuffer sut;
    sut.addWord(String("abc"));
    assertEquals("test_WordBuffer_getAndClear 1", "abc", sut.get());

    String actual1 = sut.getAndClear();
    assertEquals("test_WordBuffer_getAndClear 2", "abc", actual1);

    String actual2 = sut.getAndClear();
    assertEquals("test_WordBuffer_getAndClear 3", "", actual2);
}

void test_WordBuffer_get()
{
    WordBuffer sut;
    sut.addWord(String("abc"));
    assertEquals("test_WordBuffer_get 1", "abc", sut.get());

    String actual1 = sut.get();
    assertEquals("test_WordBuffer_get 2", "abc", actual1);

    String actual2 = sut.get();
    assertEquals("test_WordBuffer_get 3", "abc", actual2);
}

void test_WordBuffer_equals()
{
    WordBuffer sut;
    sut.addWord(String("abc"));

    assertEquals("test_WordBuffer_equals 1", true, sut == "abc");

    assertEquals("test_WordBuffer_equals 2", false, sut == "x");
}

void test_WordBuffer_matches_1()
{
    WordBuffer sut;
    sut.addWord(String("cq"));
    sut.addWord(String("cq"));
    sut.addWord(String("de"));
    sut.addWord(String("w1aw"));
    sut.addWord(String("w1aw"));

    assertEquals("matches 1 1", true, sut.matches("cq cq de # #"));
    assertEquals("matches 1 2", "w1aw", sut.getMatch());
}

void test_WordBuffer_matches_2()
{
    WordBuffer sut;
    sut.addWord(String("cq"));
    sut.addWord(String("cq"));
    sut.addWord(String("de"));
    sut.addWord(String("w1aw"));
    sut.addWord(String("w1aa"));

    assertEquals("matches 2", false, sut.matches("cq cq de # #"));
}

void test_WordBuffer_matches_2a()
{
    WordBuffer sut;
    sut.addWord(String("cq"));
    sut.addWord(String("cq"));
    sut.addWord(String("di"));
    sut.addWord(String("w1aw"));
    sut.addWord(String("w1aw"));

    assertEquals("matches 2a", false, sut.matches("cq cq de # #"));
}

void test_WordBuffer_matches_3()
{
    WordBuffer sut;
    sut.addWord(String("cq"));

    assertEquals("matches 3 1", false, sut.matches("cq de #"));

    sut.addWord(String("de"));
    assertEquals("matches 3 2", false, sut.matches("cq de #"));

    sut.addWord(String("dx"));
    assertEquals("matches 3 3", true, sut.matches("cq de #"));
    assertEquals("matches 3 4", "dx", sut.getMatch());
}

void test_WordBuffer_matches_4()
{
    assertEquals("matches 4 1", true, WordBuffer("cq de dx").matches("cq de #"));
    assertEquals("matches 4 2", false, WordBuffer("cq dx dx").matches("cq de #"));
}

void test_WordBuffer_fullPatternMatch_1()
{
    WordBuffer sut;

    sut.getAndClear();
    sut.addWord("cq de dx");
    sut.matches("cq de #");
    assertEquals("matches 5 1", "cq de dx", sut.getFullPatternMatch());

    sut.getAndClear();
    sut.addWord("cq");
    sut.addWord("de");
    sut.addWord("m");
    sut.matches("cq de #");
    assertEquals("matches 5 1a", "cq de m", sut.getFullPatternMatch());

    sut.getAndClear();
    sut.addWord("cq cq cq de dx");
    sut.matches("cq de #");
    assertEquals("matches 5 2", "cq de dx", sut.getFullPatternMatch());

    sut.getAndClear();
    sut.addWord("err err dx de me k");
    sut.matches("dx de # k");
    assertEquals("matches 5 3", "dx de me k", sut.getFullPatternMatch());
}

void test_WordBuffer()
{
    printf("Testing WordBuffer\n");
    test_WordBuffer_addChar();
    test_WordBuffer_addWord();
    test_WordBuffer_getAndClear();
    test_WordBuffer_get();
    test_WordBuffer_equals();
    test_WordBuffer_matches_1();
    test_WordBuffer_matches_2();
    test_WordBuffer_matches_2a();
    test_WordBuffer_matches_3();
    test_WordBuffer_matches_4();
    test_WordBuffer_fullPatternMatch_1();
}

