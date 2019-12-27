
/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
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

 /*****************************************************************************************************************************
 *  code by others used in this sketch, apart from the ESP32 libraries distributed by Heltec
 *  (see: https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series)
 *
 *  ClickButton library -> https://code.google.com/p/clickbutton/ by Ragnar Aronsen
 * 
 * 
 *  For volume control of NF output: I used a similar principle as  Connor Nishijima, see 
 *                                   https://hackaday.io/project/11957-10-bit-component-less-volume-control-for-arduino
 *                                   but actually using two PWM outputs, connected with an AND gate
 *  Routines for morse decoder - to a certain degree based on code by Hjalmar Skovholm Hansen OZ1JHM
 *                                   - see http://skovholm.com/cwdecoder
 ****************************************************************************************************************************/

///// include of the various libraries and include files being used

#include <Wire.h>          // Only needed for Arduino 1.6.5 and earlier
#include "ClickButton.h"   // button control library
#include <SPI.h>           // library for SPI interface
#include <LoRa.h>          // library for LoRa transceiver
#include <WiFi.h>          // basic WiFi functionality
#include <WebServer.h>     // simple web sever
#include <ESPmDNS.h>       // DNS functionality
#include <WiFiClient.h>    //WiFi clinet library
#include <Update.h>        // update "over the air" (OTA) functionality
#include "FS.h"
#include "SPIFFS.h"

#include "morsedefs.h"
#include "MorseDisplay.h"
#include "wklfonts.h"      // monospaced fonts in size 12 (regular and bold) for smaller text and 15 for larger text (regular and bold), called :
                           // DialogInput_plain_12, DialogInput_bold_12 & DialogInput_plain_15, DialogInput_bold_15
                           // these fonts were created with this tool: http://oleddisplay.squix.ch/#/home
#include "abbrev.h"        // common CW abbreviations
#include "english_words.h" // common English words
#include "MorsePreferences.h"



/// we need this for some strange reason: the min definition breaks with WiFi
#define _min(a,b) ((a)<(b)?(a):(b))
#define _max(a,b) ((a)>(b)?(a):(b))




/////////////////////// parameters for LF tone generation and  HF (= vol ctrl) PWM
int toneFreq = 500 ;
int toneChannel = 2;      // this PWM channel is used for LF generation, duty cycle is 0 (silent) or 50% (tone)
int lineOutChannel = 3;   // this PWM channel is used for line-out LF generation, duty cycle is 0 (silent) or 50% (tone)
int volChannel = 8;       // this PWM channel is used for HF generation, duty cycle between 1% (almost silent) and 100% (loud)
int pwmResolution = 10;
unsigned int volFreq = 32000; // this is the HF frequency we are using

const int  dutyCycleFiftyPercent =  512;                                                                             ;
const int  dutyCycleTwentyPercent = 25;
const int  dutyCycleZero = 0;

const int notes[] = {0, 233, 262, 294, 311, 349, 392, 440, 466, 523, 587, 622, 698, 784, 880, 932};



// things for reading the encoder
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


volatile int8_t _oldState;

#define LATCHSTATE 3

volatile int8_t encoderPos = 0;
volatile uint64_t IRTime = 0;   // interrupt time
const int encoderWaitTime = 100 ;         // how long to wait for next reading from encoder in microseconds
volatile uint8_t stateRegister = 0;

// positions: [3] 1 0 2 [3] 1 0 2 [3]
// [3] is the positions where my rotary switch detends
// ==> right, count up
// <== left,  count down




//////// variables and constants for the modus menu

enum GEN_TYPE { NA, RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER, KOCH_MIXED, KOCH_LEARN };              // the things we can generate in generator mode

enum navi {naviLevel, naviLeft, naviRight, naviUp, naviDown };

enum menuNo { _dummy, _keyer,
              _gen, _genRand, _genAbb, _genWords, _genCalls, _genMixed, _genPlayer,
              _echo, _echoRand, _echoAbb, _echoWords, _echoCalls, _echoMixed, _echoPlayer,
              _koch, _kochSel, _kochLearn, _kochGen, _kochGenRand, _kochGenAbb, _kochGenWords,
              _kochGenMixed, _kochEcho, _kochEchoRand, _kochEchoAbb, _kochEchoWords, _kochEchoMixed,
              _head, _headRand, _headAbb, _headWords, _headCalls, _headMixed, _headPlayer,
              _trx, _trxLora, _trxIcw, _decode, _wifi, _wifi_mac, _wifi_config, _wifi_check, _wifi_upload, _wifi_update, _goToSleep };


typedef struct MenuItem {
  String text;
  menuNo no;
  uint8_t nav[5];
  GEN_TYPE generatorMode;
  boolean remember;
} menuItem_t;


const menuItem_t menuItems [] = {
  {"",_dummy, { 0,0,0,0,0}, NA, true},
  {"CW Keyer",_keyer, {0,_goToSleep,_gen,_dummy,0}, NA, true},
  
  {"CW Generator",_gen, {0,_keyer,_echo,_dummy,_genRand}, NA, true},
  {"Random",_genRand, {1,_genPlayer,_genAbb,_gen,0}, RANDOMS, true},
  {"CW Abbrevs",_genAbb, {1,_genRand,_genWords,_gen,0}, ABBREVS, true},
  {"English Words",_genWords, {1,_genAbb,_genCalls,_gen,0}, WORDS, true},
  {"Call Signs",_genCalls, {1,_genWords,_genMixed,_gen,0}, CALLS, true},
  {"Mixed",_genMixed, {1,_genCalls,_genPlayer,_gen,0}, MIXED, true},
  {"File Player",_genPlayer, {1,_genMixed,_genRand,_gen,0}, PLAYER, true},

  {"Echo Trainer",_echo, {0,_gen,_koch,_dummy,_echoRand}, NA, true},
  {"Random",_echoRand, {1,_echoPlayer,_echoAbb,_echo,0}, RANDOMS, true},
  {"CW Abbrevs",_echoAbb, {1,_echoRand,_echoWords,_echo,0}, ABBREVS, true},
  {"English Words",_echoWords, {1,_echoAbb,_echoCalls,_echo,0}, WORDS, true},
  {"Call Signs",_echoCalls, {1,_echoWords,_echoMixed,_echo,0}, CALLS, true},
  {"Mixed",_echoMixed, {1,_echoCalls,_echoPlayer,_echo,0}, MIXED, true},
  {"File Player",_echoPlayer, {1,_echoMixed,_echoRand,_echo,0}, PLAYER, true},

  {"Koch Trainer",_koch,  {0,_echo,_head,_dummy,_kochSel}, NA, true},
  {"Select Lesson",_kochSel, {1,_kochEcho,_kochLearn,_koch,0}, NA, true},
  {"Learn New Chr",_kochLearn, {1,_kochSel,_kochGen,_koch,0}, NA, true},
  {"CW Generator",_kochGen, {1,_kochLearn,_kochEcho,_koch,_kochGenRand}, NA, true},
  {"Random",_kochGenRand, {2,_kochGenMixed,_kochGenAbb,_kochGen,0}, RANDOMS, true},
  {"CW Abbrevs",_kochGenAbb, {2,_kochGenRand,_kochGenWords,_kochGen,0}, ABBREVS, true},
  {"English Words",_kochGenWords, {2,_kochGenAbb,_kochGenMixed,_kochGen,0}, WORDS, true},
  {"Mixed",_kochGenMixed, {2,_kochGenWords,_kochGenRand,_kochGen,0}, MIXED, true},

  {"Echo Trainer",_kochEcho, {1,_kochGen,_kochSel,_koch,_kochEchoRand}, NA, true},
  {"Random",_kochEchoRand, {2,_kochEchoMixed,_kochEchoAbb,_kochEcho,0}, RANDOMS, true},
  {"CW Abbrevs",_kochEchoAbb, {2,_kochEchoRand,_kochEchoWords,_kochEcho,0}, ABBREVS, true},
  {"English Words",_kochEchoWords, {2,_kochEchoAbb,_kochEchoMixed,_kochEcho,0}, WORDS, true},
  {"Mixed",_kochEchoMixed, {2,_kochEchoWords,_kochEchoRand,_kochEcho,0}, MIXED, true},

  {"Head Copying",_head, {0,_koch,_trx,_dummy,_headRand}, NA, true},
  {"Random",_headRand, {1,_headPlayer,_headAbb,_head,0}, RANDOMS, true},
  {"CW Abbrevs",_headAbb, {1,_headRand,_headWords,_head,0}, ABBREVS, true},
  {"English Words",_headWords, {1,_headAbb,_headCalls,_head,0}, WORDS, true},
  {"Call Signs",_headCalls, {1,_headWords,_headMixed,_head,0}, CALLS, true},
  {"Mixed",_headMixed, {1,_headCalls,_headPlayer,_head,0}, MIXED, true},
  {"File Player",_headPlayer, {1,_headMixed,_headRand,_head,0}, PLAYER, true},

  {"Transceiver",_trx, {0,_head,_decode,_dummy,_trxLora}, NA, true},
  {"LoRa Trx",_trxLora, {1,_trxIcw,_trxIcw,_trx,0}, NA, true},
  {"iCW/Ext Trx",_trxIcw, {1,_trxLora,_trxLora,_trx,0}, NA, true},

  {"CW Decoder",_decode, {0,_trx,_wifi,_dummy,0}, NA, true},

  {"WiFi Functions",_wifi, {0,_decode,_goToSleep,_dummy,_wifi_mac}, NA, false},
  {"Disp MAC Addr",_wifi_mac, {1,_wifi_update,_wifi_config,_wifi,0}, NA, false},
  {"Config WiFi",_wifi_config, {1,_wifi_mac,_wifi_check,_wifi,0}, NA, false},
  {"Check WiFi",_wifi_check, {1,_wifi_config,_wifi_upload,_wifi,0}, NA, false},
  {"Upload File",_wifi_upload, {1,_wifi_check,_wifi_update,_wifi,0}, NA, false},
  {"Update Firmw",_wifi_update, {1,_wifi_upload,_wifi_mac,_wifi,0}, NA, false},

  {"Go To Sleep",_goToSleep, {0,_wifi,_keyer,_dummy,0}, NA, false}

};


boolean quickStart;                                     // should we execute menu item immediately?

// defines for keyer modi
//

#define    IAMBICA      1          // Curtis Mode A
#define    IAMBICB      2          // Curtis Mode B (with enhanced Curtis timing, set as parameter
#define    ULTIMATIC    3          // Ultimatic mode
#define    NONSQUEEZE   4          // Non-squeeze mode of dual-lever paddles - simulate a single-lever paddle


//// for adjusting preferences




prefPos *currentOptions = allOptions;

#define SizeOfArray(x)       (sizeof(x) / sizeof(x[0]))

int currentOptionSize;


///////////////////////////////////
//// Other Global VARIABLES ////////////
/////////////////////////////////

unsigned int interCharacterSpace, interWordSpace;   // need to be properly initialised!
unsigned int effWpm;                                // calculated effective speed in WpM
unsigned int lUntouched = 0;                        // sensor values (in untouched state) will be stored here
unsigned int rUntouched = 0;

volatile uint64_t TOTcounter;                       // holds millis for Time-Out Timer




//// not any longer defined in preferences:
  GEN_TYPE generatorMode = RANDOMS;          // trainer: what symbol (groups) are we going to send?            0 -  5

  



boolean kochActive = false;                 // set to true when in Koch trainer mode

//  keyerControl bit definitions

#define     DIT_L      0x01     // Dit latch
#define     DAH_L      0x02     // Dah latch
#define     DIT_LAST   0x04     // Dit was last processed element

//  Global Keyer Variables
//
unsigned char keyerControl = 0; // this holds the latches for the paddles and the DIT_LAST latch, see above


boolean DIT_FIRST = false; // first latched was dit?
unsigned int ditLength ;        // dit length in milliseconds - 100ms = 60bpm = 12 wpm
unsigned int dahLength ;        // dahs are 3 dits long
unsigned char keyerState;
unsigned long charCounter = 25; // we use this to count characters after changing speed - after n characters we decide to write the config into NVS
uint8_t sensor;                 // what we read from checking the touch sensors
boolean leftKey, rightKey;


///////////////////////////////////////////////////////////////////////////////
//
//  Iambic Keyer State Machine Defines
 
enum KSTYPE {IDLE_STATE, DIT, DAH, KEY_START, KEYED, INTER_ELEMENT };

// morse code decoder

struct linklist {
     const char* symb;
     const uint8_t dit;
     const uint8_t dah;
};


const struct linklist CWtree[67]  = {
  {"",1,2},            // 0
  {"e", 3,4},         // 1
  {"t",5,6},          // 2
//
  {"i", 7, 8},        // 3
  {"a", 9,10},        // 4
  {"n", 11,12},       // 5
  {"m", 13,14},       // 6
//
  {"s", 15,16},       // 7
  {"u", 17,18},       // 8
  {"r", 19,20},       // 9
  {"w", 21,22},       //10
  {"d", 23,24},       //11
  {"k", 25, 26},      //12
  {"g", 27, 28},      //13
  {"o", 29,30},       //14
//---------------------------------------------
  {"h", 31,32},       // 15
  {"v", 33, 34},      // 16
  {"f", 63, 63},      // 17
  {"ü", 35, 36},      // 18 german ue
  {"l", 37, 38},      // 19
  {"ä", 39, 63},      // 20 german ae
  {"p", 63, 40},      // 21
  {"j", 63, 41},      // 22
  {"b", 42, 43},      // 23
  {"x", 44, 63},      // 24
  {"c", 63, 45},      // 25
  {"y", 46, 63},      // 26
  {"z", 47, 48},      // 27
  {"q", 63, 63},      // 28
  {"ö", 49, 63},      // 29 german oe
  {"<ch>", 50, 51},        // 30 !!! german "ch" 
//---------------------------------------------
  {"5", 64, 63},      // 31
  {"4", 63, 63},      // 32
  {"<ve>", 63, 52},      // 33  or <sn>, sometimes "*"
  {"3", 63,63},       // 34
  {"*", 53,63,},      // 35 ¬ used for all unidentifiable characters ¬
  {"2", 63, 63},      // 36 
  {"<as>", 63,63},         // 37 !! <as>
  {"*", 54, 63},      // 38
  {"+", 63, 55},      // 39
  {"*", 56, 63},      // 40
  {"1", 57, 63},      // 41
  {"6", 63, 58},      // 42
  {"=", 63, 63},      // 43
  {"/", 63, 63},      // 44
  {"<ka>", 59, 60},        // 45 !! <ka>
  {"<kn>", 63, 63},        // 46 !! <kn>
  {"7", 63, 63},      // 47
  {"*", 63, 61},      // 48
  {"8", 62, 63},      // 49
  {"9", 63, 63},      // 50
  {"0", 63, 63},      // 51
//
  {"<sk>", 63, 63},        // 52 !! <sk>
  {"?", 63, 63},      // 53
  {"\"", 63, 63},      // 54
  {".", 63, 63},      // 55
  {"@", 63, 63},      // 56
  {"\'",63, 63},      // 57
  {"-", 63, 63},      // 58
  {";", 63, 63},      // 59
  {"!", 63, 63},      // 60
  {",", 63, 63},      // 61
  {":", 63, 63},      // 62
//
  {"*", 63, 63},       // 63 Default for all unidentified characters
  {"*", 65, 63},       // 64
  {"*", 66, 63},       // 65
  {"<err>", 66, 63}      // 66 !! Error - backspace
};


byte treeptr = 0;                          // pointer used to navigate within the linked list representing the dichotomic tree

