#include "menu.h"
#include "MorseSystem.h"
#include "koch.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include "MorseKeyer.h"
#include "MorseUI.h"
#include "MorseRotaryEncoder.h"

using namespace Menu;

namespace Menu::internal {
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
}

//////// Display the preferences menu - we display the following preferences

void Menu::displayKeyerPreferencesMenu(int pos)
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

void Menu::internal::displayCurtisMode()
{
    String keyerModus[] =
    {"Curtis A    ", "Curtis B    ", "Ultimatic   ", "Non-Squeeze "};
    MorseDisplay::printOnScroll(2, REGULAR, 1, keyerModus[MorsePreferences::prefs.keyermode - 1]);
}

void displayCurtisBTiming()
{
    // display start timing when paddles are being checked in Curtis B mode during dah: between 0 and 100
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", MorsePreferences::prefs.curtisBTiming);
}

void displayCurtisBDotTiming()
{
    // display start timing when paddles are being checked in Curtis B modeduring dit : between 0 and 100
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", MorsePreferences::prefs.curtisBDotTiming);
}

void displayACS()
{
    String ACSmode[] =
    {"Off         ", "Invalid     ", "min. 2 dots ", "min. 3 dots ", "min. 4 dots "};
    MorseDisplay::printOnScroll(2, REGULAR, 1, ACSmode[MorsePreferences::prefs.ACSlength]);
}

void displayPitch()
{
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i", notes[MorsePreferences::prefs.sidetoneFreq]);
}

void displayClicks()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.encoderClicks ? "On " : "Off");
}

void displayExtPaddles()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.useExtPaddle ? "Reversed    " : "Normal      ");
}

void displayPolarity()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.didah ? ".- di-dah  " : "-. dah-dit ");
}

void displayLatency()
{
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%1i/8 of dit", MorsePreferences::prefs.latency - 1);
}
void displayInterWordSpace()
{
    // display interword space in ditlengths
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i", MorsePreferences::prefs.interWordSpace);
}

void displayInterCharSpace()
{
    // display intercharacter space in ditlengths
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i", MorsePreferences::prefs.interCharSpace);
}

void displayRandomOption()
{
    String texts[] =
    {"All Chars   ", "Alpha       ", "Numerals    ", "Interpunct. ", "Pro Signs   ", "Alpha + Num ", "Num+Interp. ", "Interp+ProSn",
            "Alph+Num+Int", "Num+Int+ProS"};
    MorseDisplay::printOnScroll(2, REGULAR, 1, texts[MorsePreferences::prefs.randomOption]);
}

void displayRandomLength()
{
    // display length of random character groups - 2 - 6
    if (MorsePreferences::prefs.randomLength <= 6) {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%1i     ", MorsePreferences::prefs.randomLength);
    }
    else {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "2 to %1i", MorsePreferences::prefs.randomLength - 4);
    }
}

void displayCallLength()
{
    // display length of calls - 3 - 6, 0 = all
    if (MorsePreferences::prefs.callLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i   ", MorsePreferences::prefs.callLength);
    }
}

void displayAbbrevLength()
{
    // display length of abbrev - 2 - 6, 0 = all
    if (MorsePreferences::prefs.abbrevLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i    ", MorsePreferences::prefs.abbrevLength);
    }
}

void displayWordLength()
{
    // display length of english words - 2 - 6, 0 = all
    if (MorsePreferences::prefs.wordLength == 0)
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "max. %1i     ", MorsePreferences::prefs.wordLength);
    }
}

void displayMaxSequence()
{
    // display max # of words; 0 = no limit, 5, 10, 15, 20... 250; 255 = no limit
    if ((MorsePreferences::prefs.maxSequence == 0) || (MorsePreferences::prefs.maxSequence == 255))
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%3i      ", MorsePreferences::prefs.maxSequence);
    }
}

void displayTrainerDisplay()
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

void displayEchoDisplay()
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
void displayKeyTrainerMode()
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

void displayLoraTrainerMode()
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

void displayLoraSyncW()
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

