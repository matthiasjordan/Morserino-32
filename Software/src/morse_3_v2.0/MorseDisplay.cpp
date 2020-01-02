
#include <Arduino.h>
#include <Wire.h>          // Only needed for Arduino 1.6.5 and earlier

#include "SSD1306.h"       // alias for `#include "SSD1306Wire.h"

#include "MorseDisplay.h"
#include "morsedefs.h"
#include "wklfonts.h"
#include "MorsePreferences.h"
#include "MorseSystem.h"
#include "MorseMachine.h"
#include "MorseGenerator.h"
#include "MorseKeyer.h"

using namespace MorseDisplay;

////////////////////////////// New scrolling display

/// circular buffer: 14 chars by NoOfLines lines (bottom 3 are visible)
#define NoOfCharsPerLine 14
#define SCROLL_TOP 15
#define LINE_HEIGHT 16

const int8_t maxPos = NoOfLines - 3;
int8_t relPos = MorseDisplay::maxPos;
uint8_t bottomLine = 0;


char textBuffer[NoOfLines][2 * NoOfCharsPerLine + 1]; /// we need extra room for style markers (FONT_ATTRIB stored as characters to toggle on/off the style within a line)
                                                      /// and 0 terminator

uint8_t linePointer = 0;    /// defines the current bottom line


#define lora_width 6        /// a simple logo that shows when we operate with loRa, stored in XBM format
#define lora_height 11
static unsigned char lora_bits[] =
{0x0f, 0x18, 0x33, 0x24, 0x29, 0x2b, 0x29, 0x24, 0x33, 0x18, 0x0f};

// define OLED display and its address - the Heltec ESP32 LoRA uses its display on 0x3c
SSD1306 display(0x3c, OLED_SDA, OLED_SCL, OLED_RST);

void MorseDisplay::init()
{
    display.init();
    display.flipScreenVertically();  // rotate 180°  when USB is to the left of the module
    clearDisplay();

    adcAttachPin(batteryPin);
    analogSetPinAttenuation(batteryPin, ADC_11db);
}

char numBuffer[16];                // for number to string conversion with sprintf()

// display startup screen and check battery status
void MorseDisplay::displayStartUp()
{
    String stat = "Morserino-32m ";
    clearDisplay();

    stat += String(MorsePreferences::prefs.loraQRG / 10000);
    /*
     switch (p_loraBand) {
     case 0 : stat += "433M ";
     break;
     case 1 : stat += "868M ";
     break;
     case 2 : stat += "920M ";
     break;
     } */
    printOnStatusLine(true, 0, stat);

    printOnScroll(0, REGULAR, 0, "Ver. ");

    sprintf(numBuffer, "%2i", VERSION_MAJOR);
    printOnScroll(0, REGULAR, 5, numBuffer);

    printOnScroll(0, REGULAR, 7, ".");
    sprintf(numBuffer, "%1i", VERSION_MINOR);
    printOnScroll(0, REGULAR, 8, numBuffer);

    if (BETA)
        printOnScroll(0, REGULAR, 10, "beta");

    printOnScroll(1, REGULAR, 0, "© 2018 f.");

    uint16_t volt = MorseSystem::batteryVoltage();
    //Serial.println(volt);

    if (volt < 3150)
    {
        clearDisplay();
        MorseDisplay::displayEmptyBattery();                            // warn about empty battery and go to deep sleep (again)
    }
    else
    {
        MorseDisplay::displayBatteryStatus(volt);
    }
    delay(3000);
}


void MorseDisplay::clear() {
    display.clear();
}

void MorseDisplay::displayDisplay() {
    display.display();
}


void MorseDisplay::clearDisplay() {
    MorseDisplay::clear();
    MorseDisplay::displayDisplay();
}



void MorseDisplay::sleep() {
    display.sleep();
}