unsigned long interWordTimer = 0;      // timer to detect interword spaces
unsigned long acsTimer = 0;            // timer to use for automatic character spacing (ACS)


const String CWchars = "abcdefghijklmnopqrstuvwxyz0123456789.,:-/=?@+SANKVäöüH";
//                      0....5....1....5....2....5....3....5....4....5....5...    
// we use substrings as char pool for trainer mode
// SANK will be replaced by <as>, <ka>, <kn> and <sk>, H = ch
// a = CWchars.substring(0,26); 9 = CWchars.substring(26,36); ? = CWchars.substring(36,45); <> = CWchars.substring(44,49);
// a9 = CWchars.substring(0,36); 9? = CWchars.substring(26,45); ?<> = CWchars.substring(36,50);
// a9? = CWchars.substring(0,45); 9?<> = CWchars.substring(26,50);
// a9?<> = CWchars;


///// variables for generating CW

   String CWword = "";
   String clearText = "";

   int repeats = 0;

   int rxDitLength = 0;                    // set new value for length of dits and dahs and other timings
   int rxDahLength = 0;
   int rxInterCharacterSpace = 0;
   int rxInterWordSpace = 0;

  //CWword.reserve(144);
  //clearText.reserve(50);
boolean active = false;                           // flag for trainer mode
boolean startFirst = true;                        // to indicate that we are starting a new sequence in the trainer modi
boolean firstTime = true;                         /// for word doubler mode

uint8_t wordCounter = 0;                          // for maxSequence
boolean stopFlag = false;                         // for maxSequence
boolean echoStop = false;                         // for maxSequence

unsigned long genTimer;                         // timer used for generating morse code in trainer mode

enum MORSE_TYPE {KEY_DOWN, KEY_UP };                    //   State Machine Defines
unsigned char generatorState;


//byte NoE = 0;             // Number of Elements
// byte nextElement[8];      // the list of elements; 0 = dit, 1 = dah

// for each character:
// byte length// byte morse encoding as binary value, beginning with most significant bit

byte poolPair[2];           // storage in RAM for one morse code character

const byte pool[][2]  = {
// letters
               {B01000000, 2},  // a    0     
               {B10000000, 4},  // b
               {B10100000, 4},  // c
               {B10000000, 3},  // d
               {B00000000, 1},  // e
               {B00100000, 4},  // f
               {B11000000, 3},  // g
               {B00000000, 4},  // h
               {B00000000, 2},  // i
               {B01110000, 4},  // j 
               {B10100000, 3},  // k
               {B01000000, 4},  // l
               {B11000000, 2},  // m  
               {B10000000, 2},  // n
               {B11100000, 3},  // o
               {B01100000, 4},  // p  
               {B11010000, 4},  // q
               {B01000000, 3},  // r
               {B00000000, 3},  // s
               {B10000000, 1},  // t
               {B00100000, 3},  // u
               {B00010000, 4},  // v
               {B01100000, 3},  // w
               {B10010000, 4},  // x
               {B10110000, 4},  // y
               {B11000000, 4},  // z  25
// numbers
               {B11111000, 5},  // 0  26    
               {B01111000, 5},  // 1
               {B00111000, 5},  // 2
               {B00011000, 5},  // 3
               {B00001000, 5},  // 4
               {B00000000, 5},  // 5
               {B10000000, 5},  // 6
               {B11000000, 5},  // 7
               {B11100000, 5},  // 8
               {B11110000, 5},  // 9  35
// interpunct   . , : - / = ? @ +    010101 110011 111000 100001 10010 10001 001100 011010 01010
               {B01010100, 6},  // .  36    
               {B11001100, 6},  // ,  37    
               {B11100000, 6},  // :  38    
               {B10000100, 6},  // -  39    
               {B10010000, 5},  // /  40    
               {B10001000, 5},  // =  41    
               {B00110000, 6},  // ?  42    
               {B01101000, 6},  // @  43    
               {B01010000, 5},  // +  44    (at the same time <ar> !) 
// Pro signs  <>  <as> <ka> <kn> <sk>
               {B01000000, 5},  // <as> 45 S
               {B10101000, 5},  // <ka> 46 A
               {B10110000, 5},  // <kn> 47 N
               {B00010100, 6},   // <sk> 48    K
               {B00010000, 5},  // <ve> 49 E
// German characters
               {B01010000, 4},  // ä    50   
               {B11100000, 4},  // ö    51
               {B00110000, 4},  // ü    52
               {B11110000, 4}   // ch   53  H
            };

////////////////////////////////////////////////////////////////////
///// Variables for Echo Trainer Mode
/////

String echoResponse = "";
enum echoStates { START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER };
echoStates echoTrainerState = START_ECHO;
String echoTrainerPrompt, echoTrainerWord;


/////////////////// Variables for Koch modes


String kochChars;

////// variables for CW decoder

boolean keyTx = false;             // when state is set by manual key or touch paddle, then true!
                                   // we use this to decide if Tx should be keyed or not

/////////////////// Variables for LoRa: Buffer management etc

char loraTxBuffer[32];

uint8_t loRaRxBuffer[256];
uint16_t byteBuFree = 256;
uint8_t nextBuWrite = 0;
uint8_t nextBuRead = 0;

uint8_t loRaSerial;                                     /// a 6 bit serial number, start with some random value, will be incremented witch each sent LoRa packet
                                                        /// the first two bits in teh byte will be the protocol id (CWLORAVERSION)


////////////////// Variables for file handling and WiFi functions

File file;

WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

File fsUploadFile;              // a File object to temporarily store the received file

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS
const char* host = "m32";               // hostname of the webserver


/// WiFi constants
const char* ssid = "morserino";
const char* password = "";


                          // HTML for the AP server - ued to get SSID and Password for local WiFi network - needed for file upload and OTA SW updates
const char* myForm = "<html><head><meta charset='utf-8'><title>Get AP Info</title><style> form {width: 420px;}div { margin-bottom: 20px;}"
                "label {display: inline-block; width: 240px; text-align: right; padding-right: 10px;} button, input {float: right;}</style>"
                "</head><body>"
                "<form action='/set' method='get'><div>"
                "<label for='ssid'>SSID of WiFi network?</label>"
                "<input name='ssid' id='ssid' ></div> <div>"
                "<label for='pw'>WiFi Password?</label> <input name='pw' id='pw'>"
                "</div><div><button>Submit</button></div></form></body></html>";



/*
 * HTML for Upload Login page
 */

const char* uploadLoginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>M32 File Upload - Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='m32' && form.pwd.value=='upload')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";


const char* updateLoginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>M32 Firmware Update Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='m32' && form.pwd.value=='update')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Begin'>"
    "</form>"
 "<div id='prg'>Progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";



////////////////////////////////////////////////////////////////////
// encoder subroutines
/// interrupt service routine - needs to be positioned BEFORE all other functions, including setup() and loop()
/// interrupt service routine

void IRAM_ATTR isr ()  {                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
//if (micros()  > (IRTime + 1000) ) {
portENTER_CRITICAL_ISR(&mux);

    int sig2 = digitalRead(PinDT); //MSB = most significant bit
    int sig1 = digitalRead(PinCLK); //LSB = least significant bit
    delayMicroseconds(125);                 // this seems to improve the responsiveness of the encoder and avoid any bouncing

    int8_t thisState = sig1 | (sig2 << 1);
    if (_oldState != thisState) {
      stateRegister = (stateRegister << 2) | thisState;
      if (thisState == LATCHSTATE) {
        
          if (stateRegister == 135 )
            encoderPos = 1;
          else if (stateRegister == 75)
            encoderPos = -1;
          else
            encoderPos = 0;
        }
    _oldState = thisState;
    } 
portEXIT_CRITICAL_ISR(&mux);
}



int IRAM_ATTR checkEncoder() {
  int t;
  
  portENTER_CRITICAL(&mux);

  t = encoderPos;
  if (encoderPos) {
    encoderPos = 0;
    portEXIT_CRITICAL(&mux);
    return t;
  } else {
    portEXIT_CRITICAL(&mux);
    return 0;
  }
}


////////////////////////   S E T U P /////////////////////////////

void setup()
{
 
  Serial.begin(115200);
  delay(200); // give me time to bring up serial monitor

  // enable Vext
  #if BOARDVERSION == 3
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext,LOW);
  #endif

  
  // set up the encoder - we need external pull-ups as the pins used do not have built-in pull-ups!
  pinMode(PinCLK,INPUT_PULLUP);
  pinMode(PinDT,INPUT_PULLUP);  
  pinMode(keyerPin, OUTPUT);        // we can use the built-in LED to show when the transmitter is being keyed
  pinMode(leftPin, INPUT);          // external keyer left paddle
  pinMode(rightPin, INPUT);         // external keyer right paddle

  /// enable deep sleep
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, (esp_sleep_ext1_wakeup_mode_t) 0); //1 = High, 0 = Low
  analogSetAttenuation(ADC_0db);


// we MUST reset the OLED RST pin for 50 ms! for the old board only, but as it does not hurt we do it anyway
//#if BOARDVERSION == 2
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);     // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST, HIGH);    // while OLED is running, must set GPIO16 in high
//# endif

 // init display
  
  MorseDisplay::init();

// set up PWMs for tone generation
  ledcSetup(toneChannel, toneFreq, pwmResolution);
  ledcAttachPin(LF_Pin, toneChannel);
  
  ledcSetup(lineOutChannel, toneFreq, pwmResolution);
  ledcAttachPin(lineOutPin, lineOutChannel);                                      ////// change this for real version - no line out currntly
  
  ledcSetup(volChannel, volFreq, pwmResolution);
  ledcAttachPin(HF_Pin, volChannel);
  
  ledcWrite(toneChannel,0);
  ledcWrite(lineOutChannel,0);

  //call ISR when any high/low changed seen
  //on any of the enoder pins
  attachInterrupt (digitalPinToInterrupt(PinDT), isr, CHANGE);   
  attachInterrupt (digitalPinToInterrupt(PinCLK), isr, CHANGE);
 
  encoderPos = 0;           /// this is the encoder position

/// set up for encoder button
  pinMode(modeButtonPin, INPUT);
  pinMode(volButtonPin, INPUT_PULLUP);               // external pullup for all GPIOS > 32 with ESP32-LORA
                                                     // wake up also works without external pullup! Interesting!
  
  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  modeButton.debounceTime   = 11;   // Debounce timer in ms
  modeButton.multiclickTime = 220;  // Time limit for multi clicks
  modeButton.longClickTime  = 350; // time until "held-down clicks" register

  volButton.debounceTime   = 11;   // Debounce timer in ms
  volButton.multiclickTime = 220;  // Time limit for multi clicks
  volButton.longClickTime  = 350; // time until "held-down clicks" register



  // to calibrate sensors, we record the values in untouched state
  initSensors();
  
  // read preferences from non-volatile storage
  // if version cannot be read, we have a new ESP32 and need to write the preferences first
  readPreferences("morserino");

  if (p_lcwoKochSeq) kochChars = lcwoKochChars;
  else kochChars = morserinoKochChars;

   
  //// populate the array for abbreviations and words according to length and Koch filter
  createKochWords(p_wordLength, p_kochFilter) ;  // 
  createKochAbbr(p_abbrevLength, p_kochFilter);


/// check if BLACK knob has been pressed on startup - if yes, we have to perform LoRa Setup
  delay(50);
  if (SPIFFS.begin() && digitalRead(modeButtonPin) == LOW)   {        // BLACK was pressed at start-up - checking for SPIFF so that programming 1st time w/o pull-up shows menu
     display.clear();
     display.display();
     printOnStatusLine(true, 0,  "Release BLACK");
      while (digitalRead(modeButtonPin) == LOW)      // wait until released
      ;
    loraSystemSetup();
  }

  /// set up quickstart - this should only be done once at startup - after successful quickstart we disable it to allow normal menu operation
  quickStart = p_quickStart;


////////////  Setup for LoRa
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(p_loraQRG,PABOOST)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setFrequency(p_loraQRG);                       /// default = 434.150 MHz - Region 1 ISM Band, can be changed by system setup
  LoRa.setSpreadingFactor(7);                         /// default
  LoRa.setSignalBandwidth(250E3);                     /// 250 kHz
  LoRa.noCrc();                                       /// we use error correction
  LoRa.setSyncWord(p_loraSyncW);                      /// the default would be 0x34
  
  // register the receive callback
  LoRa.onReceive(onReceive);
  /// initialise the serial number
  loRaSerial = random(64);

  
  ///////////////////////// mount (or create) SPIFFS file system
    #define FORMAT_SPIFFS_IF_FAILED true

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){     ///// if SPIFFS cannot be mounted, it does not exist. So create  (format) it, and mount it
        //Serial.println("SPIFFS Mount Failed");
        return;
    }
  //////////////////////// create file player.txt if it does not exist|
  const char * defaultFile = "This is just an initial dummy file for the player. Dies ist nur die anfänglich enthaltene Standarddatei für den Player.\n"
                             "Did you not upload your own file? Hast du keine eigene Datei hochgeladen?";
                             
    if (!SPIFFS.exists("/player.txt")) {                                    // file does not exist, therefor we create it from the text above
        file = SPIFFS.open("/player.txt", FILE_WRITE);
        if(!file){
            Serial.println("- failed to open file for writing");
            return;
        }
        if(file.print(defaultFile)){
            ;
        } else {
            Serial.println("- write failed");
        }
        file.close();
    }    
    displayStartUp();

    ///delay(2500);  //// just to be able to see the startup screen for a while - is contained in displayStartUp()

  ////

  menu_();
} /////////// END setup()



//////// System Setup / LoRa Setup ///// Called when BALCK knob is pressed @ startup

void loraSystemSetup() {
    displayKeyerPreferencesMenu(posLoraBand);
    adjustKeyerPreference(posLoraBand);
    displayKeyerPreferencesMenu(posLoraQRG);
    adjustKeyerPreference(posLoraQRG);
    /// now store chosen values in Preferences
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putUChar("loraBand", p_loraBand);
    pref.putUInt("loraQRG", p_loraQRG);
    pref.end();
}





enum AutoStopModes {off, stop1, stop2}  autoStop = off;

uint8_t effectiveTrainerDisplay = p_trainerDisplay;

void setupHeadCopying() {
  effectiveAutoStop = true;
  effectiveTrainerDisplay = DISPLAY_BY_CHAR;
}


///////////////////////// THE MAIN LOOP - do this OFTEN! /////////////////////////////////