void displayEchoRepeats()
{
    if (MorsePreferences::prefs.echoRepeats < 7)
    {
        MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%i      ", MorsePreferences::prefs.echoRepeats);
    }
    else
        MorseDisplay::printOnScroll(2, REGULAR, 1, "Forever");
}

void displayEchoToneShift()
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

void displayEchoConf()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.echoConf ? "On " : "Off");
}

void displayKochFilter()
{                          // const String kochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=KA+SNE@:";
    String str;
    str.reserve(6);
    str = (String) Koch::kochChars.charAt(MorsePreferences::prefs.kochFilter - 1);
    MorseDisplay::cleanUpProSigns(str);
    MorseDisplay::vprintOnScroll(2, REGULAR, 1, "%2i %s   ", MorsePreferences::prefs.kochFilter, str.c_str());
}

void displayWordDoubler()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.wordDoubler ? "On  " : "Off ");
}

void displayRandomFile()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.randomFile ? "On  " : "Off ");
}

void displayGoertzelBandwidth()
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

void displaySpeedAdapt()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.speedAdapt ? "ON         " : "OFF        ");
}

void displayKochSeq()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.lcwoKochSeq ? "LCWO      " : "M32 / JLMC");
}

void displayTimeOut()
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

void displayQuickStart()
{
    MorseDisplay::printOnScroll(2, REGULAR, 1, MorsePreferences::prefs.quickStart ? "ON         " : "OFF        ");
}

void displayLoraBand()
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

void displayLoraQRG()
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

void displaySnapRecall()
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

void displaySnapStore()
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