///// functions to use the graphics display more or less like a character display
///// basically we use two different fixed-width fonts, Dialoginput 12 (for the status line) and Dialoginput 15 for almost everything else
///// MAYBE we might use an even bigger font (18?) for setting parameters?

//// display functions: (neben display.clear())
//// clearStatusLine();
//// clearScroll() (= Top, Middle & Bottom line);
//// printOnStatusLine(xpos, char*str)
//// printOnScroll(line, xpos, string); line (0,1 or 2), xpos 0 = leftmost postition
//// clearTopLine(), clearMiddleLine(), clearBottomLine()
//// printToScroll(char letter); push a character onto the scrolling display
//// starting coordinates of the 4 lines:
//// status line: 0,0
//// top line:    0,15
//// middle line: 0,31
//// bottom line: 0.47

void MorseDisplay::clearStatusLine()
{              // the status line is at the top, and inverted!
    display.setColor(WHITE);
    display.fillRect(0, 0, 128, 15);
    display.setColor(BLACK);

    display.display();
}

String MorseDisplay::cleanUpProSigns(String &input)
{
    /// clean up clearText   -   S <as>,  - A <ka> - N <kn> - K <sk> - H ch;
    input.replace("S", "<as>");
    input.replace("A", "<ka>");
    input.replace("N", "<kn>");
    input.replace("K", "<sk>");
    input.replace("V", "<ve>");
    input.replace("H", "ch");
    input.replace("E", "<err>");
    input.replace("U", "¬");
    //Serial.println(input);
    return input;
}

void MorseDisplay::printOnStatusLine(boolean strong, uint8_t xpos, String string)
{    // place a string onto the status line; chars are 7px wide = 18 chars per line
    if (strong)
        display.setFont(DialogInput_bold_12);
    else
        display.setFont(DialogInput_plain_12);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    uint8_t w = display.getStringWidth(string);
    display.setColor(WHITE);
    display.fillRect(xpos * 7, 0, w, 15);
    display.setColor(BLACK);
    display.drawString(xpos * 7, 0, string);
    display.setColor(WHITE);
    display.display();
    MorseSystem::resetTOT();
}

void MorseDisplay::vprintOnStatusLine(boolean strong, uint8_t xpos, const char* format, ...) {
    va_list arglist;
    va_start( arglist, format );
    vsprintf(numBuffer, format, arglist);
    va_end( arglist );
    MorseDisplay::printOnStatusLine(strong, xpos, numBuffer);
}

String printToScroll_buffer = "";
FONT_ATTRIB printToScroll_lastStyle = REGULAR;


void MorseDisplay::printToScroll(FONT_ATTRIB style, String text)
{
    // Serial.printf("printToScroll(%s)\n", text.c_str());

    String SP = " ";
    String SL = "/";

    boolean styleChanged = (style != printToScroll_lastStyle);
    boolean lengthExceeded = printToScroll_buffer.length() + text.length() > 10;
    if (styleChanged || lengthExceeded)
    {
        //Serial.print("style/length " + styleChanged + SL + lengthExceeded + SP);
        flushScroll();
    }

    printToScroll_buffer += text;
    printToScroll_lastStyle = style;

    boolean linebreak = text.endsWith("\n");
    boolean printToScroll_autoflush = !MorseGenerator::effectiveAutoStop;
    //Serial.println("AUTO: " + String(printToScroll_autoflush));
    //((Serial.println("morseState: " + String(morseState));
    if (MorseMachine::isMode(MorseMachine::morseGenerator))
        printToScroll_autoflush = true;
    if (printToScroll_autoflush || linebreak)
    {
        //Serial.print("auto/line " + printToScroll_autoflush + SP + linebreak + SP);
        flushScroll();
    }
}

void MorseDisplay::clearScrollBuffer()
{
    //Serial.println("clear 1 " + printToScroll_buffer);
    printToScroll_buffer = "";
    printToScroll_lastStyle = REGULAR;
    //Serial.println("clear 2 " + printToScroll_buffer);
}

