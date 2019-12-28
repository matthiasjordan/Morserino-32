#include "MorseSound.h"
#include "MorsePreferences.h"


using namespace MorseSound;

    /////////////////////// parameters for LF tone generation and  HF (= vol ctrl) PWM
    int toneFreq = 500 ;
    int toneChannel = 2;      // this PWM channel is used for LF generation, duty cycle is 0 (silent) or 50% (tone)
    int lineOutChannel = 3;   // this PWM channel is used for line-out LF generation, duty cycle is 0 (silent) or 50% (tone)
    int volChannel = 8;       // this PWM channel is used for HF generation, duty cycle between 1% (almost silent) and 100% (loud)
    int pwmResolution = 10;
    unsigned int volFreq = 32000; // this is the HF frequency we are using

    const int  dutyCycleFiftyPercent =  512;
    const int  dutyCycleTwentyPercent = 25;
    const int  dutyCycleZero = 0;

    const int notes[] = {0, 233, 262, 294, 311, 349, 392, 440, 466, 523, 587, 622, 698, 784, 880, 932};





    //// functions for generating a tone....


void MorseSound::setup() {
    // set up PWMs for tone generation
      ledcSetup(toneChannel, toneFreq, pwmResolution);
      ledcAttachPin(LF_Pin, toneChannel);

      ledcSetup(lineOutChannel, toneFreq, pwmResolution);
      ledcAttachPin(lineOutPin, lineOutChannel);                                      ////// change this for real version - no line out currntly

      ledcSetup(volChannel, volFreq, pwmResolution);
      ledcAttachPin(HF_Pin, volChannel);

      ledcWrite(toneChannel,0);
      ledcWrite(lineOutChannel,0);
  }



void MorseSound::pwmTone(unsigned int frequency, unsigned int volume, boolean lineOut) { // frequency in Hertz, volume in range 0 - 100; we use 10 bit resolution
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


void MorseSound::pwmNoTone() {      // stop playing a tone by changing duty cycle of the tone to 0
  ledcWrite(toneChannel, dutyCycleTwentyPercent);
  ledcWrite(lineOutChannel, dutyCycleTwentyPercent);
  delayMicroseconds(125);
  ledcWrite(toneChannel, dutyCycleZero);
  ledcWrite(lineOutChannel, dutyCycleZero);

}


void MorseSound::pwmClick(unsigned int volume) {                        /// generate a click on the speaker
    if (!MorsePreferences::prefs.encoderClicks)
      return;
    pwmTone(250,volume, false);
    delay(6);
    pwmTone(280,volume, false);
    delay(5);
    pwmNoTone();
}
