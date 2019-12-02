/////////////// READING and WRITING parameters from / into Non Volatile Storage, using ESP32 preferences
#include "preferences.h"
#include "morsedefs.h"

Preferences pref;               // use the Preferences library for storing and retrieving objects


class MorsePrefs {
	// the preferences variable and their defaults

	  uint8_t p_version_major = VERSION_MAJOR;
	  uint8_t p_version_minor = VERSION_MINOR;
	  uint8_t p_sidetoneFreq = 11;               // side tone frequency                               1 - 15
	  uint8_t p_sidetoneVolume = 60;              // side tone volume, as a value between 0 and 100   0 -100
	  boolean p_didah = false;                    // paddle polarity                                  bool
	  uint8_t p_keyermode = 2;                    // Iambic keyer mode: see the #defines above        1 -  3
	  uint8_t p_interCharSpace = 3;               // trainer: in dit lengths                          3 - 24
	  boolean p_useExtPaddle = false;             // has now a different meaning: true when we need to reverse the polarity of the ext paddle
	  uint8_t p_ACSlength = 0;                    // in ACS: we extend the pause between charcaters to the equal length of how many dots
	                                              // (2, 3 or 4 are meaningful, 0 means off) 0, 2-4
	  boolean p_encoderClicks = true;             // all: should rotating the encoder generate a click?
	  uint8_t p_randomLength = 3;                 // trainer: how many random chars in one group -    1 -  5
	  uint8_t p_randomOption = 0;                 // trainer: from which pool are we generating random characters?  0 - 9
	  uint8_t p_callLength = 0;                   // trainer: max length of call signs generated (0 = unlimited)    0, 3 - 6
	  uint8_t p_abbrevLength = 0;                 // trainer: max length of abbreviations generated (0 = unlimited) 0, 2 - 6
	  uint8_t p_wordLength = 0;                   // trainer: max length of english words generated (0 = unlimited) 0, 2 - 6
	  uint8_t p_trainerDisplay = DISPLAY_BY_CHAR; // trainer: how we display what the trainer generates: nothing, by character, or by word  0 - 2
	  uint8_t p_curtisBTiming = 45;               // keyer: timing for enhanced Curtis mode: dah                    0 - 100
	  uint8_t p_curtisBDotTiming = 75 ;           // keyer: timing for enhanced Curtis mode: dit                    0 - 100
	  uint8_t p_interWordSpace = 7;               // trainer: normal interword spacing in lengths of dit,           6 - 45 ; default = norm = 7

	  uint8_t p_echoRepeats = 3;                  // how often will echo trainer repeat an erroniously entered word? 0 - 7, 7=forever, default = 3
	  uint8_t p_echoDisplay = 1;                  //  1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP
	  uint8_t p_kochFilter = 5;                   // constrain output to characters learned according to Koch's method 2 - 45
	  boolean p_wordDoubler = false;              // in CW trainer mode only, repeat each word
	  uint8_t p_echoToneShift = 1;                // 0 = no shift, 1 = up, 2 = down (a half tone)                   0 - 2
	  boolean p_echoConf = true;                  // true if echo trainer confirms audibly too, not just visually
	  uint8_t p_keyTrainerMode = 1;               // key a transmitter in generator and player mode?
	                                              //  0: "Never";  1: "CW Keyer only";  2: "Keyer&Generator";
	  uint8_t p_loraTrainerMode = 0;              // transmit via LoRa in generator and player mode?
	                                              //  0: "No";  1: "yes"
	  uint8_t p_goertzelBandwidth = 0;            //  0: "Wide" 1: "Narrow"
	  boolean p_speedAdapt = false;               //  true: in echo modes, increase speed when OK, reduce when not ok
	  uint8_t p_latency = 5;                      //  time span after currently sent element during which paddles are not checked; in 1/8th of dit length; stored as 1 -  8
	  uint8_t p_randomFile = 0;                   // if 0, play file word by word; if 255, skip random number of words (0 - 255) between reads
	  boolean p_lcwoKochSeq = false;              // if true, replace native sequence with LCWO sequence
	  uint8_t p_timeOut = 1;                      // time-out value: 4 = no timeout, 1 = 5 min, 2 = 10 min, 3 = 15 min
	  boolean p_quickStart = false;               // should we start the last executed command immediately?
	  uint8_t p_loraSyncW = 0x27;                 // allows to set different LoRa sync words, and so creating virtual "channels"

