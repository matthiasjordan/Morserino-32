#ifndef MORSEDISPLAY_H_
#define MORSEDISPLAY_H_

#include "SSD1306.h"       // alias for `#include "SSD1306Wire.h"
#include "morsedefs.h"

namespace MorseDisplay
{

    void init();
    void displayStartUp();
    void clearStatusLine();
    String cleanUpProSigns(String &input);
    void printOnStatusLine(boolean strong, uint8_t xpos, String string);
    void printToScroll(FONT_ATTRIB style, String text);
    void clearScrollBuffer();
    void flushScroll();
    void printToScroll_internal(FONT_ATTRIB style, String text);
    void newLine();
    void refreshScrollArea(int pos);
    void refreshScrollLine(int bufferLine, int displayLine);
    uint8_t printOnScroll(uint8_t line, FONT_ATTRIB how, uint8_t xpos, String mystring);
    void clearLine(uint8_t line);
    void clearScroll();
    void drawVolumeCtrl(boolean inverse, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t volume);
    void displayScrollBar(boolean visible);
    void displayVolume();
    void displayBatteryStatus(int v);
    void displayEmptyBattery();

}

#endif /* MORSEDISPLAY_H_ */
