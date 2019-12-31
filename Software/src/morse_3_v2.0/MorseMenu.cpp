#include <Arduino.h>

#include "MorseMenu.h"
#include "MorseSystem.h"
#include "koch.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include "MorseKeyer.h"
#include "MorseUI.h"
#include "MorseRotaryEncoder.h"
#include "MorseGenerator.h"


using namespace MorseMenu;

////// The MENU


//////// variables and constants for the modus menu


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
  MorseGenerator::GEN_TYPE generatorMode;
  boolean remember;
} menuItem_t;


const menuItem_t menuItems [] = {
  {"",_dummy, { 0,0,0,0,0}, MorseGenerator::NA, true},
  {"CW Keyer",_keyer, {0,_goToSleep,_gen,_dummy,0}, MorseGenerator::NA, true},

  {"CW Generator",_gen, {0,_keyer,_echo,_dummy,_genRand}, MorseGenerator::NA, true},
  {"Random",_genRand, {1,_genPlayer,_genAbb,_gen,0}, MorseGenerator::RANDOMS, true},
  {"CW Abbrevs",_genAbb, {1,_genRand,_genWords,_gen,0}, MorseGenerator::ABBREVS, true},
  {"English Words",_genWords, {1,_genAbb,_genCalls,_gen,0}, MorseGenerator::WORDS, true},
  {"Call Signs",_genCalls, {1,_genWords,_genMixed,_gen,0}, MorseGenerator::CALLS, true},
  {"Mixed",_genMixed, {1,_genCalls,_genPlayer,_gen,0}, MorseGenerator::MIXED, true},
  {"File Player",_genPlayer, {1,_genMixed,_genRand,_gen,0}, MorseGenerator::PLAYER, true},

  {"Echo Trainer",_echo, {0,_gen,_koch,_dummy,_echoRand}, MorseGenerator::NA, true},
  {"Random",_echoRand, {1,_echoPlayer,_echoAbb,_echo,0}, MorseGenerator::RANDOMS, true},
  {"CW Abbrevs",_echoAbb, {1,_echoRand,_echoWords,_echo,0}, MorseGenerator::ABBREVS, true},
  {"English Words",_echoWords, {1,_echoAbb,_echoCalls,_echo,0}, MorseGenerator::WORDS, true},
  {"Call Signs",_echoCalls, {1,_echoWords,_echoMixed,_echo,0}, MorseGenerator::CALLS, true},
  {"Mixed",_echoMixed, {1,_echoCalls,_echoPlayer,_echo,0}, MorseGenerator::MIXED, true},
  {"File Player",_echoPlayer, {1,_echoMixed,_echoRand,_echo,0}, MorseGenerator::PLAYER, true},

  {"Koch Trainer",_koch,  {0,_echo,_head,_dummy,_kochSel}, MorseGenerator::NA, true},
  {"Select Lesson",_kochSel, {1,_kochEcho,_kochLearn,_koch,0}, MorseGenerator::NA, true},
  {"Learn New Chr",_kochLearn, {1,_kochSel,_kochGen,_koch,0}, MorseGenerator::NA, true},
  {"CW Generator",_kochGen, {1,_kochLearn,_kochEcho,_koch,_kochGenRand}, MorseGenerator::NA, true},
  {"Random",_kochGenRand, {2,_kochGenMixed,_kochGenAbb,_kochGen,0}, MorseGenerator::RANDOMS, true},
  {"CW Abbrevs",_kochGenAbb, {2,_kochGenRand,_kochGenWords,_kochGen,0}, MorseGenerator::ABBREVS, true},
  {"English Words",_kochGenWords, {2,_kochGenAbb,_kochGenMixed,_kochGen,0}, MorseGenerator::WORDS, true},
  {"Mixed",_kochGenMixed, {2,_kochGenWords,_kochGenRand,_kochGen,0}, MorseGenerator::MIXED, true},

  {"Echo Trainer",_kochEcho, {1,_kochGen,_kochSel,_koch,_kochEchoRand}, MorseGenerator::NA, true},
  {"Random",_kochEchoRand, {2,_kochEchoMixed,_kochEchoAbb,_kochEcho,0}, MorseGenerator::RANDOMS, true},
  {"CW Abbrevs",_kochEchoAbb, {2,_kochEchoRand,_kochEchoWords,_kochEcho,0}, MorseGenerator::ABBREVS, true},
  {"English Words",_kochEchoWords, {2,_kochEchoAbb,_kochEchoMixed,_kochEcho,0}, MorseGenerator::WORDS, true},
  {"Mixed",_kochEchoMixed, {2,_kochEchoWords,_kochEchoRand,_kochEcho,0}, MorseGenerator::MIXED, true},

  {"Head Copying",_head, {0,_koch,_trx,_dummy,_headRand}, MorseGenerator::NA, true},
  {"Random",_headRand, {1,_headPlayer,_headAbb,_head,0}, MorseGenerator::RANDOMS, true},
  {"CW Abbrevs",_headAbb, {1,_headRand,_headWords,_head,0}, MorseGenerator::ABBREVS, true},
  {"English Words",_headWords, {1,_headAbb,_headCalls,_head,0}, MorseGenerator::WORDS, true},
  {"Call Signs",_headCalls, {1,_headWords,_headMixed,_head,0}, MorseGenerator::CALLS, true},
  {"Mixed",_headMixed, {1,_headCalls,_headPlayer,_head,0}, MorseGenerator::MIXED, true},
  {"File Player",_headPlayer, {1,_headMixed,_headRand,_head,0}, MorseGenerator::PLAYER, true},

  {"Transceiver",_trx, {0,_head,_decode,_dummy,_trxLora}, MorseGenerator::NA, true},
  {"LoRa Trx",_trxLora, {1,_trxIcw,_trxIcw,_trx,0}, MorseGenerator::NA, true},
  {"iCW/Ext Trx",_trxIcw, {1,_trxLora,_trxLora,_trx,0}, MorseGenerator::NA, true},

  {"CW Decoder",_decode, {0,_trx,_wifi,_dummy,0}, MorseGenerator::NA, true},

  {"WiFi Functions",_wifi, {0,_decode,_goToSleep,_dummy,_wifi_mac}, MorseGenerator::NA, false},
  {"Disp MAC Addr",_wifi_mac, {1,_wifi_update,_wifi_config,_wifi,0}, MorseGenerator::NA, false},
  {"Config WiFi",_wifi_config, {1,_wifi_mac,_wifi_check,_wifi,0}, MorseGenerator::NA, false},
  {"Check WiFi",_wifi_check, {1,_wifi_config,_wifi_upload,_wifi,0}, MorseGenerator::NA, false},
  {"Upload File",_wifi_upload, {1,_wifi_check,_wifi_update,_wifi,0}, MorseGenerator::NA, false},
  {"Update Firmw",_wifi_update, {1,_wifi_upload,_wifi_mac,_wifi,0}, MorseGenerator::NA, false},

  {"Go To Sleep",_goToSleep, {0,_wifi,_keyer,_dummy,0}, MorseGenerator::NA, false}

};