void MorseDisplay::flushScroll()
{
    uint8_t len = printToScroll_buffer.length();
    if (len != 0)
    {
        //Serial.print("flush: ");
        for (int i = 0; (i < len); i++)
        {
            String t = printToScroll_buffer.substring(i, i + 1);
            //Serial.printf(" flushing %d<%s> ", i, t.c_str());
            printToScroll_internal(printToScroll_lastStyle, t);
        }
        clearScrollBuffer();
    }
    else
    {
        //Serial.println("(flush)");
    }
}

void MorseDisplay::printToScroll_internal(FONT_ATTRIB style, String text)
{
/// store text in textBuffer, if it fits the screen line; otherwise scroll up, clear bottom buffer, store in new buffer, print on new lien
    //Serial.println("printToScroll_internal: " + text);

    static uint8_t pos = 0;
    static uint8_t screenPos = 0;
//uint8_t printedChars;
//Serial.println("printToScroll_internal: " + text);
    static FONT_ATTRIB lastStyle = REGULAR;
    uint8_t l = text.length();
    if (l == 0)
    {                               // an empty string signals we should clear the buffer
        //Serial.println("printToScroll_internal clearing buffer");
        for (int i = 0; i < NoOfLines; ++i)
        {
            textBuffer[i][0] = (char) 0;                    /// empty this line
        }
        refreshScrollArea((NoOfLines + bottomLine - 2) % NoOfLines);
        pos = screenPos = 0;                                // reset the position pointers
        return;
    }

    int linebreak = text.equals("\n");
    int textTooLong = (screenPos + l > NoOfCharsPerLine);

    if (linebreak || textTooLong)
    {                 // we need to scroll up and start a new line
        newLine();
        pos = 0;
        screenPos = 0;
        lastStyle = REGULAR;
        if (linebreak)
        {
            text = "";
            l = text.length();
        }
    }

    /// store text in buffer
    if (style == REGULAR)
    {
        memcpy(&textBuffer[bottomLine][pos], text.c_str(), l);  // copy the string of characters
        pos += l;
        textBuffer[bottomLine][pos] = (char) 0;                 // add 0 character
    }
    else
    {
        if (style == lastStyle)
        {                                // not regular, but we have no change in style!
            pos -= 1;                                               // go one pos back to overwrite style marker
            memcpy(&textBuffer[bottomLine][pos], text.c_str(), l);  // copy the string of characters
            pos += l;
            textBuffer[bottomLine][pos++] = (char) style;           // add the style marker
            textBuffer[bottomLine][pos] = (char) 0;                 // add 0 character
        }
        else
        {
            textBuffer[bottomLine][pos++] = (char) style;           // add the style marker at the beginning
            memcpy(&textBuffer[bottomLine][pos], text.c_str(), l);  // copy the string of characters
            pos += l;
            textBuffer[bottomLine][pos++] = (char) style;           // add the style marker at the end
            textBuffer[bottomLine][pos] = (char) 0;                 // add 0 character
            lastStyle = style;                                      // remember new style flag
        }
    }

    if (relPos == maxPos)
    {                                     // we show the bottom lines on the screen, therefore we add the new stuff  immediately
        /// and send string to screen, avoiding refresh of complete line
        printOnScroll(2, style, screenPos, text);               // these characters are 9 pixels wide,
    }
    display.setFont(DialogInput_plain_15);
    ;
    screenPos += (display.getStringWidth(text) / 9);
}

void MorseDisplay::newLine()
{
    linePointer = (linePointer + 1) % NoOfLines;
    if (relPos && relPos != maxPos)
        --relPos;
    bottomLine = linePointer;
    textBuffer[bottomLine][0] = (char) 0;               /// and empty the bottom line
    if (!relPos)                                      /// screen ptr already on top, we need to move the whole screen one line
        refreshScrollArea((bottomLine + 1) % NoOfLines);
    else if (relPos == maxPos)
        refreshScrollArea((NoOfLines + bottomLine - 2) % NoOfLines);
    if (MorseMachine::isEncoderMode(MorseMachine::scrollMode))
        displayScrollBar(true);

}

