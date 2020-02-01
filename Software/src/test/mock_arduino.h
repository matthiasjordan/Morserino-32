/*
 * Arduino.h
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#ifndef MOCK_ARDUINO_H_
#define MOCK_ARDUINO_H_

#include <cstdio>
#include <string>

#define boolean bool


class String : public std::string {

public:
	String() : std::string() {};
	String(char *a) : std::string(a) {};
	String(const char *a) : std::string(a) {};
	String(std::string a) : std::string(a) {};
//	using std::string::string;
	void operator=(const char *a);
	void operator=(std::string a);
	String substring(unsigned int a);
	void operator+=(const char *b);
	void operator+=(String b);
	bool operator==(String a);
	unsigned int length();
};



class MockSerial {
public:
	void println(char *a);
	void println(String a);
	void println(std::__cxx11::basic_string<char>);
};

extern MockSerial Serial;


#endif /* MOCK_ARDUINO_H_ */
