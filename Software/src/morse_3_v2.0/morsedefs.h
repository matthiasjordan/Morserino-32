#ifndef MORSEDEFS_H
#define MORSEDEFS_H

//////// Program Version
#define BETA false

///// its is crucial to have the right board version - Boards 2 and 2a (prototypes only) set it to 2, Boards 3 set it to 3
///// the Board Version 2 is for HEltec Modules V1 only, Board Version 3 for Heltec V2 only
///// Board version 1 not supported anymore!

#define BOARDVERSION  3

///////////////////////
/////// protocol version for Lora - for the time being this is B01
/////// the first version of the CW over LoRA protocol; future versions will be B02, B03, B00 (reserved for future use)

#define CWLORAVERSION B01

#define VERSION_MAJOR 2
#define VERSION_MINOR 0

enum DISPLAY_TYPE
{
    NO_DISPLAY, DISPLAY_BY_CHAR, DISPLAY_BY_WORD
};
// how we display in trainer mode
enum random_OPTIONS
{
    OPT_ALL, OPT_ALPHA, OPT_NUM, OPT_PUNCT, OPT_PRO, OPT_ALNUM, OPT_NUMPUNCT, OPT_PUNCTPRO, OPT_ALNUMPUNCT, OPT_NUMPUNCTPRO, OPT_KOCH
};
enum PROMPT_TYPE
{
    NO_PROMPT, CODE_ONLY, DISP_ONLY, CODE_AND_DISP
};

enum FONT_ATTRIB
{
    REGULAR, BOLD, INVERSE_REGULAR, INVERSE_BOLD
};

//OLED pins to ESP32 GPIOs:
const int OLED_SDA = 4;
const int OLED_SCL = 15;
const int OLED_RST = 16;

///////////// Some GLOBAL defines

// SENS_FACTOR is used for auto-calibrating sensitivity of touch paddles (somewhere between 2.0 and 2.5)
#define SENS_FACTOR 2.22

///////////////////////////////      H A R D W A R E      ///////////////////////////////////////////
//// Here are the definitions for the various hardware-related I/O pins of the ESP32
/////////////////////////////////////////////////////////////////////////////////////////////////////
///// Board versions:
///// 2(a): for Heltec ESP32 LORA Version 1 (Morserino-32 prototypes)
///// 3: for heltec ESP32 LORA Version 2
///// Warning: Board version 1 not supported anymore!!!! (was an early prototype)
/////
///// the following Pins are dependent on the board version
///// 21:  V2: an encoder port, using interrupts      V3: Vext - used internally to switch Vext on and off
///// 39:  V2: ADC input to measure battery voltage   V3: encoder port (interrupt driven) instead of 21
///// 13:  V2: leftPin (external paddle, ext. pullup) V3: used internally to read battery voltage
///// 38:  V2: not in use                             V3: encoder port (interrupt driven) instead of 35
///// 32:  V2: used for LoRa internally               V3: leftPin
///// 33:  V2: used for LoRa internally               V3: rightPin
///// 34: rightPin                                    V3: cannot be used any longer, used for LoRa internally
///// 35: PinDT                                       V3: cannot be used any longer, used for LoRa internally

#ifndef BOARDVERSION
#error "You need to define a board version  at the beginning of the source file!"
#endif

/////// here are the board dependent pins definitions

#if BOARDVERSION == 2

/// where are the external paddles?
const int leftPin = 13;// external pullup resistor is necessary for closing contacts!
const int rightPin = 34;// external pullup resistor is necessary for closing contacts!

/// where is the encoder?
const int PinCLK=21;// Used for generating interrupts using CLK signal - needs external pullup resisitor!
const int PinDT=35;// Used for reading DT signal  - needs external pullup resisitor!

// input for battery voltage control
const int batteryPin = 39;

#elif BOARDVERSION == 3

/// where are the external paddles?
const int leftPin = 33;   // external pullup resistor is necessary for closing contacts!
const int rightPin = 32;  // external pullup resistor is necessary for closing contacts!

/// where is the encoder?
const int PinCLK = 38;                   // Used for generating interrupts using CLK signal - needs external pullup resisitor!
const int PinDT = 39;                    // Used for reading DT signal  - needs external pullup resisitor!

// input for battery voltage control
const int batteryPin = 13;

// pin to switch ON Vext
//const int Vext = 21;

#else
#error "Invalid Board Version! This program version only supports board versions 2 and 3."
#endif

//////// HARDWARE definitions that have not been changed between board version2 and 3 (ESP32 WIFI LORA V.1 and V.2)

// Pin definition of WIFI LoRa 32
// HelTec AutoMation 2017 support@heltec.cn
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

// #define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6
#define PABOOST true

//// with the following we define which pins are used as output for the two pwm channels
//// HF output (with varying duticycle and fixed frequency) and LF output (with varying frequency and fixed dutycycle of 50%)
/// are being added with a 2-transistor AND gate to create a tone frequency with variable frequency and volume

const int LF_Pin = 23;    // for the lower (= NF) frequency generation
const int HF_Pin = 22;    // for the HF PWM generation

/// where are the touch paddles?
const int LEFT = T2;        // = Pin 2
const int RIGHT = T5;       // = Pin 12

/// 2nd switch button - toggles between Speed control and Volume control
const int volButtonPin = 0;

// Tx keyer
const int keyerPin = 25;        // this keys the transmitter / through a MOSFET Optocoupler - at the same time lights up the LED

// audio in
const int audioInPin = 36;      // audio in for Morse decoder //

// NF Line-out (for iCW etc.)
const int lineOutPin = 17; // for NF line out

/// Switch button (on rotary encoder)
const int modeButtonPin = 37;    // input pin for mode button - needs external pullup!

///////////////////////////////////////// END OF HARDWARE DEFS ////////////////////////////////////////////////////////////////////

#endif