boolean quickStart;                                     // should we execute menu item immediately?



namespace MorseMenu::internal {
    void displayCurtisMode();
    void displayCurtisBTiming();
    void displayCurtisBDotTiming();
    void displayACS();
    void displayPitch();
    void displayClicks();
    void displayExtPaddles();
    void displayPolarity();
    void displayLatency();
    void displayInterWordSpace();
    void displayInterCharSpace();
    void displayRandomOption();
    void displayRandomLength();
    void displayCallLength();
    void displayAbbrevLength();
    void displayWordLength();
    void displayMaxSequence();
    void displayTrainerDisplay();
    void displayEchoDisplay();
    void displayKeyTrainerMode();
    void displayLoraTrainerMode();
    void displayLoraSyncW();
    void displayEchoRepeats();
    void displayEchoToneShift();
    void displayEchoConf();
    void displayKochFilter();
    void displayWordDoubler();
    void displayRandomFile();
    void displayGoertzelBandwidth();
    void displaySpeedAdapt();
    void displayKochSeq();
    void displayTimeOut();
    void displayQuickStart();
    void displayLoraBand();
    void displayLoraQRG();
    void displaySnapRecall();
    void displaySnapStore();

    void menuDisplay(uint8_t ptr);
    boolean menuExec();
}



void MorseMenu::menu_() {
   uint8_t newMenuPtr = MorsePreferences::prefs.menuPtr;
   uint8_t disp = 0;
   int t, command;

   quickStart = MorsePreferences::prefs.quickStart;

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
    MorseDisplay::clearScroll();                  // clear the buffer
    MorseDisplay::clearScrollBuffer();

    MorseGenerator::keyOut(false, true, 0, 0);
    MorseGenerator::keyOut(false, false, 0, 0);
    encoderState = speedSettingMode;             // always start with this encoderstate (decoder will change it anyway)
    currentOptions = allOptions;                 // this is the array of options when we double click the BLACK button: while in menu, you can change all of them
    currentOptionSize = SizeOfArray(allOptions);

    MorsePreferences::updateWordPointer();

    MorseDisplay::clear();

    while (true) {                          // we wait for a click (= selection)
        if (disp != newMenuPtr) {
          disp = newMenuPtr;
          internal::menuDisplay(disp);
        }
        if (quickStart) {
            quickStart = false;
            command = 1;
            delay(500);
            MorseDisplay::printOnScrollFlash(2, REGULAR, 1, "QUICK START");
        }
        else {
            modeButton.Update();
            command = modeButton.clicks;
        }

        switch (command) {                                          // actions based on enocder button
          case 2: if (setupPreferences(newMenuPtr))                       // all available options when called from top menu
                    newMenuPtr = MorsePreferences::prefs.menuPtr;
                  menuDisplay(newMenuPtr);
                  break;
          case 1: // check if we have a submenu or if we execute the selection
                  //Serial.println("newMP: " + String(newMenuPtr) + " navi: " + String(menuNav[newMenuPtr][naviDown]));
                  if (menuItems[newMenuPtr].nav[naviDown] == 0) {
                      MorsePreferences::prefs.menuPtr = newMenuPtr;
                      disp = 0;
                      if (menuItems[MorsePreferences::prefs.menuPtr].remember) {            // remember last executed, unless it is a wifi function or shutdown
                          pref.begin("morserino", false);             // open the namespace as read/write
                          pref.putUChar("lastExecuted", MorsePreferences::prefs.menuPtr);   // store last executed command
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
          //pwmClick(MorsePreferences::prefs.sidetoneVolume);         /// click
          newMenuPtr =  menuItems[newMenuPtr].nav[(t == -1) ? naviLeft : naviRight];
       }

       volButton.Update();

       switch (volButton.clicks) {
          case -1:  audioLevelAdjust();                         /// for adjusting line-in audio level (at the same time keying tx and sending oudio on line-out
                    MorseDisplay::clear();
                    menuDisplay(disp);
                    break;
          /* case  3:  wifiFunction();                                  /// configure wifi, upload file or firmware update
                    break;
          */
       }
       MorseSystem::checkShutDown(false);                  // check for time out
  } // end while - we leave as soon as the button has been pressed
} // end menu_()


void MorseMenu::internal::menuDisplay(uint8_t ptr) {
  //Serial.println("Level: " + (String) menuItems[ptr].nav[naviLevel] + " " + menuItems[ptr].text);
  uint8_t oneUp = menuItems[ptr].nav[naviUp];
  uint8_t twoUp = menuItems[oneUp].nav[naviUp];
  uint8_t oneDown = menuItems[ptr].nav[naviDown];

  MorseDisplay::printOnStatusLine(true, 0,  "Select Modus:     ");

  //MorseDisplay::clearLine(0); MorseDisplay::clearLine(1); MorseDisplay::clearLine(2);                       // delete previous content
  MorseDisplay::clearScroll();

  /// level 0: top line, possibly ".." on line 1
  /// level 1: higher level on 0, item on 1, possibly ".." on 2
  /// level 2: higher level on 1, highest level on 0, item on 2
  switch (menuItems[ptr].nav[naviLevel]) {
    case 2: {MorseDisplay::printOnScroll(2, BOLD, 0, menuItems[ptr].text);
    MorseDisplay::printOnScroll(1, REGULAR, 0, menuItems[oneUp].text);
    MorseDisplay::printOnScroll(0, REGULAR, 0, menuItems[twoUp].text);
            break;
    }
    case 1: {if (oneDown) {
        MorseDisplay::printOnScroll(2, REGULAR, 0, String(".."));
    }
    MorseDisplay::printOnScroll(1, BOLD, 0, menuItems[ptr].text);
    MorseDisplay::printOnScroll(0, REGULAR, 0, menuItems[oneUp].text);
    }
            break;
    case 0: {
            if (oneDown) {
                MorseDisplay::printOnScroll(1, REGULAR, 0, String(".."));
            }
            MorseDisplay::printOnScroll(0, BOLD, 0, menuItems[ptr].text);
    }
            break;
  }
}

///////////// GEN_TYPE { RANDOMS, ABBREVS, WORDS, CALLS, MIXED, KOCH_MIXED, KOCH_LEARN };           // the things we can generate in generator mode




boolean MorseMenu::internal::menuExec() {                                          // return true if we should  leave menu after execution, true if we should stay in menu
  //Serial.println("Executing menu item " + String(MorsePreferences::prefs.menuPtr));

  uint32_t wcount = 0;

  effectiveAutoStop = false;
  effectiveTrainerDisplay = MorsePreferences::prefs.trainerDisplay;

  kochActive = false;
  switch (MorsePreferences::prefs.menuPtr) {
    case  _keyer:  /// keyer
                currentOptions = keyerOptions;                // list of available options in keyer mode
                currentOptionSize = SizeOfArray(keyerOptions);
                morseState = morseKeyer;
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start CW Keyer" );
                delay(500);
                MorseDisplay::clear();
                displayTopLine();
                MorseDisplay::printToScroll(REGULAR,"");      // clear the buffer
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
                //skip MorsePreferences::prefs.fileWordPointer words, as they have been played before
                wcount = MorsePreferences::prefs.fileWordPointer;
                MorsePreferences::prefs.fileWordPointer = 0;
                MorsePlayerFile::skipWords(wcount);

     startTrainer:
                generatorMode = menuItems[MorsePreferences::prefs.menuPtr].generatorMode;
                startFirst = true;
                firstTime = true;
                morseState = morseGenerator;
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(0, REGULAR, 0, "Generator     ");
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start/Stop:   ");
                MorseDisplay::printOnScroll(2, REGULAR, 0, "Paddle | BLACK");
                delay(1250);
                MorseDisplay::clear();
                displayTopLine();
                MorseDisplay::clearScroll();      // clear the buffer
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
                generatorMode = menuItems[MorsePreferences::prefs.menuPtr].generatorMode;
                goto startEcho;
      case  _echoPlayer:    /// echo trainer
                generatorMode = menuItems[MorsePreferences::prefs.menuPtr].generatorMode;
                currentOptions = echoPlayerOptions;                         // list of available options in echo player mode
                currentOptionSize = SizeOfArray(echoPlayerOptions);
                file = SPIFFS.open("/player.txt");                            // open file
                //skip MorsePreferences::prefs.fileWordPointer words, as they have been played before
                wcount = MorsePreferences::prefs.fileWordPointer;
                MorsePreferences::prefs.fileWordPointer = 0;
                MorsePlayerFile::skipWords(wcount);
       startEcho:
                startFirst = true;
                morseState = echoTrainer;
                echoStop = false;
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(0, REGULAR, 0, generatorMode == KOCH_LEARN ? "New Character:" : "Echo Trainer:");
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start:       ");
                MorseDisplay::printOnScroll(2, REGULAR, 0, "Press paddle ");
                delay(1250);
                MorseDisplay::clear();
                displayTopLine();
                MorseDisplay::printToScroll(REGULAR,"");      // clear the buffer
                keyTx = false;
                return true;
                break;
      case  _kochSel: // Koch Select
                displayKeyerPreferencesMenu(posKochFilter);
                adjustKeyerPreference(posKochFilter);
                writePreferences("morserino");
                //createKochWords(MorsePreferences::prefs.wordLength, MorsePreferences::prefs.kochFilter) ;  // update the arrays!
                //createKochAbbr(MorsePreferences::prefs.abbrevLength, MorsePreferences::prefs.kochFilter);
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
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start LoRa Trx" );
                delay(600);
                MorseDisplay::clear();
                displayTopLine();
                MorseDisplay::printToScroll(REGULAR,"");      // clear the buffer
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
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start CW Trx" );
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
                MorseDisplay::clear();
                MorseDisplay::printOnScroll(1, REGULAR, 0, "Start Decoder" );
      setupDecoder:
                speedChanged = true;
                delay(650);
                MorseDisplay::clear();
                displayTopLine();
                drawInputStatus(false);
                MorseDisplay::printToScroll(REGULAR,"");      // clear the buffer

                displayCWspeed();
                MorseDisplay::displayVolume();

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
                MorseDisplay::clearDisplay();
                MorseDisplay::printOnStatusLine(true, 0,  WiFi.macAddress());
                delay(2000);
                MorseDisplay::printOnScroll(0, REGULAR, 0, "RED: restart" );
                delay(1000);
                while (true) {
                  MorseSystem::checkShutDown(false);  // possibly time-out: go to sleep
                  if (digitalRead(volButtonPin) == LOW)
                    ESP.restart();
                }
                break;
      case  _wifi_config:
                startAP();          // run as AP to get WiFi credentials from user
                break;
      case _wifi_check:
                MorseDisplay::clearDisplay();
                MorseDisplay::printOnStatusLine(true, 0,  "Connecting... ");
                if (! wifiConnect())
                    ; //return false;
                else {
                    MorseDisplay::printOnStatusLine(true, 0,  "Connected!    ");
                    MorseDisplay::printOnScroll(0, REGULAR, 0, MorsePreferences::prefs.wlanSSID);
                    MorseDisplay::printOnScroll(1, REGULAR, 0, WiFi.localIP().toString());
                }
                WiFi.mode( WIFI_MODE_NULL ); // switch off WiFi
                delay(1000);
                MorseDisplay::printOnScroll(2, REGULAR, 0, "RED: return" );
                while (true) {
                      MorseSystem::checkShutDown(false);  // possibly time-out: go to sleep
                      if (digitalRead(volButtonPin) == LOW) {
                        return false;
                      }
                }
                break;
      case _wifi_upload:
                uploadFile();       // upload a text file
                break;
      case _wifi_update:
                updateFirmware();   // run OTA update
                break;
      case  _goToSleep: /// deep sleep
                MorseSystem::checkShutDown(true);
      default:  break;
  }
  return false;
}   /// end menuExec()






//////// Display the preferences menu - we display the following preferences

void MorseMenu::displayKeyerPreferencesMenu(int pos)
{
    MorseDisplay::clearAll();
    if (pos < MorsePreferences::posLoraBand) {
        MorseDisplay::printOnStatusLine(true, 0, "Set Preferences: ");
    }
    else if (pos < MorsePreferences::posSnapRecall) {
        MorseDisplay::printOnStatusLine(true, 0, "Config LoRa:     ");
    }
    else {
        MorseDisplay::printOnStatusLine(true, 0, "Manage Snapshots:");
    }
    MorseDisplay::printOnScroll(1, BOLD, 0, MorsePreferences::prefOption[pos]);

    switch (pos)
    {
        case MorsePreferences::posCurtisMode:
            internal::displayCurtisMode();
            break;
        case MorsePreferences::posCurtisBDahTiming:
            internal::displayCurtisBTiming();
            break;
        case MorsePreferences::posCurtisBDotTiming:
            internal::displayCurtisBDotTiming();
            break;
        case MorsePreferences::posACS:
            internal::displayACS();
            break;
        case MorsePreferences::posPolarity:
            internal::displayPolarity();
            break;
        case MorsePreferences::posLatency:
            internal::displayLatency();
            break;
        case MorsePreferences::posExtPaddles:
            internal::displayExtPaddles();
            break;
        case MorsePreferences::posPitch:
            internal::displayPitch();
            break;
        case MorsePreferences::posClicks:
            internal::displayClicks();
            break;
        case MorsePreferences::posKeyTrainerMode:
            internal::displayKeyTrainerMode();
            break;
        case MorsePreferences::posInterWordSpace:
            internal::displayInterWordSpace();
            break;
        case MorsePreferences::posInterCharSpace:
            internal::displayInterCharSpace();
            break;
        case MorsePreferences::posKochFilter:
            internal::displayKochFilter();
            break;
        case MorsePreferences::posRandomOption:
            internal::displayRandomOption();
            break;
        case MorsePreferences::posRandomLength:
            internal::displayRandomLength();
            break;
        case MorsePreferences::posCallLength:
            internal::displayCallLength();
            break;
        case MorsePreferences::posAbbrevLength:
            internal::displayAbbrevLength();
            break;
        case MorsePreferences::posWordLength:
            internal::displayWordLength();
            break;
        case MorsePreferences::posTrainerDisplay:
            internal::displayTrainerDisplay();
            break;
        case MorsePreferences::posEchoDisplay:
            internal::displayEchoDisplay();
            break;
        case MorsePreferences::posEchoRepeats:
            internal::displayEchoRepeats();
            break;
        case MorsePreferences::posEchoConf:
            internal::displayEchoConf();
            break;
        case MorsePreferences::posWordDoubler:
            internal::displayWordDoubler();
            break;
        case MorsePreferences::posEchoToneShift:
            internal::displayEchoToneShift();
            break;
        case MorsePreferences::posLoraTrainerMode:
            internal::displayLoraTrainerMode();
            break;
        case MorsePreferences::posLoraSyncW:
            internal::displayLoraSyncW();
            break;
        case MorsePreferences::posGoertzelBandwidth:
            internal::displayGoertzelBandwidth();
            break;
        case MorsePreferences::posSpeedAdapt:
            internal::displaySpeedAdapt();
            break;
        case MorsePreferences::posRandomFile:
            internal::displayRandomFile();
            break;
        case MorsePreferences::posKochSeq:
            internal::displayKochSeq();
            break;
        case MorsePreferences::posTimeOut:
            internal::displayTimeOut();
            break;
        case MorsePreferences::posQuickStart:
            internal::displayQuickStart();
            break;
        case MorsePreferences::posLoraBand:
            internal::displayLoraBand();
            break;
        case MorsePreferences::posLoraQRG:
            internal::displayLoraQRG();
            break;
        case MorsePreferences::posSnapRecall:
            internal::displaySnapRecall();
            break;
        case MorsePreferences::posSnapStore:
            internal::displaySnapStore();
            break;
        case MorsePreferences::posMaxSequence:
            internal::displayMaxSequence();
            break;
    } /// switch (pos)
    MorseDisplay::display();
} // displayKeyerPreferences()

/// now follow all the menu displays

void MorseMenu::internal::displayCurtisMode()
{
    String keyerModus[] =
    {"Curtis A    ", "Curtis B    ", "Ultimatic   ", "Non-Squeeze "};
    MorseDisplay::printOnScroll(2, REGULAR, 1, keyerModus[MorsePreferences::prefs.keyermode - 1]);
}

void MorseMenu::internal::displayCurtisBTiming()
{
    // display start timing when paddles are being checked in Curtis B mode during dah: between 0 and 100
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", MorsePreferences::prefs.curtisBTiming);
}

void MorseMenu::internal::displayCurtisBDotTiming()
{
    // display start timing when paddles are being checked in Curtis B modeduring dit : between 0 and 100
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", MorsePreferences::prefs.curtisBDotTiming);
}

void MorseMenu::internal::displayACS()
{
    String ACSmode[] =
    {"Off         ", "Invalid     ", "min. 2 dots ", "min. 3 dots ", "min. 4 dots "};
    MorseDisplay::printOnScroll(2, REGULAR, 1, ACSmode[MorsePreferences::prefs.ACSlength]);
}

void MorseMenu::internal::displayPitch()
{
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", notes[MorsePreferences::prefs.sidetoneFreq]);
}

void MorseMenu::internal::displayClicks()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.encoderClicks ? "On " : "Off");
}