void loop() {
// static uint64_t loopC = 0;
   int t;

   boolean activeOld = active;
   checkPaddles();
   switch (morseState) {
      case morseKeyer:    if (doPaddleIambic(leftKey, rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }
                          break;
      case loraTrx:      if (doPaddleIambic(leftKey, rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }
                          generateCW();
                          break;
      case morseTrx:      if (doPaddleIambic(leftKey, rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }  
                          doDecode();
                          if (speedChanged) {
                            speedChanged = false;
                            displayCWspeed();
                          }
                          break;    
      case morseGenerator:  if ((autoStop == stop1) || leftKey  || rightKey)   {                                    // touching a paddle starts and stops the generation of code
                          // for debouncing:
                          while (checkPaddles() )
                              ;                                                           // wait until paddles are released

                          if (effectiveAutoStop) {
                            active = (autoStop == off);
                            switch (autoStop) {
                              case off : {
                                  break;
                                  //
                                }
                              case stop1: {
                                  autoStop = stop2;
                                  break;
                                }
                              case stop2: {
                                  printToScroll(REGULAR, "\n");
                                  autoStop = off;
                                  break;
                                }
                            }
                          }
                          else {
                            active = !active;
                            autoStop = off;
                          }

                          //delay(100);
                          } /// end squeeze
                          
                          ///// check stopFlag triggered by maxSequence
                          if (stopFlag) {
                            active = stopFlag = false;
                          }
                          if (activeOld != active) {
                            if (!active) {
                               keyOut(false, true, 0, 0);
                               printOnStatusLine(true, 0, "Continue w/ Paddle");
                            }
                          else {
                               //cleanStartSettings();        
                               generatorState = KEY_UP; 
                               genTimer = millis()-1;           // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc...          
                            }
                          }
                          if (active)
                            generateCW();
                          break;
      case echoTrainer:                             ///// check stopFlag triggered by maxSequence
                          if (stopFlag) {
                            active = stopFlag = false;
                            keyOut(false, true, 0, 0);
                            printOnStatusLine(true, 0, "Continue w/ Paddle");
                          }
                          if (!active && (leftKey  || rightKey))   {                       // touching a paddle starts  the generation of code
                              // for debouncing:
                              while (checkPaddles() )
                                  ;                                                           // wait until paddles are released
                              active = !active;
             
                              cleanStartSettings();
                          } /// end touch to start
                          if (active)
                          switch (echoTrainerState) {
                            case  START_ECHO:   
                            case  SEND_WORD:
                            case  REPEAT_WORD:  echoResponse = ""; generateCW();
                                                break;
                            case  EVAL_ANSWER:  echoTrainerEval();
                                                break;
                            case  COMPLETE_ANSWER:                    
                            case  GET_ANSWER:   if (doPaddleIambic(leftKey, rightKey)) 
                                                    return;                             // we are busy keying and so need a very tight loop !
                                                break;
                            }                              
                            break;
      case morseDecoder: doDecode();
                         if (speedChanged) {
                            speedChanged = false;
                            displayCWspeed();
                          }
      default:            break;
            
                        
  } // end switch and code depending on state of metaMorserino

/// if we have time check for button presses

    modeButton.Update();
    volButton.Update();
    
    switch (volButton.clicks) {
      case 1:   if (encoderState == scrollMode) {
                    if (morseState != morseDecoder)
                        encoderState = speedSettingMode;
                    else
                        encoderState = volumeSettingMode;
                    relPos = maxPos;
                    refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                    displayScrollBar(false);
                } else if (encoderState == volumeSettingMode && morseState != morseDecoder) {          //  single click toggles encoder between speed and volume
                  encoderState = speedSettingMode;
                  pref.begin("morserino", false);                     // open the namespace as read/write
                  if (pref.getUChar("sidetoneVolume") != p_sidetoneVolume)
                      pref.putUChar("sidetoneVolume", p_sidetoneVolume);  // store the last volume, if it has changed
                  pref.end();
                  displayCWspeed();
                  displayVolume();
                }
                else {
                  encoderState = volumeSettingMode;
                  displayCWspeed();
                  displayVolume();
                }
                break;
      case -1:  if (encoderState == scrollMode) {
                    encoderState = (morseState == morseDecoder ? volumeSettingMode : speedSettingMode);
                    relPos = maxPos;
                    refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                    displayScrollBar(false);
                }       
                else {
                    encoderState = scrollMode;
                    displayScrollBar(true);
                }
                break;
    }
   
    switch (modeButton.clicks) {                                // actions based on enocder button
       case -1:   menu_();                                       // long click exits current mode and goes to top menu
                  return;
       case 1:    if (morseState == morseGenerator || morseState == echoTrainer) {//  start/stop in trainer modi, in others does nothing currently
                  active = !active;
                  if (!active) {
                        //digitalWrite(keyerPin, LOW);           // turn the LED off, unkey transmitter, or whatever
                        //pwmNoTone(); 
                        keyOut(false, true, 0, 0);
                        printOnStatusLine(true, 0, "Continue w/ Paddle");
                  }
                  else {
                    cleanStartSettings();
                  }
                        
              }
              break;
       case 2:  setupPreferences(p_menuPtr);                               // double click shows the preferences menu (true would select a specific option only)
                display.clear();                                  // restore display
                displayTopLine();
                if (morseState == morseGenerator || morseState == echoTrainer) 
                    stopFlag = true;                                  // we stop what we had been doing
                else
                    stopFlag = false;
                //startFirst = true;
                //firstTime = true;
     default: break;
    }
    
/// and we have time to check the encoder
     if ((t = checkEncoder())) {
        //Serial.println("t: " + String(t));
        pwmClick(p_sidetoneVolume);         /// click
        switch (encoderState) {
          case speedSettingMode:  
                                  changeSpeed(t);
                                  break;
          case volumeSettingMode: 
                                  p_sidetoneVolume += (t*10)+11;
                                  p_sidetoneVolume = constrain(p_sidetoneVolume, 11, 111) -11;
                                  //Serial.println(p_sidetoneVolume);
                                  displayVolume();
                                  break;
          case scrollMode:
                                  if (t == 1 && relPos < maxPos ) {        // up = scroll towards bottom
                                    ++relPos;
                                    refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                                  }
                                  else if (t == -1 && relPos > 0) {
                                    --relPos;
                                    refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                                  }
                                  //encoderPos = 0;
                                  //portEXIT_CRITICAL(&mux);
                                  displayScrollBar(true);
                                  break;
          }
    } // encoder 
    checkShutDown(false);         // check for time out
    
}     /////////////////////// end of loop() /////////


void cleanStartSettings() {
    clearText = "";
    CWword = "";
    echoTrainerState = START_ECHO;
    generatorState = KEY_UP; 
    keyerState = IDLE_STATE;
    interWordTimer = 4294967000;                 // almost the biggest possible unsigned long number :-) - do not output a space at the beginning
    genTimer = millis()-1;                       // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc... 
    wordCounter = 0;                             // reset word counter for maxSequence
    startFirst = true;
    displayTopLine();
}


////// The MENU


void menu_() {
   uint8_t newMenuPtr = p_menuPtr;
   uint8_t disp = 0;
   int t, command;
   
     //// initialize a few things now
     //Serial.println("THE MENU");
    ///updateTimings(); // now done after reading preferences
    LoRa.idle();
    //keyerState = IDLE_STATE;
    active = false;
    //startFirst = true;
    cleanStartSettings();
    /*
    clearText = "";
    CWword = "";
    echoTrainerState = START_ECHO;
    generatorState = KEY_UP; 
    keyerState = IDLE_STATE;
    interWordTimer = 4294967000;                 // almost the biggest possible unsigned long number :-) - do not output a space at the beginning
    genTimer = millis()-1;                       // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc... 
    */
    clearScroll();                  // clear the buffer
    clearScrollBuffer();

    keyOut(false, true, 0, 0);
    keyOut(false, false, 0, 0);
    encoderState = speedSettingMode;             // always start with this encoderstate (decoder will change it anyway)
    currentOptions = allOptions;                 // this is the array of options when we double click the BLACK button: while in menu, you can change all of them
    currentOptionSize = SizeOfArray(allOptions);
    pref.begin("morserino", false);              // open the namespace as read/write
    if ((p_fileWordPointer != pref.getUInt("fileWordPtr")))   // update word pointer if necessary (if we ran player before)
       pref.putUInt("fileWordPtr", p_fileWordPointer);
    pref.end(); 
    file.close();                               // just in case it is still open....
    display.clear();
    
    while (true) {                          // we wait for a click (= selection)
        if (disp != newMenuPtr) {
          disp = newMenuPtr;
          menuDisplay(disp);
        }
        if (quickStart) {
            quickStart = false;
            command = 1;
            delay(500);
            printOnScroll(2, REGULAR, 1, "QUICK START");
            display.display();
            delay(500);
            display.clear();
        }
        else {
            modeButton.Update();
            command = modeButton.clicks;
        }

        switch (command) {                                          // actions based on enocder button
          case 2: if (setupPreferences(newMenuPtr))                       // all available options when called from top menu
                    newMenuPtr = p_menuPtr;
                  menuDisplay(newMenuPtr);
                  break;
          case 1: // check if we have a submenu or if we execute the selection
                  //Serial.println("newMP: " + String(newMenuPtr) + " navi: " + String(menuNav[newMenuPtr][naviDown]));
                  if (menuItems[newMenuPtr].nav[naviDown] == 0) {
                      p_menuPtr = newMenuPtr;
                      disp = 0;
                      if (menuItems[p_menuPtr].remember) {            // remember last executed, unless it is a wifi function or shutdown
                          pref.begin("morserino", false);             // open the namespace as read/write
                          pref.putUChar("lastExecuted", p_menuPtr);   // store last executed command
                          pref.end();                                 // close namespace
                      }
                      if (menuExec())
                        return;
                  } else {
                      newMenuPtr = menuItems[newMenuPtr].nav[naviDown];
                  }
                  break;
          case -1:  // we need to go one level up, if possible
                  if (menuItems[newMenuPtr].nav[naviUp] != 0)
                      newMenuPtr = menuItems[newMenuPtr].nav[naviUp];
          default: break;
        }

       if ((t=checkEncoder())) {
          //pwmClick(p_sidetoneVolume);         /// click 
          newMenuPtr =  menuItems[newMenuPtr].nav[(t == -1) ? naviLeft : naviRight];
       }

       volButton.Update();
    
       switch (volButton.clicks) {
          case -1:  audioLevelAdjust();                         /// for adjusting line-in audio level (at the same time keying tx and sending oudio on line-out
                    display.clear();
                    menuDisplay(disp);
                    break;
          /* case  3:  wifiFunction();                                  /// configure wifi, upload file or firmware update
                    break;
          */
       }
       checkShutDown(false);                  // check for time out   
  } // end while - we leave as soon as the button has been pressed
} // end menu_() 


void menuDisplay(uint8_t ptr) {
  //Serial.println("Level: " + (String) menuItems[ptr].nav[naviLevel] + " " + menuItems[ptr].text);
  uint8_t oneUp = menuItems[ptr].nav[naviUp];
  uint8_t twoUp = menuItems[oneUp].nav[naviUp];
  uint8_t oneDown = menuItems[ptr].nav[naviDown];
    
  printOnStatusLine(true, 0,  "Select Modus:     ");

  clearLine(0); clearLine(1); clearLine(2);                       // delete previous content
  
  /// level 0: top line, possibly ".." on line 1
  /// level 1: higher level on 0, item on 1, possibly ".." on 2
  /// level 2: higher level on 1, highest level on 0, item on 2
  switch (menuItems[ptr].nav[naviLevel]) {
    case 2: printOnScroll(2, BOLD, 0, menuItems[ptr].text);
            printOnScroll(1, REGULAR, 0, menuItems[oneUp].text);
            printOnScroll(0, REGULAR, 0, menuItems[twoUp].text);
            break;
    case 1: if (oneDown)
                printOnScroll(2, REGULAR, 0, String(".."));
            printOnScroll(1, BOLD, 0, menuItems[ptr].text);
            printOnScroll(0, REGULAR, 0, menuItems[oneUp].text);
            break;
    case 0: 
            if (oneDown)
                printOnScroll(1, REGULAR, 0, String(".."));
            printOnScroll(0, BOLD, 0, menuItems[ptr].text);
            break;
  }
}

///////////// GEN_TYPE { RANDOMS, ABBREVS, WORDS, CALLS, MIXED, KOCH_MIXED, KOCH_LEARN };           // the things we can generate in generator mode




boolean menuExec() {                                          // return true if we should  leave menu after execution, true if we should stay in menu
  //Serial.println("Executing menu item " + String(p_menuPtr));

  uint32_t wcount = 0;

  effectiveAutoStop = false;
  effectiveTrainerDisplay = p_trainerDisplay;
  
  kochActive = false;
  switch (p_menuPtr) {
    case  _keyer:  /// keyer
                currentOptions = keyerOptions;                // list of available options in keyer mode
                currentOptionSize = SizeOfArray(keyerOptions);
                morseState = morseKeyer;
                display.clear();
                printOnScroll(1, REGULAR, 0, "Start CW Keyer" );
                delay(500);
                display.clear();
                displayTopLine();
                printToScroll(REGULAR,"");      // clear the buffer
                clearPaddleLatches();
                keyTx = true;
                return true;
                break;

     case _headRand:
     case _headAbb:
     case _headWords:
     case _headCalls:
     case _headMixed:      /// head copying
                setupHeadCopying();
                currentOptions = headOptions;
                currentOptionSize = SizeOfArray(headOptions);
                goto startTrainer;
     case _genRand:
     case _genAbb:
     case _genWords:
     case _genCalls:
     case _genMixed:      /// generator
                currentOptions = generatorOptions;                            // list of available options in generator mode
                currentOptionSize = SizeOfArray(generatorOptions);
                goto startTrainer;
     case _headPlayer:
                setupHeadCopying();
                currentOptions = headOptions;
                currentOptionSize = SizeOfArray(headOptions);
                goto startPlayer;
     case _genPlayer:  
                currentOptions = playerOptions;                               // list of available options in player mode
                currentOptionSize = SizeOfArray(playerOptions);
     startPlayer:
                file = SPIFFS.open("/player.txt");                            // open file
                //skip p_fileWordPointer words, as they have been played before
                wcount = p_fileWordPointer;
                p_fileWordPointer = 0;
                skipWords(wcount);
                
     startTrainer:
                generatorMode = menuItems[p_menuPtr].generatorMode;
                startFirst = true;
                firstTime = true;
                morseState = morseGenerator;
                display.clear();
                printOnScroll(0, REGULAR, 0, "Generator     ");
                printOnScroll(1, REGULAR, 0, "Start/Stop:   ");
                printOnScroll(2, REGULAR, 0, "Paddle | BLACK");
                delay(1250);
                display.clear();
                displayTopLine();
                clearScroll();      // clear the buffer
                keyTx = true;
                return true;
                break;
      case  _echoRand:
      case  _echoAbb:
      case  _echoWords:
      case  _echoCalls:
      case  _echoMixed:
                currentOptions = echoTrainerOptions;                        // list of available options in echo trainer mode
                currentOptionSize = SizeOfArray(echoTrainerOptions);
                generatorMode = menuItems[p_menuPtr].generatorMode;
                goto startEcho;
      case  _echoPlayer:    /// echo trainer
                generatorMode = menuItems[p_menuPtr].generatorMode;
                currentOptions = echoPlayerOptions;                         // list of available options in echo player mode
                currentOptionSize = SizeOfArray(echoPlayerOptions);
                file = SPIFFS.open("/player.txt");                            // open file
                //skip p_fileWordPointer words, as they have been played before
                wcount = p_fileWordPointer;
                p_fileWordPointer = 0;
                skipWords(wcount);
       startEcho:
                startFirst = true;
                morseState = echoTrainer;
                echoStop = false;
                display.clear();
                printOnScroll(0, REGULAR, 0, generatorMode == KOCH_LEARN ? "New Character:" : "Echo Trainer:");
                printOnScroll(1, REGULAR, 0, "Start:       ");
                printOnScroll(2, REGULAR, 0, "Press paddle ");
                delay(1250);
                display.clear();
                displayTopLine();
                printToScroll(REGULAR,"");      // clear the buffer
                keyTx = false;
                return true;
                break;
      case  _kochSel: // Koch Select 
                displayKeyerPreferencesMenu(posKochFilter);
                adjustKeyerPreference(posKochFilter);
                writePreferences("morserino");
                //createKochWords(p_wordLength, p_kochFilter) ;  // update the arrays!
                //createKochAbbr(p_abbrevLength, p_kochFilter);
                return false;
                break;
      case  _kochLearn:   // Koch Learn New .  /// just a new generatormode....
                generatorMode = KOCH_LEARN;
                currentOptions = kochEchoOptions;                          // list of available options in Koch echo mode
                currentOptionSize = SizeOfArray(kochEchoOptions);
                goto startEcho;
      case  _kochGenRand: // RANDOMS 
                generatorMode = RANDOMS;
                kochActive = true;
                currentOptions = kochGenOptions;                          // list of available options in Koch generator mode
                currentOptionSize = SizeOfArray(kochGenOptions);
                goto startTrainer;
      case  _kochGenAbb: // ABBREVS - 2
                generatorMode = ABBREVS;
                kochActive = true;
                currentOptions = kochGenOptions;                          // list of available options in Koch generator mode
                currentOptionSize = SizeOfArray(kochGenOptions);
                goto startTrainer;
      case  _kochGenWords: // WORDS - 3
                generatorMode = WORDS;
                kochActive = true;
                currentOptions = kochGenOptions;                          // list of available options in Koch generator mode
                currentOptionSize = SizeOfArray(kochGenOptions);
                goto startTrainer;
      case  _kochGenMixed: // KOCH_MIXED - 5
                generatorMode = KOCH_MIXED;
                kochActive = true;
                currentOptions = kochGenOptions;                          // list of available options in Koch generator mode
                currentOptionSize = SizeOfArray(kochGenOptions);
                goto startTrainer;
      case  _kochEchoRand: // Koch Echo Random
                generatorMode = RANDOMS;
                kochActive = true;
                currentOptions = kochEchoOptions;                          // list of available options in Koch echo trainer mode
                currentOptionSize = SizeOfArray(kochEchoOptions);
                goto startEcho;
      case  _kochEchoAbb: // ABBREVS - 2
                generatorMode = ABBREVS;
                kochActive = true;
                currentOptions = kochEchoOptions;                          // list of available options in Koch echo trainer mode
                currentOptionSize = SizeOfArray(kochEchoOptions);
                goto startEcho;
      case  _kochEchoWords: // WORDS - 3
                generatorMode = WORDS;
                kochActive = true;
                currentOptions = kochEchoOptions;                          // list of available options in Koch echo trainer mode
                currentOptionSize = SizeOfArray(kochEchoOptions);
                goto startEcho;
      case  _kochEchoMixed: // KOCH_MIXED - 5
                generatorMode = KOCH_MIXED;
                kochActive = true;
                currentOptions = kochEchoOptions;                          // list of available options in Koch echo trainer mode
                currentOptionSize = SizeOfArray(kochEchoOptions);
                goto startEcho;
      case  _trxLora: // LoRa Transceiver
                currentOptions = loraTrxOptions;                            // list of available options in lora trx mode
                currentOptionSize = SizeOfArray(loraTrxOptions);
                morseState = loraTrx;
                display.clear();
                printOnScroll(1, REGULAR, 0, "Start LoRa Trx" );
                delay(600);
                display.clear();
                displayTopLine();
                printToScroll(REGULAR,"");      // clear the buffer
                clearPaddleLatches();
                keyTx = false;
                clearText = "";
                LoRa.receive();
                return true;
                break;
      case  _trxIcw: /// icw/ext TRX
                currentOptions = extTrxOptions;                            // list of available options in ext trx mode
                currentOptionSize = SizeOfArray(extTrxOptions);
                morseState = morseTrx;
                display.clear();
                printOnScroll(1, REGULAR, 0, "Start CW Trx" );
                clearPaddleLatches();
                keyTx = true;
                goto setupDecoder;

      case  _decode: /// decoder
                currentOptions = decoderOptions;                            // list of available options in lora trx mode
                currentOptionSize = SizeOfArray(decoderOptions);
                morseState = morseDecoder;
                  /// here we will do the init for decoder mode
                //trainerMode = false;
                encoderState = volumeSettingMode;
                keyTx = false;
                display.clear();
                printOnScroll(1, REGULAR, 0, "Start Decoder" );
      setupDecoder:
                speedChanged = true;
                delay(650);
                display.clear();
                displayTopLine();
                drawInputStatus(false);
                printToScroll(REGULAR,"");      // clear the buffer
                
                displayCWspeed();
                displayVolume();
                  
                /// set up variables for Goertzel Morse Decoder
                setupGoertzel();
                filteredState = filteredStateBefore = false;
                decoderState = LOW_;
                ditAvg = 60;
                dahAvg = 180;
                return true;
                break;
      case  _wifi_mac:
                WiFi.mode(WIFI_MODE_STA);               // init WiFi as client
                display.clear();
                display.display();
                printOnStatusLine(true, 0,  WiFi.macAddress());
                delay(2000);
                printOnScroll(0, REGULAR, 0, "RED: restart" );
                delay(1000);  
                while (true) {
                  checkShutDown(false);  // possibly time-out: go to sleep
                  if (digitalRead(volButtonPin) == LOW)
                    ESP.restart();
                }
                break;
      case  _wifi_config:
                startAP();          // run as AP to get WiFi credentials from user
                break;
      case _wifi_check:
                display.clear();
                display.display();
                printOnStatusLine(true, 0,  "Connecting... ");
                if (! wifiConnect())
                    ; //return false;  
                else {
                    printOnStatusLine(true, 0,  "Connected!    ");
                    printOnScroll(0, REGULAR, 0, p_wlanSSID);
                    printOnScroll(1, REGULAR, 0, WiFi.localIP().toString());
                }
                WiFi.mode( WIFI_MODE_NULL ); // switch off WiFi                      
                delay(1000);
                printOnScroll(2, REGULAR, 0, "RED: return" );
                while (true) {
                      checkShutDown(false);  // possibly time-out: go to sleep
                      if (digitalRead(volButtonPin) == LOW)
                        return false;
                }
                break;
      case _wifi_upload:
                uploadFile();       // upload a text file
                break;
      case _wifi_update:
                updateFirmware();   // run OTA update
                break;
      case  _goToSleep: /// deep sleep
                checkShutDown(true);
      default:  break;
  }
  return false;
}   /// end menuExec()





///////////////////
// we use the paddle for iambic keying
/////

boolean doPaddleIambic (boolean dit, boolean dah) {
  boolean paddleSwap;                      // temp storage if we need to swap left and right
  static long ktimer;                      // timer for current element (dit or dah)
  static long curtistimer;                 // timer for early paddle latch in Curtis mode B+
  static long latencytimer;                // timer for "muting" paddles for some time in state INTER_ELEMENT
  unsigned int pitch;

  if (!p_didah)   {              // swap left and right values if necessary!
      paddleSwap = dit; dit = dah; dah = paddleSwap; 
  }
      

  switch (keyerState) {                                         // this is the keyer state machine
     case IDLE_STATE:
         // display the interword space, if necessary
         if (millis() > interWordTimer) {
             if (morseState == loraTrx)    {                    // when in Trx mode
                 cwForLora(3);
                 sendWithLora();                        // finalise the string and send it to LoRA
             }
             printToScroll(REGULAR, " ");                       // output a blank
             interWordTimer = 4294967000;                       // almost the biggest possible unsigned long number :-) - do not output extra spaces!
             if (echoTrainerState == COMPLETE_ANSWER)   {       // change the state of the trainer at end of word
                echoTrainerState = EVAL_ANSWER;
                return false;
             }
         }
        
       // Was there a paddle press?
        if (dit || dah ) {
            updatePaddleLatch(dit, dah);  // trigger the paddle latches
            if (morseState == echoTrainer)   {      // change the state of the trainer at end of word
                echoTrainerState = COMPLETE_ANSWER;
             }
            treeptr = 0;
            if (dit) {
                setDITstate();          // set next state
                DIT_FIRST = true;          // first paddle pressed after IDLE was a DIT
            }
            else {
                setDAHstate();  
                DIT_FIRST = false;         // first paddle was a DAH
            }
        }
        else {
           if (echoTrainerState == GET_ANSWER && millis() > genTimer) {
            echoTrainerState = EVAL_ANSWER;
         } 
         return false;                // we return false if there was no paddle press in IDLE STATE - Arduino can do other tasks for a bit
        }
        break;

    case DIT:
    /// first we check that we have waited as defined by ACS settings
            if ( p_ACSlength > 0 && (millis() <= acsTimer))  // if we do automatic character spacing, and haven't waited for (3 or whatever) dits...
              break;
            clearPaddleLatches();                           // always clear the paddle latches at beginning of new element
            keyerControl |= DIT_LAST;                        // remember that we process a DIT

            ktimer = ditLength;                              // prime timer for dit
            switch ( p_keyermode ) {
              case IAMBICB:  curtistimer = 2 + (ditLength * p_curtisBDotTiming / 100);   
                             break;                         // enhanced Curtis mode B starts checking after some time
              case NONSQUEEZE:
                             curtistimer = 3;
                             break;
              default:
                             curtistimer = ditLength;        // no early paddle checking in Curtis mode A Ultimatic mode oder Non-squeeze
                             break;
            }
            keyerState = KEY_START;                          // set next state of state machine
            break;
            
    case DAH:
            if ( p_ACSlength > 0 && (millis() <= acsTimer))  // if we do automatic character spacing, and haven't waited for 3 dits...
              break;
            clearPaddleLatches();                          // clear the paddle latches
            keyerControl &= ~(DIT_LAST);                    // clear dit latch  - we are not processing a DIT
            
            ktimer = dahLength;
            switch (p_keyermode) {
              case IAMBICB:  curtistimer = 2 + (dahLength * p_curtisBTiming / 100);    // enhanced Curtis mode B starts checking after some time
                             break;
              case NONSQUEEZE:
                             curtistimer = 3;
                             break;
              default:
                             curtistimer = dahLength;        // no early paddle checking in Curtis mode A or Ultimatic mode
                             break;
            }
            keyerState = KEY_START;                          // set next state of state machine
            break;
     

      
    case KEY_START:
          // Assert key down, start timing, state shared for dit or dah
          pitch = notes[p_sidetoneFreq];
          if ((morseState == echoTrainer || morseState == loraTrx) && p_echoToneShift != 0) {
             pitch = (p_echoToneShift == 1 ? pitch * 18 / 17 : pitch * 17 / 18);        /// one half tone higher or lower, as set in parameters in echo trainer mode
          }
           //pwmTone(pitch, p_sidetoneVolume, true);
           //keyTransmitter();
           keyOut(true, true, pitch, p_sidetoneVolume);
           ktimer += millis();                     // set ktimer to interval end time          
           curtistimer += millis();                // set curtistimer to curtis end time
           keyerState = KEYED;                     // next state
           break;
 
    case KEYED:
                                                   // Wait for timers to expire
           if (millis() > ktimer) {                // are we at end of key down ?
               //digitalWrite(keyerPin, LOW);        // turn the LED off, unkey transmitter, or whatever
               //pwmNoTone();                      // stop side tone
               keyOut(false, true, 0, 0);
               ktimer = millis() + ditLength;    // inter-element time
               latencytimer = millis() + ((p_latency-1) * ditLength / 8);
               keyerState = INTER_ELEMENT;       // next state
            }
            else if (millis() > curtistimer ) {     // in Curtis mode we check paddle as soon as Curtis time is off
                 if (keyerControl & DIT_LAST)       // last element was a dit
                    updatePaddleLatch(false, dah);  // not sure here: we only check the opposite paddle - should be ok for Curtis B
                 else
                    updatePaddleLatch(dit, false);  
                 // updatePaddleLatch(dit, dah);       // but we remain in the same state until element time is off! 
            }
            break;
 
    case INTER_ELEMENT:
            //if ((p_keyermode != NONSQUEEZE) && (millis() < latencytimer)) {     // or should it be p_keyermode > 2 ? Latency for Ultimatic mode?
            if (millis() < latencytimer) {
              if (keyerControl & DIT_LAST)       // last element was a dit
                    updatePaddleLatch(false, dah);  // not sure here: we only check the opposite paddle - should be ok for Curtis B
              else
                    updatePaddleLatch(dit, false);
              // updatePaddleLatch(dit, dah); 
            }
            else {
                updatePaddleLatch(dit, dah);          // latch paddle state while between elements       
                if (millis() > ktimer) {               // at end of INTER-ELEMENT
                    switch(keyerControl) {
                          case 3:                                         // both paddles are latched
                          case 7: 
                                  switch (p_keyermode) {
                                      case NONSQUEEZE:  if (DIT_FIRST)                      // when first element was a DIT
                                                               setDITstate();            // next element is a DIT again
                                                        else                                // but when first element was a DAH
                                                               setDAHstate();            // the next element is a DAH again! 
                                                        break;
                                      case ULTIMATIC:   if (DIT_FIRST)                      // when first element was a DIT
                                                               setDAHstate();            // next element is a DAH
                                                        else                                // but when first element was a DAH
                                                               setDITstate();            // the next element is a DIT! 
                                                        break;
                                      default:          if (keyerControl & DIT_LAST)     // Iambic: last element was a dit - this is case 7, really
                                                            setDAHstate();               // next element will be a DAH
                                                        else                                // and this is case 3 - last element was a DAH
                                                            setDITstate();               // the next element is a DIT                         
                                   }
                                   break;
                                                                          // dit only is latched, regardless what was last element  
                          case 1:
                          case 5:  
                                   setDITstate();
                                   break;
                                                                          // dah only latched, regardless what was last element
                          case 2:
                          case 6:  
                                   setDAHstate();
                                   break;
                                                                          // none latched, regardless what was last element
                          case 0:
                          case 4:  
                                   keyerState = IDLE_STATE;               // we are at the end of the character and go back into IDLE STATE
                                   displayMorse();                        // display the decoded morse character(s)
                                   if (morseState == loraTrx)
                                      cwForLora(0);
                                   
                                   ++charCounter;                         // count this character
                                   // if we have seen 12 chars since changing speed, we write the config to preferences (speed and left & right thresholds)
                                   if (charCounter == 12) {
                                      pref.begin("morserino", false);             // open the namespace as read/write
                                      pref.putUChar("wpm", p_wpm);
                                      pref.putUChar("tLeft", p_tLeft);
                                      pref.putUChar("tRight", p_tRight);
                                      pref.end();
                                   }
                                   if (p_ACSlength > 0)
                                        acsTimer = millis() + p_ACSlength * ditLength; // prime the ACS timer
                                   if (morseState == morseKeyer || morseState == loraTrx || morseState == morseTrx)
                                      interWordTimer = millis() + 5*ditLength;
                                   else
                                       interWordTimer = millis() + interWordSpace;  // prime the timer to detect a space between characters
                                                                              // nominally 7 dit-lengths, but we are not quite so strict here in keyer or TrX mode,
                                                                              // use the extended time in echo trainer mode to allow longer space between characters, 
                                                                              // like in listening
                                   keyerControl = 0;                          // clear all latches completely before we go to IDLE
                          break;
                    } // switch keyerControl : evaluation of flags
                }
            } // end of INTER_ELEMENT
  } // end switch keyerState - end of state machine

  if (keyerControl & 3)                                               // any paddle latch?                            
    return true;                                                      // we return true - we processed a paddle press
  else
    return false;                                                     // when paddle latches are cleared, we return false
} /////////////////// end function doPaddleIambic()



//// this function checks the paddles (touch or external), returns true when a paddle has been activated, 
///// and sets the global variable leftKey and rightKey accordingly


boolean checkPaddles() {
  static boolean oldL = false, newL, oldR = false, newR;
  int left, right;
  static long lTimer = 0, rTimer = 0;
  const int debDelay = 750;       // debounce time = 0,75  ms
  
  /* intral and external paddle are now working in parallel - the parameter p_extPaddle is used to indicate reverse polarity of external paddle
  */
  left = p_useExtPaddle ? rightPin : leftPin;
  right = p_useExtPaddle ? leftPin : rightPin;
  sensor = readSensors(LEFT, RIGHT);
  newL = (sensor >> 1) | (!digitalRead(left)) ;
  newR = (sensor & 0x01) | (!digitalRead(right)) ;

  if ((p_keyermode == NONSQUEEZE) && newL && newR) 
    return (leftKey || rightKey);

  if (newL != oldL)
      lTimer = micros();
  if (newR != oldR)
      rTimer = micros();
  if (micros() - lTimer > debDelay)
      if (newL != leftKey) 
          leftKey = newL;
  if (micros() - rTimer > debDelay)
      if (newR != rightKey) 
          rightKey = newR;       

  oldL = newL;
  oldR = newR;
  
  return (leftKey || rightKey);
}

///
/// Keyer subroutines
///

// update the paddle latches in keyerControl
void updatePaddleLatch(boolean dit, boolean dah)
{ 
    if (dit)
      keyerControl |= DIT_L;
    if (dah)
      keyerControl |= DAH_L;
}

// clear the paddle latches in keyer control
void clearPaddleLatches ()
{
    keyerControl &= ~(DIT_L + DAH_L);   // clear both paddle latch bits
}

// functions to set DIT and DAH keyer states
void setDITstate() {
  keyerState = DIT;
  treeptr = CWtree[treeptr].dit;
  if (morseState == loraTrx)
      cwForLora(1);                         // build compressed string for LoRA
}

void setDAHstate() {
  keyerState = DAH;
  treeptr = CWtree[treeptr].dah;
  if (morseState == loraTrx)
      cwForLora(2);
}


// toggle polarity of paddles
void togglePolarity () {
      p_didah = !p_didah; 
     //displayPolarity();
}
  

/// display decoded morse code (and store it in echoTrainer
void displayMorse() {
  String symbol;
  symbol.reserve(6);
  if (treeptr == 0)
    return;
  symbol = CWtree[treeptr].symb;
  //Serial.println("Symbol: " + symbol + " treeptr: " + String(treeptr));
  printToScroll( REGULAR, symbol);
  if (morseState == echoTrainer) {                /// store the character in the response string
      symbol.replace("<as>", "S");
      symbol.replace("<ka>", "A");
      symbol.replace("<kn>", "N");
      symbol.replace("<sk>", "K");
      symbol.replace("<ve>", "V");
      symbol.replace("<ch>", "H");
      echoResponse.concat(symbol);
  }
  treeptr = 0;                                    // reset tree pointer
}   /// end of displayMorse()



//// functions for generating a tone....

void pwmTone(unsigned int frequency, unsigned int volume, boolean lineOut) { // frequency in Hertz, volume in range 0 - 100; we use 10 bit resolution
  const uint16_t vol[] = {0, 1,  2, 3, 16, 150, 380, 580, 700, 880, 1023};
  int i = constrain(volume/10, 0, 10);
  //Serial.println(vol[i]);
  //Serial.println(frequency);
  if (lineOut) {
      ledcWriteTone(lineOutChannel, (double) frequency);
      ledcWrite(lineOutChannel, dutyCycleFiftyPercent);
  }

  ledcWrite(volChannel, volFreq);
  ledcWrite(volChannel, vol[i]);
  ledcWriteTone(toneChannel, frequency);
 

  if (i == 0 ) 
      ledcWrite(toneChannel, dutyCycleZero);
  else if (i > 3)
      ledcWrite(toneChannel, dutyCycleFiftyPercent);
  else
      ledcWrite(toneChannel, i*i*i + 4 + 2*i);          /// an ugly hack to allow for lower volumes on headphones
    
   
  
}


void pwmNoTone() {      // stop playing a tone by changing duty cycle of the tone to 0
  ledcWrite(toneChannel, dutyCycleTwentyPercent);
  ledcWrite(lineOutChannel, dutyCycleTwentyPercent);
  delayMicroseconds(125);
  ledcWrite(toneChannel, dutyCycleZero);
  ledcWrite(lineOutChannel, dutyCycleZero);
  
}


void pwmClick(unsigned int volume) {                        /// generate a click on the speaker
    if (!p_encoderClicks)
      return;
    pwmTone(250,volume, false);
    delay(6);
    pwmTone(280,volume, false);
    delay(5);
    pwmNoTone();
}


//////// Display the status line in CW Keyer Mode
//////// Layout of top line:
//////// Tch ul 15 WpM
//////// 0    5    0

void displayTopLine() {
  clearStatusLine();

  // printOnStatusLine(true, 0, (p_useExtPaddle ? "X " : "T "));          // we do not show which paddle is in use anymore
  if (morseState == morseGenerator) 
    printOnStatusLine(true, 1,  p_wordDoubler ? "x2" : "  ");
  else {
    switch (p_keyermode) {
      case IAMBICA:   printOnStatusLine(false, 2,  "A "); break;          // Iambic A (no paddle eval during dah)
      case IAMBICB:   printOnStatusLine(false, 2,  "B "); break;          // orig Curtis B mode: paddle eval during element
      case ULTIMATIC: printOnStatusLine(false, 2,  "U "); break;          // Ultimatic Mode
      case NONSQUEEZE: printOnStatusLine(false, 2,  "N "); break;         // Non-squeeze mode
    }
  }

  displayCWspeed();                                     // update display of CW speed
  if ((morseState == loraTrx ) || (morseState == morseGenerator  && p_loraTrainerMode == true))
      dispLoraLogo();

  displayVolume();                                     // sidetone volume
  display.display();
}

void dispLoraLogo() {     // display a small logo in the top right corner to indicate we operate with LoRa
  display.setColor(BLACK);
  display.drawXbm(121, 2, lora_width, lora_height, lora_bits);
  display.setColor(WHITE);
  display.display();
}

//////// Display the current CW speed
/////// pos 7-8, "Wpm" on 10-12

void displayCWspeed () {
  if (( morseState == morseGenerator || morseState ==  echoTrainer )) 
      sprintf(numBuffer, "(%2i)", effWpm);   
  else sprintf(numBuffer, "    ");
  
  printOnStatusLine(false, 3,  numBuffer);                                         // effective wpm
  
  sprintf(numBuffer, "%2i", p_wpm);
  printOnStatusLine(encoderState == speedSettingMode ? true : false, 7,  numBuffer);
  printOnStatusLine(false, 10,  "WpM");
  display.display();
}


/// function to read sensors:
/// read both left and right twice, repeat reading if it returns 0
/// return a binary value, depending on a (adaptable?) threshold:
/// 0 = nothing touched,  1= right touched, 2 = left touched, 3 = both touched
/// binary:   00          01                10                11

uint8_t readSensors(int left, int right) {
  //static boolean first = true;
  uint8_t v, lValue, rValue;
  
  while ( !(v=touchRead(left)) )
    ;                                       // ignore readings with value 0
  lValue = v;
   while ( !(v=touchRead(right)) )
    ;                                       // ignore readings with value 0
  rValue = v;
  while ( !(v=touchRead(left)) )
    ;                                       // ignore readings with value 0
  lValue = (lValue+v) /2;
   while ( !(v=touchRead(right)) )
    ;                                       // ignore readings with value 0
  rValue = (rValue+v) /2;

  if (lValue < (p_tLeft+10))     {           //adaptive calibration
      //if (first) Serial.println("p-tLeft " + String(p_tLeft));
      //if (first) Serial.print("lValue: "); if (first) Serial.println(lValue);
      //printOnScroll(0, INVERSE_BOLD, 0,  String(lValue) + " " + String(p_tLeft));
      p_tLeft = ( 7*p_tLeft +  ((lValue+lUntouched) / SENS_FACTOR) ) / 8;
      //Serial.print("p_tLeft: "); Serial.println(p_tLeft);
  }
  if (rValue < (p_tRight+10))     {           //adaptive calibration
      //if (first) Serial.println("p-tRight " + String(p_tRight));
      //if (first) Serial.print("rValue: "); if (first) Serial.println(rValue);
      //printOnScroll(1, INVERSE_BOLD, 0,  String(rValue) + " " + String(p_tRight));
      p_tRight = ( 7*p_tRight +  ((rValue+rUntouched) / SENS_FACTOR) ) / 8;
      //Serial.print("p_tRight: "); Serial.println(p_tRight);
  }
  //first = false;
  return ( lValue < p_tLeft ? 2 : 0 ) + (rValue < p_tRight ? 1 : 0 );
}


void initSensors() {
  int v;
  lUntouched = rUntouched = 60;       /// new: we seek minimum
  for (int i=0; i<8; ++i) {
      while ( !(v=touchRead(LEFT)) )
        ;                                       // ignore readings with value 0
        lUntouched += v;
        //lUntouched = _min(lUntouched, v);
       while ( !(v=touchRead(RIGHT)) )
        ;                                       // ignore readings with value 0
        rUntouched += v;
        //rUntouched = _min(rUntouched, v);
  }
  lUntouched /= 8;
  rUntouched /= 8;
  p_tLeft = lUntouched - 9;
  p_tRight = rUntouched - 9;
}


String getRandomWord( int maxLength) {        //// give me a random English word, max maxLength chars long (1-5) - 0 returns any length
  if (maxLength > 5)
    maxLength = 0;
    if (kochActive)
        return kochWords[random(numberOfWords)]; 
    else 
        return words[random(WORDS_POINTER[maxLength], WORDS_NUMBER_OF_ELEMENTS)];
}

String getRandomAbbrev( int maxLength) {        //// give me a random CW abbreviation , max maxLength chars long (1-5) - 0 returns any length
  if (maxLength > 5)
    maxLength = 0;
    if (kochActive)
        return kochAbbr[random(numberOfAbbr)];
    else
        return abbreviations[random(ABBREV_POINTER[maxLength], ABBREV_NUMBER_OF_ELEMENTS)];  
}

// we use substrings as char pool for trainer mode
  // SANK will be replaced by <as>, <ka>, <kn> and <sk>
  // Options:
  //    0: a9?<> = CWchars; all of them; same as Koch 45+
  //    1: a = CWchars.substring(0,26);
  //    2: 9 = CWchars.substring(26,36);
  //    3: ? = CWchars.substring(36,45);
  //    4: <> = CWchars.substring(44,50);
  //    5: a9 = CWchars.substring(0,36);
  //    6: 9? = CWchars.substring(26,45);
  //    7: ?<> = CWchars.substring(36,50);
  //    8: a9? = CWchars.substring(0,45); 
  //    9: 9?<> = CWchars.substring(26,50);

  //  {OPT_ALL, OPT_ALPHA, OPT_NUM, OPT_PUNCT, OPT_PRO, OPT_ALNUM, OPT_NUMPUNCT, OPT_PUNCTPRO, OPT_ALNUMPUNCT, OPT_NUMPUNCTPRO}

String getRandomChars( int maxLength, int option) {             /// random char string, eg. group of 5, 9 differing character pools; maxLength = 1-6
  String result = ""; String pool;
  int s = 0, e = 50;
  int i;
    if (maxLength > 6) {                                        // we use a random length!
      maxLength = random(2, maxLength - 3);                     // maxLength is max 10, so random upper limit is 7, means max 6 chars...
    }
    if (kochActive) {                                           // kochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=KA+SNE@:"
        int endk =  p_kochFilter;                               //              1   5    1    5    2    5    3    5    4    5    5  
        for (i = 0; i < maxLength; ++i) {
        if (random(2))                                          // in Koch mode, we generate the last third of the chars learned  a bit more often
            result += kochChars.charAt(random(2*endk/3, endk));
        else
            result += kochChars.charAt(random(endk));
        }
    } else {
         switch (option) {
          case OPT_NUM: 
          case OPT_NUMPUNCT: 
          case OPT_NUMPUNCTPRO: 
                                s = 26; break;
          case OPT_PUNCT: 
          case OPT_PUNCTPRO: 
                                s = 36; break;
          case OPT_PRO: 
                                s = 44; break;
          default:              s = 0;  break;
        }
        switch (option) {
          case OPT_ALPHA: 
                                e = 26;  break;
          case OPT_ALNUM: 
          case OPT_NUM: 
                                e = 36; break;
          case OPT_ALNUMPUNCT: 
          case OPT_NUMPUNCT:
          case OPT_PUNCT: 
                                e = 45; break;
          default:              e = 50; break;
        }

        for (i = 0; i < maxLength; ++i) 
          result += CWchars.charAt(random(s,e));
  }
  return result;
}


String getRandomCall( int maxLength) {            // random call-sign like pattern, maxLength = 3 - 6, 0 returns any length
  const byte prefixType[] = {1,0,1,2,3,1};         // 0 = a, 1 = aa, 2 = a9, 3 = 9a
  byte prefix;
  String call = "";
  unsigned int l = 0;
  //int s, e;

  if (maxLength == 1 || maxLength == 2)
      maxLength = 3;
  if (maxLength > 6)
      maxLength = 6;

  if (maxLength == 3)
      prefix = 0;
  else  
      prefix = prefixType[random(0,6)];           // what type of prefix?
  switch (prefix) {
      case 1: call += CWchars.charAt(random(0,26));
              ++l;
      case 0: call += CWchars.charAt(random(0,26));
              ++l;
              break;
      case 2: call += CWchars.charAt(random(0,26));
              call += CWchars.charAt(random(26,36));
              l = 2;
              break;
      case 3: call += CWchars.charAt(random(26,36));
              call += CWchars.charAt(random(0,26));
              l = 2;
              break;
    } // we have a prefix by now; l is its length
      // now generate a number
    call += CWchars.charAt(random(26,36));
    ++l;
    // generate a suffix, 1 2 or 3 chars long - we re-use prefix for the suffix length
    if (maxLength == 3)
        prefix = 1;
    else if (maxLength == 0) {
        prefix = random(1,4);
        prefix = (prefix == 2 ? prefix :  random(1,4)); // increase the likelihood for suffixes of length 2
    }
    else {
        //prefix = random(1,_min(maxLength-l+1, 4));     // suffix not longer than 3 chars!
        prefix = _min(maxLength - l, 3);                 // we try to always give the desired length, but never more than 3 suffix chars
    }
    while (prefix--) {
      call += CWchars.charAt(random(0,26));
      ++l;
    } // now we have the suffix as well
    // are we /p or /m? - we do this only in rare cases - 1 out of 9, and only when maxLength = 0, or maxLength-l >= 2
    if (maxLength == 0 ) //|| maxLength - l >= 2)
      if (! random(0,8)) {
      call += "/";
      call += ( random(0,2) ? "m" : "p" );
    }
    // we have a complete call sign!
    return call;
}


/////// generate CW representations from its input string 
/////// CWchars = "abcdefghijklmnopqrstuvwxyz0123456789.,:-/=?@+SANKVäöüH";

String generateCWword(String symbols) {
  int pointer;
  byte bitMask, NoE;
  //byte nextElement[8];      // the list of elements; 0 = dit, 1 = dah
  String result = "";
  
  int l = symbols.length();
  
  for (int i = 0; i<l; ++i) {
    char c = symbols.charAt(i);                                 // next char in string
    pointer = CWchars.indexOf(c);                               // at which position is the character in CWchars?
    NoE = pool[pointer][1];                                     // how many elements in this morse code symbol?
    bitMask = pool[pointer][0];                                 // bitMask indicates which of the elements are dots and which are dashes
    for (int j=0; j<NoE; ++j) {
        result += (bitMask & B10000000 ? "2" : "1" );         // get MSB and store it in string - 2 is dah, 1 is dit, 0 = inter-element space
        bitMask = bitMask << 1;                               // shift bitmask 1 bit to the left 
        //Serial.print("Bitmask: ");
        //Serial.println(bitmask, BIN);
      } /// now we are at the end of one character, therefore we add enough space for inter-character
      result += "0";
  }     /// now we are at the end of the word, therefore we remove the final 0!
  result.remove(result.length()-1);
  return result;
}


void generateCW () {          // this is called from loop() (frequently!)  and generates CW
  
  static int l;
  static char c;
  boolean silentEcho;
  
  switch (generatorState) {                                             // CW generator state machine - key is up or down
    case KEY_UP:
            if (millis() < genTimer)                                    // not yet at end of the pause: just wait
                return;                                                 // therefore we return to loop()
             // here we continue if the pause has been long enough
            if (startFirst == true)
                CWword = "";
            l = CWword.length();
            
            if (l==0)  {                                               // fetch a new word if we have an empty word
                if (clearText.length() > 0) {                          // this should not be reached at all.... except when display word by word
                  //Serial.println("Text left: " + clearText);
                  if (morseState == loraTrx || (morseState == morseGenerator && effectiveTrainerDisplay == DISPLAY_BY_WORD) ||
                        ( morseState == echoTrainer && p_echoDisplay != CODE_ONLY)) {
                      printToScroll(BOLD,cleanUpProSigns(clearText));
                      clearText = "";
                  }
                }
                fetchNewWord();
                //Serial.println("New Word: " + CWword);
                if (CWword.length() == 0)                             // we really should have something here - unless in trx mode; in this case return
                  return;
                if ((morseState == echoTrainer)) {
                  printToScroll(REGULAR, "\n");
                }
            }
            c = CWword[0];                                            // retrieve next element from CWword; if 0, we were at end of character
            CWword.remove(0,1); 
            if (c == '0' || !CWword.length())  {                      // a character just had been finished //// is there an error here?
                   if (c == '0') {
                      c = CWword[0];                                  // retrieve next element from CWword;
                      CWword.remove(0,1); 
                      if (morseState == morseGenerator && p_loraTrainerMode == 1)
                          cwForLora(0);                             // send end of character to lora
                      }
            }   /// at end of character

            //// insert code here for outputting only on display, and not as morse characters - for echo trainer
            //// genTimer vy short (1ms?)
            //// no keyOut()
            if (morseState == echoTrainer && p_echoDisplay == DISP_ONLY)
                genTimer = millis() + 2;      // very short timing
            else if (morseState != loraTrx)
                genTimer = millis() + (c == '1' ? ditLength : dahLength);           // start a dit or a dah, acording to the next element
            else 
                genTimer = millis() + (c == '1' ? rxDitLength : rxDahLength);
            if (morseState == morseGenerator && p_loraTrainerMode == 1)             // send the element to LoRa
                c == '1' ? cwForLora(1) : cwForLora(2) ; 
            /// if Koch learn character we show dit or dah
            if (generatorMode == KOCH_LEARN)
                printToScroll(REGULAR, c == '1' ? "."  : "-");

            silentEcho = (morseState == echoTrainer && p_echoDisplay == DISP_ONLY); // echo mode with no audible prompt

            if (silentEcho || stopFlag)                                             // we finished maxSequence and so do start output (otherwise we get a short click)
              ;
            else  {
                keyOut(true, (morseState != loraTrx), notes[p_sidetoneFreq], p_sidetoneVolume);
            }
            /* // replaced by the lines above, to also take care of maxSequence
            if ( ! (morseState == echoTrainer && p_echoDisplay == DISP_ONLY)) 
                   
                        keyOut(true, (morseState != loraTrx), notes[p_sidetoneFreq], p_sidetoneVolume);
            */
            generatorState = KEY_DOWN;                              // next state = key down = dit or dah

            break;
    case KEY_DOWN:
            if (millis() < genTimer)                                // if not at end of key down we need to wait, so we just return to loop()
                return;
            //// otherwise we continue here; stop keying,  and determine the length of the following pause: inter Element, interCharacter or InterWord?

           keyOut(false, (morseState != loraTrx), 0, 0);
            if (! CWword.length())   {                                 // we just ended the the word, ...  //// intercept here in Echo Trainer mode
 //             // display last character - consider echo mode!
                if (morseState == morseGenerator) 
                    autoStop = effectiveAutoStop ? stop1 : off;
                dispGeneratedChar();
                if (morseState == echoTrainer) {
                    switch (echoTrainerState) {
                        case START_ECHO:  echoTrainerState = SEND_WORD;
                                          genTimer = millis() + interCharacterSpace + (p_promptPause * interWordSpace);
                                          break;
                        case REPEAT_WORD:
                                          // fall through 
                        case SEND_WORD:   if (echoStop)
                                                break;
                                          else {
                                                echoTrainerState = GET_ANSWER;
                                                if (p_echoDisplay != CODE_ONLY) {
                                                    printToScroll(REGULAR, " ");
                                                    printToScroll(INVERSE_REGULAR, ">");    /// add a blank after the word on the display
                                                }
                                                ++repeats;
                                                genTimer = millis() + p_responsePause * interWordSpace;
                                          }
                        default:          break;
                    }
                }
                else { 
                      genTimer = millis() + (morseState == loraTrx ? rxInterWordSpace : interWordSpace) ;              // we need a pause for interWordSpace
                      if (morseState == morseGenerator && p_loraTrainerMode == 1) {                                   // in generator mode and we want to send with LoRa
                          cwForLora(0);
                          cwForLora(3);                           // as we have just finished a word
                          //Serial.println("cwForLora(3);");
                          sendWithLora();                         // finalise the string and send it to LoRA
                          delay(interCharacterSpace+ditLength);             // we need a slightly longer pause otherwise the receiving end might fall too far behind...
                          } 
                }
             }
             else if ((c = CWword[0]) == '0') {                                                                        // we are at end of character
//              // display last character 
//              // genTimer small if in echo mode and no code!
                dispGeneratedChar(); 
                if (morseState == echoTrainer && p_echoDisplay == DISP_ONLY)
                    genTimer = millis() +1;
                else            
                    genTimer = millis() + (morseState == loraTrx ? rxInterCharacterSpace : interCharacterSpace);          // pause = intercharacter space
             }
             else  {                                                                                                   // we are in the middle of a character
                genTimer = millis() + (morseState == loraTrx ? rxDitLength : ditLength);                              // pause = interelement space
             }
             generatorState = KEY_UP;                               // next state = key up = pause
             break;         
  }   /// end switch (generatorState)
}


/// when generating CW, we display the character (under certain circumstances)
/// add code to display in echo mode when parameter is so set
/// p_echoDisplay 1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP

void dispGeneratedChar() {
  static String charString;
  charString.reserve(10);
  
  if (generatorMode == KOCH_LEARN || morseState == loraTrx || (morseState == morseGenerator && effectiveTrainerDisplay == DISPLAY_BY_CHAR) ||
                    ( morseState == echoTrainer && p_echoDisplay != CODE_ONLY ))
                    //&& echoTrainerState != SEND_WORD
                    //&& echoTrainerState != REPEAT_WORD)) 
    
      {       /// we need to output the character on the display now  
        charString = clearText.charAt(0);                   /// store first char of clearText in charString
        clearText.remove(0,1);                              /// and remove it from clearText
        if (generatorMode == KOCH_LEARN)
            printToScroll(REGULAR,"");                      // clear the buffer first
        printToScroll(morseState == loraTrx ? BOLD : REGULAR, cleanUpProSigns(charString));
        if (generatorMode == KOCH_LEARN)
            printToScroll(REGULAR," ");                      // output a space
      }   //// end display_by_char
      
      ++charCounter;                         // count this character
     
     // if we have seen 12 chars since changing speed, we write the config to Preferences
     if (charCounter == 12) {
        pref.begin("morserino", false);             // open the namespace as read/write
        pref.putUChar("wpm", p_wpm);
        pref.end();
     }
}

void fetchNewWord() {
  int rssi, rxWpm, rv;

//Serial.println("startFirst: " + String((startFirst ? "true" : "false")));
//Serial.println("firstTime: " + String((firstTime ? "true" : "false")));
    if (morseState == loraTrx) {                                // we check the rxBuffer and see if we received something
       updateSMeter(0);                                         // at end of word we set S-meter to 0 until we receive something again
       //Serial.print("end of word - S0? ");
       startFirst = false;
       ////// from here: retrieve next CWword from buffer!
        if (loRaBuReady()) {
            printToScroll(BOLD, " ");
            uint8_t header = decodePacket(&rssi, &rxWpm, &CWword);
            //Serial.println("Header: " + (String) header);
            //Serial.println("CWword: " + (String) CWword);
            //Serial.println("Speed: " + (String) rxWpm);
            
            if ((header >> 6) != 1)                             // invalid protocol version
              return;
            if ((rxWpm < 5) || (rxWpm >60))                    // invalid speed
              return;
            clearText = CWwordToClearText(CWword);
            //Serial.println("clearText: " + (String) clearText);
            //Serial.println("RX Speed: " + (String)rxWpm);
            //Serial.println("RSSI: " + (String)rssi);
            
            rxDitLength = 1200 /   rxWpm ;                      // set new value for length of dits and dahs and other timings
            rxDahLength = 3* rxDitLength ;                      // calculate the other timing values
            rxInterCharacterSpace = 3 * rxDitLength;
            rxInterWordSpace = 7 * rxDitLength;
            sprintf(numBuffer, "%2ir", rxWpm);
            printOnStatusLine(true, 4,  numBuffer); 
            printOnStatusLine(true, 9, "s");
            updateSMeter(rssi);                                 // indicate signal strength of new packet
       }
       else return;                                             // we did not receive anything
               
    } // end if loraTrx
    else {

    //if (morseState != echoTrainer)
    if ((morseState == morseGenerator) && !effectiveAutoStop) {
        printToScroll(REGULAR, " ");    /// in any case, add a blank after the word on the display
    }
    
    if (generatorMode == KOCH_LEARN) {
        startFirst = false;
        echoTrainerState = SEND_WORD;
    }
    if (startFirst == true)  {                                 /// do the intial sequence in trainer mode, too
        clearText = "vvvA";
        startFirst = false;
    } else if (morseState == morseGenerator && p_wordDoubler == true && firstTime == false) {
        clearText = echoTrainerWord;
        firstTime = true;
    } else if (morseState == echoTrainer) {
        interWordTimer = 4294967000;                   /// interword timer should not trigger something now
        //Serial.println("echoTrainerState: " + String(echoTrainerState));
        switch (echoTrainerState) {
            case  REPEAT_WORD:  if (p_echoRepeats == 7 || repeats <= p_echoRepeats) 
                                    clearText = echoTrainerWord;
                                else {
                                    clearText = echoTrainerWord;
                                    if (generatorMode != KOCH_LEARN) {
                                        printToScroll(INVERSE_REGULAR, cleanUpProSigns(clearText));    //// clean up first!
                                        printToScroll(REGULAR, " ");
                                    }
                                    goto randomGenerate;
                                }
                                break;
            //case  START_ECHO:
            case  SEND_WORD:    goto randomGenerate;
            default:            break;
        }   /// end special cases for echo Trainer
    } else {   
   
      randomGenerate:       repeats = 0;
                            if (((morseState == morseGenerator) || (morseState == echoTrainer)) && (p_maxSequence != 0) &&
                                    (generatorMode != KOCH_LEARN))  {                                           // a case for maxSequence
                                ++ wordCounter;
                                int limit = 1 + p_maxSequence;
                                if (wordCounter == limit) {
                                  clearText = "+";
                                    echoStop = true;
                                }
                                else if (wordCounter == (limit+1)) {
                                    stopFlag = true;
                                    echoStop = false;
                                    wordCounter = 1;
                                }
                            }
                            if (clearText != "+") {
                                switch (generatorMode) {
                                      case  RANDOMS:  clearText = getRandomChars(p_randomLength, p_randomOption);
                                                      break;
                                      case  CALLS:    clearText = getRandomCall(p_callLength);
                                                      break;
                                      case  ABBREVS:  clearText = getRandomAbbrev(p_abbrevLength);
                                                      break;
                                      case  WORDS:    clearText = getRandomWord(p_wordLength);
                                                      break;
                                      case  KOCH_LEARN:clearText = (String) kochChars.charAt(p_kochFilter - 1);
                                                      break;
                                      case  MIXED:    rv = random(4);
                                                      switch (rv) {
                                                        case  0:  clearText = getRandomWord(p_wordLength);
                                                                  break;
                                                        case  1:  clearText = getRandomAbbrev(p_abbrevLength);
                                                                    break;
                                                        case  2:  clearText = getRandomCall(p_callLength);
                                                                  break;
                                                        case  3:  clearText = getRandomChars(1,OPT_PUNCTPRO);        // just a single pro-sign or interpunct
                                                                  break;
                                                      }
                                                      break;
                                      case KOCH_MIXED:rv = random(3);
                                                      switch (rv) {
                                                        case  0:  clearText = getRandomWord(p_wordLength);
                                                                  break;
                                                        case  1:  clearText = getRandomAbbrev(p_abbrevLength);
                                                                    break;
                                                        case  2:  clearText = getRandomChars(p_randomLength, OPT_KOCH);        // Koch option!
                                                                  break;
                                                      }
                                                      break;
                                      case PLAYER:    if (p_randomFile)
                                                          skipWords(random(p_randomFile+1));
                                                      clearText = getWord();
                                                      /*
                                                      if (clearText == String()) {        /// at end of file: go to beginning again
                                                        p_fileWordPointer = 0;
                                                        file.close(); file = SPIFFS.open("/player.txt");
                                                      }
                                                      ++p_fileWordPointer;
                                                      */
                                                      clearText = cleanUpText(clearText);
                                                      break;  
                                      case NA: break;
                                    }   // end switch (generatorMode)
                            }
                            firstTime = false;
      }       /// end if else - we either already had something in trainer mode, or we got a new word

      CWword = generateCWword(clearText);
      echoTrainerWord = clearText;
    } /// else (= not in loraTrx mode)
} // end of fetchNewWord()



////// S Meter for Trx modus

void updateSMeter(int rssi) {

  static boolean wasZero = false;

  if (rssi == 0) 
      if (wasZero)
          return;
       else {
          drawVolumeCtrl( false, 93, 0, 28, 15, 0);
          wasZero = true;
       }
   else {
      drawVolumeCtrl( false, 93, 0, 28, 15, constrain(map(rssi, -150, -20, 0, 100), 0, 100));
      wasZero = false;
   }
  display.display();
}

////// setup preferences ///////

             
boolean setupPreferences(uint8_t atMenu) {
  // enum morserinoMode {morseKeyer, loraTrx, morseGenerator, echoTrainer, shutDown, morseDecoder, invalid };
  static int oldPos = 1;
  int t;

  int ptrIndex, ptrMax;
  prefPos posPtr;
 
  ptrMax = currentOptionSize;

  ///// we should check here if the old ptr (oldIndex) is contained in the current preferences collection (currentOptions)
  ptrIndex = 1;
  for (int i = 0; i < ptrMax; ++i) {
      if (currentOptions[i] == oldPos) {
          ptrIndex = i;
          break;
      }
  }
  posPtr = currentOptions[ptrIndex];  
  keyOut(false, true, 0, 0);                // turn the LED off, unkey transmitter, or whatever; just in case....
  keyOut(false,false, 0, 0);  
  displayKeyerPreferencesMenu(posPtr);
  printOnScroll(2, REGULAR, 0,  " ");

  while (true) {                            // we wait for single click = selection or long click = exit - or single or long click or RED button
        modeButton.Update();
        switch (modeButton.clicks) {            // button was clicked
          case 1:     // change the option corresponding to pos
                      if (adjustKeyerPreference(posPtr))
                         goto exitFromHere;
                      break;
          case -1:    //////// long press indicates we are done with setting preferences - check if we need to store some of the preferences
          exitFromHere: writePreferences("morserino");
                        //delay(200);
                        return false;
                        break;
          }

          volButton.Update();                 // RED button
          switch (volButton.clicks) {         // was clicked
            case 1:     // recall snapshot
                        if (recallSnapshot())
                          writePreferences("morserino");
                        //delay(100);
                        return true;
                        break;
            case -1:    //store snapshot
                        
                        if (storeSnapshot(atMenu))
                          writePreferences("morserino");
                        while(volButton.clicks)
                          volButton.Update();
                        return false;
                        break;
          }

          
          //// display the value of the preference in question

         if ((t=checkEncoder())) {
            pwmClick(p_sidetoneVolume);         /// click 
            ptrIndex = (ptrIndex +ptrMax + t) % ptrMax;
            //Serial.println("ptrIndex: " + String(ptrIndex));
            posPtr = currentOptions[ptrIndex];
            //oldIndex = ptrIndex;                                                              // remember menu position
            oldPos = posPtr;
            
            displayKeyerPreferencesMenu(posPtr);
            //printOnScroll(1, BOLD, 0, ">");
            printOnScroll(2, REGULAR, 0, " ");

            display.display();                                                        // update the display   
         }    // end if (encoderPos)
         checkShutDown(false);         // check for time out
  } // end while - we leave as soon as the button has been pressed long
}   // end function setupKeyerPreferences()



///////// evaluate the response in Echo Trainer Mode
void echoTrainerEval() {
    delay(interCharacterSpace / 2);
    if (echoResponse == echoTrainerWord) {
      echoTrainerState = SEND_WORD;
      printToScroll(BOLD,  "OK");
      if (p_echoConf) {
          pwmTone(440,  p_sidetoneVolume, false);
          delay(97);
          pwmNoTone();
          pwmTone(587,  p_sidetoneVolume, false);
          delay(193);
          pwmNoTone();
      }
      delay(interWordSpace);
      if (p_speedAdapt)
          changeSpeed(1);
    } else {
      echoTrainerState = REPEAT_WORD;
      if (generatorMode != KOCH_LEARN || echoResponse != "") {
          printToScroll(BOLD, "ERR");
          if (p_echoConf) {
              pwmTone(311,  p_sidetoneVolume, false);
              delay(193);
              pwmNoTone();
          }
      }
      delay(interWordSpace);
      if (p_speedAdapt)
          changeSpeed(-1);
    }
    echoResponse = "";
    clearPaddleLatches();
}   // end of function


void updateTimings() {
  ditLength = 1200 / p_wpm;                    // set new value for length of dits and dahs and other timings
  dahLength = 3 * ditLength;
  interCharacterSpace =  p_interCharSpace *  ditLength;
  //interWordSpace = _max(p_interWordSpace * ditLength, (p_interCharSpace+6)*ditLength);
  interWordSpace = _max(p_interWordSpace, p_interCharSpace+4) * ditLength;

  effWpm = 60000 / (31 * ditLength + 4 * interCharacterSpace + interWordSpace );  ///  effective wpm with lengthened spaces = Farnsworth speed
} 

void changeSpeed( int t) {
  p_wpm += t;
  p_wpm = constrain(p_wpm, 5, 60);
  updateTimings();
  displayCWspeed();                     // update display of CW speed
  charCounter = 0;                                    // reset character counter
}


void keyTransmitter() {
  if (p_keyTrainerMode == 0 || morseState == echoTrainer || morseState == loraTrx )
      return;                              // we never key Tx under these conditions
  if (p_keyTrainerMode == 1 && morseState == morseGenerator)
      return;                              // we key only in keyer mode; in all other case we key TX
  if (keyTx == false)
      return;                              // do not key when input is from tone decoder
   digitalWrite(keyerPin, HIGH);           // turn the LED on, key transmitter, or whatever
}








/// cwForLora packs element info (dit, dah, interelement space) into a String that can be sent via LoRA
///  element can be:
///  0: inter-element space
///  1: dit
///  2: dah
///  3: end of word -: cwForLora returns a string that is ready for sending to the LoRa transceiver

//  char loraTxBuffer[32];

void cwForLora (int element) {
  //static String result;
  //result.reserve(36);
  //static char buf[32];
  static int pairCounter = 0;
  uint8_t temp;
  uint8_t header;

  if (pairCounter == 0) {   // we start a brand new word for LoRA - clear buffer and set speed first
      for (int i=0; i<32; ++i)
          loraTxBuffer[i] = (char) 0;
          
      /// 1st byte: version + serial number
      header = ++loRaSerial % 64;
      //Serial.println("loRaSerial: " + String(loRaSerial));
      header += CWLORAVERSION * 64;        //// shift VERSION left 6 bits and add to the serial number
      loraTxBuffer[0] = header;

      temp = p_wpm * 4;                   /// shift left 2 bits
      loraTxBuffer[1] |= temp;
      pairCounter = 7;                    /// so far we have used 7 bit pairs: 4 in the first byte (protocol version+serial); 3 in the 2nd byte (wpm)
      //Serial.println(temp);
      //Serial.println(loraBuffer);
      }

  temp = element & B011;      /// take the two left bits
      //Serial.println("Temp before shift: " + String(temp));
  /// now store these two bits in the correct location in loraBuffer

  if (temp && (temp != 3)) {                 /// no need to do the operation with 0, nor with B11
      temp = temp << (2*(3-(pairCounter % 4)));
      loraTxBuffer[pairCounter/4] |= temp;
  }

  /// now increment, unless we got end of word
  /// have we get end of word, we got end of character (0) before

  if (temp != 3)
      ++pairCounter;
  else {  
      --pairCounter; /// we are at end of word and step back to end of character
      if (pairCounter % 4 != 0)      {           // do nothing if we have a zero in the topmost two bits already, as this was end of character
          temp = temp << (2*(3-(pairCounter % 4)));
          loraTxBuffer[pairCounter/4] |= temp;
      }
      pairCounter = 0;
  }
}


void sendWithLora() {           // hand this string over as payload to the LoRA transceiver
  // send packet
  LoRa.beginPacket();
  LoRa.print(loraTxBuffer);
  LoRa.endPacket();
  if (morseState == loraTrx)
      LoRa.receive();
}

void onReceive(int packetSize)
{
  String result;
  result.reserve(64);
  result = "";
  
  // received a packet
  // read packet
  for (int i = 0; i < packetSize; i++)
  {
    result += (char)LoRa.read();
    //Serial.print((char)LoRa.read());
  }
  if (packetSize < 49)
      storePacket(LoRa.packetRssi(), result);
  else
      Serial.println("LoRa Packet longer than 48 bytes! Discarded...");
  // print RSSI of packet
  //Serial.print("' with RSSI ");
  //Serial.println(LoRa.packetRssi());
  //Serial.print(" S-Meter: ");
  //Serial.println(map(LoRa.packetRssi(), -160, -20, 0, 100));
}



String CWwordToClearText(String cwword) {             // decode the Morse code character in cwword to clear text
  int ptr = 0;
  String result;
  result.reserve(40);
  String symbol;
  symbol.reserve(6);

  
  result = "";
  for (int i = 0; i < cwword.length(); ++i) {
      char c = cwword[i];
      switch (c) {
          case '1': ptr = CWtree[ptr].dit;
                    break;
          case '2': ptr = CWtree[ptr].dah;
                    break;
          case '0': symbol = CWtree[ptr].symb;

                    ptr = 0;
                    result += symbol;
                    break;
      }
  }
  symbol = CWtree[ptr].symb;
  //Serial.println("Symbol: " + symbol + " ptr: " + String(ptr));
  result += symbol;
  return encodeProSigns(result);
}


String encodeProSigns( String &input ) {
    /// clean up clearText   -   S <as>,  - A <ka> - N <kn> - K <sk> - H ch - V <ve>;
    input.replace("<as>", "S");
    input.replace("<ka>","A");
    input.replace("<kn>","N");
    input.replace("<sk>","K");
    input.replace("<ve>","V");
    input.replace("<ch>","H");
    input.replace("<err>","E");
    input.replace("¬", "U");
    //Serial.println(input);
    return input;
}


//// new buffer code: unpack when needed, to save buffer space. We just use 256 bytes of buffer, instead of 32k! 
//// in addition to the received packet, we need to store the RSSI as 8 bit positive number 
//// (it is always between -20 and -150, so an 8bit integer is fine as long as we store it without sign as an unsigned number)
//// the buffer is a 256 byte ring buffer with two pointers:
////   nextBuRead where the next packet starts for reading it out; is incremented by l to get the next buffer read position
////      you can read a packet as long as the buffer is not empty, so we need to check bytesBuFree before we read! if it is 256, the buffer is empty!
////      with a read, the bytesBuFree has to be increased by the number of bytes read
////   nextBuWrite where the next packet should be written; @write:
////       increment nextBuWrite by l to get new pointer; and decrement bytesBuFree by l to get new free space
//// we also need a variable that shows how many bytes are free (not in use): bytesBuFree
//// if the next packet to be stored is larger than bytesBuFree, it is discarded
//// structure of each packet:
////    l:  1 uint8_t length of packet
////    r:  1 uint8_t rssi as a positive number
////    d:  (var. length) data packet as received by LoRa
//// functions:
////    int loRaBuWrite(int rssi, String packet): returns length of buffer if successful. otherwise 0
////    uint8_t loRaBuRead(uint8_t* buIndex): returns length of packet, and index where to read in buffer by reference
////    boolean loRaBuReady():  true if there is something in the buffer, false otherwise
////      example:
////        (somewhere else as global var: ourBuffer[256]
////        uint8_t myIndex;
////        uint8_t mylength;
////        foo() {
////          myLength = loRaBuRead(&myIndex);
////          if (myLength != 0) 
////            doSomethingWith(ourBuffer[myIndex], myLength);
////        }


uint8_t loRaBuWrite(int rssi, String packet) {
////   int loRaBuWrite(int rssi, String packet): returns length of buffer if successful. otherwise 0
////   nextBuWrite where the next packet should be written; @write:
////       increment nextBuWrite by l to get new pointer; and decrement bytesBuFree by l to get new free space
  uint8_t l, posRssi;

  posRssi = (uint8_t) abs(rssi);
  l = 2 + packet.length();
  if (byteBuFree < l)                               // buffer full - discard packet
      return 0;
  loRaRxBuffer[nextBuWrite++] = l;
  loRaRxBuffer[nextBuWrite++] = posRssi;
  for (int i = 0; i < packet.length(); ++i) {       // do this for all chars in the packet
    loRaRxBuffer [nextBuWrite++] = packet[i];       // at end nextBuWrite is alread where it should be
  }
  byteBuFree -= l;
  //Serial.println(byteBuFree);
  //Serial.println((String)loRaRxBuffer[0]);
  return l;
}

boolean loRaBuReady() {
  if (byteBuFree == 256)
    return (false);
  else
    return true;
}


uint8_t loRaBuRead(uint8_t* buIndex) {
////    uint8_t loRaBuRead(uint8_t* buIndex): returns length of packet, and index where to read in buffer by reference
  uint8_t l;  
  if (byteBuFree == 256)
    return 0;
  else {
    l = loRaRxBuffer[nextBuRead++];
    *buIndex = nextBuRead;
    byteBuFree += l;
    --l;
    nextBuRead += l;
    return l;
  }
}




void storePacket(int rssi, String packet) {             // whenever we receive something, we just store it in our buffer
  if (loRaBuWrite(rssi, packet) == 0)
    Serial.println("LoRa Buffer full");
}


/// decodePacket analyzes packet as received and stored in buffer
/// returns the header byte (protocol version*64 + 6bit packet serial number
//// byte 0 (added by receiver): RSSI
//// byte 1: header; first two bits are the protocol version (curently 01), plus 6 bit packet serial number (starting from random)
//// byte 2: first 6 bits are wpm (must be between 5 and 60; values 00 - 04 and 61 to 63 are invalid), the remaining 2 bits are already data payload!


uint8_t decodePacket(int* rssi, int* wpm, String* cwword) {
  uint8_t l, c, header=0;
  uint8_t index = 0;

  l = loRaBuRead(&index);           // where are we in  the buffer, and how long is the total packet inkl. rssi byte?

  for (int i = 0; i < l; ++i) {     // decoding loop
    c = loRaRxBuffer[index+i];

    switch (i) {
      case  0:  * rssi = (int) (-1 * c);    // the rssi byte
                break;
      case  1:  header = c;
                break;
      case  2:  *wpm = (uint8_t) (c >> 2);  // the first data byte contains the wpm info in the first six bits, and actual morse in the remaining two bits
                                            // now take remaining two bits and store them in CWword as ASCII
                *cwword = (char) ((c & B011) +48); 
                break;
      default:                              // decode the rest of the packet; we do this for all but the first byte  /// we need to handle end of word!!! therefore the break
                for (int j = 0; j < 4; ++j) {
                    char cc = ((c >> 2*(3-j)) & B011) ;                // we store them as ASCII characters 0,1,2,3 !
                    if (cc != 3) {
                        *cwword  += (char) (cc + 48);
                    }
                    else break;
                }
                break;
    } // end switch
  }   // end for
  return header;
}      // end decodePacket

/////////////////////////////////////   MORSE DECODER ///////////////////////////////


////////////////////////////
///// Routines for morse decoder - to a certain degree based on code by Hjalmar Skovholm Hansen OZ1JHM - copyleft licence
////////////////////////////

//void setupMorseDecoder() {
//  /// here we will do the init for decoder mode
//  //trainerMode = false;
//  encoderState = volumeSettingMode;
//
//  display.clear();
//  printOnScroll(1, REGULAR, 0, "Start Decoder" );
//  delay(750);
//  display.clear();
//  displayTopLine();
//  drawInputStatus(false);
//  printToScroll(REGULAR,"");      // clear the buffer
//
//  speedChanged = true;
//  displayCWspeed();
//  displayVolume();
//
//  /// set up variables for Goertzel Morse Decoder
//  setupGoertzel();
//  filteredState = filteredStateBefore = false;
//  decoderState = LOW_;
//  ditAvg = 60;
//  dahAvg = 180;
//}

//const float sampling_freq = 106000.0;
//const float target_freq = 698.0; /// adjust for your needs see above
//const int goertzel_n = 304; //// you can use:         152, 304, 456 or 608 - thats the max buffer reserved in checktone()//


#define straightPin leftPin

boolean straightKey() {            // return true if a straight key was closed, or a touch paddle touched
if ((morseState == morseDecoder) && ((!digitalRead(straightPin)) || leftKey || rightKey) )
    return true;
else return false;
}

boolean checkTone() {              /// check if we have a tone signal at A6 with Gortzel's algorithm, and apply some noise blanking as well
                                   /// the result will be in globale variable filteredState
                                   /// we return true when we detected a change in state, false otherwise!
  
  float magnitude ;

  static boolean realstate = false;
  static boolean realstatebefore = false;
  static unsigned long lastStartTime = 0;
  
  uint16_t testData[1216];         /// buffer for freq analysis - max. 608 samples; you could increase this (and n) to a max of 1216, for sample time 10 ms, and bw 88 Hz

///// check straight key first before you check audio in.... (unless we are in transceiver mode)
///// straight key is connected to external paddle connector (tip), i.e. the same as the left pin (dit normally)

if (straightKey() ) {
    realstate = true;
    //Serial.println("Straight Key!");
    //keyTx = true;
    }
else {
    realstate = false;
    //keyTx = false;
    for (int index = 0; index < goertzel_n ; index++)
        testData[index] = analogRead(audioInPin);
    //Serial.println("Read and stored analog values!");
    for (int index = 0; index < goertzel_n ; index++) {
      float Q0;
      Q0 = coeff * Q1 - Q2 + (float) testData[index];
      Q2 = Q1;
      Q1 = Q0;
    }
    //Serial.println("Calculated Q1 and Q2!");

    float magnitudeSquared = (Q1 * Q1) + (Q2 * Q2) - (Q1 * Q2 * coeff); // we do only need the real part //
    magnitude = sqrt(magnitudeSquared);
    Q2 = 0;
    Q1 = 0;
  
   //Serial.println("Magnitude: " + String(magnitude) + " Limit: " + String(magnitudelimit));   //// here you can measure magnitude for setup..
  
    ///////////////////////////////////////////////////////////
    // here we will try to set the magnitude limit automatic //
    ///////////////////////////////////////////////////////////
  
    if (magnitude > magnitudelimit_low) {
      magnitudelimit = (magnitudelimit + ((magnitude - magnitudelimit) / 6)); /// moving average filter
    }
  
    if (magnitudelimit < magnitudelimit_low)
      magnitudelimit = magnitudelimit_low;
  
    ////////////////////////////////////
    // now we check for the magnitude //
    ////////////////////////////////////
  
    if (magnitude > magnitudelimit * 0.6) // just to have some space up
      realstate = true;
    else
      realstate = false;
  }



  /////////////////////////////////////////////////////
  // here we clean up the state with a noise blanker //
  // (debouncing)                                    //
  /////////////////////////////////////////////////////

  if (realstate != realstatebefore)
    lastStartTime = millis();
  if ((millis() - lastStartTime) > nbtime) {
    if (realstate != filteredState) {
      filteredState = realstate;
    }
  }
  realstatebefore = realstate;

 if (filteredState == filteredStateBefore)
  return false;                                 // no change detected in filteredState
 else {
    filteredStateBefore = filteredState;
    return true;                                // change detected in filteredState
 }
}   /// end checkTone()


void doDecode() {
  float lacktime;
  int wpm;

    switch(decoderState) {
      case INTERELEMENT_: if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          } else {
                              lowDuration = millis() - startTimeLow;                        // we record the length of the pause
                              lacktime = 2.2;                                               ///  when high speeds we have to have a little more pause before new letter 
                              //if (p_wpm > 35) lacktime = 2.7;
                              //  else if (p_wpm > 30) lacktime = 2.6;
                              if (lowDuration > (lacktime * ditAvg)) {
                                displayMorse();                                             /// decode the morse character and display it
                                wpm = (p_wpm + (int) (7200 / (dahAvg + 3*ditAvg))) / 2;     //// recalculate speed in wpm
                                if (p_wpm != wpm) {
                                  p_wpm = wpm;
                                  speedChanged = true;
                                }
                                decoderState = INTERCHAR_;
                              }
                          }
                          break;
      case INTERCHAR_:    if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          } else {
                              lowDuration = millis() - startTimeLow;             // we record the length of the pause
                              lacktime = 5;                 ///  when high speeds we have to have a little more pause before new word
                              if (p_wpm > 35) lacktime = 6;
                                else if (p_wpm > 30) lacktime = 5.5;
                              if (lowDuration > (lacktime * ditAvg)) {
                                   printToScroll(REGULAR, " ");                       // output a blank                                
                                   decoderState = LOW_;
                              }
                          }
                          break;
      case LOW_:          if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          }
                          break;
      case HIGH_:         if (checkTone()) {
                              OFF_();
                              decoderState = INTERELEMENT_;
                          }
                          break;
    } 
}

