#include "menu.h"

using namespace Menu;

//////// Display the preferences menu - we display the following preferences

void Menu::displayKeyerPreferencesMenu(int pos)
{
    display.clear();
    if (pos < posLoraBand)
        printOnStatusLine(true, 0, "Set Preferences: ");
    else if (pos < posSnapRecall)
        printOnStatusLine(true, 0, "Config LoRa:     ");
    else
        printOnStatusLine(true, 0, "Manage Snapshots:");
    printOnScroll(1, BOLD, 0, prefOption[pos]);

    switch (pos)
    {
        case posCurtisMode:
            displayCurtisMode();
            break;
        case posCurtisBDahTiming:
            displayCurtisBTiming();
            break;
        case posCurtisBDotTiming:
            displayCurtisBDotTiming();
            break;
        case posACS:
            displayACS();
            break;
        case posPolarity:
            displayPolarity();
            break;
        case posLatency:
            displayLatency();
            break;
        case posExtPaddles:
            displayExtPaddles();
            break;
        case posPitch:
            displayPitch();
            break;
        case posClicks:
            displayClicks();
            break;
        case posKeyTrainerMode:
            displayKeyTrainerMode();
            break;
        case posInterWordSpace:
            displayInterWordSpace();
            break;
        case posInterCharSpace:
            displayInterCharSpace();
            break;
        case posKochFilter:
            displayKochFilter();
            break;
        case posRandomOption:
            displayRandomOption();
            break;
        case posRandomLength:
            displayRandomLength();
            break;
        case posCallLength:
            displayCallLength();
            break;
        case posAbbrevLength:
            displayAbbrevLength();
            break;
        case posWordLength:
            displayWordLength();
            break;
        case posTrainerDisplay:
            displayTrainerDisplay();
            break;
        case posEchoDisplay:
            displayEchoDisplay();
            break;
        case posEchoRepeats:
            displayEchoRepeats();
            break;
        case posEchoConf:
            displayEchoConf();
            break;
        case posWordDoubler:
            displayWordDoubler();
            break;
        case posEchoToneShift:
            displayEchoToneShift();
            break;
        case posLoraTrainerMode:
            displayLoraTrainerMode();
            break;
        case posLoraSyncW:
            displayLoraSyncW();
            break;
        case posGoertzelBandwidth:
            displayGoertzelBandwidth();
            break;
        case posSpeedAdapt:
            displaySpeedAdapt();
            break;
        case posRandomFile:
            displayRandomFile();
            break;
        case posKochSeq:
            displayKochSeq();
            break;
        case posTimeOut:
            displayTimeOut();
            break;
        case posQuickStart:
            displayQuickStart();
            break;
        case posLoraBand:
            displayLoraBand();
            break;
        case posLoraQRG:
            displayLoraQRG();
            break;
        case posSnapRecall:
            displaySnapRecall();
            break;
        case posSnapStore:
            displaySnapStore();
            break;
        case posMaxSequence:
            displayMaxSequence();
            break;
    } /// switch (pos)
    display.display();
} // displayKeyerPreferences()

/// now follow all the menu displays

void displayCurtisMode()
{
    String keyerModus[] =
    {"Curtis A    ", "Curtis B    ", "Ultimatic   ", "Non-Squeeze "};
    printOnScroll(2, REGULAR, 1, keyerModus[p_keyermode - 1]);
}