void MorseMenu::internal::displayExtPaddles()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.useExtPaddle ? "Reversed    " : "Normal      ");
}

void MorseMenu::internal::displayPolarity()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.didah ? ".- di-dah  " : "-. dah-dit ");
}

void MorseMenu::internal::displayLatency()
{
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%1i/8 of dit", MorsePreferences::prefs.latency - 1);
}
void MorseMenu::internal::displayInterWordSpace()
{
    // display interword space in ditlengths
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i", MorsePreferences::prefs.interWordSpace);
}

void MorseMenu::internal::displayInterCharSpace()
{
    // display intercharacter space in ditlengths
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i", MorsePreferences::prefs.interCharSpace);
}

void MorseMenu::internal::displayRandomOption()
{
    String texts[] =
    {"All Chars   ", "Alpha       ", "Numerals    ", "Interpunct. ", "Pro Signs   ", "Alpha + Num ", "Num+Interp. ", "Interp+ProSn",
            "Alph+Num+Int", "Num+Int+ProS"};
    MorseDisplay::printOnScroll(2, REGULAR, 1, texts[MorsePreferences::prefs.randomOption]);
}

void MorseMenu::internal::displayRandomLength()
{
    // display length of random character groups - 2 - 6
    if (MorsePreferences::prefs.randomLength <= 6) {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%1i     ", MorsePreferences::prefs.randomLength);
    }
    else {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "2 to %1i", MorsePreferences::prefs.randomLength - 4);
    }
}