void ON_() {                                  /// what we do when we just detected a rising flank, from low to high
   unsigned long timeNow = millis();
   lowDuration = timeNow - startTimeLow;             // we record the length of the pause
   startTimeHigh = timeNow;                          // prime the timer for the high state

   keyOut(true, false, notes[p_sidetoneFreq], p_sidetoneVolume);

   drawInputStatus(true);
   
   if (lowDuration < ditAvg * 2.4)                    // if we had an inter-element pause,
      recalculateDit(lowDuration);                    // use it to adjust speed
}

void OFF_() {                                 /// what we do when we just detected a falling flank, from high to low
  unsigned long timeNow = millis();
  unsigned int threshold = (int) ( ditAvg * sqrt( dahAvg / ditAvg));

  //Serial.print("threshold: ");
  //Serial.println(threshold);
  highDuration = timeNow - startTimeHigh;
  startTimeLow = timeNow;

  if (highDuration > (ditAvg * 0.5) && highDuration < (dahAvg * 2.5)) {    /// filter out VERY short and VERY long highs
      if (highDuration < threshold) { /// we got a dit -
            treeptr = CWtree[treeptr].dit;
            //Serial.print(".");
            recalculateDit(highDuration);
      }
      else  {        /// we got a dah
            treeptr = CWtree[treeptr].dah;   
            //Serial.print("-");   
            recalculateDah(highDuration);                 
      }
  }
  //pwmNoTone();                     // stop side tone
  //digitalWrite(keyerPin, LOW);      // stop keying Tx
  keyOut(false, false, 0, 0);
  ///////
  drawInputStatus(false);

}

