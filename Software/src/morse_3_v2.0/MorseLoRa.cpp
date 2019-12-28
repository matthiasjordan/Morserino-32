#include <Arduino.h>
#include "SPIFFS.h"
#include <SPI.h>           // library for SPI interface
#include <LoRa.h>          // library for LoRa transceiver

#include "morsedefs.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include "MorseLoRa.h"
#include "menu.h"

using namespace MorseLoRa;


/////////////////// Variables for LoRa: Buffer management etc

char loraTxBuffer[32];

uint8_t loRaRxBuffer[256];
uint16_t byteBuFree = 256;
uint8_t nextBuWrite = 0;
uint8_t nextBuRead = 0;

uint8_t loRaSerial;                                     /// a 6 bit serial number, start with some random value, will be incremented witch each sent LoRa packet
                                                        /// the first two bits in teh byte will be the protocol id (CWLORAVERSION)


namespace MorseLoRa::internal {
    void MorseLoRa::internal::loraSystemSetup();
    void onReceive(int packetSize);
}


void MorseLoRa::setup() {
    /// check if BLACK knob has been pressed on startup - if yes, we have to perform LoRa Setup
      delay(50);
      if (SPIFFS.begin() && digitalRead(modeButtonPin) == LOW)   {        // BLACK was pressed at start-up - checking for SPIFF so that programming 1st time w/o pull-up shows menu
          MorseDisplay::clearDisplay();
          MorseDisplay::printOnStatusLine(true, 0,  "Release BLACK");
          while (digitalRead(modeButtonPin) == LOW)      // wait until released
          ;
        internal::loraSystemSetup();
      }


    ////////////  Setup for LoRa
      SPI.begin(SCK,MISO,MOSI,SS);
      LoRa.setPins(SS,RST,DI0);
      if (!LoRa.begin(MorsePreferences::prefs.loraQRG,PABOOST)) {
        Serial.println("Starting LoRa failed!");
        while (1);
      }
      LoRa.setFrequency(MorsePreferences::prefs.loraQRG);                       /// default = 434.150 MHz - Region 1 ISM Band, can be changed by system setup
      LoRa.setSpreadingFactor(7);                         /// default
      LoRa.setSignalBandwidth(250E3);                     /// 250 kHz
      LoRa.noCrc();                                       /// we use error correction
      LoRa.setSyncWord(MorsePreferences::prefs.loraSyncW);                      /// the default would be 0x34

      // register the receive callback
      LoRa.onReceive(internal::onReceive);
      /// initialise the serial number
      loRaSerial = random(64);

}



//////// System Setup / LoRa Setup ///// Called when BALCK knob is pressed @ startup

void MorseLoRa::internal::loraSystemSetup() {
    Menu::displayKeyerPreferencesMenu(MorsePreferences::posLoraBand);
    Menu::adjustKeyerPreference(MorsePreferences::posLoraBand);
    Menu::displayKeyerPreferencesMenu(MorsePreferences::posLoraQRG);
    Menu::adjustKeyerPreference(MorsePreferences::posLoraQRG);
    /// now store chosen values in Preferences

    MorsePreferences::writeLoRaPrefs(MorsePreferences::prefs.loraBand, MorsePreferences::prefs.loraQRG);
}


/// cwForLora packs element info (dit, dah, interelement space) into a String that can be sent via LoRA
///  element can be:
///  0: inter-element space
///  1: dit
///  2: dah
///  3: end of word -: cwForLora returns a string that is ready for sending to the LoRa transceiver

//  char loraTxBuffer[32];

void cwForLora (int element) {
  //static String result;
  //result.reserve(36);
  //static char buf[32];
  static int pairCounter = 0;
  uint8_t temp;
  uint8_t header;

  if (pairCounter == 0) {   // we start a brand new word for LoRA - clear buffer and set speed first
      for (int i=0; i<32; ++i)
          loraTxBuffer[i] = (char) 0;

      /// 1st byte: version + serial number
      header = ++loRaSerial % 64;
      //Serial.println("loRaSerial: " + String(loRaSerial));
      header += CWLORAVERSION * 64;        //// shift VERSION left 6 bits and add to the serial number
      loraTxBuffer[0] = header;

      temp = MorsePreferences::prefs.wpm * 4;                   /// shift left 2 bits
      loraTxBuffer[1] |= temp;
      pairCounter = 7;                    /// so far we have used 7 bit pairs: 4 in the first byte (protocol version+serial); 3 in the 2nd byte (wpm)
      //Serial.println(temp);
      //Serial.println(loraBuffer);
      }

  temp = element & B011;      /// take the two left bits
      //Serial.println("Temp before shift: " + String(temp));
  /// now store these two bits in the correct location in loraBuffer

  if (temp && (temp != 3)) {                 /// no need to do the operation with 0, nor with B11
      temp = temp << (2*(3-(pairCounter % 4)));
      loraTxBuffer[pairCounter/4] |= temp;
  }

  /// now increment, unless we got end of word
  /// have we get end of word, we got end of character (0) before

  if (temp != 3)
      ++pairCounter;
  else {
      --pairCounter; /// we are at end of word and step back to end of character
      if (pairCounter % 4 != 0)      {           // do nothing if we have a zero in the topmost two bits already, as this was end of character
          temp = temp << (2*(3-(pairCounter % 4)));
          loraTxBuffer[pairCounter/4] |= temp;
      }
      pairCounter = 0;
  }
}


void sendWithLora() {           // hand this string over as payload to the LoRA transceiver
  // send packet
  LoRa.beginPacket();
  LoRa.print(loraTxBuffer);
  LoRa.endPacket();
  if (morseState == loraTrx)
      LoRa.receive();
}

void onReceive(int packetSize)
{
  String result;
  result.reserve(64);
  result = "";

  // received a packet
  // read packet
  for (int i = 0; i < packetSize; i++)
  {
    result += (char)LoRa.read();
    //Serial.print((char)LoRa.read());
  }
  if (packetSize < 49)
      storePacket(LoRa.packetRssi(), result);
  else
      Serial.println("LoRa Packet longer than 48 bytes! Discarded...");
  // print RSSI of packet
  //Serial.print("' with RSSI ");
  //Serial.println(LoRa.packetRssi());
  //Serial.print(" S-Meter: ");
  //Serial.println(map(LoRa.packetRssi(), -160, -20, 0, 100));
}

