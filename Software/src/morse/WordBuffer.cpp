/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2020  Matthias Jordan, DL4MAT
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/
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

            if (!textOK(wcc)) {
                return false;
            }

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
        fullPatternMatch = trim(buffer.substring(tmpbuf.length()));
        return true;
    }
}

boolean WordBuffer::textOK(String s) {
    int i = s.indexOf('*');
    Serial.println("textOK " + s + " -> " + String(i));
    return (i == -1);
}

String WordBuffer::trim(String in) {
    String out = in;
    while ((out.length() != 0) && (out.substring(0, 1) == " ")) {
        out = out.substring(1);
    }
    unsigned int l = out.length();
    while (((l = out.length()) != 0) && (out.substring(l-1, l) == " ")) {
        out = out.substring(0, l-1);
    }
    return out;
}

String WordBuffer::getMatch()
{
    return match;
}

String WordBuffer::getFullPatternMatch()
{
    return fullPatternMatch;
}

boolean WordBuffer::operator==(String other)
{
    return buffer == other;
}