void drawInputStatus( boolean on) {
  if (on)
    display.setColor(BLACK);
  else
      display.setColor(WHITE);
  display.fillRect(1, 1, 20, 13);   
  display.display();
}



void recalculateDit(unsigned long duration) {       /// recalculate the average dit length
  ditAvg = (4*ditAvg + duration) / 5;
  //Serial.print("ditAvg: ");
  //Serial.println(ditAvg);
  //nbtime =ditLength / 5; 
  nbtime = constrain(ditAvg/5, 7, 20);
  //Serial.println(nbtime);
}

void recalculateDah(unsigned long duration) {       /// recalculate the average dah length
  //static uint8_t rot = 0;
  //static unsigned long collector;

  if (duration > 2* dahAvg)   {                       /// very rapid decrease in speed!
      dahAvg = (dahAvg + 2* duration) / 3;            /// we adjust faster, ditAvg as well!
      ditAvg = ditAvg/2 + dahAvg/6;
  }
  else { 
      dahAvg = (3* ditAvg + dahAvg + duration) / 3;
  }
    //Serial.print("dahAvg: ");
    //Serial.println(dahAvg);
    
}


void keyOut(boolean on,  boolean fromHere, int f, int volume) {                                      
  //// generate a side-tone with frequency f when on==true, or turn it off
  //// differentiate external (decoder, sometimes cw_generate) and internal (keyer, sometimes Cw-generate) side tones
  //// key transmitter (and line-out audio if we are in a suitable mode)

  static boolean intTone = false;
  static boolean extTone = false;

  static int intPitch, extPitch;

// Serial.println("keyOut: " + String(on) + String(fromHere));
  if (on) {
      if (fromHere) {
        intPitch = f;
        intTone = true;
        pwmTone(intPitch, volume, true);
        keyTransmitter();
      } else {                    // not from here
        extTone = true;
        extPitch = f;
        if (!intTone) 
          pwmTone(extPitch, volume, false);
        }
  } else {                      // key off
        if (fromHere) {
          intTone = false;
          if (extTone)
            pwmTone(extPitch, volume, false);
          else
            pwmNoTone();
          digitalWrite(keyerPin, LOW);      // stop keying Tx
        } else {                 // not from here
          extTone = false;
          if (!intTone)
            pwmNoTone();
        }
  }   // end key off
}

