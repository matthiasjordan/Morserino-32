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

#include "MorseDisplay.h"
#include "MorsePreferencesMenu.h"
#include "MorseLoRa.h"

using namespace MorseLoRa;

/////////////////// Variables for LoRa: Buffer management etc

uint8_t loRaRxBuffer[256];
uint16_t byteBuFree = 256;
uint8_t nextBuWrite = 0;
uint8_t nextBuRead = 0;


namespace internal
{
    void onReceive(int packetSize);
    void storePacket(int rssi, String packet);
    uint8_t loRaBuRead(uint8_t* buIndex);
    uint8_t loRaBuWrite(int rssi, String packet);
    void loraSystemSetup();
    RawPacket decodePacket();
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
    LoRa.setFrequency(MorsePreferences::prefs.loraQRG);    /// default = 434.150 MHz - Region 1 ISM Band, can be changed by system setup
    LoRa.setSpreadingFactor(7);                         /// default
    LoRa.setSignalBandwidth(250E3);                     /// 250 kHz
    LoRa.noCrc();                                       /// we use error correction
    LoRa.setSyncWord(MorsePreferences::prefs.loraSyncW);                      /// the default would be 0x34

    // register the receive callback
    LoRa.onReceive(internal::onReceive);
    /// initialise the serial number
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



void MorseLoRa::sendWithLora(const char loraTxBuffer[])
{           // hand this string over as payload to the LoRA transceiver
    // send packet
    LoRa.beginPacket();
    LoRa.print(loraTxBuffer);
    LoRa.endPacket();
    LoRa.receive();
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


/// decodePacket analyzes packet as received and stored in buffer
/// returns the header byte (protocol version*64 + 6bit packet serial number
//// byte 0 (added by receiver): RSSI
//// byte 1: header; first two bits are the protocol version (curently 01), plus 6 bit packet serial number (starting from random)
//// byte 2: first 6 bits are wpm (must be between 5 and 60; values 00 - 04 and 61 to 63 are invalid), the remaining 2 bits are already data payload!

RawPacket MorseLoRa::decodePacket()
{
    RawPacket rp;
    uint8_t l, c = 0;
    uint8_t index = 0;

    l = internal::loRaBuRead(&index);           // where are we in  the buffer, and how long is the total packet inkl. rssi byte?
    rp.payloadLength = l - 2;
    rp.payload = (uint8_t *) malloc(l);
    uint8_t *p = rp.payload;

    for (int i = 0; i < l; ++i)
    {     // decoding loop
        c = loRaRxBuffer[index + i];
        switch (i)
        {
            case 0:
            {
                rp.rssi = (int) (-1 * c);    // the rssi byte
                break;
            }
            case 1:
            {
                rp.header = c;
                break;
            }
            default:
            {
                *p = c;
                p += 1;
            }
        } // end switch
    }   // end for
    return rp;
}