void MorseMenu::internal::displayCallLength()
{
    // display length of calls - 3 - 6, 0 = all
    if (MorsePreferences::prefs.callLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i   ", MorsePreferences::prefs.callLength);
    }
}

void MorseMenu::internal::displayAbbrevLength()
{
    // display length of abbrev - 2 - 6, 0 = all
    if (MorsePreferences::prefs.abbrevLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i    ", MorsePreferences::prefs.abbrevLength);
    }
}

void MorseMenu::internal::displayWordLength()
{
    // display length of english words - 2 - 6, 0 = all
    if (MorsePreferences::prefs.wordLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i     ", MorsePreferences::prefs.wordLength);
    }
}

void MorseMenu::internal::displayMaxSequence()
{
    // display max # of words; 0 = no limit, 5, 10, 15, 20... 250; 255 = no limit
    if ((MorsePreferences::prefs.maxSequence == 0) || (MorsePreferences::prefs.maxSequence == 255))
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i      ", MorsePreferences::prefs.maxSequence);
    }
}

void MorseMenu::internal::displayTrainerDisplay()
{
    switch (MorsePreferences::prefs.trainerDisplay)
    {
        case NO_DISPLAY:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Display off ");
            break;
        case DISPLAY_BY_CHAR:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Char by char");
            break;
        case DISPLAY_BY_WORD:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Word by word");
            break;
    }
}

