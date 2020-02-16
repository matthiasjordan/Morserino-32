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


const char* String::c_str() {
    return delegate.c_str();
}


void String::operator+=(const char *b)
{
//	printf("+=1 '%s'", b);
    delegate += (std::string(b));
}

void String::operator+=(String b)
{
//	printf("+=2 '%s'", b.c_str());
    delegate += (std::string(b.c_str()));
}

String String::operator+(String b)
{
//  printf("+=2 '%s'", b.c_str());
    std::string c = delegate;
    return String(c + std::string(b.c_str()));
}

String operator+(char *a, String &b) {
    return String(std::string(a) + b.c_str());
}

String operator+(const char *a, String b) {
    return String(std::string(a) + b.c_str());
}

String String::operator+(const char *b)
{
//  printf("+=2 '%s'", b.c_str());
    std::string c = delegate;
    return String(c + std::string(b));
}

void String::operator=(const char *b)
{
//	printf("=1 '%s'", b);
    delegate = std::string(b);
}

void String::operator=(std::string b)
{
//  printf("=2 '%s'\n", b.c_str());
    delegate = (b);
}

void String::operator=(String b)
{
//    printf("=3 '%s'\n", b.c_str());
    delegate = std::string(b.c_str());
}

bool String::operator!=(String b)
{
    return 0 != strcmp(c_str(), b.c_str());
}

bool String::operator!=(const char *b)
{
    return 0 != strcmp(c_str(), b);
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
    return delegate.length();
}

String String::substring(unsigned int a)
{
    return String(delegate.substr(a));
}

String String::substring(unsigned int a, unsigned int b)
{
    return String(delegate.substr(a, b));
}

int String::lastIndexOf(char c)
{
    return String::indexOf(c, false);
}

int String::indexOf(char c) {
    return String::indexOf(c, true);
}

int String::indexOf(char c, boolean first)
{
    int res = -1;
    for (unsigned int i = 0; (i < length()); i++)
    {
        if (c_str()[i] == c)
        {
            res = i;
            if (first)
            {
                break;
            }
        }
    }
    return res;
}

void String::replace(const char *what, String &byWhat) {
    String whatS = String(what);
    String::replace(whatS, byWhat);
}

void String::replace(const char *what, const char *byWhat) {
    String ws = String(what);
    String bws = String(byWhat);
    String::replace(ws, bws);
}

void String::replace(String &what, String &byWhat) {
//    printf("replace in %s the part %s by %s\n", c_str(), what.c_str(), byWhat.c_str());
    std::string::size_type pos = delegate.find(what.c_str());
    if (pos != std::string::npos) {
        String prefix = substring(0, pos);
        unsigned int whatLen = what.length();
        String suffix = substring((unsigned int) pos + whatLen);
        String replacement = prefix + byWhat + suffix;
        delegate = replacement.c_str();
    }
}


//void String::replace(String &what, String &byWhat) {
//    size_type pos = find(what);
//    if (pos != npos) {
////        (pos, what.length(), byWhat);
//        std::string prefix = substring(0, pos);
//        std::string suffix = substring(pos + what.length());
//        std::string replacement = prefix + byWhat + suffix;
//        clear();
////        copy(replacement, 1);
////        ((std::string *)this) += replacement;
////        this << (std::string) replacement;
//        assign(replacement);
//    }
//}

void MockSerial::println(const char *a)
{
}

void MockSerial::println(String a)
{
}

void MockSerial::println(std::__cxx11::basic_string<char>)
{
}