///////////////// a test function for adjusting audio levels

void audioLevelAdjust() {
    uint16_t i, maxi, mini;
    uint16_t testData[1216];

    display.clear();
    printOnScroll(0, BOLD, 0, "Audio Adjust");
    printOnScroll(1, REGULAR, 0, "End with RED");
    keyTx = true;
    keyOut(true,  true, 698, 0);                                  /// we generate a side tone, f=698 Hz, also on line-out, but with vol down on speaker
    while (true) {
        volButton.Update();
        if (volButton.clicks)
            break;                                                /// pressing the red button gets you out of this mode!
        for (i = 0; i < goertzel_n ; ++i)
            testData[i] = analogRead(audioInPin);                 /// read analog input
        maxi = mini = testData[0];
        for (i = 1; i< goertzel_n ; ++i) {
            if (testData[i] < mini)
              mini = testData[i];
            if (testData[i] > maxi)
              maxi = testData[i];
        }
        int a, b, c;
        a = map(mini, 0, 4096, 0, 125);
        b = map(maxi, 0, 4000, 0, 125);
        c = b - a;
        clearLine(2);
        display.drawRect(5, SCROLL_TOP + 2 * LINE_HEIGHT +5, 102, LINE_HEIGHT-8);
        display.drawRect(30, SCROLL_TOP + 2 * LINE_HEIGHT +5, 52, LINE_HEIGHT-8);
        display.fillRect(a, SCROLL_TOP + 2 * LINE_HEIGHT + 7 , c, LINE_HEIGHT -11);
        display.display();
    } // end while
    keyOut(false,  true, 698, 0);                                  /// stop keying
    keyTx = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// stuff using WiFi - ask for access point credentials, upload player file, do OTA software update
///////////////////////////////////////////////////////////////////////////////////////////////////

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void startAP() {
  //IPaddress a;
  WiFi.mode(WIFI_AP);
  WiFi.setHostname(ssid);
  WiFi.softAP(ssid);
  //a = WiFi.softAPIP();
  display.clear();
  printOnStatusLine(true, 0,    "Enter Wifi Info @");
  printOnScroll(0, REGULAR, 0,  "AP: morserino");
  printOnScroll(1, REGULAR, 0,  "URL: m32.local");
  printOnScroll(2, REGULAR, 0,  "RED to abort");

  //printOnScroll(2, REGULAR, 0, WiFi.softAPIP().toString());
  //Serial.println(WiFi.softAPIP());

  startMDNS();
  
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", myForm);
  });
  
  server.on("/set", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", "Wifi Info updated - now restarting Morserino-32...");
    p_wlanSSID = server.arg("ssid");
    p_wlanPassword = server.arg("pw");
    //Serial.println("SSID: " + p_wlanSSID + " Password: " + p_wlanPassword);
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putString("wlanSSID", p_wlanSSID);
    pref.putString("wlanPassword", p_wlanPassword);
    pref.end();
    
    ESP.restart();
  });
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  while (true) {
      server.handleClient();
      delay(20);
      volButton.Update();
      if (volButton.clicks) {
        display.clear();
        printOnStatusLine(true, 0, "Resetting now...");
        delay(2000);
        ESP.restart();
      }
  }
}


