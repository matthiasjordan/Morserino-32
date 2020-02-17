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

class String {

public:
	String() { delegate = std::string();};
    String(char *a) { delegate = std::string(a); };
	String(const char *a) { delegate = std::string(a);};
	String(std::string a) { delegate = std::string(a);};
	String(unsigned long a) { delegate = std::string(std::to_string(a));};

	const char* c_str();
	void operator=(const char *a);
	void operator=(std::string a);
	void operator=(String b);
    String substring(unsigned int a);
    String substring(unsigned int a, unsigned int b);
    String operator+(String b);
    String operator+(const char *b);
	void operator+=(const char *b);
	void operator+=(String b);
	bool operator!=(String b);
    bool operator!=(const char *a);
	bool operator==(String a);
    bool operator==(const char *a);
	unsigned int length();
	void trim();
	int lastIndexOf(char c);
	int indexOf(char c);
    void replace(const char *what, String &byWhat);
    void replace(const char *what, const char *byWhat);
	void replace(String &what, String &byWhat);
	void getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index = 0);

private:
	std::string delegate;
	int indexOf(char c, boolean first);
};

String operator+(char *a, String b);

String operator+(const char *a, String b);



class MockSerial {
public:
	void println(const char *a);
	void println(String a);
	void println(std::__cxx11::basic_string<char>);
};

extern MockSerial Serial;


#endif /* MOCK_ARDUINO_H_ */