	///// stored in preferences, but not adjustable through preferences menu:
	  uint8_t p_responsePause = 5;                // in echoTrainer mode, how long do we wait for response? in interWordSpaces; 2-12, default 5
	  uint8_t p_wpm = 15;                         // keyer speed in words per minute                  5 - 60
	  uint8_t p_menuPtr = 1;                      // current position of menu
	  String  p_wlanSSID = "";                    // SSID for connecting to the Internet
	  String  p_wlanPassword = "";                // password for connecting to WiFi router
	  uint32_t p_fileWordPointer = 0;             // remember how far we have read the file in player mode / reset when loading new file
	  uint8_t p_promptPause = 2;                  // in echoTrainer mode, length of pause before we send next word; multiplied by interWordSpace
	  uint8_t p_tLeft = 20;                       // threshold for left paddle
	  uint8_t p_tRight = 20;                      // threshold for right paddle

	  uint8_t p_loraBand = 0;                     // 0 = 433, 1 = 868, 2 = 920
	#define QRG433 434.15E6
	#define QRG866 869.15E6
	#define QRG920 920.55E6
	  uint32_t p_loraQRG = QRG433;                // for 70 cm band

	  uint8_t p_snapShots = 0;                    // keep track which snapshots are being used ( 0 .. 7, called 1 to 8)
	  uint8_t p_maxSequence = 0;                  // max # of words generated beofre the Morserino pauses

	////// end of variables stored in preferences

};



using namespace MorsePreferences;