void updateFirmware()   {                   /// start wifi client, web server and upload new binary from a local computer
  if (! wifiConnect())
    return;
 
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateLoginIndex);
  });
  
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      //Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  //Serial.println("Starting web server");
  server.begin();
  display.clear();
  printOnStatusLine(true, 0, "Waiting f. Update ");
  printOnScroll(0, REGULAR, 0,  "URL: m32.local");
  printOnScroll(1, REGULAR, 0,  "IP:");
  printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());
  while(true) {
    server.handleClient();
    delay(10);
  }
}
  

boolean wifiConnect() {                   // connect to local WLAN
  // Connect to WiFi network
  if (p_wlanSSID == "") 
      return errorConnect(String("WiFi Not Conf"));
    
  WiFi.begin(p_wlanSSID.c_str(), p_wlanPassword.c_str());

  // Wait for connection
  long unsigned int wait = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if ((millis() - wait) > 20000)
      return errorConnect(String("No WiFi:"));
  }
  //Serial.print("Connected to ");
  //Serial.println(p_wlanSSID);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  startMDNS();
  return true;
}

boolean errorConnect(String msg) {
  display.clear();
  printOnStatusLine(true, 0, "Not connected");
  printOnScroll(0, INVERSE_BOLD, 0, msg);
  printOnScroll(1, REGULAR, 0, p_wlanSSID);
  delay(3500);
  return false;
}

