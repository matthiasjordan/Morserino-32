#ifndef MORSEDISPLAY_H_
#define MORSEDISPLAY_H_

#include <Arduino.h>
#include "morsedefs.h"

#define NoOfLines 15

namespace MorseDisplay
{

    typedef struct display_config {
            boolean autoFlush;
    } Config;

    extern const int8_t maxPos;
    extern int8_t relPos;
    extern uint8_t bottomLine;

    void init();
    Config* getConfig();
    void displayStartUp();
    void displayDisplay();
    void clearDisplay();
    void sleep();
    void clear();
    void clearStatusLine();
    String cleanUpProSigns(String &input);
    void printOnStatusLine(boolean strong, uint8_t xpos, String string);
    void vprintOnStatusLine(boolean strong, uint8_t xpos, const char* format, ...);
    void printToScroll(FONT_ATTRIB style, String text);
    void clearScrollBuffer();
    void flushScroll();
    void printToScroll_internal(FONT_ATTRIB style, String text);
    void newLine();
    void refreshScrollArea(int pos);
    void refreshScrollLine(int bufferLine, int displayLine);
    uint8_t printOnScroll(uint8_t line, FONT_ATTRIB how, uint8_t xpos, String mystring);
    uint8_t vprintOnScroll(uint8_t line, FONT_ATTRIB how, uint8_t xpos, const char* format, ...);
    void printOnScrollFlash(uint8_t line, FONT_ATTRIB how, uint8_t xpos, String mystring);
    void clearLine(uint8_t line);
    void clearScroll();
    void drawVolumeCtrl(boolean inverse, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t volume);
    void displayScrollBar(boolean visible);
    void displayVolume();
    void displayBatteryStatus(int v);
    void displayEmptyBattery();
    void dispLoraLogo();
    void updateSMeter(int rssi);
    void drawInputStatus(boolean on);
    void displayTopLine();
    void displayCWspeed();
    void showVolumeBar(uint16_t mini, uint16_t maxi);

}

#endif /* MORSEDISPLAY_H_ */