void MorseMenu::internal::displayEchoDisplay()
{
    switch (MorsePreferences::prefs.echoDisplay)
    {
        case CODE_ONLY:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Sound only  ");
            break;
        case DISP_ONLY:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Display only");
            break;
        case CODE_AND_DISP:
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Sound & Disp");
            break;

    }
}
void MorseMenu::internal::displayKeyTrainerMode()
{
    String option;
    switch (MorsePreferences::prefs.keyTrainerMode)
    {
        case 0:
            option = "Never        ";
            break;
        case 1:
            option = "CW Keyer only";
            break;
        case 2:
            option = "Keyer&Genertr";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, option);
}

void MorseMenu::internal::displayLoraTrainerMode()
{
    String option;
    switch (MorsePreferences::prefs.loraTrainerMode)
    {
        case 0:
            option = "LoRa Tx OFF  ";
            break;
        case 1:
            option = "LoRa Tx ON   ";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, option);
}

void MorseMenu::internal::displayLoraSyncW()
{
    String option;
    switch (MorsePreferences::prefs.loraSyncW)
    {
        case 0x27:
            option = "Standard Ch  ";
            break;
        case 0x66:
            option = "Secondary Ch ";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, option);
}

void MorseMenu::internal::displayEchoRepeats()
{
    if (MorsePreferences::prefs.echoRepeats < 7)
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%i      ", MorsePreferences::prefs.echoRepeats);
    }
    else
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Forever");
}

void MorseMenu::internal::displayEchoToneShift()
{
    String option;
    switch (MorsePreferences::prefs.echoToneShift)
    {
        case 0:
            option = "No Tone Shift";
            break;
        case 1:
            option = "Up 1/2 Tone  ";
            break;
        case 2:
            option = "Down 1/2 Tone";
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, option);
}

void MorseMenu::internal::displayEchoConf()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.echoConf ? "On " : "Off");
}

void MorseMenu::internal::displayKochFilter()
{                          // const String kochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=KA+SNE@:";
    String str;
    str.reserve(6);
    str = (String) Koch::kochChars.charAt(MorsePreferences::prefs.kochFilter - 1);
    MorseDisplay::cleanUpProSigns(str);
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i %s   ", MorsePreferences::prefs.kochFilter, str.c_str());
}