void MorseDisplay::refreshScrollArea(int pos)
{                                  /// refresh all three lines from buffer in scroll area; pos is topmost buffer line
    refreshScrollLine(pos, 0);           /// refresh all three lines
    refreshScrollLine((pos + 1) % NoOfLines, 1);
    //unsigned int t = pos % 15;
    refreshScrollLine((pos + 2) % NoOfLines, 2);
    display.display();
}

void MorseDisplay::refreshScrollLine(int bufferLine, int displayLine)
{   /// print a line to the screen
    String temp;
    temp.reserve(16);
    temp = "";
    char c;
    boolean irFlag = false;
    FONT_ATTRIB style = REGULAR;
    int pos = 0;
    uint8_t charsPrinted;

    display.setColor(BLACK);
    display.fillRect(0, SCROLL_TOP + displayLine * LINE_HEIGHT, 127, LINE_HEIGHT + 1);   // black out the line on screen
    for (int i = 0; (c = textBuffer[bufferLine][i]); ++i)
    {
//Serial.print(String(c,HEX) + " ");
        if (c < 4)
        {           /// a flag
            if (irFlag)         /// at the end of an emphasized string
            {
                charsPrinted = printOnScroll(displayLine, style, pos, temp) / 9;
                style = REGULAR;
                pos += charsPrinted;
                temp = "";
                irFlag = false;
            }
            else                /// at the beginning of an emphasized string
            {
                if (temp.length())
                {
                    charsPrinted = printOnScroll(displayLine, style, pos, temp) / 9;
                    style = REGULAR;
                    pos += charsPrinted;
                    temp = "";
                }
                style = (FONT_ATTRIB) c;
                irFlag = true;
            }
        }
        else
        {                /// normal character - add it to temp
            temp += c;
        }
    }
//Serial.println("temp: " + temp);
    if (temp.length())
        printOnScroll(displayLine, style, pos, temp);
}


uint8_t MorseDisplay::vprintOnScroll(uint8_t line, FONT_ATTRIB how, uint8_t xpos, const char* format, ...) {
    va_list arglist;
    va_start( arglist, format );
    vsprintf(numBuffer, format, arglist);
    va_end( arglist );
    return MorseDisplay::printOnScroll(2, REGULAR, 1, numBuffer);
}

uint8_t MorseDisplay::printOnScroll(uint8_t line, FONT_ATTRIB how, uint8_t xpos, String mystring)
{    // place a string onto the scroll area; line = 0, 1 or 2
    uint8_t w;

    if (how > BOLD)
        display.setColor(WHITE);
    else
        display.setColor(BLACK);

    if (how & BOLD)
        display.setFont(DialogInput_bold_15);
    else
        display.setFont(DialogInput_plain_15);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    // convert the array characters into a String object

    //if (xpos == 0)
    //        w = 127;
    //else
    w = display.getStringWidth(mystring);

    display.fillRect(xpos * 9, SCROLL_TOP + line * LINE_HEIGHT, w, LINE_HEIGHT + 1);

    if (how > BOLD)
        display.setColor(BLACK);
    else
        display.setColor(WHITE);

    display.drawString(xpos * 9, SCROLL_TOP + line * LINE_HEIGHT, mystring);
    display.display();
    MorseSystem::resetTOT();
    return w;         // we return the actual width of the output, in case of converted UTF8 characters
}


void MorseDisplay::printOnScrollFlash(uint8_t line, FONT_ATTRIB how, uint8_t xpos, String mystring) {
    MorseDisplay::printOnScroll(line, how, xpos, mystring);
    displayDisplay();
    delay(500);
    clear();
}

