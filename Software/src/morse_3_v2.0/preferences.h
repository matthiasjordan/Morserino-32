#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "Arduino.h"
#include <Preferences.h>   // ESP 32 library for storing things in non-volatile storage

enum prefPos  { posClicks, posPitch, posExtPaddles, posPolarity,
                posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption,
                posRandomLength, posCallLength, posAbbrevLength, posWordLength,
                posTrainerDisplay, posWordDoubler, posEchoDisplay, posEchoRepeats,  posEchoConf,
                posKeyTrainerMode, posLoraTrainerMode, posGoertzelBandwidth, posSpeedAdapt,
                posKochSeq, posKochFilter, posLatency, posRandomFile,
                posTimeOut, posQuickStart, posLoraSyncW,
                posLoraBand, posLoraQRG, posSnapRecall, posSnapStore,
                posMaxSequence};

const String prefOption[] = { "Encoder Click", "Tone Pitch Hz", "External Pol.", "Paddle Polar.",
                              "Keyer Mode   ", "CurtisB DahT%", "CurtisB DitT%", "AutoChar Spce",
                              "Tone Shift   ", "InterWord Spc", "InterChar Spc", "Random Groups",
                              "Length Rnd Gr", "Length Calls ", "Length Abbrev", "Length Words ",
                              "CW Gen Displ ", "Each Word 2x ", "Echo Prompt  ", "Echo Repeats ", "Confrm. Tone ",
                              "Key ext TX   ", "Send via LoRa", "Bandwidth    ", "Adaptv. Speed",
                              "Koch Sequence", "Koch         ", "Latency      ", "Randomize File",
                              "Time Out     ", "Quick Start  ", "LoRa Channel  ",
                              "LoRa Band    ", "LoRa Frequ   ", "RECALLSnapshot", "STORE Snapshot",
                              "Max # of Words"};

 prefPos keyerOptions[] =      {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS, posKeyTrainerMode, posTimeOut, posQuickStart };
 prefPos generatorOptions[] =  {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomOption,
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence,
                                    posTrainerDisplay, posWordDoubler, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posTimeOut, posQuickStart };
 prefPos headOptions[] =  {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomOption,
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence,
                                    posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posTimeOut, posQuickStart };
 prefPos playerOptions[] =     {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posMaxSequence, posTrainerDisplay,
                                     posRandomFile, posWordDoubler, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posTimeOut, posQuickStart };
 prefPos echoPlayerOptions[] = {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posMaxSequence, posRandomFile, posEchoRepeats,  posEchoDisplay, posEchoConf, posTimeOut, posQuickStart};
 prefPos echoTrainerOptions[]= {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption,
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence, posEchoRepeats,  posEchoDisplay, posEchoConf, posSpeedAdapt, posTimeOut, posQuickStart };
 prefPos kochGenOptions[] =    {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace,
                                    posRandomLength,  posAbbrevLength, posWordLength, posMaxSequence,
                                    posTrainerDisplay, posWordDoubler, posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posKochSeq, posTimeOut, posQuickStart };
 prefPos kochEchoOptions[] =   {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace,
                                    posRandomLength, posAbbrevLength, posWordLength, posMaxSequence, posEchoRepeats, posEchoDisplay, posEchoConf, posSpeedAdapt, posKochSeq, posTimeOut, posQuickStart };
 prefPos loraTrxOptions[] =    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posTimeOut, posQuickStart, posLoraSyncW };
 prefPos extTrxOptions[] =     {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posGoertzelBandwidth, posTimeOut, posQuickStart };
 prefPos decoderOptions[] =    {posClicks, posPitch, posGoertzelBandwidth, posTimeOut, posQuickStart };

 prefPos allOptions[] =        { posClicks, posPitch, posExtPaddles, posPolarity, posLatency,
                                    posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption,
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence,
                                    posTrainerDisplay, posRandomFile, posWordDoubler, posEchoRepeats, posEchoDisplay, posEchoConf,
                                    posKeyTrainerMode, posLoraTrainerMode, posLoraSyncW, posGoertzelBandwidth, posSpeedAdapt, posKochSeq, posTimeOut, posQuickStart};



namespace MorsePreferences {

	void readPreferences(String repository);
	void writePreferences(String repository);
	boolean  recallSnapshot();
	boolean storeSnapshot(uint8_t menu);
	void updateMemory(uint8_t temp);
	void clearMemory(uint8_t ptr);
}

#endif
