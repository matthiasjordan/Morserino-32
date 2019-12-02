#ifndef MORSEDEFS_H
#define MORSEDEFS_H


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




enum FONT_ATTRIB {REGULAR, BOLD, INVERSE_REGULAR, INVERSE_BOLD};


#endif