void MorseDisplay::clearLine(uint8_t line)
{                                              /// clear a line - display is done somewhere else!
    display.setColor(BLACK);
    display.fillRect(0, SCROLL_TOP + line * LINE_HEIGHT, 127, LINE_HEIGHT + 1);
    display.setColor(WHITE);
}

void MorseDisplay::clearScroll()
{
    printToScroll_internal(REGULAR, "");
    clearScrollBuffer();
    clearLine(0);
    clearLine(1);
    clearLine(2);
    //Serial.println("clearScroll()");
}

void MorseDisplay::drawVolumeCtrl(boolean inverse, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t volume)
{

    if (inverse)
        display.setColor(BLACK);
    else
        display.setColor(WHITE);

    display.fillRect(x, y, width, height);

    if (!inverse)
        display.setColor(BLACK);
    else
        display.setColor(WHITE);

    display.fillRect(x + 2, y + 4, (width - 4) * volume / 100, height - 8);
    display.drawHorizontalLine(x + 2, y + height / 2, width - 4);
    display.display();
    MorseSystem::resetTOT();
}

void MorseDisplay::displayScrollBar(boolean visible)
{
    const int l_bar = 3 * 49 / NoOfLines;

    if (visible)
    {
        display.setColor(WHITE);
        display.drawVerticalLine(127, 15, 49);
        display.setColor(BLACK);
        display.drawVerticalLine(127, 15 + (relPos * (49 - l_bar) / maxPos), l_bar);
    }
    else
    {
        display.setColor(BLACK);
        display.drawVerticalLine(127, 15, 49);
    }
    display.display();
    MorseSystem::resetTOT();
}

/// display volume as a progress bar: vol = 1-100
void MorseDisplay::displayVolume()
{
    MorseDisplay::drawVolumeCtrl(MorseMachine::isEncoderMode(MorseMachine::speedSettingMode) ? false : true, 93, 0, 28, 15, MorsePreferences::prefs.sidetoneVolume);
    display.display();
}

///// display battery status as icon, parameter v: Voltage in mV
void MorseDisplay::displayBatteryStatus(int v)
{    /// v in millivolts!

    int a, b, c;
    String s;
    double d;
    d = v / 50;
    c = round(d) * 50;
// Serial.println("v: " + String (v) + " c: " + String(c));
    a = c / 1000;
    b = (c - 1000 * a) / 100;
    s = "U: " + String(a) + "." + String(b);
    printOnScroll(2, REGULAR, 0, s + " V");
    int w = constrain(v, 3200, 4050);
    w = map(w, 3200, 4050, 0, 31);
    display.drawRect(75, SCROLL_TOP + 2 * LINE_HEIGHT + 3, 35, LINE_HEIGHT - 4);
    display.drawRect(110, SCROLL_TOP + 2 * LINE_HEIGHT + 5, 4, LINE_HEIGHT - 8);
    display.fillRect(77, SCROLL_TOP + 2 * LINE_HEIGHT + 5, w, LINE_HEIGHT - 8);
    display.display();
}

void MorseDisplay::displayEmptyBattery()
{                                /// display a warning and go to (return to) deep sleep
    display.drawRect(10, 11, 95, 50);
    display.drawRect(105, 26, 15, 20);
    printOnScroll(1, INVERSE_BOLD, 4, "EMPTY");
    delay(4000);
    MorseSystem::shutMeDown();
}

void MorseDisplay::dispLoraLogo() {     // display a small logo in the top right corner to indicate we operate with LoRa
  display.setColor(BLACK);
  display.drawXbm(121, 2, lora_width, lora_height, lora_bits);
  display.setColor(WHITE);
  display.display();
}


////// S Meter for Trx modus