boolean Menu::adjustKeyerPreference(MorsePreferences::prefPos pos)
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
                    displayCurtisBTiming();
                    break;
                case MorsePreferences::posCurtisBDotTiming:
                    MorsePreferences::prefs.curtisBDotTiming += (t * 5);                   // Curtis B timing dit (enhanced Curtis mode)
                    MorsePreferences::prefs.curtisBDotTiming = constrain(MorsePreferences::prefs.curtisBDotTiming, 0, 100);
                    displayCurtisBDotTiming();
                    break;
                case MorsePreferences::posACS:
                    MorsePreferences::prefs.ACSlength += (t + 1);                       // ACS
                    if (MorsePreferences::prefs.ACSlength == 2)
                        MorsePreferences::prefs.ACSlength += t;
                    MorsePreferences::prefs.ACSlength = constrain(MorsePreferences::prefs.ACSlength - 1, 0, 4);
                    displayACS();
                    break;
                case MorsePreferences::posPitch:
                    MorsePreferences::prefs.sidetoneFreq += t;                             // sidetone pitch
                    MorsePreferences::prefs.sidetoneFreq = constrain(MorsePreferences::prefs.sidetoneFreq, 1, 15);
                    displayPitch();
                    break;
                case MorsePreferences::posClicks:
                    MorsePreferences::prefs.encoderClicks = !MorsePreferences::prefs.encoderClicks;
                    displayClicks();
                    break;
                case MorsePreferences::posExtPaddles:
                    MorsePreferences::prefs.useExtPaddle = !MorsePreferences::prefs.useExtPaddle;                           // ext paddle on/off
                    displayExtPaddles();
                    break;
                case MorsePreferences::posPolarity:
                    MorsePreferences::prefs.didah = !MorsePreferences::prefs.didah;                                            // polarity
                    displayPolarity();
                    break;
                case MorsePreferences::posLatency:
                    MorsePreferences::prefs.latency += t;
                    MorsePreferences::prefs.latency = constrain(MorsePreferences::prefs.latency, 1, 8);
                    displayLatency();
                    break;
                case MorsePreferences::posKeyTrainerMode:
                    MorsePreferences::prefs.keyTrainerMode += (t + 1);                     // Key TRX: 0=never, 1= keyer only, 2 = keyer & trainer
                    MorsePreferences::prefs.keyTrainerMode = constrain(MorsePreferences::prefs.keyTrainerMode - 1, 0, 2);
                    displayKeyTrainerMode();
                    break;
                case MorsePreferences::posInterWordSpace:
                    MorsePreferences::prefs.interWordSpace += t;                         // interword space in lengths of dit
                    MorsePreferences::prefs.interWordSpace = constrain(MorsePreferences::prefs.interWordSpace, 6, 45);            // has to be between 6 and 45 dits
                    displayInterWordSpace();
                    MorseKeyer::updateTimings();
                    break;
                case MorsePreferences::posInterCharSpace:
                    MorsePreferences::prefs.interCharSpace = constrain(MorsePreferences::prefs.interCharSpace + t, 3, 24);  // set Interchar space - 3 - 24 dits
                    displayInterCharSpace();
                    MorseKeyer::updateTimings();
                    break;
                case MorsePreferences::posKochFilter:
                    MorsePreferences::prefs.kochFilter = constrain(MorsePreferences::prefs.kochFilter + t, 1, Koch::kochChars.length());
                    displayKochFilter();
                    break;
                    //case  posGenerate : MorsePreferences::prefs.generatorMode = (MorsePreferences::prefs.generatorMode + t + 6) % 6;     // what trainer generates (0 - 5)
                    //               displayGenerate();
                    //               break;
                case MorsePreferences::posRandomOption:
                    MorsePreferences::prefs.randomOption = (MorsePreferences::prefs.randomOption + t + 10) % 10;     // which char set for random chars?
                    displayRandomOption();
                    break;
                case MorsePreferences::posRandomLength:
                    MorsePreferences::prefs.randomLength += t;                                 // length of random char group: 2-6
                    MorsePreferences::prefs.randomLength = constrain(MorsePreferences::prefs.randomLength, 1, 10);                   // 7-10 for rnd length 2 to 3-6
                    displayRandomLength();
                    break;
                case MorsePreferences::posCallLength:
                    if (MorsePreferences::prefs.callLength)                                             // length of calls: 0, or 3-6
                        MorsePreferences::prefs.callLength -= 2;                                        // temorarily make it 0-4
                    MorsePreferences::prefs.callLength = constrain(MorsePreferences::prefs.callLength + t, 0, 4);
                    if (MorsePreferences::prefs.callLength)                                             // length of calls: 0, or 3-6
                        MorsePreferences::prefs.callLength += 2;                                        // expand again if not 0

                    displayCallLength();
                    break;
                case MorsePreferences::posAbbrevLength:
                    MorsePreferences::prefs.abbrevLength += (t + 1);                                 // length of abbreviations: 0, or 2-6
                    if (MorsePreferences::prefs.abbrevLength == 2)                                      // get rid of 1
                        MorsePreferences::prefs.abbrevLength += t;
                    MorsePreferences::prefs.abbrevLength = constrain(MorsePreferences::prefs.abbrevLength - 1, 0, 6);
                    displayAbbrevLength();
                    break;
                case MorsePreferences::posWordLength:
                    MorsePreferences::prefs.wordLength += (t + 1);                                   // length of English words: 0, or 2-6
                    if (MorsePreferences::prefs.wordLength == 2)                                        // get rid of 1
                        MorsePreferences::prefs.wordLength += t;
                    MorsePreferences::prefs.wordLength = constrain(MorsePreferences::prefs.wordLength - 1, 0, 6);
                    displayWordLength();
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
                    displayMaxSequence();
                    break;
                case MorsePreferences::posTrainerDisplay:
                    MorsePreferences::prefs.trainerDisplay = (MorsePreferences::prefs.trainerDisplay + t + 3) % 3;   // display options for trainer: 0-2
                    displayTrainerDisplay();
                    break;
                case MorsePreferences::posEchoDisplay:
                    MorsePreferences::prefs.echoDisplay += t;
                    MorsePreferences::prefs.echoDisplay = constrain(MorsePreferences::prefs.echoDisplay, 1, 3);             // what prompt for echo trainer mode
                    displayEchoDisplay();
                    break;
                case MorsePreferences::posEchoRepeats:
                    MorsePreferences::prefs.echoRepeats += (t + 1);                                 // no of echo repeats: 0-6, 7=forever
                    MorsePreferences::prefs.echoRepeats = constrain(MorsePreferences::prefs.echoRepeats - 1, 0, 7);
                    displayEchoRepeats();
                    break;
                case MorsePreferences::posEchoToneShift:
                    MorsePreferences::prefs.echoToneShift += (t + 1);                             // echo tone shift can be 0, 1 (up) or 2 (down)
                    MorsePreferences::prefs.echoToneShift = constrain(MorsePreferences::prefs.echoToneShift - 1, 0, 2);
                    displayEchoToneShift();
                    break;
                case MorsePreferences::posWordDoubler:
                    MorsePreferences::prefs.wordDoubler = !MorsePreferences::prefs.wordDoubler;
                    displayWordDoubler();
                    break;
                case MorsePreferences::posRandomFile:
                    if (MorsePreferences::prefs.randomFile)
                        MorsePreferences::prefs.randomFile = 0;
                    else
                        MorsePreferences::prefs.randomFile = 255;
                    displayRandomFile();
                    break;
                case MorsePreferences::posEchoConf:
                    MorsePreferences::prefs.echoConf = !MorsePreferences::prefs.echoConf;
                    displayEchoConf();
                    break;
                case MorsePreferences::posLoraTrainerMode:
                    MorsePreferences::prefs.loraTrainerMode += (t + 2);                    // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    MorsePreferences::prefs.loraTrainerMode = (MorsePreferences::prefs.loraTrainerMode % 2);
                    displayLoraTrainerMode();
                    break;
                case MorsePreferences::posLoraSyncW:
                    MorsePreferences::prefs.loraSyncW = (MorsePreferences::prefs.loraSyncW == 0x27 ? 0x66 : 0x27);
                    displayLoraSyncW();
                    break;
                case MorsePreferences::posGoertzelBandwidth:
                    MorsePreferences::prefs.goertzelBandwidth += (t + 2);                  // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    MorsePreferences::prefs.goertzelBandwidth = (MorsePreferences::prefs.goertzelBandwidth % 2);
                    displayGoertzelBandwidth();
                    break;
                case MorsePreferences::posSpeedAdapt:
                    MorsePreferences::prefs.speedAdapt = !MorsePreferences::prefs.speedAdapt;
                    displaySpeedAdapt();
                    break;
                case MorsePreferences::posKochSeq:
                    MorsePreferences::prefs.lcwoKochSeq = !MorsePreferences::prefs.lcwoKochSeq;
                    displayKochSeq();
                    break;
                case MorsePreferences::posTimeOut:
                    MorsePreferences::prefs.timeOut += (t + 1);
                    MorsePreferences::prefs.timeOut = constrain(MorsePreferences::prefs.timeOut - 1, 1, 4);
                    displayTimeOut();
                    break;
                case MorsePreferences::posQuickStart:
                    MorsePreferences::prefs.quickStart = !MorsePreferences::prefs.quickStart;
                    displayQuickStart();
                    break;
                case MorsePreferences::posLoraBand:
                    MorsePreferences::prefs.loraBand += (t + 1);                              // set the LoRa band
                    MorsePreferences::prefs.loraBand = constrain(MorsePreferences::prefs.loraBand - 1, 0, 2);
                    displayLoraBand();                                // display LoRa band
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
                    displayLoraQRG();
                    break;
                case MorsePreferences::posSnapRecall:
                case MorsePreferences::posSnapRecall:
                    if (MorsePreferences::memCounter)
                    {
                        MorsePreferences::memPtr = (MorsePreferences::memPtr + t + MorsePreferences::memCounter + 1) % (MorsePreferences::memCounter + 1);
                        //memPtr += (t+1);
                        //memPtr = constrain(memPtr-1, 0, memCounter);
                    }
                    displaySnapRecall();
                    break;
                case MorsePreferences::posSnapStore:
                    MorsePreferences::memPtr = (MorsePreferences::memPtr + t + 9) % 9;
                    displaySnapStore();
                    break;
            }   // end switch(pos)
            MorseDisplay::display();                                                      // update the display

        }      // end if(encoderPos)
        MorseSystem::checkShutDown(false);         // check for time out
    }    // end while(true)
}   // end of function

