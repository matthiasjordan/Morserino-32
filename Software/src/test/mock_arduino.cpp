/*
 * Arduino.h
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#include <cstring>
#include <cstdio>
#include <string>

#include "mock_arduino.h"

void delay(unsigned long int)
{

}

MockSerial Serial;

void String::operator+=(const char *b)
{
//	printf("+=1 '%s'", b);
    std::string::operator+=(std::string(b));
}

void String::operator+=(String b)
{
//	printf("+=2 '%s'", b.c_str());
    std::string::operator+=(std::string(b.c_str()));
}

void String::operator=(const char *b)
{
//	printf("=1 '%s'", b);
    std::string::operator=(std::string(b));
}

void String::operator=(std::string b)
{
//	printf("=2 '%s'\n", b.c_str());
    std::string::operator=(b);
}

bool String::operator==(String b)
{
//  printf("'%s' == '%s'\n", c_str(), b.c_str());
    return 0 == strcmp(c_str(), b.c_str());
}

bool String::operator==(const char *b)
{
//  printf("'%s' == '%s'", c_str(), b.c_str());
    return 0 == strcmp(c_str(), b);
}

unsigned int String::length()
{
    return std::string::length();
}

String String::substring(unsigned int a)
{
    return String(substr(a));
}

String String::substring(unsigned int a, unsigned int b)
{
    return String(substr(a, b));
}

int String::lastIndexOf(char c)
{
    int res = -1;
    for (unsigned int i = 0; (i < length()); i++)
    {
        if (c_str()[i] == c)
        {
            res = i;
        }
    }
    return res;
}

void MockSerial::println(const char *a)
{
}

void MockSerial::println(String a)
{
}

void MockSerial::println(std::__cxx11::basic_string<char>)
{
}

