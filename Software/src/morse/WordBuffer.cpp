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

WordBuffer::WordBuffer(const char* initial)
{
    buffer += initial;
    endWord();
}

WordBuffer::WordBuffer(String initial)
{
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

String* split(String s)
{
    int i = s.lastIndexOf(' ');
    if (i == -1)
    {
        return new String[2]{"", s};
    }
    String token = s.substring(i + 1);
    String rest = s.substring(0, i);
    String *r = new String[2]{rest, token};
    return r;
}

/*
 * Patterns can use the wildcard # for "any word". If all # tokens in the buffer are equal,
 * the token is returned. Else the empty string is returned.
 *
 * The comparison is executed from the right (i.e. most recently keyed tokens first).
 *
 * Example:
 * Buffer contains "cq de w1aw", pattern is "cq de #", returns "w1aw".
 * Buffer contains "cq de w1aw w1aw", pattern is cq de # #", returns "w1aw".
 * Buffer contains "cq de w1aw w1aa", pattern is cq de # #", returns "".
 */
boolean WordBuffer::matches(String pattern)
{
    String tmppat = pattern;
    String tmpbuf = buffer;
    String wildcardContent = "";
    do
    {
        String *p = split(tmppat);
        String *b = split(tmpbuf);
        if (p[1] == "#")
        {
            String wcc = b[1];
            if (wildcardContent == "")
            {
                wildcardContent = wcc;
            }
            else if (wcc != wildcardContent)
            {
                // Repeat wildcard mismatch
                return false;
            }
        }
        else if (p[1] != b[1])
        {
            // word mismatch
            return false;
        }
        tmppat = *p;
        tmpbuf = *b;
    }
    while ((tmppat != "") && (tmpbuf != ""));

    if ((tmppat != "") && (tmpbuf == ""))
    {
        wildcardContent = "";
        return false;
    }
    else
    {
        match = wildcardContent;
        return true;
    }
}

String WordBuffer::getMatch()
{
    return match;
}

boolean WordBuffer::operator==(String other)
{
    return buffer == other;
}