void MorseMenu::internal::displayWordDoubler()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.wordDoubler ? "On  " : "Off ");
}

void MorseMenu::internal::displayRandomFile()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.randomFile ? "On  " : "Off ");
}

void MorseMenu::internal::displayGoertzelBandwidth()
{
    String option;
    switch (MorsePreferences::prefs.goertzelBandwidth)
    {
        case 0:
            option = "Wide         ";
            break;
        case 1:
            option = "Narrow       ";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, option);
}

void MorseMenu::internal::displaySpeedAdapt()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.speedAdapt ? "ON         " : "OFF        ");
}

void MorseMenu::internal::displayKochSeq()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.lcwoKochSeq ? "LCWO      " : "M32 / JLMC");
}

void MorseMenu::internal::displayTimeOut()
{
    String TOValue;

    switch (MorsePreferences::prefs.timeOut)
    {
        case 1:
            TOValue = " 5 min    ";
            break;
        case 2:
            TOValue = "10 min    ";
            break;
        case 3:
            TOValue = "15 min    ";
            break;
        case 4:
            TOValue = "No timeout";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, TOValue);
}

void MorseMenu::internal::displayQuickStart()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.quickStart ? "ON         " : "OFF        ");
}

void MorseMenu::internal::displayLoraBand()
{
    String bandName;
    switch (MorsePreferences::prefs.loraBand)
    {
        case 0:
            bandName = "433 MHz ";
            break;
        case 1:
            bandName = "868 MHz ";
            break;
        case 2:
            bandName = "920 MHz ";
            break;
    }
    MorseDisplay::printOnScroll(2, REGULAR, 1, bandName);
}

void MorseMenu::internal::displayLoraQRG()
{
    const int a = (int) QRG433;
    const int b = (int) QRG866;
    const int c = (int) QRG920;
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%6d kHz", MorsePreferences::prefs.loraQRG / 1000);

    switch (MorsePreferences::prefs.loraQRG)
    {
        case a:
        case b:
        case c:
            MorseDisplay::printOnScroll(2, BOLD, 11, "DEF");
            break;
        default:
            MorseDisplay::printOnScroll(2, REGULAR, 11, "   ");
            break;
    }
}

void MorseMenu::internal::displaySnapRecall()
{
    if (MorsePreferences::memCounter)
    {
        if (MorsePreferences::memPtr == MorsePreferences::memCounter)
            MorseDisplay::printOnScroll(2, REGULAR, 1, "Cancel Recall");
        else
        {
            MorseDisplay::vprintOnScroll(2, REGULAR, 1, "Snapshot %d   ", MorsePreferences::memories[MorsePreferences::memPtr] + 1);
        }
    }
    else
        MorseDisplay::printOnScroll(2, REGULAR, 1, "NO SNAPSHOTS");
}

void MorseMenu::internal::displaySnapStore()
{
    uint8_t mask = 1;
    mask = mask << MorsePreferences::memPtr;
    if (MorsePreferences::memPtr == 8)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Cancel Store");
    else
    {
        MorseDisplay::vprintOnScroll(2, MorsePreferences::prefs.snapShots & mask ? BOLD : REGULAR, 1, "Snapshot %d  ", MorsePreferences::memPtr + 1);
    }
}

//// function to addjust the selected preference

