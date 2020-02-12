#ifndef PREFS_H
#define PREFS_H

#include <Arduino.h>
#include <Preferences.h>   // ESP 32 library for storing things in non-volatile storage

#include "morsedefs.h"

namespace MorsePreferences
{

    enum prefPos
    {
        posClicks,
        posPitch,
        posExtPaddles,
        posPolarity,
        posCurtisMode,
        posCurtisBDahTiming,
        posCurtisBDotTiming,
        posACS,
        posEchoToneShift,
        posInterWordSpace,
        posInterCharSpace,
        posRandomOption,
        posRandomLength,
        posCallLength,
        posAbbrevLength,
        posWordLength,
        posTrainerDisplay,
        posWordDoubler,
        posEchoDisplay,
        posEchoRepeats,
        posEchoConf,
        posKeyTrainerMode,
        posLoraTrainerMode,
        posGoertzelBandwidth,
        posSpeedAdapt,
        posKochSeq,
        posKochFilter,
        posLatency,
        posRandomFile,
        posTimeOut,
        posQuickStart,
        posLoraSyncW,
        posLoraBand,
        posLoraQRG,
        posSnapRecall,
        posSnapStore,
        posMaxSequence,
        //
        sentinel
    };

    extern const String prefOption[];

    extern prefPos keyerOptions[];
    extern prefPos generatorOptions[];
    extern prefPos headOptions[];
    extern prefPos playerOptions[];
    extern prefPos echoPlayerOptions[];
    extern prefPos echoTrainerOptions[];
    extern prefPos kochGenOptions[];
    extern prefPos kochEchoOptions[];
    extern prefPos morseTennisOptions[];
    extern prefPos loraTrxOptions[];
    extern prefPos extTrxOptions[];
    extern prefPos decoderOptions[];
    extern prefPos allOptions[];
    extern prefPos noOptions[];

    class MorsePrefs
    {
        public:
            // the preferences variable and their defaults

            uint8_t version_major = VERSION_MAJOR;
            uint8_t version_minor = VERSION_MINOR;
            uint8_t sidetoneFreq = 11;               // side tone frequency                               1 - 15
            uint8_t sidetoneVolume = 60;              // side tone volume, as a value between 0 and 100   0 -100
            boolean didah = false;                    // paddle polarity                                  bool
            uint8_t keyermode = 2;                    // Iambic keyer mode: see the #defines above        1 -  3
            uint8_t interCharSpace = 3;               // trainer: in dit lengths                          3 - 24
            boolean useExtPaddle = false;        // has now a different meaning: true when we need to reverse the polarity of the ext paddle
            uint8_t ACSlength = 0;                    // in ACS: we extend the pause between charcaters to the equal length of how many dots
                                                      // (2, 3 or 4 are meaningful, 0 means off) 0, 2-4
            boolean encoderClicks = true;             // all: should rotating the encoder generate a click?
            uint8_t randomLength = 3;                 // trainer: how many random chars in one group -    1 -  5
            uint8_t randomOption = 0;                 // trainer: from which pool are we generating random characters?  0 - 9
            uint8_t callLength = 0;                   // trainer: max length of call signs generated (0 = unlimited)    0, 3 - 6
            uint8_t abbrevLength = 0;                 // trainer: max length of abbreviations generated (0 = unlimited) 0, 2 - 6
            uint8_t wordLength = 0;                   // trainer: max length of english words generated (0 = unlimited) 0, 2 - 6
            uint8_t trainerDisplay = DISPLAY_BY_CHAR; // trainer: how we display what the trainer generates: nothing, by character, or by word  0 - 2
            uint8_t curtisBTiming = 45;               // keyer: timing for enhanced Curtis mode: dah                    0 - 100
            uint8_t curtisBDotTiming = 75;           // keyer: timing for enhanced Curtis mode: dit                    0 - 100
            uint8_t interWordSpace = 7;        // trainer: normal interword spacing in lengths of dit,           6 - 45 ; default = norm = 7

