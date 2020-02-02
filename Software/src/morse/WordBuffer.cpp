/*
 * WordBuffer.cpp
 *
 *  Created on: 30.01.2020
 *      Author: mj
 */

#include "WordBuffer.h"

WordBuffer::WordBuffer()
{
    getAndClear();
}

WordBuffer::WordBuffer(const char* initial) {
    buffer += initial;
    endWord();
}

void WordBuffer::handleWordEnd()
{
    if (wordEnd)
    {
        buffer += " ";
        wordEnd = false;
    }
}

void WordBuffer::addWord(String word)
{
    handleWordEnd();
    buffer += word;
    endWord();
}

void WordBuffer::addChar(String c)
{
    handleWordEnd();
    buffer += c;
}

void WordBuffer::endWord()
{
    wordEnd = true;
}

String WordBuffer::getAndClear()
{
    String tmp = buffer;
    buffer = "";
    wordEnd = false;
    return tmp;
}

String WordBuffer::get()
{
    return buffer;
}

String WordBuffer::matches(String pattern) {

    return "";
}

boolean WordBuffer::operator==(String other)
{
    return buffer == other;
}

boolean WordBuffer::operator>=(String other)
{
    unsigned int ol = other.length();
    unsigned int tl = buffer.length();
    if (ol > tl)
    {
        return false;
    }
    String substring = buffer.substring(tl - ol);
    Serial.println("Wordbuffer " + buffer + ">=" + other + " -> '" + substring + "'");
    return substring == other;
}