boolean MorseMenu::adjustKeyerPreference(MorsePreferences::prefPos pos)
{        /// rotating the encoder changes the value, click returns to preferences menu
    //MorseDisplay::printOnScroll(1, REGULAR, 0, " ");       /// returns true when a long button press ended it, and false when there was a short click
    MorseDisplay::printOnScroll(2, INVERSE_BOLD, 0, ">");

    int t;
    while (true)
    {                            // we wait for single click = selection or long click = exit
        modeButton.Update();
        switch (modeButton.clicks)
        {
            case -1: //delay(200);
                return true;
                break;
            case 1: //MorseDisplay::printOnScroll(1, BOLD, 0,  ">");
                MorseDisplay::printOnScroll(2, REGULAR, 0, " ");
                return false;
        }
        if (pos == MorsePreferences::posSnapRecall)
        {         // here we can delete a memory....
            volButton.Update();
            if (volButton.clicks)
            {
                if (MorsePreferences::memCounter)
                    MorsePreferences::clearMemory (MorsePreferences::memPtr);
                return true;
            }
        }
        if ((t = MorseRotaryEncoder::checkEncoder()))
        {
            MorseUI::click();
            switch (pos)
            {
                case MorsePreferences::posCurtisMode:
                    MorsePreferences::prefs.keyermode = (MorsePreferences::prefs.keyermode + t);                        // set the curtis mode
                    MorsePreferences::prefs.keyermode = constrain(MorsePreferences::prefs.keyermode, 1, 4);
                    internal::displayCurtisMode();                                    // display curtis mode
                    break;
                case MorsePreferences::posCurtisBDahTiming:
                    MorsePreferences::prefs.curtisBTiming += (t * 5);                          // Curtis B timing dah (enhanced Curtis mode)
                    MorsePreferences::prefs.curtisBTiming = constrain(MorsePreferences::prefs.curtisBTiming, 0, 100);
                    internal::displayCurtisBTiming();
                    break;
                case MorsePreferences::posCurtisBDotTiming:
                    MorsePreferences::prefs.curtisBDotTiming += (t * 5);                   // Curtis B timing dit (enhanced Curtis mode)
                    MorsePreferences::prefs.curtisBDotTiming = constrain(MorsePreferences::prefs.curtisBDotTiming, 0, 100);
                    internal::displayCurtisBDotTiming();
                    break;
                case MorsePreferences::posACS:
                    MorsePreferences::prefs.ACSlength += (t + 1);                       // ACS
                    if (MorsePreferences::prefs.ACSlength == 2)
                        MorsePreferences::prefs.ACSlength += t;
                    MorsePreferences::prefs.ACSlength = constrain(MorsePreferences::prefs.ACSlength - 1, 0, 4);
                    internal::displayACS();
                    break;
                case MorsePreferences::posPitch:
                    MorsePreferences::prefs.sidetoneFreq += t;                             // sidetone pitch
                    MorsePreferences::prefs.sidetoneFreq = constrain(MorsePreferences::prefs.sidetoneFreq, 1, 15);
                    internal::displayPitch();
                    break;
                case MorsePreferences::posClicks:
                    MorsePreferences::prefs.encoderClicks = !MorsePreferences::prefs.encoderClicks;
                    internal::displayClicks();
                    break;
                case MorsePreferences::posExtPaddles:
                    MorsePreferences::prefs.useExtPaddle = !MorsePreferences::prefs.useExtPaddle;                           // ext paddle on/off
                    internal::displayExtPaddles();
                    break;
                case MorsePreferences::posPolarity:
                    MorsePreferences::prefs.didah = !MorsePreferences::prefs.didah;                                            // polarity
                    internal::displayPolarity();
                    break;
                case MorsePreferences::posLatency:
                    MorsePreferences::prefs.latency += t;
                    MorsePreferences::prefs.latency = constrain(MorsePreferences::prefs.latency, 1, 8);
                    internal::displayLatency();
                    break;
                case MorsePreferences::posKeyTrainerMode:
                    MorsePreferences::prefs.keyTrainerMode += (t + 1);                     // Key TRX: 0=never, 1= keyer only, 2 = keyer & trainer
                    MorsePreferences::prefs.keyTrainerMode = constrain(MorsePreferences::prefs.keyTrainerMode - 1, 0, 2);
                    internal::displayKeyTrainerMode();
                    break;
                case MorsePreferences::posInterWordSpace:
                    MorsePreferences::prefs.interWordSpace += t;                         // interword space in lengths of dit
                    MorsePreferences::prefs.interWordSpace = constrain(MorsePreferences::prefs.interWordSpace, 6, 45);            // has to be between 6 and 45 dits
                    internal::displayInterWordSpace();
                    MorseKeyer::updateTimings();
                    break;
                case MorsePreferences::posInterCharSpace:
                    MorsePreferences::prefs.interCharSpace = constrain(MorsePreferences::prefs.interCharSpace + t, 3, 24);  // set Interchar space - 3 - 24 dits
                    internal::displayInterCharSpace();
                    MorseKeyer::updateTimings();
                    break;
                case MorsePreferences::posKochFilter:
                    MorsePreferences::prefs.kochFilter = constrain(MorsePreferences::prefs.kochFilter + t, 1, Koch::kochChars.length());
                    internal::displayKochFilter();
                    break;
                    //case  posGenerate : MorsePreferences::prefs.generatorMode = (MorsePreferences::prefs.generatorMode + t + 6) % 6;     // what trainer generates (0 - 5)
                    //               displayGenerate();
                    //               break;
                case MorsePreferences::posRandomOption:
                    MorsePreferences::prefs.randomOption = (MorsePreferences::prefs.randomOption + t + 10) % 10;     // which char set for random chars?
                    internal::displayRandomOption();
                    break;
                case MorsePreferences::posRandomLength:
                    MorsePreferences::prefs.randomLength += t;                                 // length of random char group: 2-6
                    MorsePreferences::prefs.randomLength = constrain(MorsePreferences::prefs.randomLength, 1, 10);                   // 7-10 for rnd length 2 to 3-6
                    internal::displayRandomLength();
                    break;
                case MorsePreferences::posCallLength:
                    if (MorsePreferences::prefs.callLength)                                             // length of calls: 0, or 3-6
                        MorsePreferences::prefs.callLength -= 2;                                        // temorarily make it 0-4
                    MorsePreferences::prefs.callLength = constrain(MorsePreferences::prefs.callLength + t, 0, 4);
                    if (MorsePreferences::prefs.callLength)                                             // length of calls: 0, or 3-6
                        MorsePreferences::prefs.callLength += 2;                                        // expand again if not 0

                    internal::displayCallLength();
                    break;
                case MorsePreferences::posAbbrevLength:
                    MorsePreferences::prefs.abbrevLength += (t + 1);                                 // length of abbreviations: 0, or 2-6
                    if (MorsePreferences::prefs.abbrevLength == 2)                                      // get rid of 1
                        MorsePreferences::prefs.abbrevLength += t;
                    MorsePreferences::prefs.abbrevLength = constrain(MorsePreferences::prefs.abbrevLength - 1, 0, 6);
                    internal::displayAbbrevLength();
                    break;
                case MorsePreferences::posWordLength:
                    MorsePreferences::prefs.wordLength += (t + 1);                                   // length of English words: 0, or 2-6
                    if (MorsePreferences::prefs.wordLength == 2)                                        // get rid of 1
                        MorsePreferences::prefs.wordLength += t;
                    MorsePreferences::prefs.wordLength = constrain(MorsePreferences::prefs.wordLength - 1, 0, 6);
                    internal::displayWordLength();
                    break;
                case MorsePreferences::posMaxSequence:
                    switch (MorsePreferences::prefs.maxSequence)
                    {
                        case 0:
                            if (t == -1)
                                MorsePreferences::prefs.maxSequence = 250;
                            else
                                MorsePreferences::prefs.maxSequence = 5;
                            break;
                        case 250:
                            if (t == -1)
                                MorsePreferences::prefs.maxSequence = 245;
                            else
                                MorsePreferences::prefs.maxSequence = 0;
                            break;
                        default:
                            MorsePreferences::prefs.maxSequence += 5 * t;
                            break;
                    }
                    internal::displayMaxSequence();
                    break;
                case MorsePreferences::posTrainerDisplay:
                    MorsePreferences::prefs.trainerDisplay = (MorsePreferences::prefs.trainerDisplay + t + 3) % 3;   // display options for trainer: 0-2
                    internal::displayTrainerDisplay();
                    break;
                case MorsePreferences::posEchoDisplay:
                    MorsePreferences::prefs.echoDisplay += t;
                    MorsePreferences::prefs.echoDisplay = constrain(MorsePreferences::prefs.echoDisplay, 1, 3);             // what prompt for echo trainer mode
                    internal::displayEchoDisplay();
                    break;
                case MorsePreferences::posEchoRepeats:
                    MorsePreferences::prefs.echoRepeats += (t + 1);                                 // no of echo repeats: 0-6, 7=forever
                    MorsePreferences::prefs.echoRepeats = constrain(MorsePreferences::prefs.echoRepeats - 1, 0, 7);
                    internal::displayEchoRepeats();
                    break;
                case MorsePreferences::posEchoToneShift:
                    MorsePreferences::prefs.echoToneShift += (t + 1);                             // echo tone shift can be 0, 1 (up) or 2 (down)
                    MorsePreferences::prefs.echoToneShift = constrain(MorsePreferences::prefs.echoToneShift - 1, 0, 2);
                    internal::displayEchoToneShift();
                    break;
                case MorsePreferences::posWordDoubler:
                    MorsePreferences::prefs.wordDoubler = !MorsePreferences::prefs.wordDoubler;
                    internal::displayWordDoubler();
                    break;
                case MorsePreferences::posRandomFile:
                    if (MorsePreferences::prefs.randomFile)
                        MorsePreferences::prefs.randomFile = 0;
                    else
                        MorsePreferences::prefs.randomFile = 255;
                    internal::displayRandomFile();
                    break;
                case MorsePreferences::posEchoConf:
                    MorsePreferences::prefs.echoConf = !MorsePreferences::prefs.echoConf;
                    internal::displayEchoConf();
                    break;
                case MorsePreferences::posLoraTrainerMode:
                    MorsePreferences::prefs.loraTrainerMode += (t + 2);                    // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    MorsePreferences::prefs.loraTrainerMode = (MorsePreferences::prefs.loraTrainerMode % 2);
                    internal::displayLoraTrainerMode();
                    break;
                case MorsePreferences::posLoraSyncW:
                    MorsePreferences::prefs.loraSyncW = (MorsePreferences::prefs.loraSyncW == 0x27 ? 0x66 : 0x27);
                    internal::displayLoraSyncW();
                    break;
                case MorsePreferences::posGoertzelBandwidth:
                    MorsePreferences::prefs.goertzelBandwidth += (t + 2);                  // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    MorsePreferences::prefs.goertzelBandwidth = (MorsePreferences::prefs.goertzelBandwidth % 2);
                    internal::displayGoertzelBandwidth();
                    break;
                case MorsePreferences::posSpeedAdapt:
                    MorsePreferences::prefs.speedAdapt = !MorsePreferences::prefs.speedAdapt;
                    internal::displaySpeedAdapt();
                    break;
                case MorsePreferences::posKochSeq:
                    MorsePreferences::prefs.lcwoKochSeq = !MorsePreferences::prefs.lcwoKochSeq;
                    internal::displayKochSeq();
                    break;
                case MorsePreferences::posTimeOut:
                    MorsePreferences::prefs.timeOut += (t + 1);
                    MorsePreferences::prefs.timeOut = constrain(MorsePreferences::prefs.timeOut - 1, 1, 4);
                    internal::displayTimeOut();
                    break;
                case MorsePreferences::posQuickStart:
                    MorsePreferences::prefs.quickStart = !MorsePreferences::prefs.quickStart;
                    internal::displayQuickStart();
                    break;
                case MorsePreferences::posLoraBand:
                    MorsePreferences::prefs.loraBand += (t + 1);                              // set the LoRa band
                    MorsePreferences::prefs.loraBand = constrain(MorsePreferences::prefs.loraBand - 1, 0, 2);
                    internal::displayLoraBand();                                // display LoRa band
                    switch (MorsePreferences::prefs.loraBand)
                    {
                        case 0:
                            MorsePreferences::prefs.loraQRG = QRG433;
                            break;
                        case 1:
                            MorsePreferences::prefs.loraQRG = QRG866;
                            break;
                        case 2:
                            MorsePreferences::prefs.loraQRG = QRG920;
                            break;
                    }
                    break;
                case MorsePreferences::posLoraQRG:
                    MorsePreferences::prefs.loraQRG += (t * 1E5);
                    switch (MorsePreferences::prefs.loraBand)
                    {
                        case 0:
                            MorsePreferences::prefs.loraQRG = constrain(MorsePreferences::prefs.loraQRG, 433.65E6, 434.55E6);
                            break;
                        case 1:
                            MorsePreferences::prefs.loraQRG = constrain(MorsePreferences::prefs.loraQRG, 866.25E6, 869.45E6);
                            break;
                        case 2:
                            MorsePreferences::prefs.loraQRG = constrain(MorsePreferences::prefs.loraQRG, 920.25E6, 923.15E6);
                            break;
                    }
                    internal::displayLoraQRG();
                    break;
                case MorsePreferences::posSnapRecall:
                case MorsePreferences::posSnapRecall:
                    if (MorsePreferences::memCounter)
                    {
                        MorsePreferences::memPtr = (MorsePreferences::memPtr + t + MorsePreferences::memCounter + 1) % (MorsePreferences::memCounter + 1);
                        //memPtr += (t+1);
                        //memPtr = constrain(memPtr-1, 0, memCounter);
                    }
                    internal::displaySnapRecall();
                    break;
                case MorsePreferences::posSnapStore:
                    MorsePreferences::memPtr = (MorsePreferences::memPtr + t + 9) % 9;
                    internal::displaySnapStore();
                    break;
            }   // end switch(pos)
            MorseDisplay::display();                                                      // update the display

        }      // end if(encoderPos)
        MorseSystem::checkShutDown(false);         // check for time out
    }    // end while(true)
}   // end of function

