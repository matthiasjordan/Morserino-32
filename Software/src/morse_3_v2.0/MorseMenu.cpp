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
#include "MorseLoRa.h"


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
    MorseLoRa::idle();
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
                MorseDisplay::displayTopLine();
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
                MorseDisplay::displayTopLine();
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
                MorseDisplay::displayTopLine();
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
                MorseDisplay::displayTopLine();
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
                MorseDisplay::displayTopLine();
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