void MorseDisplay::updateSMeter(int rssi) {

  static boolean wasZero = false;

  if (rssi == 0)
      if (wasZero)
          return;
       else {
           MorseDisplay::drawVolumeCtrl( false, 93, 0, 28, 15, 0);
          wasZero = true;
       }
   else {
       MorseDisplay::drawVolumeCtrl( false, 93, 0, 28, 15, constrain(map(rssi, -150, -20, 0, 100), 0, 100));
      wasZero = false;
   }
  MorseDisplay::displayDisplay();
}

void MorseDisplay::drawInputStatus( boolean on) {
  if (on)
    display.setColor(BLACK);
  else
      display.setColor(WHITE);
  display.fillRect(1, 1, 20, 13);
  display.display();
}



//////// Display the status line in CW Keyer Mode
//////// Layout of top line:
//////// Tch ul 15 WpM
//////// 0    5    0

void MorseDisplay::displayTopLine() {
    MorseDisplay::clearStatusLine();

  // printOnStatusLine(true, 0, (MorsePreferences::prefs.useExtPaddle ? "X " : "T "));          // we do not show which paddle is in use anymore
  if (MorseMachine::isMode(MorseMachine::morseGenerator))
      MorseDisplay::printOnStatusLine(true, 1,  MorsePreferences::prefs.wordDoubler ? "x2" : "  ");
  else {
    switch (MorsePreferences::prefs.keyermode) {
      case IAMBICA:   MorseDisplay::printOnStatusLine(false, 2,  "A "); break;          // Iambic A (no paddle eval during dah)
      case IAMBICB:   MorseDisplay::printOnStatusLine(false, 2,  "B "); break;          // orig Curtis B mode: paddle eval during element
      case ULTIMATIC: MorseDisplay::printOnStatusLine(false, 2,  "U "); break;          // Ultimatic Mode
      case NONSQUEEZE: MorseDisplay::printOnStatusLine(false, 2,  "N "); break;         // Non-squeeze mode
    }
  }

  displayCWspeed();                                     // update display of CW speed
  if ((MorseMachine::isMode(MorseMachine::loraTrx) ) || (MorseMachine::isMode(MorseMachine::morseGenerator)  && MorsePreferences::prefs.loraTrainerMode == true))
      dispLoraLogo();

  MorseDisplay::displayVolume();                                     // sidetone volume
  MorseDisplay::displayDisplay();
}


//////// Display the current CW speed
/////// pos 7-8, "Wpm" on 10-12
void MorseDisplay::displayCWspeed() {
  if ((MorseMachine::isMode(MorseMachine::morseGenerator) || MorseMachine::isMode(MorseMachine::echoTrainer) ))
      sprintf(numBuffer, "(%2i)", MorseKeyer::effWpm);
  else sprintf(numBuffer, "    ");

  MorseDisplay::printOnStatusLine(false, 3,  numBuffer);                                         // effective wpm

  sprintf(numBuffer, "%2i", MorsePreferences::prefs.wpm);
  MorseDisplay::printOnStatusLine(MorseMachine::isEncoderMode(MorseMachine::speedSettingMode) ? true : false, 7,  numBuffer);
  MorseDisplay::printOnStatusLine(false, 10,  "WpM");
  MorseDisplay::displayDisplay();
}


void MorseDisplay::showVolumeBar(uint16_t mini, uint16_t maxi) {
    int a, b, c;
    a = map(mini, 0, 4096, 0, 125);
    b = map(maxi, 0, 4000, 0, 125);
    c = b - a;
    MorseDisplay::clearLine(2);
    display.drawRect(5, SCROLL_TOP + 2 * LINE_HEIGHT +5, 102, LINE_HEIGHT-8);
    display.drawRect(30, SCROLL_TOP + 2 * LINE_HEIGHT +5, 52, LINE_HEIGHT-8);
    display.fillRect(a, SCROLL_TOP + 2 * LINE_HEIGHT + 7 , c, LINE_HEIGHT -11);
    display.display();
}