void startMDNS() {
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://m32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
      if (MDNS.begin(host))
        break;
    }
  }
  //Serial.println("mDNS responder started");
}

void uploadFile() {
  if (! wifiConnect())
    return;
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", uploadLoginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

server.on("/update", HTTP_POST,                       // if the client posts to the upload page
    [](){ server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
    ESP.restart();},                                  // Send status 200 (OK) to tell the client we are ready to receive; when done, restart the ESP32
    handleFileUpload                                    // Receive and save the file
  );
  
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();                           // Actually start the server
  //Serial.println("HTTP server started");
  display.clear();
  display.clear();
  printOnStatusLine(true, 0, "Waiting f. Upload ");
  printOnScroll(0, REGULAR, 0,  "URL: m32.local");
  printOnScroll(1, REGULAR, 0,  "IP:");
  printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());  
  while(true) {
    server.handleClient();
    //delay(5);
  }
}


String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {     // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                            // If there's a compressed version available
      path += ".gz";                                          // Use the compressed verion
    File file = SPIFFS.open(path, "r");                       // Open the file
    server.streamFile(file, contentType);                     // Send it to the client
    file.close();                                             // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  //Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    //Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open("/player.txt", "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      p_fileWordPointer = 0;                              // reset word counter for file player
      pref.begin("morserino", false);              // open the namespace as read/write
          pref.putUInt("fileWordPtr", p_fileWordPointer);
      pref.end(); 

      //Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}


String getWord() {
  String result = "";
  byte c;
  
  while (file.available()) {
      c=file.read();
      //Serial.println((int) c);
      if (!isSpace(c))
        result += (char) c;
      else if (result.length() > 0)    {               // end of word
        ++p_fileWordPointer;
        //Serial.println("word: " + result);
        return result;
      }
    }
    file.close(); file = SPIFFS.open("/player.txt");
    p_fileWordPointer = 0;
    while (!file.available())
      ;
    return result;                                    // at eof
}

String cleanUpText(String w) {                        // all to lower case, and convert umlauts
  String result = "";
  char c;
  result.reserve(64);
  w.toLowerCase();
  w = utf8umlaut(w);
  
  for (unsigned int i = 0; i<w.length(); ++i) {
    if (kochChars.indexOf(c = w.charAt(i)) != -1)
      result += c;
  }
  return result;
}


String utf8umlaut(String s) { /// replace umtf umlauts with digraphs, and interpret pro signs, written e.g. as [kn] or <kn>
      s.replace("ä", "ae");
      s.replace("ö", "oe");
      s.replace("ü", "ue");
      s.replace("Ä", "ae");
      s.replace("Ö", "oe");
      s.replace("Ü", "ue");
      s.replace("ß", "ss");
      s.replace("[", "<");
      s.replace("]", ">");
      s.replace("<ar>", "+");
      s.replace("<bt>", "=");
      s.replace("<as>", "S");
      s.replace("<ka>", "K");
      s.replace("<kn>", "N");
      s.replace("<sk>", "K");
      s.replace("<ve>", "V");
      return s;
}

void skipWords(uint32_t count) {             /// just skip count words in open file fn
  while (count > 0) {
    getWord();
    --count;
  }
}