            uint8_t echoRepeats = 3;        // how often will echo trainer repeat an erroneously entered word? 0 - REPEAT_FOREVER, 7=forever, default = 3
            uint8_t echoDisplay = 1;                  //  1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP
            uint8_t kochFilter = 5;                   // constrain output to characters learned according to Koch's method 2 - 45
            boolean wordDoubler = false;              // in CW trainer mode only, repeat each word
            uint8_t echoToneShift = 1;                // 0 = no shift, 1 = up, 2 = down (a half tone)                   0 - 2
            boolean echoConf = true;                  // true if echo trainer confirms audibly too, not just visually
            uint8_t keyTrainerMode = 1;               // key a transmitter in generator and player mode?
                                                      //  0: "Never";  1: "CW Keyer only";  2: "Keyer&Generator";
            uint8_t loraTrainerMode = 0;              // transmit via LoRa in generator and player mode?
                                                      //  0: "No";  1: "yes"
            uint8_t goertzelBandwidth = 0;            //  0: "Wide" 1: "Narrow"
            boolean speedAdapt = false;               //  true: in echo modes, increase speed when OK, reduce when not ok
            uint8_t latency = 5; //  time span after currently sent element during which paddles are not checked; in 1/8th of dit length; stored as 1 -  8
            uint8_t randomFile = 0;             // if 0, play file word by word; if 255, skip random number of words (0 - 255) between reads
            boolean lcwoKochSeq = false;              // if true, replace native sequence with LCWO sequence
            uint8_t timeOut = 1;                      // time-out value: 4 = no timeout, 1 = 5 min, 2 = 10 min, 3 = 15 min
            boolean quickStart = false;               // should we start the last executed command immediately?
            uint8_t loraSyncW = 0x27;                 // allows to set different LoRa sync words, and so creating virtual "channels"

            ///// stored in preferences, but not adjustable through preferences menu:
            uint8_t responsePause = 5;         // in echoTrainer mode, how long do we wait for response? in interWordSpaces; 2-12, default 5
            uint8_t wpm = 15;                         // keyer speed in words per minute                  5 - 60
            uint8_t menuPtr = 1;                      // current position of menu
            String wlanSSID = "";                    // SSID for connecting to the Internet
            String wlanPassword = "";                // password for connecting to WiFi router
            uint32_t fileWordPointer = 0;             // remember how far we have read the file in player mode / reset when loading new file
            uint8_t promptPause = 2;          // in echoTrainer mode, length of pause before we send next word; multiplied by interWordSpace
            uint8_t tLeft = 20;                       // threshold for left paddle
            uint8_t tRight = 20;                      // threshold for right paddle

            uint8_t loraBand = 0;                     // 0 = 433, 1 = 868, 2 = 920
#define QRG433 434.15E6
#define QRG866 869.15E6
#define QRG920 920.55E6
            uint32_t loraQRG = QRG433;                // for 70 cm band

            uint8_t snapShots = 0;                    // keep track which snapshots are being used ( 0 .. 7, called 1 to 8)
            uint8_t maxSequence = 0;                  // max # of words generated beofre the Morserino pauses

            ////// end of variables stored in preferences

    };

    const uint8_t REPEAT_FOREVER = 7;

    /// variables for managing snapshots
    extern uint8_t memories[8];
    extern uint8_t memCounter;
    extern uint8_t memPtr;

    extern MorsePrefs prefs;

    extern prefPos *currentOptions;

    extern unsigned long charCounter; // we use this to count characters after changing speed - after n characters we decide to write the config into NVS

    MorsePrefs readPreferences(String repository);
    void writePreferences(String repository);
    boolean recallSnapshot();
    boolean storeSnapshot(uint8_t menu);
    void updateMemory(uint8_t temp);
    void clearMemory(uint8_t ptr);

    // Private
    uint8_t wordIsKoch(String thisWord);
    void createKochAbbr(uint8_t maxl, uint8_t koch);

    void writeLoRaPrefs(uint8_t loraBand, uint32_t loraQRG);
    void writeWordPointer();
    void writeVolume();
    void writeLastExecuted(uint8_t menuPtr);
    void writeWifiInfo(String SSID, String passwd);

    void fireCharSeen(boolean wpmOnly);
}

#endif