MorsePrefs MorsePreferences::readPreferences(String repository) {
	MorsePrefs p = new MorsePrefs();
  unsigned int l = 15;
  char repName[l];
  uint8_t temp;
  uint32_t tempInt;

  boolean atStart = false;

  if (repository == "morserino")
    atStart = true;

  repository.toCharArray(repName, l);
  // Serial.println("Reading from repository: " + String(repName));
  // read preferences from non-volatile storage
  // if version cannot be read, we have a new ESP32 and need to write the preferences first

  if (atStart)
    pref.begin(repName, false);                // open namespace in read/write mode
  else
    pref.begin(repName, true);                 // read only in all other cases

  /// new code for reading preferences values - we check if we have a value, and if yes, we use it; if no, we use and write a default value

    if (atStart) {
      if ((temp = pref.getUChar("version_major")) != p_version_major)
         pref.putUChar("version_major", p_version_major);
      if ((temp = pref.getUChar("version_minor")) != p_version_minor)
         pref.putUChar("version_minor", p_version_minor);
    }

    if ((temp = pref.getUChar("sidetoneFreq")))
       p_sidetoneFreq = temp;
    else if (atStart)
       pref.putUChar("sidetoneFreq", p_sidetoneFreq);

    if ((temp = pref.getUChar("wpm")))
       p_wpm = temp;
    else if (atStart)
       pref.putUChar("wpm", p_wpm);

    if ((temp = pref.getUChar("sidetoneVolume",255)) != 255)
       p_sidetoneVolume = temp;
    else if (atStart)
       pref.putUChar("sidetoneVolume", p_sidetoneVolume);

    if ((temp = pref.getUChar("keyermode")))
       p_keyermode = temp;
    else if (atStart)
       pref.putUChar("keyermode", p_keyermode);

    if ((temp = pref.getUChar("farnsworthMode")))
       p_interCharSpace = temp;
    else if (atStart)
       pref.putUChar("farnsworthMode", p_interCharSpace);

    if ((temp = pref.getUChar("ACSlength",255)) != 255)
       p_ACSlength = temp;
    else if (atStart)
       pref.putUChar("ACSlength", p_ACSlength);

    if ((temp = pref.getUChar("keyTrainerMode", 255)) != 255)
       p_keyTrainerMode = temp;
    else if (atStart)
       pref.putUChar("keyTrainerMode", p_keyTrainerMode);

    if ((temp = pref.getUChar("randomLength")))
       p_randomLength = temp;
    else if (atStart)
       pref.putUChar("randomLength", p_randomLength);

    if ((temp = pref.getUChar("randomOption", 255)) != 255)
       p_randomOption = temp;
    else if (atStart)
       pref.putUChar("randomOption", p_randomOption);

    if ((temp = pref.getUChar("callLength", 255)) != 255)
       p_callLength = temp;
    else if (atStart)
       pref.putUChar("callLength", p_callLength);

    if ((temp = pref.getUChar("abbrevLength", 255)) != 255)
       p_abbrevLength = temp;
    else if (atStart)
       pref.putUChar("abbrevLength", p_abbrevLength);

    if ((temp = pref.getUChar("wordLength", 255)) != 255)
       p_wordLength = temp;
    else if (atStart)
       pref.putUChar("wordLength", p_wordLength);

    if ((temp = pref.getUChar("trainerDisplay", 255)) != 255)
       p_trainerDisplay = temp;
    else if (atStart)
       pref.putUChar("trainerDisplay", p_trainerDisplay);

    if ((temp = pref.getUChar("echoDisplay", 255)) != 255)
       p_echoDisplay = temp;
    else if (atStart)
       pref.putUChar("echoDisplay", p_echoDisplay);

    if ((temp = pref.getUChar("curtisBTiming", 255)) != 255)
       p_curtisBTiming = temp;
    else if (atStart)
       pref.putUChar("curtisBTiming", p_curtisBTiming);

    if ((temp = pref.getUChar("curtisBDotT", 255)) != 255)
       p_curtisBDotTiming = temp;
    else if (atStart)
       pref.putUChar("curtisBDotT", p_curtisBDotTiming);

    if ((temp = pref.getUChar("interWordSpace")))
       p_interWordSpace = temp;
    else if (atStart)
       pref.putUChar("interWordSpace", p_interWordSpace);

    if ((temp = pref.getUChar("echoRepeats", 255)) != 255)
       p_echoRepeats = temp;
    else if (atStart)
       pref.putUChar("echoRepeats", p_echoRepeats);

    if ((temp = pref.getUChar("echoToneShift", 255)) != 255)
       p_echoToneShift = temp;
    else if (atStart)
       pref.putUChar("echoToneShift", p_echoToneShift);

    if (atStart) {
        if ((temp = pref.getUChar("kochFilter")))
           p_kochFilter = temp;
        else
           pref.putUChar("kochFilter", p_kochFilter);
    }

    if ((temp = pref.getUChar("loraTrainerMode")))
        p_loraTrainerMode = temp;
    else if (atStart)
        pref.putUChar("loraTrainerMode", p_loraTrainerMode);

    if ((temp = pref.getUChar("goertzelBW")))
        p_goertzelBandwidth = temp;
    else if (atStart)
        pref.putUChar("goertzelBW", p_goertzelBandwidth);

    if ((temp = pref.getUChar("latency")))
        p_latency = temp;
    else if (atStart)
        pref.putUChar("latency", p_latency);

    if ((temp = pref.getUChar("randomFile")))
        p_randomFile = temp;

    if ((temp = pref.getUChar("lastExecuted")))
       p_menuPtr = temp;
    //Serial.println("read: p_menuPtr = " + String(p_menuPtr));

    if ((temp = pref.getUChar("timeOut")))
       p_timeOut = temp;
    else if (atStart)
       pref.putUChar("timeOut", p_timeOut);

    if ((temp = pref.getUChar("loraSyncW")))
        p_loraSyncW = temp;
    else if (atStart)
        pref.putUChar("loraSyncW", p_loraSyncW);

    if ((temp = pref.getUChar("maxSequence", p_maxSequence)))
        p_maxSequence = temp;

    p_didah = pref.getBool("didah", true);
    p_useExtPaddle = pref.getBool("useExtPaddle");
    p_encoderClicks = pref.getBool("encoderClicks", true);
    p_echoConf = pref.getBool("echoConf", true);
    p_wordDoubler = pref.getBool("wordDoubler");
    p_speedAdapt  = pref.getBool("speedAdapt");

    p_wlanSSID = pref.getString("wlanSSID");
    p_wlanPassword = pref.getString("wlanPassword");
    p_fileWordPointer = pref.getUInt("fileWordPtr");
    p_lcwoKochSeq = pref.getBool("lcwoKochSeq");
    p_quickStart = pref.getBool("quickStart");


    if (atStart) {
      if ((temp = pref.getUChar("loraBand")))
        p_loraBand = temp;
      else
       p_loraBand = 0;

      if ((tempInt = pref.getUInt("loraQRG"))) {
        p_loraQRG = tempInt;
      }
      else
        p_loraQRG = QRG433;

     if ((temp = pref.getUChar("snapShots"))) {
        p_snapShots = temp;
        updateMemory(temp);
     }  // end: we have snapshots
    }  // endif atStart
   pref.end();
   updateTimings();

   return p;
}

