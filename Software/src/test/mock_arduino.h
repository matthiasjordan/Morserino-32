/*
 * Arduino.h
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#ifndef MOCK_ARDUINO_H_
#define MOCK_ARDUINO_H_

#include <stdlib.h>
#include <cstdio>
#include <string>

#define boolean bool

#define T2 2
#define T5 5


void delay(unsigned long int);

class String : public std::string {

public:
	String() : std::string() {};
	String(char *a) : std::string(a) {};
	String(const char *a) : std::string(a) {};
	String(std::string a) : std::string(a) {};
	String(unsigned long a) : std::string(std::to_string(a)) {};

	void operator=(const char *a);
	void operator=(std::string a);
    String substring(unsigned int a);
    String substring(unsigned int a, unsigned int b);
	void operator+=(const char *b);
	void operator+=(String b);
    bool operator==(String a);
    bool operator==(const char *a);
	unsigned int length();
	void trim();
	int lastIndexOf(char c);
	int indexOf(char c);

private:
	int indexOf(char c, boolean first);
};



class MockSerial {
public:
	void println(const char *a);
	void println(String a);
	void println(std::__cxx11::basic_string<char>);
};

extern MockSerial Serial;


#endif /* MOCK_ARDUINO_H_ */
