/*
 * WordBuffer.h
 *
 *  Created on: 30.01.2020
 *      Author: mj
 */

#ifndef WORDBUFFER_H_
#define WORDBUFFER_H_

#include "arduino.h"

class WordBuffer
{
    public:
        WordBuffer();
        WordBuffer(const char* initial);
        WordBuffer(String initial);
        ~WordBuffer() = default;
        void addWord(String word);
        void addChar(String c);
        void endWord();
        String getAndClear();
        String get();
        boolean matches(String pattern);
        String getMatch();
        String getFullPatternMatch();
        boolean operator==(String other);

    private:
        String buffer;
        String match;
        String fullPatternMatch;
        boolean wordEnd = false;
        void handleWordEnd();
        String trim(String in);
        boolean textOK(String s);
};

#endif /* WORDBUFFER_H_ */