void Preferences::writePreferences(String repository) {
  unsigned int l = 15;
  char repName[l];
  uint8_t temp;
  uint32_t tempInt;

  boolean morserino = false;

  if (repository == "morserino")
    morserino = true;
//Serial.println("Writing to repository: " + repository);
  repository.toCharArray(repName, l);

  pref.begin(repName, false);                // open namespace in read/write mode

    if (p_sidetoneFreq != pref.getUChar("sidetoneFreq"))
        pref.putUChar("sidetoneFreq", p_sidetoneFreq);
    if (p_didah != pref.getBool("didah"))
        pref.putBool("didah", p_didah);
    if (p_keyermode != pref.getUChar("keyermode"))
        pref.putUChar("keyermode", p_keyermode);
    if (p_interCharSpace != pref.getUChar("farnsworthMode"))
        pref.putUChar("farnsworthMode", p_interCharSpace);
    if (p_useExtPaddle != pref.getBool("useExtPaddle"))
        pref.putBool("useExtPaddle", p_useExtPaddle);
    if (p_ACSlength != pref.getUChar("ACSlength"))
        pref.putUChar("ACSlength", p_ACSlength);
    if (p_keyTrainerMode != pref.getUChar("keyTrainerMode"))
        pref.putUChar("keyTrainerMode", p_keyTrainerMode);
    if (p_encoderClicks != pref.getBool("encoderClicks"))
        pref.putBool("encoderClicks", p_encoderClicks);
    if (p_randomLength != pref.getUChar("randomLength"))
        pref.putUChar("randomLength", p_randomLength);
    if (p_randomOption != pref.getUChar("randomOption"))
        pref.putUChar("randomOption", p_randomOption);
    if (p_callLength != pref.getUChar("callLength"))
        pref.putUChar("callLength", p_callLength);
    if (p_abbrevLength != pref.getUChar("abbrevLength")) {
        pref.putUChar("abbrevLength", p_abbrevLength);
        if (morserino)
          createKochAbbr(p_abbrevLength, p_kochFilter); // update the abbrev array!
    }
    if (p_wordLength != pref.getUChar("wordLength")) {
        pref.putUChar("wordLength", p_wordLength);
        if (morserino)
          createKochWords(p_wordLength, p_kochFilter) ;  // update the word array!
    }
    if (p_trainerDisplay != pref.getUChar("trainerDisplay"))
        pref.putUChar("trainerDisplay", p_trainerDisplay);
    if (p_echoDisplay != pref.getUChar("echoDisplay"))
        pref.putUChar("echoDisplay", p_echoDisplay);
    if (p_curtisBTiming != pref.getUChar("curtisBTiming"))
        pref.putUChar("curtisBTiming", p_curtisBTiming);
    if (p_curtisBDotTiming != pref.getUChar("curtisBDotT"))
        pref.putUChar("curtisBDotT", p_curtisBDotTiming);
    if (p_interWordSpace != pref.getUChar("interWordSpace"))
        pref.putUChar("interWordSpace", p_interWordSpace);
    if (p_echoRepeats != pref.getUChar("echoRepeats"))
        pref.putUChar("echoRepeats", p_echoRepeats);
    if (p_echoToneShift != pref.getUChar("echoToneShift"))
        pref.putUChar("echoToneShift", p_echoToneShift);
    if (p_echoConf != pref.getBool("echoConf"))
        pref.putBool("echoConf", p_echoConf);
    if (morserino) {
        if (p_kochFilter != pref.getUChar("kochFilter")) {
          pref.putUChar("kochFilter", p_kochFilter);
          createKochWords(p_wordLength, p_kochFilter) ;  // update the arrays!
          createKochAbbr(p_abbrevLength, p_kochFilter);
        }
    }
    if (p_lcwoKochSeq != pref.getBool("lcwoKochSeq")) {
        pref.putBool("lcwoKochSeq", p_lcwoKochSeq);
        if (morserino) {
          kochChars = p_lcwoKochSeq ? lcwoKochChars : morserinoKochChars;
          createKochWords(p_wordLength, p_kochFilter) ;  // update the arrays!
          createKochAbbr(p_abbrevLength, p_kochFilter);
        }
    }
    if (p_wordDoubler != pref.getBool("wordDoubler"))
        pref.putBool("wordDoubler", p_wordDoubler);
    if (p_speedAdapt != pref.getBool("speedAdapt"))
        pref.putBool("speedAdapt", p_speedAdapt);
    if (p_loraTrainerMode != pref.getUChar("loraTrainerMode"))
        pref.putUChar("loraTrainerMode", p_loraTrainerMode);
    if (p_goertzelBandwidth != pref.getUChar("goertzelBW")) {
        pref.putUChar("goertzelBW", p_goertzelBandwidth);
        if (morserino)
          setupGoertzel();
    }
    if (p_loraSyncW != pref.getUChar("loraSyncW")) {
        pref.putUChar("loraSyncW", p_loraSyncW);
        if (morserino)
          LoRa.setSyncWord(p_loraSyncW);
    }
    if (p_maxSequence != pref.getUChar("maxSequence"))
        pref.putUChar("maxSequence", p_maxSequence);

    if (p_latency != pref.getUChar("latency"))
        pref.putUChar("latency", p_latency);
    if (p_randomFile != pref.getUChar("randomFile"))
        pref.putUChar("randomFile", p_randomFile);
    if (p_timeOut != pref.getUChar("timeOut"))
        pref.putUChar("timeOut", p_timeOut);
    if (p_quickStart != pref.getBool("quickStart"))
        pref.putBool("quickStart", p_quickStart);

    if (p_snapShots != pref.getUChar("snapShots"))
        pref.putUChar("snapShots", p_snapShots);

    if (! morserino)  {
        pref.putUChar("lastExecuted", p_menuPtr);   // store last executed command in snapshots
        //Serial.println("write: last executed: " + String(p_menuPtr));
    }


  pref.end();
}

