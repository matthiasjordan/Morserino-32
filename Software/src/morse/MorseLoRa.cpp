/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/

#include <Arduino.h>
#include "SPIFFS.h"
#include <SPI.h>           // library for SPI interface
#include <LoRa.h>          // library for LoRa transceiver

#include "morsedefs.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include "MorsePreferencesMenu.h"
#include "MorseMachine.h"
#include "MorseLoRa.h"
#include "MorseKeyer.h"
#include "MorseGenerator.h"

using namespace MorseLoRa;

/////////////////// Variables for LoRa: Buffer management etc

char loraTxBuffer[32];

uint8_t loRaRxBuffer[256];
uint16_t byteBuFree = 256;
uint8_t nextBuWrite = 0;
uint8_t nextBuRead = 0;

uint8_t loRaSerial;                 /// a 6 bit serial number, start with some random value, will be incremented witch each sent LoRa packet
                                    /// the first two bits in teh byte will be the protocol id (CWLORAVERSION)

namespace internal
{
    void onReceive(int packetSize);
    void storePacket(int rssi, String packet);
    uint8_t loRaBuRead(uint8_t* buIndex);
    uint8_t loRaBuWrite(int rssi, String packet);
    void loraSystemSetup();
    uint8_t decodePacket(int* rssi, int* wpm, String* cwword);
}

void MorseLoRa::setup()
{
    /// check if BLACK knob has been pressed on startup - if yes, we have to perform LoRa Setup
    delay(50);
    if (SPIFFS.begin() && digitalRead(modeButtonPin) == LOW)
    {        // BLACK was pressed at start-up - checking for SPIFF so that programming 1st time w/o pull-up shows menu
        MorseDisplay::clearDisplay();
        MorseDisplay::printOnStatusLine(true, 0, "Release BLACK");
        while (digitalRead(modeButtonPin) == LOW)
        {
            // wait until released
            ;
        }
        internal::loraSystemSetup();
    }

    ////////////  Setup for LoRa
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);
    if (!LoRa.begin(MorsePreferences::prefs.loraQRG, PABOOSTx))
    {
        MORSELOGLN("Starting LoRa failed!");
        while (1)
        {
            ;
        }
    }
    LoRa.setFrequency(MorsePreferences::prefs.loraQRG);        /// default = 434.150 MHz - Region 1 ISM Band, can be changed by system setup
    LoRa.setSpreadingFactor(7);                         /// default
    LoRa.setSignalBandwidth(250E3);                     /// 250 kHz
    LoRa.noCrc();                                       /// we use error correction
    LoRa.setSyncWord(MorsePreferences::prefs.loraSyncW);                      /// the default would be 0x34

    // register the receive callback
    LoRa.onReceive(internal::onReceive);
    /// initialise the serial number
    loRaSerial = random(64);

}

void MorseLoRa::idle()
{
    LoRa.idle();
}

void MorseLoRa::receive()
{
    LoRa.receive();
}

//////// System Setup / LoRa Setup ///// Called when BALCK knob is pressed @ startup