void displayCurtisBTiming()
{
    // display start timing when paddles are being checked in Curtis B mode during dah: between 0 and 100
    sprintf(numBuffer, "%3i", p_curtisBTiming);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayCurtisBDotTiming()
{
    // display start timing when paddles are being checked in Curtis B modeduring dit : between 0 and 100
    sprintf(numBuffer, "%3i", p_curtisBDotTiming);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayACS()
{
    String ACSmode[] =
    {"Off         ", "Invalid     ", "min. 2 dots ", "min. 3 dots ", "min. 4 dots "};
    printOnScroll(2, REGULAR, 1, ACSmode[p_ACSlength]);
}

void displayPitch()
{
    sprintf(numBuffer, "%3i", notes[p_sidetoneFreq]);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayClicks()
{
    printOnScroll(2, REGULAR, 1, p_encoderClicks ? "On " : "Off");
}

void displayExtPaddles()
{
    printOnScroll(2, REGULAR, 1, p_useExtPaddle ? "Reversed    " : "Normal      ");
}

void displayPolarity()
{
    printOnScroll(2, REGULAR, 1, p_didah ? ".- di-dah  " : "-. dah-dit ");
}

void displayLatency()
{
    sprintf(numBuffer, "%1i/8 of dit", p_latency - 1);
    printOnScroll(2, REGULAR, 1, numBuffer);
}
void displayInterWordSpace()
{
    // display interword space in ditlengths
    sprintf(numBuffer, "%2i", p_interWordSpace);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayInterCharSpace()
{
    // display intercharacter space in ditlengths
    sprintf(numBuffer, "%2i", p_interCharSpace);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayRandomOption()
{
    String texts[] =
    {"All Chars   ", "Alpha       ", "Numerals    ", "Interpunct. ", "Pro Signs   ", "Alpha + Num ", "Num+Interp. ", "Interp+ProSn",
            "Alph+Num+Int", "Num+Int+ProS"};
    printOnScroll(2, REGULAR, 1, texts[p_randomOption]);
}

void displayRandomLength()
{
    // display length of random character groups - 2 - 6
    if (p_randomLength <= 6)
        sprintf(numBuffer, "%1i     ", p_randomLength);
    else
        sprintf(numBuffer, "2 to %1i", p_randomLength - 4);
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayCallLength()
{
    // display length of calls - 3 - 6, 0 = all
    if (p_callLength == 0)
        printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        sprintf(numBuffer, "max. %1i   ", p_callLength);
        printOnScroll(2, REGULAR, 1, numBuffer);
    }
}

void displayAbbrevLength()
{
    // display length of abbrev - 2 - 6, 0 = all
    if (p_abbrevLength == 0)
        printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        sprintf(numBuffer, "max. %1i    ", p_abbrevLength);
        printOnScroll(2, REGULAR, 1, numBuffer);
    }
}

void displayWordLength()
{
    // display length of english words - 2 - 6, 0 = all
    if (p_wordLength == 0)
        printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        sprintf(numBuffer, "max. %1i     ", p_wordLength);
        printOnScroll(2, REGULAR, 1, numBuffer);
    }
}

void displayMaxSequence()
{
    // display max # of words; 0 = no limit, 5, 10, 15, 20... 250; 255 = no limit
    if ((p_maxSequence == 0) || (p_maxSequence == 255))
        printOnScroll(2, REGULAR, 1, "Unlimited");
    else
    {
        sprintf(numBuffer, "%3i      ", p_maxSequence);
        printOnScroll(2, REGULAR, 1, numBuffer);
    }
}

void displayTrainerDisplay()
{
    switch (p_trainerDisplay)
    {
        case NO_DISPLAY:
            printOnScroll(2, REGULAR, 1, "Display off ");
            break;
        case DISPLAY_BY_CHAR:
            printOnScroll(2, REGULAR, 1, "Char by char");
            break;
        case DISPLAY_BY_WORD:
            printOnScroll(2, REGULAR, 1, "Word by word");
            break;
    }
}

void displayEchoDisplay()
{
    switch (p_echoDisplay)
    {
        case CODE_ONLY:
            printOnScroll(2, REGULAR, 1, "Sound only  ");
            break;
        case DISP_ONLY:
            printOnScroll(2, REGULAR, 1, "Display only");
            break;
        case CODE_AND_DISP:
            printOnScroll(2, REGULAR, 1, "Sound & Disp");
            break;

    }
}
void displayKeyTrainerMode()
{
    String option;
    switch (p_keyTrainerMode)
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
    printOnScroll(2, REGULAR, 1, option);
}

void displayLoraTrainerMode()
{
    String option;
    switch (p_loraTrainerMode)
    {
        case 0:
            option = "LoRa Tx OFF  ";
            break;
        case 1:
            option = "LoRa Tx ON   ";
            break;
    }
    printOnScroll(2, REGULAR, 1, option);
}

void displayLoraSyncW()
{
    String option;
    switch (p_loraSyncW)
    {
        case 0x27:
            option = "Standard Ch  ";
            break;
        case 0x66:
            option = "Secondary Ch ";
            break;
    }
    printOnScroll(2, REGULAR, 1, option);
}

void displayEchoRepeats()
{
    if (p_echoRepeats < 7)
    {
        sprintf(numBuffer, "%i      ", p_echoRepeats);
        printOnScroll(2, REGULAR, 1, numBuffer);
    }
    else
        printOnScroll(2, REGULAR, 1, "Forever");
}

void displayEchoToneShift()
{
    String option;
    switch (p_echoToneShift)
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
    printOnScroll(2, REGULAR, 1, option);
}

void displayEchoConf()
{
    printOnScroll(2, REGULAR, 1, p_echoConf ? "On " : "Off");
}

void displayKochFilter()
{                          // const String kochChars = "mkrsuaptlowi.njef0yv,g5/q9zh38b?427c1d6x-=KA+SNE@:";
    String str;
    str.reserve(6);
    str = (String) kochChars.charAt(p_kochFilter - 1);
    cleanUpProSigns(str);
    sprintf(numBuffer, "%2i %s   ", p_kochFilter, str.c_str());
    printOnScroll(2, REGULAR, 1, numBuffer);
}

void displayWordDoubler()
{
    printOnScroll(2, REGULAR, 1, p_wordDoubler ? "On  " : "Off ");
}

void displayRandomFile()
{
    printOnScroll(2, REGULAR, 1, p_randomFile ? "On  " : "Off ");
}

void displayGoertzelBandwidth()
{
    String option;
    switch (p_goertzelBandwidth)
    {
        case 0:
            option = "Wide         ";
            break;
        case 1:
            option = "Narrow       ";
            break;
    }
    printOnScroll(2, REGULAR, 1, option);
}

void displaySpeedAdapt()
{
    printOnScroll(2, REGULAR, 1, p_speedAdapt ? "ON         " : "OFF        ");
}

void displayKochSeq()
{
    printOnScroll(2, REGULAR, 1, p_lcwoKochSeq ? "LCWO      " : "M32 / JLMC");
}

void displayTimeOut()
{
    String TOValue;

    switch (p_timeOut)
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
    printOnScroll(2, REGULAR, 1, TOValue);
}

void displayQuickStart()
{
    printOnScroll(2, REGULAR, 1, p_quickStart ? "ON         " : "OFF        ");
}

void displayLoraBand()
{
    String bandName;
    switch (p_loraBand)
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
    printOnScroll(2, REGULAR, 1, bandName);
}

void displayLoraQRG()
{
    const int a = (int) QRG433;
    const int b = (int) QRG866;
    const int c = (int) QRG920;
    sprintf(numBuffer, "%6d kHz", p_loraQRG / 1000);
    printOnScroll(2, REGULAR, 1, numBuffer);

    switch (p_loraQRG)
    {
        case a:
        case b:
        case c:
            printOnScroll(2, BOLD, 11, "DEF");
            break;
        default:
            printOnScroll(2, REGULAR, 11, "   ");
            break;
    }
}

void displaySnapRecall()
{
    if (memCounter)
    {
        if (memPtr == memCounter)
            printOnScroll(2, REGULAR, 1, "Cancel Recall");
        else
        {
            sprintf(numBuffer, "Snapshot %d   ", memories[memPtr] + 1);
            printOnScroll(2, REGULAR, 1, numBuffer);
        }
    }
    else
        printOnScroll(2, REGULAR, 1, "NO SNAPSHOTS");
}

void displaySnapStore()
{
    uint8_t mask = 1;
    mask = mask << memPtr;
    if (memPtr == 8)
        printOnScroll(2, REGULAR, 1, "Cancel Store");
    else
    {
        sprintf(numBuffer, "Snapshot %d  ", memPtr + 1);
        printOnScroll(2, p_snapShots & mask ? BOLD : REGULAR, 1, numBuffer);
    }
}

//// function to addjust the selected preference

boolean adjustKeyerPreference(prefPos pos)
{        /// rotating the encoder changes the value, click returns to preferences menu
    //printOnScroll(1, REGULAR, 0, " ");       /// returns true when a long button press ended it, and false when there was a short click
    printOnScroll(2, INVERSE_BOLD, 0, ">");

    int t;
    while (true)
    {                            // we wait for single click = selection or long click = exit
        modeButton.Update();
        switch (modeButton.clicks)
        {
            case -1: //delay(200);
                return true;
                break;
            case 1: //printOnScroll(1, BOLD, 0,  ">");
                printOnScroll(2, REGULAR, 0, " ");
                return false;
        }
        if (pos == posSnapRecall)
        {         // here we can delete a memory....
            volButton.Update();
            if (volButton.clicks)
            {
                if (memCounter)
                    clearMemory (memPtr);
                return true;
            }
        }
        if ((t = checkEncoder()))
        {
            pwmClick (p_sidetoneVolume);         /// click
            switch (pos)
            {
                case posCurtisMode:
                    p_keyermode = (p_keyermode + t);                        // set the curtis mode
                    p_keyermode = constrain(p_keyermode, 1, 4);
                    displayCurtisMode();                                    // display curtis mode
                    break;
                case posCurtisBDahTiming:
                    p_curtisBTiming += (t * 5);                          // Curtis B timing dah (enhanced Curtis mode)
                    p_curtisBTiming = constrain(p_curtisBTiming, 0, 100);
                    displayCurtisBTiming();
                    break;
                case posCurtisBDotTiming:
                    p_curtisBDotTiming += (t * 5);                   // Curtis B timing dit (enhanced Curtis mode)
                    p_curtisBDotTiming = constrain(p_curtisBDotTiming, 0, 100);
                    displayCurtisBDotTiming();
                    break;
                case posACS:
                    p_ACSlength += (t + 1);                       // ACS
                    if (p_ACSlength == 2)
                        p_ACSlength += t;
                    p_ACSlength = constrain(p_ACSlength - 1, 0, 4);
                    displayACS();
                    break;
                case posPitch:
                    p_sidetoneFreq += t;                             // sidetone pitch
                    p_sidetoneFreq = constrain(p_sidetoneFreq, 1, 15);
                    displayPitch();
                    break;
                case posClicks:
                    p_encoderClicks = !p_encoderClicks;
                    displayClicks();
                    break;
                case posExtPaddles:
                    p_useExtPaddle = !p_useExtPaddle;                           // ext paddle on/off
                    displayExtPaddles();
                    break;
                case posPolarity:
                    p_didah = !p_didah;                                            // polarity
                    displayPolarity();
                    break;
                case posLatency:
                    p_latency += t;
                    p_latency = constrain(p_latency, 1, 8);
                    displayLatency();
                    break;
                case posKeyTrainerMode:
                    p_keyTrainerMode += (t + 1);                     // Key TRX: 0=never, 1= keyer only, 2 = keyer & trainer
                    p_keyTrainerMode = constrain(p_keyTrainerMode - 1, 0, 2);
                    displayKeyTrainerMode();
                    break;
                case posInterWordSpace:
                    p_interWordSpace += t;                         // interword space in lengths of dit
                    p_interWordSpace = constrain(p_interWordSpace, 6, 45);            // has to be between 6 and 45 dits
                    displayInterWordSpace();
                    updateTimings();
                    break;
                case posInterCharSpace:
                    p_interCharSpace = constrain(p_interCharSpace + t, 3, 24);  // set Interchar space - 3 - 24 dits
                    displayInterCharSpace();
                    updateTimings();
                    break;
                case posKochFilter:
                    p_kochFilter = constrain(p_kochFilter + t, 1, kochChars.length());
                    displayKochFilter();
                    break;
                    //case  posGenerate : p_generatorMode = (p_generatorMode + t + 6) % 6;     // what trainer generates (0 - 5)
                    //               displayGenerate();
                    //               break;
                case posRandomOption:
                    p_randomOption = (p_randomOption + t + 10) % 10;     // which char set for random chars?
                    displayRandomOption();
                    break;
                case posRandomLength:
                    p_randomLength += t;                                 // length of random char group: 2-6
                    p_randomLength = constrain(p_randomLength, 1, 10);                   // 7-10 for rnd length 2 to 3-6
                    displayRandomLength();
                    break;
                case posCallLength:
                    if (p_callLength)                                             // length of calls: 0, or 3-6
                        p_callLength -= 2;                                        // temorarily make it 0-4
                    p_callLength = constrain(p_callLength + t, 0, 4);
                    if (p_callLength)                                             // length of calls: 0, or 3-6
                        p_callLength += 2;                                        // expand again if not 0

                    displayCallLength();
                    break;
                case posAbbrevLength:
                    p_abbrevLength += (t + 1);                                 // length of abbreviations: 0, or 2-6
                    if (p_abbrevLength == 2)                                      // get rid of 1
                        p_abbrevLength += t;
                    p_abbrevLength = constrain(p_abbrevLength - 1, 0, 6);
                    displayAbbrevLength();
                    break;
                case posWordLength:
                    p_wordLength += (t + 1);                                   // length of English words: 0, or 2-6
                    if (p_wordLength == 2)                                        // get rid of 1
                        p_wordLength += t;
                    p_wordLength = constrain(p_wordLength - 1, 0, 6);
                    displayWordLength();
                    break;
                case posMaxSequence:
                    switch (p_maxSequence)
                    {
                        case 0:
                            if (t == -1)
                                p_maxSequence = 250;
                            else
                                p_maxSequence = 5;
                            break;
                        case 250:
                            if (t == -1)
                                p_maxSequence = 245;
                            else
                                p_maxSequence = 0;
                            break;
                        default:
                            p_maxSequence += 5 * t;
                            break;
                    }
                    displayMaxSequence();
                    break;
                case posTrainerDisplay:
                    p_trainerDisplay = (p_trainerDisplay + t + 3) % 3;   // display options for trainer: 0-2
                    displayTrainerDisplay();
                    break;
                case posEchoDisplay:
                    p_echoDisplay += t;
                    p_echoDisplay = constrain(p_echoDisplay, 1, 3);             // what prompt for echo trainer mode
                    displayEchoDisplay();
                    break;
                case posEchoRepeats:
                    p_echoRepeats += (t + 1);                                 // no of echo repeats: 0-6, 7=forever
                    p_echoRepeats = constrain(p_echoRepeats - 1, 0, 7);
                    displayEchoRepeats();
                    break;
                case posEchoToneShift:
                    p_echoToneShift += (t + 1);                             // echo tone shift can be 0, 1 (up) or 2 (down)
                    p_echoToneShift = constrain(p_echoToneShift - 1, 0, 2);
                    displayEchoToneShift();
                    break;
                case posWordDoubler:
                    p_wordDoubler = !p_wordDoubler;
                    displayWordDoubler();
                    break;
                case posRandomFile:
                    if (p_randomFile)
                        p_randomFile = 0;
                    else
                        p_randomFile = 255;
                    displayRandomFile();
                    break;
                case posEchoConf:
                    p_echoConf = !p_echoConf;
                    displayEchoConf();
                    break;
                case posLoraTrainerMode:
                    p_loraTrainerMode += (t + 2);                    // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    p_loraTrainerMode = (p_loraTrainerMode % 2);
                    displayLoraTrainerMode();
                    break;
                case posLoraSyncW:
                    p_loraSyncW = (p_loraSyncW == 0x27 ? 0x66 : 0x27);
                    displayLoraSyncW();
                    break;
                case posGoertzelBandwidth:
                    p_goertzelBandwidth += (t + 2);                  // transmit lora in generator and player mode; can be 0 (no) or 1 (yes)
                    p_goertzelBandwidth = (p_goertzelBandwidth % 2);
                    displayGoertzelBandwidth();
                    break;
                case posSpeedAdapt:
                    p_speedAdapt = !p_speedAdapt;
                    displaySpeedAdapt();
                    break;
                case posKochSeq:
                    p_lcwoKochSeq = !p_lcwoKochSeq;
                    displayKochSeq();
                    break;
                case posTimeOut:
                    p_timeOut += (t + 1);
                    p_timeOut = constrain(p_timeOut - 1, 1, 4);
                    displayTimeOut();
                    break;
                case posQuickStart:
                    p_quickStart = !p_quickStart;
                    displayQuickStart();
                    break;
                case posLoraBand:
                    p_loraBand += (t + 1);                              // set the LoRa band
                    p_loraBand = constrain(p_loraBand - 1, 0, 2);
                    displayLoraBand();                                // display LoRa band
                    switch (p_loraBand)
                    {
                        case 0:
                            p_loraQRG = QRG433;
                            break;
                        case 1:
                            p_loraQRG = QRG866;
                            break;
                        case 2:
                            p_loraQRG = QRG920;
                            break;
                    }
                    break;
                case posLoraQRG:
                    p_loraQRG += (t * 1E5);
                    switch (p_loraBand)
                    {
                        case 0:
                            p_loraQRG = constrain(p_loraQRG, 433.65E6, 434.55E6);
                            break;
                        case 1:
                            p_loraQRG = constrain(p_loraQRG, 866.25E6, 869.45E6);
                            break;
                        case 2:
                            p_loraQRG = constrain(p_loraQRG, 920.25E6, 923.15E6);
                            break;
                    }
                    displayLoraQRG();
                    break;
                case posSnapRecall:
                case posSnapRecall:
                    if (memCounter)
                    {
                        memPtr = (memPtr + t + memCounter + 1) % (memCounter + 1);
                        //memPtr += (t+1);
                        //memPtr = constrain(memPtr-1, 0, memCounter);
                    }
                    displaySnapRecall();
                    break;
                case posSnapStore:
                    memPtr = (memPtr + t + 9) % 9;
                    displaySnapStore();
                    break;
            }   // end switch(pos)
            display.display();                                                      // update the display

        }      // end if(encoderPos)
        checkShutDown(false);         // check for time out
    }    // end while(true)
}   // end of function