boolean  Preferences::recallSnapshot() {         // return true if we selected a real recall, false when it was cancelled
    String snapname;
    String text;

    memPtr = 0;
    displayKeyerPreferencesMenu(posSnapRecall);
    if (!adjustKeyerPreference(posSnapRecall)) {
        //Serial.println("recall memPtr: " + String(memPtr));
        text = "Snap " + String(memories[memPtr]+1) + " RECALLD";
        if(memCounter) {
          if (memPtr != memCounter)  {
            snapname = "snap" + String(memories[memPtr]);
            //Serial.println("recall snapname: " + snapname);
            readPreferences(snapname);
            printOnScroll(2, BOLD, 0, text);
            //Serial.println("after recall - p_menuPtr: " + String(p_menuPtr));
            delay(1000);
            return true;
          }
          return false;
        }
    } return false;

}

boolean Preferences::storeSnapshot(uint8_t menu) {        // return true if we selected a real store, false when it was cancelled
    String snapname;
    uint8_t mask = 1;
    String text;

    memPtr = 0;
    displayKeyerPreferencesMenu(posSnapStore);
    adjustKeyerPreference(posSnapStore);
    volButton.Update();
        //Serial.println("store memPtr: " + String(memPtr));
    if (memPtr != 8)  {
        p_menuPtr = menu;     // also store last menu selection
        //Serial.println("menu: " + String(p_menuPtr));
        text = "Snap " + String(memPtr+1) + " STORED ";
        snapname = "snap" + String(memPtr);
        //Serial.println("store snapname: " + snapname);
        //Serial.println("store: p_menuPtr = " + String(p_menuPtr));
        writePreferences(snapname);
        /// insert the correct bit into p_snapShots & update memory variables
        mask = mask << memPtr;
        p_snapShots = p_snapShots | mask;
        //Serial.println("store p_snapShots: " + String(p_snapShots));
        printOnScroll(2, BOLD, 0, text);
        updateMemory(p_snapShots);
        delay(1000);
        return true;
      }
      return false;
}


void Preferences::updateMemory(uint8_t temp) {
  memCounter = 0;                 // create an array that contains the snapshots (memories) that are in use
        for (int i = 0; i<8; ++i) {
          if (temp & 1) {     // mask rightmost bit
            memories[memCounter] = i;
            ++memCounter;
          }
          temp = temp >> 1;   // shift one position to the right
        }
}


void Preferences::clearMemory(uint8_t ptr) {
  String text = "Snap " + String(memories[ptr]+1) + " CLEARED";

  p_snapShots &= ~(1 << memories[ptr]);     // clear the bit
  printOnScroll(2, BOLD, 0, text);
  updateMemory(p_snapShots);
  delay(1000);
}