void internal::loraSystemSetup()
{
    MorsePreferencesMenu::displayKeyerPreferencesMenu(MorsePreferences::posLoraBand);
    MorsePreferencesMenu::adjustKeyerPreference(MorsePreferences::posLoraBand);
    MorsePreferencesMenu::displayKeyerPreferencesMenu(MorsePreferences::posLoraQRG);
    MorsePreferencesMenu::adjustKeyerPreference(MorsePreferences::posLoraQRG);
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

void MorseLoRa::cwForLora(int element)
{
    static int pairCounter = 0;
    uint8_t temp;
    uint8_t header;

    if (pairCounter == 0)
    {   // we start a brand new word for LoRA - clear buffer and set speed first
        for (int i = 0; i < 32; ++i)
            loraTxBuffer[i] = (char) 0;

        /// 1st byte: version + serial number
        header = ++loRaSerial % 64;
        header += CWLORAVERSION * 64;        //// shift VERSION left 6 bits and add to the serial number
        loraTxBuffer[0] = header;

        temp = MorsePreferences::prefs.wpm * 4;                   /// shift left 2 bits
        loraTxBuffer[1] |= temp;
        pairCounter = 7;         /// so far we have used 7 bit pairs: 4 in the first byte (protocol version+serial); 3 in the 2nd byte (wpm)
    }

    temp = element & B011;      /// take the two left bits
    /// now store these two bits in the correct location in loraBuffer

    if (temp && (temp != 3))
    {                 /// no need to do the operation with 0, nor with B11
        temp = temp << (2 * (3 - (pairCounter % 4)));
        loraTxBuffer[pairCounter / 4] |= temp;
    }

    /// now increment, unless we got end of word
    /// have we get end of word, we got end of character (0) before

    if (temp != 3)
    {
        ++pairCounter;
    }
    else
    {
        --pairCounter; /// we are at end of word and step back to end of character
        if (pairCounter % 4 != 0)
        {           // do nothing if we have a zero in the topmost two bits already, as this was end of character
            temp = temp << (2 * (3 - (pairCounter % 4)));
            loraTxBuffer[pairCounter / 4] |= temp;
        }
        pairCounter = 0;
    }
}

void MorseLoRa::sendWithLora()
{           // hand this string over as payload to the LoRA transceiver
    // send packet
    LoRa.beginPacket();
    LoRa.print(loraTxBuffer);
    LoRa.endPacket();
    if (MorseMachine::isMode(MorseMachine::loraTrx))
    {
        LoRa.receive();
    }
}

void internal::onReceive(int packetSize)
{
    String result;
    result.reserve(64);
    result = "";

    // received a packet
    // read packet
    for (int i = 0; i < packetSize; i++)
    {
        result += (char) LoRa.read();
    }

    if (packetSize < 49)
    {
        internal::storePacket(LoRa.packetRssi(), result);
    }
    else
    {
        Serial.println("LoRa Packet longer than 48 bytes! Discarded...");
    }
    // print RSSI of packet
}

//// new buffer code: unpack when needed, to save buffer space. We just use 256 bytes of buffer, instead of 32k!
//// in addition to the received packet, we need to store the RSSI as 8 bit positive number
//// (it is always between -20 and -150, so an 8bit integer is fine as long as we store it without sign as an unsigned number)
//// the buffer is a 256 byte ring buffer with two pointers:
////   nextBuRead where the next packet starts for reading it out; is incremented by l to get the next buffer read position
////      you can read a packet as long as the buffer is not empty, so we need to check bytesBuFree before we read! if it is 256, the buffer is empty!
////      with a read, the bytesBuFree has to be increased by the number of bytes read
////   nextBuWrite where the next packet should be written; @write:
////       increment nextBuWrite by l to get new pointer; and decrement bytesBuFree by l to get new free space
//// we also need a variable that shows how many bytes are free (not in use): bytesBuFree
//// if the next packet to be stored is larger than bytesBuFree, it is discarded
//// structure of each packet:
////    l:  1 uint8_t length of packet
////    r:  1 uint8_t rssi as a positive number
////    d:  (var. length) data packet as received by LoRa
//// functions:
////    int loRaBuWrite(int rssi, String packet): returns length of buffer if successful. otherwise 0
////    uint8_t loRaBuRead(uint8_t* buIndex): returns length of packet, and index where to read in buffer by reference
////    boolean loRaBuReady():  true if there is something in the buffer, false otherwise
////      example:
////        (somewhere else as global var: ourBuffer[256]
////        uint8_t myIndex;
////        uint8_t mylength;
////        foo() {
////          myLength = loRaBuRead(&myIndex);
////          if (myLength != 0)
////            doSomethingWith(ourBuffer[myIndex], myLength);
////        }

uint8_t internal::loRaBuWrite(int rssi, String packet)
{
////   int loRaBuWrite(int rssi, String packet): returns length of buffer if successful. otherwise 0
////   nextBuWrite where the next packet should be written; @write:
////       increment nextBuWrite by l to get new pointer; and decrement bytesBuFree by l to get new free space
    uint8_t l, posRssi;

    posRssi = (uint8_t) abs(rssi);
    l = 2 + packet.length();
    if (byteBuFree < l)
    {                               // buffer full - discard packet
        return 0;
    }

    loRaRxBuffer[nextBuWrite++] = l;
    loRaRxBuffer[nextBuWrite++] = posRssi;
    for (int i = 0; i < packet.length(); ++i)
    {       // do this for all chars in the packet
        loRaRxBuffer[nextBuWrite++] = packet[i];       // at end nextBuWrite is alread where it should be
    }
    byteBuFree -= l;
    return l;
}

boolean MorseLoRa::loRaBuReady()
{
    if (byteBuFree == 256)
    {
        return false;
    }
    else
    {
        return true;
    }
}

uint8_t internal::loRaBuRead(uint8_t* buIndex)
{
////    uint8_t loRaBuRead(uint8_t* buIndex): returns length of packet, and index where to read in buffer by reference
    uint8_t l;
    if (byteBuFree == 256)
    {
        return 0;
    }
    else
    {
        l = loRaRxBuffer[nextBuRead++];
        *buIndex = nextBuRead;
        byteBuFree += l;
        --l;
        nextBuRead += l;
        return l;
    }
}

void internal::storePacket(int rssi, String packet)
{             // whenever we receive something, we just store it in our buffer
    if (loRaBuWrite(rssi, packet) == 0)
    {
        Serial.println("LoRa Buffer full");
    }
}

MorseLoRa::Packet MorseLoRa::decodePacket()
{
    MorseLoRa::Packet packet;

    uint8_t header = internal::decodePacket(&packet.rssi, &packet.rxWpm, &packet.payload);
    packet.protocolVersion = (header >> 6);
    if (packet.protocolVersion != CWLORAVERSION)
    {
        // invalid protocol version
        packet.valid = false;
        packet.payload = "";
    }

    if ((packet.rxWpm < 5) || (packet.rxWpm > 60))
    {
        // invalid speed
        packet.valid = false;
        packet.payload = "";
    }

    return packet;
}

/// decodePacket analyzes packet as received and stored in buffer
/// returns the header byte (protocol version*64 + 6bit packet serial number
//// byte 0 (added by receiver): RSSI
//// byte 1: header; first two bits are the protocol version (curently 01), plus 6 bit packet serial number (starting from random)
//// byte 2: first 6 bits are wpm (must be between 5 and 60; values 00 - 04 and 61 to 63 are invalid), the remaining 2 bits are already data payload!

uint8_t internal::decodePacket(int* rssi, int* wpm, String* cwword)
{
    uint8_t l, c, header = 0;
    uint8_t index = 0;

    l = internal::loRaBuRead(&index);           // where are we in  the buffer, and how long is the total packet inkl. rssi byte?

    for (int i = 0; i < l; ++i)
    {     // decoding loop
        c = loRaRxBuffer[index + i];

        switch (i)
        {
            case 0:
            {
                *rssi = (int) (-1 * c);    // the rssi byte
                break;
            }
            case 1:
            {
                header = c;
                break;
            }
            case 2:
            {
                *wpm = (uint8_t) (c >> 2); // the first data byte contains the wpm info in the first six bits, and actual morse in the remaining two bits
                                           // now take remaining two bits and store them in CWword as ASCII
                *cwword = (char) ((c & B011) + 48);
                break;
            }
            default:
            {
                // decode the rest of the packet; we do this for all but the first byte  /// we need to handle end of word!!! therefore the break

                for (int j = 0; j < 4; ++j)
                {
                    char cc = ((c >> 2 * (3 - j)) & B011);                // we store them as ASCII characters 0,1,2,3 !
                    if (cc != 3)
                    {
                        *cwword += (char) (cc + 48);
                    }
                    else
                    {
                        break;
                    }
                }
                break;
            }
        } // end switch
    }   // end for
    return header;
}      // end decodePacket

